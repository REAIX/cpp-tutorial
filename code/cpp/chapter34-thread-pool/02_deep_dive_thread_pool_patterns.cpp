/**
 * @file 02_deep_dive_thread_pool_patterns.cpp
 * @brief 线程池设计模式, 线程数选择, 任务调度, 协程集成概念
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

void demo_thread_pool_sizing() {
    std::cout << "\n=== demo_thread_pool_sizing ===\n";
    std::cout << "线程池大小选择\n\n";

    unsigned int hw_threads = std::thread::hardware_concurrency();
    std::cout << "硬件并发线程数: " << hw_threads << "\n\n";

    std::cout << "CPU密集型任务:\n";
    std::cout << "  线程数 = CPU核心数 (+1可能更好)\n";
    std::cout << "  过多线程会增加上下文切换开销\n";
    std::cout << "  推荐: " << hw_threads << " 个线程\n\n";

    std::cout << "I/O密集型任务:\n";
    std::cout << "  线程数 = CPU核心数 * (1 + 等待时间/计算时间)\n";
    std::cout << "  例: 等待/计算=2, 则 " << hw_threads << " * 3 = " << hw_threads * 3 << "\n";
    std::cout << "  过少线程浪费CPU等待时间\n\n";

    std::cout << "混合型任务:\n";
    std::cout << "  根据CPU和I/O比例调整\n";
    std::cout << "  或使用两个线程池: CPU池 + I/O池\n\n";

    std::cout << "动态调整:\n";
    std::cout << "  根据队列长度和CPU使用率动态增减线程\n";
    std::cout << "  C++实现: 监控指标, 定时调整\n";
    std::cout << "  注意: 线程创建/销毁有开销, 不宜频繁调整\n";

    auto benchmark_size = [](size_t num_threads, size_t tasks) {
        std::vector<std::thread> threads;
        std::atomic<size_t> counter{0};
        auto start = std::chrono::high_resolution_clock::now();

        for (size_t i = 0; i < num_threads; ++i) {
            threads.emplace_back([&counter, tasks, num_threads]() {
                size_t per_thread = tasks / num_threads;
                for (size_t j = 0; j < per_thread; ++j) {
                    volatile int sum = 0;
                    for (int k = 0; k < 100; ++k) {
                        sum += k;
                    }
                    counter.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }
        for (auto& t : threads) t.join();

        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    };

    std::cout << "\nCPU密集型基准测试 (10000个任务):\n";
    for (size_t n = 1; n <= hw_threads * 2; n *= 2) {
        auto ms = benchmark_size(n, 10000);
        std::cout << "  " << n << "线程: " << ms << " ms\n";
    }
}

void demo_task_scheduling() {
    std::cout << "\n=== demo_task_scheduling ===\n";
    std::cout << "任务调度策略\n\n";

    std::cout << "1. FIFO (先进先出):\n";
    std::cout << "   最简单的调度策略\n";
    std::cout << "   公平, 但不考虑任务优先级\n\n";

    std::cout << "2. 优先级调度:\n";
    std::cout << "   高优先级任务先执行\n";
    std::cout << "   可能导致低优先级饥饿\n";
    std::cout << "   解决: 老化(aging) - 等待时间增加优先级\n\n";

    std::cout << "3. 短任务优先:\n";
    std::cout << "   估计任务执行时间, 短任务先执行\n";
    std::cout << "   提高平均响应时间\n";
    std::cout << "   难点: 如何估计执行时间\n\n";

    std::cout << "4. 时间片轮转:\n";
    std::cout << "   每个任务分配时间片\n";
    std::cout << "   超时则放回队列\n";
    std::cout << "   适合长时间运行的任务\n\n";

    std::cout << "5. 分组调度:\n";
    std::cout << "   将相关任务分到同一组\n";
    std::cout << "   组内串行, 组间并行\n";
    std::cout << "   适合有依赖关系的任务\n";

    std::cout << "\n调度策略选择:\n";
    std::cout << "  通用场景: FIFO + 优先级\n";
    std::cout << "  实时场景: 优先级 + 老化\n";
    std::cout << "  交互场景: 短任务优先\n";
    std::cout << "  批处理场景: FIFO\n";
}

void demo_thread_pool_patterns() {
    std::cout << "\n=== demo_thread_pool_patterns ===\n";
    std::cout << "线程池设计模式\n\n";

    std::cout << "模式1: 固定大小线程池\n";
    std::cout << "  线程数固定, 任务队列无界\n";
    std::cout << "  简单可靠, 但可能内存增长\n";
    std::cout << "  Java Executors.newFixedThreadPool\n\n";

    std::cout << "模式2: 缓存线程池\n";
    std::cout << "  线程数动态, 空闲线程超时回收\n";
    std::cout << "  适合大量短任务\n";
    std::cout << "  风险: 可能创建过多线程\n\n";

    std::cout << "模式3: 调度线程池\n";
    std::cout << "  支持延迟执行和定时执行\n";
    std::cout << "  内部使用时间轮或优先队列\n";
    std::cout << "  Java ScheduledThreadPoolExecutor\n\n";

    std::cout << "模式4: Fork-Join池\n";
    std::cout << "  专门用于分治任务\n";
    std::cout << "  Work-stealing调度\n";
    std::cout << "  Intel TBB, Java ForkJoinPool\n\n";

    std::cout << "模式5: I/O线程池\n";
    std::cout << "  配合epoll/io_uring使用\n";
    std::cout << "  线程数通常远大于CPU核心数\n";
    std::cout << "  Netty, Boost.Asio\n";

    std::cout << "\nC++线程池设计建议:\n";
    std::cout << "  1. 支持不同任务类型(普通/定时/延迟)\n";
    std::cout << "  2. 支持future获取结果\n";
    std::cout << "  3. 支持优雅关闭\n";
    std::cout << "  4. 支持任务取消\n";
    std::cout << "  5. 提供监控指标(队列长度/活跃线程数)\n";
    std::cout << "  6. 考虑异常安全\n";
}

void demo_coroutine_integration_concept() {
    std::cout << "\n=== demo_coroutine_integration_concept ===\n";
    std::cout << "线程池与协程集成概念\n\n";

    std::cout << "C++20协程基础:\n";
    std::cout << "  co_await: 挂起协程, 等待异步操作\n";
    std::cout << "  co_return: 协程返回值\n";
    std::cout << "  co_yield: 产生一个值, 挂起协程\n\n";

    std::cout << "线程池 + 协程的集成方式:\n\n";

    std::cout << "1. 协程调度器:\n";
    std::cout << "   线程池作为协程的执行器(Executor)\n";
    std::cout << "   co_await时, 将协程提交到线程池\n";
    std::cout << "   协程恢复时, 在线程池的工作线程上执行\n\n";

    std::cout << "2. 异步任务模型:\n";
    std::cout << "   Task<int> compute() {\n";
    std::cout << "     int result = co_await pool.schedule();\n";
    std::cout << "     co_return result * 2;\n";
    std::cout << "   }\n\n";

    std::cout << "3. I/O集成:\n";
    std::cout << "   协程 + epoll/io_uring\n";
    std::cout << "   I/O等待时不占用线程\n";
    std::cout << "   I/O完成后恢复协程\n";
    std::cout << "   代表: Boost.Asio + C++20协程\n\n";

    std::cout << "4. 结构化并发:\n";
    std::cout << "   协程提供结构化的并发模型\n";
    std::cout << "   父协程等待所有子协程完成\n";
    std::cout << "   异常自动传播\n\n";

    std::cout << "C++23/26展望:\n";
    std::cout << "  std::execution (Sender/Receiver): 标准化异步执行\n";
    std::cout << "  std::static_thread_pool: 标准线程池\n";
    std::cout << "  目前: 使用第三方库(Boost.Asio, folly, cppcoro)\n";

    std::cout << "\n实用建议:\n";
    std::cout << "  1. 短期: 使用传统线程池 + future\n";
    std::cout << "  2. 中期: 引入Boost.Asio + 协程\n";
    std::cout << "  3. 长期: 等待C++26标准化\n";
    std::cout << "  4. 不建议: 自己实现协程调度器\n";
}

void demo_error_handling() {
    std::cout << "\n=== demo_error_handling ===\n";
    std::cout << "线程池中的错误处理\n\n";

    std::cout << "1. 任务异常:\n";
    std::cout << "   任务抛出异常不应影响其他任务\n";
    std::cout << "   异常通过future传播给调用者\n\n";

    std::cout << "2. 工作线程崩溃:\n";
    std::cout << "   捕获所有异常, 防止线程退出\n";
    std::cout << "   记录异常日志\n\n";

    std::cout << "3. 队列满:\n";
    std::cout << "   有界队列: 拒绝策略(丢弃/调用者执行/阻塞)\n";
    std::cout << "   无界队列: 注意内存增长\n\n";

    std::cout << "4. 关闭时:\n";
    std::cout << "   等待进行中的任务完成\n";
    std::cout << "   丢弃队列中的未执行任务\n";
    std::cout << "   通知调用者关闭状态\n";

    class SafeThreadPool {
        std::vector<std::thread> workers_;
        std::queue<std::function<void()>> tasks_;
        std::mutex mutex_;
        std::condition_variable cv_;
        std::atomic<bool> stop_{false};
        std::atomic<size_t> error_count_{0};

    public:
        explicit SafeThreadPool(size_t n) {
            for (size_t i = 0; i < n; ++i) {
                workers_.emplace_back([this]() {
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(mutex_);
                            cv_.wait(lock, [this]() {
                                return stop_ || !tasks_.empty();
                            });
                            if (stop_ && tasks_.empty()) return;
                            task = std::move(tasks_.front());
                            tasks_.pop();
                        }
                        try {
                            task();
                        } catch (const std::exception& e) {
                            std::cerr << "  任务异常: " << e.what() << "\n";
                            error_count_.fetch_add(1);
                        } catch (...) {
                            std::cerr << "  任务未知异常\n";
                            error_count_.fetch_add(1);
                        }
                    }
                });
            }
        }

        ~SafeThreadPool() {
            stop_ = true;
            cv_.notify_all();
            for (auto& w : workers_) if (w.joinable()) w.join();
        }

        void submit(std::function<void()> task) {
            {
                std::lock_guard<std::mutex> lock(mutex_);
                tasks_.push(std::move(task));
            }
            cv_.notify_one();
        }

        size_t error_count() const { return error_count_.load(); }
    };

    SafeThreadPool pool(2);
    pool.submit([]() { std::cout << "  正常任务执行\n"; });
    pool.submit([]() { throw std::runtime_error("模拟异常"); });
    pool.submit([]() { std::cout << "  异常后的任务正常执行\n"; });

    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "  错误计数: " << pool.error_count() << "\n";
}

int main() {
    std::cout << "线程池设计模式深入\n";

    demo_thread_pool_sizing();
    demo_task_scheduling();
    demo_thread_pool_patterns();
    demo_coroutine_integration_concept();
    demo_error_handling();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
