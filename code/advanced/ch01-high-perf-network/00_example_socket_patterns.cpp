/**
 * @file 00_example_socket_patterns.cpp
 * @brief 网络编程进阶概述: Socket模式回顾, 阻塞与非阻塞, 简单Echo服务器/客户端
 * @description 对应文档: 高性能网络与异步IO / 第0节 网络编程进阶概述
 */

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>

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
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <fcntl.h>
    #include <netdb.h>
    #include <errno.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VAL (-1)
    #define CLOSE_SOCKET close
#endif

// ============================================================
// Winsock初始化/清理的RAII包装器
// ============================================================
class WinsockInit {
#ifdef _WIN32
    WSADATA wsa_data_;
#endif
public:
    WinsockInit() {
#ifdef _WIN32
        if (WSAStartup(MAKEWORD(2, 2), &wsa_data_) != 0) {
            std::cerr << "[错误] WSAStartup 初始化失败\n";
            std::exit(1);
        }
        std::cout << "[信息] Winsock 初始化成功 (版本: "
                  << LOBYTE(wsa_data_.wVersion) << "."
                  << HIBYTE(wsa_data_.wVersion) << ")\n";
#endif
    }
    ~WinsockInit() {
#ifdef _WIN32
        WSACleanup();
        std::cout << "[信息] Winsock 清理完成\n";
#endif
    }
    WinsockInit(const WinsockInit&) = delete;
    WinsockInit& operator=(const WinsockInit&) = delete;
};

// ============================================================
// 跨平台Socket错误码获取
// ============================================================
int get_socket_error() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

// ============================================================
// 设置socket为非阻塞模式
// ============================================================
bool set_nonblocking(socket_t fd) {
#ifdef _WIN32
    u_long mode = 1;
    return ioctlsocket(fd, FIONBIO, &mode) == 0;
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK) != -1;
#endif
}

// ============================================================
// 演示1: 阻塞模式 vs 非阻塞模式
// ============================================================
void demo_blocking_vs_nonblocking() {
    std::cout << "\n========================================\n";
    std::cout << "  演示1: 阻塞模式 vs 非阻塞模式\n";
    std::cout << "========================================\n\n";

    std::cout << "【阻塞模式 (Blocking)】\n";
    std::cout << "  - 默认的socket模式\n";
    std::cout << "  - recv()/send() 会阻塞当前线程直到操作完成\n";
    std::cout << "  - 优点: 编程简单, 逻辑直观\n";
    std::cout << "  - 缺点: 一个线程只能处理一个连接\n";
    std::cout << "  - 适用: 简单客户端、低并发场景\n\n";

    std::cout << "【非阻塞模式 (Non-blocking)】\n";
    std::cout << "  - recv()/send() 立即返回, 若无数据返回 EAGAIN/EWOULDBLOCK\n";
    std::cout << "  - 优点: 单线程可管理多个连接\n";
    std::cout << "  - 缺点: 需要配合IO多路复用(select/poll/epoll)使用\n";
    std::cout << "  - 适用: 高并发服务器\n\n";

    // 实际演示: 创建socket并切换非阻塞模式
    socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET_VAL) {
        std::cerr << "[错误] 创建socket失败\n";
        return;
    }

    // 默认是阻塞模式
    std::cout << "新创建的socket默认为阻塞模式\n";

    // 切换为非阻塞
    if (set_nonblocking(sock)) {
        std::cout << "成功切换为非阻塞模式\n";
    } else {
        std::cerr << "[错误] 切换非阻塞模式失败, 错误码: " << get_socket_error() << "\n";
    }

    CLOSE_SOCKET(sock);
    std::cout << "\n";
}

