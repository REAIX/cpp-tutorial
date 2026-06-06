# Reactor模式

> 掌握事件驱动网络编程的核心设计模式

---

> **The Reactor pattern is the backbone of most high-performance network frameworks.**
> （Reactor模式是大多数高性能网络框架的基石。）

> **Reactor：事件来了我通知，业务处理你来干。**

---

> 🎯 **以不变应万变，事件驱动架构的核心。**

> （掌握Reactor模式，构建可扩展的高性能网络服务。）

---

> 💡 **通俗理解 - Reactor模式**

想象医院分诊台：
- **单Reactor单线程**：一个护士分诊+看病 → 效率低但简单
- **单Reactor多线程**：一个护士分诊，多个医生看病 → 分诊可能成瓶颈
- **主从Reactor**：多个分诊台+多个医生 → 最高效

**Reactor模式就是"分诊台"——负责接收事件并分发给对应的处理器！**

> 🔬 **抽象理解 - Reactor模式**：
> - **Reactor**：是"事件分发器"，负责等待事件并将事件分发给对应的处理器
> - **Handler**：是"事件处理器"，与特定IO绑定，处理具体的读写事件
> - **Acceptor**：是"连接接收器"，处理新连接事件
> - **事件循环**：是"驱动引擎"，不断等待事件并分发
> - **Reactor的本质**：将"等待IO"和"处理IO"分离，实现非阻塞高效IO

---

## 前置知识
- [IO多路复用深入](01-IO多路复用深入.md)
- [多线程基础](../02-CPP/29-多线程基础.md)
## 后续内容
- [Proactor模式与异步IO](03-Proactor模式与异步IO.md)

## 目录

