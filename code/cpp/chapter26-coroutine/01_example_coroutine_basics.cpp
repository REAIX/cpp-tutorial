/**
 * @file 01_example_coroutine_basics.cpp
 * @brief 协程基础示例
 * @description 对应文档: 02-CPP/26-coroutine
 */

#include <iostream>
#include <coroutine>
#include <string>
#include <exception>

struct Generator {
    struct promise_type {
        int current_value;

        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        std::suspend_always yield_value(int value) {
            current_value = value;
            return {};
        }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    Generator(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Generator() {
        if (handle && !handle.done()) handle.destroy();
    }

    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    Generator(Generator&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }

    bool next() {
        if (handle && !handle.done()) {
            handle.resume();
            return !handle.done();
        }
        return false;
    }

    int value() const {
        return handle.promise().current_value;
    }
};

Generator simple_counter(int start, int end) {
    for (int i = start; i <= end; ++i) {
        co_yield i;
    }
}

Generator fibonacci(int count) {
    int a = 0, b = 1;
    for (int i = 0; i < count; ++i) {
        co_yield a;
        auto temp = a;
        a = b;
        b = temp + b;
    }
}

struct Task {
    struct promise_type {
        int result;

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_value(int val) { result = val; }
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;

    Task(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Task() {
        if (handle && !handle.done()) handle.destroy();
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    int get_result() {
        if (!handle.done()) handle.resume();
        return handle.promise().result;
    }
};

Task compute_sum(int a, int b) {
    co_return a + b;
}

struct Awaiter {
    int value;

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> h) {
        h.resume();
    }
    int await_resume() { return value; }
};

Task demo_co_await() {
    std::cout << "  co_await前\n";
    int val = co_await Awaiter{42};
    std::cout << "  co_await后, 得到: " << val << "\n";
    co_return val * 2;
}

void demo_co_yield() {
    std::cout << "\n=== co_yield基础 ===\n";

    std::cout << "简单计数器(1-5):\n";
    auto gen = simple_counter(1, 5);
    while (gen.next()) {
        std::cout << "  " << gen.value() << "\n";
    }

    std::cout << "\n斐波那契数列(前10个):\n";
    auto fib = fibonacci(10);
    while (fib.next()) {
        std::cout << "  " << fib.value() << "\n";
    }
}

void demo_co_return() {
    std::cout << "\n=== co_return基础 ===\n";

    auto task = compute_sum(10, 20);
    std::cout << "compute_sum(10, 20) = " << task.get_result() << "\n";
}

void demo_co_await_basic() {
    std::cout << "\n=== co_await基础 ===\n";

    auto task = demo_co_await();
    std::cout << "co_await结果: " << task.get_result() << "\n";
}

void demo_coroutine_handle() {
    std::cout << "\n=== coroutine_handle操作 ===\n";

    auto gen = simple_counter(100, 105);

    std::cout << "初始状态 - 协程挂起在initial_suspend\n";
    gen.handle.resume();
    std::cout << "第一次resume后: " << gen.value() << "\n";

    std::cout << "继续resume:\n";
    while (!gen.handle.done()) {
        gen.handle.resume();
        if (!gen.handle.done()) {
            std::cout << "  " << gen.value() << "\n";
        }
    }
    std::cout << "协程已完成(done)\n";
}

void demo_promise_type() {
    std::cout << "\n=== promise_type定制点 ===\n";

    std::cout << "promise_type是协程的核心定制机制:\n";
    std::cout << "  get_return_object() -> 创建返回对象\n";
    std::cout << "  initial_suspend() -> 协程启动时是否挂起\n";
    std::cout << "  final_suspend() -> 协程结束时是否挂起\n";
    std::cout << "  yield_value() -> co_yield的处理\n";
    std::cout << "  return_value()/return_void() -> co_return的处理\n";
    std::cout << "  unhandled_exception() -> 异常处理\n";

    std::cout << "\nsuspend_always vs suspend_never:\n";
    std::cout << "  suspend_always: 总是挂起(惰性执行)\n";
    std::cout << "  suspend_never: 从不挂起(急切执行)\n";
}

int main() {
    std::cout << "========== C++20 协程基础示例 ==========\n";
    std::cout << "注意: GCC可能需要 -fcoroutines 编译选项\n";

    demo_co_yield();
    demo_co_return();
    demo_co_await_basic();
    demo_coroutine_handle();
    demo_promise_type();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
