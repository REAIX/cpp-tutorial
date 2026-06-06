/**
 * @file 07_example_tcp_tuning.cpp
 * @brief 网络性能调优: TCP选项, Socket缓冲区, 性能优化技巧
 * @description 对应文档: 高性能网络与异步IO / 第7节 网络性能调优
 */

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>

#ifdef _WIN32
    #ifndef _WINSOCK2API_
        #include <winsock2.h>
        #include <ws2tcpip.h>
    #endif
    #ifdef _MSC_VER
        #pragma comment(lib, "ws2_32.lib")
    #endif
    typedef int socklen_t;
    typedef SOCKET socket_t;
    #ifndef __MINGW32__
        typedef int ssize_t;
    #endif
    #define INVALID_SOCKET_VAL INVALID_SOCKET
    #define CLOSE_SOCKET closesocket
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <netinet/tcp.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <errno.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VAL (-1)
    #define CLOSE_SOCKET close
#endif

// ============================================================
// Winsock RAII
// ============================================================
class WinsockInit {
#ifdef _WIN32
    WSADATA wsa_data_;
#endif
public:
    WinsockInit() {
#ifdef _WIN32
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data_) != 0) {
            std::cerr << "[错误] WSAStartup失败\n";
            std::exit(1);
        }
#endif
    }
    ~WinsockInit() {
#ifdef _WIN32
        WSACleanup();
#endif
    }
    WinsockInit(const WinsockInit&) = delete;
    WinsockInit& operator=(const WinsockInit&) = delete;
};

// ============================================================
// 辅助: 获取并打印socket选项
// ============================================================
void print_socket_option(socket_t fd, int level, int optname, const char* name) {
    int val = 0;
    socklen_t len = sizeof(val);
    if (getsockopt(fd, level, optname,
                   reinterpret_cast<char*>(&val), &len) == 0) {
        std::cout << "  " << name << " = " << val << "\n";
    } else {
        std::cout << "  " << name << " = <获取失败>\n";
    }
}

// ============================================================
// 演示1: TCP_NODELAY - 禁用Nagle算法
// ============================================================
void demo_tcp_nodelay() {
    std::cout << "\n========================================\n";
    std::cout << "  演示1: TCP_NODELAY - 禁用Nagle算法\n";
    std::cout << "========================================\n\n";

    std::cout << "【Nagle算法】\n\n";

    std::cout << "  Nagle算法的目的是减少小包数量:\n";
    std::cout << "  - 如果有未确认的数据, 缓存小包\n";
    std::cout << "  - 等到收到ACK或积累足够数据再发送\n\n";

    std::cout << "  问题: 增加延迟!\n";
    std::cout << "  场景: 发送1字节命令 → 等待200ms(RTT) → 才发送\n\n";

    std::cout << "  write(data1)  ──→  Nagle缓存\n";
    std::cout << "       ...等待ACK... (最多200ms延迟)\n";
    std::cout << "  write(data2)  ──→  合并发送\n\n";

    std::cout << "【TCP_NODELAY选项】\n\n";

    std::cout << "  setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &opt, sizeof(opt))\n\n";

    std::cout << "  启用TCP_NODELAY后:\n";
    std::cout << "  - 每次write立即发送, 不等待\n";
    std::cout << "  - 延迟降低, 但小包数量增加\n\n";

    std::cout << "【何时启用TCP_NODELAY?】\n\n";

    std::cout << "  ✅ 必须启用:\n";
    std::cout << "     - 实时游戏 (延迟敏感)\n";
    std::cout << "     - SSH/Telnet (交互式)\n";
    std::cout << "     - RPC (请求-响应模式)\n";
    std::cout << "     - WebSocket (消息协议)\n\n";

    std::cout << "  ❌ 不需要启用:\n";
    std::cout << "     - 文件传输 (带宽优先)\n";
    std::cout << "     - 日志传输 (延迟不敏感)\n";
    std::cout << "     - 视频流 (批量发送)\n\n";

    // 实际操作
    socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock != INVALID_SOCKET_VAL) {
        // 查看默认值
        std::cout << "  默认TCP_NODELAY: ";
        int val = 0;
        socklen_t len = sizeof(val);
        getsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<char*>(&val), &len);
        std::cout << val << " (0=禁用, Nagle算法生效)\n";

        // 启用TCP_NODELAY
        int enable = 1;
        setsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<const char*>(&enable), sizeof(enable));
        std::cout << "  设置TCP_NODELAY=1后: ";
        getsockopt(sock, IPPROTO_TCP, TCP_NODELAY,
                   reinterpret_cast<char*>(&val), &len);
        std::cout << val << " (1=启用, Nagle算法禁用)\n";

        CLOSE_SOCKET(sock);
    }
}

