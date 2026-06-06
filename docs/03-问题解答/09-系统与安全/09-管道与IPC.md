# 什么是管道与 IPC 机制
> 📖 相关章节：[错误处理与信号](../../01-C语言/13-错误处理与信号.md)、[进程与线程](../../01-C语言/24-进程与线程.md)、[网络编程](../../02-CPP/35-网络编程.md)

> "进程间通信是操作系统的血脉"——多进程协作的基石

***

### 1. 要点直击

IPC（Inter-Process Communication）是操作系统提供的进程间数据交换与同步机制，包括管道、消息队列、共享内存、信号量、信号和套接字等，各有适用场景与性能特征。

***

### 2. 管道（Pipe）

管道是最基本的 IPC 机制，提供单向字节流通信。

**匿名管道（pipe）**：

```cpp
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <sys/wait.h>

int main() {
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return 1;
    }

    pid_t pid = fork();
    if (pid == 0) {
        close(pipefd[1]);

        char buf[256];
        ssize_t n = read(pipefd[0], buf, sizeof(buf) - 1);
        buf[n] = '\0';
        printf("子进程收到: %s\n", buf);

        close(pipefd[0]);
    } else {
        close(pipefd[0]);

        const char* msg = "来自父进程的问候";
        write(pipefd[1], msg, strlen(msg));

        close(pipefd[1]);
        waitpid(pid, nullptr, 0);
    }
    return 0;
}
```

**管道特性**：

| 特性 | 说明 |
|------|------|
| 方向 | 单向（一端写，一端读） |
| 生命周期 | 随进程（进程退出即消失） |
| 亲缘关系 | 仅限父子进程间使用 |
| 缓冲区 | 65536 字节（Linux 默认） |
| 阻塞行为 | 读空管道阻塞，写满管道阻塞 |
| SIGPIPE | 读端关闭后写端触发 SIGPIPE |

**管道的阻塞语义**：

```cpp
// 读端行为
// 管道有数据 → read 返回读取字节数
// 管道无数据且写端打开 → read 阻塞
// 管道无数据且写端关闭 → read 返回 0（EOF）

// 写端行为
// 管道有空间 → write 返回写入字节数
// 管道已满 → write 阻塞
// 读端全部关闭 → write 触发 SIGPIPE
```

***

### 3. 命名管道（FIFO）

命名管道存在于文件系统中，允许无亲缘关系的进程通信。

**创建与使用 FIFO**：

```cpp
// 进程 A：写入端
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

int main() {
    const char* fifo_path = "/tmp/my_fifo";
    mkfifo(fifo_path, 0666);

    int fd = open(fifo_path, O_WRONLY);
    const char* msg = "通过 FIFO 发送的消息";
    write(fd, msg, strlen(msg) + 1);
    close(fd);
    return 0;
}
```

```cpp
// 进程 B：读取端
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <cstdio>

int main() {
    const char* fifo_path = "/tmp/my_fifo";

    int fd = open(fifo_path, O_RDONLY);
    char buf[256];
    ssize_t n = read(fd, buf, sizeof(buf));
    buf[n] = '\0';
    printf("收到: %s\n", buf);
    close(fd);
    return 0;
}
```

**匿名管道 vs 命名管道**：

| 维度 | 匿名管道（pipe） | 命名管道（FIFO） |
|------|-----------------|-----------------|
| 文件系统 | 不存在 | 存在为特殊文件 |
| 通信范围 | 仅父子进程 | 任意进程 |
| 创建方式 | `pipe()` | `mkfifo()` 或 `mknod()` |
| 生命周期 | 随进程 | 随文件（需手动删除） |
| 数据持久 | 否 | 否（内核缓冲区） |
| 用法 | `read/write` | `open + read/write` |

**非阻塞 FIFO**：

```cpp
// 非阻塞打开（读端不等待写端）
int fd = open(fifo_path, O_RDONLY | O_NONBLOCK);

// 非阻塞打开（写端不等待读端）
int fd = open(fifo_path, O_WRONLY | O_NONBLOCK);
```

***

### 4. 消息队列

消息队列提供结构化的消息传递，每条消息有类型标识。

**POSIX 消息队列**：

