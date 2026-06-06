/**
 * @file 05_example_binary_protocol.cpp
 * @brief 高性能协议设计: TLV (Type-Length-Value) 协议实现, 二进制帧
 * @description 对应文档: 高性能网络与异步IO / 第5节 高性能协议设计
 */

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <cstdint>
#include <stdexcept>
#include <algorithm>
#include <sstream>
#include <iomanip>
#include <optional>

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
// TLV协议定义
// ============================================================

/// TLV类型枚举
enum class TlvType : uint16_t {
    HEARTBEAT    = 0x0001,   // 心跳
    AUTH_REQUEST = 0x0002,   // 认证请求
    AUTH_RESPONSE= 0x0003,   // 认证响应
    DATA         = 0x0004,   // 数据消息
    ACK          = 0x0005,   // 确认
    DISCONNECT   = 0x0006,   // 断开连接
};

/// TLV帧头 (固定8字节)
/// ┌──────────┬──────────┬──────────┬──────────┐
/// │  Magic   │   Type   │  Length  │   Seq    │
/// │ (2字节)  │ (2字节)  │ (4字节)  │ (4字节)  │
/// └──────────┴──────────┴──────────┴──────────┘
/// Magic: 0xABCD (固定魔数, 用于识别协议)
/// Type:  消息类型
/// Length: Value字段的字节长度
/// Seq:   序列号 (用于请求-响应匹配)
struct TlvHeader {
    uint16_t magic;    // 魔数 0xABCD
    uint16_t type;     // 消息类型
    uint32_t length;   // Value长度
    uint32_t sequence; // 序列号

    /// 转换为网络字节序 (大端)
    void to_network_order() {
        magic = htons(magic);
        type = htons(type);
        length = htonl(length);
        sequence = htonl(sequence);
    }

    /// 从网络字节序转换为主机字节序
    void to_host_order() {
        magic = ntohs(magic);
        type = ntohs(type);
        length = ntohl(length);
        sequence = ntohl(sequence);
    }

    /// 序列化为字节流
    std::vector<uint8_t> serialize() const {
        TlvHeader net_order = *this;
        net_order.to_network_order();

        std::vector<uint8_t> bytes(sizeof(TlvHeader));
        std::memcpy(bytes.data(), &net_order, sizeof(TlvHeader));
        return bytes;
    }

    /// 从字节流反序列化
    static bool deserialize(const uint8_t* data, size_t len, TlvHeader& header) {
        if (len < sizeof(TlvHeader)) return false;
        std::memcpy(&header, data, sizeof(TlvHeader));
        header.to_host_order();
        return true;
    }

    static constexpr uint16_t MAGIC_VALUE = 0xABCD;
    static constexpr size_t HEADER_SIZE = 12;  // 实际大小
};

// ============================================================
// TLV消息类
// ============================================================
class TlvMessage {
public:
    TlvMessage() = default;

    TlvMessage(TlvType type, uint32_t seq, const std::vector<uint8_t>& value)
        : type_(type), sequence_(seq), value_(value) {}

    TlvMessage(TlvType type, uint32_t seq, const std::string& str)
        : type_(type), sequence_(seq),
          value_(str.begin(), str.end()) {}

    /// 序列化为字节流
    std::vector<uint8_t> serialize() const {
        TlvHeader header{};
        header.magic = TlvHeader::MAGIC_VALUE;
        header.type = static_cast<uint16_t>(type_);
        header.length = static_cast<uint32_t>(value_.size());
        header.sequence = sequence_;

        auto header_bytes = header.serialize();

        std::vector<uint8_t> result;
        result.reserve(header_bytes.size() + value_.size());
        result.insert(result.end(), header_bytes.begin(), header_bytes.end());
        result.insert(result.end(), value_.begin(), value_.end());

        return result;
    }

