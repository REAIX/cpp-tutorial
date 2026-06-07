# 什么是future与promise
> 📖 相关章节：[多线程基础](../../02-CPP/27-多线程基础.md)、[线程同步](../../02-CPP/28-线程同步.md)、[原子操作](../../02-CPP/29-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

> future 和 promise 是一对"契约"——promise 负责写入结果，future 负责读取结果，它们是 C++ 异步编程的核心抽象。

***

### 1. 核心要义

`std::promise` 在生产端设置结果（或异常），`std::future` 在消费端阻塞等待获取结果，两者通过共享状态连接，实现线程间安全的一次性值传递。

***

### 2. promise 与 future 的基本模型

`promise` 和 `future` 共享同一块"共享状态"（shared state）：

```
线程A (生产者)              共享状态              线程B (消费者)
promise.set_value()  ────>  [value/exception]  ────>  future.get()
```

```cpp
#include <future>
#include <cstdio>
#include <thread>

int main() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();

    std::thread producer([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        prom.set_value(42);
        std::printf("生产者：已设置值 42\n");
    });

    std::printf("消费者：等待结果...\n");
    int result = fut.get();
    std::printf("消费者：拿到结果 %d\n", result);

    producer.join();
    return 0;
}
```

**核心要点**：
- `promise` 只能 `set_value` 一次，重复调用抛 `std::future_error`
- `future` 只能 `get` 一次，第二次调用行为未定义（通常抛异常）
- `get()` 是阻塞调用，直到结果就绪

***

### 3. std::async 与启动策略

`std::async` 是创建异步任务的高层封装，自动创建 promise/future：

```cpp
#include <future>
#include <cstdio>
#include <chrono>

int heavy_computation(int x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return x * x;
}

int main() {
    std::future<int> f1 = std::async(std::launch::async, heavy_computation, 10);
    std::future<int> f2 = std::async(std::launch::deferred, heavy_computation, 20);

    std::printf("做其他事情...\n");

    std::printf("f1 结果: %d\n", f1.get());
    std::printf("f2 结果: %d\n", f2.get());
    return 0;
}
```

| 启动策略 | 行为 | 适用场景 |
|----------|------|----------|
| `std::launch::async` | 立即创建新线程执行 | 真正的并行、IO 密集型 |
| `std::launch::deferred` | 延迟到 `get()` 时在当前线程同步执行 | 节省线程、CPU 密集型 |
| `async | deferred`（默认） | 实现自行决定 | 通用场景，行为不确定 |

> **重要陷阱**：默认策略下，`std::async` 返回的 future 析构时会阻塞等待任务完成。如果不需要结果，也要保存 future 对象或调用 `.wait()`。

```cpp
#include <future>
#include <cstdio>
#include <vector>

int main() {
    std::vector<std::future<int>> futures;

    for (int i = 0; i < 5; ++i) {
        futures.push_back(std::async(std::launch::async, [](int x) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return x * 2;
        }, i));
    }

    for (auto& f : futures) {
        std::printf("结果: %d\n", f.get());
    }
    return 0;
}
```

***

### 4. shared_future：多人等待同一结果

`std::future` 只能 `get()` 一次，而 `std::shared_future` 允许多次 `get()`，多个线程可以共享同一结果：

```cpp
#include <future>
#include <cstdio>
#include <thread>
#include <vector>

int main() {
    std::promise<int> prom;
    std::shared_future<int> sf = prom.get_future().share();

    std::thread setter([&] {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        prom.set_value(100);
    });

    std::vector<std::thread> readers;
    for (int i = 0; i < 4; ++i) {
        readers.emplace_back([sf, i] {
            int val = sf.get();
            std::printf("读者 %d 获取值: %d\n", i, val);
        });
    }

    setter.join();
    for (auto& t : readers) {
        t.join();
    }
    return 0;
}
```

| 特性 | `std::future` | `std::shared_future` |
|------|---------------|---------------------|
| `get()` 次数 | 仅一次 | 多次 |
| 可复制 | 否（仅移动） | 是 |
| 多线程共享 | 否 | 是（每次 get 返回相同值） |
| 典型场景 | 单消费者 | 多消费者/广播 |

***

### 5. packaged_task：包装可调用对象

`std::packaged_task` 将任意可调用对象包装，使其返回值存入共享状态：

```cpp
#include <future>
#include <cstdio>
#include <thread>
#include <queue>
#include <mutex>
#include <functional>

class ThreadPool {
    std::queue<std::function<void()>> tasks_;
    std::mutex mtx_;
    bool stop_ = false;
    std::vector<std::thread> workers_;
public:
    explicit ThreadPool(int n) {
        for (int i = 0; i < n; ++i) {
            workers_.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::lock_guard<std::mutex> lock(mtx_);
                        if (tasks_.empty()) break;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    template<typename F>
    auto submit(F&& f) -> std::future<decltype(f())> {
        using ReturnType = decltype(f());
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<F>(f));
        std::future<ReturnType> result = task->get_future();
        {
            std::lock_guard<std::mutex> lock(mtx_);
            tasks_.emplace([task] { (*task)(); });
        }
        return result;
    }

    ~ThreadPool() {
        for (auto& w : workers_) w.join();
    }
};

int main() {
    ThreadPool pool(4);

    auto f1 = pool.submit([] { return 1 + 1; });
    auto f2 = pool.submit([] { return 2 * 3; });
    auto f3 = pool.submit([] { return 10 - 4; });

    std::printf("f1 = %d\n", f1.get());
    std::printf("f2 = %d\n", f2.get());
    std::printf("f3 = %d\n", f3.get());
    return 0;
}
```

**三者的关系**：

| 组件 | 角色 | 典型用法 |
|------|------|----------|
| `promise` | 手动设置结果/异常 | 底层线程间通信 |
| `packaged_task` | 包装可调用对象，自动设置结果 | 线程池任务提交 |
| `async` | 自动创建线程 + promise/future | 简单异步任务 |

***

### 6. future.get() 阻塞与状态查询

`future` 提供了多种等待和状态查询方式：

```cpp
#include <future>
#include <cstdio>
#include <chrono>

int slow_task() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 99;
}

int main() {
    std::future<int> fut = std::async(std::launch::async, slow_task);

    std::printf("等待前状态: ");
    auto status = fut.wait_for(std::chrono::milliseconds(0));
    switch (status) {
        case std::future_status::ready:   std::printf("就绪\n"); break;
        case std::future_status::timeout: std::printf("未就绪\n"); break;
        case std::future_status::deferred: std::printf("延迟执行\n"); break;
    }

    std::printf("等待 500ms...\n");
    status = fut.wait_for(std::chrono::milliseconds(500));
    if (status == std::future_status::timeout) {
        std::printf("超时，还没完成\n");
    }

    std::printf("等待完成...\n");
    fut.wait();
    std::printf("结果: %d\n", fut.get());
    return 0;
}
```

| 方法 | 行为 | 返回值 |
|------|------|--------|
| `get()` | 阻塞直到就绪，返回值（仅一次） | T 或抛异常 |
| `wait()` | 阻塞直到就绪 | void |
| `wait_for(duration)` | 最多等待一段时间 | `future_status` |
| `wait_until(time_point)` | 等到指定时间点 | `future_status` |
| `valid()` | 是否有共享状态 | bool |

| `future_status` | 含义 |
|-----------------|------|
| `ready` | 结果已就绪 |
| `timeout` | 等待超时，结果未就绪 |
| `deferred` | 任务延迟执行，尚未启动 |

***

### 7. 异常传播

`promise` 可以存储异常，`future::get()` 会重新抛出该异常：

```cpp
#include <future>
#include <cstdio>
#include <stdexcept>

int risky_task(int x) {
    if (x < 0) {
        throw std::runtime_error("负数不允许");
    }
    return x * 2;
}

int main() {
    auto fut1 = std::async(std::launch::async, risky_task, 5);
    auto fut2 = std::async(std::launch::async, risky_task, -1);

    try {
        std::printf("fut1: %d\n", fut1.get());
    } catch (const std::exception& e) {
        std::printf("fut1 异常: %s\n", e.what());
    }

    try {
        std::printf("fut2: %d\n", fut2.get());
    } catch (const std::exception& e) {
        std::printf("fut2 异常: %s\n", e.what());
    }
    return 0;
}
```

**手动设置异常**：

```cpp
#include <future>
#include <cstdio>
#include <stdexcept>

int main() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();

    try {
        throw std::runtime_error("内部错误");
    } catch (...) {
        prom.set_exception(std::current_exception());
    }

    try {
        fut.get();
    } catch (const std::exception& e) {
        std::printf("捕获到传播的异常: %s\n", e.what());
    }
    return 0;
}
```

**关键规则**：
- `set_value()` 和 `set_exception()` 只能调用其中一个，且只能调用一次
- 异步任务中未捕获的异常会自动通过 `future` 传播
- `get()` 是异常传播的唯一出口

***

### 8. future vs 回调

| 对比维度 | future/promise | 回调（Callback） |
|----------|---------------|-----------------|
| 获取结果方式 | 阻塞等待 `get()` | 异步通知 |
| 异常处理 | `get()` 重新抛出 | 需手动传递错误码 |
| 组合性 | 较弱（C++20 改善） | 可链式调用 |
| 代码可读性 | 顺序式，易理解 | 回调地狱 |
| 资源开销 | 共享状态堆分配 | 仅函数指针/闭包 |
| 适用场景 | 需要等待结果的场景 | 事件驱动、IO 回调 |

**模拟回调风格的 future**：

```cpp
#include <future>
#include <cstdio>
#include <thread>
#include <functional>

template<typename T>
void on_success(std::future<T>& fut, std::function<void(T)> callback) {
    std::thread([&fut, cb = std::move(callback)] {
        T result = fut.get();
        cb(result);
    }).detach();
}

int main() {
    std::future<int> fut = std::async(std::launch::async, [] {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        return 42;
    });

    on_success(fut, [](int result) {
        std::printf("回调收到结果: %d\n", result);
    });

    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
```

> **C++20 增强**：`std::future` 仍然不支持原生 then-continuation。如需链式异步，可使用第三方库（如 Boost.Asio、folly::Future）。

***

### 9. 常见错误与最佳实践

| 错误 | 后果 | 正确做法 |
|------|------|----------|
| `future::get()` 调用两次 | 第二次行为未定义/抛异常 | 只调用一次，或用 `shared_future` |
| 忽略 `async` 返回的 future | 析构时阻塞，可能死锁 | 保存 future 或显式 `wait()` |
| `promise` 未 set 就析构 | `future::get()` 抛 `std::future_error` | 确保 promise 一定 set_value 或 set_exception |
| `promise` 被多次 set | 抛 `std::future_error` | 只 set 一次 |
| `get()` 在主线程阻塞 | 整个程序卡住 | 使用 `wait_for` 带超时 |
| `deferred` 策略误用 | `get()` 时同步执行，失去并行性 | 需要并行时用 `launch::async` |
| `shared_future` 从 future 复制 | future 不可复制 | 先 `.share()` 再复制 |

**安全使用模板**：

```cpp
#include <future>
#include <cstdio>
#include <vector>

template<typename F, typename... Args>
auto run_async(F&& f, Args&&... args) -> std::future<decltype(f(args...))> {
    return std::async(std::launch::async, std::forward<F>(f), std::forward<Args>(args)...);
}

int compute(int a, int b) {
    return a + b;
}

int main() {
    auto f1 = run_async(compute, 1, 2);
    auto f2 = run_async(compute, 10, 20);

    if (f1.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
        std::printf("f1 = %d\n", f1.get());
    }
    if (f2.wait_for(std::chrono::seconds(5)) == std::future_status::ready) {
        std::printf("f2 = %d\n", f2.get());
    }
    return 0;
}
```

***

### 10. 多 future 等待与组合

C++ 没有原生的 `when_all` / `when_any`，但可以手动实现常见模式：

```cpp
#include <future>
#include <cstdio>
#include <vector>

int task(int id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * id));
    return id * 10;
}

void wait_all(std::vector<std::future<int>>& futures) {
    for (auto& f : futures) {
        f.wait();
    }
}

int main() {
    std::vector<std::future<int>> futures;
    for (int i = 1; i <= 5; ++i) {
        futures.push_back(std::async(std::launch::async, task, i));
    }

    wait_all(futures);

    for (int i = 0; i < 5; ++i) {
        std::printf("任务 %d 结果: %d\n", i + 1, futures[i].get());
    }
    return 0;
}
```

**等待任意一个完成**：

```cpp
#include <future>
#include <cstdio>
#include <atomic>
#include <mutex>

int main() {
    std::atomic<bool> done{false};
    std::mutex print_mtx;

    auto make_task = [&](int id, int ms) -> std::future<void> {
        return std::async(std::launch::async, [&, id, ms] {
            std::this_thread::sleep_for(std::chrono::milliseconds(ms));
            if (!done.exchange(true)) {
                std::lock_guard<std::mutex> lock(print_mtx);
                std::printf("任务 %d 最先完成（耗时 %dms）\n", id, ms);
            }
        });
    };

    auto f1 = make_task(1, 300);
    auto f2 = make_task(2, 100);
    auto f3 = make_task(3, 200);

    f1.wait();
    f2.wait();
    f3.wait();
    return 0;
}
```

***

### 11. 极简总结

| 概念 | 要点 |
|------|------|
| `promise` | 生产端，`set_value()` / `set_exception()`，仅一次 |
| `future` | 消费端，`get()` 阻塞获取，仅一次 |
| `shared_future` | 可复制，多次 `get()`，多线程共享结果 |
| `async` | 高层封装，`launch::async` 真并行，`deferred` 延迟同步 |
| `packaged_task` | 包装可调用对象，自动关联 promise/future |
| 异常传播 | `get()` 重新抛出异步任务中的异常 |
| 状态查询 | `wait_for` / `wait_until` 返回 `future_status` |
| vs 回调 | future 顺序式等待，回调异步通知；各有适用场景 |
| 常见错误 | get 两次、忽略 future、promise 未 set |
| 最佳实践 | 用 `launch::async` 确保并行、带超时等待、RAII 管理 |

***

### 相关阅读

- [什么是信号量Semaphore](17-什么是信号量Semaphore.md)
- [同步和异步](./13-同步和异步.md)
- [多线程通讯与数据获取](23-多线程通讯与数据获取.md)

***