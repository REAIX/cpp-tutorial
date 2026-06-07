/**
 * @file 02_deep_dive_async_patterns.cpp
 * @brief 异步编程模式深入探讨
 * @description 对应文档: 02-CPP/31-atomic-async
 */

#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <chrono>
#include <mutex>
#include <queue>
#include <functional>
#include <atomic>
#include <condition_variable>

void demo_async_patterns() {
    std::cout << "\n=== 异步编程模式 ===\n";

    std::cout << "1. Fire-and-Forget(发射后不管):\n";
    {
        std::vector<std::future<void>> futures;
        for (int i = 0; i < 5; ++i) {
            futures.push_back(std::async(std::launch::async, [i]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(i * 10));
                std::cout << "  任务" << i << "完成\n";
            }));
        }
    }
    std::cout << "  所有fire-and-forget任务完成\n";

    std::cout << "\n2. 请求-响应:\n";
    {
        auto response = std::async(std::launch::async, []() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            return std::string("响应数据");
        });
        std::cout << "  等待响应...\n";
        std::cout << "  收到: " << response.get() << "\n";
    }

    std::cout << "\n3. 流水线:\n";
    {
        auto stage1 = std::async(std::launch::async, []() {
            return std::string("raw_data");
        });
        auto stage2 = std::async(std::launch::async, [&stage1]() {
            return "processed_" + stage1.get();
        });
        auto stage3 = std::async(std::launch::async, [&stage2]() {
            return "final_" + stage2.get();
        });
        std::cout << "  流水线结果: " << stage3.get() << "\n";
    }
}

template<typename T>
std::vector<std::future<T>> when_all(std::vector<std::future<T>> futures) {
    return std::move(futures);
}

template<typename T>
T collect_all(std::vector<std::future<T>>& futures) {
    T total{};
    for (auto& f : futures) {
        total += f.get();
    }
    return total;
}

void demo_when_all_pattern() {
    std::cout << "\n=== when_all模式 ===\n";

    std::vector<std::future<int>> futures;
    for (int i = 1; i <= 5; ++i) {
        futures.push_back(std::async(std::launch::async, [i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            return i * i;
        }));
    }

    std::cout << "所有任务结果: ";
    std::vector<int> results;
    for (auto& f : futures) {
        results.push_back(f.get());
    }
    for (int r : results) std::cout << r << " ";
    std::cout << "\n";

    int sum = 0;
    for (int r : results) sum += r;
    std::cout << "总和: " << sum << "\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  C++标准库没有when_all, 但可以手动实现\n";
    std::cout << "  C++23的std::expected可以与future结合\n";
    std::cout << "  协程可以更优雅地实现when_all\n";
}

void demo_when_any_pattern() {
    std::cout << "\n=== when_any模式 ===\n";

    std::vector<std::future<int>> futures;
    for (int i = 0; i < 5; ++i) {
        futures.push_back(std::async(std::launch::async, [i]() {
            int delay = (5 - i) * 30;
            std::this_thread::sleep_for(std::chrono::milliseconds(delay));
            return i;
        }));
    }

    std::cout << "等待任意一个完成:\n";
    bool found = false;
    while (!found) {
        for (size_t i = 0; i < futures.size(); ++i) {
            if (futures[i].wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
                std::cout << "  最先完成: 任务" << futures[i].get() << "\n";
                found = true;
                break;
            }
        }
        if (!found) {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    std::cout << "\nwhen_any: 等待任意一个future完成\n";
    std::cout << "用途: 超时竞赛, 冗余请求, 快速响应\n";
}

void demo_continuation_pattern() {
    std::cout << "\n=== 续延(Continuation)模式 ===\n";

    std::cout << "C++标准future不支持.then(), 但可以模拟:\n";

    auto chain = [](std::future<int>&& f, auto func) -> std::future<decltype(func(f.get()))> {
        return std::async(std::launch::async, [f = std::move(f), func]() mutable {
            return func(f.get());
        });
    };

    auto f = std::async(std::launch::async, []() { return 5; });

    auto f2 = chain(std::move(f), [](int x) { return x * 2; });
    auto f3 = chain(std::move(f2), [](int x) { return x + 10; });
    auto f4 = chain(std::move(f3), [](int x) { return std::to_string(x); });

    std::cout << "链式结果: " << f4.get() << "\n";

    std::cout << "\n更好的方案:\n";
    std::cout << "  1. 使用协程(co_await)\n";
    std::cout << "  2. 使用第三方库(boost::future, folly::Future)\n";
    std::cout << "  3. C++23可能引入std::future的.then()\n";
}

class SimpleThreadPool {
    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};

public:
    explicit SimpleThreadPool(size_t threads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < threads; ++i) {
            workers_.emplace_back([this]() {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(mutex_);
                        cv_.wait(lock, [this]() { return stop_.load() || !tasks_.empty(); });
                        if (stop_.load() && tasks_.empty()) return;
                        task = std::move(tasks_.front());
                        tasks_.pop();
                    }
                    task();
                }
            });
        }
    }

    ~SimpleThreadPool() {
        stop_.store(true);
        cv_.notify_all();
        for (auto& w : workers_) {
            if (w.joinable()) w.join();
        }
    }

    template<typename F>
    auto submit(F&& f) -> std::future<decltype(f())> {
        using ReturnType = decltype(f());
        auto task = std::make_shared<std::packaged_task<ReturnType()>>(std::forward<F>(f));
        auto future = task->get_future();
        {
            std::lock_guard<std::mutex> lock(mutex_);
            tasks_.emplace([task]() { (*task)(); });
        }
        cv_.notify_one();
        return future;
    }
};

