/**
 * @file 01_deep_dive_epoll_concepts.cpp
 * @brief IO多路复用深入: epoll概念与API详解 (Linux专属, Windows用select演示)
 * @description 对应文档: 高性能网络与异步IO / 第1节 IO多路复用深入
 *
 * 注意: epoll是Linux专属API, 本文件在Windows上使用select模拟演示epoll的概念。
 *       在Linux上编译时, 会使用真正的epoll API。
 */

#include <iostream>
#include <string>
#include <cstring>
#include <vector>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>
#include <unordered_map>

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
    #define HAS_EPOLL 0
#elif defined(__linux__)
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <sys/epoll.h>
    #include <netdb.h>
    #include <errno.h>
    #include <fcntl.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VAL (-1)
    #define CLOSE_SOCKET close
    #define HAS_EPOLL 1
#else
    // macOS/BSD 使用 kqueue
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
    #include <netdb.h>
    #include <errno.h>
    typedef int socket_t;
    #define INVALID_SOCKET_VAL (-1)
    #define CLOSE_SOCKET close
    #define HAS_EPOLL 0
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
// 演示1: epoll核心概念
// ============================================================
void demo_epoll_concepts() {
    std::cout << "\n========================================\n";
    std::cout << "  演示1: epoll核心概念\n";
    std::cout << "========================================\n\n";

    std::cout << "【epoll三大API】\n\n";

    std::cout << "1. epoll_create(int size)\n";
    std::cout << "   创建epoll实例, 返回epoll fd\n";
    std::cout << "   size参数在Linux 2.6.8+被忽略(自动扩展)\n";
    std::cout << "   内部维护: 红黑树(注册fd) + 就绪链表(就绪fd)\n\n";

    std::cout << "2. epoll_ctl(int epfd, int op, int fd, struct epoll_event *event)\n";
    std::cout << "   操作epoll实例中的fd:\n";
    std::cout << "   EPOLL_CTL_ADD : 注册新fd\n";
    std::cout << "   EPOLL_CTL_MOD : 修改fd的事件\n";
    std::cout << "   EPOLL_CTL_DEL : 删除fd\n";
    std::cout << "   只在注册时拷贝一次数据到内核, 之后无需再拷贝!\n\n";

    std::cout << "3. epoll_wait(int epfd, struct epoll_event *events,\n";
    std::cout << "              int maxevents, int timeout)\n";
    std::cout << "   等待就绪事件, 只返回就绪的fd\n";
    std::cout << "   时间复杂度: O(就绪fd数), 而非O(总fd数)!\n\n";

    std::cout << "【epoll_event结构体】\n";
    std::cout << "  struct epoll_event {\n";
    std::cout << "      uint32_t     events;  // 事件位掩码\n";
    std::cout << "      epoll_data_t data;    // 用户数据\n";
    std::cout << "  };\n\n";

    std::cout << "  union epoll_data_t {\n";
    std::cout << "      void    *ptr;   // 指针(最常用, 指向连接对象)\n";
    std::cout << "      int      fd;    // 文件描述符\n";
    std::cout << "      uint32_t u32;\n";
    std::cout << "      uint64_t u64;\n";
    std::cout << "  };\n\n";

    std::cout << "【epoll的两种触发模式】\n\n";

    std::cout << "  水平触发 (Level Triggered, LT) - 默认:\n";
    std::cout << "    只要fd有数据可读, 每次epoll_wait都会返回\n";
    std::cout << "    编程简单, 不怕漏事件\n";
    std::cout << "    可能多次通知同一事件\n\n";

    std::cout << "  边缘触发 (Edge Triggered, ET) - 高性能:\n";
    std::cout << "    只在fd状态变化时通知一次\n";
    std::cout << "    必须一次性读完/写完所有数据(循环recv直到EAGAIN)\n";
    std::cout << "    减少epoll_wait调用次数, 性能更高\n";
    std::cout << "    编程复杂, 容易漏数据\n\n";

    std::cout << "【epoll为什么比select/poll快?】\n\n";

    std::cout << "  1. 无需每次拷贝fd集合\n";
    std::cout << "     select/poll: 每次调用都要把fd集合从用户态拷贝到内核态\n";
    std::cout << "     epoll: 只在epoll_ctl时拷贝一次, epoll_wait无需拷贝\n\n";

    std::cout << "  2. 无需遍历所有fd\n";
    std::cout << "     select/poll: 返回后遍历所有fd检查就绪 O(N)\n";
    std::cout << "     epoll: 只返回就绪的fd O(就绪数)\n\n";

    std::cout << "  3. 内核使用回调而非轮询\n";
    std::cout << "     select/poll: 内核遍历所有fd检查就绪 O(N)\n";
    std::cout << "     epoll: fd就绪时通过回调加入就绪链表 O(1)\n";
}

