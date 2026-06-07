/**
 * @file 01_deep_dive_thread_pool_advanced.cpp
 * @brief 高级线程池: work stealing, 优先队列, future, 优雅关闭, 负载均衡
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
#include <random>
#include <chrono>

class AdvancedThreadPool {
    struct PrioritizedTask {
        int priority;
        std::function<void()> task;
        bool operator<(const PrioritizedTask& other) const {
            return priority < other.priority;
        }
    };

    std::vector<std::thread> workers_;
    std::priority_queue<PrioritizedTask> task_queue_;
    mutable std::mutex queue_mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stop_{false};
    std::atomic<size_t> active_tasks_{0};
    std::atomic<size_t> total_completed_{0};

public:
    explicit AdvancedThreadPool(size_t num_threads = std::thread::hardware_concurrency()) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this]() { worker_loop(); });
        }
        std::cout << "高级线程池创建: " << num_threads << "个工作线程\n";
    }

    ~AdvancedThreadPool() {
        shutdown();
    }

    void submit(std::function<void()> task, int priority = 0) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (stop_.load()) return;
            task_queue_.push({priority, std::move(task)});
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
            if (stop_.load()) return result;
            task_queue_.push({0, [task_ptr]() { (*task_ptr)(); }});
        }
        cv_.notify_one();
        return result;
    }

    void shutdown() {
        if (stop_.load()) return;
        stop_.store(true);
        cv_.notify_all();
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        std::cout << "线程池已关闭, 完成任务总数: " << total_completed_.load() << "\n";
    }

    void wait_all() {
        while (true) {
            std::unique_lock<std::mutex> lock(queue_mutex_);
            if (task_queue_.empty() && active_tasks_.load() == 0) break;
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    size_t active_count() const { return active_tasks_.load(); }
    size_t completed_count() const { return total_completed_.load(); }
    size_t pending_count() const {
        std::lock_guard<std::mutex> lock(queue_mutex_);
        return task_queue_.size();
    }

private:
    void worker_loop() {
        while (true) {
            std::function<void()> task;
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                cv_.wait(lock, [this]() {
                    return stop_.load() || !task_queue_.empty();
                });
                if (stop_.load() && task_queue_.empty()) return;
                task = std::move(task_queue_.top().task);
                task_queue_.pop();
                active_tasks_.fetch_add(1);
            }
            task();
            active_tasks_.fetch_sub(1);
            total_completed_.fetch_add(1);
        }
    }
};

void demo_priority_queue() {
    std::cout << "\n=== demo_priority_queue ===\n";
    std::cout << "优先级任务队列\n\n";

    AdvancedThreadPool pool(2);

    std::atomic<int> execution_order{0};
    std::mutex print_mutex;

    auto make_task = [&](int priority, const std::string& name) {
        return [&, priority, name]() {
            int order = execution_order.fetch_add(1);
            std::lock_guard<std::mutex> lock(print_mutex);
            std::cout << "  执行: " << name << " (优先级=" << priority
                      << ", 执行顺序=" << order << ")\n";
        };
    };

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    pool.submit(make_task(1, "低优先级任务A"), 1);
    pool.submit(make_task(1, "低优先级任务B"), 1);
    pool.submit(make_task(10, "高优先级任务C"), 10);
    pool.submit(make_task(5, "中优先级任务D"), 5);
    pool.submit(make_task(10, "高优先级任务E"), 10);

    pool.wait_all();
    std::cout << "  优先级高的任务先执行\n";
}

void demo_graceful_shutdown() {
    std::cout << "\n=== demo_graceful_shutdown ===\n";
    std::cout << "优雅关闭: 等待所有任务完成\n\n";

    {
        AdvancedThreadPool pool(2);

        for (int i = 0; i < 5; ++i) {
            pool.submit([i]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                std::cout << "  任务" << i << "完成\n";
            });
        }

        std::cout << "  等待所有任务完成...\n";
        pool.wait_all();
        std::cout << "  所有任务完成, 活跃数=" << pool.active_count()
                  << ", 完成数=" << pool.completed_count() << "\n";
    }

    std::cout << "\n优雅关闭要点:\n";
    std::cout << "  1. 设置stop标志\n";
    std::cout << "  2. 通知所有等待的线程\n";
    std::cout << "  3. 等待所有任务完成\n";
    std::cout << "  4. join所有工作线程\n";
    std::cout << "  5. 析构函数中自动关闭\n";
}

void demo_work_stealing_concept() {
    std::cout << "\n=== demo_work_stealing_concept ===\n";
    std::cout << "Work Stealing (工作窃取) 概念\n\n";

    std::cout << "传统线程池问题:\n";
    std::cout << "  全局任务队列是瓶颈 (锁竞争)\n";
    std::cout << "  某些线程可能空闲, 而其他线程积压\n\n";

    std::cout << "Work Stealing方案:\n";
    std::cout << "  每个工作线程有自己的本地队列(双端队列)\n";
    std::cout << "  线程从自己队列的头部取任务(LIFO)\n";
    std::cout << "  空闲线程从其他线程队列的尾部窃取(FIFO)\n\n";

    std::cout << "Work Stealing优势:\n";
    std::cout << "  1. 减少全局锁竞争\n";
    std::cout << "  2. 更好的缓存局部性\n";
    std::cout << "  3. 自动负载均衡\n";
    std::cout << "  4. 适合分治任务(如并行排序)\n\n";

    std::cout << "C++实现要点:\n";
    std::cout << "  1. 每线程一个deque (需线程安全)\n";
    std::cout << "  2. 本地操作用lock-free deque\n";
    std::cout << "  3. 窃取操作用CAS或mutex\n";
    std::cout << "  4. 随机选择被窃取线程\n";
    std::cout << "  5. 代表实现: Intel TBB, Folly\n";

    std::cout << "\n简化Work Stealing演示:\n";
    constexpr int num_workers = 4;
    std::vector<std::deque<std::function<void()>>> local_queues(num_workers);
    std::vector<std::mutex> local_mutexes(num_workers);
    std::atomic<bool> done{false};

    auto try_steal = [&](int thief_id) -> std::function<void()> {
        std::mt19937 rng(thief_id);
        for (int attempt = 0; attempt < 3; ++attempt) {
            int victim = rng() % num_workers;
            if (victim == thief_id) continue;
            std::lock_guard<std::mutex> lock(local_mutexes[victim]);
            if (!local_queues[victim].empty()) {
                auto task = std::move(local_queues[victim].front());
                local_queues[victim].pop_front();
                std::cout << "  线程" << thief_id << "从线程" << victim << "窃取任务\n";
                return task;
            }
        }
        return nullptr;
    };

    for (int i = 0; i < num_workers; ++i) {
        local_queues[i].push_back([i]() {
            std::cout << "  线程" << i << "执行自己的任务\n";
        });
    }

    for (int i = 0; i < num_workers; ++i) {
        std::lock_guard<std::mutex> lock(local_mutexes[i]);
        if (!local_queues[i].empty()) {
            auto task = std::move(local_queues[i].back());
            local_queues[i].pop_back();
            task();
        }
    }

    for (int i = 0; i < num_workers; ++i) {
        auto stolen = try_steal(i);
        if (stolen) stolen();
    }
}

void demo_load_balancing() {
    std::cout << "\n=== demo_load_balancing ===\n";
    std::cout << "负载均衡策略\n\n";

    std::cout << "策略1: 随机分配\n";
    std::cout << "  最简单, 但可能不均匀\n";
    std::cout << "  适合任务大小相近的场景\n\n";

    std::cout << "策略2: 轮询(Round-Robin)\n";
    std::cout << "  依次分配给每个线程\n";
    std::cout << "  适合任务大小相近的场景\n\n";

    std::cout << "策略3: 最少任务优先\n";
    std::cout << "  分配给当前任务最少的线程\n";
    std::cout << "  需要跟踪每个线程的任务数\n\n";

    std::cout << "策略4: Work Stealing\n";
    std::cout << "  空闲线程主动窃取任务\n";
    std::cout << "  自动均衡, 无需中心调度\n\n";

    std::cout << "策略5: 任务窃取 + 优先级\n";
    std::cout << "  结合优先级和窃取\n";
    std::cout << "  高优先级任务优先执行\n\n";

    AdvancedThreadPool pool(4);

    std::atomic<int> task_counter{0};
    constexpr int total_tasks = 20;

    for (int i = 0; i < total_tasks; ++i) {
        pool.submit([&task_counter, i]() {
            int duration = (i % 3 + 1) * 10;
            std::this_thread::sleep_for(std::chrono::milliseconds(duration));
            task_counter.fetch_add(1);
        });
    }

    pool.wait_all();
    std::cout << "  完成" << task_counter.load() << "/" << total_tasks << "个任务\n";
    std::cout << "  活跃线程=" << pool.active_count() << "\n";
}

int main() {
    std::cout << "高级线程池特性演示\n";

    demo_priority_queue();
    demo_graceful_shutdown();
    demo_work_stealing_concept();
    demo_load_balancing();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
