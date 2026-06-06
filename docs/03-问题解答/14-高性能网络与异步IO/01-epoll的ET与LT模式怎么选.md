# epoll的ET与LT模式怎么选
> 📖 相关章节：[网络编程](../../02-CPP/35-网络编程.md)、[IO多路复用](../../08-高性能网络与异步IO/01-IO多路复用深入.md)、[Reactor模式](../../08-高性能网络与异步IO/02-Reactor模式.md)

> "核心提炼：LT（水平触发）像闹钟——不关就一直响；ET（边缘触发）像闪电——只闪一次，错过了就没了。选LT省心安全，选ET性能极致但必须搭配非阻塞IO循环读写。"

***

### 1. 通俗理解

- **LT（Level Triggered，水平触发）**：只要fd上有数据可读，每次epoll_wait都会通知你
- **ET（Edge Triggered，边缘触发）**：只在fd从"无数据"变成"有数据"的那一刻通知一次
- 就像水位传感器：LT是"水位超过警戒线就一直报警"，ET是"水位刚过警戒线时报警一次"

| 概念 | 类比 | 说明 |
|------|------|------|
| LT模式 | 闹钟不关一直响 | 有数据就一直通知，直到读完 |
| ET模式 | 闪电只闪一次 | 状态变化时只通知一次 |
| 非阻塞IO | 不等快递签收 | 读写不阻塞，没数据立即返回EAGAIN |
| 循环读写 | 把快递一次性搬完 | ET模式必须一次读完/写完所有数据 |

***

### 2. 技术说明

#### 1. 两种模式的本质区别

**LT模式的工作流程**：

```
时刻1: 缓冲区有100字节数据 → epoll_wait返回（通知可读）
时刻2: 你读了50字节，还剩50字节 → 下次epoll_wait仍会返回（还有数据！）
时刻3: 你又读了30字节，还剩20字节 → 下次epoll_wait仍会返回（还有数据！）
时刻4: 你读了最后20字节 → 下次epoll_wait不再返回（缓冲区空了）
```

**ET模式的工作流程**：

```
时刻1: 缓冲区从空变为有100字节数据 → epoll_wait返回（状态变化，通知一次！）
时刻2: 你读了50字节，还剩50字节 → epoll_wait不会返回（状态没变化！）
时刻3: 你又读了30字节，还剩20字节 → epoll_wait不会返回（状态没变化！）
时刻4: 你读了最后20字节 → 缓冲区空了
时刻5: 又来了50字节 → epoll_wait返回（状态又变化了！）
```

**核心差异图**：

```
数据到达 ────→ ┌──────────────────────────────────┐
              │        内核接收缓冲区               │
              │  ┌───┬───┬───┬───┬───┬───┬───┐    │
              │  │ D │ A │ T │ A │ . │ . │ . │    │
              │  └───┴───┴───┴───┴───┴───┴───┘    │
              │       ↑                            │
              │    数据来了！                       │
              └──────┬─────────────────────────────┘
                     │
         ┌───────────┴────────────┐
         │                        │
    LT模式                   ET模式
         │                        │
    每次epoll_wait           只在数据到达时
    都返回可读事件           返回一次可读事件
    (直到读完)              (必须一次读完)
```

#### 2. ET模式为什么需要非阻塞IO

**ET模式的铁律**：必须使用非阻塞IO + 循环读写直到EAGAIN。

**原因**：

| 场景 | 阻塞IO的问题 | 非阻塞IO的解决 |
|------|-------------|---------------|
| 读数据 | 只读了一部分，剩余数据在缓冲区，但ET不会再通知 → 数据丢失 | 循环read直到EAGAIN，确保读完 |
| 写数据 | 只写了一部分，缓冲区满了，阻塞等待 → 其他连接饿死 | 循环write直到EAGAIN，未写完的下次再写 |
| accept | 多个连接同时到达，只accept一个 → 其余连接丢失 | 循环accept直到EAGAIN |

**ET模式数据丢失的经典场景**：