// ============================================================
// 演示2: SO_REUSEADDR - 地址重用
// ============================================================
void demo_so_reuseaddr() {
    std::cout << "\n========================================\n";
    std::cout << "  演示2: SO_REUSEADDR - 地址重用\n";
    std::cout << "========================================\n\n";

    std::cout << "【TIME_WAIT问题】\n\n";

    std::cout << "  服务器主动关闭连接后, 端口进入TIME_WAIT状态:\n";
    std::cout << "  - 持续2MSL (通常60秒)\n";
    std::cout << "  - 期间无法bind同一端口\n";
    std::cout << "  - 服务器重启失败: 'Address already in use'\n\n";

    std::cout << "  客户端    服务器\n";
    std::cout << "    │         │\n";
    std::cout << "    │◄──FIN───│  服务器主动关闭\n";
    std::cout << "    │───ACK──►│\n";
    std::cout << "    │───FIN──►│  客户端关闭\n";
    std::cout << "    │◄──ACK───│\n";
    std::cout << "    │  TIME_WAIT (2MSL)\n\n";

    std::cout << "【SO_REUSEADDR选项】\n\n";

    std::cout << "  setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))\n\n";

    std::cout << "  效果: 允许bind处于TIME_WAIT状态的地址\n";
    std::cout << "  注意: 必须在bind之前设置!\n\n";

    std::cout << "  正确顺序:\n";
    std::cout << "    1. socket()\n";
    std::cout << "    2. setsockopt(SO_REUSEADDR)  ← bind之前\n";
    std::cout << "    3. bind()\n";
    std::cout << "    4. listen()\n\n";

    // 实际操作
    socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock != INVALID_SOCKET_VAL) {
        int enable = 1;
        int ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR,
                            reinterpret_cast<const char*>(&enable), sizeof(enable));
        std::cout << "  设置SO_REUSEADDR: " << (ret == 0 ? "成功" : "失败") << "\n";

        print_socket_option(sock, SOL_SOCKET, SO_REUSEADDR, "SO_REUSEADDR");
        CLOSE_SOCKET(sock);
    }

    std::cout << "\n【SO_REUSEPORT (Linux 3.9+)】\n";
    std::cout << "  允许多个socket绑定同一端口\n";
    std::cout << "  内核自动负载均衡\n";
    std::cout << "  用于多进程服务器 (Nginx worker)\n";
}

// ============================================================
// 演示3: SO_KEEPALIVE - 保持连接
// ============================================================
void demo_so_keepalive() {
    std::cout << "\n========================================\n";
    std::cout << "  演示3: SO_KEEPALIVE - 保持连接\n";
    std::cout << "========================================\n\n";

    std::cout << "【TCP Keep-Alive机制】\n\n";

    std::cout << "  作用: 检测死连接 (对端崩溃, 未发FIN)\n\n";

    std::cout << "  工作方式:\n";
    std::cout << "  1. 空闲超过tcp_keepalive_time (默认7200秒/2小时)\n";
    std::cout << "  2. 发送探测包\n";
    std::cout << "  3. 无响应则重试tcp_keepalive_intvl (默认75秒)\n";
    std::cout << "  4. 超过tcp_keepalive_probes (默认9次) 则断开\n\n";

    std::cout << "  默认参数太保守 (2小时才检测!), 需要调整:\n\n";

    socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock != INVALID_SOCKET_VAL) {
        // 启用SO_KEEPALIVE
        int enable = 1;
        setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE,
                   reinterpret_cast<const char*>(&enable), sizeof(enable));
        std::cout << "  已设置SO_KEEPALIVE=1\n";