// ============================================================
// 演示2: 阻塞式Echo服务器 + 客户端 (同进程演示)
// ============================================================
void demo_blocking_echo() {
    std::cout << "========================================\n";
    std::cout << "  演示2: 阻塞式Echo服务器/客户端\n";
    std::cout << "========================================\n\n";

    const uint16_t port = 15000;

    // --- 服务器线程 ---
    std::atomic<bool> server_ready{false};
    std::atomic<bool> server_done{false};

    std::thread server_thread([&]() {
        // 创建监听socket
        socket_t listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd == INVALID_SOCKET_VAL) {
            std::cerr << "[服务器] 创建socket失败\n";
            return;
        }

        // 设置SO_REUSEADDR, 避免TIME_WAIT导致地址占用
        int opt = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&opt), sizeof(opt));

        // 绑定地址
        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (::bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
            std::cerr << "[服务器] bind失败, 错误码: " << get_socket_error() << "\n";
            CLOSE_SOCKET(listen_fd);
            return;
        }

        // 开始监听
        if (::listen(listen_fd, 5) != 0) {
            std::cerr << "[服务器] listen失败\n";
            CLOSE_SOCKET(listen_fd);
            return;
        }

        std::cout << "[服务器] 阻塞式Echo服务器启动, 监听端口: " << port << "\n";
        server_ready = true;

        // 阻塞等待客户端连接
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        socket_t client_fd = ::accept(listen_fd,
            reinterpret_cast<sockaddr*>(&client_addr), &client_len);

        if (client_fd == INVALID_SOCKET_VAL) {
            std::cerr << "[服务器] accept失败\n";
            CLOSE_SOCKET(listen_fd);
            return;
        }

        char ip[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
        std::cout << "[服务器] 接受连接: " << ip << ":"
                  << ntohs(client_addr.sin_port) << "\n";

        // 阻塞式Echo循环: 收到数据后原样回发
        char buf[1024];
        while (true) {
            // recv会阻塞, 直到收到数据或连接断开
            ssize_t n = ::recv(client_fd, buf, sizeof(buf) - 1, 0);
            if (n <= 0) {
                std::cout << "[服务器] 客户端断开连接 (recv返回: " << n << ")\n";
                break;
            }
            buf[n] = '\0';
            std::cout << "[服务器] 收到: \"" << buf << "\" (" << n << " 字节)\n";

            // 原样回显
            ssize_t sent = ::send(client_fd, buf, static_cast<int>(n), 0);
            if (sent <= 0) {
                std::cerr << "[服务器] 发送失败\n";
                break;
            }
            std::cout << "[服务器] 回显: \"" << buf << "\" (" << sent << " 字节)\n";
        }

        CLOSE_SOCKET(client_fd);
        CLOSE_SOCKET(listen_fd);
        server_done = true;
        std::cout << "[服务器] 关闭完成\n";
    });

    // 等待服务器就绪
    while (!server_ready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // --- 客户端 ---
    {
        socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET_VAL) {
            std::cerr << "[客户端] 创建socket失败\n";
            server_thread.join();
            return;
        }

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        // 阻塞式connect
        if (::connect(sock, reinterpret_cast<sockaddr*>(&server_addr),
                      sizeof(server_addr)) != 0) {
            std::cerr << "[客户端] 连接失败, 错误码: " << get_socket_error() << "\n";
            CLOSE_SOCKET(sock);
            server_thread.join();
            return;
        }
        std::cout << "[客户端] 连接成功\n\n";

        // 发送几条消息并接收回显
        std::vector<std::string> messages = {
            "Hello, Echo Server!",
            "阻塞模式演示",
            "Goodbye!"
        };

        char buf[1024];
        for (const auto& msg : messages) {
            ::send(sock, msg.c_str(), static_cast<int>(msg.size()), 0);
            std::cout << "[客户端] 发送: \"" << msg << "\"\n";

            ssize_t n = ::recv(sock, buf, sizeof(buf) - 1, 0);
            if (n > 0) {
                buf[n] = '\0';
                std::cout << "[客户端] 收到回显: \"" << buf << "\"\n\n";
            }
        }

        CLOSE_SOCKET(sock);
        std::cout << "[客户端] 关闭连接\n";
    }

    server_thread.join();
    std::cout << "\n";
}