- [1. Reactor模式原理](#1-reactor模式原理)
- [2. 单Reactor单线程](#2-单reactor单线程)
- [3. 单Reactor多线程](#3-单reactor多线程)
- [4. 主从Reactor多线程](#4-主从reactor多线程)
- [5. Reactor模式代码实现](#5-reactor模式代码实现)
- [6. Netty与muduo的Reactor设计](#6-netty与muduo的reactor设计)
- [7. 小结](#7-小结)

---

## 1. Reactor模式原理

### 1.1 Reactor模式定义

Reactor模式是一种事件驱动的并发模式，核心思想是：**将事件的检测与事件的处理分离**。

```
┌──────────────────────────────────────────────────────────────┐
│                    Reactor模式结构                             │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│   ┌──────────────────────────────────────────────────┐      │
│   │                  Reactor（反应器）                  │      │
│   │                                                    │      │
│   │   ┌─────────────┐    ┌──────────────────────┐    │      │
│   │   │ 事件循环     │    │ 事件多路分发器        │    │      │
│   │   │ epoll_wait  │───▶│ 根据fd找到Handler    │    │      │
│   │   │ / select    │    │ 调用对应回调          │    │      │
│   │   └─────────────┘    └──────┬───────────────┘    │      │
│   │                             │                     │      │
│   └─────────────────────────────┼─────────────────────┘      │
│                                 │                             │
│              ┌──────────────────┼──────────────────┐         │
│              │                  │                  │         │
│        ┌─────▼─────┐    ┌──────▼─────┐    ┌──────▼─────┐   │
│        │ Handler A  │    │ Handler B  │    │  Acceptor   │   │
│        │ onRead()   │    │ onRead()   │    │ onAccept()  │   │
│        │ onWrite()  │    │ onWrite()  │    │             │   │
│        └────────────┘    └────────────┘    └────────────┘   │
│                                                              │
│   角色说明：                                                  │
│   - Reactor：负责事件循环和分发                                │
│   - Handler：处理具体的IO事件                                  │
│   - Acceptor：处理新连接，创建对应Handler                      │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 1.2 Reactor模式核心组件

```cpp
#include <iostream>
#include <functional>
#include <unordered_map>
#include <memory>

// Reactor模式核心接口定义

// 事件类型
enum class EventType {
    READ_EVENT = 0x01,
    WRITE_EVENT = 0x02,
    ERROR_EVENT = 0x04,
    ACCEPT_EVENT = 0x08
};

// 事件处理器接口
class Handler {
public:
    virtual ~Handler() = default;
    virtual void on_read() = 0;     // 可读事件处理
    virtual void on_write() = 0;    // 可写事件处理
    virtual void on_error() = 0;    // 错误事件处理
    virtual int get_fd() const = 0; // 获取关联的fd
};

// 事件对象
struct Event {
    int fd;                         // 文件描述符
    EventType type;                 // 事件类型
    std::shared_ptr<Handler> handler; // 事件处理器
};

// Reactor接口
class Reactor {
public:
    virtual ~Reactor() = default;
    virtual void register_handler(std::shared_ptr<Handler> handler,
                                   EventType type) = 0;
    virtual void remove_handler(std::shared_ptr<Handler> handler) = 0;
    virtual void modify_handler(std::shared_ptr<Handler> handler,
                                 EventType type) = 0;
    virtual void loop() = 0;       // 事件循环
    virtual void stop() = 0;       // 停止循环
};
```

---

## 2. 单Reactor单线程

### 2.1 架构图

```
┌──────────────────────────────────────────────────────────────┐
│                  单Reactor单线程                               │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│                    单线程                                     │
│   ┌──────────────────────────────────────────────────┐      │
│   │                                                    │      │
│   │   ┌──────────┐                                    │      │
│   │   │ Reactor  │ ← epoll_wait / select              │      │
│   │   │ 事件循环  │                                    │      │
│   │   └────┬─────┘                                    │      │
│   │        │ 分发事件                                   │      │
│   │        │                                           │      │
│   │   ┌────▼──────────────────────────────────┐       │      │
│   │   │                                        │       │      │
│   │   │  Acceptor    Handler1    Handler2      │       │      │
│   │   │  accept()    read()     read()        │       │      │
│   │   │              write()    write()       │       │      │
│   │   │              process()  process()     │       │      │
│   │   │                                        │       │      │
│   │   └────────────────────────────────────────┘       │      │
│   │                                                     │      │
│   └─────────────────────────────────────────────────────┘      │
│                                                              │
│   优点：简单，无线程安全问题                                   │
│   缺点：一个Handler阻塞会影响所有连接                           │
│   适用：轻量级服务（如Redis单线程模式）                         │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 2.2 代码实现

```cpp
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <unordered_map>
#include <functional>
#include <string>

// 单Reactor单线程实现
class SingleReactorSingleThread {
public:
    using Callback = std::function<void(int, uint32_t)>;

    SingleReactorSingleThread(int port)
        : port_(port), running_(false) {
        epfd_ = epoll_create1(0);
    }

    ~SingleReactorSingleThread() {
        if (epfd_ >= 0) close(epfd_);
        if (listenfd_ >= 0) close(listenfd_);
    }

    void start() {
        // 创建监听socket
        listenfd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        int opt = 1;
        setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = INADDR_ANY;

        bind(listenfd_, (sockaddr*)&addr, sizeof(addr));
        listen(listenfd_, 128);

        // 注册监听fd
        add_fd(listenfd_, EPOLLIN, [this](int fd, uint32_t events) {
            handle_accept();
        });

        running_ = true;
        std::cout << "单Reactor单线程服务器启动，端口: " << port_ << std::endl;

        // 事件循环（单线程）
        std::vector<epoll_event> events(1024);
        while (running_) {
            int nfds = epoll_wait(epfd_, events.data(), events.size(), -1);
            if (nfds < 0) {
                if (errno == EINTR) continue;
                break;
            }

            for (int i = 0; i < nfds; ++i) {
                auto it = callbacks_.find(events[i].data.fd);
                if (it != callbacks_.end()) {
                    it->second(events[i].data.fd, events[i].events);
                }
            }
        }
    }

    void stop() { running_ = false; }

private:
    void handle_accept() {
        sockaddr_in client_addr{};
        socklen_t len = sizeof(client_addr);
        int clientfd = accept4(listenfd_, (sockaddr*)&client_addr,
                                &len, SOCK_NONBLOCK);
        if (clientfd < 0) return;

        // 为新连接注册读写回调
        add_fd(clientfd, EPOLLIN | EPOLLRDHUP, [this](int fd, uint32_t events) {
            if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                // 连接关闭
                remove_fd(fd);
                close(fd);
                return;
            }
            handle_read(fd);
        });

        std::cout << "新连接: fd=" << clientfd << std::endl;
    }

    void handle_read(int fd) {
        char buffer[4096];
        ssize_t n = recv(fd, buffer, sizeof(buffer), 0);

        if (n <= 0) {
            remove_fd(fd);
            close(fd);
            return;
        }

        // 在同一线程中处理业务逻辑
        // ⚠️ 如果这里处理耗时，会阻塞整个事件循环！
        std::string response = process_request(buffer, n);
        send(fd, response.data(), response.size(), MSG_NOSIGNAL);
    }

    std::string process_request(const char* data, size_t len) {
        // 模拟业务处理
        return std::string(data, len);
    }

    void add_fd(int fd, uint32_t events, Callback cb) {
        epoll_event ev{};
        ev.events = events;
        ev.data.fd = fd;
        epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
        callbacks_[fd] = std::move(cb);
    }

    void remove_fd(int fd) {
        epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
        callbacks_.erase(fd);
    }

    int port_;
    int listenfd_;
    int epfd_;
    bool running_;
    std::unordered_map<int, Callback> callbacks_;
};

int main() {
    SingleReactorSingleThread server(8080);
    server.start();
    return 0;
}
```

---

## 3. 单Reactor多线程

### 3.1 架构图

```
┌──────────────────────────────────────────────────────────────┐
│                  单Reactor多线程                               │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│   Reactor线程                                                │
│   ┌──────────────────────────────────────────────────┐      │
│   │   ┌──────────┐                                    │      │
│   │   │ Reactor  │ ← epoll_wait                       │      │
│   │   │ 事件循环  │                                    │      │
│   │   └────┬─────┘                                    │      │
│   │        │                                           │      │
│   │   ┌────▼──────────────────────────┐               │      │
│   │   │  Acceptor    Handler1  Handler2│              │      │
│   │   │  accept()    read()   read()  │              │      │
│   │   │              ↓       ↓        │              │      │
│   │   │           提交到线程池          │              │      │
│   │   └───────────────────────────────┘               │      │
│   └─────────────────────────────────────────────────────┘      │
│                                                              │
│   工作线程池                                                  │
│   ┌──────────────────────────────────────────────────┐      │
│   │  Thread1      Thread2      Thread3                │      │
│   │  process()    process()    process()              │      │
│   │  write()      write()      write()                │      │
│   └──────────────────────────────────────────────────┘      │
│                                                              │
│   优点：业务处理不阻塞事件循环                                 │
│   缺点：Reactor单点，高并发下accept/read可能成瓶颈             │
│   适用：中等并发场景                                          │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 3.2 代码实现

```cpp
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <string>
#include <memory>

// 线程池
class ThreadPool {
public:
    ThreadPool(size_t num_threads) : stop_(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        cv_.wait(lock, [this] {
                            return stop_ || !tasks_.empty();
                        });
                        if (stop_ && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& w : workers_) w.join();
    }

    void enqueue(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            tasks_.push(std::move(task));
        }
        cv_.notify_one();
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_;
};

// 单Reactor多线程实现
class SingleReactorMultiThread {
public:
    SingleReactorMultiThread(int port, size_t num_workers = 4)
        : port_(port), running_(false), pool_(num_workers) {
        epfd_ = epoll_create1(0);
    }

    ~SingleReactorMultiThread() {
        if (epfd_ >= 0) close(epfd_);
        if (listenfd_ >= 0) close(listenfd_);
    }

    void start() {
        listenfd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        int opt = 1;
        setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = INADDR_ANY;

        bind(listenfd_, (sockaddr*)&addr, sizeof(addr));
        listen(listenfd_, 128);

        add_fd(listenfd_, EPOLLIN, [this](int fd, uint32_t) {
            handle_accept();
        });

        running_ = true;
        std::cout << "单Reactor多线程服务器启动，端口: " << port_ << std::endl;

        std::vector<epoll_event> events(1024);
        while (running_) {
            int nfds = epoll_wait(epfd_, events.data(), events.size(), -1);
            if (nfds < 0) {
                if (errno == EINTR) continue;
                break;
            }

            for (int i = 0; i < nfds; ++i) {
                auto it = callbacks_.find(events[i].data.fd);
                if (it != callbacks_.end()) {
                    it->second(events[i].data.fd, events[i].events);
                }
            }
        }
    }

private:
    void handle_accept() {
        while (true) {
            int clientfd = accept4(listenfd_, nullptr, nullptr,
                                    SOCK_NONBLOCK);
            if (clientfd < 0) break;

            add_fd(clientfd, EPOLLIN | EPOLLRDHUP, [this](int fd, uint32_t ev) {
                if (ev & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                    remove_fd(fd);
                    close(fd);
                    return;
                }
                handle_read(fd);
            });
        }
    }

    void handle_read(int fd) {
        char buffer[4096];
        ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
        if (n <= 0) {
            remove_fd(fd);
            close(fd);
            return;
        }

        // 将业务处理提交到线程池
        // ⚠️ 注意：写操作可能需要在Reactor线程完成
        std::string data(buffer, n);
        pool_.enqueue([this, fd, data = std::move(data)]() {
            // 在工作线程中处理业务
            std::string response = process_request(data);

            // 在工作线程中直接发送（简化版）
            // 生产环境应通过管道通知Reactor线程写入
            send(fd, response.data(), response.size(), MSG_NOSIGNAL);
        });
    }

    std::string process_request(const std::string& data) {
        // 模拟耗时业务处理
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        return data;
    }

    void add_fd(int fd, uint32_t events, Callback cb) {
        epoll_event ev{};
        ev.events = events;
        ev.data.fd = fd;
        epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
        callbacks_[fd] = std::move(cb);
    }

    void remove_fd(int fd) {
        epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
        callbacks_.erase(fd);
    }

    using Callback = std::function<void(int, uint32_t)>;
    int port_;
    int listenfd_;
    int epfd_;
    bool running_;
    ThreadPool pool_;
    std::unordered_map<int, Callback> callbacks_;
};

int main() {
    SingleReactorMultiThread server(8080, 4);
    server.start();
    return 0;
}
```

---

## 4. 主从Reactor多线程

### 4.1 架构图

```
┌──────────────────────────────────────────────────────────────┐
│                  主从Reactor多线程                             │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│   主Reactor（1个线程）                                        │
│   ┌──────────────────────────────────────────────────┐      │
│   │   ┌──────────┐                                    │      │
│   │   │ Main     │ ← 只负责accept                     │      │
│   │   │ Reactor  │                                    │      │
│   │   └────┬─────┘                                    │      │
│   │        │ accept新连接                               │      │
│   │        │ 分配给Sub Reactor                          │      │
│   └────────┼───────────────────────────────────────────┘      │
│            │                                                  │
│     ┌──────┼──────────────────────┐                          │
│     │      │                      │                          │
│     ▼      ▼                      ▼                          │
│   从Reactor1  从Reactor2  ...  从ReactorN                    │
│   ┌──────────┐ ┌──────────┐    ┌──────────┐                │
│   │Sub       │ │Sub       │    │Sub       │                │
│   │Reactor1  │ │Reactor2  │    │ReactorN  │                │
│   │epoll_wait│ │epoll_wait│    │epoll_wait│                │
│   │read/write│ │read/write│    │read/write│                │
│   │  ↓       │ │  ↓       │    │  ↓       │                │
│   │提交到     │ │提交到     │    │提交到     │                │
│   │线程池    │ │线程池    │    │线程池    │                │
│   └──────────┘ └──────────┘    └──────────┘                │
│                                                              │
│   工作线程池（共享）                                          │
│   ┌──────────────────────────────────────────────────┐      │
│   │  Thread1  Thread2  Thread3  ...  ThreadN          │      │
│   │  业务处理（process）                               │      │
│   └──────────────────────────────────────────────────┘      │
│                                                              │
│   优点：充分利用多核，accept和IO处理分离                       │
│   缺点：实现复杂                                              │
│   适用：高并发生产环境（Nginx、Netty、muduo）                  │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 4.2 代码实现

```cpp
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <unordered_map>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <string>
#include <memory>
#include <atomic>

// 线程池（复用之前的实现）
class ThreadPool {
public:
    ThreadPool(size_t num_threads) : stop_(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        cv_.wait(lock, [this] {
                            return stop_ || !tasks_.empty();
                        });
                        if (stop_ && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& w : workers_) w.join();
    }

    void enqueue(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(mutex_);
            tasks_.push(std::move(task));
        }
        cv_.notify_one();
    }

private:
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_;
};

// 从Reactor——负责IO读写
class SubReactor {
public:
    using Callback = std::function<void(int, uint32_t)>;

    SubReactor() : running_(false) {
        epfd_ = epoll_create1(0);
    }

    ~SubReactor() {
        stop();
        if (epfd_ >= 0) close(epfd_);
    }

    // 在独立线程中运行事件循环
    void start() {
        running_ = true;
        thread_ = std::thread([this] { event_loop(); });
    }

    void stop() {
        running_ = false;
        if (thread_.joinable()) thread_.join();
    }

    // 添加新连接（由主Reactor调用，线程安全）
    void add_connection(int fd) {
        {
            std::lock_guard<std::mutex> lock(pending_mutex_);
            pending_fds_.push(fd);
        }
        // 唤醒从Reactor（通过eventfd或管道）
        // 简化版：直接添加
        do_add_connection(fd);
    }

    void set_thread_pool(ThreadPool* pool) { pool_ = pool; }

private:
    void event_loop() {
        std::vector<epoll_event> events(1024);

        while (running_) {
            int nfds = epoll_wait(epfd_, events.data(), events.size(), 100);
            if (nfds < 0) {
                if (errno == EINTR) continue;
                break;
            }

            // 先处理待添加的连接
            process_pending();

            for (int i = 0; i < nfds; ++i) {
                auto it = callbacks_.find(events[i].data.fd);
                if (it != callbacks_.end()) {
                    it->second(events[i].data.fd, events[i].events);
                }
            }
        }
    }

    void process_pending() {
        std::lock_guard<std::mutex> lock(pending_mutex_);
        while (!pending_fds_.empty()) {
            do_add_connection(pending_fds_.front());
            pending_fds_.pop();
        }
    }

    void do_add_connection(int fd) {
        epoll_event ev{};
        ev.events = EPOLLIN | EPOLLET | EPOLLRDHUP;
        ev.data.fd = fd;
        epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);

        callbacks_[fd] = [this](int fd, uint32_t events) {
            if (events & (EPOLLERR | EPOLLHUP | EPOLLRDHUP)) {
                epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
                close(fd);
                callbacks_.erase(fd);
                return;
            }
            handle_read(fd);
        };
    }

    void handle_read(int fd) {
        char buffer[4096];
        while (true) {
            ssize_t n = recv(fd, buffer, sizeof(buffer), 0);
            if (n > 0) {
                std::string data(buffer, n);
                if (pool_) {
                    pool_->enqueue([fd, data = std::move(data)]() {
                        std::string response = data;  // 业务处理
                        send(fd, response.data(), response.size(),
                             MSG_NOSIGNAL);
                    });
                }
            } else if (n == 0) {
                epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
                close(fd);
                callbacks_.erase(fd);
                break;
            } else {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
                close(fd);
                callbacks_.erase(fd);
                break;
            }
        }
    }

    int epfd_;
    bool running_;
    std::thread thread_;
    std::unordered_map<int, Callback> callbacks_;
    std::queue<int> pending_fds_;
    std::mutex pending_mutex_;
    ThreadPool* pool_ = nullptr;
};

// 主Reactor——负责accept
class MainReactor {
public:
    MainReactor(int port, size_t num_sub_reactors = 4,
                size_t num_workers = 8)
        : port_(port), running_(false), next_sub_(0),
          pool_(num_workers) {

        // 创建从Reactor
        for (size_t i = 0; i < num_sub_reactors; ++i) {
            auto sub = std::make_unique<SubReactor>();
            sub->set_thread_pool(&pool_);
            sub->start();
            sub_reactors_.push_back(std::move(sub));
        }
    }

    ~MainReactor() {
        stop();
        sub_reactors_.clear();
        if (epfd_ >= 0) close(epfd_);
        if (listenfd_ >= 0) close(listenfd_);
    }

    void start() {
        // 创建监听socket
        listenfd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        int opt = 1;
        setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = INADDR_ANY;

        bind(listenfd_, (sockaddr*)&addr, sizeof(addr));
        listen(listenfd_, 128);

        // 注册监听fd到主Reactor的epoll
        epfd_ = epoll_create1(0);
        epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = listenfd_;
        epoll_ctl(epfd_, EPOLL_CTL_ADD, listenfd_, &ev);

        running_ = true;
        std::cout << "主从Reactor服务器启动，端口: " << port_
                  << ", 从Reactor数: " << sub_reactors_.size() << std::endl;

        // 主Reactor事件循环
        std::vector<epoll_event> events(64);
        while (running_) {
            int nfds = epoll_wait(epfd_, events.data(), events.size(), -1);
            if (nfds < 0) {
                if (errno == EINTR) continue;
                break;
            }

            for (int i = 0; i < nfds; ++i) {
                if (events[i].data.fd == listenfd_) {
                    handle_accept();
                }
            }
        }
    }

    void stop() {
        running_ = false;
        for (auto& sub : sub_reactors_) {
            sub->stop();
        }
    }

private:
    void handle_accept() {
        while (true) {
            int clientfd = accept4(listenfd_, nullptr, nullptr,
                                    SOCK_NONBLOCK);
            if (clientfd < 0) break;

            // 轮询分配给从Reactor
            size_t idx = next_sub_++ % sub_reactors_.size();
            sub_reactors_[idx]->add_connection(clientfd);
        }
    }

    int port_;
    int listenfd_;
    int epfd_;
    bool running_;
    std::atomic<size_t> next_sub_;
    ThreadPool pool_;
    std::vector<std::unique_ptr<SubReactor>> sub_reactors_;
};

int main() {
    MainReactor server(8080, 4, 8);
    server.start();
    return 0;
}
```

### 4.3 三种Reactor模式对比

| 特性 | 单Reactor单线程 | 单Reactor多线程 | 主从Reactor多线程 |
|------|---------------|---------------|-----------------|
| **Reactor数** | 1 | 1 | N |
| **线程数** | 1 | 1+N | 1+N+M |
| **accept** | 同一线程 | Reactor线程 | 主Reactor线程 |
| **IO处理** | 同一线程 | Reactor线程 | 从Reactor线程 |
| **业务处理** | 同一线程 | 工作线程 | 工作线程 |
| **性能** | 低 | 中 | 高 |
| **复杂度** | 简单 | 中等 | 复杂 |
| **典型应用** | Redis | - | Nginx/Netty/muduo |

---

## 5. Reactor模式代码实现

### 5.1 完整的Reactor框架

```cpp
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <functional>
#include <unordered_map>
#include <vector>
#include <string>
#include <memory>
#include <atomic>

// ==================== 事件循环 ====================
class EventLoop {
public:
    using Functor = std::function<void()>;

    EventLoop() : epfd_(epoll_create1(0)), running_(false) {}

    ~EventLoop() {
        if (epfd_ >= 0) close(epfd_);
    }

    // 运行事件循环
    void loop() {
        running_ = true;
        std::vector<epoll_event> events(1024);

        while (running_) {
            int nfds = epoll_wait(epfd_, events.data(), events.size(), -1);
            if (nfds < 0) {
                if (errno == EINTR) continue;
                break;
            }

            for (int i = 0; i < nfds; ++i) {
                auto* channel = static_cast<Channel*>(events[i].data.ptr);
                channel->handle_event(events[i].events);
            }
        }
    }

    void stop() { running_ = false; }

    // 更新Channel的epoll注册
    void update_channel(Channel* channel);
    void remove_channel(Channel* channel);

    int epfd() const { return epfd_; }

private:
    int epfd_;
    std::atomic<bool> running_;
};

// ==================== Channel（事件分发） ====================
class Channel {
public:
    using EventCallback = std::function<void()>;

    Channel(EventLoop* loop, int fd)
        : loop_(loop), fd_(fd), events_(0), revents_(0) {}

    // 处理事件
    void handle_event(uint32_t revents) {
        revents_ = revents;

        if (revents & (EPOLLERR | EPOLLHUP)) {
            if (error_callback_) error_callback_();
        }
        if (revents & EPOLLIN) {
            if (read_callback_) read_callback_();
        }
        if (revents & EPOLLOUT) {
            if (write_callback_) write_callback_();
        }
        if (revents & EPOLLRDHUP) {
            if (close_callback_) close_callback_();
        }
    }

    // 设置回调
    void set_read_callback(EventCallback cb) { read_callback_ = std::move(cb); }
    void set_write_callback(EventCallback cb) { write_callback_ = std::move(cb); }
    void set_close_callback(EventCallback cb) { close_callback_ = std::move(cb); }
    void set_error_callback(EventCallback cb) { error_callback_ = std::move(cb); }

    // 启用/禁用事件
    void enable_reading() {
        events_ |= EPOLLIN;
        update();
    }
    void enable_writing() {
        events_ |= EPOLLOUT;
        update();
    }
    void disable_writing() {
        events_ &= ~EPOLLOUT;
        update();
    }
    void disable_all() {
        events_ = 0;
        update();
    }

    int fd() const { return fd_; }
    uint32_t events() const { return events_; }

    // 为epoll_event设置data
    epoll_event get_epoll_event() const {
        epoll_event ev{};
        ev.events = events_;
        ev.data.ptr = const_cast<Channel*>(this);
        return ev;
    }

private:
    void update() { loop_->update_channel(this); }

    EventLoop* loop_;
    int fd_;
    uint32_t events_;   // 关注的事件
    uint32_t revents_;  // 返回的事件
    EventCallback read_callback_;
    EventCallback write_callback_;
    EventCallback close_callback_;
    EventCallback error_callback_;
};

// EventLoop方法实现
void EventLoop::update_channel(Channel* channel) {
    epoll_event ev = channel->get_epoll_event();
    int fd = channel->fd();

    // 尝试修改，失败则添加
    if (epoll_ctl(epfd_, EPOLL_CTL_MOD, fd, &ev) < 0) {
        epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);
    }
}

void EventLoop::remove_channel(Channel* channel) {
    epoll_ctl(epfd_, EPOLL_CTL_DEL, channel->fd(), nullptr);
}

// ==================== TCP连接 ====================
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    using MessageCallback = std::function<void(std::shared_ptr<TcpConnection>,
                                                const std::string&)>;
    using CloseCallback = std::function<void(std::shared_ptr<TcpConnection>)>;

    TcpConnection(EventLoop* loop, int fd)
        : loop_(loop), channel_(loop, fd), fd_(fd) {
        // 设置Channel回调
        channel_.set_read_callback([this] { handle_read(); });
        channel_.set_close_callback([this] { handle_close(); });
        channel_.set_error_callback([this] { handle_error(); });
    }

    ~TcpConnection() {
        if (fd_ >= 0) close(fd_);
    }

    void established() {
        channel_.enable_reading();
    }

    void send(const std::string& message) {
        send(fd_, message.data(), message.size(), MSG_NOSIGNAL);
    }

    void set_message_callback(MessageCallback cb) {
        message_callback_ = std::move(cb);
    }
    void set_close_callback(CloseCallback cb) {
        close_callback_ = std::move(cb);
    }

    int fd() const { return fd_; }

private:
    void handle_read() {
        char buffer[65536];
        ssize_t n = recv(fd_, buffer, sizeof(buffer), 0);
        if (n > 0) {
            if (message_callback_) {
                message_callback_(shared_from_this(),
                                   std::string(buffer, n));
            }
        } else if (n == 0) {
            handle_close();
        } else {
            handle_error();
        }
    }

    void handle_close() {
        channel_.disable_all();
        if (close_callback_) {
            close_callback_(shared_from_this());
        }
    }

    void handle_error() {
        handle_close();
    }

    EventLoop* loop_;
    Channel channel_;
    int fd_;
    std::string input_buffer_;
    std::string output_buffer_;
    MessageCallback message_callback_;
    CloseCallback close_callback_;
};

// ==================== TCP服务器 ====================
class TcpServer {
public:
    using ConnectionCallback = std::function<void(std::shared_ptr<TcpConnection>)>;
    using MessageCallback = std::function<void(std::shared_ptr<TcpConnection>,
                                                const std::string&)>;

    TcpServer(EventLoop* loop, int port)
        : loop_(loop), port_(port) {}

    void start() {
        // 创建监听socket
        listenfd_ = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        int opt = 1;
        setsockopt(listenfd_, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = INADDR_ANY;

        bind(listenfd_, (sockaddr*)&addr, sizeof(addr));
        listen(listenfd_, 128);

        // 创建accept Channel
        accept_channel_ = std::make_unique<Channel>(loop_, listenfd_);
        accept_channel_->set_read_callback([this] { handle_accept(); });
        accept_channel_->enable_reading();

        std::cout << "Reactor TCP服务器启动，端口: " << port_ << std::endl;
    }

    void set_connection_callback(ConnectionCallback cb) {
        connection_callback_ = std::move(cb);
    }
    void set_message_callback(MessageCallback cb) {
        message_callback_ = std::move(cb);
    }

private:
    void handle_accept() {
        while (true) {
            int clientfd = accept4(listenfd_, nullptr, nullptr,
                                    SOCK_NONBLOCK);
            if (clientfd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                break;
            }

            auto conn = std::make_shared<TcpConnection>(loop_, clientfd);
            conn->set_message_callback(message_callback_);
            conn->set_close_callback([this](std::shared_ptr<TcpConnection> c) {
                connections_.erase(c->fd());
            });

            connections_[clientfd] = conn;
            conn->established();

            if (connection_callback_) {
                connection_callback_(conn);
            }
        }
    }

    EventLoop* loop_;
    int port_;
    int listenfd_;
    std::unique_ptr<Channel> accept_channel_;
    std::unordered_map<int, std::shared_ptr<TcpConnection>> connections_;
    ConnectionCallback connection_callback_;
    MessageCallback message_callback_;
};

// ==================== 使用示例 ====================
int main() {
    EventLoop loop;
    TcpServer server(&loop, 8080);

    // 设置回调
    server.set_connection_callback([](std::shared_ptr<TcpConnection> conn) {
        std::cout << "新连接: fd=" << conn->fd() << std::endl;
    });

    server.set_message_callback([](std::shared_ptr<TcpConnection> conn,
                                    const std::string& msg) {
        // 回显
        conn->send(msg);
    });

    server.start();
    loop.loop();

    return 0;
}
```

---

## 6. Netty与muduo的Reactor设计

### 6.1 muduo的Reactor设计

```
┌──────────────────────────────────────────────────────────────┐
│                    muduo 架构                                 │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│   muduo核心组件：                                            │
│   ┌──────────────────────────────────────────────┐          │
│   │  EventLoop       —— 事件循环（one loop per   │          │
│   │                     thread）                  │          │
│   │  Channel         —— 事件分发                  │          │
│   │  Poller          —— IO多路复用抽象            │          │
│   │  EventLoopThread —— IO线程                    │          │
│   │  EventLoopThreadPool —— IO线程池              │          │
│   │  TcpServer       —— TCP服务器                 │          │
│   │  TcpConnection   —— TCP连接                   │          │
│   │  Acceptor        —— 连接接收器                │          │
│   │  Buffer          —— 自动增长的缓冲区          │          │
│   └──────────────────────────────────────────────┘          │
│                                                              │
│   muduo的Reactor模式：                                       │
│   ┌──────────────────────────────────────────────┐          │
│   │  主Reactor（mainLoop）                        │          │
│   │  ├── Acceptor（accept新连接）                 │          │
│   │  └── 分配连接给subLoop                        │          │
│   │                                               │          │
│   │  从Reactor（subLoop1, subLoop2, ...）         │          │
│   │  ├── TcpConnection（管理连接生命周期）         │          │
│   │  ├── Channel（事件分发）                      │          │
│   │  └── Buffer（读写缓冲区）                     │          │
│   └──────────────────────────────────────────────┘          │
│                                                              │
│   设计特点：                                                  │
│   ✅ One Loop Per Thread —— 每个线程一个事件循环              │
│   ✅ 线程安全 —— 回调在所属EventLoop中执行                    │
│   ✅ LT模式 —— 编程更安全，不会漏事件                         │
│   ✅ 自动缓冲区管理 —— 应用层无需关心读写细节                  │
│   ✅ 跨平台 —— 支持epoll/poll/Windows IOCP                   │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 6.2 Netty的Reactor设计

```
┌──────────────────────────────────────────────────────────────┐
│                    Netty 架构（Java）                          │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│   Netty的Reactor模式：                                       │
│   ┌──────────────────────────────────────────────┐          │
│   │  BossGroup（主Reactor线程组）                  │          │
│   │  ├── NioEventLoop1（accept）                  │          │
│   │  └── NioEventLoop2（accept）                  │          │
│   │                                               │          │
│   │  WorkerGroup（从Reactor线程组）                │          │
│   │  ├── NioEventLoop1（read/write）              │          │
│   │  ├── NioEventLoop2（read/write）              │          │
│   │  ├── ...                                     │          │
│   │  └── NioEventLoopN（read/write）              │          │
│   └──────────────────────────────────────────────┘          │
│                                                              │
│   Netty vs muduo：                                           │
│   ┌──────────────┬──────────────┬──────────────────┐       │
│   │ 特性         │ muduo        │ Netty             │       │
│   ├──────────────┼──────────────┼──────────────────┤       │
│   │ 语言         │ C++          │ Java              │       │
│   │ 主Reactor    │ 1个线程      │ 线程组(BossGroup) │       │
│   │ 从Reactor    │ 线程池       │ 线程组(WorkerGrp) │       │
│   │ IO模型       │ epoll(LT)    │ epoll/poll/select │       │
│   │ Pipeline     │ 无           │ ChannelPipeline   │       │
│   │ 内存管理     │ RAII         │ ByteBuf(引用计数) │       │
│   └──────────────┴──────────────┴──────────────────┘       │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 6.3 muduo风格框架核心代码

```cpp
#include <iostream>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <cerrno>
#include <cstring>
#include <functional>
#include <vector>
#include <string>
#include <memory>
#include <thread>
#include <atomic>

// muduo风格：One Loop Per Thread

// EventLoop —— 事件循环
class EventLoop {
public:
    EventLoop() : epfd_(epoll_create1(0)), running_(false),
                  thread_id_(std::this_thread::get_id()) {}

    ~EventLoop() {
        if (epfd_ >= 0) close(epfd_);
    }

    // 必须在创建线程中调用
    void loop() {
        assert_in_loop_thread();
        running_ = true;
        std::vector<epoll_event> events(1024);

        while (running_) {
            int nfds = epoll_wait(epfd_, events.data(), events.size(), -1);
            if (nfds < 0) {
                if (errno == EINTR) continue;
                break;
            }

            for (int i = 0; i < nfds; ++i) {
                // 处理活跃事件
                // 实际muduo中通过Channel映射
            }
        }
    }

    void stop() { running_ = false; }

    // 断言在循环线程中执行
    void assert_in_loop_thread() {
        if (!is_in_loop_thread()) {
            std::cerr << "EventLoop不在创建线程中！" << std::endl;
            std::abort();
        }
    }

    bool is_in_loop_thread() const {
        return thread_id_ == std::this_thread::get_id();
    }

    int epfd() const { return epfd_; }

private:
    int epfd_;
    std::atomic<bool> running_;
    std::thread::id thread_id_;
};

// EventLoopThread —— IO线程
class EventLoopThread {
public:
    EventLoopThread() : loop_(nullptr), running_(false) {}

    ~EventLoopThread() {
        if (loop_) loop_->stop();
        if (thread_.joinable()) thread_.join();
    }

    EventLoop* start_loop() {
        running_ = true;
        thread_ = std::thread([this] {
            EventLoop loop;
            loop_ = &loop;
            loop.loop();
        });
        // 等待loop创建完成
        while (loop_ == nullptr) {
            std::this_thread::yield();
        }
        return loop_;
    }

private:
    EventLoop* loop_;
    std::atomic<bool> running_;
    std::thread thread_;
};

// EventLoopThreadPool —— IO线程池
class EventLoopThreadPool {
public:
    EventLoopThreadPool(EventLoop* base_loop, size_t num_threads)
        : base_loop_(base_loop), num_threads_(num_threads), next_(0) {}

    void start() {
        for (size_t i = 0; i < num_threads_; ++i) {
            auto thread = std::make_unique<EventLoopThread>();
            loops_.push_back(thread->start_loop());
            threads_.push_back(std::move(thread));
        }
    }

    // 获取下一个EventLoop（轮询）
    EventLoop* get_next_loop() {
        if (loops_.empty()) return base_loop_;
        return loops_[next_++ % loops_.size()];
    }

private:
    EventLoop* base_loop_;  // 主Reactor的EventLoop
    size_t num_threads_;
    std::atomic<size_t> next_;
    std::vector<std::unique_ptr<EventLoopThread>> threads_;
    std::vector<EventLoop*> loops_;
};

// 使用示例
int main() {
    EventLoop main_loop;  // 主Reactor

    // 创建IO线程池（从Reactor）
    EventLoopThreadPool pool(&main_loop, 4);
    pool.start();

    std::cout << "muduo风格Reactor框架启动" << std::endl;
    std::cout << "主Reactor: 1, 从Reactor: 4" << std::endl;

    // 主Reactor负责accept
    main_loop.loop();

    return 0;
}
```

---

## 7. 小结

### 核心要点回顾

| 要点 | 说明 |
|------|------|
| **Reactor模式** | 事件驱动的并发模式，将事件检测与处理分离 |
| **单Reactor单线程** | 最简单，适合轻量级服务（如Redis） |
| **单Reactor多线程** | 业务处理不阻塞IO，但Reactor是瓶颈 |
| **主从Reactor** | accept和IO分离，充分利用多核 |
| **Channel** | 封装fd和事件回调，是事件分发的核心 |
| **One Loop Per Thread** | muduo的核心设计，线程安全有保障 |
| **muduo** | C++ Reactor框架的典范，LT模式 |
| **Netty** | Java Reactor框架，Boss+Worker线程组 |

### 设计选择建议

| 场景 | 推荐模式 | 原因 |
|------|---------|------|
| **学习/原型** | 单Reactor单线程 | 简单易懂 |
| **少量连接** | 单Reactor单线程 | 性能足够 |
| **中等并发** | 单Reactor多线程 | 业务不阻塞IO |
| **高并发生产** | 主从Reactor多线程 | 充分利用多核 |
| **极致性能** | 主从Reactor + io_uring | 最新技术栈 |

### 下一步

- 学习Proactor模式（第03章），了解异步IO的架构设计
- 学习零拷贝技术（第04章），进一步优化数据传输