```cpp
#include <mqueue.h>
#include <cstdio>
#include <cstring>

int main() {
    struct mq_attr attr;
    attr.mq_flags = 0;
    attr.mq_maxmsg = 10;
    attr.mq_msgsize = 256;
    attr.mq_curmsgs = 0;

    mqd_t mq = mq_open("/test_queue",
        O_CREAT | O_RDWR, 0666, &attr);

    const char* msg = "Hello, Message Queue!";
    mq_send(mq, msg, strlen(msg) + 1, 1);

    char buf[256];
    unsigned int prio;
    ssize_t n = mq_receive(mq, buf, sizeof(buf), &prio);
    printf("收到消息(优先级%u): %s\n", prio, buf);

    mq_close(mq);
    mq_unlink("/test_queue");
    return 0;
}
```

**System V 消息队列**：

```cpp
#include <sys/ipc.h>
#include <sys/msg.h>
#include <cstdio>
#include <cstring>

struct MsgBuf {
    long mtype;
    char mtext[256];
};

int main() {
    key_t key = ftok("/tmp", 'Q');
    int msqid = msgget(key, IPC_CREAT | 0666);

    MsgBuf snd;
    snd.mtype = 1;
    strcpy(snd.mtext, "System V 消息");
    msgsnd(msqid, &snd, sizeof(snd.mtext), 0);

    MsgBuf rcv;
    msgrcv(msqid, &rcv, sizeof(rcv.mtext), 1, 0);
    printf("收到: %s\n", rcv.mtext);

    msgctl(msqid, IPC_RMID, nullptr);
    return 0;
}
```

**POSIX vs System V 消息队列**：

| 维度 | POSIX | System V |
|------|-------|----------|
| 标识 | 名称字符串 | key_t 整数 |
| 优先级 | 支持 | 通过 mtype 模拟 |
| 通知 | 支持异步通知 | 不支持 |
| 查看工具 | `ls /dev/mqueue/` | `ipcs -q` |
| 可移植性 | Linux/macOS | Linux/Unix |

***

### 5. 共享内存

共享内存是最快的 IPC 方式，进程直接读写同一块物理内存。

**POSIX 共享内存**：

```cpp
// 写入进程
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>

struct SharedBuf {
    int ready;
    char data[4096];
};

int main() {
    int fd = shm_open("/my_shm", O_CREAT | O_RDWR, 0666);
    ftruncate(fd, sizeof(SharedBuf));

    auto* buf = static_cast<SharedBuf*>(
        mmap(nullptr, sizeof(SharedBuf), PROT_READ | PROT_WRITE,
             MAP_SHARED, fd, 0));
    close(fd);

    strcpy(buf->data, "共享内存中的数据");
    __sync_synchronize();
    buf->ready = 1;

    printf("写入完成\n");
    munmap(buf, sizeof(SharedBuf));
    return 0;
}
```

```cpp
// 读取进程
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstdio>

struct SharedBuf {
    int ready;
    char data[4096];
};

int main() {
    int fd = shm_open("/my_shm", O_RDWR, 0);

    auto* buf = static_cast<SharedBuf*>(
        mmap(nullptr, sizeof(SharedBuf), PROT_READ | PROT_WRITE,
             MAP_SHARED, fd, 0));
    close(fd);

    while (buf->ready == 0) {
        __sync_synchronize();
        usleep(1000);
    }

    printf("读取: %s\n", buf->data);
    munmap(buf, sizeof(SharedBuf));
    shm_unlink("/my_shm");
    return 0;
}
```

**共享内存必须配合同步机制**：

```cpp
// 使用 POSIX 信号量同步共享内存
#include <semaphore.h>

struct SharedWithSem {
    sem_t sem;
    int data;
};

// 初始化
sem_init(&buf->sem, 1, 0);

// 写入后通知
buf->data = 42;
sem_post(&buf->sem);

// 读取前等待
sem_wait(&buf->sem);
printf("data = %d\n", buf->data);
```

***

### 6. 信号量

信号量用于进程间同步，控制对共享资源的访问。

**POSIX 有名信号量**：

```cpp
#include <semaphore.h>
#include <fcntl.h>
#include <cstdio>

int main() {
    sem_t* sem = sem_open("/my_sem", O_CREAT, 0666, 1);

    sem_wait(sem);
    printf("进入临界区\n");
    // ... 临界区操作 ...
    sem_post(sem);

    sem_close(sem);
    sem_unlink("/my_sem");
    return 0;
}
```

