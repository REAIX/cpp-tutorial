/**
 * @file 02_deep_dive_coroutine_patterns.cpp
 * @brief 协程高级模式与常见错误
 * @description 对应文档: 02-CPP/26-coroutine
 */

#include <iostream>
#include <coroutine>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <queue>
#include <functional>
#include <chrono>
#include <atomic>
#include <condition_variable>

class SimpleThreadPool {
public:
    SimpleThreadPool(int count = 2) : stop_(false) {
        for (int i = 0; i < count; ++i) {
            workers_.emplace_back([this]() { worker_loop(); });
        }
    }

    ~SimpleThreadPool() {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& w : workers_) {
            if (w.joinable()) w.join();
        }
    }

    void submit(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.push(std::move(task));
        }
        cv_.notify_one();
    }

private:
    void worker_loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(mutex_);
                cv_.wait(lock, [this]() { return stop_ || !tasks_.empty(); });
                if (stop_ && tasks_.empty()) return;
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            task();
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    bool stop_;
};

SimpleThreadPool& get_pool() {
    static SimpleThreadPool pool(2);
    return pool;
}

template<typename T>
class AsyncResult {
public:
    struct promise_type {
        T value;
        std::exception_ptr exception;

        AsyncResult get_return_object() {
            return AsyncResult{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_value(T v) { value = std::move(v); }
        void unhandled_exception() { exception = std::current_exception(); }
    };

    AsyncResult(std::coroutine_handle<promise_type> h) : handle_(h) {}
    ~AsyncResult() { if (handle_ && !handle_.done()) handle_.destroy(); }

    AsyncResult(const AsyncResult&) = delete;
    AsyncResult& operator=(const AsyncResult&) = delete;
    AsyncResult(AsyncResult&& o) noexcept : handle_(o.handle_) { o.handle_ = nullptr; }

    bool ready() const { return handle_.done(); }
    T get() {
        if (handle_.promise().exception) std::rethrow_exception(handle_.promise().exception);
        return std::move(handle_.promise().value);
    }

    auto operator co_await() {
        struct Awaiter {
            std::coroutine_handle<promise_type> handle;
            bool await_ready() { return handle.done(); }
            void await_suspend(std::coroutine_handle<> waiter) {
                get_pool().submit([waiter]() { waiter.resume(); });
            }
            T await_resume() {
                if (handle.promise().exception) std::rethrow_exception(handle.promise().exception);
                return std::move(handle.promise().value);
            }
        };
        return Awaiter{handle_};
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

struct ThreadPoolAwaiter {
    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> handle) {
        get_pool().submit([handle]() { handle.resume(); });
    }
    void await_resume() {}
};

AsyncResult<int> async_compute_on_pool(int x) {
    co_await ThreadPoolAwaiter{};
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    co_return x * x;
}

AsyncResult<int> chained_async() {
    std::cout << "  开始链式异步计算\n";
    int a = co_await async_compute_on_pool(3);
    std::cout << "  第一步: " << a << "\n";
    int b = co_await async_compute_on_pool(4);
    std::cout << "  第二步: " << b << "\n";
    int c = co_await async_compute_on_pool(5);
    std::cout << "  第三步: " << c << "\n";
    co_return a + b + c;
}

template<typename T>
class Generator {
public:
    struct promise_type {
        T value;
        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(T v) { value = std::move(v); return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    Generator(std::coroutine_handle<promise_type> h) : handle_(h) {}
    ~Generator() { if (handle_) handle_.destroy(); }

    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    Generator(Generator&& o) noexcept : handle_(o.handle_) { o.handle_ = nullptr; }

    bool next() {
        if (handle_ && !handle_.done()) {
            handle_.resume();
            return !handle_.done();
        }
        return false;
    }

    T value() const { return handle_.promise().value; }

private:
    std::coroutine_handle<promise_type> handle_;
};

Generator<int> gen_range(int start, int end) {
    for (int i = start; i < end; ++i) co_yield i;
}

template<typename T>
Generator<T> generator_filter(Generator<T> source, std::function<bool(const T&)> pred) {
    while (source.next()) {
        if (pred(source.value())) co_yield source.value();
    }
}

template<typename T>
Generator<T> generator_transform(Generator<T> source, std::function<T(const T&)> func) {
    while (source.next()) {
        co_yield func(source.value());
    }
}

void demo_async_io() {
    std::cout << "\n=== 异步I/O与协程 ===\n";

    auto result = chained_async();
    while (!result.ready()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "链式异步结果: " << result.get() << "\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  网络I/O: co_await socket.read() / socket.write()\n";
    std::cout << "  文件I/O: co_await file.read_async() / file.write_async()\n";
    std::cout << "  数据库: co_await db.query(\"SELECT ...\")\n";
    std::cout << "  定时器: co_await timer.wait(100ms)\n";
}

void demo_generator_composition() {
    std::cout << "\n=== 生成器组合 ===\n";

    auto numbers = gen_range(1, 20);
    auto even = generator_filter<int>(std::move(numbers), [](const int& n) { return n % 2 == 0; });
    auto squared = generator_transform<int>(std::move(even), [](const int& n) { return n * n; });

    std::cout << "1-20中偶数的平方: ";
    while (squared.next()) {
        std::cout << squared.value() << " ";
    }
    std::cout << "\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  生成器可以像Ranges一样组合\n";
    std::cout << "  但生成器是pull模型, Ranges也是pull模型\n";
    std::cout << "  生成器适合动态/无限序列, Ranges适合已有容器\n";
}

void demo_coroutine_thread_pool() {
    std::cout << "\n=== 协程与线程池 ===\n";

    std::cout << "协程+线程池模式:\n";
    std::cout << "  1. 协程在需要阻塞操作时, co_await切换到线程池\n";
    std::cout << "  2. 线程池完成操作后, resume协程\n";
    std::cout << "  3. 协程继续执行, 可能调度到不同线程\n";

    std::cout << "\n注意事项:\n";
    std::cout << "  thread_local变量在协程resume时可能不同\n";
    std::cout << "  需要确保线程安全或使用事件循环序列化\n";
    std::cout << "  避免在协程中使用互斥锁(可能死锁)\n";

    auto r1 = async_compute_on_pool(10);
    auto r2 = async_compute_on_pool(20);
    while (!r1.ready() || !r2.ready()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "并行计算: " << r1.get() << " + " << r2.get() << "\n";
}

void demo_common_mistakes() {
    std::cout << "\n=== 协程常见错误 ===\n";

    std::cout << "1. 悬垂引用:\n";
    std::cout << "   const std::string& ref = co_await get_string();\n";
    std::cout << "   如果返回临时对象, ref会悬垂!\n";
    std::cout << "   修复: 用值捕获 auto val = co_await get_string();\n";

    std::cout << "\n2. 忘记resume:\n";
    std::cout << "   协程在initial_suspend挂起后, 如果不resume,\n";
    std::cout << "   协程永远不会执行, 但帧也不会被销毁 -> 内存泄漏\n";
    std::cout << "   修复: 确保RAII管理coroutine_handle\n";

    std::cout << "\n3. 忘记destroy:\n";
    std::cout << "   如果协程在final_suspend返回suspend_always,\n";
    std::cout << "   协程结束后帧不会自动释放, 必须手动destroy\n";
    std::cout << "   修复: 在返回对象的析构函数中调用destroy\n";

    std::cout << "\n4. 在协程中使用阻塞操作:\n";
    std::cout << "   std::this_thread::sleep_for()会阻塞整个线程\n";
    std::cout << "   如果运行在单线程事件循环上, 会阻塞所有协程\n";
    std::cout << "   修复: 使用异步等待 co_await async_sleep()\n";

    std::cout << "\n5. 异常安全:\n";
    std::cout << "   协程中抛出异常, 如果unhandled_exception()调用terminate\n";
    std::cout << "   应该在unhandled_exception中保存异常, 在get_result时rethrow\n";

    std::cout << "\n6. 生命周期问题:\n";
    std::cout << "   协程按引用捕获外部变量, 如果外部变量先于协程销毁\n";
    std::cout << "   协程resume时访问悬垂引用 -> 未定义行为\n";
    std::cout << "   修复: 尽量按值捕获, 或确保生命周期\n";
}

void demo_best_practices() {
    std::cout << "\n=== 协程最佳实践 ===\n";

    std::cout << "1. 使用RAII管理coroutine_handle\n";
    std::cout << "   返回对象析构时自动destroy\n";

    std::cout << "\n2. 优先使用suspend_always作为final_suspend\n";
    std::cout << "   允许调用者在协程结束后获取结果\n";
    std::cout << "   避免在协程帧销毁后访问promise\n";

    std::cout << "\n3. 使用对称转移避免栈溢出\n";
    std::cout << "   深度递归的协程链可能导致栈溢出\n";
    std::cout << "   await_suspend返回handle实现尾调用优化\n";

    std::cout << "\n4. 区分lazy和eager协程\n";
    std::cout << "   lazy: initial_suspend返回suspend_always\n";
    std::cout << "   eager: initial_suspend返回suspend_never\n";
    std::cout << "   Generator用lazy, Task用eager\n";

    std::cout << "\n5. 考虑使用成熟的协程库\n";
    std::cout << "   cppcoro (原始版, C++20协程)\n";
    std::cout << "   concurrencpp (完整的异步框架)\n";
    std::cout << "   folly/coro (Facebook的高性能协程)\n";
}

int main() {
    std::cout << "========== 协程高级模式与常见错误 ==========\n";
    std::cout << "注意: GCC可能需要 -fcoroutines 编译选项\n";

    demo_async_io();
    demo_generator_composition();
    demo_coroutine_thread_pool();
    demo_common_mistakes();
    demo_best_practices();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