// ============================================================
// 演示2: epoll服务器概念代码 (跨平台, Windows用select模拟)
// ============================================================
void demo_epoll_style_server() {
    std::cout << "\n========================================\n";
    std::cout << "  演示2: epoll风格服务器 (概念演示)\n";
    std::cout << "========================================\n\n";

#if HAS_EPOLL
    std::cout << "[信息] 当前平台支持epoll, 使用真实epoll API\n\n";
#else
    std::cout << "[信息] 当前平台不支持epoll, 使用select模拟epoll概念\n\n";
#endif

    const uint16_t port = 15003;

    std::atomic<bool> server_ready{false};
    std::atomic<bool> server_done{false};

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

        std::cout << "[服务器] epoll风格服务器启动, 端口: " << port << "\n";

#if HAS_EPOLL
        // ===== 真正的epoll实现 (Linux) =====
        int epfd = epoll_create1(0);
        if (epfd < 0) {
            std::cerr << "[服务器] epoll_create1失败\n";
            CLOSE_SOCKET(listen_fd);
            return;
        }

        // 注册监听socket
        struct epoll_event ev{};
        ev.events = EPOLLIN;       // 水平触发, 可读事件
        ev.data.fd = listen_fd;
        epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

        std::vector<struct epoll_event> events(64);

        server_ready = true;

        while (!server_done) {
            int nfds = epoll_wait(epfd, events.data(),
                                  static_cast<int>(events.size()), 1000);
            if (nfds < 0) {
                if (errno == EINTR) continue;
                break;
            }

            for (int i = 0; i < nfds; i++) {
                if (events[i].data.fd == listen_fd) {
                    // 新连接
                    sockaddr_in client_addr{};
                    socklen_t client_len = sizeof(client_addr);
                    socket_t client_fd = ::accept(listen_fd,
                        reinterpret_cast<sockaddr*>(&client_addr), &client_len);
                    if (client_fd != INVALID_SOCKET_VAL) {
                        ev.events = EPOLLIN | EPOLLET;  // 边缘触发
                        ev.data.fd = client_fd;
                        epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &ev);
                        std::cout << "[服务器] 新连接 (fd=" << client_fd << ")\n";
                    }
                } else {
                    // 客户端数据 (ET模式必须循环读取直到EAGAIN)
                    char buf[1024];
                    while (true) {
                        ssize_t n = ::recv(events[i].data.fd, buf, sizeof(buf) - 1, 0);
                        if (n > 0) {
                            buf[n] = '\0';
                            std::cout << "[服务器] 收到: \"" << buf << "\"\n";
                            ::send(events[i].data.fd, buf, static_cast<int>(n), 0);
                        } else if (n == 0) {
                            // 对端关闭连接
                            epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                            CLOSE_SOCKET(events[i].data.fd);
                            std::cout << "[服务器] 客户端断开\n";
                            break;
                        } else {
                            // n < 0
                            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                                break;  // ET模式: 数据已全部读完
                            }
                            epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, nullptr);
                            CLOSE_SOCKET(events[i].data.fd);
                            std::cout << "[服务器] recv错误, 关闭连接\n";
                            break;
                        }
                    }
                }
            }
        }

        close(epfd);
#else
        // ===== Windows/其他平台: 用select模拟epoll概念 =====
        // 注意: 这只是为了演示epoll的编程模型, 性能并不等同
        std::cout << "[服务器] 使用select模拟epoll事件驱动模型\n";
        std::cout << "[服务器] (生产环境Windows应使用IOCP)\n\n";

        // 模拟epoll的"注册/注销"机制
        std::unordered_map<socket_t, std::string> fd_info;  // fd → 描述信息
        fd_info[listen_fd] = "监听socket";

        server_ready = true;

        int iteration = 0;
        while (iteration < 50 && !server_done) {
            iteration++;

            fd_set read_fds;
            FD_ZERO(&read_fds);

            socket_t max_fd = 0;
            for (auto& [fd, info] : fd_info) {
                FD_SET(fd, &read_fds);
                if (fd > max_fd) max_fd = fd;
            }

            struct timeval tv{};
            tv.tv_sec = 1;
            tv.tv_usec = 0;

            int ret = ::select(static_cast<int>(max_fd + 1), &read_fds, nullptr, nullptr, &tv);
            if (ret <= 0) continue;

            // 模拟epoll_wait: 只处理就绪的fd
            // (select需要遍历, epoll只返回就绪的 — 这是关键区别)
            std::vector<socket_t> ready_fds;
            for (auto& [fd, info] : fd_info) {
                if (FD_ISSET(fd, &read_fds)) {
                    ready_fds.push_back(fd);
                }
            }

            // 处理就绪事件 (类似epoll_wait返回的events数组)
            for (socket_t fd : ready_fds) {
                if (fd == listen_fd) {
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
                        // 模拟epoll_ctl(ADD)
                        fd_info[client_fd] = "客户端socket";
                    }
                } else {
                    char buf[1024];
                    ssize_t n = ::recv(fd, buf, sizeof(buf) - 1, 0);
                    if (n <= 0) {
                        std::cout << "[服务器] 客户端断开 (fd=" << fd << ")\n";
                        CLOSE_SOCKET(fd);
                        // 模拟epoll_ctl(DEL)
                        fd_info.erase(fd);
                    } else {
                        buf[n] = '\0';
                        std::cout << "[服务器] 收到 (fd=" << fd << "): \""
                                  << buf << "\"\n";
                        ::send(fd, buf, static_cast<int>(n), 0);
                    }
                }
            }
        }

        for (auto& [fd, info] : fd_info) {
            CLOSE_SOCKET(fd);
        }
