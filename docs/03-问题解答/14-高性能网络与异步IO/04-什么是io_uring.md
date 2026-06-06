# 什么是io_uring
> 📖 相关章节：[网络编程](../../02-CPP/35-网络编程.md)、[IO多路复用](../../08-高性能网络与异步IO/01-IO多路复用深入.md)、[Reactor模式](../../08-高性能网络与异步IO/02-Reactor模式.md)

> "本质洞察：io_uring=两个环形缓冲区（提交队列+完成队列）让应用和内核共享内存，批量提交IO请求、批量接收完成通知，几乎不需要系统调用，是Linux异步IO的未来。"

***

### 1. 通俗理解

- **io_uring** = Linux 5.1引入的新一代异步IO接口
- 核心思想：应用和内核通过共享内存的环形缓冲区通信，减少系统调用
- 就像餐厅的"取餐号"系统：你把订单（提交队列）放进去，做好后叫号（完成队列），不用一直等

| 概念 | 类比 | 说明 |
|------|------|------|
| SQ（提交队列） | 下单窗口 | 应用把IO请求放进去 |
| CQ（完成队列） | 取餐窗口 | 内核把完成结果放进去 |
| 共享内存 | 传菜窗口 | 应用和内核都能直接访问 |
| 系统调用 | 叫服务员 | 尽量少叫，批量处理 |
| epoll | 每个菜都要叫一次服务员 | 每次IO都要系统调用 |

***

### 2. 技术说明

#### 1. io_uring的设计动机

**Linux异步IO的历史**：

| 时代 | 接口 | 问题 |
|------|------|------|
| 2000s | POSIX AIO | 只支持文件O_DIRECT，API复杂，基于线程模拟 |
| 2010s | libaio | 同样只支持O_DIRECT，需要内存对齐，不通用 |
| 2010s | epoll + 非阻塞IO | 只支持socket，不支持文件IO，每次都要系统调用 |
| 2019 | io_uring | 统一文件和socket的异步IO，共享内存，批量提交 |

**epoll的痛点**：

| 痛点 | 说明 |
|------|------|
| 每次IO都要系统调用 | read/write各一次，1万个IO就是2万次系统调用 |
| 只通知就绪，不帮你做IO | epoll_wait返回后还要自己read/write |
| 不支持文件IO | 文件fd总是"就绪"的，epoll对文件无效 |
| 两次拷贝 | 事件从内核拷贝到用户态 |

**io_uring的目标**：

- 统一文件IO和网络IO的异步接口
- 减少系统调用（批量提交）
- 零拷贝通信（共享环形缓冲区）
- 可扩展（支持超时、信号、文件注册等）

#### 2. 共享环形缓冲区原理

**两个环形缓冲区**：

```
┌─────────────────────────────────────────────────────┐
│                    io_uring 架构                      │
│                                                      │
│  ┌─────────────────────┐  ┌──────────────────────┐  │
│  │  SQ (提交队列)       │  │  CQ (完成队列)        │  │
│  │                     │  │                      │  │
│  │  ┌───┬───┬───┬───┐  │  │  ┌───┬───┬───┬───┐  │  │
│  │  │sqe│sqe│sqe│sqe│  │  │  │cqe│cqe│cqe│cqe│  │  │
│  │  │ 0 │ 1 │ 2 │ 3 │  │  │  │ 0 │ 1 │ 2 │ 3 │  │  │
│  │  └───┴───┴───┴───┘  │  │  └───┴───┴───┴───┘  │  │
│  │        ↑             │  │             ↑        │  │
│  │     应用写入          │  │          内核写入     │  │
│  │     内核读取          │  │          应用读取     │  │
│  └─────────────────────┘  └──────────────────────┘  │
│                                                      │
│  SQE = 提交队列条目（IO请求）                         │
│  CQE = 完成队列条目（IO结果）                         │
│                                                      │
│  应用写SQE → 内核消费SQE → 内核写CQE → 应用消费CQE   │
└─────────────────────────────────────────────────────┘
```

**工作流程**：

```
1. 应用准备SQE（填写IO请求）
2. 应用提交SQE（更新tail指针）
3. 可选：调用io_uring_enter()通知内核（或内核自动轮询）
4. 内核处理SQE，执行IO操作
5. 内核将结果写入CQE（更新tail指针）
6. 应用检查CQE（读取head到tail之间的条目）
7. 应用消费CQE（更新head指针）
```

**关键：SQ和CQ都是共享内存，应用和内核都可以直接访问，不需要拷贝！**