```
1. 客户端发送100字节数据
2. epoll_wait返回，通知fd可读
3. 你只读了50字节（比如缓冲区只设了50）
4. 剩余50字节还在内核缓冲区
5. 你回到epoll_wait...
6. 但ET模式不会再次通知！（因为状态没有从"无数据"变为"有数据"）
7. 这50字节数据永远不会被读取 → 数据丢失！
```

**正确做法**：

```c
// ET模式必须这样读
while (1) {
    ssize_t n = read(fd, buf, sizeof(buf));
    if (n > 0) {
        // 处理数据
        process_data(buf, n);
    } else if (n == 0) {
        // 连接关闭
        close(fd);
        break;
    } else {
        // n < 0
        if (errno == EAGAIN || errno == EWOULDBLOCK) {
            // 数据读完了，正常退出循环
            break;
        }
        // 其他错误
        close(fd);
        break;
    }
}
```

#### 3. LT模式的优势

| 优势 | 说明 |
|------|------|
| 编程简单 | 不需要循环读写，每次读一部分就行 |
| 不容易出错 | 不会因为忘记循环读而丢失数据 |
| 兼容阻塞IO | 理论上可以搭配阻塞IO（不推荐） |
| 调试友好 | 行为可预测，容易排查问题 |

**LT模式的"缺点"**：

| 缺点 | 说明 |
|------|------|
| 系统调用次数多 | 每次epoll_wait都可能返回已知的就绪fd |
| 效率略低 | 对同一个fd可能多次通知 |

> 实际上，LT模式的"缺点"在大多数场景下影响微乎其微。Nginx使用ET模式是为了极致性能，但大多数应用用LT模式完全足够。

#### 4. 常见陷阱

**陷阱1：ET模式忘记循环accept**

```c
// 错误：ET模式只accept一次
if (events[i].data.fd == listen_fd) {
    int client_fd = accept(listen_fd, NULL, NULL);  // 只接一个！
    // 如果同时来了10个连接，其余9个永远不会被处理
}

// 正确：ET模式必须循环accept
if (events[i].data.fd == listen_fd) {
    while (1) {
        int client_fd = accept(listen_fd, NULL, NULL);
        if (client_fd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
            break;
        }
        // 处理新连接
    }
}
```

**陷阱2：ET模式忘记注册EPOLLOUT**

```c
// 错误：写不完就丢失数据
write(fd, data, len);  // 可能只写了一部分

// 正确：写不完时注册EPOLLOUT，等缓冲区可写时继续写
ssize_t n = write(fd, data + offset, len - offset);
if (n < len - offset) {
    // 没写完，注册EPOLLOUT事件
    ev.events = EPOLLIN | EPOLLOUT | EPOLLET;
    epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
}
// 在EPOLLOUT回调中继续写
```

**陷阱3：LT模式下EPOLLOUT风暴**

```c
// 错误：LT模式一直注册EPOLLOUT
ev.events = EPOLLIN | EPOLLOUT;  // 缓冲区几乎总是可写的！
// LT模式下，只要缓冲区可写，每次epoll_wait都会返回
// 导致CPU空转

// 正确：只在需要写时注册EPOLLOUT，写完后移除
// 需要写数据时：
ev.events = EPOLLIN | EPOLLOUT;
epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);

// 写完数据后：
ev.events = EPOLLIN;  // 移除EPOLLOUT
epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
```

**陷阱4：ET模式多线程惊群**

```c
// 多个线程epoll_wait同一个fd，ET模式只唤醒一个线程
// 但如果用EPOLLONESHOT，处理完后需要重新注册
ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
epoll_ctl(epfd, EPOLL_CTL_ADD, fd, &ev);

// 处理完后重新注册
ev.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);
```

***

### 3. 代码示例

#### 1. LT模式服务器

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>

