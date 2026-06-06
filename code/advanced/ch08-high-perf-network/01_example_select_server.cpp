/**
 * @file 01_example_select_server.cpp
 * @brief IO多路复用: select模型服务器, 详解select的用法与局限性
 * @description 对应文档: 高性能网络与异步IO / 第1节 IO多路复用深入
 */

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <set>

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

int get_socket_error() {
#ifdef _WIN32
    return WSAGetLastError();
#else
    return errno;
#endif
}

// ============================================================
// 演示1: select API详解
// ============================================================
void demo_select_concepts() {
    std::cout << "\n========================================\n";
    std::cout << "  演示1: select API详解\n";
    std::cout << "========================================\n\n";

    std::cout << "【select函数原型】\n";
    std::cout << "  int select(int nfds, fd_set *readfds,\n";
    std::cout << "             fd_set *writefds, fd_set *exceptfds,\n";
    std::cout << "             struct timeval *timeout);\n\n";

    std::cout << "【参数说明】\n";
    std::cout << "  nfds      : 监控的fd最大值+1 (Windows下被忽略)\n";
    std::cout << "  readfds   : 监控可读事件的fd集合 (新连接/数据到达)\n";
    std::cout << "  writefds  : 监控可写事件的fd集合 (连接成功/缓冲区可用)\n";
    std::cout << "  exceptfds : 监控异常事件的fd集合 (OOB数据)\n";
    std::cout << "  timeout   : 等待超时, NULL表示无限等待\n\n";

    std::cout << "【fd_set操作宏】\n";
    std::cout << "  FD_ZERO(&set)    : 清空集合\n";
    std::cout << "  FD_SET(fd, &set) : 将fd加入集合\n";
    std::cout << "  FD_CLR(fd, &set) : 将fd从集合移除\n";
    std::cout << "  FD_ISSET(fd, &set): 检查fd是否在集合中\n\n";

    std::cout << "【select的局限性】\n";
    std::cout << "  1. FD_SETSIZE限制: 默认1024(Windows为64), 无法扩展\n";
    std::cout << "     - Linux: 可通过编译时重定义FD_SETSIZE增大\n";
    std::cout << "     - Windows: 硬编码限制, 无法修改\n";
    std::cout << "  2. 每次调用需要重新设置fd_set (select会修改传入的集合)\n";
    std::cout << "  3. 返回后需要遍历所有fd检查就绪状态 O(n)\n";
    std::cout << "  4. 内核需要遍历所有fd检查就绪状态 O(n)\n";
    std::cout << "  5. fd_set在内核态和用户态之间拷贝, 开销大\n\n";

    std::cout << "【select的适用场景】\n";
    std::cout << "  - 连接数较少(< 1024)的场景\n";
    std::cout << "  - 需要跨平台兼容的简单服务器\n";
    std::cout << "  - 同时等待socket和标准输入的程序\n";
    std::cout << "  - Windows上作为IOCP的备选方案\n";
}

