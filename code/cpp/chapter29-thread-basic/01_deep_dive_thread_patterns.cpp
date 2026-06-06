/** @file 01_deep_dive_thread_patterns.cpp @brief 线程管理模式深入探讨 @description 对应文档: 02-CPP/29-thread-basic
 *  @note C 语言中使用 pthread API 实现类似功能, 参见 C 章节 24-进程与线程
 *  编译命令: g++ -std=c++20 01_deep_dive_thread_patterns.cpp -o 01_deep_dive_thread_patterns
 */

#include <iostream>
#include <thread>
#include <vector>
#include <memory>
#include <chrono>
#include <functional>
#include <sstream>

class thread_guard {
    std::thread t_;
public:
    explicit thread_guard(std::thread t) : t_(std::move(t)) {
        if (!t_.joinable()) {
            throw std::logic_error("线程不可join");
        }
    }

    ~thread_guard() {
        if (t_.joinable()) {
            t_.join();
        }
    }

    thread_guard(const thread_guard&) = delete;
    thread_guard& operator=(const thread_guard&) = delete;
    thread_guard(thread_guard&&) = default;
    thread_guard& operator=(thread_guard&&) = default;
};

class scoped_thread {
    std::thread t_;
public:
    explicit scoped_thread(std::thread t) : t_(std::move(t)) {
        if (!t_.joinable()) {
            throw std::logic_error("线程不可join");
        }
    }

    ~scoped_thread() {
        t_.join();
    }

    scoped_thread(const scoped_thread&) = delete;
    scoped_thread& operator=(const scoped_thread&) = delete;
};

class joining_thread {
    std::thread t_;
public:
    joining_thread() = default;
    explicit joining_thread(std::thread t) : t_(std::move(t)) {}

    template<typename Callable, typename... Args>
    explicit joining_thread(Callable&& func, Args&&... args)
        : t_(std::forward<Callable>(func), std::forward<Args>(args)...) {}

    ~joining_thread() {
        if (t_.joinable()) t_.join();
    }

    joining_thread(joining_thread&& other) noexcept : t_(std::move(other.t_)) {}
    joining_thread& operator=(joining_thread&& other) noexcept {
        if (t_.joinable()) t_.join();
        t_ = std::move(other.t_);
        return *this;
    }

    void swap(joining_thread& other) noexcept { t_.swap(other.t_); }
    bool joinable() const noexcept { return t_.joinable(); }
    void join() { t_.join(); }
    void detach() { t_.detach(); }
    std::thread::id get_id() const noexcept { return t_.get_id(); }
};