**POSIX 无名信号量**：

```cpp
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdio>

int main() {
    auto* sem = static_cast<sem_t*>(
        mmap(nullptr, sizeof(sem_t), PROT_READ | PROT_WRITE,
             MAP_SHARED | MAP_ANONYMOUS, -1, 0));
    sem_init(sem, 1, 1);

    int counter = 0;
    pid_t pid = fork();
    if (pid == 0) {
        for (int i = 0; i < 100000; ++i) {
            sem_wait(sem);
            ++counter;
            sem_post(sem);
        }
        _exit(0);
    } else {
        for (int i = 0; i < 100000; ++i) {
            sem_wait(sem);
            ++counter;
            sem_post(sem);
        }
        waitpid(pid, nullptr, 0);
        printf("counter = %d\n", counter);
    }

    sem_destroy(sem);
    munmap(sem, sizeof(sem_t));
    return 0;
}
```

**System V 信号量**：

```cpp
#include <sys/sem.h>
#include <sys/ipc.h>
#include <cstdio>

union semun {
    int val;
    struct semid_ds* buf;
    unsigned short* array;
};

int main() {
    key_t key = ftok("/tmp", 'S');
    int semid = semget(key, 1, IPC_CREAT | 0666);

    semun arg;
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg);

    struct sembuf op_wait = {0, -1, 0};
    struct sembuf op_post = {0, 1, 0};

    semop(semid, &op_wait, 1);
    // 临界区
    semop(semid, &op_post, 1);

    semctl(semid, 0, IPC_RMID);
    return 0;
}
```

***

### 7. 信号（Signal）

信号是异步通知机制，用于通知进程发生了某个事件。

**常见信号**：

| 信号 | 编号 | 含义 | 默认行为 |
|------|------|------|---------|
| SIGHUP | 1 | 终端挂断 | 终止 |
| SIGINT | 2 | Ctrl+C | 终止 |
| SIGQUIT | 3 | Ctrl+\ | 终止+核心转储 |
| SIGKILL | 9 | 强制终止 | 终止（不可捕获） |
| SIGSEGV | 11 | 段错误 | 终止+核心转储 |
| SIGPIPE | 13 | 管道破裂 | 终止 |
| SIGTERM | 15 | 请求终止 | 终止 |
| SIGUSR1 | 10 | 用户自定义1 | 终止 |
| SIGUSR2 | 12 | 用户自定义2 | 终止 |
| SIGCHLD | 17 | 子进程状态变化 | 忽略 |

**信号处理**：

```cpp
#include <csignal>
#include <cstdio>
#include <unistd.h>
#include <atomic>

static std::atomic<bool> g_running{true};

void signal_handler(int signo) {
    printf("收到信号: %d\n", signo);
    g_running = false;
}

int main() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;

    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    signal(SIGPIPE, SIG_IGN);

    while (g_running) {
        printf("运行中...\n");
        sleep(1);
    }

    printf("优雅退出\n");
    return 0;
}
```

**信号用于 IPC 的局限**：

| 局限 | 说明 |
|------|------|
| 信息量少 | 仅传递信号编号，无附加数据 |
| 异步性 | 随时打断程序，需小心可重入性 |
| 不可靠 | 标准信号可能丢失（不排队） |
| 实时信号 | SIGRTMIN~SIGRTMAX 可排队，可携带数据 |

```cpp
// 实时信号携带数据
#include <csignal>

void rt_handler(int signo, siginfo_t* info, void* context) {
    printf("实时信号 %d, 值=%d, 发送者=%d\n",
           signo, info->si_value.sival_int, info->si_pid);
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = rt_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGRTMIN, &sa, nullptr);

    union sigval value;
    value.sival_int = 42;
    sigqueue(getpid(), SIGRTMIN, value);

    pause();
    return 0;
}
```

***

### 8. 套接字（Socket）

套接字是最通用的 IPC 机制，支持本机和网络通信。

**UNIX 域套接字（本地 IPC）**：