void demo_executor_concept() {
    std::cout << "\n=== 执行器(Executor)概念 ===\n";

    std::cout << "执行器: 决定任务在哪里执行\n\n";

    std::cout << "1. 线程池执行器:\n";
    std::cout << "   任务提交到线程池, 由池中线程执行\n";
    std::cout << "   避免频繁创建销毁线程\n";

    std::cout << "\n2. 内联执行器:\n";
    std::cout << "   任务在提交线程上直接执行\n";
    std::cout << "   适合测试或轻量任务\n";

    std::cout << "\n3. 调度执行器:\n";
    std::cout << "   任务延迟执行或周期执行\n";
    std::cout << "   适合定时任务\n";

    std::cout << "\n简单线程池实现:\n";

    SimpleThreadPool pool(4);
    std::vector<std::future<int>> results;
    for (int i = 0; i < 8; ++i) {
        results.push_back(pool.submit([i]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            return i * i;
        }));
    }

    std::cout << "线程池结果: ";
    for (auto& r : results) std::cout << r.get() << " ";
    std::cout << "\n";
}

void demo_async_vs_thread() {
    std::cout << "\n=== async vs thread ===\n";

    std::cout << "std::async:\n";
    std::cout << "  优点: 返回future, 方便获取结果\n";
    std::cout << "  优点: 自动异常传播\n";
    std::cout << "  优点: 可以选择延迟执行\n";
    std::cout << "  缺点: 启动策略不确定(默认)\n";
    std::cout << "  缺点: future只能get一次\n";

    std::cout << "\nstd::thread:\n";
    std::cout << "  优点: 完全控制线程生命周期\n";
    std::cout << "  优点: 可以detach\n";
    std::cout << "  缺点: 需要手动管理结果\n";
    std::cout << "  缺点: 异常不会传播(terminate)\n";

    std::cout << "\nstd::jthread (C++20):\n";
    std::cout << "  优点: 自动join, 支持停止请求\n";
    std::cout << "  优点: 比thread更安全\n";
    std::cout << "  缺点: 同样需要手动管理结果\n";

    std::cout << "\n选择建议:\n";
    std::cout << "  需要结果: async + future\n";
    std::cout << "  需要控制: thread / jthread\n";
    std::cout << "  后台任务: jthread + detach\n";
    std::cout << "  线程池: packaged_task + 线程池\n";
    std::cout << "  异步I/O: 协程 + executor\n";
}

int main() {
    std::cout << "========== 异步编程模式深入探讨 ==========\n";

    demo_async_patterns();
    demo_when_all_pattern();
    demo_when_any_pattern();
    demo_continuation_pattern();
    demo_executor_concept();
    demo_async_vs_thread();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
