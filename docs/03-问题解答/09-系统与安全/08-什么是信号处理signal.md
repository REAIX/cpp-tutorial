# 什么是信号处理 signal
> 📖 相关章节：[错误处理与信号](../../01-C语言/13-错误处理与信号.md)、[进程与线程](../../01-C语言/23-进程与线程.md)、[网络编程](../../02-CPP/31-网络编程.md)

> 信号是操作系统发给进程的"快递"，你可以签收处理，也可以拒收，但 SIGKILL 除外。

***

### 1. 核心速览

信号是操作系统向进程发送的异步通知机制，用于报告异常事件（如段错误）或外部请求（如 Ctrl+C），进程可通过注册信号处理函数来自定义响应行为。

***

### 2. 信号类型与分类

信号本质是一个小的整数消息，由内核或其他进程发送给目标进程。不同信号代表不同事件。

**标准信号一览：**

| 信号 | 值（常见） | 来源 | 默认行为 | 可捕获 |
|------|-----------|------|----------|--------|
| SIGHUP | 1 | 终端挂断 | 终止 | ✅ |
| SIGINT | 2 | Ctrl+C | 终止 | ✅ |
| SIGQUIT | 3 | Ctrl+\ | 终止+core | ✅ |
| SIGILL | 4 | 非法指令 | 终止+core | ✅ |
| SIGABRT | 6 | abort() | 终止+core | ✅ |
| SIGFPE | 8 | 算术异常 | 终止+core | ✅ |
| SIGKILL | 9 | 强制终止 | 终止 | ❌ |
| SIGSEGV | 11 | 段错误 | 终止+core | ✅（但危险） |
| SIGPIPE | 13 | 管道断开 | 终止 | ✅ |
| SIGALRM | 14 | alarm 定时 | 终止 | ✅ |
| SIGTERM | 15 | 请求终止 | 终止 | ✅ |
| SIGUSR1 | 10 | 用户自定义 | 终止 | ✅ |
| SIGUSR2 | 12 | 用户自定义 | 终止 | ✅ |
| SIGCHLD | 17 | 子进程状态 | 忽略 | ✅ |
| SIGSTOP | 19 | 停止进程 | 停止 | ❌ |
| SIGTSTP | 20 | Ctrl+Z | 停止 | ✅ |

信号分类：

| 分类 | 说明 | 示例 |
|------|------|------|
| 终止信号 | 默认终止进程 | SIGTERM, SIGINT, SIGHUP |
| 核心转储信号 | 终止并生成 core 文件 | SIGSEGV, SIGABRT, SIGFPE |
| 停止信号 | 暂停进程执行 | SIGSTOP, SIGTSTP |
| 继续信号 | 恢复停止的进程 | SIGCONT |
| 忽略信号 | 默认被忽略 | SIGCHLD, SIGURG |

***

### 3. signal() 函数

`signal()` 是最早的信号注册接口，C 标准库提供，跨平台可用但功能有限。

```cpp
#include <csignal>
#include <iostream>
#include <unistd.h>

void sigint_handler(int signo) {
    std::cout << "received SIGINT (" << signo << ")\n";
}

int main() {
    signal(SIGINT, sigint_handler);
    std::cout << "press Ctrl+C...\n";
    while (true) {
        sleep(1);
    }
    return 0;
}
```

`signal()` 的特殊值：

| 值 | 含义 |
|------|------|
| `SIG_DFL` | 恢复默认处理 |
| `SIG_IGN` | 忽略该信号 |
| 自定义函数指针 | 注册处理函数 |

```cpp
signal(SIGPIPE, SIG_IGN);
signal(SIGINT, SIG_DFL);
```

`signal()` 的严重缺陷：

| 缺陷 | 说明 |
|------|------|
| 不可移植 | 不同系统行为不一致 |
| 信号重置 | 某些系统触发后重置为 SIG_DFL |
| 不传递额外信息 | 只有信号编号 |
| 无法屏蔽信号 | 处理期间无法阻塞其他信号 |
| 竞态条件 | 非原子操作 |

因此，POSIX 系统推荐使用 `sigaction()`。

***

### 4. sigaction() 函数

`sigaction()` 是 POSIX 标准的信号注册接口，功能完善且行为确定。

