# Proactor模式与异步IO

> 掌握异步IO编程模型与Proactor设计模式

---

> **The Proactor pattern decouples initiation from completion.**
> （Proactor模式将IO发起与IO完成解耦。）

> **Proactor：你只管发起，完成了我通知你。**

---

> 🎯 **异步之道，不待而成。**

> （真正的异步IO，让线程不再等待。）

---

> 💡 **通俗理解 - Proactor模式**

想象去干洗店洗衣服：
- **Reactor**：衣服洗好了通知你，你自己去取 → 还需要你动手
- **Proactor**：你把衣服放下，洗好烘干叠好通知你来拿 → 全程不用你管

**Proactor模式就是"干洗店模式"——你只管发起IO请求，内核完成整个操作后通知你！**

> 🔬 **抽象理解 - Proactor模式**：
> - **Proactor**：是"异步完成分发器"，负责等待IO完成并分发完成事件
> - **AsyncOperation**：是"异步操作"，由内核执行，应用不参与等待
> - **CompletionHandler**：是"完成处理器"，IO完成后被回调
> - **完成队列**：存储已完成的IO操作，Proactor从中取出并分发
> - **Proactor的本质**：将"发起IO"和"IO完成"完全分离，线程永不阻塞

---

## 前置知识
- [Reactor模式](02-Reactor模式.md)
- [IO多路复用深入](01-IO多路复用深入.md)
## 后续内容
- [零拷贝与高效数据传输](04-零拷贝与高效数据传输.md)

## 目录

