# 什么是条件变量 condition variable
> 📖 相关章节：[多线程基础](../../02-CPP/29-多线程基础.md)、[线程同步](../../02-CPP/30-线程同步.md)、[原子操作](../../02-CPP/31-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

> 条件变量不是条件本身，而是"等条件满足再叫醒我"的协作机制。

***

### 1. 要点直击

条件变量是一种同步原语，允许线程在满足某个条件前挂起等待，由另一个线程在条件满足时发出通知唤醒，必须与互斥锁配合使用。

***

### 2. 等待/通知机制的本质

考虑一个简单场景：线程 A 需要等数据就绪后才能处理，线程 B 负责准备数据。如何让线程 A 高效等待而不是忙轮询？

**忙轮询（错误做法）：**

```cpp
#include <mutex>

std::mutex mtx;
bool data_ready = false;

void waiting_thread() {
    while (true) {
        std::lock_guard<std::mutex> lk(mtx);
        if (data_ready) break;
    }
    process_data();
}
```

CPU 空转，浪费资源。

**条件变量（正确做法）：**

```cpp
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool data_ready = false;

void waiting_thread() {
    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [] { return data_ready; });
    lk.unlock();
    process_data();
}

void notifying_thread() {
    {
        std::lock_guard<std::mutex> lk(mtx);
        data_ready = true;
    }
    cv.notify_one();
}
```

条件变量的核心操作：

| 操作 | 说明 |
|------|------|
| `wait` | 释放锁 → 挂起线程 → 被唤醒后重新获取锁 |
| `notify_one` | 唤醒一个等待线程 |
| `notify_all` | 唤醒所有等待线程 |

***

### 3. 为什么仅靠互斥锁不够

互斥锁解决的是互斥访问，不是条件等待。仅用互斥锁无法高效实现"等条件成立"。

```cpp
#include <mutex>
#include <iostream>
#include <thread>

std::mutex mtx;
int shared_counter = 0;

void waiting_with_mutex_only() {
    std::unique_lock<std::mutex> lk(mtx);
    while (shared_counter < 100) {
        lk.unlock();
        std::this_thread::yield();
        lk.lock();
    }
    std::cout << "counter reached 100\n";
}
```

这段代码的问题：

| 问题 | 说明 |
|------|------|
| 忙等待 | unlock → yield → lock 循环消耗 CPU |
| 延迟不确定 | yield 不保证何时重新调度 |
| 锁争用 | 反复加锁解锁增加开销 |
| 不可组合 | 多个条件难以管理 |

条件变量将"检查条件"和"等待通知"原子化：

```cpp
void waiting_with_cv() {
    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [] { return shared_counter >= 100; });
    std::cout << "counter reached 100\n";
}
```

`wait` 内部执行流程：

```
1. 检查谓词 → 满足则直接返回
2. 不满足 → 释放锁 → 线程挂起（不消耗 CPU）
3. 被唤醒 → 重新获取锁 → 再次检查谓词
4. 谓词满足 → 返回；不满足 → 回到步骤 2
```

| 维度 | 仅互斥锁 | 互斥锁 + 条件变量 |
|------|----------|-------------------|
| CPU 使用 | 忙等待，高 | 挂起，零 |
| 响应延迟 | 取决于轮询频率 | 立即响应通知 |
| 代码复杂度 | 手动轮询逻辑 | wait 封装 |
| 可扩展性 | 线程越多越慢 | 线程数无关 |

***

### 4. wait 与谓词

`condition_variable` 提供两种 `wait` 形式：

```cpp
std::condition_variable cv;
std::mutex mtx;

void basic_wait() {
    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk);
}

void predicate_wait() {
    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [] { return data_ready; });
}
```

**无谓词 wait 等价于：**

```cpp
cv.wait(lk);
while (!condition) {
    cv.wait(lk);
}
```

**带谓词 wait 等价于：**

```cpp
while (!predicate()) {
    cv.wait(lk);
}
```

带谓词的版本更安全，因为它自动处理虚假唤醒。始终优先使用带谓词的 `wait`。

**wait_for 和 wait_until：**

```cpp
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <iostream>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void timed_wait() {
    std::unique_lock<std::mutex> lk(mtx);
    if (cv.wait_for(lk, std::chrono::seconds(5), [] { return ready; })) {
        std::cout << "condition met\n";
    } else {
        std::cout << "timeout\n";
    }
}

void deadline_wait() {
    std::unique_lock<std::mutex> lk(mtx);
    auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(5);
    if (cv.wait_until(lk, deadline, [] { return ready; })) {
        std::cout << "condition met\n";
    } else {
        std::cout << "timeout\n";
    }
}
```

| wait 变体 | 返回值 | 说明 |
|-----------|--------|------|
| `wait(lock)` | void | 无限等待 |
| `wait(lock, pred)` | bool（谓词结果） | 等到谓词为 true |
| `wait_for(lock, duration)` | cv_status | 超时或被唤醒 |
| `wait_for(lock, duration, pred)` | bool | 超时内等到谓词为 true |
| `wait_until(lock, time_point)` | cv_status | 到时间点或被唤醒 |
| `wait_until(lock, time_point, pred)` | bool | 时间点前等到谓词为 true |

***

### 5. 虚假唤醒

虚假唤醒（Spurious Wakeup）是指线程在没有收到 `notify` 调用的情况下从 `wait` 返回。POSIX 标准明确允许这种行为。

**虚假唤醒的原因：**

- 某些多处理器架构上，确保每次唤醒都精确对应一次 notify 代价过高
- 操作系统实现可能将多个等待者同时唤醒

**因此，wait 返回后必须重新检查条件：**

```cpp
void unsafe_pattern() {
    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk);
    if (data_ready) {
        process();
    }
}
```

上面代码在虚假唤醒时 `data_ready` 可能为 false，导致逻辑错误。正确做法：

```cpp
void safe_pattern() {
    std::unique_lock<std::mutex> lk(mtx);
    while (!data_ready) {
        cv.wait(lk);
    }
    process();
}
```

或直接使用带谓词的版本：

```cpp
void best_pattern() {
    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [] { return data_ready; });
    process();
}
```

| 模式 | 安全性 | 推荐度 |
|------|--------|--------|
| `wait` + `if` | ❌ 不安全 | 禁止 |
| `wait` + `while` | ✅ 安全 | 可用 |
| `wait(lock, pred)` | ✅ 安全 | 推荐 |

***

### 6. notify_one vs notify_all

`notify_one` 唤醒一个等待线程，`notify_all` 唤醒所有等待线程。选择不当会导致性能问题或逻辑错误。

**notify_one 适用场景：只唤醒一个处理者**

```cpp
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>

std::mutex mtx;
std::condition_variable cv;
std::queue<int> tasks;

void worker() {
    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [] { return !tasks.empty(); });
    int task = tasks.front();
    tasks.pop();
    lk.unlock();
    process(task);
}

void producer() {
    {
        std::lock_guard<std::mutex> lk(mtx);
        tasks.push(42);
    }
    cv.notify_one();
}
```

**notify_all 适用场景：所有等待者都需要响应**

```cpp
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <vector>
#include <thread>

std::mutex mtx;
std::condition_variable cv;
bool shutdown_flag = false;

void worker_thread(int id) {
    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [] { return shutdown_flag; });
    std::cout << "worker " << id << " exiting\n";
}

void shutdown_all() {
    {
        std::lock_guard<std::mutex> lk(mtx);
        shutdown_flag = true;
    }
    cv.notify_all();
}
```

| 场景 | 选择 | 原因 |
|------|------|------|
| 任务队列，一个任务一个线程 | notify_one | 只需唤醒一个消费者 |
| 全局状态变更（关闭/配置更新） | notify_all | 所有等待者都需要响应 |
| 资源释放（连接池归还） | notify_one | 一个等待者即可使用 |
| 屏障同步 | notify_all | 所有线程都需要继续 |

**错误使用 notify_one 的案例：**

```cpp
std::mutex mtx;
std::condition_variable cv;
bool resource_a_ready = false;
bool resource_b_ready = false;

void wait_for_a() {
    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [] { return resource_a_ready; });
}

void wait_for_b() {
    std::unique_lock<std::mutex> lk(mtx);
    cv.wait(lk, [] { return resource_b_ready; });
}

void set_a() {
    {
        std::lock_guard<std::mutex> lk(mtx);
        resource_a_ready = true;
    }
    cv.notify_one();
}
```

如果 `wait_for_b` 的线程先被唤醒，谓词检查失败后重新等待，而 `wait_for_a` 的线程没被唤醒——这就是"惊群遗漏"。应使用 `notify_all` 或使用不同的条件变量。

***

### 7. 丢失唤醒

丢失唤醒（Lost Wakeup）是指通知在等待之前发出，导致等待线程永远阻塞。

```cpp
#include <mutex>
#include <condition_variable>
#include <thread>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void lost_wakeup_example() {
    std::thread t1([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        {
            std::lock_guard<std::mutex> lk(mtx);
            ready = true;
        }
        cv.notify_one();
    });

    std::thread t2([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk, [] { return ready; });
    });

    t1.join();
    t2.join();
}
```

上面代码不会丢失唤醒，因为谓词 `ready` 在 wait 中会先检查。但如果用无谓词的 wait 且通知先于等待到达：

```cpp
void real_lost_wakeup() {
    std::thread t1([&] {
        {
            std::lock_guard<std::mutex> lk(mtx);
            ready = true;
        }
        cv.notify_one();
    });

    std::thread t2([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::unique_lock<std::mutex> lk(mtx);
        cv.wait(lk);
    });

    t1.join();
    t2.join();
}
```

t2 会永远阻塞，因为 notify 在 wait 之前发出。

**防丢失唤醒的核心原则：**

| 原则 | 说明 |
|------|------|
| 使用带谓词的 wait | 谓词检查条件是否已满足 |
| 先修改条件再 notify | 确保条件在 notify 前已更新 |
| 在锁内修改条件 | 保证条件修改的原子性 |
| notify 可在锁外 | 减少不必要的唤醒阻塞 |

```cpp
void safe_pattern() {
    {
        std::lock_guard<std::mutex> lk(mtx);
        ready = true;
    }
    cv.notify_one();
}
```

***

### 8. condition_variable vs condition_variable_any

C++ 提供两种条件变量：

| 特性 | condition_variable | condition_variable_any |
|------|--------------------|------------------------|
| 锁类型 | 仅 `std::unique_lock<std::mutex>` | 任何满足 BasicLockable 的锁 |
| 头文件 | `<condition_variable>` | `<condition_variable>` |
| 性能 | 最优 | 可能有额外开销 |
| 灵活性 | 低 | 高 |
| 适用场景 | 绝大多数场景 | 自定义互斥锁 |

```cpp
#include <mutex>
#include <shared_mutex>
#include <condition_variable>

std::shared_mutex rw_mtx;
std::condition_variable_any cv_any;
bool ready = false;

void reader() {
    std::shared_lock<std::shared_mutex> lk(rw_mtx);
    cv_any.wait(lk, [] { return ready; });
}

void writer() {
    {
        std::unique_lock<std::shared_mutex> lk(rw_mtx);
        ready = true;
    }
    cv_any.notify_all();
}
```

`condition_variable` 不接受 `shared_lock`，而 `condition_variable_any` 可以。

**何时选择 `condition_variable_any`：**

- 需要与 `shared_mutex` 配合
- 使用自定义互斥锁类型
- 需要与第三方锁类型配合

**何时选择 `condition_variable`：**

- 默认选择
- 性能敏感场景
- 使用标准 `std::mutex`

***

### 9. pthread_cond 与 C++ 封装

C++ 的 `condition_variable` 底层封装了 POSIX 的 `pthread_cond_t`。

**POSIX 原始接口：**

```cpp
Linux:
#include <pthread.h>
#include <stdio.h>

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
int data_ready = 0;

void* waiting_thread(void* arg) {
    pthread_mutex_lock(&mtx);
    while (!data_ready) {
        pthread_cond_wait(&cond, &mtx);
    }
    printf("data is ready\n");
    pthread_mutex_unlock(&mtx);
    return NULL;
}

void* notifying_thread(void* arg) {
    pthread_mutex_lock(&mtx);
    data_ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mtx);
    return NULL;
}
```

POSIX 与 C++ 接口映射：

| POSIX | C++ |
|-------|-----|
| `pthread_cond_t` | `std::condition_variable` |
| `pthread_cond_init` | 构造函数 |
| `pthread_cond_destroy` | 析构函数 |
| `pthread_cond_wait` | `wait()` |
| `pthread_cond_timedwait` | `wait_for()` / `wait_until()` |
| `pthread_cond_signal` | `notify_one()` |
| `pthread_cond_broadcast` | `notify_all()` |
| `pthread_mutex_t` | `std::mutex` |

C++ 封装的优势：

| 维度 | POSIX | C++ |
|------|-------|-----|
| RAII | 手动 init/destroy | 自动构造/析构 |
| 谓词支持 | 手写 while 循环 | `wait(lock, pred)` |
| 超时 | 绝对时间 struct timespec | `chrono` 时长/时间点 |
| 异常安全 | 无 | 析构保证 |
| 跨平台 | POSIX only | 跨平台 |

***

### 10. 生产者-消费者模型

生产者-消费者是条件变量最经典的应用场景。

```cpp
#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <iostream>
#include <atomic>

std::mutex mtx;
std::condition_variable cv_not_empty;
std::condition_variable cv_not_full;
std::queue<int> buffer;
const size_t MAX_SIZE = 10;
std::atomic<bool> done{false};

void producer(int id) {
    for (int i = 0; i < 20; ++i) {
        std::unique_lock<std::mutex> lk(mtx);
        cv_not_full.wait(lk, [] { return buffer.size() < MAX_SIZE; });
        buffer.push(i);
        std::cout << "producer " << id << " produced " << i << "\n";
        lk.unlock();
        cv_not_empty.notify_one();
    }
}

void consumer(int id) {
    while (!done || !buffer.empty()) {
        std::unique_lock<std::mutex> lk(mtx);
        if (cv_not_empty.wait_for(lk, std::chrono::milliseconds(100),
                                   [] { return !buffer.empty(); })) {
            int val = buffer.front();
            buffer.pop();
            std::cout << "consumer " << id << " consumed " << val << "\n";
            lk.unlock();
            cv_not_full.notify_one();
        }
    }
}

int main() {
    std::thread p1(producer, 1);
    std::thread p2(producer, 2);
    std::thread c1(consumer, 1);
    std::thread c2(consumer, 2);

    p1.join();
    p2.join();
    done = true;
    c1.join();
    c2.join();
    return 0;
}
```

关键设计点：

| 要点 | 说明 |
|------|------|
| 两个条件变量 | `not_empty` 供消费者等待，`not_full` 供生产者等待 |
| 有界缓冲区 | 防止生产者无限生产导致内存耗尽 |
| 谓词检查 | `wait` 带谓词防止虚假唤醒 |
| notify 时机 | 修改缓冲区后通知对方 |
| 优雅退出 | `done` 标志 + 超时等待 |

***

### 11. 线程池通知与常见错误

**线程池中的条件变量：**

```cpp
#include <mutex>
#include <condition_variable>
#include <queue>
#include <vector>
#include <thread>
#include <functional>
#include <atomic>

class ThreadPool {
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mtx_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};

public:
    ThreadPool(size_t count) {
        for (size_t i = 0; i < count; ++i) {
            workers_.emplace_back([this] { worker_loop(); });
        }
    }

    ~ThreadPool() {
        stop_ = true;
        cv_.notify_all();
        for (auto& w : workers_) w.join();
    }

    void submit(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lk(mtx_);
            tasks_.push(std::move(task));
        }
        cv_.notify_one();
    }

private:
    void worker_loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lk(mtx_);
                cv_.wait(lk, [this] { return stop_ || !tasks_.empty(); });
                if (stop_ && tasks_.empty()) return;
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            task();
        }
    }
};
```

**常见错误汇总：**

| 错误 | 示例 | 修复 |
|------|------|------|
| 不用谓词 | `cv.wait(lk)` | `cv.wait(lk, pred)` |
| notify 在锁内 | `lk` 未 unlock 就 notify | 先 unlock 再 notify |
| 检查条件不加锁 | `if (ready)` 无锁访问 | 在锁内检查 |
| 忘记 notify | 修改条件后不通知 | 修改后调用 notify |
| 用 if 不用 while | `if (!cond) cv.wait(lk)` | `while (!cond) cv.wait(lk)` 或带谓词 |
| 多条件共用一个 cv | 不同条件用同一 cv | 不同条件用不同 cv |
| wait 用 lock_guard | `cv.wait(lock_guard)` | 必须用 `unique_lock` |

**notify 在锁内 vs 锁外：**

```cpp
void notify_inside_lock() {
    std::lock_guard<std::mutex> lk(mtx);
    ready = true;
    cv.notify_one();
}

void notify_outside_lock() {
    {
        std::lock_guard<std::mutex> lk(mtx);
        ready = true;
    }
    cv.notify_one();
}
```

| 方式 | 优点 | 缺点 |
|------|------|------|
| 锁内 notify | 逻辑简单 | 被唤醒线程需等锁释放 |
| 锁外 notify | 唤醒后可立即获取锁 | 代码稍复杂 |

大多数场景下两者性能差异可忽略，但锁外 notify 是更优实践。

***

### 12. 极简总结

| 要点 | 内容 |
|------|------|
| 本质 | 等待/通知同步原语，必须配合互斥锁 |
| 核心操作 | wait（挂起）、notify_one（唤醒一个）、notify_all（唤醒全部） |
| 为什么需要 | 互斥锁只能互斥，不能高效等待条件 |
| 谓词 wait | `wait(lock, pred)` 自动处理虚假唤醒，始终优先使用 |
| 虚假唤醒 | wait 可能无 notify 就返回，必须重新检查条件 |
| notify 选择 | 单消费者用 notify_one，多等待者用 notify_all |
| 丢失唤醒 | 通知先于等待到达，用谓词 wait 可避免 |
| cv vs cv_any | cv 仅配 mutex，cv_any 可配任意锁 |
| 经典场景 | 生产者-消费者、线程池、事件等待 |
| 常见错误 | 不用谓词、不加锁检查、忘记 notify |

核心记忆口诀：**条件变量配互斥锁，wait 必须带谓词，notify 选对 one/all，检查条件要在锁内**。

***

### 相关阅读

- [什么是信号量Semaphore](./27-什么是信号量Semaphore.md)
- [latch与barrier](./22-latch与barrier.md)
- [多线程通讯与数据获取](./14-多线程通讯与数据获取.md)