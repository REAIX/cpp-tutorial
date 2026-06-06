# 什么是Reactor模式
> 📖 相关章节：[网络编程](../../02-CPP/35-网络编程.md)、[IO多路复用](../../08-高性能网络与异步IO/01-IO多路复用深入.md)、[Reactor模式](../../08-高性能网络与异步IO/02-Reactor模式.md)

> "要义概览：Reactor模式=一个前台（事件循环）接电话分派给对应部门（回调函数），而不是每个部门自己派人等电话。核心思想是'事件驱动+非阻塞IO+回调分派'。"

***

### 1. 通俗理解

- **Reactor** = 反应器，来了事件就"反应"——分派给对应的处理函数
- 不同于传统"一个连接一个线程"的模型，Reactor用少量线程处理大量连接
- 就像公司前台：所有电话先到前台，前台根据问题类型转给不同部门

| 概念 | 类比 | 说明 |
|------|------|------|
| Reactor | 公司前台 | 接收事件，分派给处理器 |
| 事件循环 | 前台不停接电话 | epoll_wait循环 |
| 回调函数 | 各部门 | 处理具体业务 |
| Handler | 部门员工 | 具体的读写处理逻辑 |
| Acceptor | 接待专员 | 专门处理新连接 |

***

### 2. 技术说明

#### 1. Reactor的核心组件

```
┌──────────────────────────────────────────────────┐
│                  Reactor 模式                      │
│                                                   │
│   ┌──────────┐    ┌──────────┐    ┌──────────┐  │
│   │ Handler  │    │ Handler  │    │ Handler  │  │
│   │ (读回调) │    │ (写回调) │    │ (连接回调)│  │
│   └────┬─────┘    └────┬─────┘    └────┬─────┘  │
│        │               │               │         │
│        └───────────────┼───────────────┘         │
│                        │                         │
│              ┌─────────▼──────────┐              │
│              │   Demux (事件分发)  │              │
│              │   epoll_wait()     │              │
│              └─────────┬──────────┘              │
│                        │                         │
│              ┌─────────▼──────────┐              │
│              │  Event Loop (循环) │              │
│              │  while(true) {     │              │
│              │    wait → dispatch │              │
│              │  }                 │              │
│              └────────────────────┘              │
└──────────────────────────────────────────────────┘
```

**四大核心角色**：

| 角色 | 职责 | 对应实现 |
|------|------|---------|
| Handle | 操作系统资源（fd） | socket fd |
| Synchronous Event Demultiplexer | 阻塞等待事件 | epoll_wait |
| Event Handler | 事件处理接口 | 回调函数/虚函数 |
| Concrete Event Handler | 具体业务处理 | onRead/onWrite/onConnect |

#### 2. 单线程Reactor

**架构**：

```
                    ┌──────────────────┐
                    │   主线程          │
                    │                  │
   新连接 ──→ Acceptor ──→ 注册Handler │
                    │                  │
   IO事件 ──→ epoll_wait ──→ 分派Handler│
                    │                  │
   业务处理 ──→ Handler回调 ──→ 读写响应│
                    └──────────────────┘
```

**优点**：简单，无线程同步问题
**缺点**：一个慢操作（如数据库查询）会阻塞整个事件循环

**适用场景**：Echo服务器、Redis（单线程处理命令）

#### 3. 多线程Reactor

**架构**：

```
                    ┌──────────────────┐
                    │   主线程          │
                    │   (Reactor)      │
   新连接 ──→ Acceptor ──→ 注册Handler │
                    │                  │
   IO事件 ──→ epoll_wait ──→ 分派任务  │
                    └───────┬──────────┘
                            │
              ┌─────────────┼─────────────┐
              │             │             │
        ┌─────▼─────┐ ┌────▼─────┐ ┌────▼─────┐
        │  工作线程1 │ │ 工作线程2 │ │ 工作线程3 │
        │  (业务处理)│ │  (业务处理)│ │  (业务处理)│
        └───────────┘ └──────────┘ └──────────┘
```

**优点**：IO和业务处理分离，慢操作不阻塞事件循环
**缺点**：需要线程间通信，Handler回调中不能直接操作IO

**适用场景**：大多数业务服务器

#### 4. 主从Reactor（Multiple Reactors）