#ifdef _WIN32
        // Windows: 使用WSAIoctl设置Keep-Alive参数
        tcp_keepalive keepalive{};
        keepalive.onoff = 1;
        keepalive.keepalivetime = 30000;   // 30秒后开始探测
        keepalive.keepaliveinterval = 5000; // 每5秒探测一次

        DWORD bytes_returned = 0;
        int ret = WSAIoctl(sock, SIO_KEEPALIVE_VALS, &keepalive, sizeof(keepalive),
                          nullptr, 0, &bytes_returned, nullptr, nullptr);
        std::cout << "  Windows Keep-Alive参数设置: "
                  << (ret == 0 ? "成功" : "失败") << "\n";
        std::cout << "    探测间隔: " << keepalive.keepalivetime / 1000 << " 秒\n";
        std::cout << "    探测频率: " << keepalive.keepaliveinterval / 1000 << " 秒\n";
#else
        // Linux: 使用setsockopt
        int idle = 30;    // 30秒后开始探测
        int interval = 5; // 每5秒探测一次
        int count = 3;    // 探测3次无响应则断开

        setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(idle));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(interval));
        setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &count, sizeof(count));

        std::cout << "  Linux Keep-Alive参数:\n";
        std::cout << "    TCP_KEEPIDLE: " << idle << " 秒\n";
        std::cout << "    TCP_KEEPINTVL: " << interval << " 秒\n";
        std::cout << "    TCP_KEEPCNT: " << count << " 次\n";
#endif

        CLOSE_SOCKET(sock);
    }

    std::cout << "\n【应用层心跳 vs TCP Keep-Alive】\n\n";

    std::cout << "  TCP Keep-Alive:\n";
    std::cout << "    - 优点: 操作系统自动处理\n";
    std::cout << "    - 缺点: 参数不灵活, 不能传业务数据\n\n";

    std::cout << "  应用层心跳:\n";
    std::cout << "    - 优点: 灵活, 可携带业务信息\n";
    std::cout << "    - 缺点: 需要自己实现\n";
    std::cout << "    - 推荐: 同时使用两者\n";
}

// ============================================================
// 演示4: Socket缓冲区大小
// ============================================================
void demo_socket_buffers() {
    std::cout << "\n========================================\n";
    std::cout << "  演示4: Socket缓冲区大小\n";
    std::cout << "========================================\n\n";

    std::cout << "【缓冲区对性能的影响】\n\n";

    std::cout << "  SO_RCVBUF: 接收缓冲区大小\n";
    std::cout << "  SO_SNDBUF: 发送缓冲区大小\n\n";

    std::cout << "  缓冲区太小:\n";
    std::cout << "    - 频繁的通知和上下文切换\n";
    std::cout << "    - 发送方可能被阻塞\n";
    std::cout << "    - 吞吐量下降\n\n";

    std::cout << "  缓冲区太大:\n";
    std::cout << "    - 内存占用增加\n";
    std::cout << "    - 延迟可能增加 (数据排队)\n\n";

    socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock != INVALID_SOCKET_VAL) {
        // 查看默认缓冲区大小
        std::cout << "【默认缓冲区大小】\n";
        print_socket_option(sock, SOL_SOCKET, SO_RCVBUF, "SO_RCVBUF (接收)");
        print_socket_option(sock, SOL_SOCKET, SO_SNDBUF, "SO_SNDBUF (发送)");

        // 设置更大的缓冲区
        int bufsize = 256 * 1024;  // 256KB
        setsockopt(sock, SOL_SOCKET, SO_RCVBUF,
                   reinterpret_cast<const char*>(&bufsize), sizeof(bufsize));
        setsockopt(sock, SOL_SOCKET, SO_SNDBUF,
                   reinterpret_cast<const char*>(&bufsize), sizeof(bufsize));

        std::cout << "\n【设置后缓冲区大小 (请求256KB)】\n";
        print_socket_option(sock, SOL_SOCKET, SO_RCVBUF, "SO_RCVBUF (接收)");
        print_socket_option(sock, SOL_SOCKET, SO_SNDBUF, "SO_SNDBUF (发送)");

        std::cout << "\n  注意: 实际值可能是请求值的2倍 (内核会翻倍)\n";

        CLOSE_SOCKET(sock);
    }

    std::cout << "\n【带宽延迟积 (BDP)】\n\n";

    std::cout << "  BDP = 带宽 × RTT\n";
    std::cout << "  缓冲区大小应 ≥ BDP, 才能充分利用带宽\n\n";

    std::cout << "  示例:\n";
    std::cout << "    1Gbps网络, RTT=1ms:  BDP = 125KB → 缓冲区128KB足够\n";
    std::cout << "    1Gbps网络, RTT=50ms: BDP = 6.25MB → 缓冲区需要8MB\n";
    std::cout << "    10Gbps网络, RTT=50ms: BDP = 62.5MB → 缓冲区需要64MB\n\n";

    std::cout << "  长肥网络 (高带宽+高延迟): 需要特别大的缓冲区\n";
}