- [1. Proactor模式原理](#1-proactor模式原理)
- [2. Reactor vs Proactor对比](#2-reactor-vs-proactor对比)
- [3. Windows IOCP实现](#3-windows-iocp实现)
- [4. Linux AIO与io_uring](#4-linux-aio与io_uring)
- [5. Boost.Asio的Proactor设计](#5-boostasio的proactor设计)
- [6. 小结](#6-小结)

---

## 1. Proactor模式原理

### 1.1 Proactor模式定义

Proactor模式是一种异步事件驱动的并发模式，核心思想是：**应用发起IO操作后立即返回，内核完成整个IO操作后通知应用**。

```
┌──────────────────────────────────────────────────────────────┐
│                    Proactor模式结构                            │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│   步骤1：应用发起异步IO请求                                    │
│   ┌──────────┐         ┌──────────┐                         │
│   │ 应用线程  │ ──────▶ │  内核     │                         │
│   │ async_   │  发起    │ 执行IO   │                         │
│   │ read()   │  请求    │          │                         │
│   │ 立即返回  │ ◀────── │ 记录请求 │                         │
│   └──────────┘         └──────────┘                         │
│                                                              │
│   步骤2：应用继续执行其他任务                                  │
│   ┌──────────┐         ┌──────────┐                         │
│   │ 应用线程  │         │  内核     │                         │
│   │ 执行其他  │         │ 等待数据 │                         │
│   │ 任务     │         │ 到达     │                         │
│   └──────────┘         └──────────┘                         │
│                                                              │
│   步骤3：内核完成IO后通知应用                                  │
│   ┌──────────┐         ┌──────────┐                         │
│   │ 应用线程  │ ◀────── │  内核     │                         │
│   │ 处理完成  │  完成    │ 拷贝数据 │                         │
│   │ 事件     │  通知    │ 到用户   │                         │
│   │          │         │ 缓冲区   │                         │
│   └──────────┘         └──────────┘                         │
│                                                              │
│   Proactor核心组件：                                          │
│   ┌──────────────────────────────────────────────┐          │
│   │  Proactor          —— 完成事件分发器          │          │
│   │  AsyncOperation    —— 异步操作（read/write）  │          │
│   │  CompletionHandler —— 完成回调处理器          │          │
│   │  CompletionQueue   —— 完成事件队列            │          │
│   └──────────────────────────────────────────────┘          │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 1.2 Proactor模式核心接口

```cpp
#include <iostream>
#include <functional>
#include <memory>
#include <vector>

// 异步操作结果
template<typename T>
class AsyncResult {
public:
    AsyncResult(bool success, T data, size_t bytes, int error = 0)
        : success_(success), data_(std::move(data)),
          bytes_transferred_(bytes), error_(error) {}

    bool success() const { return success_; }
    const T& data() const { return data_; }
    size_t bytes_transferred() const { return bytes_transferred_; }
    int error() const { return error_; }

private:
    bool success_;
    T data_;
    size_t bytes_transferred_;
    int error_;
};

// 完成处理器接口
template<typename T>
class CompletionHandler {
public:
    virtual ~CompletionHandler() = default;
    virtual void on_completion(const AsyncResult<T>& result) = 0;
};

// 异步操作
class AsyncOperation {
public:
    enum class Type { READ, WRITE, ACCEPT, CONNECT };

    Type type;
    int fd;
    void* buffer;
    size_t buffer_size;
    std::function<void(bool, size_t)> callback;
};

// Proactor接口
class Proactor {
public:
    virtual ~Proactor() = default;

    // 发起异步读操作
    virtual void async_read(int fd, void* buffer, size_t size,
                            std::function<void(bool, size_t)> callback) = 0;

    // 发起异步写操作
    virtual void async_write(int fd, const void* buffer, size_t size,
                             std::function<void(bool, size_t)> callback) = 0;

    // 发起异步accept
    virtual void async_accept(int listenfd,
                              std::function<void(bool, int)> callback) = 0;

    // 运行完成事件循环
    virtual void run() = 0;

    // 停止
    virtual void stop() = 0;
};
```

---

## 2. Reactor vs Proactor对比

### 2.1 核心区别

```
┌──────────────────────────────────────────────────────────────┐
│                Reactor vs Proactor 核心区别                    │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│   Reactor（同步非阻塞）：                                     │
│   ┌──────────────────────────────────────────────┐          │
│   │  1. epoll_wait → 通知fd可读                   │          │
│   │  2. 应用调用recv() → 应用自己读数据            │          │
│   │  3. 应用处理数据                               │          │
│   │                                               │          │
│   │  关键：内核只通知"可以读了"，应用自己执行读      │          │
│   └──────────────────────────────────────────────┘          │
│                                                              │
│   Proactor（异步）：                                          │
│   ┌──────────────────────────────────────────────┐          │
│   │  1. 应用发起async_read() → 立即返回            │          │
│   │  2. 内核等待数据 → 内核拷贝数据到用户缓冲区     │          │
│   │  3. 内核通知"读完成了" → 应用处理数据           │          │
│   │                                               │          │
│   │  关键：内核完成整个IO操作，应用只处理结果        │          │
│   └──────────────────────────────────────────────┘          │
│                                                              │
│   数据拷贝对比：                                              │
│   Reactor:  内核缓冲区 ──▶ 应用调用recv ──▶ 用户缓冲区       │
│   Proactor: 内核缓冲区 ──▶ 内核自动拷贝 ──▶ 用户缓冲区       │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 2.2 详细对比

| 特性 | Reactor | Proactor |
|------|---------|----------|
| **IO模型** | 同步非阻塞 | 真正异步 |
| **通知时机** | IO可操作时 | IO完成时 |
| **数据拷贝** | 应用执行 | 内核执行 |
| **编程模型** | 事件驱动 | 完成驱动 |
| **平台支持** | Linux(epoll) | Windows(IOCP) |
| **Linux支持** | 原生支持 | io_uring(5.1+) |
| **复杂度** | 中等 | 较高 |
| **性能** | 高 | 更高 |
| **典型框架** | muduo/Nginx | IOCP/Boost.Asio |

### 2.3 用代码理解区别

```cpp
#include <iostream>
#include <string>
#include <functional>

// Reactor风格——应用自己读数据
void reactor_style(int fd) {
    // 1. epoll通知fd可读
    // 2. 应用自己调用recv读取数据
    char buffer[4096];
    ssize_t n = recv(fd, buffer, sizeof(buffer), 0);  // 应用执行读

    if (n > 0) {
        // 3. 应用处理数据
        std::cout << "Reactor: 读取了 " << n << " 字节" << std::endl;
    }
}

// Proactor风格——内核读好数据后通知应用
void proactor_style(int fd) {
    char buffer[4096];

    // 1. 发起异步读请求
    async_read(fd, buffer, sizeof(buffer), [](bool success, size_t bytes) {
        // 3. 内核完成读取后回调——数据已在buffer中
        if (success) {
            std::cout << "Proactor: 读取了 " << bytes << " 字节" << std::endl;
        }
    });

    // 2. 应用可以继续做其他事情
    do_other_work();
}

// 模拟异步读（简化版）
void async_read(int fd, void* buffer, size_t size,
                std::function<void(bool, size_t)> callback) {
    // 实际由内核完成，这里简化模拟
    // 在真实实现中，这是由IOCP或io_uring完成的
    ssize_t n = recv(fd, buffer, size, 0);
    callback(n > 0, n > 0 ? n : 0);
}

void do_other_work() {
    // 应用在IO进行时可以做其他工作
}
```

### 2.4 Reactor模拟Proactor

在Linux上，由于原生AIO对网络IO支持有限，常用Reactor模拟Proactor：

```cpp
#include <iostream>
#include <sys/epoll.h>
#include <functional>
#include <unordered_map>
#include <vector>
#include <cstring>

// 用Reactor模拟Proactor
class ReactorAsProactor {
public:
    using CompletionCallback = std::function<void(bool, size_t)>;

    ReactorAsProactor() : epfd_(epoll_create1(0)) {}

    // 异步读——Proactor风格接口
    void async_read(int fd, void* buffer, size_t size,
                    CompletionCallback callback) {
        // 1. 保存异步操作信息
        AsyncReadOp op;
        op.fd = fd;
        op.buffer = buffer;
        op.size = size;
        op.callback = std::move(callback);

        // 2. 注册可读事件
        epoll_event ev{};
        ev.events = EPOLLIN;
        ev.data.fd = fd;
        epoll_ctl(epfd_, EPOLL_CTL_ADD, fd, &ev);

        pending_reads_[fd] = std::move(op);
    }

    // 事件循环
    void run() {
        std::vector<epoll_event> events(1024);

        while (true) {
            int nfds = epoll_wait(epfd_, events.data(), events.size(), -1);

            for (int i = 0; i < nfds; ++i) {
                int fd = events[i].data.fd;
                auto it = pending_reads_.find(fd);
                if (it != pending_reads_.end()) {
                    // 3. 在Reactor中执行实际读取
                    auto& op = it->second;
                    ssize_t n = recv(fd, op.buffer, op.size, 0);

                    // 4. 调用完成回调——看起来像Proactor
                    bool success = n > 0;
                    op.callback(success, success ? n : 0);

                    // 清理
                    epoll_ctl(epfd_, EPOLL_CTL_DEL, fd, nullptr);
                    pending_reads_.erase(it);
                }
            }
        }
    }

private:
    struct AsyncReadOp {
        int fd;
        void* buffer;
        size_t size;
        CompletionCallback callback;
    };

    int epfd_;
    std::unordered_map<int, AsyncReadOp> pending_reads_;
};
```

---

## 3. Windows IOCP实现

### 3.1 IOCP原理

IOCP（I/O Completion Port）是Windows上真正的异步IO机制：

```
┌──────────────────────────────────────────────────────────────┐
│                    IOCP 架构                                  │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│   ┌──────────────────────────────────────────────────┐      │
│   │                  完成端口                          │      │
│   │  ┌──────────────────────────────────────────┐    │      │
│   │  │           完成队列 (FIFO)                 │    │      │
│   │  │  [完成1] → [完成2] → [完成3] → ...        │    │      │
│   │  └──────────────────────────────────────────┘    │      │
│   │                                                    │      │
│   │  工作线程1  工作线程2  工作线程3                    │      │
│   │  ┌──────┐  ┌──────┐  ┌──────┐                   │      │
│   │  │GetQue│  │GetQue│  │GetQue│                   │      │
│   │  │uedCo │  │uedCo │  │uedCo │                   │      │
│   │  │mplet │  │mplet │  │mplet │                   │      │
│   │  │ionSt │  │ionSt │  │ionSt │                   │      │
│   │  │atus()│  │atus()│  │atus()│                   │      │
│   │  └──┬───┘  └──┬───┘  └──┬───┘                   │      │
│   │     │         │         │                         │      │
│   │     ▼         ▼         ▼                         │      │
│   │  处理完成事件                                    │      │
│   └──────────────────────────────────────────────────┘      │
│                                                              │
│   IO流程：                                                    │
│   1. WSASend/WSARecv 发起异步IO                              │
│   2. 内核执行IO操作                                           │
│   3. IO完成后，结果放入完成队列                                │
│   4. 工作线程通过GetQueuedCompletionStatus获取结果             │
│                                                              │
│   关键特性：                                                  │
│   ✅ 真正的异步IO——内核完成整个操作                            │
│   ✅ 多线程安全——同一完成端口可关联多个线程                     │
│   ✅ LIFO调度——最近阻塞的线程先被唤醒（缓存友好）               │
│   ✅ 自动负载均衡——完成端口自动分配任务                        │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 3.2 IOCP完整示例

```cpp
#include <iostream>
#include <vector>
#include <thread>
#include <cstring>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")

// IOCP服务器
class IOCPServer {
public:
    IOCPServer(int port, int num_workers = 4)
        : port_(port), num_workers_(num_workers),
          listen_socket_(INVALID_SOCKET),
          completion_port_(nullptr) {}

    void start() {
        // 初始化Winsock
        WSADATA wsa_data;
        WSAStartup(MAKEWORD(2, 2), &wsa_data);

        // 创建完成端口
        completion_port_ = CreateIoCompletionPort(
            INVALID_HANDLE_VALUE, nullptr, 0, num_workers_);

        // 创建监听socket
        listen_socket_ = WSASocket(AF_INET, SOCK_STREAM, 0,
                                    nullptr, 0, WSA_FLAG_OVERLAPPED);

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = INADDR_ANY;

        bind(listen_socket_, (sockaddr*)&addr, sizeof(addr));
        listen(listen_socket_, SOMAXCONN);

        // 关联监听socket到完成端口
        CreateIoCompletionPort((HANDLE)listen_socket_,
                                completion_port_, 0, 0);

        std::cout << "IOCP服务器启动，端口: " << port_ << std::endl;

        // 启动工作线程
        for (int i = 0; i < num_workers_; ++i) {
            workers_.emplace_back([this] { worker_thread(); });
        }

        // 接受连接循环
        while (true) {
            SOCKET client = accept(listen_socket_, nullptr, nullptr);
            if (client == INVALID_SOCKET) continue;

            // 关联到完成端口
            CreateIoCompletionPort((HANDLE)client,
                                    completion_port_, 0, 0);

            // 投递异步接收
            post_recv(client);
        }
    }

private:
    // Per-IO数据
    struct PerIOData {
        OVERLAPPED overlapped;
        WSABUF wsa_buf;
        char buffer[4096];
        SOCKET socket;
        enum { RECV, SEND } operation;
    };

    void post_recv(SOCKET socket) {
        auto* per_io = new PerIOData{};
        per_io->socket = socket;
        per_io->operation = PerIOData::RECV;
        per_io->wsa_buf.buf = per_io->buffer;
        per_io->wsa_buf.len = sizeof(per_io->buffer);
        memset(&per_io->overlapped, 0, sizeof(OVERLAPPED));

        DWORD flags = 0;
        WSARecv(socket, &per_io->wsa_buf, 1, nullptr,
                &flags, &per_io->overlapped, nullptr);
    }

    void worker_thread() {
        DWORD bytes_transferred;
        ULONG_PTR completion_key;
        OVERLAPPED* overlapped;

        while (true) {
            BOOL success = GetQueuedCompletionStatus(
                completion_port_,
                &bytes_transferred,
                &completion_key,
                &overlapped,
                INFINITE);

            if (!success) {
                if (overlapped == nullptr) continue;
                // 连接错误或关闭
                auto* per_io = CONTAINING_RECORD(overlapped, PerIOData, overlapped);
                closesocket(per_io->socket);
                delete per_io;
                continue;
            }

            auto* per_io = CONTAINING_RECORD(overlapped, PerIOData, overlapped);

            if (bytes_transferred == 0) {
                // 连接关闭
                closesocket(per_io->socket);
                delete per_io;
                continue;
            }

            if (per_io->operation == PerIOData::RECV) {
                // 收到数据，回显
                per_io->operation = PerIOData::SEND;
                per_io->wsa_buf.len = bytes_transferred;
                memset(&per_io->overlapped, 0, sizeof(OVERLAPPED));

                WSASend(per_io->socket, &per_io->wsa_buf, 1,
                         nullptr, 0, &per_io->overlapped, nullptr);
            } else {
                // 发送完成，继续接收
                post_recv(per_io->socket);
                delete per_io;
            }
        }
    }

    int port_;
    int num_workers_;
    SOCKET listen_socket_;
    HANDLE completion_port_;
    std::vector<std::thread> workers_;
};
#endif
```

---

## 4. Linux AIO与io_uring

### 4.1 Linux AIO

Linux原生AIO（libaio）主要用于文件IO，对网络IO支持有限：

```cpp
#include <iostream>

#ifdef __linux__
#include <libaio.h>

// Linux AIO 文件IO示例
void linux_aio_file_example() {
    io_context_t ctx = 0;

    // 初始化AIO上下文
    if (io_setup(128, &ctx) < 0) {
        perror("io_setup");
        return;
    }

    // 打开文件
    int fd = open("test.dat", O_RDONLY | O_DIRECT);

    // 准备IO操作
    struct iocb cb{};
    char* buffer;
    posix_memalign((void**)&buffer, 512, 4096);  // O_DIRECT需要对齐

    io_prep_pread(&cb, fd, buffer, 4096, 0);  // 异步读

    // 提交IO
    struct iocb* cbs[1] = { &cb };
    io_submit(ctx, 1, cbs);

    // 等待完成
    struct io_event events[1];
    int n = io_getevents(ctx, 1, 1, events, nullptr);

    if (n > 0) {
        std::cout << "AIO读取完成: " << events[0].res << " 字节" << std::endl;
    }

    free(buffer);
    close(fd);
    io_destroy(ctx);
}
#endif

// Linux AIO的局限性
void explain_linux_aio_limitations() {
    std::cout << "Linux AIO (libaio) 的局限性：" << std::endl;
    std::cout << "1. 仅支持文件IO（O_DIRECT），不支持网络IO" << std::endl;
    std::cout << "2. 必须使用O_DIRECT（绕过页缓存）" << std::endl;
    std::cout << "3. API不够友好，错误处理复杂" << std::endl;
    std::cout << "4. 不支持缓存IO（buffered I/O）" << std::endl;
    std::cout << "5. io_submit可能阻塞（不是真正异步）" << std::endl;
    std::cout << "\n推荐：使用io_uring替代Linux AIO" << std::endl;
}
```

### 4.2 io_uring——Linux异步IO的未来

```
┌──────────────────────────────────────────────────────────────┐
│                    io_uring vs Linux AIO                      │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│   Linux AIO (libaio)：                                       │
│   ❌ 仅支持文件IO（O_DIRECT）                                 │
│   ❌ io_submit可能阻塞                                       │
│   ❌ API复杂，需要内存对齐                                    │
│   ❌ 不支持缓存IO                                            │
│                                                              │
│   io_uring：                                                 │
│   ✅ 同时支持文件IO和网络IO                                   │
│   ✅ 真正异步（SQPOLL模式无系统调用）                         │
│   ✅ 统一简洁的API                                           │
│   ✅ 支持缓存IO和直接IO                                      │
│   ✅ 批量提交和完成                                          │
│   ✅ 链式请求（IOSQE_IO_LINK）                               │
│   ✅ 超时、信号、文件注册等高级特性                           │
│                                                              │
│   io_uring的三种模式：                                       │
│   ┌──────────────────────────────────────────────┐          │
│   │  1. 默认模式：每次io_uring_enter系统调用      │          │
│   │  2. SQPOLL：内核线程轮询SQ，无系统调用        │          │
│   │  3. SQPOLL + IOPOLL：内核线程轮询SQ和CQ      │          │
│   └──────────────────────────────────────────────┘          │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 4.3 io_uring Proactor实现

```cpp
#include <iostream>
#include <cstring>
#include <memory>
#include <functional>
#include <vector>

#ifdef __linux__
#include <liburing.h>

// 基于io_uring的Proactor实现
class IOUringProactor {
public:
    using CompletionCallback = std::function<void(bool, size_t)>;

    IOUringProactor(unsigned int entries = 128) {
        if (io_uring_queue_init(entries, &ring_, 0) < 0) {
            perror("io_uring_queue_init");
        }
    }

    ~IOUringProactor() {
        io_uring_queue_exit(&ring_);
    }

    // 异步accept
    void async_accept(int listenfd,
                      std::function<void(bool, int)> callback) {
        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
        if (!sqe) return;

        auto* req = new AcceptRequest{listenfd, std::move(callback)};

        sockaddr_in client_addr{};
        socklen_t client_len = sizeof(client_addr);

        io_uring_prep_accept(sqe, listenfd,
                              (sockaddr*)&client_addr, &client_len, 0);
        io_uring_sqe_set_data(sqe, req);
        io_uring_submit(&ring_);
    }

    // 异步读
    void async_read(int fd, void* buffer, size_t size,
                    CompletionCallback callback) {
        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
        if (!sqe) return;

        auto* req = new IORequest{fd, std::move(callback)};

        io_uring_prep_recv(sqe, fd, buffer, size, 0);
        io_uring_sqe_set_data(sqe, req);
        io_uring_submit(&ring_);
    }

    // 异步写
    void async_write(int fd, const void* buffer, size_t size,
                     CompletionCallback callback) {
        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring_);
        if (!sqe) return;

        auto* req = new IORequest{fd, std::move(callback)};

        io_uring_prep_send(sqe, fd, buffer, size, 0);
        io_uring_sqe_set_data(sqe, req);
        io_uring_submit(&ring_);
    }

    // 运行完成事件循环
    void run() {
        while (true) {
            struct io_uring_cqe* cqe;
            int ret = io_uring_wait_cqe(&ring_, &cqe);

            if (ret < 0) {
                if (ret == -EINTR) continue;
                break;
            }

            handle_completion(cqe);
            io_uring_cqe_seen(&ring_, cqe);
        }
    }

    // 批量处理完成事件
    void run_batch() {
        struct io_uring_cqe* cqes[128];

        while (true) {
            int n = io_uring_peek_batch_cqe(&ring_, cqes, 128);

            if (n == 0) {
                // 等待至少一个完成事件
                struct io_uring_cqe* cqe;
                int ret = io_uring_wait_cqe(&ring_, &cqe);
                if (ret < 0) continue;
                handle_completion(cqe);
                io_uring_cqe_seen(&ring_, cqe);
                continue;
            }

            for (int i = 0; i < n; ++i) {
                handle_completion(cqes[i]);
                io_uring_cqe_seen(&ring_, cqes[i]);
            }
        }
    }

private:
    struct RequestBase {
        virtual ~RequestBase() = default;
        virtual void complete(int result) = 0;
    };

    struct AcceptRequest : RequestBase {
        int listenfd;
        std::function<void(bool, int)> callback;

        void complete(int result) override {
            callback(result >= 0, result);
            delete this;
        }
    };

    struct IORequest : RequestBase {
        int fd;
        CompletionCallback callback;

        void complete(int result) override {
            callback(result >= 0, result > 0 ? result : 0);
            delete this;
        }
    };

    void handle_completion(struct io_uring_cqe* cqe) {
        auto* req = static_cast<RequestBase*>(io_uring_cqe_get_data(cqe));
        if (req) {
            req->complete(cqe->res);
        }
    }

    struct io_uring ring_;
};

// 使用io_uring Proactor的TCP服务器
class IOUringServer {
public:
    IOUringServer(int port) : port_(port) {}

    void start() {
        int listenfd = socket(AF_INET, SOCK_STREAM, 0);
        int opt = 1;
        setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_port = htons(port_);
        addr.sin_addr.s_addr = INADDR_ANY;

        bind(listenfd, (sockaddr*)&addr, sizeof(addr));
        listen(listenfd, 128);

        std::cout << "io_uring Proactor服务器启动，端口: " << port_ << std::endl;

        // 发起第一个accept
        post_accept(listenfd);

        // 运行完成循环
        proactor_.run();
    }

private:
    void post_accept(int listenfd) {
        proactor_.async_accept(listenfd,
            [this, listenfd](bool success, int clientfd) {
                if (success) {
                    std::cout << "新连接: fd=" << clientfd << std::endl;
                    post_recv(clientfd);
                }
                // 继续accept
                post_accept(listenfd);
            });
    }

    void post_recv(int fd) {
        auto* buffer = new char[4096];
        proactor_.async_read(fd, buffer, 4096,
            [this, fd, buffer](bool success, size_t bytes) {
                if (success && bytes > 0) {
                    // 回显
                    post_send(fd, buffer, bytes);
                } else {
                    close(fd);
                    delete[] buffer;
                }
            });
    }

    void post_send(int fd, const char* data, size_t len) {
        auto* buffer = new char[len];
        memcpy(buffer, data, len);
        delete[] data;  // 释放接收缓冲区

        proactor_.async_write(fd, buffer, len,
            [this, fd, buffer](bool success, size_t bytes) {
                delete[] buffer;
                if (success) {
                    post_recv(fd);  // 继续接收
                } else {
                    close(fd);
                }
            });
    }

    int port_;
    IOUringProactor proactor_;
};
#endif
```

---

## 5. Boost.Asio的Proactor设计

### 5.1 Boost.Asio架构

Boost.Asio在所有平台上都采用Proactor设计模式：

```
┌──────────────────────────────────────────────────────────────┐
│                  Boost.Asio Proactor架构                      │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│   ┌──────────────────────────────────────────────────┐      │
│   │               io_context                          │      │
│   │                                                    │      │
│   │  ┌────────────┐    ┌────────────────────────┐    │      │
│   │  │ Reactor    │    │ Proactor               │    │      │
│   │  │ (epoll/    │    │ (完成队列)              │    │      │
│   │  │  kqueue/   │───▶│                        │    │      │
│   │  │  select)   │    │  完成处理器分发          │    │      │
│   │  └────────────┘    └────────────────────────┘    │      │
│   │                                                    │      │
│   │  在Linux上：Reactor(epoll) + 模拟Proactor         │      │
│   │  在Windows上：直接使用IOCP（真正的Proactor）       │      │
│   └──────────────────────────────────────────────────┘      │
│                                                              │
│   Asio的Proactor实现策略：                                    │
│   ┌──────────────────────────────────────────────┐          │
│   │  Linux:  async_read() → epoll_wait → recv()  │          │
│   │         → 完成回调（Reactor模拟Proactor）      │          │
│   │                                               │          │
│   │  Windows: async_read() → WSARecv()            │          │
│   │           → IOCP完成通知 → 完成回调             │          │
│   │           （真正的Proactor）                    │          │
│   └──────────────────────────────────────────────┘          │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

### 5.2 Boost.Asio完整示例

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <boost/asio.hpp>

namespace asio = boost::asio;
using tcp = asio::ip::tcp;

// Boost.Asio TCP服务器（Proactor风格）
class AsioTcpServer {
public:
    AsioTcpServer(asio::io_context& io_ctx, int port)
        : io_ctx_(io_ctx),
          acceptor_(io_ctx, tcp::endpoint(tcp::v4(), port)) {
        std::cout << "Boost.Asio Proactor服务器启动，端口: " << port << std::endl;
        start_accept();
    }

private:
    // 发起异步accept
    void start_accept() {
        auto socket = std::make_shared<tcp::socket>(io_ctx_);

        acceptor_.async_accept(*socket,
            [this, socket](boost::system::error_code ec) {
                if (!ec) {
                    std::cout << "新连接" << std::endl;
                    // 为新连接创建会话
                    auto session = std::make_shared<Session>(std::move(*socket));
                    session->start();
                }
                // 继续accept
                start_accept();
            });
    }

    // TCP会话
    class Session : public std::enable_shared_from_this<Session> {
    public:
        Session(tcp::socket socket)
            : socket_(std::move(socket)) {}

        void start() {
            do_read();
        }

    private:
        // 异步读
        void do_read() {
            auto self = shared_from_this();

            socket_.async_read_some(
                asio::buffer(buffer_),
                [this, self](boost::system::error_code ec, size_t bytes) {
                    if (!ec) {
                        // 读完数据后，异步写回（回显）
                        do_write(bytes);
                    }
                    // 连接关闭或出错时，Session自动析构
                });
        }

        // 异步写
        void do_write(size_t length) {
            auto self = shared_from_this();

            asio::async_write(
                socket_,
                asio::buffer(buffer_, length),
                [this, self](boost::system::error_code ec, size_t /*bytes*/) {
                    if (!ec) {
                        // 写完后继续读
                        do_read();
                    }
                });
        }

        tcp::socket socket_;
        std::array<char, 4096> buffer_;
    };

    asio::io_context& io_ctx_;
    tcp::acceptor acceptor_;
};

// Boost.Asio + 多线程
void multi_threaded_asio_server() {
    asio::io_context io_ctx;
    AsioTcpServer server(io_ctx, 8080);

    // 多个线程运行同一个io_context
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&io_ctx] {
            io_ctx.run();  // 每个线程都运行事件循环
        });
    }

    for (auto& t : threads) {
        t.join();
    }
}

// Boost.Asio + 协程（C++20）
void coroutine_asio_server() {
    asio::io_context io_ctx;
    tcp::acceptor acceptor(io_ctx, tcp::endpoint(tcp::v4(), 8080));

    std::cout << "协程版Asio服务器启动" << std::endl;

    // 使用协程的异步accept循环
    asio::co_spawn(io_ctx,
        [&acceptor]() -> asio::awaitable<void> {
            while (true) {
                auto socket = co_await acceptor.async_accept(
                    asio::use_awaitable);

                // 为每个连接启动协程
                asio::co_spawn(
                    socket.get_executor(),
                    [socket = std::move(socket)]() mutable -> asio::awaitable<void> {
                        std::array<char, 4096> buffer;
                        while (true) {
                            size_t n = co_await socket.async_read_some(
                                asio::buffer(buffer),
                                asio::use_awaitable);

                            co_await asio::async_write(
                                socket,
                                asio::buffer(buffer, n),
                                asio::use_awaitable);
                        }
                    },
                    asio::detached);
            }
        },
        asio::detached);

    io_ctx.run();
}

int main() {
    // 单线程版本
    asio::io_context io_ctx;
    AsioTcpServer server(io_ctx, 8080);
    io_ctx.run();

    return 0;
}
```

### 5.3 Boost.Asio设计分析

```
┌──────────────────────────────────────────────────────────────┐
│                  Boost.Asio Proactor设计分析                   │
├──────────────────────────────────────────────────────────────┤
│                                                              │
│   核心组件：                                                  │
│   ┌──────────────────────────────────────────────┐          │
│   │  io_context      —— 完成端口/事件循环         │          │
│   │  executor        —— 执行上下文                │          │
│   │  strand          —— 串行执行保证线程安全       │          │
│   │  buffer          —— 缓冲区抽象                │          │
│   │  error_code      —— 错误处理                  │          │
│   └──────────────────────────────────────────────┘          │
│                                                              │
│   Proactor模式在Asio中的体现：                                │
│   ┌──────────────────────────────────────────────┐          │
│   │  1. async_read_some() → 发起异步读            │          │
│   │  2. 内核/模拟层执行IO                         │          │
│   │  3. io_context.run() → 等待完成事件           │          │
│   │  4. 完成回调被调用 → 应用处理结果             │          │
│   └──────────────────────────────────────────────┘          │
│                                                              │
│   线程安全策略：                                              │
│   ┌──────────────────────────────────────────────┐          │
│   │  1. io_context.run() 可以在多线程中调用        │          │
│   │  2. 回调在调用run()的线程中执行                │          │
│   │  3. strand保证回调串行执行                     │          │
│   │  4. 不同连接的回调可以并行执行                  │          │
│   └──────────────────────────────────────────────┘          │
│                                                              │
└──────────────────────────────────────────────────────────────┘
```

```cpp
#include <iostream>
#include <boost/asio.hpp>

namespace asio = boost::asio;

// strand保证线程安全
class SafeSession : public std::enable_shared_from_this<SafeSession> {
public:
    SafeSession(asio::ip::tcp::socket socket)
        : socket_(std::move(socket)),
          strand_(socket_.get_executor()) {}

    void start() { do_read(); }

private:
    void do_read() {
        auto self = shared_from_this();

        // 使用strand保证回调在strand中串行执行
        socket_.async_read_some(
            asio::buffer(buffer_),
            asio::bind_executor(strand_,
                [this, self](boost::system::error_code ec, size_t bytes) {
                    if (!ec) {
                        do_write(bytes);
                    }
                }));
    }

    void do_write(size_t length) {
        auto self = shared_from_this();

        asio::async_write(
            socket_,
            asio::buffer(buffer_, length),
            asio::bind_executor(strand_,
                [this, self](boost::system::error_code ec, size_t) {
                    if (!ec) {
                        do_read();
                    }
                }));
    }

    asio::ip::tcp::socket socket_;
    asio::strand<asio::ip::tcp::socket::executor_type> strand_;
    std::array<char, 4096> buffer_;
};

// 定时器——Proactor模式下的超时处理
void timer_example() {
    asio::io_context io_ctx;

    // 5秒超时定时器
    asio::steady_timer timer(io_ctx,
                              std::chrono::seconds(5));

    timer.async_wait([](boost::system::error_code ec) {
        if (!ec) {
            std::cout << "定时器触发！" << std::endl;
        }
    });

    io_ctx.run();
}

// 连接超时控制
class TimeoutSession : public std::enable_shared_from_this<TimeoutSession> {
public:
    TimeoutSession(asio::ip::tcp::socket socket)
        : socket_(std::move(socket)),
          timer_(socket_.get_executor()),
          timeout_(std::chrono::seconds(30)) {}

    void start() {
        set_timeout();
        do_read();
    }

private:
    void set_timeout() {
        timer_.expires_after(timeout_);
        timer_.async_wait(
            [this](boost::system::error_code ec) {
                if (!ec) {
                    std::cout << "连接超时，关闭" << std::endl;
                    boost::system::error_code ignored;
                    socket_.close(ignored);
                }
            });
    }

    void cancel_timeout() {
        timer_.cancel();
    }

    void do_read() {
        auto self = shared_from_this();
        socket_.async_read_some(
            asio::buffer(buffer_),
            [this, self](boost::system::error_code ec, size_t bytes) {
                cancel_timeout();  // 取消超时
                if (!ec) {
                    do_write(bytes);
                }
            });
    }

    void do_write(size_t length) {
        auto self = shared_from_this();
        asio::async_write(
            socket_,
            asio::buffer(buffer_, length),
            [this, self](boost::system::error_code ec, size_t) {
                if (!ec) {
                    set_timeout();  // 重新设置超时
                    do_read();
                }
            });
    }

    asio::ip::tcp::socket socket_;
    asio::steady_timer timer_;
    std::chrono::seconds timeout_;
    std::array<char, 4096> buffer_;
};
```

---

## 6. 小结

### 核心要点回顾

| 要点 | 说明 |
|------|------|
| **Proactor模式** | 异步完成驱动的并发模式，IO完成后通知应用 |
| **Reactor vs Proactor** | Reactor通知"可操作"，Proactor通知"已完成" |
| **IOCP** | Windows真正的异步IO，Proactor模式的完美实现 |
| **Linux AIO** | 仅支持文件IO，API复杂，不推荐 |
| **io_uring** | Linux新一代异步IO，统一文件和网络IO |
| **Boost.Asio** | 跨平台Proactor实现，Linux上用Reactor模拟 |
| **strand** | Asio的线程安全保证，串行执行回调 |
| **协程** | C++20协程让异步代码像同步一样写 |

### 平台与模式选择

| 平台 | 推荐模式 | 推荐框架 |
|------|---------|---------|
| **Linux** | Reactor(epoll) | muduo / 自研 |
| **Linux 5.1+** | Proactor(io_uring) | liburing / 自研 |
| **Windows** | Proactor(IOCP) | IOCP / Boost.Asio |
| **跨平台** | Proactor(Asio) | Boost.Asio |
| **快速开发** | Proactor+协程 | Boost.Asio + C++20协程 |

### 下一步

- 学习零拷贝技术（第04章），减少数据传输开销
- 学习高并发服务器架构（第06章），综合运用各种技术