**架构**：

```
┌─────────────────────────────────────────────────────┐
│                                                      │
│   ┌──────────────────┐                               │
│   │  Main Reactor    │  ← 只负责accept新连接         │
│   │  (主Reactor)     │                               │
│   │  epoll_wait()    │                               │
│   └────────┬─────────┘                               │
│            │ 新连接                                    │
│            │ 分配给Sub Reactor                         │
│   ┌────────┼──────────────────────────┐              │
│   │        │                          │              │
│   │  ┌─────▼──────┐  ┌───────▼──────┐ │              │
│   │  │Sub Reactor1│  │Sub Reactor2  │ │              │
│   │  │(从Reactor) │  │(从Reactor)   │ │              │
│   │  │epoll_wait()│  │epoll_wait()  │ │              │
│   │  │线程1       │  │线程2         │ │              │
│   │  │            │  │              │ │              │
│   │  │ conn1 conn2│  │ conn3 conn4  │ │              │
│   │  │ conn5      │  │ conn6        │ │              │
│   │  └────────────┘  └──────────────┘ │              │
│   │                                  │              │
│   └──────────────────────────────────┘              │
└─────────────────────────────────────────────────────┘
```

**优点**：
- Main Reactor专注accept，不会因IO处理慢而影响新连接
- Sub Reactor各自独立，无线程竞争
- 天然负载均衡（round-robin分配连接）

**缺点**：架构复杂，线程数多

**适用场景**：高性能服务器（Nginx、Netty、muduo）

#### 5. muduo的设计

陈硕的muduo网络库采用**one loop per thread + thread pool**模型：

| 组件 | 说明 |
|------|------|
| EventLoop | 事件循环，封装epoll_wait |
| EventLoopThread | 一个线程一个EventLoop |
| EventLoopThreadPool | EventLoop线程池 |
| TcpServer | 组合Main Reactor + Sub Reactors |
| TcpConnection | 一个连接的完整生命周期 |
| Channel | fd + 回调的封装 |

**muduo的关键设计决策**：

| 决策 | 选择 | 原因 |
|------|------|------|
| 触发模式 | LT | 编程简单，不易出错 |
| 线程模型 | one loop per thread | 避免锁，每个连接只属于一个线程 |
| 跨线程调用 | runInThread | 通过事件循环安全地跨线程操作 |
| 定时器 | timerfd | 统一到epoll事件循环中 |

**muduo的事件循环核心逻辑**：

```cpp
void EventLoop::loop() {
    while (!quit_) {
        // 1. 等待IO事件
        activeChannels_.clear();
        poller_->poll(kPollTimeMs, &activeChannels_);

        // 2. 处理IO事件
        for (Channel* channel : activeChannels_) {
            channel->handleEvent();
        }

        // 3. 处理跨线程调用
        doPendingFunctors();
    }
}
```

#### 6. Reactor vs Proactor

| 维度 | Reactor | Proactor |
|------|---------|----------|
| 通知时机 | IO就绪（可以读/写了） | IO完成（已经读/写好了） |
| 谁做IO | 应用程序自己read/write | 操作系统代劳 |
| 编程模型 | 同步非阻塞 | 异步 |
| 代表实现 | epoll(LT/ET) | IOCP(Windows)、io_uring(Linux) |
| 数据拷贝 | 应用程序自己读 | 操作系统已读好，直接给缓冲区 |
| 复杂度 | 较低 | 较高 |

**Reactor流程**：

```
epoll_wait返回可读 → 应用程序调用read() → 处理数据 → write()响应
```

**Proactor流程**：

```
发起异步read请求 → 操作系统读好数据 → 通知完成 → 处理数据 → 发起异步write
```

***

### 3. 代码示例

#### 1. 单线程Reactor实现

