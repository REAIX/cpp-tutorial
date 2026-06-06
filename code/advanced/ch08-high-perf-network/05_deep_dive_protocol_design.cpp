/**
 * @file 05_deep_dive_protocol_design.cpp
 * @brief 高性能协议设计深入: 协议版本化, 头部设计, 校验和
 * @description 对应文档: 高性能网络与异步IO / 第5节 高性能协议设计
 */

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <cstdint>
#include <optional>
#include <algorithm>
#include <sstream>
#include <iomanip>

#ifdef _WIN32
    #ifndef _WINSOCK2API_
        #include <winsock2.h>
        #include <ws2tcpip.h>
    #endif
    #ifdef _MSC_VER
        #pragma comment(lib, "ws2_32.lib")
    #endif
#else
    #include <arpa/inet.h>
#endif

// ============================================================
// 协议版本定义
// ============================================================
namespace protocol {

/// 协议版本号
enum class ProtocolVersion : uint8_t {
    V1 = 1,   // 初始版本: 基本TLV
    V2 = 2,   // 增加校验和
    V3 = 3,   // 增加压缩标志和扩展头
};

/// 消息标志位
enum class MessageFlags : uint8_t {
    NONE       = 0x00,
    COMPRESSED = 0x01,   // 数据已压缩
    URGENT     = 0x02,   // 紧急消息
    ENCRYPTED  = 0x04,   // 数据已加密
    BATCH      = 0x08,   // 批量消息
};

/// 消息类型
enum class MessageType : uint16_t {
    HEARTBEAT     = 0x0001,
    AUTH_REQUEST  = 0x0002,
    AUTH_RESPONSE = 0x0003,
    DATA          = 0x0004,
    ACK           = 0x0005,
    NACK          = 0x0006,
    DISCONNECT    = 0x0007,
    MSG_ERROR     = 0x0008,
};

// ============================================================
// V2协议头: 带校验和
// ============================================================
/// ┌──────────┬─────────┬──────────┬──────────┬──────────┬──────────┬──────────┐
/// │  Magic   │ Version │  Flags   │   Type   │  Length  │   Seq    │ Checksum │
/// │ (2字节)  │ (1字节)  │ (1字节)   │ (2字节)   │ (4字节)  │ (4字节)   │ (4字节)   │
/// └──────────┴─────────┴──────────┴──────────┴──────────┴──────────┴──────────┘
struct V2Header {
    uint16_t magic;      // 魔数 0xABCD
    uint8_t  version;    // 协议版本
    uint8_t  flags;      // 标志位
    uint16_t type;       // 消息类型
    uint32_t length;     // Value长度
    uint32_t sequence;   // 序列号
    uint32_t checksum;   // 校验和 (CRC32)

    static constexpr uint16_t MAGIC = 0xABCD;
    static constexpr size_t SIZE = 18;  // 头部总大小
    static constexpr uint8_t CURRENT_VERSION = 2;

    void to_network_order() {
        magic = htons(magic);
        type = htons(type);
        length = htonl(length);
        sequence = htonl(sequence);
        checksum = htonl(checksum);
        // version和flags是单字节, 无需转换
    }

    void to_host_order() {
        magic = ntohs(magic);
        type = ntohs(type);
        length = ntohl(length);
        sequence = ntohl(sequence);
        checksum = ntohl(checksum);
    }
};

// ============================================================
// CRC32校验和计算
// ============================================================
class CRC32 {
public:
    /// 计算CRC32校验和 (运行时计算, 避免大查找表)
    static uint32_t calculate(const uint8_t* data, size_t length) {
        uint32_t crc = 0xFFFFFFFF;
        for (size_t i = 0; i < length; ++i) {
            crc ^= data[i];
            for (int j = 0; j < 8; ++j) {
                crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
            }
        }
        return crc ^ 0xFFFFFFFF;
    }

    static uint32_t calculate(const std::vector<uint8_t>& data) {
        return calculate(data.data(), data.size());
    }
};

// ============================================================
// V2协议消息
// ============================================================
class V2Message {
public:
    V2Message() = default;

