/**
 * @file 03_example_cas_patterns.cpp
 * @brief CAS模式: 自旋锁, 原子标志, CAS循环
 * @description 对应文档: 02-CPP/33-无锁编程
 */

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

class Spinlock {
    std::atomic_flag flag_{ATOMIC_FLAG_INIT};

public:
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {}
    }

    void unlock() {
        flag_.clear(std::memory_order_release);
    }

    bool try_lock() {
        return !flag_.test_and_set(std::memory_order_acquire);
    }
};

void demo_spinlock() {
    std::cout << "\n=== demo_spinlock ===\n";
    std::cout << "自旋锁(Spinlock): 使用atomic_flag实现\n\n";

    Spinlock spinlock;
    int counter = 0;
    constexpr int iterations = 100000;

    auto worker = [&]() {
        for (int i = 0; i < iterations; ++i) {
            spinlock.lock();
            ++counter;
            spinlock.unlock();
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(worker);
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "4个线程各递增" << iterations << "次\n";
    std::cout << "结果: " << counter << " (期望: " << 4 * iterations << ")\n";

    std::cout << "\n自旋锁特点:\n";
    std::cout << "  优点: 无上下文切换开销, 短临界区效率高\n";
    std::cout << "  缺点: 忙等待消耗CPU, 长临界区效率低\n";
    std::cout << "  适用: 临界区极短(几条指令), 不适合I/O操作\n";

    std::cout << "\ntry_lock示例:\n";
    Spinlock sl;
    int success_count = 0;
    int fail_count = 0;

    auto try_worker = [&]() {
        for (int i = 0; i < 1000; ++i) {
            if (sl.try_lock()) {
                ++success_count;
                sl.unlock();
            } else {
                ++fail_count;
            }
        }
    };

    std::thread t1(try_worker);
    std::thread t2(try_worker);
    t1.join();
    t2.join();
    std::cout << "  try_lock成功: " << success_count << ", 失败: " << fail_count << "\n";
}

void demo_atomic_flag() {
    std::cout << "\n=== demo_atomic_flag ===\n";
    std::cout << "atomic_flag: 最简单的原子类型, 只有set和clear\n\n";

    std::cout << "1. 基本操作:\n";
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    std::cout << "  初始状态: " << (flag.test_and_set() ? "已设置" : "未设置") << "\n";
    flag.clear();
    std::cout << "  clear后: " << (flag.test_and_set() ? "已设置" : "未设置") << "\n";

    std::cout << "\n2. 用atomic_flag实现一次性初始化:\n";
    std::atomic_flag initialized = ATOMIC_FLAG_INIT;
    int expensive_data = 0;

    auto init_once = [&]() {
        if (!initialized.test_and_set(std::memory_order_acquire)) {
            expensive_data = 42;
            std::cout << "  初始化完成, data=" << expensive_data << "\n";
        } else {
            std::cout << "  已初始化, data=" << expensive_data << "\n";
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(init_once);
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "\n3. atomic_flag vs atomic<bool>:\n";
    std::cout << "  atomic_flag: 保证lock-free, 无load/store, 只有test_and_set/clear\n";
    std::cout << "  atomic<bool>: 可能有load/store, 不保证lock-free\n";
    std::cout << "  建议: 实现自旋锁用atomic_flag, 其他场景用atomic<bool>\n";
}

void demo_cas_loop_patterns() {
    std::cout << "\n=== demo_cas_loop_patterns ===\n";
    std::cout << "CAS循环的常见模式\n\n";

    std::cout << "模式1: 原子更新 (fetch_add的CAS实现)\n";
    std::atomic<int> value{0};
    auto cas_add = [&value](int delta) {
        int old = value.load(std::memory_order_relaxed);
        while (!value.compare_exchange_weak(
            old, old + delta,
            std::memory_order_relaxed,
            std::memory_order_relaxed)) {
        }
        return old;
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 1000; ++j) {
                cas_add(1);
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    std::cout << "  CAS递增结果: " << value.load() << "\n";

    std::cout << "\n模式2: 条件更新 (仅当值满足条件时更新)\n";
    std::atomic<int> data{10};
    auto update_if_greater = [&data](int new_val) {
        int old = data.load(std::memory_order_acquire);
        while (old < new_val) {
            if (data.compare_exchange_weak(
                old, new_val,
                std::memory_order_acq_rel,
                std::memory_order_acquire)) {
                return true;
            }
        }
        return false;
    };

    bool r1 = update_if_greater(20);
    bool r2 = update_if_greater(15);
    std::cout << "  更新到20: " << std::boolalpha << r1 << ", 当前值=" << data.load() << "\n";
    std::cout << "  更新到15: " << r2 << ", 当前值=" << data.load() << " (15<20, 不更新)\n";

    std::cout << "\n模式3: 状态机转换\n";
    enum class State { Idle, Running, Stopped };
    std::atomic<State> state{State::Idle};

    auto transition = [&state](State from, State to) {
        State expected = from;
        return state.compare_exchange_strong(
            expected, to,
            std::memory_order_acq_rel,
            std::memory_order_acquire);
    };

    bool t1_ok = transition(State::Idle, State::Running);
    bool t2_ok = transition(State::Idle, State::Running);
    bool t3_ok = transition(State::Running, State::Stopped);
    std::cout << "  Idle->Running: " << std::boolalpha << t1_ok << "\n";
    std::cout << "  Idle->Running(重复): " << t2_ok << " (失败, 当前为Running)\n";
    std::cout << "  Running->Stopped: " << t3_ok << "\n";

    std::cout << "\n模式4: 无锁读取-修改-写入 (使用两个原子变量)\n";
    std::atomic<int> stats_count{0};
    std::atomic<double> stats_sum{0.0};

    auto update_stats = [&](double val) {
        stats_count.fetch_add(1, std::memory_order_relaxed);
        double old_sum = stats_sum.load(std::memory_order_relaxed);
        while (!stats_sum.compare_exchange_weak(
            old_sum, old_sum + val,
            std::memory_order_relaxed,
            std::memory_order_relaxed)) {
        }
    };

    for (int i = 0; i < 10; ++i) {
        update_stats(i * 1.5);
    }
    std::cout << "  统计: count=" << stats_count.load()
              << ", sum=" << stats_sum.load() << "\n";
}

void demo_backoff_strategies() {
    std::cout << "\n=== demo_backoff_strategies ===\n";
    std::cout << "CAS失败时的退避策略\n\n";

    std::atomic<int> counter{0};
    constexpr int total = 100000;

    std::cout << "1. 无退避 (忙等待):\n";
    auto no_backoff = [&]() {
        for (int i = 0; i < total; ++i) {
            int old = counter.load(std::memory_order_relaxed);
            while (!counter.compare_exchange_weak(
                old, old + 1,
                std::memory_order_relaxed,
                std::memory_order_relaxed)) {
            }
        }
    };

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(no_backoff);
    }
    for (auto& t : threads) {
        t.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto no_backoff_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "  耗时: " << no_backoff_us << " us, 结果: " << counter.load() << "\n";

    std::cout << "\n2. 指数退避:\n";
    counter.store(0);
    auto exp_backoff = [&]() {
        for (int i = 0; i < total; ++i) {
            int old = counter.load(std::memory_order_relaxed);
            int backoff = 1;
            while (!counter.compare_exchange_weak(
                old, old + 1,
                std::memory_order_relaxed,
                std::memory_order_relaxed)) {
                for (int b = 0; b < backoff; ++b) {}
                backoff = std::min(backoff * 2, 64);
            }
        }
    };

    start = std::chrono::high_resolution_clock::now();
    threads.clear();
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(exp_backoff);
    }
    for (auto& t : threads) {
        t.join();
    }
    end = std::chrono::high_resolution_clock::now();
    auto exp_backoff_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    std::cout << "  耗时: " << exp_backoff_us << " us, 结果: " << counter.load() << "\n";

    std::cout << "\n退避策略选择:\n";
    std::cout << "  低竞争: 无退避即可\n";
    std::cout << "  中竞争: 固定退避或指数退避\n";
    std::cout << "  高竞争: 指数退避 + 随机抖动\n";
    std::cout << "  极高竞争: 考虑使用mutex替代\n";
}

int main() {
    std::cout << "CAS模式演示\n";

    demo_spinlock();
    demo_atomic_flag();
    demo_cas_loop_patterns();
    demo_backoff_strategies();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
