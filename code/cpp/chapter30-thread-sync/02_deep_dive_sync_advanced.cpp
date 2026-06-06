/** @file 02_deep_dive_sync_advanced.cpp @brief 高级同步原语 @description 对应文档: 02-CPP/30-thread-sync
 *  编译命令: g++ -std=c++20 02_deep_dive_sync_advanced.cpp -o 02_deep_dive_sync_advanced
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <semaphore>
#include <latch>
#include <barrier>
#include <vector>
#include <chrono>
#include <atomic>
#include <functional>

void demo_binary_semaphore() {
    std::cout << "\n=== 二值信号量 (C++20) ===\n";

    std::binary_semaphore sem{1};
    int counter = 0;

    auto worker = [&sem, &counter](int id) {
        sem.acquire();
        ++counter;
        std::cout << "  线程" << id << ": counter=" << counter << "\n";
        sem.release();
    };

    std::vector<std::jthread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(worker, i);
    }

    std::cout << "binary_semaphore vs mutex:\n";
    std::cout << "  相似: 都可以用于互斥\n";
    std::cout << "  不同: 信号量的release不需要由acquire的同一线程调用\n";
    std::cout << "  信号量更灵活, 但mutex通常更高效\n";
}

void demo_counting_semaphore() {
    std::cout << "\n=== 计数信号量 (C++20) ===\n";

    std::counting_semaphore<3> sem{3};

    auto worker = [&sem](int id) {
        sem.acquire();
        std::cout << "  线程" << id << "进入(资源占用)\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        std::cout << "  线程" << id << "离开(资源释放)\n";
        sem.release();
    };

    std::vector<std::jthread> threads;
    for (int i = 0; i < 6; ++i) {
        threads.emplace_back(worker, i);
    }

    std::cout << "\n计数信号量用途:\n";
    std::cout << "  1. 限制并发数(连接池, 线程池)\n";
    std::cout << "  2. 资源池管理\n";
    std::cout << "  3. 生产者-消费者信号\n";
}

void demo_latch() {
    std::cout << "\n=== Latch (C++20) ===\n";

    std::latch done{3};

    auto worker = [&done](int id) {
        std::cout << "  工作者" << id << "完成\n";
        done.count_down();
    };

    std::vector<std::jthread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(worker, i);
    }

    done.wait();
    std::cout << "所有工作者完成, 主线程继续\n";

    std::cout << "\nlatch特点:\n";
    std::cout << "  1. 一次性: 计数器减到0后不能重置\n";
    std::cout << "  2. wait(): 阻塞直到计数器为0\n";
    std::cout << "  3. count_down(): 计数器减1\n";
    std::cout << "  4. arrive_and_wait(): 减1并等待\n";

    std::cout << "\n用途:\n";
    std::cout << "  等待N个任务全部完成\n";
    std::cout << "  初始化同步(所有线程就绪后开始)\n";
}

void demo_barrier() {
    std::cout << "\n=== Barrier (C++20) ===\n";

    auto on_completion = []() noexcept {
        std::cout << "  === 阶段完成 ===\n";
    };

    std::barrier sync_point{3, on_completion};

    auto worker = [&sync_point](int id) {
        for (int phase = 1; phase <= 3; ++phase) {
            std::cout << "  线程" << id << " 阶段" << phase << "工作\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(id * 10));
            sync_point.arrive_and_wait();
        }
    };

    std::vector<std::jthread> threads;
    for (int i = 1; i <= 3; ++i) {
        threads.emplace_back(worker, i);
    }

    std::cout << "\nbarrier特点:\n";
    std::cout << "  1. 可重用: 每次所有线程到达后自动重置\n";
    std::cout << "  2. 完成函数: 每个周期执行一次\n";
    std::cout << "  3. arrive_and_wait(): 到达并等待其他线程\n";
    std::cout << "  4. arrive_and_drop(): 到达后退出(减少参与者)\n";

    std::cout << "\n用途:\n";
    std::cout << "  分阶段并行计算\n";
    std::cout << "  迭代算法的同步点\n";
    std::cout << "  多线程流水线\n";
}

void demo_sync_comparison() {
    std::cout << "\n=== 同步原语对比 ===\n";

    std::cout << "互斥量家族:\n";
    std::cout << "  mutex:          基本互斥, 最常用\n";
    std::cout << "  timed_mutex:    支持超时的互斥\n";
    std::cout << "  recursive_mutex: 可重入互斥(同一线程可多次加锁)\n";
    std::cout << "  shared_mutex:   读写互斥(C++17)\n";

    std::cout << "\n锁包装器:\n";
    std::cout << "  lock_guard:     RAII, 最简单\n";
    std::cout << "  unique_lock:    RAII, 灵活(可手动控制)\n";
    std::cout << "  shared_lock:    RAII, 共享锁(读锁)\n";
    std::cout << "  scoped_lock:    RAII, 多锁(避免死锁)\n";

    std::cout << "\n同步原语(C++20):\n";
    std::cout << "  binary_semaphore:   二值信号量\n";
    std::cout << "  counting_semaphore: 计数信号量\n";
    std::cout << "  latch:              一次性同步点\n";
    std::cout << "  barrier:            可重用同步点\n";

    std::cout << "\n条件变量:\n";
    std::cout << "  condition_variable:   配合unique_lock使用\n";
    std::cout << "  condition_variable_any: 配合任意锁使用\n";
}

void demo_sync_performance() {
    std::cout << "\n=== 同步原语性能 ===\n";

    std::cout << "性能排序(从快到慢):\n";
    std::cout << "  1. atomic (无锁)\n";
    std::cout << "  2. spin lock (忙等待, 短临界区)\n";
    std::cout << "  3. mutex (内核对象, 长临界区)\n";
    std::cout << "  4. semaphore (比mutex略慢)\n";
    std::cout << "  5. condition_variable (最灵活但最慢)\n";

    std::cout << "\n选择建议:\n";
    std::cout << "  简单计数/标志: atomic\n";
    std::cout << "  互斥访问: mutex + lock_guard\n";
    std::cout << "  读多写少: shared_mutex\n";
    std::cout << "  等待条件: condition_variable\n";
    std::cout << "  限制并发数: counting_semaphore\n";
    std::cout << "  一次性同步: latch\n";
    std::cout << "  分阶段同步: barrier\n";
    std::cout << "  多锁操作: scoped_lock\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  锁的粒度: 粗粒度(简单但并发低) vs 细粒度(复杂但并发高)\n";
    std::cout << "  锁的持有时间: 越短越好\n";
    std::cout << "  优先使用标准库原语, 避免自己实现\n";
}

int main() {
    std::cout << "========== 高级同步原语 ==========\n";

    demo_binary_semaphore();
    demo_counting_semaphore();
    demo_latch();
    demo_barrier();
    demo_sync_comparison();
    demo_sync_performance();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
