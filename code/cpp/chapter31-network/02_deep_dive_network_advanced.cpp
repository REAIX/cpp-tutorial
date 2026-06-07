/**
 * @file 02_deep_dive_network_advanced.cpp
 * @brief 协议设计, 序列化, 心跳, 重连, 网络安全基础
 * @description 对应文档: 02-CPP/35-网络编程
 *  @note C 语言中使用原始 socket API 实现类似功能, 参见 C 章节 25-网络编程基础
 */

#include <iostream>
#include <string>
#include <vector>
#include <cstdint>
#include <cstring>
#include <chrono>
#include <algorithm>
#include <random>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

void demo_protocol_design() {
    std::cout << "\n=== demo_protocol_design ===\n";
    std::cout << "应用层协议设计\n\n";

    std::cout << "协议设计要素:\n";
    std::cout << "  1. 消息格式: 头部 + 载荷\n";
    std::cout << "  2. 消息边界: 长度前缀 or 分隔符\n";
    std::cout << "  3. 字节序: 网络字节序(大端)\n";
    std::cout << "  4. 版本号: 支持协议升级\n";
    std::cout << "  5. 错误码: 统一错误处理\n\n";

    struct MessageHeader {
        uint8_t magic;
        uint8_t version;
        uint16_t type;
        uint32_t length;
        uint32_t sequence;
    };

    std::cout << "消息头示例 (13字节):\n";
    std::cout << "  ┌───────┬───────┬───────┬───────┬───────┐\n";
    std::cout << "  │ magic │version│ type  │length │sequence│\n";
    std::cout << "  │ 1字节 │ 1字节 │2字节  │4字节  │4字节  │\n";
    std::cout << "  └───────┴───────┴───────┴───────┴───────┘\n\n";

    auto build_message = [](uint8_t type, const std::string& payload) -> std::vector<uint8_t> {
        std::vector<uint8_t> msg;
        MessageHeader hdr;
        hdr.magic = 0xAB;
        hdr.version = 1;
        hdr.type = htons(type);
        hdr.length = htonl(static_cast<uint32_t>(payload.size()));
        hdr.sequence = htonl(1);

        const uint8_t* hdr_bytes = reinterpret_cast<const uint8_t*>(&hdr);
        msg.assign(hdr_bytes, hdr_bytes + sizeof(hdr));
        msg.insert(msg.end(), payload.begin(), payload.end());
        return msg;
    };

    auto parse_message = [](const std::vector<uint8_t>& data) {
        if (data.size() < sizeof(MessageHeader)) {
            std::cout << "  消息太短\n";
            return;
        }
        MessageHeader hdr;
        std::memcpy(&hdr, data.data(), sizeof(hdr));
        hdr.type = ntohs(hdr.type);
        hdr.length = ntohl(hdr.length);
        hdr.sequence = ntohl(hdr.sequence);

        std::cout << "  解析消息: magic=0x" << std::hex << (int)hdr.magic
                  << ", version=" << std::dec << (int)hdr.version
                  << ", type=" << hdr.type
                  << ", length=" << hdr.length
                  << ", seq=" << hdr.sequence << "\n";

        if (data.size() >= sizeof(MessageHeader) + hdr.length) {
            std::string payload(data.begin() + sizeof(MessageHeader),
                              data.begin() + sizeof(MessageHeader) + hdr.length);
            std::cout << "  载荷: \"" << payload << "\"\n";
        }
    };

    auto msg = build_message(1, "Hello, Protocol!");
    std::cout << "构建消息 (" << msg.size() << " 字节):\n";
    parse_message(msg);

    std::cout << "\n协议设计最佳实践:\n";
    std::cout << "  1. 使用固定大小的头部\n";
    std::cout << "  2. 所有整数用网络字节序\n";
    std::cout << "  3. 包含magic number用于校验\n";
    std::cout << "  4. 支持版本号, 便于升级\n";
    std::cout << "  5. 限制最大消息大小\n";
    std::cout << "  6. 包含校验和(CRC32)\n";
}

