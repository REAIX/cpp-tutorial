# 什么是信号量Semaphore
> 📖 相关章节：[多线程基础](../../02-CPP/29-多线程基础.md)、[线程同步](../../02-CPP/30-线程同步.md)、[原子操作](../../02-CPP/31-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

> 信号量是并发编程中最经典的同步原语——从 Dijkstra 的 P/V 操作到 C++20 的 counting_semaphore，它始终是资源管理的基石。

***

### 1. 本质速解

信号量是一个非负整数计数器，通过"减一等待（P）"和"加一唤醒（V）"两个原子操作，控制同时访问共享资源的线程数量。

***

### 2. 信号量的基本概念与 Dijkstra P/V 操作

信号量由荷兰计算机科学家 Edsger W. Dijkstra 于 1965 年提出，核心是两个原子操作：

| 操作 | 名称 | 行为 |
|------|------|------|
| P（Proberen / wait） | 申请资源 | 计数器减 1；若结果 < 0，线程阻塞等待 |
| V（Verhogen / signal） | 释放资源 | 计数器加 1；若有等待线程，唤醒其中一个 |

```cpp
#include <cstdio>
#include <thread>
#include <mutex>
#include <condition_variable>

class SimpleSemaphore {
    int count_;
    std::mutex mtx_;
    std::condition_variable cv_;
public:
    explicit SimpleSemaphore(int initial = 0) : count_(initial) {}

    void wait() {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return count_ > 0; });
        --count_;
    }

    void signal() {
        std::lock_guard<std::mutex> lock(mtx_);
        ++count_;
        cv_.notify_one();
    }
};

int main() {
    SimpleSemaphore sem(0);

    std::thread consumer([&] {
        sem.wait();
        std::printf("消费者：拿到资源\n");
    });

    std::thread producer([&] {
        std::printf("生产者：准备资源\n");
        sem.signal();
    });

    producer.join();
    consumer.join();
    return 0;
}
```

**关键理解**：计数器的值 = 可用资源数；当计数器为 0 时，下一个 P 操作将阻塞。

***

### 3. 二值信号量与计数信号量

| 特性 | 二值信号量（Binary Semaphore） | 计数信号量（Counting Semaphore） |
|------|------|------|
| 计数范围 | 0 或 1 | 0 到任意正整数 |
| 典型用途 | 互斥访问、事件通知 | 限制并发数、资源池管理 |
| 与互斥锁区别 | 二值信号量不绑定线程所有权 | 互斥锁有所有权概念 |
| 释放者 | 任意线程均可 V 操作 | 任意线程均可 V 操作 |

```cpp
#include <cstdio>
#include <thread>

class BinarySemaphore {
    bool available_;
    std::mutex mtx_;
    std::condition_variable cv_;
public:
    explicit BinarySemaphore(bool initial = true) : available_(initial) {}

    void wait() {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_.wait(lock, [this] { return available_; });
        available_ = false;
    }

    void signal() {
        std::lock_guard<std::mutex> lock(mtx_);
        available_ = true;
        cv_.notify_one();
    }
};

int main() {
    BinarySemaphore binSem(false);

    std::thread waiter([&] {
        std::printf("等待事件...\n");
        binSem.wait();
        std::printf("事件已到达，继续执行\n");
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::thread notifier([&] {
        std::printf("发送事件通知\n");
        binSem.signal();
    });

    waiter.join();
    notifier.join();
    return 0;
}
```

***

### 4. C++20 counting_semaphore 与 binary_semaphore

C++20 终于将信号量纳入标准库，定义在 `<semaphore>` 头文件中：

```cpp
#include <semaphore>
#include <cstdio>
#include <thread>
#include <vector>

int main() {
    constexpr int MAX_CONCURRENT = 3;
    std::counting_semaphore<MAX_CONCURRENT> sem(MAX_CONCURRENT);

    auto worker = [&](int id) {
        sem.acquire();
        std::printf("线程 %d 开始工作（获得许可）\n", id);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::printf("线程 %d 完成工作（释放许可）\n", id);
        sem.release();
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back(worker, i);
    }
    for (auto& t : threads) {
        t.join();
    }
    return 0;
}
```

**binary_semaphore 示例**：

```cpp
#include <semaphore>
#include <cstdio>
#include <thread>

int main() {
    std::binary_semaphore signalReady(0);

    std::thread t1([&] {
        std::printf("等待数据就绪...\n");
        signalReady.acquire();
        std::printf("数据已就绪，开始处理\n");
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::thread t2([&] {
        std::printf("数据准备完成\n");
        signalReady.release();
    });

    t1.join();
    t2.join();
    return 0;
}
```

| API | 说明 |
|-----|------|
| `acquire()` | 计数器减 1，若为 0 则阻塞（P 操作） |
| `release(n=1)` | 计数器加 n，唤醒等待线程（V 操作） |
| `try_acquire()` | 尝试减 1，不阻塞，返回 bool |
| `try_acquire_for(duration)` | 带超时的尝试 |
| `try_acquire_until(time_point)` | 带截止时间的尝试 |

> **注意**：`counting_semaphore<least_max_value>` 的模板参数是计数器最大值，实际值可以小于它。`binary_semaphore` 是 `counting_semaphore<1>` 的别名。

***

### 5. POSIX 信号量：sem_init / sem_wait / sem_post

在 Linux/Unix 环境下，POSIX 信号量是更底层的选择：

```cpp
#include <cstdio>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>

sem_t sem;

void* worker(void* arg) {
    int id = *(int*)arg;
    sem_wait(&sem);
    std::printf("线程 %d 进入临界区\n", id);
    sleep(1);
    std::printf("线程 %d 离开临界区\n", id);
    sem_post(&sem);
    return nullptr;
}

int main() {
    sem_init(&sem, 0, 2);

    pthread_t threads[5];
    int ids[5] = {1, 2, 3, 4, 5};

    for (int i = 0; i < 5; ++i) {
        pthread_create(&threads[i], nullptr, worker, &ids[i]);
    }
    for (int i = 0; i < 5; ++i) {
        pthread_join(threads[i], nullptr);
    }

    sem_destroy(&sem);
    return 0;
}
```

| POSIX API | 说明 |
|-----------|------|
| `sem_init(sem, pshared, value)` | 初始化无名信号量，pshared=0 为线程共享 |
| `sem_wait(sem)` | P 操作，阻塞 |
| `sem_post(sem)` | V 操作，唤醒 |
| `sem_trywait(sem)` | 非阻塞 P 操作 |
| `sem_timedwait(sem, abstime)` | 带超时的 P 操作 |
| `sem_destroy(sem)` | 销毁无名信号量 |
| `sem_open(name, flags, mode, value)` | 创建/打开有名信号量 |
| `sem_close(sem)` / `sem_unlink(name)` | 关闭/删除有名信号量 |

> **Linux 平台注意**：`sem_init` 在某些 glibc 版本中，`pshared=0` 时 fork 后行为未定义。有名信号量（`sem_open`）可用于进程间同步。

***

### 6. 信号量 vs 互斥锁

| 对比维度 | 信号量（Semaphore） | 互斥锁（Mutex） |
|----------|---------------------|-----------------|
| 计数能力 | 支持 N 个并发 | 只允许 1 个 |
| 所有权 | 无所有权，任意线程可 release | 有所有权，谁 lock 谁 unlock |
| 用途 | 资源计数 + 同步通知 | 互斥保护临界区 |
| 递归 | 不支持递归获取 | recursive_mutex 支持 |
| 优先级反转 | 可能发生 | 优先级继承互斥锁可缓解 |
| 典型场景 | 连接池、停车场模型 | 保护共享数据结构 |

**经典误区**：用二值信号量代替互斥锁——虽然功能上看似可行，但二值信号量没有所有权，同一个线程可以连续 acquire 两次（第二次阻塞），而另一个线程可以 release 解锁，这会导致优先级反转等问题。

```cpp
#include <mutex>
#include <cstdio>
#include <thread>

int shared_data = 0;
std::mutex mtx;

void safe_increment(int times) {
    for (int i = 0; i < times; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        ++shared_data;
    }
}

int main() {
    std::thread t1(safe_increment, 100000);
    std::thread t2(safe_increment, 100000);
    t1.join();
    t2.join();
    std::printf("最终值: %d\n", shared_data);
    return 0;
}
```

***

### 7. 生产者-消费者模型

信号量是实现生产者-消费者模型最自然的同步原语：

```cpp
#include <semaphore>
#include <cstdio>
#include <thread>
#include <queue>
#include <mutex>

constexpr int BUFFER_SIZE = 5;

std::queue<int> buffer;
std::mutex buffer_mtx;
std::counting_semaphore<BUFFER_SIZE> empty_slots(BUFFER_SIZE);
std::counting_semaphore<BUFFER_SIZE> filled_slots(0);

void producer(int id) {
    for (int i = 1; i <= 10; ++i) {
        empty_slots.acquire();
        {
            std::lock_guard<std::mutex> lock(buffer_mtx);
            buffer.push(i);
            std::printf("生产者 %d: 生产 %d（缓冲区大小 %zu）\n", id, i, buffer.size());
        }
        filled_slots.release();
    }
}

void consumer(int id) {
    for (int i = 1; i <= 10; ++i) {
        filled_slots.acquire();
        int value;
        {
            std::lock_guard<std::mutex> lock(buffer_mtx);
            value = buffer.front();
            buffer.pop();
            std::printf("消费者 %d: 消费 %d（缓冲区大小 %zu）\n", id, value, buffer.size());
        }
        empty_slots.release();
    }
}

int main() {
    std::thread p1(producer, 1);
    std::thread p2(producer, 2);
    std::thread c1(consumer, 1);
    std::thread c2(consumer, 2);

    p1.join();
    p2.join();
    c1.join();
    c2.join();
    return 0;
}
```

**三个同步要素**：
- `empty_slots`：空闲槽位信号量，初值 = 缓冲区大小
- `filled_slots`：已填充槽位信号量，初值 = 0
- `buffer_mtx`：互斥锁保护缓冲区本身

***

### 8. 资源池管理

信号量天然适合管理有限资源池：

```cpp
#include <semaphore>
#include <cstdio>
#include <thread>
#include <vector>
#include <mutex>
#include <queue>

class ConnectionPool {
    std::queue<int> pool_;
    std::mutex mtx_;
    std::counting_semaphore<10> sem_;
    int next_id_;
public:
    explicit ConnectionPool(int size) : sem_(size), next_id_(1) {
        for (int i = 0; i < size; ++i) {
            pool_.push(next_id_++);
        }
    }

    int acquire() {
        sem_.acquire();
        std::lock_guard<std::mutex> lock(mtx_);
        int conn = pool_.front();
        pool_.pop();
        return conn;
    }

    void release(int conn) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            pool_.push(conn);
        }
        sem_.release();
    }
};

int main() {
    ConnectionPool pool(3);

    auto use_connection = [&](int thread_id) {
        int conn = pool.acquire();
        std::printf("线程 %d 获取连接 %d\n", thread_id, conn);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        std::printf("线程 %d 释放连接 %d\n", thread_id, conn);
        pool.release(conn);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 8; ++i) {
        threads.emplace_back(use_connection, i + 1);
    }
    for (auto& t : threads) {
        t.join();
    }
    return 0;
}
```

***

### 9. 常见陷阱与最佳实践

| 陷阱 | 说明 | 解决方案 |
|------|------|----------|
| 忘记 release | 异常路径未释放信号量，导致死锁 | 使用 RAII 封装 acquire/release |
| 信号量初始值错误 | 初始值与实际资源数不匹配 | 仔细核对初始值与资源数 |
| 用信号量做互斥 | 二值信号量无所有权，可能优先级反转 | 互斥保护用 mutex，资源计数用 semaphore |
| V 操作放在锁内 | 在持有互斥锁时调用 signal，可能导致不必要的上下文切换 | 先释放锁，再 signal |
| 多次 acquire 死锁 | 同一线程多次 acquire 同一信号量 | 确保每次 acquire 都有对应 release |
| POSIX 有名信号量未 unlink | 进程崩溃后信号量残留 | 使用 RAII 或 atexit 注册清理 |

**RAII 封装示例**：

```cpp
#include <semaphore>
#include <cstdio>

class SemaphoreGuard {
    std::counting_semaphore<>& sem_;
public:
    explicit SemaphoreGuard(std::counting_semaphore<>& s) : sem_(s) {
        sem_.acquire();
    }
    ~SemaphoreGuard() {
        sem_.release();
    }
    SemaphoreGuard(const SemaphoreGuard&) = delete;
    SemaphoreGuard& operator=(const SemaphoreGuard&) = delete;
};

std::counting_semaphore<5> g_sem(5);

void safe_work(int id) {
    SemaphoreGuard guard(g_sem);
    std::printf("线程 %d 工作中\n", id);
}
```

***

### 10. 跨平台注意事项

| 平台 | 信号量实现 | 备注 |
|------|-----------|------|
| C++20 | `std::counting_semaphore` / `std::binary_semaphore` | 需编译器支持 C++20 |
| Linux | POSIX `sem_*` | 无名/有名信号量，链接时需 `-lpthread` |
| Windows | `CreateSemaphore` / `WaitForSingleObject` / `ReleaseSemaphore` | 内核对象，可跨进程 |
| macOS | POSIX `sem_open` | 无名信号量 `sem_init` 已废弃 |

**Windows 原生信号量示例**：

```cpp
#ifdef _WIN32
#include <windows.h>
#include <cstdio>

int main() {
    HANDLE hSem = CreateSemaphore(nullptr, 3, 3, nullptr);

    for (int i = 0; i < 5; ++i) {
        WaitForSingleObject(hSem, INFINITE);
        std::printf("线程获取资源 %d\n", i);
    }

    CloseHandle(hSem);
    return 0;
}
#endif
```

***

### 11. 极简总结

| 概念 | 要点 |
|------|------|
| 信号量本质 | 非负整数计数器 + P/V 原子操作 |
| 二值信号量 | 计数 0/1，适合事件通知 |
| 计数信号量 | 计数 0~N，适合资源池/并发限制 |
| C++20 | `<semaphore>` 头文件，`acquire()/release()` |
| POSIX | `sem_wait()/sem_post()`，需 `-lpthread` |
| vs Mutex | 信号量无所有权，用于资源计数；Mutex 有所有权，用于互斥保护 |
| 生产者-消费者 | empty_slots + filled_slots + mutex 三件套 |
| 常见陷阱 | 忘记 release、初始值错误、V 操作放锁内 |
| 最佳实践 | RAII 封装、互斥用 mutex、计数用 semaphore |

***

### 相关阅读

- [什么是条件变量condition-variable](./26-什么是条件变量condition-variable.md)
- [latch与barrier](./22-latch与barrier.md)
- [什么是future与promise](./28-什么是future与promise.md)

***