```cpp
Linux:
#include <csignal>
#include <iostream>
#include <unistd.h>

void handler(int signo, siginfo_t* info, void* context) {
    std::cout << "signal " << signo
              << " from pid=" << info->si_pid
              << " uid=" << info->si_uid << "\n";
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO | SA_RESTART;

    sigaction(SIGUSR1, &sa, nullptr);
    sigaction(SIGINT, &sa, nullptr);

    std::cout << "pid=" << getpid() << "\n";
    while (true) sleep(1);
    return 0;
}
```

`sigaction` 的关键标志位：

| 标志 | 说明 |
|------|------|
| `SA_SIGINFO` | 使用三参数处理函数，传递 siginfo_t |
| `SA_RESTART` | 自动重启被信号中断的系统调用 |
| `SA_NOCLDSTOP` | 子进程停止时不发 SIGCHLD |
| `SA_NOCLDWAIT` | 子进程终止时不产生僵尸进程 |
| `SA_NODEFER` | 处理期间不自动屏蔽同一信号 |

`siginfo_t` 结构体关键成员：

| 成员 | 说明 |
|------|------|
| `si_signo` | 信号编号 |
| `si_code` | 信号来源代码 |
| `si_pid` | 发送进程 PID |
| `si_uid` | 发送进程 UID |
| `si_addr` | 触发地址（SIGSEGV 等） |
| `si_value` | 实时信号附带数据 |

`signal()` vs `sigaction()` 对比：

| 维度 | signal() | sigaction() |
|------|----------|-------------|
| 标准 | C 标准 | POSIX |
| 可移植性 | 广但行为不一致 | POSIX 系统一致 |
| 信号信息 | 仅编号 | siginfo_t 详细信息 |
| 信号屏蔽 | 不支持 | sa_mask 支持 |
| 触发后行为 | 可能重置 | 持久注册 |
| 系统调用重启 | 不保证 | SA_RESTART 可控 |

***

### 5. 异步信号安全函数

信号处理函数在异步上下文中执行，随时可能打断正常控制流。因此，并非所有标准库函数都可以在信号处理函数中安全调用——只有**异步信号安全（async-signal-safe）**函数才能使用。

```cpp
#include <csignal>
#include <iostream>
#include <cstring>

volatile sig_atomic_t g_interrupted = 0;

void unsafe_handler(int signo) {
    std::cout << "interrupted!\n";
    g_interrupted = 1;
}

void safe_handler(int signo) {
    const char msg[] = "interrupted!\n";
    write(STDOUT_FILENO, msg, sizeof(msg) - 1);
    g_interrupted = 1;
}
```

`unsafe_handler` 中使用 `std::cout` 是不安全的，因为 `cout` 内部有缓冲区和锁，可能已被打断。`safe_handler` 使用 `write()` 是安全的。

POSIX 定义的异步信号安全函数（部分）：

| 安全函数 | 说明 |
|----------|------|
| `write()` | 系统调用，无锁 |
| `read()` | 系统调用 |
| `_exit()` | 直接退出 |
| `signal()` | 信号操作 |
| `sigprocmask()` | 信号屏蔽 |
| `getpid()` | 获取 PID |
| `abort()` | 终止进程 |

**绝对不安全**的函数：

| 不安全函数 | 原因 |
|------------|------|
| `printf()`/`std::cout` | 内部缓冲区+锁 |
| `malloc()`/`free()` | 堆锁 |
| `new`/`delete` | 调用 malloc/free |
| `pthread_mutex_lock()` | 可能死锁 |
| `std::string` 操作 | 动态内存分配 |

**推荐模式：信号处理函数只设标志，主循环检查标志：**

```cpp
#include <csignal>
#include <unistd.h>

volatile sig_atomic_t g_running = 1;

void signal_handler(int) {
    g_running = 0;
}

int main() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    while (g_running) {
        sleep(1);
    }
    return 0;
}
```

***

### 6. 竞态条件与 sig_atomic_t

信号处理函数与主程序共享数据时存在竞态条件。`volatile sig_atomic_t` 是 C 标准保证的原子访问类型。

