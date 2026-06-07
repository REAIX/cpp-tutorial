/**
 * @file 03_example_task.cpp
 * @brief Task异步任务类型
 * @description 对应文档: 02-CPP/26-coroutine
 */

#include <iostream>
#include <coroutine>
#include <vector>
#include <functional>
#include <string>
#include <chrono>
#include <thread>
#include <queue>
#include <mutex>

class EventLoop {
public:
    static EventLoop& instance() {
        static EventLoop loop;
        return loop;
    }

    void post(std::function<void()> task) {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.push(std::move(task));
    }

    void run_once() {
        std::function<void()> task;
        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (tasks_.empty()) return;
            task = std::move(tasks_.front());
            tasks_.pop();
        }
        task();
    }

    void run_until_empty() {
        while (true) {
            std::function<void()> task;
            {
                std::lock_guard<std::mutex> lock(mutex_);
                if (tasks_.empty()) break;
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            task();
        }
    }

private:
    EventLoop() = default;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
};

template<typename T = void>
class Task {
public:
    struct promise_type {
        T result_value;
        std::exception_ptr exception;

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_value(T val) { result_value = std::move(val); }
        void unhandled_exception() { exception = std::current_exception(); }
    };

    Task(std::coroutine_handle<promise_type> h) : handle_(h) {}
    ~Task() {
        if (handle_ && !handle_.done()) handle_.destroy();
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;
    Task(Task&& other) noexcept : handle_(other.handle_) {
        other.handle_ = nullptr;
    }

    bool is_ready() const { return handle_.done(); }

    T get_result() {
        if (handle_.promise().exception) {
            std::rethrow_exception(handle_.promise().exception);
        }
        return std::move(handle_.promise().result_value);
    }

    auto operator co_await() {
        struct Awaiter {
            std::coroutine_handle<promise_type> handle;

            bool await_ready() { return handle.done(); }
            void await_suspend(std::coroutine_handle<> waiting_handle) {
                EventLoop::instance().post([waiting_handle]() {
                    waiting_handle.resume();
                });
            }
            T await_resume() {
                if (handle.promise().exception) {
                    std::rethrow_exception(handle.promise().exception);
                }
                return std::move(handle.promise().result_value);
            }
        };
        return Awaiter{handle_};
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

template<>
class Task<void> {
public:
    struct promise_type {
        std::exception_ptr exception;

        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { exception = std::current_exception(); }
    };

    Task(std::coroutine_handle<promise_type> h) : handle_(h) {}
    ~Task() {
        if (handle_ && !handle_.done()) handle_.destroy();
    }

    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    bool is_ready() const { return handle_.done(); }

    void get_result() {
        if (handle_.promise().exception) {
            std::rethrow_exception(handle_.promise().exception);
        }
    }

private:
    std::coroutine_handle<promise_type> handle_;
};

struct AsyncTimer {
    int milliseconds;

    bool await_ready() { return false; }
    void await_suspend(std::coroutine_handle<> handle) {
        EventLoop::instance().post([handle]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            handle.resume();
        });
    }
    void await_resume() {}
};

Task<int> async_compute(int x) {
    std::cout << "  开始计算: " << x << "\n";
    co_await AsyncTimer{10};
    int result = x * x;
    std::cout << "  计算完成: " << x << "^2 = " << result << "\n";
    co_return result;
}

Task<int> chained_compute() {
    std::cout << "链式计算开始\n";
    int a = co_await async_compute(3);
    int b = co_await async_compute(4);
    int c = co_await async_compute(5);
    co_return a + b + c;
}

Task<std::string> async_fetch_data(const std::string& query) {
    std::cout << "  获取数据: " << query << "\n";
    co_await AsyncTimer{5};
    co_return "result_of_" + query;
}

Task<void> async_process() {
    std::cout << "处理流程开始\n";
    auto data1 = co_await async_fetch_data("users");
    std::cout << "  得到: " << data1 << "\n";
    auto data2 = co_await async_fetch_data("orders");
    std::cout << "  得到: " << data2 << "\n";
    std::cout << "处理流程完成\n";
}

Task<int> when_all_demo() {
    std::cout << "when_all概念演示(顺序执行模拟):\n";

    auto t1 = async_compute(10);
    auto t2 = async_compute(20);
    auto t3 = async_compute(30);

    EventLoop::instance().run_until_empty();

    int r1 = t1.get_result();
    int r2 = t2.get_result();
    int r3 = t3.get_result();

    co_return r1 + r2 + r3;
}

void demo_basic_task() {
    std::cout << "\n=== 基础Task ===\n";

    auto task = async_compute(7);
    EventLoop::instance().run_until_empty();
    std::cout << "结果: " << task.get_result() << "\n";
}

void demo_chained_task() {
    std::cout << "\n=== 链式Task ===\n";

    auto task = chained_compute();
    EventLoop::instance().run_until_empty();
    std::cout << "链式结果: " << task.get_result() << "\n";
}

void demo_async_process() {
    std::cout << "\n=== 异步处理流程 ===\n";

    auto task = async_process();
    EventLoop::instance().run_until_empty();
}

void demo_when_all() {
    std::cout << "\n=== when_all概念 ===\n";

    auto task = when_all_demo();
    EventLoop::instance().run_until_empty();
    std::cout << "when_all结果: " << task.get_result() << "\n";
}

int main() {
    std::cout << "========== Task异步任务类型 ==========\n";
    std::cout << "注意: GCC可能需要 -fcoroutines 编译选项\n";

    demo_basic_task();
    demo_chained_task();
    demo_async_process();
    demo_when_all();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
