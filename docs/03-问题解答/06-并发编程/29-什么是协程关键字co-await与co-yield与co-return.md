# 什么是协程关键字co_await与co_yield与co_return
> 📖 相关章节：[多线程基础](../../02-CPP/27-多线程基础.md)、[线程同步](../../02-CPP/28-线程同步.md)、[原子操作](../../02-CPP/29-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

> 协程不是线程，它是可暂停的函数——三个关键字定义了暂停、产出与完成的语义。

***

### 1. 本质洞察

C++20 协程通过 `co_await`（挂起等待）、`co_yield`（产出值并挂起）、`co_return`（完成返回）三个关键字，让函数可以在执行中途暂停、恢复，实现惰性生成与异步等待。

***

### 2. 协程 vs 普通函数：根本区别

| 特性 | 普通函数 | 协程 |
|------|---------|------|
| 执行模型 | 调用后一直运行到 return | 可在中途挂起，稍后恢复 |
| 栈帧 | 在调用栈上 | 协程帧在堆上 |
| 状态管理 | 无需保存中间状态 | 编译器自动保存局部变量到协程帧 |
| 关键字 | `return` | `co_await` / `co_yield` / `co_return` |
| 返回类型 | 直接返回值 | 返回协程句柄/代理对象 |
| C++ 标准 | C++98 起 | C++20 起 |

```cpp
#include <iostream>

int normal_func(int x) {
    int y = x * 2;
    return y;
}

#include <coroutine>

Generator coroutine_func(int x) {
    int y = x * 2;
    co_yield y;
    co_yield y + 1;
}
```

> **关键认知**：只要函数体中出现任意一个 `co_await`、`co_yield` 或 `co_return`，编译器就将其视为协程，自动进行协程帧分配和状态机转换。

***

### 3. 协程帧与 promise_type 机制

协程帧是编译器在堆上分配的结构，保存了协程恢复所需的全部状态：

```
┌─────────────────────────────┐
│        协程帧 (堆上)         │
├─────────────────────────────┤
│ promise 对象                 │
│ 局部变量                     │
│ 挂起点索引 (状态机)          │
│ 参数副本                     │
│ awaiter 状态                 │
└─────────────────────────────┘
```

`promise_type` 是协程的"控制面板"，定义了协程生命周期各阶段的行为：

```cpp
#include <coroutine>
#include <iostream>

struct MyPromise {
    int current_value;

    struct suspend_never initial_suspend() noexcept { return {}; }
    struct suspend_always final_suspend() noexcept { return {}; }
    void unhandled_exception() { std::terminate(); }

    MyCoroutine get_return_object();

    void return_value(int v) {
        current_value = v;
        std::cout << "co_return 值: " << v << "\n";
    }

    auto yield_value(int v) {
        current_value = v;
        std::cout << "co_yield 值: " << v << "\n";
        return std::suspend_always{};
    }
};
```

| promise_type 方法 | 调用时机 | 作用 |
|-------------------|---------|------|
| `get_return_object()` | 协程体执行前 | 创建返回给调用者的对象 |
| `initial_suspend()` | 协程体执行前 | 决定是否立即挂起 |
| `final_suspend()` | 协程体执行完毕 | 决定是否在结束时挂起 |
| `return_value()` / `return_void()` | `co_return` 时 | 处理返回值 |
| `yield_value()` | `co_yield` 时 | 处理产出值 |
| `unhandled_exception()` | 异常逃逸时 | 处理未捕获异常 |

***

### 4. co_await：挂起与等待

`co_await expr` 是协程的核心原语，表示"在此处挂起，等待某个操作完成后再恢复"。

**执行流程：**

```
co_await expr
    │
    ▼
awaiter = promise.await_transform(expr)  (如果定义)
    │
    ▼
await_ready() ──返回 true──→ 直接继续，不挂起
    │
    返回 false
    ▼
await_suspend(handle) ──返回 void──→ 挂起，稍后由外部 resume
    │                     │
    │                  返回 false
    │                     │
    │                  返回另一个 handle → 切换到该协程
    ▼
await_resume() → co_await 表达式的结果
```

**Awaiter 三件套：**

```cpp
struct AsyncRead {
    bool await_ready() noexcept {
        return false;
    }

    void await_suspend(std::coroutine_handle<> h) noexcept {
        std::thread([h]() mutable {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            h.resume();
        }).detach();
    }

    int await_resume() noexcept {
        return 42;
    }
};

Task async_read() {
    int result = co_await AsyncRead{};
    std::cout << "读取结果: " << result << "\n";
}
```

| awaiter 方法 | 返回类型 | 含义 |
|-------------|---------|------|
| `await_ready()` | `bool` | `true` = 操作已完成，无需挂起 |
| `await_suspend(handle)` | `void`/`bool`/`handle` | `void`=必定挂起，`false`=不挂起，`handle`=切换协程 |
| `await_resume()` | 任意 | 挂起恢复后的返回值 |

> **平台注意**：MSVC 需要使用 `/await` 或 `/std:c++20` 编译选项；GCC 12+ 和 Clang 14+ 默认支持 `<coroutine>` 头文件。

***

### 5. co_yield：产出值并挂起

`co_yield expr` 等价于 `co_await promise.yield_value(expr)`，是"产出当前值，然后挂起"的语法糖。

```cpp
#include <coroutine>
#include <memory>

template<typename T>
struct Generator {
    struct promise_type {
        T current_value;

        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}

        std::suspend_always yield_value(T value) {
            current_value = value;
            return {};
        }
    };

    std::coroutine_handle<promise_type> handle;

    Generator(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Generator() { if (handle) handle.destroy(); }

    struct Iterator {
        std::coroutine_handle<promise_type> handle;
        bool operator!=(std::default_sentinel_t) const { return !handle.done(); }
        Iterator& operator++() { handle.resume(); return *this; }
        T operator*() const { return handle.promise().current_value; }
    };

    Iterator begin() { handle.resume(); return {handle}; }
    std::default_sentinel_t end() { return {}; }
};

Generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;
        auto tmp = a;
        a = b;
        b = tmp + b;
    }
}

int main() {
    int count = 0;
    for (auto val : fibonacci()) {
        std::cout << val << " ";
        if (++count >= 10) break;
    }
}
```

**co_yield 与 co_await 的关系：**

| 对比项 | co_yield | co_await |
|--------|---------|---------|
| 语义 | 产出值 + 挂起 | 等待操作 + 可能挂起 |
| 等价写法 | `co_await promise.yield_value(expr)` | `co_await expr` |
| 典型场景 | 生成器、数据流 | 异步 I/O、定时器 |
| 是否有返回值 | 无 | `await_resume()` 的返回值 |
| promise_type 方法 | `yield_value()` | `await_transform()` (可选) |

***

### 6. co_return：完成协程

`co_return` 表示协程执行完毕，与普通 `return` 的关键区别：

| 对比项 | return | co_return |
|--------|--------|-----------|
| 适用范围 | 普通函数 | 协程 |
| 返回值处理 | 直接返回 | 调用 `promise.return_value()` |
| 无返回值 | `return;` | `co_return;` 调用 `promise.return_void()` |
| 之后行为 | 函数栈帧销毁 | 协程进入 `final_suspend` |

```cpp
struct Task {
    struct promise_type {
        int result_;

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::terminate(); }

        void return_value(int v) {
            result_ = v;
        }

        void return_void() {
            result_ = 0;
        }
    };

    std::coroutine_handle<promise_type> handle;

    Task(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Task() { if (handle) handle.destroy(); }

    int get_result() { return handle.promise().result_; }
};

Task compute() {
    co_return 42;
}

Task compute_void() {
    co_return;
}
```

> **注意**：协程中不能使用普通 `return`，必须使用 `co_return`。混用会导致编译错误。

***

### 7. 完整 Generator 示例：范围生成器

```cpp
#include <coroutine>
#include <iostream>

template<typename T>
struct RangeGenerator {
    struct promise_type {
        T value_;

        RangeGenerator get_return_object() {
            return RangeGenerator{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::terminate(); }
        void return_void() {}

        std::suspend_always yield_value(T v) {
            value_ = v;
            return {};
        }
    };

    struct Iter {
        std::coroutine_handle<promise_type> coro_;

        T& operator*() { return coro_.promise().value_; }
        Iter& operator++() {
            coro_.resume();
            return *this;
        }
        bool operator!=(std::default_sentinel_t) const {
            return !coro_.done();
        }
    };

    std::coroutine_handle<promise_type> coro_;

    RangeGenerator(std::coroutine_handle<promise_type> h) : coro_(h) {}
    ~RangeGenerator() { if (coro_) coro_.destroy(); }

    Iter begin() {
        coro_.resume();
        return Iter{coro_};
    }
    std::default_sentinel_t end() { return {}; }
};

RangeGenerator<int> range(int start, int end, int step = 1) {
    for (int i = start; i < end; i += step) {
        co_yield i;
    }
}

int main() {
    for (auto v : range(0, 10, 2)) {
        std::cout << v << " ";
    }
}
```

输出：

```
0 2 4 6 8
```

***

### 8. 完整 Task 示例：异步任务链

```cpp
#include <coroutine>
#include <iostream>
#include <functional>
#include <queue>

struct Task {
    struct promise_type;
    using handle_type = std::coroutine_handle<promise_type>;

    struct promise_type {
        int result_;
        std::coroutine_handle<> continuation_;

        Task get_return_object() {
            return Task{handle_type::from_promise(*this)};
        }
        std::suspend_never initial_suspend() { return {}; }
        auto final_suspend() noexcept {
            struct FinalAwaiter {
                bool await_ready() noexcept { return false; }
                std::coroutine_handle<> await_suspend(handle_type h) noexcept {
                    auto cont = h.promise().continuation_;
                    if (cont) return cont;
                    return std::noop_coroutine();
                }
                void await_resume() noexcept {}
            };
            return FinalAwaiter{};
        }
        void unhandled_exception() { std::terminate(); }
        void return_value(int v) { result_ = v; }
    };

    handle_type handle_;

    Task(handle_type h) : handle_(h) {}
    ~Task() { if (handle_) handle_.destroy(); }

    struct Awaiter {
        handle_type handle;
        bool await_ready() { return false; }
        handle_type await_suspend(std::coroutine_handle<> h) {
            handle.promise().continuation_ = h;
            return handle;
        }
        int await_resume() { return handle.promise().result_; }
    };

    auto operator co_await() { return Awaiter{handle_}; }
};

Task add_async(int a, int b) {
    co_return a + b;
}

Task multiply_async(int a, int b) {
    co_return a * b;
}

Task compute() {
    int sum = co_await add_async(3, 4);
    int product = co_await multiply_async(sum, 2);
    std::cout << "结果: " << product << "\n";
    co_return product;
}

int main() {
    auto task = compute();
}
```

***

### 9. 协程状态机：编译器视角

编译器将协程转换为状态机，每个挂起点对应一个状态：

```cpp
Generator<int> simple_coro() {
    co_yield 1;
    co_yield 2;
    co_return;
}
```

编译器等价转换（伪代码）：

```
状态 0: initial_suspend → 挂起
状态 1: yield_value(1) → 挂起
状态 2: yield_value(2) → 挂起
状态 3: return_void → final_suspend
```

| 状态 | 执行位置 | 恢复后跳转 |
|------|---------|-----------|
| 0 | 协程入口 | → 状态 1 |
| 1 | 第一个 co_yield 之后 | → 状态 2 |
| 2 | 第二个 co_yield 之后 | → 状态 3 |
| 3 | final_suspend | 协程结束 |

> **性能提示**：每个挂起点都会增加状态机的一个分支，但开销极小（一次 switch-case 跳转），远低于线程上下文切换。

***

### 10. 三个关键字的完整对比

| 维度 | co_await | co_yield | co_return |
|------|---------|---------|-----------|
| 语义 | 等待异步操作 | 产出值并挂起 | 完成协程 |
| 等价展开 | `co_await expr` | `co_await promise.yield_value(expr)` | `promise.return_value(v)` |
| 是否挂起 | 取决于 `await_ready()` | 始终挂起 | 不挂起，进入 final_suspend |
| 返回值 | `await_resume()` 的结果 | 无 | 无 |
| promise_type 方法 | `await_transform()` (可选) | `yield_value()` | `return_value()` / `return_void()` |
| 典型用途 | 异步 I/O、定时器 | 生成器、流式数据 | 返回最终结果 |
| 可出现次数 | 任意次 | 任意次 | 最多一次 |

***

### 11. 极简总结

| 概念 | 核心要点 |
|------|---------|
| `co_await` | 挂起当前协程，等待操作完成后恢复，可获取返回值 |
| `co_yield` | 产出值并挂起，等价 `co_await promise.yield_value(v)` |
| `co_return` | 结束协程，调用 `promise.return_value()` 或 `return_void()` |
| 协程帧 | 堆上分配，保存局部变量、挂起点、promise 对象 |
| promise_type | 协程行为控制面板，定义各生命周期钩子 |
| awaiter | 定义 `await_ready/suspend/resume`，控制挂起与恢复逻辑 |
| Generator | 惰性序列，用 `co_yield` 逐个产出值 |
| Task | 异步任务，用 `co_await` 串联多个异步操作 |
| 状态机 | 编译器将协程转为 switch-case 状态机，开销极低 |

**记忆口诀**：`co_await` 等、`co_yield` 产、`co_return` 完；promise 管生命周期，awaiter 管挂起恢复。

***

### 相关阅读

- [协程与线程](28-协程与线程.md)
- [什么是回调函数](../04-CPP核心特性/27-什么是回调函数.md)
- [latch与barrier](30-latch与barrier.md)

***