#define PORT         8080
#define MAX_EVENTS   1024
#define BUF_SIZE     256

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

    int epfd = epoll_create1(0);

    // LT模式：默认就是LT，不需要EPOLLET标志
    struct epoll_event ev;
    ev.events = EPOLLIN;  // 水平触发，可读事件
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    struct epoll_event *events = calloc(MAX_EVENTS, sizeof(struct epoll_event));
    printf("LT模式服务器启动\n");

    while (1) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_fd) {
                // LT模式：accept一次就够了，下次有连接还会通知
                int client_fd = accept(listen_fd, NULL, NULL);
                if (client_fd >= 0) {
                    // 设置非阻塞（虽然LT不强制，但推荐）
                    int flags = fcntl(client_fd, F_GETFL, 0);
                    fcntl(client_fd, F_SETFL, flags | O_NONBLOCK);

                    struct epoll_event cev;
                    cev.events = EPOLLIN;  // LT模式
                    cev.data.fd = client_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &cev);
                    printf("新连接 fd=%d\n", client_fd);
                }
            } else {
                int fd = events[i].data.fd;
                char buf[BUF_SIZE];

                // LT模式：不需要循环读，读一次就行
                // 下次还有数据，epoll_wait会再次通知
                int n = read(fd, buf, sizeof(buf));
                if (n > 0) {
                    write(fd, buf, n);  // 回显
                    printf("LT: 读取%d字节 fd=%d\n", n, fd);
                } else if (n == 0) {
                    close(fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    printf("连接关闭 fd=%d\n", fd);
                } else {
                    if (errno != EAGAIN && errno != EWOULDBLOCK) {
                        close(fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                    }
                }
            }
        }
    }

    free(events);
    close(epfd);
    close(listen_fd);
    return 0;
}
```

#### 2. ET模式服务器

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <errno.h>

#define PORT         8080
#define MAX_EVENTS   1024
#define BUF_SIZE     4096  // ET模式建议用大缓冲区

// 设置非阻塞（ET模式必须！）
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

    int epfd = epoll_create1(0);

    // ET模式：需要EPOLLET标志
    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;  // 边缘触发
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    struct epoll_event *events = calloc(MAX_EVENTS, sizeof(struct epoll_event));
    printf("ET模式服务器启动\n");

    while (1) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, -1);
        if (nfds < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_fd) {
                // ET模式：必须循环accept！
                while (1) {
                    int client_fd = accept(listen_fd, NULL, NULL);
                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        break;
                    }
                    set_nonblocking(client_fd);

                    struct epoll_event cev;
                    cev.events = EPOLLIN | EPOLLET;  // ET模式
                    cev.data.fd = client_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &cev);
                    printf("新连接 fd=%d\n", client_fd);
                }
            } else {
                int fd = events[i].data.fd;
                char buf[BUF_SIZE];

                // ET模式：必须循环读直到EAGAIN！
                while (1) {
                    int n = read(fd, buf, sizeof(buf));
                    if (n > 0) {
                        write(fd, buf, n);  // 回显
                        printf("ET: 读取%d字节 fd=%d\n", n, fd);
                    } else if (n == 0) {
                        close(fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        printf("连接关闭 fd=%d\n", fd);
                        break;
                    } else {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) {
                            // 数据读完了，正常退出
                            break;
                        }
                        close(fd);
                        epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);
                        break;
                    }
                }
            }
        }
    }

    free(events);
    close(epfd);
    close(listen_fd);
    return 0;
}
```

#### 3. ET模式带写缓冲区的完整示例

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
#include <vector>
#include <string>

#define PORT         8080
#define MAX_EVENTS   1024