    /// 从字节流反序列化
    static std::optional<TlvMessage> deserialize(const uint8_t* data, size_t len) {
        TlvHeader header;
        if (!TlvHeader::deserialize(data, len, header)) {
            return std::nullopt;
        }

        // 校验魔数
        if (header.magic != TlvHeader::MAGIC_VALUE) {
            std::cerr << "[TLV] 魔数校验失败: 0x"
                      << std::hex << header.magic << " (期望: 0xABCD)\n";
            return std::nullopt;
        }

        // 检查数据是否完整
        if (len < TlvHeader::HEADER_SIZE + header.length) {
            return std::nullopt;  // 数据不完整
        }

        TlvMessage msg;
        msg.type_ = static_cast<TlvType>(header.type);
        msg.sequence_ = header.sequence;

        const uint8_t* value_ptr = data + TlvHeader::HEADER_SIZE;
        msg.value_.assign(value_ptr, value_ptr + header.length);

        return msg;
    }

    /// 获取消息占用的总字节数
    size_t total_size() const {
        return TlvHeader::HEADER_SIZE + value_.size();
    }

    // 访问器
    TlvType type() const { return type_; }
    uint32_t sequence() const { return sequence_; }
    const std::vector<uint8_t>& value() const { return value_; }

    /// 将Value解释为字符串
    std::string value_as_string() const {
        return std::string(value_.begin(), value_.end());
    }

    /// 获取类型名称
    std::string type_name() const {
        switch (type_) {
            case TlvType::HEARTBEAT:     return "HEARTBEAT";
            case TlvType::AUTH_REQUEST:  return "AUTH_REQUEST";
            case TlvType::AUTH_RESPONSE: return "AUTH_RESPONSE";
            case TlvType::DATA:          return "DATA";
            case TlvType::ACK:           return "ACK";
            case TlvType::DISCONNECT:    return "DISCONNECT";
            default:                     return "UNKNOWN(0x" +
                std::to_string(static_cast<uint16_t>(type_)) + ")";
        }
    }

private:
    TlvType type_ = TlvType::HEARTBEAT;
    uint32_t sequence_ = 0;
    std::vector<uint8_t> value_;
};

// ============================================================
// 帧解析器 (处理TCP粘包/半包)
// ============================================================
class FrameParser {
public:
    /// 将接收到的数据追加到缓冲区
    void append(const uint8_t* data, size_t len) {
        buffer_.insert(buffer_.end(), data, data + len);
    }

    /// 尝试从缓冲区中解析出一条完整消息
    /// 返回解析出的消息, 以及是否成功
    std::optional<TlvMessage> parse_one() {
        if (buffer_.size() < TlvHeader::HEADER_SIZE) {
            return std::nullopt;  // 数据不够一个头部
        }

        // 先解析头部获取length
        TlvHeader header;
        TlvHeader::deserialize(buffer_.data(), buffer_.size(), header);

        // 检查是否有足够的数据
        size_t total = TlvHeader::HEADER_SIZE + header.length;
        if (buffer_.size() < total) {
            return std::nullopt;  // 数据不完整 (半包)
        }

        // 解析完整消息
        auto msg = TlvMessage::deserialize(buffer_.data(), total);
        if (msg) {
            // 移除已解析的数据
            buffer_.erase(buffer_.begin(), buffer_.begin() + static_cast<ptrdiff_t>(total));
        }
        return msg;
    }

    /// 缓冲区中的未解析数据大小
    size_t pending_size() const { return buffer_.size(); }

    /// 清空缓冲区
    void clear() { buffer_.clear(); }

private:
    std::vector<uint8_t> buffer_;
};

// ============================================================
// 辅助函数: 十六进制转储
// ============================================================
std::string hex_dump(const std::vector<uint8_t>& data, size_t max_bytes = 64) {
    std::ostringstream oss;
    size_t n = std::min(data.size(), max_bytes);
    for (size_t i = 0; i < n; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0')
            << static_cast<int>(data[i]) << " ";
    }
    if (data.size() > max_bytes) {
        oss << "... (共 " << data.size() << " 字节)";
    }
    return oss.str();
}