void demo_heartbeat_mechanism() {
    std::cout << "\n=== demo_heartbeat_mechanism ===\n";
    std::cout << "心跳机制\n\n";

    std::cout << "心跳的作用:\n";
    std::cout << "  1. 检测死连接\n";
    std::cout << "  2. 保持NAT映射\n";
    std::cout << "  3. 测量网络延迟\n\n";

    std::cout << "心跳策略:\n";
    std::cout << "  1. 定时心跳: 每隔N秒发送一次\n";
    std::cout << "  2. 自适应心跳: 根据网络状况调整间隔\n";
    std::cout << "  3. 应用层心跳: 在协议中定义心跳消息\n\n";

    class HeartbeatManager {
        std::chrono::steady_clock::time_point last_recv_;
        std::chrono::steady_clock::time_point last_send_;
        int interval_ms_;
        int timeout_ms_;
        int missed_count_ = 0;
        int max_missed_ = 3;

    public:
        HeartbeatManager(int interval_ms = 5000, int timeout_ms = 15000)
            : interval_ms_(interval_ms), timeout_ms_(timeout_ms) {
            last_recv_ = last_send_ = std::chrono::steady_clock::now();
        }

        void on_data_received() {
            last_recv_ = std::chrono::steady_clock::now();
            missed_count_ = 0;
        }

        bool should_send_heartbeat() const {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - last_send_).count();
            return elapsed >= interval_ms_;
        }

        void mark_heartbeat_sent() {
            last_send_ = std::chrono::steady_clock::now();
        }

        bool is_connection_alive() {
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - last_recv_).count();
            if (elapsed > interval_ms_) {
                missed_count_++;
            }
            return missed_count_ < max_missed_;
        }
    };

    HeartbeatManager hb(100, 300);
    std::cout << "心跳管理器: 间隔100ms, 超时300ms\n";

    for (int i = 0; i < 5; ++i) {
        if (hb.should_send_heartbeat()) {
            std::cout << "  发送心跳 #" << i << "\n";
            hb.mark_heartbeat_sent();
        }
        if (i < 3) {
            hb.on_data_received();
            std::cout << "  收到数据, 连接存活\n";
        } else {
            bool alive = hb.is_connection_alive();
            std::cout << "  未收到数据, 连接" << (alive ? "存活" : "断开") << "\n";
        }
    }

    std::cout << "\n心跳设计要点:\n";
    std::cout << "  1. 心跳间隔: 5-30秒 (根据场景)\n";
    std::cout << "  2. 超时时间: 心跳间隔的3-5倍\n";
    std::cout << "  3. 心跳数据: 尽量小 (甚至0字节载荷)\n";
    std::cout << "  4. 双向心跳: 客户端和服务端都发\n";
    std::cout << "  5. 心跳与业务复用: 有业务数据时不发心跳\n";
}

