/**
 * @file 02_deep_dive_reactor_eventloop.cpp
 * @brief Reactor模式深入: EventLoop + Channel + Callback架构实现
 * @description 对应文档: 高性能网络与异步IO / 第2节 Reactor模式
 *
 * 本文件实现一个更完整的Reactor架构, 模仿muduo网络库的核心设计:
 *   - EventLoop: 事件循环, 封装select/poll
 *   - Channel: fd与事件回调的绑定
 *   - 回调驱动的编程模型
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
#include <queue>
#include <mutex>
#include <cassert>

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
// Channel: 封装fd及其事件回调 (模仿muduo::Channel)
// ============================================================
class Channel {
public:
    using EventCallback = std::function<void()>;

    Channel(socket_t fd) : fd_(fd) {}
    ~Channel() = default;

    /// 设置可读回调
    void set_read_callback(EventCallback cb) { read_callback_ = std::move(cb); }

    /// 设置可写回调
    void set_write_callback(EventCallback cb) { write_callback_ = std::move(cb); }

    /// 设置错误回调
    void set_error_callback(EventCallback cb) { error_callback_ = std::move(cb); }

    /// 设置关闭回调
    void set_close_callback(EventCallback cb) { close_callback_ = std::move(cb); }

    /// 启用可读事件监控
    void enable_reading() { events_ |= kReadEvent; }

    /// 启用可写事件监控
    void enable_writing() { events_ |= kWriteEvent; }

    /// 禁用可写事件监控
    void disable_writing() { events_ &= ~kWriteEvent; }

    /// 禁用所有事件
    void disable_all() { events_ = kNoneEvent; }

    /// 处理事件 (由EventLoop调用)
    void handle_event() {
        if (revents_ & kReadEvent) {
            if (read_callback_) read_callback_();
        }
        if (revents_ & kWriteEvent) {
            if (write_callback_) write_callback_();
        }
        if (revents_ & kErrorEvent) {
            if (error_callback_) error_callback_();
        }
        if (revents_ & kCloseEvent) {
            if (close_callback_) close_callback_();
        }
    }

    socket_t fd() const { return fd_; }
    int events() const { return events_; }
    void set_revents(int revt) { revents_ = revt; }
    bool is_none_event() const { return events_ == kNoneEvent; }

    // 事件常量
    static constexpr int kNoneEvent  = 0;
#ifdef _WIN32
    // Windows: 使用自定义值 (select模型不使用POLLIN等)
    static constexpr int kReadEvent  = 0x01;  // 可读
    static constexpr int kWriteEvent = 0x02;  // 可写
    static constexpr int kErrorEvent = 0x04;  // 错误
    static constexpr int kCloseEvent = 0x08;  // 关闭
#else
    // POSIX: 使用poll的事件标志
    static constexpr int kReadEvent  = POLLIN;
    static constexpr int kWriteEvent = POLLOUT;
    static constexpr int kErrorEvent = POLLERR;
    static constexpr int kCloseEvent = POLLHUP;
#endif

private:
    socket_t fd_;
    int events_ = kNoneEvent;   // 关注的事件
    int revents_ = kNoneEvent;  // 实际发生的事件

    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback error_callback_;
    EventCallback close_callback_;
};

// ============================================================
// EventLoop: 事件循环 (模仿muduo::EventLoop)
// ============================================================
class EventLoop {
public:
    EventLoop() = default;
    ~EventLoop() = default;

    /// 在事件循环中执行回调 (线程安全)
    void run_in_loop(std::function<void()> cb) {
        if (is_in_loop_thread()) {
            cb();
        } else {
            queue_in_loop(std::move(cb));
        }
    }

    /// 将回调加入队列 (线程安全)
    void queue_in_loop(std::function<void()> cb) {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_callbacks_.push(std::move(cb));
    }

    /// 更新Channel (注册/修改)
    void update_channel(Channel* channel) {
        channels_[channel->fd()] = channel;
    }

    /// 移除Channel
    void remove_channel(Channel* channel) {
        channels_.erase(channel->fd());
    }

    /// 事件循环 (核心)
    void loop() {
        std::cout << "[EventLoop] 事件循环启动\n";
        looping_ = true;

        while (!quit_) {
            // 1. 构建fd_set
            fd_set read_fds, write_fds, error_fds;
            FD_ZERO(&read_fds);
            FD_ZERO(&write_fds);
            FD_ZERO(&error_fds);

            socket_t max_fd = 0;
            for (auto& [fd, channel] : channels_) {
                if (channel->events() & Channel::kReadEvent) {
                    FD_SET(fd, &read_fds);
                }
                if (channel->events() & Channel::kWriteEvent) {
                    FD_SET(fd, &write_fds);
                }
                FD_SET(fd, &error_fds);
                if (fd > max_fd) max_fd = fd;
            }

            // 2. 等待事件
            struct timeval tv{};
            tv.tv_sec = 0;
            tv.tv_usec = 100000;  // 100ms超时, 允许处理pending回调

            int ret = ::select(static_cast<int>(max_fd + 1),
                               &read_fds, &write_fds, &error_fds, &tv);

            // 3. 分发事件 (先收集就绪channel, 再分发, 避免回调中修改channels_导致迭代器失效)
            if (ret > 0) {
                std::vector<Channel*> active_channels;
                for (auto& [fd, channel] : channels_) {
                    int revents = Channel::kNoneEvent;
                    if (FD_ISSET(fd, &read_fds))  revents |= Channel::kReadEvent;
                    if (FD_ISSET(fd, &write_fds)) revents |= Channel::kWriteEvent;
                    if (FD_ISSET(fd, &error_fds)) revents |= Channel::kErrorEvent;

                    if (revents != Channel::kNoneEvent) {
                        channel->set_revents(revents);
                        active_channels.push_back(channel);
                    }
                }
                for (auto* channel : active_channels) {
                    channel->handle_event();
                }
            }

            // 4. 处理pending回调
            do_pending_callbacks();
        }

        looping_ = false;
        std::cout << "[EventLoop] 事件循环结束\n";
    }

    /// 停止事件循环
    void quit() { quit_ = true; }

    bool is_in_loop_thread() const {
        // 简化: 始终返回true (单线程模型)
        return true;
    }

private:
    void do_pending_callbacks() {
        std::queue<std::function<void()>> callbacks;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            callbacks.swap(pending_callbacks_);
        }
        while (!callbacks.empty()) {
            auto& cb = callbacks.front();
            cb();
            callbacks.pop();
        }
    }

    std::unordered_map<socket_t, Channel*> channels_;
    std::queue<std::function<void()>> pending_callbacks_;
    std::mutex mutex_;
    bool looping_ = false;
    bool quit_ = false;
};

// ============================================================
// TcpConnection: 封装TCP连接 (模仿muduo::TcpConnection)
// ============================================================
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using MessageCallback = std::function<void(std::shared_ptr<TcpConnection>,
                                               const std::string&)>;

    TcpConnection(EventLoop& loop, socket_t fd, const std::string& name)
        : loop_(loop), fd_(fd), name_(name), channel_(fd) {
        // 设置Channel回调
        channel_.set_read_callback([this]() { handle_read(); });
        channel_.set_error_callback([this]() { handle_error(); });
        channel_.set_close_callback([this]() { handle_close(); });
        channel_.enable_reading();
    }

    ~TcpConnection() {
        if (fd_ != INVALID_SOCKET_VAL) {
            CLOSE_SOCKET(fd_);
        }
    }

    /// 初始化: 将Channel注册到EventLoop
    void connect_established() {
        loop_.update_channel(&channel_);
        std::cout << "[TcpConnection:" << name_ << "] 连接建立 (fd=" << fd_ << ")\n";
    }

    /// 发送数据
    void send(const std::string& msg) {
        ::send(fd_, msg.c_str(), static_cast<int>(msg.size()), 0);
    }

    /// 设置消息回调
    void set_message_callback(MessageCallback cb) {
        message_callback_ = std::move(cb);
    }

    socket_t fd() const { return fd_; }
    const std::string& name() const { return name_; }

private:
    void handle_read() {
        char buf[4096];
        ssize_t n = ::recv(fd_, buf, sizeof(buf) - 1, 0);
        if (n > 0) {
            buf[n] = '\0';
            if (message_callback_) {
                message_callback_(shared_from_this(), std::string(buf, n));
            }
        } else if (n == 0) {
            handle_close();
        } else {
            handle_error();
        }
    }

    void handle_error() {
        std::cout << "[TcpConnection:" << name_ << "] 错误\n";
        handleCloseInternal();
    }

    void handle_close() {
        std::cout << "[TcpConnection:" << name_ << "] 连接关闭\n";
        handleCloseInternal();
    }

    void handleCloseInternal() {
        channel_.disable_all();
        loop_.remove_channel(&channel_);
    }

    EventLoop& loop_;
    socket_t fd_;
    std::string name_;
    Channel channel_;
    MessageCallback message_callback_;
};

// ============================================================
// 演示1: EventLoop + Channel架构详解
// ============================================================
void demo_eventloop_architecture() {
    std::cout << "\n========================================\n";
    std::cout << "  演示1: EventLoop + Channel架构详解\n";
    std::cout << "========================================\n\n";

    std::cout << "【muduo风格Reactor架构】\n\n";

    std::cout << "  EventLoop (事件循环)\n";
    std::cout << "    ├── loop()          : 主循环, 等待事件+分发\n";
    std::cout << "    ├── update_channel() : 注册/修改Channel\n";
    std::cout << "    ├── run_in_loop()   : 线程安全地执行回调\n";
    std::cout << "    └── quit()          : 停止循环\n\n";

    std::cout << "  Channel (事件通道)\n";
    std::cout << "    ├── fd_             : 关联的文件描述符\n";
    std::cout << "    ├── events_         : 关注的事件\n";
    std::cout << "    ├── revents_        : 实际发生的事件\n";
    std::cout << "    ├── read_callback_  : 可读回调\n";
    std::cout << "    ├── write_callback_ : 可写回调\n";
    std::cout << "    └── close_callback_ : 关闭回调\n\n";

    std::cout << "  TcpConnection (TCP连接)\n";
    std::cout << "    ├── 封装socket fd\n";
    std::cout << "    ├── 拥有一个Channel\n";
    std::cout << "    ├── 提供send()方法\n";
    std::cout << "    └── 通过回调通知上层\n\n";

    std::cout << "【关键设计原则】\n\n";

    std::cout << "  1. One Loop Per Thread\n";
    std::cout << "     每个EventLoop只在自己的线程中运行\n";
    std::cout << "     避免锁, 减少竞争\n\n";

    std::cout << "  2. 回调驱动\n";
    std::cout << "     所有事件处理通过回调完成\n";
    std::cout << "     不继承, 用std::function组合\n\n";

    std::cout << "  3. run_in_loop\n";
    std::cout << "     其他线程通过run_in_loop安全地操作EventLoop\n";
    std::cout << "     实现线程间安全通信\n\n";

    std::cout << "  4. Channel不拥有fd\n";
    std::cout << "     Channel只负责事件分发, 不负责fd生命周期\n";
    std::cout << "     fd由TcpConnection或Acceptor管理\n";
}

// ============================================================
// 演示2: EventLoop + Channel + TcpConnection 完整服务器
// ============================================================
void demo_full_reactor_server() {
    std::cout << "\n========================================\n";
    std::cout << "  演示2: EventLoop+Channel+TcpConnection服务器\n";
    std::cout << "========================================\n\n";

    const uint16_t port = 15005;

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

    std::cout << "[信息] EventLoop Echo服务器启动, 端口: " << port << "\n\n";

    // 创建EventLoop
    EventLoop loop;

    // 创建监听Channel
    Channel listen_channel(listen_fd);
    listen_channel.enable_reading();

    // 连接计数器
    int conn_count = 0;

    // 设置监听回调: accept新连接
    listen_channel.set_read_callback([&]() {
        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);
        socket_t client_fd = ::accept(listen_fd,
            reinterpret_cast<sockaddr*>(&client_addr), &client_len);

        if (client_fd != INVALID_SOCKET_VAL) {
            conn_count++;
            std::string conn_name = "conn-" + std::to_string(conn_count);

            char ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &client_addr.sin_addr, ip, sizeof(ip));
            std::cout << "[服务器] 新连接: " << ip << ":"
                      << ntohs(client_addr.sin_port)
                      << " → " << conn_name << "\n";

            // 创建TcpConnection
            auto conn = std::make_shared<TcpConnection>(loop, client_fd, conn_name);

            // 设置消息回调: Echo
            conn->set_message_callback([](std::shared_ptr<TcpConnection> conn,
                                          const std::string& msg) {
                std::cout << "[Echo] 收到: \"" << msg << "\"\n";
                conn->send(msg);
                std::cout << "[Echo] 已回显\n";
            });

            conn->connect_established();
        }
    });

    loop.update_channel(&listen_channel);

    // 在另一个线程运行EventLoop
    std::atomic<bool> test_done{false};

    std::thread loop_thread([&]() {
        int iterations = 0;
        while (iterations < 80 && !test_done) {
            // 手动实现单次事件循环
            fd_set read_fds;
            FD_ZERO(&read_fds);
            FD_SET(listen_fd, &read_fds);

            struct timeval tv{};
            tv.tv_sec = 0;
            tv.tv_usec = 100000;

            int ret = ::select(static_cast<int>(listen_fd + 1),
                               &read_fds, nullptr, nullptr, &tv);

            if (ret > 0 && FD_ISSET(listen_fd, &read_fds)) {
                listen_channel.handle_event();
            }

            iterations++;
        }
    });

    // 客户端测试
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

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
            std::vector<std::string> msgs = {"EventLoop测试", "Channel+Callback"};
            for (const auto& msg : msgs) {
                ::send(sock, msg.c_str(), static_cast<int>(msg.size()), 0);
                std::cout << "[客户端] 发送: \"" << msg << "\"\n";

                ssize_t n = ::recv(sock, buf, sizeof(buf) - 1, 0);
                if (n > 0) {
                    buf[n] = '\0';
                    std::cout << "[客户端] 收到: \"" << buf << "\"\n";
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(200));
            }
            CLOSE_SOCKET(sock);
        }
    }

    test_done = true;
    loop_thread.join();
    CLOSE_SOCKET(listen_fd);
    std::cout << "\n";
}

// ============================================================
// 演示3: 回调驱动的编程模式
// ============================================================
void demo_callback_pattern() {
    std::cout << "========================================\n";
    std::cout << "  演示3: 回调驱动的编程模式\n";
    std::cout << "========================================\n\n";

    std::cout << "【传统阻塞式编程】\n";
    std::cout << "  void handle_client(socket_t fd) {\n";
    std::cout << "      while (true) {\n";
    std::cout << "          string msg = recv(fd);     // 阻塞\n";
    std::cout << "          if (msg.empty()) break;\n";
    std::cout << "          string resp = process(msg);\n";
    std::cout << "          send(fd, resp);            // 阻塞\n";
    std::cout << "      }\n";
    std::cout << "  }\n\n";

    std::cout << "【Reactor回调式编程】\n";
    std::cout << "  void on_message(TcpConnectionPtr conn, string msg) {\n";
    std::cout << "      string resp = process(msg);\n";
    std::cout << "      conn->send(resp);  // 非阻塞\n";
    std::cout << "  }\n\n";

    std::cout << "  // 注册回调\n";
    std::cout << "  server.set_message_callback(on_message);\n\n";

    std::cout << "【回调式编程的注意事项】\n\n";

    std::cout << "  1. 回调中不能执行耗时操作\n";
    std::cout << "     否则会阻塞整个EventLoop\n";
    std::cout << "     耗时操作应投递到线程池\n\n";

    std::cout << "  2. 生命周期管理\n";
    std::cout << "     回调中使用的对象必须保证生命周期\n";
    std::cout << "     使用shared_ptr/weak_ptr管理\n\n";

    std::cout << "  3. 回调嵌套\n";
    std::cout << "     避免在回调中调用会触发其他回调的操作\n";
    std::cout << "     使用run_in_loop延迟执行\n\n";

    std::cout << "【现代C++改进: 协程】\n";
    std::cout << "  C++20协程可以以同步风格写异步代码:\n\n";

    std::cout << "  // 协程版本 (伪代码)\n";
    std::cout << "  Task handle_client(TcpConnection conn) {\n";
    std::cout << "      while (true) {\n";
    std::cout << "          auto msg = co_await conn.async_recv();\n";
    std::cout << "          auto resp = process(msg);\n";
    std::cout << "          co_await conn.async_send(resp);\n";
    std::cout << "      }\n";
    std::cout << "  }\n\n";

    std::cout << "  协程的优势:\n";
    std::cout << "  - 代码直观, 接近同步风格\n";
    std::cout << "  - 编译器自动管理状态机\n";
    std::cout << "  - 无回调地狱\n";
    std::cout << "  - 是C++网络编程的未来方向\n";
}

// ============================================================
// 主函数
// ============================================================
int main() {
    std::cout << "============================================================\n";
    std::cout << "  第8章: 高性能网络与异步IO - 第2节 EventLoop+Channel架构\n";
    std::cout << "============================================================\n";

    WinsockInit winsock_init;

    demo_eventloop_architecture();
    demo_full_reactor_server();
    demo_callback_pattern();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