**SQE（提交队列条目）结构**：

```c
struct io_uring_sqe {
    __u8  opcode;       // 操作码：read/write/accept/connect等
    __u8  flags;        // 标志位
    __u16 ioprio;       // IO优先级
    __s32 fd;           // 文件描述符
    __u64 off;          // 偏移量
    __u64 addr;         // 缓冲区地址
    __u32 len;          // 长度
    // ... 其他字段
    __u64 user_data;    // 用户数据（关联请求和完成）
};
```

**CQE（完成队列条目）结构**：

```c
struct io_uring_cqe {
    __u64 user_data;    // 对应SQE的user_data
    __s32 res;          // IO结果（返回值或错误码）
    __u32 flags;        // 标志位
};
```

#### 3. io_uring vs epoll对比

| 维度 | epoll | io_uring |
|------|-------|----------|
| IO模型 | 通知就绪 | 通知完成 |
| 系统调用 | 每次IO都要 | 批量提交，可零系统调用 |
| 数据拷贝 | 事件需要拷贝 | 共享内存，零拷贝 |
| 文件IO | 不支持 | 支持 |
| 批量操作 | 不支持 | 支持（一次提交多个IO） |
| 编程复杂度 | 中 | 高（liburing简化后中） |
| 内核版本 | 2.5.44+ | 5.1+ |
| 成熟度 | 非常成熟 | 快速发展中 |

**性能对比（单线程，随机读）**：

```
操作          epoll+read     io_uring
1万次4K读     ~3.5ms         ~1.2ms
10万次4K读    ~35ms          ~10ms
100万次4K读   ~350ms         ~95ms
```

> io_uring的优势在批量IO操作时尤为明显。

#### 4. io_uring的使用方式

**三种使用层次**：

| 层次 | 说明 | 适用场景 |
|------|------|---------|
| 原始接口 | 直接操作环形缓冲区 | 极致性能，需要完全控制 |
| liburing | 封装库，简化使用 | 大多数场景推荐 |
| io_uring协程 | C++20协程封装 | 最优雅的编程模型 |

**liburing核心API**：

| API | 说明 |
|-----|------|
| `io_uring_queue_init` | 初始化io_uring实例 |
| `io_uring_get_sqe` | 获取一个空闲的SQE |
| `io_uring_prep_read` | 准备read操作 |
| `io_uring_prep_write` | 准备write操作 |
| `io_uring_prep_accept` | 准备accept操作 |
| `io_uring_submit` | 提交所有SQE |
| `io_uring_wait_cqe` | 等待一个CQE完成 |
| `io_uring_cqe_seen` | 标记CQE已消费 |
| `io_uring_queue_exit` | 销毁io_uring实例 |

**高级特性**：

| 特性 | 说明 |
|------|------|
| SQPOLL | 内核线程轮询SQ，应用不需要调用io_uring_enter |
| 固定文件注册 | 预注册fd，避免每次IO查找fd |
| 缓冲区注册 | 预注册缓冲区，减少内核开销 |
| 链式请求 | IO依赖链（先read再write） |
| 超时 | 为IO操作设置超时 |
| 取消 | 取消已提交的IO请求 |

***

### 3. 代码示例

#### 1. liburing基本读写

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "liburing.h"

#define QUEUE_DEPTH  4
#define BUF_SIZE     4096