```cpp
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>
#include <functional>
#include <unordered_map>

#define PORT         8080
#define MAX_EVENTS   1024

// 事件回调类型
using EventCallback = std::function<void(int fd, uint32_t events)>;

// 通道：封装fd和回调
struct Channel {
    int fd;
    EventCallback readCallback;
    EventCallback writeCallback;
    EventCallback closeCallback;
};

// 单线程Reactor
class Reactor {
public:
    Reactor() : epfd_(-1), running_(false) {
        epfd_ = epoll_create1(0);
        events_ = new struct epoll_event[MAX_EVENTS];
    }

    ~Reactor() {
        close(epfd_);
        delete[] events_;
    }

    // 注册通道
    void registerChannel(Channel* ch) {
        struct epoll_event ev;
        ev.events = EPOLLIN;  // LT模式
        ev.data.ptr = ch;     // 存储Channel指针
        epoll_ctl(epfd_, EPOLL_CTL_ADD, ch->fd, &ev);
        channels_[ch->fd] = ch;
    }

    // 移除通道
    void removeChannel(Channel* ch) {
        epoll_ctl(epfd_, EPOLL_CTL_DEL, ch->fd, nullptr);
        channels_.erase(ch->fd);
    }

    // 事件循环
    void loop() {
        running_ = true;
        printf("Reactor事件循环启动\n");

        while (running_) {
            int nfds = epoll_wait(epfd_, events_, MAX_EVENTS, 1000);
            if (nfds < 0) {
                if (errno == EINTR) continue;
                perror("epoll_wait");
                break;
            }

            for (int i = 0; i < nfds; i++) {
                Channel* ch = static_cast<Channel*>(events_[i].data.ptr);
                uint32_t revents = events_[i].events;

                if (revents & (EPOLLERR | EPOLLHUP)) {
                    if (ch->closeCallback) ch->closeCallback(ch->fd, revents);
                } else if (revents & EPOLLIN) {
                    if (ch->readCallback) ch->readCallback(ch->fd, revents);
                } else if (revents & EPOLLOUT) {
                    if (ch->writeCallback) ch->writeCallback(ch->fd, revents);
                }
            }
        }
    }

    void stop() { running_ = false; }

private:
    int epfd_;
    bool running_;
    struct epoll_event* events_;
    std::unordered_map<int, Channel*> channels_;
};

// 设置非阻塞
static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int main(void) {
    int listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
    int opt = 1;
    setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    struct sockaddr_in addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = INADDR_ANY,
        .sin_port = htons(PORT)
    };
    bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(listen_fd, 512);

    Reactor reactor;

    // 监听通道：处理新连接
    Channel listen_ch;
    listen_ch.fd = listen_fd;
    listen_ch.readCallback = [&reactor](int fd, uint32_t) {
        while (1) {
            int client_fd = accept(fd, NULL, NULL);
            if (client_fd < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                break;
            }
            set_nonblocking(client_fd);
            printf("新连接 fd=%d\n", client_fd);

            // 为每个连接创建通道
            Channel* client_ch = new Channel();
            client_ch->fd = client_fd;
            client_ch->readCallback = [client_fd](int fd, uint32_t) {
                char buf[256];
                int n = read(fd, buf, sizeof(buf));
                if (n > 0) {
                    write(fd, buf, n);  // 回显
                } else {
                    close(fd);
                    printf("连接关闭 fd=%d\n", fd);
                }
            };
            client_ch->closeCallback = [client_fd](int fd, uint32_t) {
                close(fd);
                printf("连接异常关闭 fd=%d\n", fd);
            };

            reactor.registerChannel(client_ch);
        }
    };

    reactor.registerChannel(&listen_ch);
    reactor.loop();

    close(listen_fd);
    return 0;
}
```

#### 2. 主从Reactor示意

