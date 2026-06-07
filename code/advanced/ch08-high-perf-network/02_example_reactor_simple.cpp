/**
 * @file 02_example_reactor_simple.cpp
 * @brief Reactor模式: 简单Reactor模式实现, 基于select的事件驱动架构
 * @description 对应文档: 高性能网络与异步IO / 第2节 Reactor模式
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
#include <memory>

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
// 事件类型枚举
// ============================================================
enum class EventType : int {
    READABLE  = 0x01,   // 可读事件
    WRITABLE  = 0x02,   // 可写事件
};

// ============================================================
// 事件回调类型
// ============================================================
using EventCallback = std::function<void(socket_t fd)>;

// ============================================================
// 事件处理器基类 (模仿muduo的Channel)
// ============================================================
class EventHandler {
public:
    virtual ~EventHandler() = default;

    /// 当fd可读时被调用
    virtual void handle_read(socket_t fd) = 0;

    /// 当fd可写时被调用
    virtual void handle_write(socket_t fd) {
        // 默认空实现
        (void)fd;
    }

    /// 当发生错误时被调用
    virtual void handle_error(socket_t fd) {
        std::cerr << "[EventHandler] fd=" << fd << " 发生错误\n";
    }
};

// ============================================================
// 简单Reactor实现
// ============================================================
class SimpleReactor {
public:
    SimpleReactor() = default;
    ~SimpleReactor() = default;

    /// 注册事件处理器
    void register_handler(socket_t fd, EventHandler* handler, int events) {
        handlers_[fd] = handler;
        events_[fd] = events;
        std::cout << "[Reactor] 注册处理器: fd=" << fd
                  << ", 事件=" << (events & static_cast<int>(EventType::READABLE) ? "R" : "-")
                  << (events & static_cast<int>(EventType::WRITABLE) ? "W" : "-") << "\n";
    }

    /// 注销事件处理器
    void unregister_handler(socket_t fd) {
        handlers_.erase(fd);
        events_.erase(fd);
        std::cout << "[Reactor] 注销处理器: fd=" << fd << "\n";
    }

    /// 修改关注的事件
    void modify_events(socket_t fd, int events) {
        events_[fd] = events;
    }

    /// 事件循环 (核心)
    void loop(int timeout_ms = 1000) {
        std::cout << "[Reactor] 事件循环启动\n";

        while (running_) {
            // 构建fd_set
            fd_set read_fds, write_fds;
            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);

            socket_t max_fd = 0;
            for (auto& [fd, evts] : events_) {
                if (evts & static_cast<int>(EventType::READABLE)) {
                    FD_SET(fd, &read_fds);
                }
                if (evts & static_cast<int>(EventType::WRITABLE)) {
                    FD_SET(fd, &write_fds);
                }
                if (fd > max_fd) max_fd = fd;
            }

            struct timeval tv{};
            tv.tv_sec = timeout_ms / 1000;
            tv.tv_usec = (timeout_ms % 1000) * 1000;

            int ret = ::select(static_cast<int>(max_fd + 1),
                               &read_fds, &write_fds, nullptr, &tv);

            if (ret < 0) {
                std::cerr << "[Reactor] select错误: " << get_socket_error() << "\n";
                break;
            }

            if (ret == 0) continue;  // 超时

            // 分发事件给对应的处理器
            for (auto& [fd, evts] : events_) {
                auto it = handlers_.find(fd);
                if (it == handlers_.end()) continue;

                EventHandler* handler = it->second;

                if (FD_ISSET(fd, &read_fds)) {
                    handler->handle_read(fd);
                }
                if (FD_ISSET(fd, &write_fds)) {
                    handler->handle_write(fd);
                }
            }
        }

        std::cout << "[Reactor] 事件循环结束\n";
    }

    /// 停止事件循环
    void stop() {
        running_ = false;
    }

private:
    std::unordered_map<socket_t, EventHandler*> handlers_;
    std::unordered_map<socket_t, int> events_;
    bool running_ = true;
};

// ============================================================
// 接受连接的处理器 (Acceptor)
// ============================================================
class AcceptorHandler : public EventHandler {
public:
    using NewConnectionCallback = std::function<void(socket_t client_fd)>;

    AcceptorHandler(SimpleReactor& reactor, socket_t listen_fd)
        : reactor_(reactor), listen_fd_(listen_fd) {}

    void set_new_connection_callback(NewConnectionCallback cb) {
        on_new_connection_ = std::move(cb);
    }

    void handle_read(socket_t fd) override {
        // 监听socket可读 → 有新连接
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        socket_t client_fd = ::accept(fd,
            reinterpret_cast<sockaddr*>(&client_addr), &client_len);

        if (client_fd != INVALID_SOCKET_VAL) {
            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
            std::cout << "[Acceptor] 新连接: " << ip << ":"
                      << ntohs(client_addr.sin_port) << "\n";

            if (on_new_connection_) {
                on_new_connection_(client_fd);
            }
        }
    }

private:
    SimpleReactor& reactor_;
    socket_t listen_fd_;
    NewConnectionCallback on_new_connection_;
};

// ============================================================
// Echo连接的处理器
// ============================================================
class EchoHandler : public EventHandler {
public:
    EchoHandler(SimpleReactor& reactor, socket_t fd)
        : reactor_(reactor), fd_(fd) {}

    void handle_read(socket_t fd) override {
        char buf[1024];
        ssize_t n = ::recv(fd, buf, sizeof(buf) - 1, 0);

        if (n <= 0) {
            std::cout << "[EchoHandler] 连接关闭 (fd=" << fd << ")\n";
            CLOSE_SOCKET(fd);
            reactor_.unregister_handler(fd);
            delete this;  // 自删除 (简单演示, 生产环境应用智能指针)
            return;
        }

        buf[n] = '\0';
        std::cout << "[EchoHandler] 收到 (fd=" << fd << "): \"" << buf << "\"\n";

        // Echo回显
        ::send(fd, buf, static_cast<int>(n), 0);
        std::cout << "[EchoHandler] 已回显\n";
    }

    void handle_error(socket_t fd) override {
        std::cout << "[EchoHandler] 错误 (fd=" << fd << ")\n";
        CLOSE_SOCKET(fd);
        reactor_.unregister_handler(fd);
        delete this;
    }

private:
    SimpleReactor& reactor_;
    socket_t fd_;
};

// ============================================================
// 演示1: Reactor模式概念
// ============================================================
void demo_reactor_concepts() {
    std::cout << "\n========================================\n";
    std::cout << "  演示1: Reactor模式概念\n";
    std::cout << "========================================\n\n";

    std::cout << "【Reactor模式核心思想】\n\n";

    std::cout << "  Reactor = 事件驱动 + 回调\n\n";

    std::cout << "  传统模型:\n";
    std::cout << "    while (true) {\n";
    std::cout << "        等待事件 → 处理事件 (阻塞)\n";
    std::cout << "    }\n\n";

    std::cout << "  Reactor模型:\n";
    std::cout << "    1. 注册感兴趣的事件和回调\n";
    std::cout << "    2. Reactor等待事件 (epoll_wait/select)\n";
    std::cout << "    3. 事件发生时, 分发给对应的回调处理\n";
    std::cout << "    4. 回调处理完后返回Reactor继续等待\n\n";

    std::cout << "【Reactor的三大组件】\n\n";

    std::cout << "  1. Reactor (事件循环)\n";
    std::cout << "     - 负责等待事件 (select/poll/epoll)\n";
    std::cout << "     - 将事件分发给对应的Handler\n";
    std::cout << "     - 也称为Dispatcher或EventLoop\n\n";

    std::cout << "  2. Handler (事件处理器)\n";
    std::cout << "     - 定义事件处理逻辑\n";
    std::cout << "     - 每个fd对应一个Handler\n";
    std::cout << "     - 实现handle_read/handle_write等方法\n\n";

    std::cout << "  3. Acceptor (连接接受器)\n";
    std::cout << "     - 专门处理监听socket的可读事件\n";
    std::cout << "     - accept新连接后创建对应的Handler\n\n";

    std::cout << "【Reactor模式的优点】\n";
    std::cout << "  - 解耦: 事件检测与事件处理分离\n";
    std::cout << "  - 可扩展: 新增事件类型只需新增Handler\n";
    std::cout << "  - 高性能: 单线程处理多连接\n";
    std::cout << "  - 可移植: 底层可用select/poll/epoll/IOCP\n\n";

    std::cout << "【使用Reactor模式的知名项目】\n";
    std::cout << "  - Redis: 单线程Reactor + ae事件库\n";
    std::cout << "  - Nginx: 多进程Reactor\n";
    std::cout << "  - muduo: 多Reactor多线程 (陈硕)\n";
    std::cout << "  - libevent: 跨平台Reactor封装\n";
    std::cout << "  - Netty (Java): Reactor模型\n";
}

// ============================================================
// 演示2: 简单Reactor Echo服务器
// ============================================================
void demo_reactor_server() {
    std::cout << "\n========================================\n";
    std::cout << "  演示2: 简单Reactor Echo服务器\n";
    std::cout << "========================================\n\n";

    const uint16_t port = 15004;

    // 创建监听socket
    socket_t listen_fd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == INVALID_SOCKET_VAL) {
        std::cerr << "[错误] 创建socket失败\n";
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
        std::cerr << "[错误] bind失败\n";
        CLOSE_SOCKET(listen_fd);
        return;
    }

    if (::listen(listen_fd, 5) != 0) {
        std::cerr << "[错误] listen失败\n";
        CLOSE_SOCKET(listen_fd);
        return;
    }

    std::cout << "[信息] Reactor Echo服务器启动, 端口: " << port << "\n\n";

    // 创建Reactor
    SimpleReactor reactor;

    // 创建Acceptor
    AcceptorHandler acceptor(reactor, listen_fd);
    acceptor.set_new_connection_callback([&reactor](socket_t client_fd) {
        // 新连接到来, 创建EchoHandler
        auto* handler = new EchoHandler(reactor, client_fd);
        reactor.register_handler(client_fd, handler,
                                static_cast<int>(EventType::READABLE));
    });

    // 注册监听socket
    reactor.register_handler(listen_fd, &acceptor,
                            static_cast<int>(EventType::READABLE));

    // 在另一个线程运行Reactor事件循环
    std::atomic<bool> clients_done{false};

    std::thread reactor_thread([&]() {
        // 使用 Reactor 自身的事件循环
        reactor.loop(200);  // 200ms 超时
    });

    // 客户端测试
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    auto run_client = [&](const std::string& name,
                          const std::vector<std::string>& messages) {
        socket_t sock = ::socket(AF_INET, SOCK_STREAM, 0);
        if (sock == INVALID_SOCKET_VAL) return;

        sockaddr_in server_addr{};
        server_addr.sin_family = AF_INET;
        server_addr.sin_port = htons(port);
        inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr);

        if (::connect(sock, reinterpret_cast<sockaddr*>(&server_addr),
                      sizeof(server_addr)) == 0) {
            std::cout << "[" << name << "] 连接成功\n";

            char buf[1024];
            for (const auto& msg : messages) {
                ::send(sock, msg.c_str(), static_cast<int>(msg.size()), 0);
                std::cout << "[" << name << "] 发送: \"" << msg << "\"\n";

                ssize_t n = ::recv(sock, buf, sizeof(buf) - 1, 0);
                if (n > 0) {
                    buf[n] = '\0';
                    std::cout << "[" << name << "] 收到回显: \"" << buf << "\"\n";
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
            CLOSE_SOCKET(sock);
        }
    };

    std::thread client1([&]() {
        run_client("Reactor客户端1", {"Reactor模式测试", "事件驱动架构"});
    });

    client1.join();
    clients_done = true;
    reactor.stop();  // 通知 reactor 退出事件循环
    reactor_thread.join();

    CLOSE_SOCKET(listen_fd);
    std::cout << "\n";
}

// ============================================================
// 演示3: Reactor模式演进路线
// ============================================================
void demo_reactor_evolution() {
    std::cout << "========================================\n";
    std::cout << "  演示3: Reactor模式演进路线\n";
    std::cout << "========================================\n\n";

    std::cout << "【单Reactor单线程】(本节演示)\n";
    std::cout << "  一个线程负责: accept + IO + 业务逻辑\n";
    std::cout << "  优点: 简单, 无并发问题\n";
    std::cout << "  缺点: 业务逻辑不能太慢, 否则影响其他连接\n";
    std::cout << "  适用: Redis等轻量业务\n\n";

    std::cout << "【单Reactor多线程】\n";
    std::cout << "  Reactor线程: accept + IO\n";
    std::cout << "  工作线程池: 业务逻辑\n";
    std::cout << "  优点: 业务逻辑不阻塞IO\n";
    std::cout << "  缺点: Reactor线程仍是瓶颈\n\n";

    std::cout << "【主从Reactor多线程】(muduo/Netty)\n";
    std::cout << "  主Reactor: 只负责accept\n";
    std::cout << "  从Reactor(多个): 负责已连接的IO\n";
    std::cout << "  工作线程池: 业务逻辑\n";
    std::cout << "  优点: 充分利用多核, 高并发\n";
    std::cout << "  缺点: 架构复杂\n";
    std::cout << "  适用: 高性能服务器\n\n";

    std::cout << "【muduo的架构】\n";
    std::cout << "  EventLoop       : 事件循环 (Reactor)\n";
    std::cout << "  Channel         : fd + 事件 + 回调\n";
    std::cout << "  Poller          : select/poll/epoll封装\n";
    std::cout << "  TcpServer       : 服务器封装\n";
    std::cout << "  TcpConnection   : 连接封装\n";
    std::cout << "  EventLoopThreadPool : 从Reactor线程池\n";
}

// ============================================================
// 主函数
// ============================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  第8章: 高性能网络与异步IO - 第2节 简单Reactor模式\n";
    std::cout << "============================================================\n";

    WinsockInit winsock_init;

    demo_reactor_concepts();
    demo_reactor_server();
    demo_reactor_evolution();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