```cpp
#include <csignal>

volatile sig_atomic_t g_flag = 0;

void handler(int) {
    g_flag = 1;
}

int main() {
    signal(SIGINT, handler);
    while (g_flag == 0) {
    }
    return 0;
}
```

`volatile` 告诉编译器不要缓存该变量，每次必须从内存读取。`sig_atomic_t` 保证读写是原子的。

**常见错误——使用非原子类型：**

```cpp
#include <csignal>
#include <string>

std::string g_message;

void bad_handler(int) {
    g_message = "interrupted";
}
```

`std::string` 赋值涉及内存分配和拷贝，不是原子操作，信号处理函数中绝对不能使用。

**信号屏蔽解决竞态：**

```cpp
Linux:
#include <csignal>
#include <iostream>

int g_counter = 0;

void safe_increment() {
    sigset_t old_mask, block_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGINT);

    sigprocmask(SIG_BLOCK, &block_mask, &old_mask);
    ++g_counter;
    sigprocmask(SIG_SETMASK, &old_mask, nullptr);
}
```

| 方法 | 适用场景 | 复杂度 |
|------|----------|--------|
| `volatile sig_atomic_t` | 简单标志 | 低 |
| 信号屏蔽 | 保护临界区 | 中 |
| 自旋锁/原子操作 | 高频更新 | 高（注意死锁） |
| pipe/socketpair | 传递复杂数据 | 中 |

**pipe 模式——安全传递复杂信息：**

```cpp
Linux:
#include <csignal>
#include <unistd.h>
#include <iostream>

int g_pipe_fd[2];

void handler(int signo) {
    write(g_pipe_fd[1], &signo, sizeof(signo));
}

int main() {
    pipe(g_pipe_fd);

    struct sigaction sa;
    sa.sa_handler = handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    while (true) {
        int signo;
        auto n = read(g_pipe_fd[0], &signo, sizeof(signo));
        if (n > 0) {
            std::cout << "handled signal " << signo << "\n";
            if (signo == SIGTERM) break;
        }
    }
    close(g_pipe_fd[0]);
    close(g_pipe_fd[1]);
    return 0;
}
```

***

### 7. 优雅关闭

服务端程序收到 SIGTERM 时应优雅关闭：停止接受新连接、处理完现有请求、释放资源。

```cpp
Linux:
#include <csignal>
#include <iostream>
#include <unistd.h>
#include <atomic>

std::atomic<bool> g_running{true};
std::atomic<bool> g_draining{false};

void graceful_handler(int signo) {
    if (signo == SIGTERM || signo == SIGINT) {
        g_draining = true;
        g_running = false;
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = graceful_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, nullptr);
    sigaction(SIGTERM, &sa, nullptr);

    while (g_running) {
        accept_connection();
        process_request();
    }

    std::cout << "draining remaining requests...\n";
    while (has_pending_requests()) {
        process_request();
    }

    cleanup_resources();
    std::cout << "graceful shutdown complete\n";
    return 0;
}
```

优雅关闭的典型流程：

| 阶段 | 动作 |
|------|------|
| 1. 收到信号 | 设置停止标志 |
| 2. 停止接受 | 不再 accept 新连接 |
| 3. 排空请求 | 处理完进行中的请求 |
| 4. 释放资源 | 关闭文件、数据库连接 |
| 5. 退出 | 返回退出码 |

超时保护——防止优雅关闭无限等待：

```cpp
Linux:
#include <csignal>
#include <atomic>
#include <thread>
#include <chrono>

std::atomic<bool> g_running{true};

void shutdown_handler(int) {
    g_running = false;
}

void watchdog() {
    auto start = std::chrono::steady_clock::now();
    while (!g_running) {
        auto elapsed = std::chrono::steady_clock::now() - start;
        if (elapsed > std::chrono::seconds(30)) {
            _exit(1);
        }
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

int main() {
    signal(SIGTERM, shutdown_handler);
    if (!g_running) {
        std::thread(watchdog).detach();
    }
}
```

***

### 8. 崩溃处理

当程序收到 SIGSEGV、SIGABRT 等信号时，可以生成崩溃报告辅助调试。