void demo_thread_guard_raii() {
    std::cout << "\n=== 线程守卫RAII ===\n";

    {
        int result = 0;
        thread_guard g(std::thread([&result]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            result = 42;
        }));
        std::cout << "thread_guard保护中...\n";
    }
    std::cout << "离开作用域, thread_guard析构自动join\n";

    {
        scoped_thread st(std::thread([]() {
            std::cout << "  scoped_thread运行中\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }));
        std::cout << "scoped_thread保护中...\n";
    }
    std::cout << "scoped_thread析构自动join\n";

    {
        joining_thread jt([]() {
            std::cout << "  joining_thread运行中\n";
        });
    }
    std::cout << "joining_thread析构自动join\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  C++20的std::jthread就是标准版的joining_thread\n";
    std::cout << "  析构时自动join, 避免忘记join导致terminate\n";
}

void demo_thread_local_storage() {
    std::cout << "\n=== 线程局部存储(thread_local) ===\n";

    thread_local int tls_counter = 0;

    auto worker = [](int id) {
        ++tls_counter;
        std::cout << "  线程" << id << ": tls_counter=" << tls_counter
                  << " (地址=" << &tls_counter << ")\n";
        ++tls_counter;
        std::cout << "  线程" << id << ": tls_counter=" << tls_counter << "\n";
    };

    std::thread t1(worker, 1);
    std::thread t2(worker, 2);
    std::thread t3(worker, 3);

    t1.join();
    t2.join();
    t3.join();

    std::cout << "\nthread_local特点:\n";
    std::cout << "  1. 每个线程有独立副本\n";
    std::cout << "  2. 线程启动时初始化, 线程结束时销毁\n";
    std::cout << "  3. 不同线程的地址不同\n";
    std::cout << "  4. 可用于避免锁竞争(每个线程操作自己的数据)\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  日志系统: 每个线程有自己的日志缓冲区\n";
    std::cout << "  随机数: 每个线程有自己的随机数引擎\n";
    std::cout << "  内存池: 每个线程有自己的内存池(避免锁)\n";
}

void demo_hardware_concurrency() {
    std::cout << "\n=== hardware_concurrency ===\n";

    unsigned int cores = std::thread::hardware_concurrency();
    std::cout << "硬件并发数: " << cores << "\n";
    std::cout << "(返回0表示无法检测)\n";

    std::cout << "\n用途:\n";
    std::cout << "  1. 确定线程池大小\n";
    std::cout << "  2. 并行算法的分区数\n";
    std::cout << "  3. 避免过度订阅(oversubscription)\n";

    std::cout << "\n建议线程数:\n";
    std::cout << "  CPU密集型: cores 或 cores+1\n";
    std::cout << "  I/O密集型: cores * 2 或更多\n";
    std::cout << "  混合型: 根据实际测试调整\n";

    if (cores > 0) {
        std::cout << "\n使用" << cores << "个线程并行:\n";
        std::vector<std::thread> threads;
        for (unsigned int i = 0; i < cores; ++i) {
            threads.emplace_back([i]() {
                std::cout << "  工作线程" << i << "\n";
            });
        }
        for (auto& t : threads) t.join();
    }
}

void demo_thread_management_patterns() {
    std::cout << "\n=== 线程管理模式 ===\n";

    std::cout << "1. 等待所有完成(Barrier模式):\n";
    {
        std::vector<std::jthread> threads;
        for (int i = 0; i < 3; ++i) {
            threads.emplace_back([i]() {
                std::cout << "  任务" << i << "完成\n";
            });
        }
    }
    std::cout << "  所有jthread析构(join)\n";

    std::cout << "\n2. 分散-收集(Scatter-Gather):\n";
    {
        std::vector<int> results(4, 0);
        std::vector<std::jthread> threads;
        for (int i = 0; i < 4; ++i) {
            threads.emplace_back([&results, i]() {
                results[i] = i * i;
            });
        }
        threads.clear();
        int total = 0;
        for (int r : results) total += r;
        std::cout << "  结果总和: " << total << "\n";
    }

    std::cout << "\n3. 生产者-消费者(后续章节详解)\n";

    std::cout << "\n4. 线程池(后续章节详解)\n";
}

void demo_thread_yield() {
    std::cout << "\n=== 线程让步 ===\n";

    std::cout << "std::this_thread::yield():\n";
    std::cout << "  提示调度器可以切换到其他线程\n";
    std::cout << "  适合忙等待中的优化\n";

    std::cout << "\nstd::this_thread::sleep_for() / sleep_until():\n";
    std::cout << "  主动让出CPU一段时间\n";
    std::cout << "  比yield更确定, 适合定时任务\n";

    std::cout << "\n忙等待 vs sleep:\n";
    std::cout << "  忙等待: while(!ready) {} -> 浪费CPU\n";
    std::cout << "  yield:  while(!ready) { yield(); } -> 稍好\n";
    std::cout << "  sleep:  while(!ready) { sleep(1ms); } -> 更好\n";
    std::cout << "  条件变量: cv.wait(lock, pred) -> 最佳\n";
}

int main() {
    std::cout << "========== 线程管理模式深入探讨 ==========\n";

    demo_thread_guard_raii();
    demo_thread_local_storage();
    demo_hardware_concurrency();
    demo_thread_management_patterns();
    demo_thread_yield();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