// ============================================================
// 演示5: 其他重要TCP选项
// ============================================================
void demo_other_tcp_options() {
    std::cout << "\n========================================\n";
    std::cout << "  演示5: 其他重要TCP选项与调优总结\n";
    std::cout << "========================================\n\n";

    std::cout << "【SO_LINGER - 关闭行为控制】\n\n";

    std::cout << "  struct linger {\n";
    std::cout << "      int l_onoff;   // 0=禁用, 1=启用\n";
    std::cout << "      int l_linger;  // 超时秒数\n";
    std::cout << "  };\n\n";

    std::cout << "  l_onoff=0: 默认行为, closesocket立即返回\n";
    std::cout << "    发送缓冲区数据在后台发送\n\n";

    std::cout << "  l_onoff=1, l_linger=0: RST关闭\n";
    std::cout << "    丢弃缓冲区数据, 发送RST\n";
    std::cout << "    对端收到Connection Reset\n";
    std::cout << "    不进入TIME_WAIT!\n\n";

    std::cout << "  l_onoff=1, l_linger>0: 等待关闭\n";
    std::cout << "    等待数据发送完或超时\n";
    std::cout << "    超时则丢弃数据\n\n";

    std::cout << "【TCP_QUICKACK (Linux)】\n";
    std::cout << "  禁用延迟ACK, 立即发送确认\n";
    std::cout << "  减少ACK延迟 (通常40ms)\n\n";

    std::cout << "【TCP_DEFER_ACCEPT (Linux)】\n";
    std::cout << "  延迟accept, 直到有数据到达\n";
    std::cout << "  避免空连接的accept\n\n";

    std::cout << "【性能调优总结】\n\n";

    std::cout << "  ┌─────────────────┬──────────────────────────────────┐\n";
    std::cout << "  │ 选项             │ 建议值/操作                       │\n";
    std::cout << "  ├─────────────────┼──────────────────────────────────┤\n";
    std::cout << "  │ TCP_NODELAY     │ 实时应用必须启用                   │\n";
    std::cout << "  │ SO_REUSEADDR    │ 服务器必须设置                     │\n";
    std::cout << "  │ SO_KEEPALIVE    │ 长连接必须启用                     │\n";
    std::cout << "  │ SO_RCVBUF       │ 根据BDP调整, 通常≥64KB            │\n";
    std::cout << "  │ SO_SNDBUF       │ 根据BDP调整, 通常≥64KB            │\n";
    std::cout << "  │ SO_LINGER       │ 特殊场景使用, 默认即可             │\n";
    std::cout << "  │ IO多路复用      │ Linux用epoll, Windows用IOCP       │\n";
    std::cout << "  │ 批量发送        │ 合并小包, 减少系统调用             │\n";
    std::cout << "  └─────────────────┴──────────────────────────────────┘\n";
}

// ============================================================
// 主函数
// ============================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  第8章: 高性能网络与异步IO - 第7节 网络性能调优\n";
    std::cout << "============================================================\n";

    WinsockInit winsock_init;

    demo_tcp_nodelay();
    demo_so_reuseaddr();
    demo_so_keepalive();
    demo_socket_buffers();
    demo_other_tcp_options();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