```cpp
#include <stdio.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <thread>
#include <vector>
#include <atomic>
#include <functional>

// 简化的EventLoop
class EventLoop {
public:
    EventLoop() : epfd_(epoll_create1(0)), running_(false) {}

    void loop() {
        running_ = true;
        struct epoll_event events[1024];
        while (running_) {
            int nfds = epoll_wait(epfd_, events, 1024, 100);
            for (int i = 0; i < nfds; i++) {
                auto* cb = static_cast<std::function<void()>*>(events[i].data.ptr);
                if (cb && *cb) (*cb)();
            }
        }
    }

    void stop() { running_ = false; }
    int epfd() const { return epfd_; }

private:
    int epfd_;
    std::atomic<bool> running_;
};

// 主从Reactor服务器
class MultiReactorServer {
public:
    MultiReactorServer(int sub_reactor_count = 4)
        : sub_count_(sub_reactor_count), next_sub_(0) {}

    void start(int port) {
        // 启动Sub Reactor线程
        for (int i = 0; i < sub_count_; i++) {
            sub_loops_.push_back(new EventLoop());
            sub_threads_.emplace_back([this, i]() {
                printf("Sub Reactor %d 启动\n", i);
                sub_loops_[i]->loop();
            });
        }

        // Main Reactor：只负责accept
        int listen_fd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0);
        int opt = 1;
        setsockopt(listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

        struct sockaddr_in addr = {
            .sin_family = AF_INET,
            .sin_addr.s_addr = INADDR_ANY,
            .sin_port = htons(port)
        };
        bind(listen_fd, (struct sockaddr*)&addr, sizeof(addr));
        listen(listen_fd, 512);

        // Main Reactor的accept回调
        auto accept_cb = [this, listen_fd]() {
            while (1) {
                int client_fd = accept(listen_fd, NULL, NULL);
                if (client_fd < 0) break;

                // Round-robin分配给Sub Reactor
                int sub_idx = next_sub_++ % sub_count_;
                printf("新连接 fd=%d 分配给Sub Reactor %d\n",
                       client_fd, sub_idx);

                // 在Sub Reactor中注册该连接
                // （简化示例，实际需要线程安全的事件通知机制）
                struct epoll_event ev;
                ev.events = EPOLLIN | EPOLLET;
                ev.data.fd = client_fd;
                epoll_ctl(sub_loops_[sub_idx]->epfd(),
                         EPOLL_CTL_ADD, client_fd, &ev);
            }
        };

        // 注册accept事件到Main Reactor
        struct epoll_event ev;
        ev.events = EPOLLIN | EPOLLET;
        ev.data.ptr = new std::function<void()>(accept_cb);
        epoll_ctl(main_loop_.epfd(), EPOLL_CTL_ADD, listen_fd, &ev);

        printf("Main Reactor启动\n");
        main_loop_.loop();
    }

private:
    EventLoop main_loop_;                // 主Reactor
    std::vector<EventLoop*> sub_loops_;  // 从Reactor数组
    std::vector<std::thread> sub_threads_;
    int sub_count_;
    std::atomic<int> next_sub_;          // 轮询索引
};

int main(void) {
    MultiReactorServer server(4);  // 4个Sub Reactor
    server.start(8080);
    return 0;
}
```

***

### 4. 常见问题

#### Q1：Reactor模式一定比线程-per-连接好吗？

不一定。连接数少（<1000）且每个连接需要大量CPU计算时，线程-per-连接可能更简单高效。Reactor的优势在"大量连接、少量活跃"的场景。

#### Q2：muduo为什么选择LT而不是ET？

陈硕在《Linux多线程服务端编程》中解释：LT编程更简单，不容易出错，且性能差异在实际场景中可以忽略。LT模式让muduo的代码更易理解和维护。

#### Q3：Reactor模式中如何处理耗时操作？

将耗时操作（数据库查询、文件IO、复杂计算）提交到线程池，处理完后通过runInThread安全地将结果写回连接。不要在Reactor线程中执行耗时操作。

#### Q4：Proactor比Reactor更好吗？

Proactor理论上更高效（操作系统代劳IO），但Linux原生不支持真正的异步IO（AIO只支持文件），所以Linux上主流还是Reactor。io_uring正在改变这个局面。

#### Q5：Nginx用的是什么模型？

Nginx使用多进程主从Reactor模型：master进程管理worker进程，每个worker进程是一个独立的Reactor（单线程事件循环）。通过SO_REUSEPORT让多个worker同时accept。

***

### 5. 总结

| 模型 | 线程数 | 复杂度 | 性能 | 适用场景 |
|------|--------|--------|------|---------|
| 线程-per-连接 | 多 | 低 | 低 | 简单服务器 |
| 单线程Reactor | 1 | 中 | 中 | Redis、Echo服务器 |
| 多线程Reactor | N+1 | 中高 | 高 | 通用业务服务器 |
| 主从Reactor | N+1+M | 高 | 极高 | 高性能服务器 |

Reactor模式的本质是**将"等待"和"处理"分离**——用事件循环高效地等待，用回调函数灵活地处理。这个思想贯穿了所有现代高性能服务器的设计。