int main(int argc, char* argv[]) {
    const char* filepath = "test_io_uring.txt";

    // 创建测试文件
    int fd = open(filepath, O_RDWR | O_CREAT | O_TRUNC, 0644);
    const char* write_data = "Hello, io_uring! 这是通过io_uring写入的数据。";
    size_t data_len = strlen(write_data);

    // 初始化io_uring
    struct io_uring ring;
    if (io_uring_queue_init(QUEUE_DEPTH, &ring, 0) < 0) {
        perror("io_uring_queue_init");
        close(fd);
        return 1;
    }

    // ===== 写操作 =====
    // 1. 获取SQE
    struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
    if (!sqe) {
        fprintf(stderr, "获取SQE失败\n");
        goto cleanup;
    }

    // 2. 准备写操作
    io_uring_prep_write(sqe, fd, write_data, data_len, 0);
    // 设置user_data用于关联完成事件
    io_uring_sqe_set_data(sqe, (void*)"write_op");

    // 3. 提交
    int submitted = io_uring_submit(&ring);
    printf("提交了%d个写请求\n", submitted);

    // 4. 等待完成
    struct io_uring_cqe* cqe;
    if (io_uring_wait_cqe(&ring, &cqe) < 0) {
        perror("io_uring_wait_cqe");
        goto cleanup;
    }

    // 5. 检查结果
    printf("写操作完成: user_data=%s, 结果=%d\n",
           (char*)io_uring_cqe_get_data(cqe), cqe->res);
    io_uring_cqe_seen(&ring, cqe);  // 标记已消费

    // ===== 读操作 =====
    char read_buf[BUF_SIZE];
    sqe = io_uring_get_sqe(&ring);
    io_uring_prep_read(sqe, fd, read_buf, BUF_SIZE, 0);
    io_uring_sqe_set_data(sqe, (void*)"read_op");

    io_uring_submit(&ring);

    if (io_uring_wait_cqe(&ring, &cqe) < 0) {
        perror("io_uring_wait_cqe");
        goto cleanup;
    }

    printf("读操作完成: user_data=%s, 读取%d字节\n",
           (char*)io_uring_cqe_get_data(cqe), cqe->res);
    read_buf[cqe->res] = '\0';
    printf("读取内容: %s\n", read_buf);
    io_uring_cqe_seen(&ring, cqe);

cleanup:
    io_uring_queue_exit(&ring);
    close(fd);
    unlink(filepath);
    return 0;
}
```

#### 2. io_uring网络服务器

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include "liburing.h"

#define PORT          8080
#define QUEUE_DEPTH   256
#define MAX_CONN      1024
#define BUF_SIZE      4096

// 连接上下文
struct ConnInfo {
    int fd;
    char buf[BUF_SIZE];
};

// 连接数组
static struct ConnInfo conns[MAX_CONN];
static int conn_count = 0;

static int set_nonblocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// 提交accept请求
static void submit_accept(struct io_uring* ring, int listen_fd) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    io_uring_prep_accept(sqe, listen_fd, NULL, NULL, 0);
    io_uring_sqe_set_data(sqe, (void*)0xFFFFFFFF);  // 标记为accept
}

// 提交read请求
static void submit_read(struct io_uring* ring, int fd, char* buf) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    io_uring_prep_recv(sqe, fd, buf, BUF_SIZE, 0);
    io_uring_sqe_set_data64(sqe, (unsigned long)fd);
}

// 提交write请求
static void submit_write(struct io_uring* ring, int fd, char* buf, size_t len) {
    struct io_uring_sqe* sqe = io_uring_get_sqe(ring);
    io_uring_prep_send(sqe, fd, buf, len, 0);
    io_uring_sqe_set_data64(sqe, (unsigned long)fd | 0x80000000UL);  // 标记为write
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

    // 初始化io_uring
    struct io_uring ring;
    io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

    // 提交初始accept
    submit_accept(&ring, listen_fd);

    printf("io_uring服务器启动，端口: %d\n", PORT);

    while (1) {
        // 提交所有待处理的SQE
        io_uring_submit(&ring);

        // 等待完成事件
        struct io_uring_cqe* cqe;
        unsigned head;
        unsigned count = 0;

        io_uring_for_each_cqe(&ring, head, cqe) {
            count++;

            if (io_uring_cqe_get_data(cqe) == (void*)0xFFFFFFFF) {
                // accept完成
                int client_fd = cqe->res;
                if (client_fd >= 0) {
                    set_nonblocking(client_fd);
                    printf("新连接 fd=%d\n", client_fd);

                    // 分配连接上下文
                    if (conn_count < MAX_CONN) {
                        conns[conn_count].fd = client_fd;
                        submit_read(&ring, client_fd, conns[conn_count].buf);
                        conn_count++;
                    }

                    // 继续提交accept
                    submit_accept(&ring, listen_fd);
                }
            } else if ((unsigned long)io_uring_cqe_get_data(cqe) & 0x80000000UL) {
                // write完成
                int fd = (unsigned long)io_uring_cqe_get_data(cqe) & ~0x80000000UL;
                // 继续读
                for (int i = 0; i < conn_count; i++) {
                    if (conns[i].fd == fd) {
                        submit_read(&ring, fd, conns[i].buf);
                        break;
                    }
                }
            } else {
                // read完成
                int fd = (unsigned long)io_uring_cqe_get_data(cqe);
                int res = cqe->res;

                if (res > 0) {
                    // 找到对应的缓冲区，回显
                    for (int i = 0; i < conn_count; i++) {
                        if (conns[i].fd == fd) {
                            submit_write(&ring, fd, conns[i].buf, res);
                            break;
                        }
                    }
                } else {
                    // 连接关闭或错误
                    printf("连接关闭 fd=%d\n", fd);
                    close(fd);
                }
            }
        }

        // 标记所有CQE已消费
        if (count > 0) {
            io_uring_cq_advance(&ring, count);
        }
    }

    io_uring_queue_exit(&ring);
    close(listen_fd);
    return 0;
}
```