    V2Message(MessageType type, uint32_t seq, const std::vector<uint8_t>& value,
              uint8_t flags = 0)
        : type_(type), sequence_(seq), flags_(flags), value_(value) {}

    V2Message(MessageType type, uint32_t seq, const std::string& str,
              uint8_t flags = 0)
        : type_(type), sequence_(seq), flags_(flags),
          value_(str.begin(), str.end()) {}

    /// 序列化为字节流 (包含校验和)
    std::vector<uint8_t> serialize() const {
        V2Header header{};
        header.magic = V2Header::MAGIC;
        header.version = V2Header::CURRENT_VERSION;
        header.flags = flags_;
        header.type = static_cast<uint16_t>(type_);
        header.length = static_cast<uint32_t>(value_.size());
        header.sequence = sequence_;

        // 计算校验和: 对Value部分计算CRC32
        header.checksum = CRC32::calculate(value_);

        // 转为网络字节序
        header.to_network_order();

        // 拼接: Header + Value
        std::vector<uint8_t> result;
        result.reserve(V2Header::SIZE + value_.size());
        result.insert(result.end(),
                     reinterpret_cast<const uint8_t*>(&header),
                     reinterpret_cast<const uint8_t*>(&header) + V2Header::SIZE);
        result.insert(result.end(), value_.begin(), value_.end());

        return result;
    }

    /// 从字节流反序列化 (包含校验和验证)
    static std::optional<V2Message> deserialize(const uint8_t* data, size_t len) {
        if (len < V2Header::SIZE) return std::nullopt;

        V2Header header;
        std::memcpy(&header, data, V2Header::SIZE);
        header.to_host_order();

        // 校验魔数
        if (header.magic != V2Header::MAGIC) {
            std::cerr << "[协议] 魔数校验失败\n";
            return std::nullopt;
        }

        // 校验版本
        if (header.version > V2Header::CURRENT_VERSION) {
            std::cerr << "[协议] 不支持的版本: " << static_cast<int>(header.version) << "\n";
            return std::nullopt;
        }

        // 检查数据完整性
        if (len < V2Header::SIZE + header.length) {
            return std::nullopt;
        }

        // 提取Value
        std::vector<uint8_t> value(data + V2Header::SIZE,
                                   data + V2Header::SIZE + header.length);

        // 校验CRC32
        uint32_t expected_crc = CRC32::calculate(value);
        if (header.checksum != expected_crc) {
            std::cerr << "[协议] CRC32校验失败! 期望: 0x"
                      << std::hex << expected_crc
                      << ", 实际: 0x" << header.checksum << std::dec << "\n";
            return std::nullopt;
        }

        V2Message msg;
        msg.type_ = static_cast<MessageType>(header.type);
        msg.sequence_ = header.sequence;
        msg.flags_ = header.flags;
        msg.value_ = std::move(value);

        return msg;
    }

    size_t total_size() const { return V2Header::SIZE + value_.size(); }

    MessageType type() const { return type_; }
    uint32_t sequence() const { return sequence_; }
    uint8_t flags() const { return flags_; }
    const std::vector<uint8_t>& value() const { return value_; }
    std::string value_as_string() const {
        return std::string(value_.begin(), value_.end());
    }

    std::string type_name() const {
        switch (type_) {
            case MessageType::HEARTBEAT:     return "HEARTBEAT";
            case MessageType::AUTH_REQUEST:  return "AUTH_REQUEST";
            case MessageType::AUTH_RESPONSE: return "AUTH_RESPONSE";
            case MessageType::DATA:          return "DATA";
            case MessageType::ACK:           return "ACK";
            case MessageType::NACK:          return "NACK";
            case MessageType::DISCONNECT:    return "DISCONNECT";
            case MessageType::MSG_ERROR:      return "ERROR";
            default:                         return "UNKNOWN";
        }
    }

private:
    MessageType type_ = MessageType::HEARTBEAT;
    uint32_t sequence_ = 0;
    uint8_t flags_ = 0;
    std::vector<uint8_t> value_;
};

}  // namespace protocol