#endif

        std::cout << "[服务器] 关闭完成\n";
    });

    // 等待服务器就绪
    while (!server_ready) {
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 客户端
    socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
    if (sock != INVALID_SOCKET_VAL) {
        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        if (::connect(sock, reinterpret_cast<sockaddr*>(&server_addr),
                      sizeof(server_addr)) == 0) {
            std::cout << "[客户端] 连接成功\n";

            char buf[1024];
            std::vector<std::string> msgs = {"epoll概念测试", "事件驱动模型"};
            for (const auto& msg : msgs) {
                ::send(sock, msg.c_str(), static_cast<int>(msg.size()), 0);
                std::cout << "[客户端] 发送: \"" << msg << "\"\n";
                ssize_t n = ::recv(sock, buf, sizeof(buf) - 1, 0);
                if (n > 0) {
                    buf[n] = '\0';
                    std::cout << "[客户端] 收到: \"" << buf << "\"\n";
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }
        CLOSE_SOCKET(sock);
    }

    server_done = true;
    server_thread.join();
    std::cout << "\n";
}

// ============================================================
// 演示3: epoll边缘触发(ET)模式详解
// ============================================================
void demo_epoll_et_mode() {
    std::cout << "========================================\n";
    std::cout << "  演示3: epoll边缘触发(ET)模式详解\n";
    std::cout << "========================================\n\n";

    std::cout << "【LT vs ET 对比】\n\n";

    std::cout << "  场景: socket接收缓冲区有100字节, 应用只读了50字节\n\n";

    std::cout << "  水平触发(LT):\n";
    std::cout << "    第1次epoll_wait → 返回可读事件\n";
    std::cout << "    应用recv 50字节\n";
    std::cout << "    第2次epoll_wait → 仍然返回可读事件(还有50字节)\n";
    std::cout << "    应用recv 50字节\n";
    std::cout << "    第3次epoll_wait → 不返回(缓冲区空了)\n\n";

    std::cout << "  边缘触发(ET):\n";
    std::cout << "    第1次epoll_wait → 返回可读事件(状态变化: 空→有数据)\n";
    std::cout << "    应用recv 50字节\n";
    std::cout << "    第2次epoll_wait → 不返回!(没有新的状态变化)\n";
    std::cout << "    ⚠ 剩余50字节被遗漏!\n\n";

    std::cout << "【ET模式的正确用法】\n\n";

    std::cout << "  1. 将socket设为非阻塞\n";
    std::cout << "  2. 收到可读事件后, 循环recv直到返回EAGAIN/EWOULDBLOCK\n";
    std::cout << "  3. 收到可写事件后, 循环send直到返回EAGAIN/EWOULDBLOCK\n\n";

    std::cout << "  伪代码:\n";
    std::cout << "    // ET模式读取\n";
    std::cout << "    while (true) {\n";
    std::cout << "        ssize_t n = recv(fd, buf, sizeof(buf), 0);\n";
    std::cout << "        if (n > 0) { 处理数据; }\n";
    std::cout << "        else if (n == 0) { 连接关闭; break; }\n";
    std::cout << "        else {\n";
    std::cout << "            if (errno == EAGAIN) {\n";
    std::cout << "                // 数据读完了, 等待下次通知\n";
    std::cout << "                break;\n";
    std::cout << "            }\n";
    std::cout << "            // 其他错误\n";
    std::cout << "            break;\n";
    std::cout << "        }\n";
    std::cout << "    }\n\n";

    std::cout << "【ET模式的优缺点】\n\n";

    std::cout << "  优点:\n";
    std::cout << "    - 减少epoll_wait调用次数\n";
    std::cout << "    - 每个fd只在状态变化时通知一次\n";
    std::cout << "    - 高并发下性能优于LT\n\n";

    std::cout << "  缺点:\n";
    std::cout << "    - 编程复杂, 必须循环读/写\n";
    std::cout << "    - 容易漏数据\n";
    std::cout << "    - 需要处理EAGAIN\n\n";

    std::cout << "【各平台等价机制】\n\n";

    std::cout << "  Linux : epoll  (EPOLLET标志)\n";
    std::cout << "  Windows: IOCP  (完成端口, 真异步)\n";
    std::cout << "  macOS/BSD: kqueue (EV_CLEAR标志 = ET)\n";
    std::cout << "  跨平台: Boost.Asio (统一封装)\n";
}

// ============================================================
// 主函数
// ============================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  第8章: 高性能网络与异步IO - 第1节 epoll概念与API\n";
    std::cout << "============================================================\n";

    WinsockInit winsock_init;

    demo_epoll_concepts();
    demo_epoll_style_server();
    demo_epoll_et_mode();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