void demo_reconnection_strategy() {
    std::cout << "\n=== demo_reconnection_strategy ===\n";
    std::cout << "重连策略\n\n";

    std::cout << "重连策略类型:\n\n";

    std::cout << "1. 固定间隔重连:\n";
    std::cout << "   每隔固定时间重试\n";
    std::cout << "   简单但可能加重服务器负担\n\n";

    std::cout << "2. 指数退避重连:\n";
    std::cout << "   间隔逐渐增大: 1s, 2s, 4s, 8s, 16s, ...\n";
    std::cout << "   避免重连风暴\n";
    std::cout << "   最常用的策略\n\n";

    std::cout << "3. 随机抖动:\n";
    std::cout << "   在退避基础上加随机偏移\n";
    std::cout << "   防止多个客户端同时重连\n\n";

    std::cout << "4. 最大重试次数:\n";
    std::cout << "   超过次数后放弃或通知用户\n\n";

    class ReconnectionPolicy {
        int attempt_ = 0;
        int max_attempts_;
        int base_delay_ms_;
        int max_delay_ms_;

    public:
        ReconnectionPolicy(int max_attempts = 10, int base_ms = 1000, int max_ms = 60000)
            : max_attempts_(max_attempts), base_delay_ms_(base_ms), max_delay_ms_(max_ms) {}

        int next_delay() {
            if (attempt_ >= max_attempts_) return -1;
            int delay = base_delay_ms_;
            for (int i = 0; i < attempt_; ++i) {
                delay *= 2;
                if (delay > max_delay_ms_) {
                    delay = max_delay_ms_;
                    break;
                }
            }
            std::mt19937 rng(std::random_device{}());
            int jitter = std::uniform_int_distribution<int>(-delay / 4, delay / 4)(rng);
            delay += jitter;
            delay = std::max(delay, 100);
            ++attempt_;
            return delay;
        }

        void reset() { attempt_ = 0; }
        int attempt_count() const { return attempt_; }
        bool exhausted() const { return attempt_ >= max_attempts_; }
    };

    ReconnectionPolicy policy(8, 1000, 30000);
    std::cout << "指数退避重连策略 (模拟):\n";
    for (int i = 0; i < 8; ++i) {
        int delay = policy.next_delay();
        if (delay < 0) {
            std::cout << "  重试次数耗尽, 放弃重连\n";
            break;
        }
        std::cout << "  第" << (i + 1) << "次重连, 等待~" << delay << "ms\n";
    }

    std::cout << "\n重连最佳实践:\n";
    std::cout << "  1. 使用指数退避 + 随机抖动\n";
    std::cout << "  2. 设置最大重试次数\n";
    std::cout << "  3. 连接成功后重置计数器\n";
    std::cout << "  4. 重连期间保持UI响应\n";
    std::cout << "  5. 记录重连日志\n";
}

void demo_network_security() {
    std::cout << "\n=== demo_network_security ===\n";
    std::cout << "网络安全基础\n\n";

    std::cout << "1. 传输安全:\n";
    std::cout << "   TLS/SSL: 加密传输, 防止窃听和篡改\n";
    std::cout << "   证书验证: 防止中间人攻击\n";
    std::cout << "   OpenSSL: C++最常用的TLS库\n";
    std::cout << "   Boost.Beast: 基于Asio的HTTP/TLS库\n\n";

    std::cout << "2. 输入验证:\n";
    std::cout << "   验证所有网络输入, 不信任客户端\n";
    std::cout << "   限制消息大小, 防止内存耗尽\n";
    std::cout << "   过滤特殊字符, 防止注入攻击\n\n";

    std::cout << "3. 认证与授权:\n";
    std::cout << "   Token认证: JWT, OAuth2\n";
    std::cout << "   会话管理: 超时, 单点登录\n";
    std::cout << "   权限控制: 最小权限原则\n\n";

    std::cout << "4. 防御常见攻击:\n";
    std::cout << "   DDoS: 限流, 连接数限制\n";
    std::cout << "   缓冲区溢出: 使用安全函数, 边界检查\n";
    std::cout << "   SQL注入: 参数化查询\n";
    std::cout << "   中间人攻击: TLS + 证书固定\n\n";

    std::cout << "5. 安全编码实践:\n";
    std::cout << "   使用安全字符串函数 (strncpy代替strcpy)\n";
    std::cout << "   限制连接速率和并发数\n";
    std::cout << "   敏感数据加密存储\n";
    std::cout << "   日志脱敏 (不记录密码/Token)\n";
    std::cout << "   定期更新依赖库 (修复安全漏洞)\n";

    std::cout << "\nC++网络安全库:\n";
    std::cout << "  OpenSSL: TLS/SSL, 加密算法\n";
    std::cout << "  libsodium: 现代加密库, 易用\n";
    std::cout << "  Crypto++: 丰富的加密算法\n";
    std::cout << "  Boost.Beast: 安全HTTP/WebSocket\n";
}

int main() {
    std::cout << "网络编程高级主题\n";

#ifdef _WIN32
    WSADATA wsa_data;
    WSAStartup(MAKEWORD(2, 2), &wsa_data);
#endif

    demo_protocol_design();
    demo_heartbeat_mechanism();
    demo_reconnection_strategy();
    demo_network_security();

#ifdef _WIN32
    WSACleanup();
#endif

    std::cout << "\n所有演示完成!\n";
    return 0;
}
