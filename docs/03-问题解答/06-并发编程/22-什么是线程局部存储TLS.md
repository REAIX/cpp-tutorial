# 什么是线程局部存储 TLS
> 📖 相关章节：[多线程基础](../../02-CPP/29-多线程基础.md)、[线程同步](../../02-CPP/30-线程同步.md)、[原子操作](../../02-CPP/31-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

> "每个员工有自己的工位储物柜，不共用，不需要锁"——TLS 让每个线程独享变量副本，天然线程安全。

***

### 1. 核心定义

**TLS（Thread Local Storage，线程局部存储）** = 每个线程拥有独立的变量副本，互不干扰。一个线程修改自己的副本，不会影响其他线程的副本。

关键点：**同一个变量名，每个线程各持一份，彼此隔离，天然无竞争**。

***

### 2. 生活类比

**工位储物柜**：

想象一个办公室里有 10 个员工（线程）：

- **全局变量** = 公共储物柜，所有人共用 → 拿东西要排队（加锁）
- **局部变量** = 随身口袋，出了函数就没了 → 生命周期太短
- **TLS** = 每人一个专属储物柜，贴了名字 → 不需要排队，不需要锁，随时用

| 方式 | 类比 | 需要锁？ | 生命周期 |
|------|------|:---:|------|
| 全局变量 | 公共储物柜 | 需要 | 整个程序 |
| 局部变量 | 随身口袋 | 不需要 | 函数内 |
| TLS | 专属储物柜 | 不需要 | 线程内 |

***

### 3. thread_local 关键字用法

C++11 引入 `thread_local` 关键字，声明线程局部存储变量。

#### 1. 基本语法

```cpp
thread_local int tls_value = 0;

void worker() {
    tls_value++;
    std::cout << "tls_value = " << tls_value << std::endl;
}

int main() {
    std::thread t1(worker);
    std::thread t2(worker);
    t1.join();
    t2.join();
}
```

输出：

```
tls_value = 1
tls_value = 1
```

两个线程各自从 0 开始，互不影响。

#### 2. 三种声明位置

```cpp
thread_local int g_tls = 0;

struct Foo {
    thread_local static int s_tls;
};

void bar() {
    thread_local int local_tls = 0;
    local_tls++;
}
```

| 声明位置 | 作用域 | 初始化时机 |
|------|------|------|
| 命名空间/全局 | 全局 | 线程首次使用时 |
| 类的 static 成员 | 类 | 线程首次使用时 |
| 函数内局部 | 函数 | 线程首次执行到该语句时 |

**函数内的 `thread_local`** 只初始化一次（per thread），后续调用直接使用已有值：

```cpp
void counter() {
    thread_local int count = 0;
    count++;
    std::cout << "count = " << count << std::endl;
}

int main() {
    std::thread t1([] { counter(); counter(); counter(); });
    std::thread t2([] { counter(); counter(); });
    t1.join();
    t2.join();
}
```

输出：

```
count = 1
count = 2
count = 3
count = 1
count = 2
```

t1 的 count 从 1 数到 3，t2 的 count 从 1 数到 2，各自独立。

***

### 4. 与全局变量/局部变量的区别

```cpp
#include <iostream>
#include <thread>

int g_global = 0;
thread_local int g_tls = 0;

void worker(int id) {
    for (int i = 0; i < 3; i++) {
        g_global++;
        g_tls++;
        int local = 0;
        local++;

        std::cout << "[" << id << "] "
                  << "global=" << g_global
                  << " tls=" << g_tls
                  << " local=" << local
                  << std::endl;
    }
}

int main() {
    std::thread t1(worker, 1);
    std::thread t2(worker, 2);
    t1.join();
    t2.join();
}
```

可能输出：

```
[1] global=1 tls=1 local=1
[1] global=2 tls=2 local=1
[1] global=3 tls=3 local=1
[2] global=4 tls=1 local=1
[2] global=5 tls=2 local=1
[2] global=6 tls=3 local=1
```

| 变量类型 | 行为 | 线程安全？ |
|------|------|:---:|
| `g_global` | 所有线程共享，值持续累加 | ❌ 不安全（需要加锁） |
| `g_tls` | 每个线程独立副本，各从 0 开始 | ✅ 安全（无需加锁） |
| `local` | 每次循环重新初始化为 0 | ✅ 安全（栈上独立） |

***

### 5. TLS 的初始化时机

TLS 变量的初始化发生在**线程启动后、首次使用前**：

```
线程创建 → TLS 初始化 → 线程函数执行 → 线程结束 → TLS 析构
```

关键细节：

1. **每个线程独立初始化**：主线程有一份，每个子线程也各有一份
2. **延迟初始化**：函数内的 `thread_local` 在线程首次执行到该语句时初始化
3. **线程退出时析构**：线程结束时，该线程的 TLS 副本被析构
4. **主线程的 TLS**：程序退出时析构

```cpp
struct Logger {
    Logger() { std::cout << "Logger constructed in thread " << std::this_thread::get_id() << std::endl; }
    ~Logger() { std::cout << "Logger destroyed in thread " << std::this_thread::get_id() << std::endl; }
    void log(const std::string& msg) { std::cout << msg << std::endl; }
};

void task() {
    thread_local Logger logger;
    logger.log("doing work");
}

int main() {
    std::cout << "main thread: " << std::this_thread::get_id() << std::endl;
    std::thread t1(task);
    std::thread t2(task);
    t1.join();
    t2.join();
}
```

每个线程首次调用 `task()` 时构造 `Logger`，线程退出时析构。

***

### 6. 适用场景

#### 1. 场景1：随机数种子

每个线程需要独立的随机数种子，避免多线程共享导致序列重复或竞争：

```cpp
thread_local std::mt19937 rng(std::random_device{}());

int random_int(int min_val, int max_val) {
    std::uniform_int_distribution<int> dist(min_val, max_val);
    return dist(rng);
}
```

#### 2. 场景2：日志缓冲

每个线程有独立的日志缓冲区，避免频繁加锁：

```cpp
thread_local std::string log_buffer;

void log_message(const std::string& msg) {
    log_buffer.clear();
    log_buffer = "[" + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id())) + "] " + msg;
    write_to_file(log_buffer);
}
```

#### 3. 场景3：线程 ID 缓存

避免反复调用 `std::this_thread::get_id()` 的开销：

```cpp
thread_local int cached_tid = -1;

int get_tid() {
    if (cached_tid == -1) {
        cached_tid = allocate_tid();
    }
    return cached_tid;
}
```

#### 4. 场景4：内存分配器

每个线程有独立的内存池，减少锁竞争：

```cpp
thread_local ThreadLocalPool local_pool(4096);

void* fast_alloc(size_t size) {
    return local_pool.allocate(size);
}

void fast_free(void* ptr) {
    local_pool.deallocate(ptr);
}
```

***

### 7. 与加锁的关系

[加锁解锁](./05-加锁解锁.md) 讲了加锁保护共享数据。TLS 是**无锁方案之一**：

| 方案 | 原理 | 优点 | 缺点 |
|------|------|------|------|
| 加锁 | 共享数据 + 互斥访问 | 数据一致 | 锁开销、可能死锁 |
| TLS | 每线程独立副本 | 无锁、无竞争 | 内存占用 × 线程数 |
| 原子操作 | 硬件级原子指令 | 轻量 | 只适合简单操作 |

**选择原则**：

- 数据需要跨线程共享 → 加锁或原子操作
- 数据只需线程内使用 → TLS（零竞争）
- 数据需要共享但读多写少 → 读写锁

```cpp
int g_shared_counter = 0;
std::mutex g_mtx;

thread_local int tls_local_counter = 0;

void worker() {
    for (int i = 0; i < 100000; i++) {
        tls_local_counter++;
    }

    std::lock_guard<std::mutex> lock(g_mtx);
    g_shared_counter += tls_local_counter;
}
```

**模式**：线程内用 TLS 累加（无锁），最后一次性合并到共享变量（加锁一次）。这比每次累加都加锁快得多。

***

### 8. 极简总结

**TLS = 每个线程独享变量副本 = 天然线程安全 = 无锁方案之一**

| 要点 | 说明 |
|------|------|
| 关键字 | `thread_local`（C++11） |
| 本质 | 同一变量名，每线程各持一份 |
| 初始化 | 线程首次使用时初始化，线程退出时析构 |
| 线程安全 | ✅ 天然安全，无需加锁 |
| 代价 | 内存占用 = 变量大小 × 线程数 |
| 典型场景 | 随机数种子、日志缓冲、线程ID缓存、内存池 |
| 一句话 | 不需要共享的数据，别用全局变量，用 TLS |

***

### 相关阅读

- [线程安全](./08-线程安全.md)
- [原子操作与原子变量](./10-原子操作与原子变量.md)
- [多线程底层原理与通信](./00-多线程底层原理与通信.md)

***