// ============================================================
// 演示3: 非阻塞式连接尝试
// ============================================================
void demo_nonblocking_connect() {
    std::cout << "========================================\n";
    std::cout << "  演示3: 非阻塞式连接尝试\n";
    std::cout << "========================================\n\n";

    std::cout << "非阻塞connect的工作原理:\n";
    std::cout << "  1. 将socket设为非阻塞\n";
    std::cout << "  2. 调用connect(), 通常返回EINPROGRESS(正在连接)\n";
    std::cout << "  3. 使用select()等待socket变为可写(连接完成)\n";
    std::cout << "  4. 检查SO_ERROR确认连接是否成功\n\n";

    socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET_VAL) {
        std::cerr << "[错误] 创建socket失败\n";
        return;
    }

    // 设置非阻塞
    if (!set_nonblocking(sock)) {
        std::cerr << "[错误] 设置非阻塞失败\n";
        CLOSE_SOCKET(sock);
        return;
    }

    // 尝试连接到一个可能不存在的地址, 演示非阻塞行为
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port = htons(19999);  // 不太可能开放的端口
    inet_pton(AF_INET, "127.0.0.1", &addr.sin_addr);

    std::cout << "尝试非阻塞连接到 127.0.0.1:19999...\n";
    int ret = ::connect(sock, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));

    if (ret != 0) {
        int err = get_socket_error();
#ifdef _WIN32
        bool in_progress = (err == WSAEWOULDBLOCK);
#else
        bool in_progress = (err == EINPROGRESS);
#endif
        if (in_progress) {
            std::cout << "connect()返回EINPROGRESS/EWOULDBLOCK, 连接正在后台进行\n";

            // 使用select等待连接完成, 超时1秒
            fd_set write_fds;
            FD_ZERO(&write_fds);
            FD_SET(sock, &write_fds);

            struct timeval tv{};
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            int sel_ret = ::select(static_cast<int>(sock) + 1, nullptr, &write_fds, nullptr, &tv);

            if (sel_ret > 0) {
                // 检查连接结果
                int sock_err = 0;
                socklen_t err_len = sizeof(sock_err);
                getsockopt(sock, SOL_SOCKET, SO_ERROR,
                          reinterpret_cast<char*>(&sock_err), &err_len);
                if (sock_err == 0) {
                    std::cout << "连接成功!\n";
                } else {
                    std::cout << "连接失败, SO_ERROR: " << sock_err << "\n";
                    std::cout << "(连接被拒绝是正常的, 因为目标端口未开放)\n";
                }
            } else if (sel_ret == 0) {
                std::cout << "连接超时 (1秒内未完成)\n";
            } else {
                std::cout << "select错误\n";
            }
        } else {
            std::cout << "connect立即失败, 错误码: " << err << "\n";
        }
    } else {
        std::cout << "连接立即成功!\n";
    }

    CLOSE_SOCKET(sock);
    std::cout << "\n";
}

// ============================================================
// 演示4: Socket模式总结
// ============================================================
void demo_socket_patterns_summary() {
    std::cout << "========================================\n";
    std::cout << "  演示4: Socket模式总结与进阶路线\n";
    std::cout << "========================================\n\n";

    std::cout << "【网络编程模式演进】\n\n";

    std::cout << "1. 阻塞式 (本节回顾)\n";
    std::cout << "   一个连接一个线程, 编程最简单\n";
    std::cout << "   缺点: 线程开销大, C10K问题无法解决\n\n";

    std::cout << "2. 非阻塞 + IO多路复用 (下一节)\n";
    std::cout << "   select/poll/epoll 监控多个fd\n";
    std::cout << "   单线程处理多连接, 性能大幅提升\n\n";

    std::cout << "3. Reactor模式 (第2节)\n";
    std::cout << "   事件驱动 + 回调, 经典网络库架构\n";
    std::cout << "   muduo/libevent/Redis等采用此模式\n\n";

    std::cout << "4. Proactor模式 (第3节)\n";
    std::cout << "   真正的异步IO, Windows IOCP为代表\n";
    std::cout << "   Boost.Asio在Windows上使用此模式\n\n";

    std::cout << "5. 协程 (C++20)\n";
    std::cout << "   以同步风格写异步代码\n";
    std::cout << "   C++20协程 + io_uring/IOCP 是未来方向\n\n";

    std::cout << "【性能关键指标】\n";
    std::cout << "   - 并发连接数 (C10K → C10M)\n";
    std::cout << "   - 吞吐量 (QPS/TPS)\n";
    std::cout << "   - 延迟 (P50/P99/P999)\n";
    std::cout << "   - CPU利用率\n";
    std::cout << "   - 内存占用\n";
}

// ============================================================
// 主函数
// ============================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  第8章: 高性能网络与异步IO - 第0节 网络编程进阶概述\n";
    std::cout << "============================================================\n";

    // Winsock初始化 (Windows必须)
    WinsockInit winsock_init;

    demo_blocking_vs_nonblocking();
    demo_blocking_echo();
    demo_nonblocking_connect();
    demo_socket_patterns_summary();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