```cpp
// 服务端
#include <sys/socket.h>
#include <sys/un.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>

int main() {
    int server_fd = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/tmp/unix_socket");

    unlink("/tmp/unix_socket");
    bind(server_fd, (struct sockaddr*)&addr, sizeof(addr));
    listen(server_fd, 5);

    int client_fd = accept(server_fd, nullptr, nullptr);

    char buf[256];
    ssize_t n = read(client_fd, buf, sizeof(buf) - 1);
    buf[n] = '\0';
    printf("服务端收到: %s\n", buf);

    const char* reply = "服务端回复";
    write(client_fd, reply, strlen(reply));

    close(client_fd);
    close(server_fd);
    unlink("/tmp/unix_socket");
    return 0;
}
```

```cpp
// 客户端
#include <sys/socket.h>
#include <sys/un.h>
#include <cstdio>
#include <cstring>
#include <unistd.h>

int main() {
    int sock = socket(AF_UNIX, SOCK_STREAM, 0);

    struct sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, "/tmp/unix_socket");

    connect(sock, (struct sockaddr*)&addr, sizeof(addr));

    const char* msg = "客户端请求";
    write(sock, msg, strlen(msg));

    char buf[256];
    ssize_t n = read(sock, buf, sizeof(buf) - 1);
    buf[n] = '\0';
    printf("客户端收到: %s\n", buf);

    close(sock);
    return 0;
}
```

**UNIX 域套接字传递文件描述符**：

```cpp
#include <sys/socket.h>
#include <cstdio>
#include <unistd.h>

void send_fd(int sock, int fd) {
    struct msghdr msg = {};
    struct iovec iov;
    char buf[1] = {0};
    iov.iov_base = buf;
    iov.iov_len = 1;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    char cmsg_buf[CMSG_SPACE(sizeof(int))];
    msg.msg_control = cmsg_buf;
    msg.msg_controllen = sizeof(cmsg_buf);

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    cmsg->cmsg_len = CMSG_LEN(sizeof(int));
    cmsg->cmsg_level = SOL_SOCKET;
    cmsg->cmsg_type = SCM_RIGHTS;
    *reinterpret_cast<int*>(CMSG_DATA(cmsg)) = fd;

    sendmsg(sock, &msg, 0);
}

int recv_fd(int sock) {
    struct msghdr msg = {};
    struct iovec iov;
    char buf[1];
    iov.iov_base = buf;
    iov.iov_len = 1;
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    char cmsg_buf[CMSG_SPACE(sizeof(int))];
    msg.msg_control = cmsg_buf;
    msg.msg_controllen = sizeof(cmsg_buf);

    recvmsg(sock, &msg, 0);

    struct cmsghdr* cmsg = CMSG_FIRSTHDR(&msg);
    return *reinterpret_cast<int*>(CMSG_DATA(cmsg));
}
```

***

### 9. IPC 机制全面对比与选型

| 机制 | 方向 | 数据量 | 速度 | 同步 | 持久性 | 适用场景 |
|------|------|--------|------|------|--------|---------|
| 匿名管道 | 单向 | 字节流 | 中 | 阻塞 | 进程级 | 父子进程命令流 |
| FIFO | 单向 | 字节流 | 中 | 阻塞 | 文件级 | 无亲缘进程简单通信 |
| 消息队列 | 双向 | 结构化消息 | 中 | 非阻塞可选 | 内核级 | 任务分发、请求响应 |
| 共享内存 | 双向 | 大块数据 | 最快 | 需外部同步 | 内核级 | 大数据共享、图像传输 |
| 信号量 | N/A | 计数器 | 快 | 同步原语 | 内核级 | 资源计数、互斥 |
| 信号 | 异步 | 极少 | 快 | 异步 | 无 | 事件通知、优雅退出 |
| UNIX Socket | 双向 | 字节流/数据报 | 快 | 阻塞/非阻塞 | 连接级 | 通用本地通信 |
| 网络 Socket | 双向 | 字节流 | 较慢 | 阻塞/非阻塞 | 连接级 | 跨机器通信 |

**选型决策树**：