```cpp
Linux:
#include <csignal>
#include <execinfo.h>
#include <unistd.h>
#include <iostream>
#include <cstdlib>

void crash_handler(int signo) {
    const char* name = "UNKNOWN";
    switch (signo) {
        case SIGSEGV: name = "SIGSEGV"; break;
        case SIGABRT: name = "SIGABRT"; break;
        case SIGFPE:  name = "SIGFPE";  break;
        case SIGILL:  name = "SIGILL";  break;
    }

    void* buffer[128];
    int n = backtrace(buffer, 128);
    std::cerr << "crash: " << name << " (signal " << signo << ")\n";
    std::cerr << "backtrace:\n";
    backtrace_symbols_fd(buffer, n, STDERR_FILENO);
    _exit(128 + signo);
}

int main() {
    struct sigaction sa;
    sa.sa_handler = crash_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGFPE, &sa, nullptr);
    sigaction(SIGILL, &sa, nullptr);

    int* p = nullptr;
    *p = 42;
    return 0;
}
```

Windows 下的崩溃处理：

```cpp
Windows:
#include <windows.h>
#include <dbghelp.h>
#include <iostream>

LONG WINAPI exception_handler(EXCEPTION_POINTERS* ep) {
    std::cerr << "exception code: " << ep->ExceptionRecord->ExceptionCode << "\n";
    MINIDUMP_EXCEPTION_INFORMATION mei;
    mei.ThreadId = GetCurrentThreadId();
    mei.ExceptionPointers = ep;
    mei.ClientPointers = FALSE;

    HANDLE hFile = CreateFileA("crash.dmp", GENERIC_WRITE, 0, nullptr,
                               CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
                      hFile, MiniDumpWithDataSegs, &mei, nullptr, nullptr);
    CloseHandle(hFile);
    return EXCEPTION_EXECUTE_HANDLER;
}

int main() {
    SetUnhandledExceptionFilter(exception_handler);
    int* p = nullptr;
    *p = 42;
    return 0;
}
```

| 平台 | 崩溃捕获 | 堆栈回溯 | 转储文件 |
|------|----------|----------|----------|
| Linux | signal + sigaction | backtrace() | core dump |
| Windows | SetUnhandledExceptionFilter | StackWalk64 | MiniDumpWriteDump |

***

### 9. 信号 vs 异常

信号和异常是两种不同的错误处理机制，各有适用场景。

```cpp
#include <csignal>
#include <iostream>
#include <stdexcept>

void signal_approach() {
    int* p = nullptr;
    *p = 42;
}

void exception_approach() {
    throw std::runtime_error("bad operation");
}

int main() {
    signal(SIGSEGV, [](int) {
        std::cerr << "segfault caught\n";
        _exit(1);
    });

    try {
        exception_approach();
    } catch (const std::exception& e) {
        std::cerr << "exception: " << e.what() << "\n";
    }
}
```

| 维度 | 信号 | 异常 |
|------|------|------|
| 触发源 | 操作系统/外部进程 | 程序自身 |
| 同步性 | 异步 | 同步 |
| 可预测性 | 不可预测 | 可预测 |
| 处理限制 | 仅可调用异步信号安全函数 | 无限制 |
| 栈展开 | 不展开 | 自动展开 |
| 资源清理 | 需手动 | RAII 自动 |
| 跨语言 | C 接口，通用 | C++ 特有 |

核心区别：**异常是程序可控的错误处理流程，信号是操作系统强加的中断**。不要试图用信号替代异常，也不要在信号处理函数中抛出异常。

***

### 10. POSIX 实时信号

POSIX 实时信号（Real-time Signals）扩展了标准信号，提供排队、携带数据和有序传递能力。

```cpp
Linux:
#include <csignal>
#include <iostream>
#include <unistd.h>
#include <csignal>

void rt_handler(int signo, siginfo_t* info, void* ctx) {
    std::cout << "RT signal " << signo
              << " value=" << info->si_value.sival_int
              << " from pid=" << info->si_pid << "\n";
}

int main() {
    struct sigaction sa;
    sa.sa_sigaction = rt_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_SIGINFO;
    sigaction(SIGRTMIN, &sa, nullptr);

    union sigval sv;
    sv.sival_int = 42;
    sigqueue(getpid(), SIGRTMIN, sv);

    sv.sival_int = 99;
    sigqueue(getpid(), SIGRTMIN, sv);

    sleep(1);
    return 0;
}
```