// ============================================================
// 演示2: select服务器 + 客户端实际运行
// ============================================================
void demo_select_server() {
    std::cout << "\n========================================\n";
    std::cout << "  演示2: select多路复用Echo服务器\n";
    std::cout << "========================================\n\n";

    const uint16_t port = 15001;

    std::atomic<bool> server_ready{false};
    std::atomic<bool> server_done{false};

    // --- select服务器线程 ---
    std::thread server_thread([&]() {
        // 创建监听socket
        socket_t listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
        if (listen_fd == INVALID_SOCKET_VAL) {
            std::cerr << "[服务器] 创建socket失败\n";
            return;
        }

        int opt = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR,
                   reinterpret_cast<const char*>(&opt), sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = INADDR_ANY;
        addr.sin_port = htons(port);

        if (::bind(listen_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) != 0) {
            std::cerr << "[服务器] bind失败\n";
            CLOSE_SOCKET(listen_fd);
            return;
        }

        if (::listen(listen_fd, 5) != 0) {
            std::cerr << "[服务器] listen失败\n";
            CLOSE_SOCKET(listen_fd);
            return;
        }

        std::cout << "[服务器] select Echo服务器启动, 端口: " << port << "\n";

        // 维护所有活跃的客户端fd
        std::set<socket_t> client_fds;
        client_fds.insert(listen_fd);

        server_ready = true;

        int iteration = 0;
        const int max_iterations = 50;  // 限制迭代次数, 演示用

        while (iteration < max_iterations && !server_done) {
            iteration++;

            // 每次循环都要重新构建fd_set (因为select会修改它)
            fd_set read_fds;
            FD_ZERO(&read_fds);

            // 将所有fd加入read_fds
            socket_t max_fd = 0;
            for (socket_t fd : client_fds) {
                FD_SET(fd, &read_fds);
                if (fd > max_fd) max_fd = fd;
            }

            // 设置超时: 1秒 (避免无限阻塞)
            struct timeval tv{};
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            // 调用select, 只监控可读事件
            int ret = ::select(static_cast<int>(max_fd + 1), &read_fds, nullptr, nullptr, &tv);

            if (ret < 0) {
                std::cerr << "[服务器] select错误, 错误码: " << get_socket_error() << "\n";
                break;
            }

            if (ret == 0) {
                // 超时, 没有就绪事件
                continue;
            }

            // 遍历所有fd, 检查哪些就绪了
            // 注意: 这里必须遍历所有fd, 是select的主要性能瓶颈
            for (auto it = client_fds.begin(); it != client_fds.end(); ) {
                socket_t fd = *it;

                if (!FD_ISSET(fd, &read_fds)) {
                    ++it;
                    continue;
                }

                if (fd == listen_fd) {
                    // 监听socket可读 → 有新连接
                    sockaddr_in client_addr{};
                    socklen_t client_len = sizeof(client_addr);
                    socket_t client_fd = ::accept(listen_fd,
                        reinterpret_cast<sockaddr*>(&client_addr), &client_len);

                    if (client_fd != INVALID_SOCKET_VAL) {
                        char ip[INET_ADDRSTRLEN];
                        inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
                        std::cout << "[服务器] 新连接: " << ip << ":"
                                  << ntohs(client_addr.sin_port)
                                  << " (fd=" << client_fd << ")\n";
                        client_fds.insert(client_fd);
                    }
                    ++it;
                } else {
                    // 客户端socket可读 → 有数据到达或连接断开
                    char buf[1024];
                    ssize_t n = ::recv(fd, buf, sizeof(buf) - 1, 0);

                    if (n <= 0) {
                        // 连接断开
                        std::cout << "[服务器] 客户端断开 (fd=" << fd << ")\n";
                        CLOSE_SOCKET(fd);
                        it = client_fds.erase(it);
                    } else {
                        buf[n] = '\0';
                        std::cout << "[服务器] 收到数据 (fd=" << fd << "): \""
                                  << buf << "\"\n";
                        // Echo回显
                        ::send(fd, buf, static_cast<int>(n), 0);
                        std::cout << "[服务器] 已回显\n";
                        ++it;
                    }
                }
            }
        }

        // 清理所有连接
        for (socket_t fd : client_fds) {
            CLOSE_SOCKET(fd);
        }
        std::cout << "[服务器] 关闭完成\n";
    });

    // 等待服务器就绪
    while (!server_ready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // --- 客户端1 ---
    auto run_client = [](uint16_t port, const std::string& name,
                         const std::vector<std::string>& messages) {
        socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET_VAL) return;

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        if (::connect(sock, reinterpret_cast<sockaddr*>(&server_addr),
                      sizeof(server_addr)) != 0) {
            std::cerr << "[" << name << "] 连接失败\n";
            CLOSE_SOCKET(sock);
            return;
        }
        std::cout << "[" << name << "] 连接成功\n";

        char buf[1024];
        for (const auto& msg : messages) {
            ::send(sock, msg.c_str(), static_cast<int>(msg.size()), 0);
            std::cout << "[" << name << "] 发送: \"" << msg << "\"\n";

            ssize_t n = ::recv(sock, buf, sizeof(buf) - 1, 0);
            if (n > 0) {
                buf[n] = '\0';
                std::cout << "[" << name << "] 收到: \"" << buf << "\"\n";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }

        CLOSE_SOCKET(sock);
        std::cout << "[" << name << "] 关闭连接\n";
    };

    // 启动两个客户端, 演示select同时处理多个连接
    std::thread client1([&]() {
        run_client(port, "客户端1", {"Hello from C1", "C1消息2"});
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::thread client2([&]() {
        run_client(port, "客户端2", {"Hello from C2", "C2消息2"});
    });

    client1.join();
    client2.join();

    // 通知服务器可以退出
    server_done = true;
    server_thread.join();

    std::cout << "\n";
}

// ============================================================
// 演示3: select性能分析
// ============================================================
void demo_select_performance() {
    std::cout << "========================================\n";
    std::cout << "  演示3: select性能分析\n";
    std::cout << "========================================\n\n";

    std::cout << "【select的时间复杂度分析】\n\n";

    std::cout << "  假设有N个fd:\n";
    std::cout << "  - 每次调用select: 内核遍历N个fd → O(N)\n";
    std::cout << "  - select返回后: 用户遍历N个fd检查就绪 → O(N)\n";
    std::cout << "  - fd_set拷贝: 用户态→内核态→用户态 → O(N)\n\n";

    std::cout << "  总开销 = O(N) × 3, 与连接数线性增长\n\n";

    std::cout << "【select vs poll vs epoll 对比】\n\n";

    std::cout << "  特性              select       poll         epoll\n";
    std::cout << "  ─────────────────────────────────────────────────\n";
    std::cout << "  最大fd数          1024         无限制        无限制\n";
    std::cout << "  数据结构          bitmap       结构体数组    红黑树+就绪链表\n";
    std::cout << "  内核遍历          O(N)         O(N)         O(1)返回就绪\n";
    std::cout << "  用户遍历          O(N)         O(N)         O(就绪数)\n";
    std::cout << "  数据拷贝          每次全量      每次全量      首次注册,增量\n";
    std::cout << "  触发模式          LT           LT           LT+ET\n";
    std::cout << "  跨平台            是           是           否(Linux)\n\n";

    std::cout << "【结论】\n";
    std::cout << "  - 少量连接(<100): select/poll足够\n";
    std::cout << "  - 中等连接(100~1000): poll优于select\n";
    std::cout << "  - 大量连接(>1000): 必须用epoll(Linux)或IOCP(Windows)\n";
}

// ============================================================
// 主函数
// ============================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  第8章: 高性能网络与异步IO - 第1节 select模型服务器\n";
    std::cout << "============================================================\n";

    WinsockInit winsock_init;

    demo_select_concepts();
    demo_select_server();
    demo_select_performance();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