// ============================================================
// 演示1: TLV协议基本操作
// ============================================================
void demo_tlv_basic() {
    std::cout << "\n========================================\n";
    std::cout << "  演示1: TLV协议基本操作\n";
    std::cout << "========================================\n\n";

    std::cout << "【TLV协议格式】\n\n";

    std::cout << "  ┌──────────┬──────────┬──────────┬──────────┬─────────────┐\n";
    std::cout << "  │  Magic   │   Type   │  Length  │   Seq    │    Value    │\n";
    std::cout << "  │ (2字节)  │ (2字节)  │ (4字节)  │ (4字节)  │ (Length字节)│\n";
    std::cout << "  └──────────┴──────────┴──────────┴──────────┴─────────────┘\n\n";

    std::cout << "  Magic: 0xABCD  固定魔数, 快速识别协议\n";
    std::cout << "  Type:  消息类型 (心跳/数据/确认等)\n";
    std::cout << "  Length: Value字段的字节长度\n";
    std::cout << "  Seq:   序列号, 请求-响应匹配\n";
    std::cout << "  Value: 消息体, 长度由Length指定\n\n";

    // 创建各种类型的消息
    std::cout << "【创建TLV消息】\n\n";

    TlvMessage heartbeat(TlvType::HEARTBEAT, 1, std::vector<uint8_t>{});
    TlvMessage auth_req(TlvType::AUTH_REQUEST, 2, "user:password");
    TlvMessage data_msg(TlvType::DATA, 3, "这是一条数据消息");
    TlvMessage ack_msg(TlvType::ACK, 4, std::vector<uint8_t>{});

    // 序列化
    auto hb_bytes = heartbeat.serialize();
    auto auth_bytes = auth_req.serialize();
    auto data_bytes = data_msg.serialize();

    std::cout << "  心跳消息 (" << hb_bytes.size() << " 字节): "
              << hex_dump(hb_bytes) << "\n";
    std::cout << "  认证请求 (" << auth_bytes.size() << " 字节): "
              << hex_dump(auth_bytes) << "\n";
    std::cout << "  数据消息 (" << data_bytes.size() << " 字节): "
              << hex_dump(data_bytes) << "\n\n";

    // 反序列化
    std::cout << "【反序列化TLV消息】\n\n";

    auto parsed = TlvMessage::deserialize(data_bytes.data(), data_bytes.size());
    if (parsed) {
        std::cout << "  类型: " << parsed->type_name() << "\n";
        std::cout << "  序列号: " << parsed->sequence() << "\n";
        std::cout << "  内容: \"" << parsed->value_as_string() << "\"\n";
        std::cout << "  总大小: " << parsed->total_size() << " 字节\n";
    }
}

// ============================================================
// 演示2: TCP粘包/半包处理
// ============================================================
void demo_frame_parsing() {
    std::cout << "\n========================================\n";
    std::cout << "  演示2: TCP粘包/半包处理\n";
    std::cout << "========================================\n\n";

    std::cout << "【TCP粘包问题】\n";
    std::cout << "  TCP是字节流协议, 不保证消息边界:\n";
    std::cout << "  - 一次recv可能收到多条消息 (粘包)\n";
    std::cout << "  - 一次recv可能只收到半条消息 (半包)\n\n";

    std::cout << "  发送: [消息1][消息2][消息3]\n";
    std::cout << "  接收: [消息1+消息2][消息3前半] [消息3后半]\n\n";

    std::cout << "  解决方案: 使用帧解析器 (FrameParser)\n";
    std::cout << "  1. 将recv的数据追加到缓冲区\n";
    std::cout << "  2. 检查缓冲区是否有完整的帧\n";
    std::cout << "  3. 有则取出, 没有则等待更多数据\n\n";

    // 模拟粘包: 两条消息合并发送
    TlvMessage msg1(TlvType::DATA, 100, "第一条消息");
    TlvMessage msg2(TlvType::DATA, 101, "第二条消息");

    auto bytes1 = msg1.serialize();
    auto bytes2 = msg2.serialize();

    // 合并两条消息 (模拟粘包)
    std::vector<uint8_t> combined;
    combined.insert(combined.end(), bytes1.begin(), bytes1.end());
    combined.insert(combined.end(), bytes2.begin(), bytes2.end());

    std::cout << "【模拟粘包】\n";
    std::cout << "  合并发送 " << combined.size() << " 字节 (2条消息)\n\n";

    // 使用FrameParser解析
    FrameParser parser;
    parser.append(combined.data(), combined.size());

    int count = 0;
    while (auto msg = parser.parse_one()) {
        count++;
        std::cout << "  解析出消息" << count << ": "
                  << msg->type_name() << ", seq=" << msg->sequence()
                  << ", value=\"" << msg->value_as_string() << "\"\n";
    }
    std::cout << "  成功解析 " << count << " 条消息, 缓冲区剩余: "
              << parser.pending_size() << " 字节\n\n";

    // 模拟半包: 分两次接收
    std::cout << "【模拟半包】\n";
    TlvMessage msg3(TlvType::DATA, 200, "第三条消息(半包测试)");
    auto bytes3 = msg3.serialize();

    FrameParser parser2;
    // 第一次只收到一半
    size_t half = bytes3.size() / 2;
    parser2.append(bytes3.data(), half);
    std::cout << "  第一次接收 " << half << " 字节\n";

    auto result = parser2.parse_one();
    std::cout << "  尝试解析: " << (result ? "成功" : "数据不完整(半包)") << "\n";
    std::cout << "  缓冲区: " << parser2.pending_size() << " 字节\n";

    // 第二次收到剩余数据
    parser2.append(bytes3.data() + half, bytes3.size() - half);
    std::cout << "  第二次接收 " << (bytes3.size() - half) << " 字节\n";

    result = parser2.parse_one();
    if (result) {
        std::cout << "  解析成功: " << result->type_name()
                  << ", seq=" << result->sequence()
                  << ", value=\"" << result->value_as_string() << "\"\n";
    }
}

