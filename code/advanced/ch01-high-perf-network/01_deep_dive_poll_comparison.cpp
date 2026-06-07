/**
 * @file 01_deep_dive_poll_comparison.cpp
 * @brief IO多路复用深入: poll模型服务器, 与select的详细对比分析
 * @description 对应文档: 高性能网络与异步IO / 第1节 IO多路复用深入
 */

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>

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
    // Windows使用WSAPoll (Vista+), 接口与poll兼容
    // WSAPOLLFD等同于POSIX的pollfd结构
    using pollfd = WSAPOLLFD;
    #define poll WSAPoll
#else
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <poll.h>
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
// 演示1: poll API详解与select对比
// ============================================================
void demo_poll_vs_select() {
    std::cout << "\n========================================\n";
    std::cout << "  演示1: poll API详解与select对比\n";
    std::cout << "========================================\n\n";

    std::cout << "【poll函数原型】\n";
    std::cout << "  int poll(struct pollfd *fds, nfds_t nfds, int timeout);\n\n";

    std::cout << "【pollfd结构体】\n";
    std::cout << "  struct pollfd {\n";
    std::cout << "      int   fd;       // 要监控的文件描述符\n";
    std::cout << "      short events;   // 感兴趣的事件 (输入参数)\n";
    std::cout << "      short revents;  // 实际发生的事件 (输出参数)\n";
    std::cout << "  };\n\n";

    std::cout << "【常用事件标志】\n";
    std::cout << "  POLLIN     : 有数据可读\n";
    std::cout << "  POLLPRI    : 有紧急数据可读\n";
    std::cout << "  POLLOUT    : 写操作不会阻塞\n";
    std::cout << "  POLLERR    : 错误条件 (仅revents)\n";
    std::cout << "  POLLHUP    : 挂断 (仅revents)\n";
    std::cout << "  POLLNVAL   : 无效请求, fd未打开 (仅revents)\n\n";

    std::cout << "【poll相比select的优势】\n\n";

    std::cout << "  1. 无fd数量限制\n";
    std::cout << "     select: FD_SETSIZE硬限制(通常1024)\n";
    std::cout << "     poll:   用动态数组, 理论上无上限\n\n";

    std::cout << "  2. 输入/输出参数分离\n";
    std::cout << "     select: 每次调用后fd_set被修改, 必须重建\n";
    std::cout << "     poll:   events(输入)和revents(输出)分离, 无需重建\n\n";

    std::cout << "  3. 更精确的事件通知\n";
    std::cout << "     select: 只区分可读/可写/异常\n";
    std::cout << "     poll:   区分POLLIN/POLLPRI/POLLOUT/POLLERR/POLLHUP\n\n";

    std::cout << "  4. 更好的大fd支持\n";
    std::cout << "     select: nfds参数导致遍历0~max_fd\n";
    std::cout << "     poll:   只遍历实际注册的fd\n\n";

    std::cout << "【poll与select的共同局限】\n";
    std::cout << "  1. 每次调用都需要将fd集合从用户态拷贝到内核态\n";
    std::cout << "  2. 返回后需要遍历所有fd检查revents O(N)\n";
    std::cout << "  3. 内核也需要遍历所有fd检查就绪状态 O(N)\n";
    std::cout << "  4. 活跃连接比例低时, 大量无效遍历\n";
}

