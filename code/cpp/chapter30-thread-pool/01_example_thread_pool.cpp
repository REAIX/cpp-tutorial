/**
 * @file 01_example_thread_pool.cpp
 * @brief 基本线程池实现: 任务队列, 工作线程, 提交任务
 * @description 对应文档: 02-CPP/34-线程池实现
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>
#include <future>
#include <vector>
#include <atomic>
#include <chrono>

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads = std::thread::hardware_concurrency())
        : stop_(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this]() {
                worker_loop();
            });
        }
        std::cout << "线程池创建: " << num_threads << "个工作线程\n";
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            stop_ = true;
        }
        cv_.notify_all();
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        std::cout << "线程池销毁\n";
    }

    void submit(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (stop_) return;
            tasks_.push(std::move(task));
        }
        cv_.notify_one();
    }

    template<typename F, typename... Args>
    auto submit_with_future(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
        using ReturnType = std::invoke_result_t<F, Args...>;
        auto task_ptr = std::make_shared<std::packaged_task<ReturnType()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        std::future<ReturnType> result = task_ptr->get_future();
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (stop_) return result;
            tasks_.push([task_ptr]() { (*task_ptr)(); });
        }
        cv_.notify_one();
        return result;
    }

    size_t pending_tasks() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return tasks_.size();
    }

private:
    void worker_loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                cv_.wait(lock, [this]() {
                    return stop_ || !tasks_.empty();
                });
                if (stop_ && tasks_.empty()) return;
                task = std::move(tasks_.front());
                tasks_.pop();
            }
            task();
        }
    }

    std::vector<std::thread> workers_;
    std::queue<std::function<void()>> tasks_;
    mutable std::mutex queue_mutex_;
    std::condition_variable cv_;
    bool stop_;
};

void demo_basic_thread_pool() {
    std::cout << "\n=== demo_basic_thread_pool ===\n";
    std::cout << "基本线程池: 提交任务, 工作线程执行\n\n";

    ThreadPool pool(4);

    for (int i = 0; i < 10; ++i) {
        pool.submit([i]() {
            std::cout << "  任务" << i << "在线程"
                      << std::this_thread::get_id() << "上执行\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(1500));
    std::cout << "  剩余任务: " << pool.pending_tasks() << "\n";
}

void demo_future_results() {
    std::cout << "\n=== demo_future_results ===\n";
    std::cout << "使用future获取任务结果\n\n";

    ThreadPool pool(4);

    auto add = [](int a, int b) -> int {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        return a + b;
    };

    std::vector<std::future<int>> results;
    for (int i = 0; i < 5; ++i) {
        results.push_back(pool.submit_with_future(add, i * 10, i * 20));
    }

    std::cout << "任务结果:\n";
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "  任务" << i << ": " << results[i].get() << "\n";
    }

    auto compute_string = [](int n) -> std::string {
        return "结果=" + std::to_string(n * n);
    };

    auto str_result = pool.submit_with_future(compute_string, 7);
    std::cout << "  字符串结果: " << str_result.get() << "\n";
}

void demo_parallel_computation() {
    std::cout << "\n=== demo_parallel_computation ===\n";
    std::cout << "并行计算示例: 数组求和\n\n";

    constexpr size_t data_size = 1000000;
    std::vector<int> data(data_size);
    for (size_t i = 0; i < data_size; ++i) {
        data[i] = static_cast<int>(i);
    }

    auto sequential_sum = [&]() -> long long {
        long long sum = 0;
        for (size_t i = 0; i < data_size; ++i) {
            sum += data[i];
        }
        return sum;
    };

    auto parallel_sum = [&](ThreadPool& pool, size_t num_parts) -> long long {
        std::vector<std::future<long long>> futures;
        size_t part_size = data_size / num_parts;

        for (size_t i = 0; i < num_parts; ++i) {
            size_t start = i * part_size;
            size_t end = (i == num_parts - 1) ? data_size : start + part_size;
            futures.push_back(pool.submit_with_future([&data, start, end]() -> long long {
                long long partial = 0;
                for (size_t j = start; j < end; ++j) {
                    partial += data[j];
                }
                return partial;
            }));
        }

        long long total = 0;
        for (auto& f : futures) {
            total += f.get();
        }
        return total;
    };

    ThreadPool pool(4);

    auto start = std::chrono::high_resolution_clock::now();
    long long seq_result = sequential_sum();
    auto end = std::chrono::high_resolution_clock::now();
    auto seq_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    start = std::chrono::high_resolution_clock::now();
    long long par_result = parallel_sum(pool, 4);
    end = std::chrono::high_resolution_clock::now();
    auto par_ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "  顺序求和: " << seq_result << ", 耗时: " << seq_ms << " us\n";
    std::cout << "  并行求和: " << par_result << ", 耗时: " << par_ms << " us\n";
    std::cout << "  结果一致: " << std::boolalpha << (seq_result == par_result) << "\n";
}

void demo_task_chaining() {
    std::cout << "\n=== demo_task_chaining ===\n";
    std::cout << "任务链: 前一任务的结果作为后一任务的输入\n\n";

    ThreadPool pool(2);

    auto multiply = [](int x) -> int { return x * 2; };
    auto add_ten = [](int x) -> int { return x + 10; };
    auto to_string = [](int x) -> std::string { return "最终结果: " + std::to_string(x); };

    auto f1 = pool.submit_with_future(multiply, 5);
    int r1 = f1.get();
    std::cout << "  步骤1 (x*2): " << r1 << "\n";

    auto f2 = pool.submit_with_future(add_ten, r1);
    int r2 = f2.get();
    std::cout << "  步骤2 (x+10): " << r2 << "\n";

    auto f3 = pool.submit_with_future(to_string, r2);
    std::string r3 = f3.get();
    std::cout << "  步骤3 (转字符串): " << r3 << "\n";

    std::cout << "\n线程池基本组成:\n";
    std::cout << "  1. 任务队列: 存储待执行的任务\n";
    std::cout << "  2. 工作线程: 从队列取任务执行\n";
    std::cout << "  3. 同步机制: mutex + condition_variable\n";
    std::cout << "  4. 提交接口: submit() 和 submit_with_future()\n";
}

int main() {
    std::cout << "线程池基本实现演示\n";

    demo_basic_thread_pool();
    demo_future_results();
    demo_parallel_computation();
    demo_task_chaining();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