```
需要 IPC？
├── 是否跨机器？
│   └── 是 → 网络 Socket (TCP/UDP)
│   └── 否 → 本地 IPC
│       ├── 数据量？
│       │   ├── 大块数据（>1MB）→ 共享内存 + 信号量
│       │   ├── 结构化消息 → 消息队列
│       │   └── 字节流 → 管道/FIFO/UNIX Socket
│       ├── 是否有亲缘关系？
│       │   ├── 父子进程 → 匿名管道
│       │   └── 无关进程 → FIFO/UNIX Socket/消息队列
│       ├── 只需通知？
│       │   └── 是 → 信号
│       └── 需要传递文件描述符？
│           └── 是 → UNIX Socket (SCM_RIGHTS)
```

***

### 10. Windows 上的 IPC

Windows 提供不同的 IPC 机制，部分与 POSIX 对应。

**Windows 命名管道**：

```cpp
#include <windows.h>
#include <cstdio>

int main() {
    HANDLE hPipe = CreateNamedPipeA(
        "\\\\.\\pipe\\my_pipe",
        PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT,
        1, 4096, 4096, 0, nullptr);

    ConnectNamedPipe(hPipe, nullptr);

    char buf[256];
    DWORD bytesRead;
    ReadFile(hPipe, buf, sizeof(buf), &bytesRead, nullptr);
    buf[bytesRead] = '\0';
    printf("收到: %s\n", buf);

    const char* reply = "Windows 管道回复";
    DWORD bytesWritten;
    WriteFile(hPipe, reply, strlen(reply), &bytesWritten, nullptr);

    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);
    return 0;
}
```

```cpp
// 客户端
#include <windows.h>
#include <cstdio>

int main() {
    HANDLE hPipe = CreateFileA(
        "\\\\.\\pipe\\my_pipe",
        GENERIC_READ | GENERIC_WRITE,
        0, nullptr, OPEN_EXISTING, 0, nullptr);

    const char* msg = "Windows 管道消息";
    DWORD bytesWritten;
    WriteFile(hPipe, msg, strlen(msg), &bytesWritten, nullptr);

    char buf[256];
    DWORD bytesRead;
    ReadFile(hPipe, buf, sizeof(buf), &bytesRead, nullptr);
    buf[bytesRead] = '\0';
    printf("收到: %s\n", buf);

    CloseHandle(hPipe);
    return 0;
}
```

**Linux vs Windows IPC 对照**：

| 功能 | Linux | Windows |
|------|-------|---------|
| 匿名管道 | `pipe()` | `CreatePipe()` |
| 命名管道 | `mkfifo()` | `CreateNamedPipe()` |
| 消息队列 | POSIX/System V | `PostMessage()`/`SendMessage()` |
| 共享内存 | `shm_open()`/`mmap()` | `CreateFileMapping()`/`MapViewOfFile()` |
| 信号量 | POSIX/System V | `CreateSemaphore()` |
| 信号 | `kill()`/`sigaction()` | 无直接等价 |
| 本地套接字 | UNIX Socket | 命名管道（替代） |
| 事件 | `eventfd()` | `CreateEvent()` |
| 互斥体 | `pthread_mutex_t` | `CreateMutex()` |

**跨平台 IPC 封装思路**：

```cpp
class IpcChannel {
public:
    bool send(const void* data, size_t len);
    bool receive(void* buf, size_t buf_size, size_t* received);

#ifdef _WIN32
    HANDLE handle_ = INVALID_HANDLE_VALUE;
#else
    int fd_ = -1;
#endif
};
```

***

### 11. 极简总结

| 要点 | 内容 |
|------|------|
| 管道 | 单向字节流，匿名(pipe)限父子进程，FIFO 可跨进程 |
| 消息队列 | 结构化消息，支持优先级，异步非阻塞可选 |
| 共享内存 | 最快 IPC，需配合信号量/互斥锁同步 |
| 信号量 | 计数同步原语，有名/无名、POSIX/System V |
| 信号 | 异步通知，信息量少，不可靠（标准信号） |
| 套接字 | 最通用，UNIX 域本地高效，网络域跨机器 |
| 选型关键 | 数据量、方向、亲缘关系、同步需求、跨机器否 |
| Windows | 命名管道、CreateFileMapping、CreateSemaphore 等 |

***

### 相关阅读

- [CPP网络编程进阶](../10-工程实践/22-CPP网络编程进阶.md)
- [什么是I-O多路复用](../06-并发编程/23-什么是I-O多路复用.md)
- [操作系统接口编程](./06-操作系统接口编程.md)

***