// ============================================================
// 演示1: 协议版本化
// ============================================================
void demo_protocol_versioning() {
    std::cout << "\n========================================\n";
    std::cout << "  演示1: 协议版本化\n";
    std::cout << "========================================\n\n";

    std::cout << "【为什么需要协议版本化?】\n\n";

    std::cout << "  1. 兼容性: 新旧客户端共存\n";
    std::cout << "  2. 演进: 逐步添加新功能\n";
    std::cout << "  3. 调试: 快速识别协议版本\n\n";

    std::cout << "【版本化策略】\n\n";

    std::cout << "  V1: 基本TLV (Magic + Type + Length + Seq + Value)\n";
    std::cout << "  V2: 增加Version字段 + Flags + Checksum\n";
    std::cout << "  V3: 增加压缩标志 + 扩展头长度\n\n";

    std::cout << "【版本兼容规则】\n\n";

    std::cout << "  1. 服务器必须支持所有历史版本\n";
    std::cout << "  2. 客户端使用最新版本\n";
    std::cout << "  3. 新版本只能添加字段, 不能修改已有字段\n";
    std::cout << "  4. 未知版本返回错误, 触发版本协商\n\n";

    std::cout << "【版本协商流程】\n";
    std::cout << "  客户端 → 服务器: V3消息\n";
    std::cout << "  服务器 → 客户端: NACK(版本不支持, 建议V2)\n";
    std::cout << "  客户端 → 服务器: V2消息 (降级)\n";
    std::cout << "  服务器 → 客户端: ACK\n";
}

// ============================================================
// 演示2: 头部设计与校验和
// ============================================================
void demo_header_and_checksum() {
    std::cout << "\n========================================\n";
    std::cout << "  演示2: 头部设计与校验和\n";
    std::cout << "========================================\n\n";

    std::cout << "【V2协议头部布局】\n\n";

    std::cout << "  偏移  字段      大小   说明\n";
    std::cout << "  ──────────────────────────────────\n";
    std::cout << "  0     Magic     2字节  固定0xABCD\n";
    std::cout << "  2     Version   1字节  协议版本\n";
    std::cout << "  3     Flags     1字节  标志位\n";
    std::cout << "  4     Type      2字节  消息类型\n";
    std::cout << "  6     Length    4字节  Value长度\n";
    std::cout << "  10    Sequence  4字节  序列号\n";
    std::cout << "  14    Checksum  4字节  CRC32校验和\n";
    std::cout << "  ──────────────────────────────────\n";
    std::cout << "  总计: 18字节\n\n";

    // 创建消息并序列化
    protocol::V2Message msg(protocol::MessageType::DATA, 42,
                            "测试CRC32校验和的消息内容");

    auto bytes = msg.serialize();

    std::cout << "【序列化示例】\n";
    std::cout << "  消息类型: " << msg.type_name() << "\n";
    std::cout << "  序列号: " << msg.sequence() << "\n";
    std::cout << "  内容: \"" << msg.value_as_string() << "\"\n";
    std::cout << "  总大小: " << bytes.size() << " 字节\n\n";

    // 反序列化并验证
    auto parsed = protocol::V2Message::deserialize(bytes.data(), bytes.size());
    if (parsed) {
        std::cout << "【反序列化成功, CRC32校验通过】\n";
        std::cout << "  类型: " << parsed->type_name() << "\n";
        std::cout << "  序列号: " << parsed->sequence() << "\n";
        std::cout << "  内容: \"" << parsed->value_as_string() << "\"\n\n";
    }

    // 模拟数据损坏
    std::cout << "【模拟数据损坏】\n";
    auto corrupted = bytes;
    if (corrupted.size() > 20) {
        corrupted[20] ^= 0xFF;  // 翻转一个字节
    }

    auto bad_result = protocol::V2Message::deserialize(corrupted.data(), corrupted.size());
    std::cout << "  损坏数据反序列化: "
              << (bad_result ? "成功(不应发生!)" : "失败(CRC32检测到损坏)") << "\n\n";

    std::cout << "【校验和算法选择】\n\n";

    std::cout << "  算法       大小   性能    检错能力\n";
    std::cout << "  ────────────────────────────────────\n";
    std::cout << "  校验和     1~2字节 最快   弱(无法检测顺序错误)\n";
    std::cout << "  CRC16      2字节   快     中等\n";
    std::cout << "  CRC32      4字节   中     强(检测99.99%错误)\n";
    std::cout << "  MD5        16字节  慢     极强(防篡改)\n";
    std::cout << "  SHA256     32字节  最慢   最强(密码学安全)\n\n";

    std::cout << "  推荐: 网络协议用CRC32, 安全场景用SHA256\n";
}