#### 3. 批量IO操作

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include "liburing.h"

#define QUEUE_DEPTH   32
#define NUM_FILES     8
#define BUF_SIZE      4096

int main(void) {
    struct io_uring ring;
    io_uring_queue_init(QUEUE_DEPTH, &ring, 0);

    // 打开多个文件
    int fds[NUM_FILES];
    char* buffers[NUM_FILES];
    const char* filenames[NUM_FILES] = {
        "file0.dat", "file1.dat", "file2.dat", "file3.dat",
        "file4.dat", "file5.dat", "file6.dat", "file7.dat"
    };

    // 创建测试文件
    for (int i = 0; i < NUM_FILES; i++) {
        fds[i] = open(filenames[i], O_RDWR | O_CREAT | O_TRUNC, 0644);
        char data[BUF_SIZE];
        snprintf(data, BUF_SIZE, "这是文件%d的数据，通过io_uring批量写入。", i);
        write(fds[i], data, strlen(data));
        lseek(fds[i], 0, SEEK_SET);
        buffers[i] = aligned_alloc(4096, BUF_SIZE);
    }

    // ===== 批量提交读请求 =====
    printf("批量提交%d个读请求...\n", NUM_FILES);

    for (int i = 0; i < NUM_FILES; i++) {
        struct io_uring_sqe* sqe = io_uring_get_sqe(&ring);
        io_uring_prep_read(sqe, fds[i], buffers[i], BUF_SIZE, 0);
        io_uring_sqe_set_data64(sqe, i);  // 用文件索引作为user_data
    }

    // 一次性提交所有请求
    int submitted = io_uring_submit(&ring);
    printf("提交了%d个请求\n", submitted);

    // 批量等待完成
    int completed = 0;
    while (completed < NUM_FILES) {
        struct io_uring_cqe* cqe;
        if (io_uring_wait_cqe(&ring, &cqe) < 0) break;

        int idx = io_uring_cqe_get_data64(cqe);
        printf("文件%d读取完成: %d字节\n", idx, cqe->res);
        if (cqe->res > 0) {
            buffers[idx][cqe->res] = '\0';
            printf("  内容: %s\n", buffers[idx]);
        }
        io_uring_cqe_seen(&ring, cqe);
        completed++;
    }

    // 清理
    for (int i = 0; i < NUM_FILES; i++) {
        close(fds[i]);
        free(buffers[i]);
        unlink(filenames[i]);
    }
    io_uring_queue_exit(&ring);
    return 0;
}
```

***

### 4. 常见问题

#### Q1：io_uring能完全替代epoll吗？

理论上可以。io_uring支持所有epoll能做的（网络IO），还支持epoll不能做的（文件IO、超时、信号等）。但目前io_uring还在快速发展中，epoll更成熟稳定。

#### Q2：io_uring需要特殊硬件吗？

不需要。io_uring是纯软件接口，任何Linux 5.1+内核都支持。但某些高级特性（如SQPOLL）在特定硬件上效果更好。

#### Q3：应该用原始接口还是liburing？

大多数场景用liburing。原始接口只在需要极致优化（如自定义内存布局、零分配）时才需要。liburing已经足够高效。

#### Q4：io_uring和Windows IOCP相比如何？

io_uring的设计借鉴了IOCP的完成通知模型，但更灵活。IOCP只支持异步完成，io_uring还支持缓冲区注册、链式请求等高级特性。编程模型上io_uring更复杂，但性能潜力更大。

#### Q5：生产环境可以用io_uring了吗？

可以。Redis 7.0、libuv、Rust的tokio等都已支持io_uring。但需要注意内核版本要求（5.1+，推荐5.10+），以及某些特性在不同内核版本间的差异。

***

### 5. 总结

io_uring代表了Linux异步IO的未来方向。通过共享环形缓冲区，它将应用和内核之间的通信从"系统调用"模式升级为"共享内存"模式，大幅减少了系统调用开销和上下文切换。虽然编程复杂度比epoll高，但liburing库和越来越多的框架支持正在降低使用门槛。对于新项目，特别是需要高吞吐量IO的项目，io_uring值得认真考虑。