标准信号 vs 实时信号：

| 维度 | 标准信号 | 实时信号 |
|------|----------|----------|
| 编号范围 | 1~31 | SIGRTMIN~SIGRTMAX |
| 排队 | 不排队，同一信号只计一次 | 排队，每个信号都传递 |
| 携带数据 | 无 | siginfo_t + sigval |
| 传递顺序 | 不保证 | FIFO |
| 编号范围 | 固定名称 | 可自由选择 |

实时信号的应用场景：

- 精确事件通知（携带数据避免额外查询）
- 高优先级中断模拟
- 进程间通信（携带整数值）

***

### 11. Linux 与 Windows 信号差异

| 维度 | Linux | Windows |
|------|-------|---------|
| 信号标准 | POSIX 完整支持 | 有限子集 |
| sigaction | ✅ | ❌（仅 signal()） |
| 实时信号 | ✅ SIGRTMIN~SIGRTMAX | ❌ |
| SIGKILL/SIGSTOP | 不可捕获/忽略 | 无等价信号 |
| backtrace | ✅ | ❌（用 StackWalk） |
| core dump | 自动生成 | 需 MiniDumpWriteDump |
| Ctrl+C | SIGINT | SIGINT（控制台） |
| 段错误 | SIGSEGV | EXCEPTION_ACCESS_VIOLATION |

Windows 下的信号替代方案：

```cpp
Windows:
#include <windows.h>
#include <iostream>

BOOL WINAPI console_handler(DWORD type) {
    switch (type) {
        case CTRL_C_EVENT:
            std::cout << "Ctrl+C\n";
            return TRUE;
        case CTRL_BREAK_EVENT:
            std::cout << "Ctrl+Break\n";
            return TRUE;
        case CTRL_CLOSE_EVENT:
            std::cout << "console closing\n";
            return TRUE;
        case CTRL_SHUTDOWN_EVENT:
            std::cout << "system shutting down\n";
            return TRUE;
    }
    return FALSE;
}

int main() {
    SetConsoleCtrlHandler(console_handler, TRUE);
    Sleep(INFINITE);
    return 0;
}
```

跨平台信号处理封装：

```cpp
#include <csignal>

#if defined(_WIN32)
    #include <windows.h>
#endif

class SignalHandler {
    static volatile sig_atomic_t running_;
public:
    static void init() {
        std::signal(SIGINT, on_signal);
        std::signal(SIGTERM, on_signal);
        #if defined(_WIN32)
            SetConsoleCtrlHandler(win_handler, TRUE);
        #endif
    }
    static bool is_running() { return running_ != 0; }

private:
    static void on_signal(int) { running_ = 0; }
    #if defined(_WIN32)
    static BOOL WINAPI win_handler(DWORD) { running_ = 0; return TRUE; }
    #endif
};

volatile sig_atomic_t SignalHandler::running_ = 1;
```

***

### 12. 极简总结

| 要点 | 内容 |
|------|------|
| 本质 | 操作系统向进程发送的异步通知 |
| 常见信号 | SIGINT(2)/SIGTERM(15)/SIGSEGV(11)/SIGKILL(9) |
| 注册方式 | signal()（简单）/ sigaction()（推荐，POSIX） |
| 安全约束 | 处理函数中只能调用异步信号安全函数 |
| 共享数据 | 使用 volatile sig_atomic_t 或信号屏蔽 |
| 优雅关闭 | 设标志 → 排空请求 → 释放资源 → 退出 |
| 崩溃处理 | backtrace(Linux)/MiniDump(Windows) |
| vs 异常 | 信号是异步中断，异常是同步流程 |
| 实时信号 | 排队传递、携带数据、FIFO 顺序 |
| 平台差异 | Linux 完整 POSIX，Windows 仅子集 |

核心记忆口诀：**信号是异步的，处理要安全；sigaction 优先，SIGKILL 不可挡**。

***

### 相关阅读

- [僵尸进程孤儿进程与守护进程](./07-僵尸进程孤儿进程与守护进程.md)
- [守护进程创建](./10-守护进程创建.md)
- [什么是setjmp与longjmp](./02-什么是setjmp与longjmp.md)