// 连接上下文：保存写缓冲区
struct ConnContext {
    int fd;
    std::string write_buf;  // 待写数据
};

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

    int epfd = epoll_create1(0);

    struct epoll_event ev;
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = listen_fd;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    struct epoll_event *events = calloc(MAX_EVENTS, sizeof(struct epoll_event));

    // 简单的连接上下文映射（生产环境用unordered_map）
    ConnContext* ctx_map[65536] = {0};

    printf("ET模式服务器启动（带写缓冲区）\n");

    while (1) {
        int nfds = epoll_wait(epfd, events, MAX_EVENTS, 1000);
        if (nfds < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < nfds; i++) {
            if (events[i].data.fd == listen_fd) {
                // ET模式：循环accept
                while (1) {
                    int client_fd = accept(listen_fd, NULL, NULL);
                    if (client_fd < 0) {
                        if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                        break;
                    }
                    set_nonblocking(client_fd);

                    // 创建连接上下文
                    auto* ctx = new ConnContext{client_fd, ""};
                    ctx_map[client_fd] = ctx;

                    struct epoll_event cev;
                    cev.events = EPOLLIN | EPOLLET;
                    cev.data.fd = client_fd;
                    epoll_ctl(epfd, EPOLL_CTL_ADD, client_fd, &cev);
                    printf("新连接 fd=%d\n", client_fd);
                }
            } else {
                int fd = events[i].data.fd;
                auto* ctx = ctx_map[fd];
                if (!ctx) continue;

                // 处理可读事件
                if (events[i].events & EPOLLIN) {
                    char buf[4096];
                    while (1) {
                        int n = read(fd, buf, sizeof(buf));
                        if (n > 0) {
                            // 模拟处理：将数据加入写缓冲区
                            ctx->write_buf.append(buf, n);

                            // 注册EPOLLOUT以发送响应
                            struct epoll_event cev;
                            cev.events = EPOLLIN | EPOLLOUT | EPOLLET;
                            cev.data.fd = fd;
                            epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &cev);
                        } else if (n == 0) {
                            close(fd);
                            delete ctx;
                            ctx_map[fd] = nullptr;
                            break;
                        } else {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                            close(fd);
                            delete ctx;
                            ctx_map[fd] = nullptr;
                            break;
                        }
                    }
                }

                // 处理可写事件
                if (events[i].events & EPOLLOUT && ctx && !ctx->write_buf.empty()) {
                    while (!ctx->write_buf.empty()) {
                        int n = write(fd, ctx->write_buf.data(),
                                     ctx->write_buf.size());
                        if (n > 0) {
                            ctx->write_buf.erase(0, n);
                        } else {
                            if (errno == EAGAIN || errno == EWOULDBLOCK) break;
                            close(fd);
                            delete ctx;
                            ctx_map[fd] = nullptr;
                            break;
                        }
                    }

                    // 写缓冲区空了，移除EPOLLOUT
                    if (ctx && ctx->write_buf.empty()) {
                        struct epoll_event cev;
                        cev.events = EPOLLIN | EPOLLET;
                        cev.data.fd = fd;
                        epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &cev);
                    }
                }
            }
        }
    }

    free(events);
    close(epfd);
    close(listen_fd);
    return 0;
}
```

***

### 4. LT vs ET选择指南

| 维度 | LT模式 | ET模式 |
|------|--------|--------|
| 编程复杂度 | 低 | 高 |
| 数据丢失风险 | 几乎没有 | 有（忘记循环读写） |
| 系统调用次数 | 较多 | 较少 |
| CPU利用率 | 略低 | 略高 |
| 适用场景 | 通用服务器、初学者 | 高性能服务器（Nginx等） |
| 非阻塞IO | 推荐但不强制 | 必须 |
| 循环读写 | 不需要 | 必须 |
| EPOLLOUT管理 | 需注意风暴 | 需注意注册/移除 |

**选择建议**：

- **新手/一般项目** → 选LT，简单安全
- **高性能要求** → 选ET，减少系统调用
- **已有框架** → 跟随框架（muduo用LT，Nginx用ET）
- **多线程Reactor** → ET + EPOLLONESHOT

***

### 5. 总结

ET和LT的选择不是"哪个更好"，而是"哪个更适合你的场景"。LT是安全的选择，ET是极致的选择。理解两者的本质差异——"持续通知"vs"变化通知"——是正确使用epoll的关键。记住ET模式的铁律：**非阻塞IO + 循环读写直到EAGAIN**，就不会踩坑。