// ============================================================
// 演示3: 标志位与扩展设计
// ============================================================
void demo_flags_and_extensions() {
    std::cout << "\n========================================\n";
    std::cout << "  演示3: 标志位与扩展设计\n";
    std::cout << "========================================\n\n";

    std::cout << "【标志位设计】\n\n";

    std::cout << "  1字节 = 8个标志位, 每位代表一个布尔开关:\n\n";

    std::cout << "  Bit 0: COMPRESSED  数据已压缩\n";
    std::cout << "  Bit 1: URGENT      紧急消息(优先处理)\n";
    std::cout << "  Bit 2: ENCRYPTED   数据已加密\n";
    std::cout << "  Bit 3: BATCH       批量消息\n";
    std::cout << "  Bit 4-7: 保留\n\n";

    // 演示标志位组合
    uint8_t flags = static_cast<uint8_t>(protocol::MessageFlags::COMPRESSED)
                  | static_cast<uint8_t>(protocol::MessageFlags::URGENT);

    std::cout << "  示例: COMPRESSED | URGENT = 0x"
              << std::hex << static_cast<int>(flags) << std::dec << "\n\n";

    // 检查标志位
    bool is_compressed = (flags & static_cast<uint8_t>(protocol::MessageFlags::COMPRESSED)) != 0;
    bool is_urgent = (flags & static_cast<uint8_t>(protocol::MessageFlags::URGENT)) != 0;
    bool is_encrypted = (flags & static_cast<uint8_t>(protocol::MessageFlags::ENCRYPTED)) != 0;

    std::cout << "  is_compressed = " << std::boolalpha << is_compressed << "\n";
    std::cout << "  is_urgent     = " << is_urgent << "\n";
    std::cout << "  is_encrypted  = " << is_encrypted << "\n\n";

    // 创建带标志位的消息
    protocol::V2Message urgent_msg(protocol::MessageType::DATA, 100,
                                    "紧急数据!", flags);

    std::cout << "【协议扩展策略】\n\n";

    std::cout << "  1. 保留字段\n";
    std::cout << "     头部预留未使用的位/字节\n";
    std::cout << "     未来启用时无需修改头部结构\n\n";

    std::cout << "  2. TLV嵌套\n";
    std::cout << "     Value中可以包含TLV子结构\n";
    std::cout << "     实现无限嵌套扩展\n\n";

    std::cout << "  3. 扩展头\n";
    std::cout << "     Flags中增加HAS_EXTENSION标志\n";
    std::cout << "     Extension Length + Extension Data\n";
    std::cout << "     路由器/中间件可跳过扩展头\n\n";

    std::cout << "  4. 能力协商\n";
    std::cout << "     连接建立时交换支持的能力列表\n";
    std::cout << "     只使用双方都支持的功能\n\n";

    std::cout << "【协议设计最佳实践】\n\n";

    std::cout << "  1. 头部固定长度, 方便解析\n";
    std::cout << "  2. 使用网络字节序 (大端)\n";
    std::cout << "  3. 魔数 + 版本号 + 校验和\n";
    std::cout << "  4. 长度字段必须包含自身\n";
    std::cout << "  5. 预留扩展空间\n";
    std::cout << "  6. 限制最大消息大小\n";
    std::cout << "  7. 心跳保活机制\n";
    std::cout << "  8. 优雅关闭流程\n";
}

// ============================================================
// 主函数
// ============================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  第8章: 高性能网络与异步IO - 第5节 协议设计深入\n";
    std::cout << "============================================================\n";

    demo_protocol_versioning();
    demo_header_and_checksum();
    demo_flags_and_extensions();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