// ============================================================
// 演示2: poll服务器实际运行
// ============================================================
void demo_poll_server() {
    std::cout << "\n========================================\n";
    std::cout << "  演示2: poll多路复用Echo服务器\n";
    std::cout << "========================================\n\n";

    const uint16_t port = 15002;

    std::atomic<bool> server_ready{false};
    std::atomic<bool> server_done{false};

    // --- poll服务器线程 ---
    std::thread server_thread([&]() {
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

        std::cout << "[服务器] poll Echo服务器启动, 端口: " << port << "\n";

        // pollfd数组: 第一个元素始终是监听socket
        std::vector<pollfd> poll_fds;
        poll_fds.push_back({listen_fd, POLLIN, 0});

        server_ready = true;

        int iteration = 0;
        const int max_iterations = 50;

        while (iteration < max_iterations && !server_done) {
            iteration++;

            // 调用poll, 超时1秒
            // 注意: poll的events字段不需要每次重设 (与select不同)
            int ret = ::poll(poll_fds.data(),
                            static_cast<unsigned long>(poll_fds.size()), 1000);

            if (ret < 0) {
                std::cerr << "[服务器] poll错误, 错误码: " << get_socket_error() << "\n";
                break;
            }

            if (ret == 0) {
                continue;  // 超时
            }

            // 遍历检查就绪的fd
            // 注意: 仍然需要遍历所有fd, 但只需检查revents
            for (size_t i = 0; i < poll_fds.size(); ) {
                auto& pfd = poll_fds[i];

                if (pfd.revents == 0) {
                    ++i;
                    continue;
                }

                // 检查错误事件
                if (pfd.revents & (POLLERR | POLLHUP | POLLNVAL)) {
                    std::cout << "[服务器] fd=" << pfd.fd
                              << " 发生错误事件: " << pfd.revents << "\n";
                    CLOSE_SOCKET(pfd.fd);
                    // 移除该fd (与最后一个交换, 避免移动开销)
                    if (i != poll_fds.size() - 1) {
                        poll_fds[i] = poll_fds.back();
                    }
                    poll_fds.pop_back();
                    continue;
                }

                if (pfd.fd == listen_fd) {
                    // 监听socket有新连接
                    if (pfd.revents & POLLIN) {
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

                            // 将新客户端加入poll监控
                            poll_fds.push_back({client_fd, POLLIN, 0});
                        }
                    }
                    ++i;
                } else {
                    // 客户端socket有数据
                    if (pfd.revents & POLLIN) {
                        char buf[1024];
                        ssize_t n = ::recv(pfd.fd, buf, sizeof(buf) - 1, 0);

                        if (n <= 0) {
                            std::cout << "[服务器] 客户端断开 (fd=" << pfd.fd << ")\n";
                            CLOSE_SOCKET(pfd.fd);
                            if (i != poll_fds.size() - 1) {
                                poll_fds[i] = poll_fds.back();
                            }
                            poll_fds.pop_back();
                        } else {
                            buf[n] = '\0';
                            std::cout << "[服务器] 收到 (fd=" << pfd.fd << "): \""
                                      << buf << "\"\n";
                            ::send(pfd.fd, buf, static_cast<int>(n), 0);
                            std::cout << "[服务器] 已回显\n";
                            ++i;
                        }
                    } else {
                        ++i;
                    }
                }
            }
        }

        // 清理
        for (auto& pfd : poll_fds) {
            CLOSE_SOCKET(pfd.fd);
        }
        std::cout << "[服务器] 关闭完成\n";
    });

    // 等待服务器就绪
    while (!server_ready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // --- 客户端 ---
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

    std::thread client1([&]() {
        run_client(port, "客户端A", {"poll测试消息1", "poll测试消息2"});
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    std::thread client2([&]() {
        run_client(port, "客户端B", {"poll测试消息3", "poll测试消息4"});
    });

    client1.join();
    client2.join();

    server_done = true;
    server_thread.join();
    std::cout << "\n";
}

// ============================================================
// 演示3: poll代码结构优化技巧
// ============================================================
void demo_poll_optimization() {
    std::cout << "========================================\n";
    std::cout << "  演示3: poll代码结构优化技巧\n";
    std::cout << "========================================\n\n";

    std::cout << "【poll服务器的常见优化】\n\n";

    std::cout << "1. 动态数组管理\n";
    std::cout << "   使用std::vector<pollfd>动态管理fd集合\n";
    std::cout << "   删除fd时与末尾元素交换, 避免移动中间元素\n\n";

    std::cout << "2. 分离监听socket和客户端socket\n";
    std::cout << "   poll_fds[0]始终为监听socket\n";
    std::cout << "   新连接追加到数组末尾\n\n";

    std::cout << "3. 水平触发(LT)注意事项\n";
    std::cout << "   poll默认水平触发:\n";
    std::cout << "   - 如果数据未读完, 下次poll仍会返回\n";
    std::cout << "   - 不需要一次recv读完所有数据\n";
    std::cout << "   - 但可能导致同一fd多次触发\n\n";

    std::cout << "4. 连接数扩展性\n";
    std::cout << "   poll的fd数组可以动态增长\n";
    std::cout << "   但遍历开销仍为O(N)\n";
    std::cout << "   大量连接时仍需epoll/IOCP\n\n";

    std::cout << "【select → poll 迁移要点】\n\n";

    std::cout << "  select代码:\n";
    std::cout << "    fd_set read_fds;\n";
    std::cout << "    FD_ZERO(&read_fds);\n";
    std::cout << "    FD_SET(fd, &read_fds);\n";
    std::cout << "    select(max_fd+1, &read_fds, ...);\n";
    std::cout << "    if (FD_ISSET(fd, &read_fds)) { ... }\n\n";

    std::cout << "  poll代码:\n";
    std::cout << "    std::vector<pollfd> fds = {{fd, POLLIN, 0}};\n";
    std::cout << "    poll(fds.data(), fds.size(), timeout);\n";
    std::cout << "    if (fds[0].revents & POLLIN) { ... }\n\n";

    std::cout << "  关键区别:\n";
    std::cout << "  - 无需FD_ZERO/FD_SET/FD_ISSET宏\n";
    std::cout << "  - 无需每次重建fd集合\n";
    std::cout << "  - 代码更简洁, 更易维护\n";
}

// ============================================================
// 主函数
// ============================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  第8章: 高性能网络与异步IO - 第1节 poll模型与select对比\n";
    std::cout << "============================================================\n";

    WinsockInit winsock_init;

    demo_poll_vs_select();
    demo_poll_server();
    demo_poll_optimization();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