// ============================================================
// 演示3: TLV协议设计要点
// ============================================================
void demo_protocol_design_points() {
    std::cout << "\n========================================\n";
    std::cout << "  演示3: TLV协议设计要点\n";
    std::cout << "========================================\n\n";

    std::cout << "【为什么选择TLV格式?】\n\n";

    std::cout << "  1. 自描述\n";
    std::cout << "     Type标识消息类型, 无需外部schema\n";
    std::cout << "     Length标识消息长度, 解决粘包问题\n\n";

    std::cout << "  2. 可扩展\n";
    std::cout << "     新增Type不影响已有消息解析\n";
    std::cout << "     Value可以是任意格式(JSON/Protobuf/自定义)\n\n";

    std::cout << "  3. 高效解析\n";
    std::cout << "     固定长度头部, 解析简单\n";
    std::cout << "     无需扫描分隔符, O(1)确定帧边界\n\n";

    std::cout << "【常见二进制协议格式】\n\n";

    std::cout << "  1. TLV (Type-Length-Value)\n";
    std::cout << "     代表: LDAP, SNMP, EMV\n";
    std::cout << "     优点: 简单, 自描述\n\n";

    std::cout << "  2. Header+Body\n";
    std::cout << "     代表: HTTP/2, WebSocket, Redis RESP\n";
    std::cout << "     优点: 头部固定, 解析快\n\n";

    std::cout << "  3. Protobuf/FlatBuffers\n";
    std::cout << "     代表: gRPC, Android IPC\n";
    std::cout << "     优点: 跨语言, 高效序列化\n\n";

    std::cout << "【网络字节序注意事项】\n\n";

    std::cout << "  1. 多字节字段必须使用网络字节序 (大端)\n";
    std::cout << "     发送前: htons/htonl\n";
    std::cout << "     接收后: ntohs/ntohl\n\n";

    std::cout << "  2. 避免直接发送结构体\n";
    std::cout << "     不同平台可能有不同的对齐和填充\n";
    std::cout << "     使用memcpy逐字段序列化\n\n";

    std::cout << "  3. 魔数(Magic)的作用\n";
    std::cout << "     快速识别协议类型\n";
    std::cout << "     检测数据错位\n";
    std::cout << "     防御非法连接\n";
}

// ============================================================
// 主函数
// ============================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  第8章: 高性能网络与异步IO - 第5节 TLV协议与二进制帧\n";
    std::cout << "============================================================\n";

    demo_tlv_basic();
    demo_frame_parsing();
    demo_protocol_design_points();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
