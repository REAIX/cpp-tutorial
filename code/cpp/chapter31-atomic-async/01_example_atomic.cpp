/** @file 01_example_atomic.cpp @brief 原子操作示例 @description 对应文档: 02-CPP/31-atomic-async
 *  编译命令: g++ -std=c++20 01_example_atomic.cpp -o 01_example_atomic
 */

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>

void demo_atomic_basic() {
    std::cout << "\n=== atomic基础 ===\n";

    std::atomic<int> counter{0};

    auto increment = [&counter](int count) {
        for (int i = 0; i < count; ++i) {
            counter.fetch_add(1);
        }
    };

    std::thread t1(increment, 100000);
    std::thread t2(increment, 100000);
    t1.join();
    t2.join();
    std::cout << "atomic计数: " << counter.load() << " (期望200000)\n";

    std::atomic<bool> flag{false};
    std::cout << "atomic<bool>: " << flag.load() << "\n";
    flag.store(true);
    std::cout << "store(true)后: " << flag.load() << "\n";
}

void demo_fetch_operations() {
    std::cout << "\n=== fetch操作 ===\n";

    std::atomic<int> val{100};

    int old = val.fetch_add(10);
    std::cout << "fetch_add(10): 旧值=" << old << " 新值=" << val.load() << "\n";

    old = val.fetch_sub(5);
    std::cout << "fetch_sub(5): 旧值=" << old << " 新值=" << val.load() << "\n";

    old = val.fetch_and(0xFF);
    std::cout << "fetch_and(0xFF): 旧值=" << old << " 新值=" << val.load() << "\n";

    old = val.fetch_or(0x100);
    std::cout << "fetch_or(0x100): 旧值=" << old << " 新值=" << val.load() << "\n";

    old = val.fetch_xor(0x55);
    std::cout << "fetch_xor(0x55): 旧值=" << old << " 新值=" << val.load() << "\n";

    std::cout << "\nfetch操作返回旧值, 这是原子的:\n";
    std::cout << "  fetch_add: 先返回旧值, 再加\n";
    std::cout << "  fetch_sub: 先返回旧值, 再减\n";
    std::cout << "  整个操作是不可分割的\n";
}

void demo_exchange() {
    std::cout << "\n=== exchange操作 ===\n";

    std::atomic<int> val{42};

    int old = val.exchange(100);
    std::cout << "exchange(100): 旧值=" << old << " 新值=" << val.load() << "\n";

    std::cout << "\nexchange: 原子地设置新值并返回旧值\n";
    std::cout << "用途: 无锁数据结构中交换值\n";

    std::atomic<std::string*> ptr{nullptr};
    auto* new_str = new std::string("hello");
    std::string* old_ptr = ptr.exchange(new_str);
    std::cout << "指针exchange: 旧=" << old_ptr << " 新=" << ptr.load() << "\n";
    delete ptr.exchange(nullptr);
}

void demo_compare_exchange() {
    std::cout << "\n=== compare_exchange操作 ===\n";

    std::atomic<int> val{42};

    int expected = 42;
    bool success = val.compare_exchange_strong(expected, 100);
    std::cout << "CAS(42->100): 成功=" << success << " 值=" << val.load()
              << " expected=" << expected << "\n";

    expected = 42;
    success = val.compare_exchange_strong(expected, 200);
    std::cout << "CAS(42->200): 成功=" << success << " 值=" << val.load()
              << " expected=" << expected << "\n";
    std::cout << "失败时expected被更新为当前值: " << expected << "\n";

    std::cout << "\ncompare_exchange_weak vs strong:\n";
    std::cout << "  weak: 可能虚假失败(即使值相等也返回false)\n";
    std::cout << "  strong: 只有值不相等时才返回false\n";
    std::cout << "  weak在循环中性能可能更好(某些架构)\n";
    std::cout << "  非循环场景用strong\n";

    std::cout << "\nCAS循环模式:\n";
    std::atomic<int> counter{0};
    auto atomic_increment = [&counter]() {
        int old = counter.load();
        while (!counter.compare_exchange_weak(old, old + 1)) {
            // old被自动更新为当前值, 重试
        }
    };

    std::vector<std::jthread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&atomic_increment]() {
            for (int j = 0; j < 10000; ++j) atomic_increment();
        });
    }
    std::cout << "CAS循环计数: " << counter.load() << " (期望40000)\n";
}

void demo_atomic_flag() {
    std::cout << "\n=== atomic_flag ===\n";

    std::atomic_flag flag = ATOMIC_FLAG_INIT;

    bool was_set = flag.test_and_set();
    std::cout << "test_and_set: 之前=" << was_set << " 之后=" << flag.test() << "\n";

    flag.clear();
    std::cout << "clear后: " << flag.test() << "\n";

    std::cout << "\natomic_flag特点:\n";
    std::cout << "  1. 最简单的原子类型, 保证无锁\n";
    std::cout << "  2. 只有test_and_set, clear, test操作\n";
    std::cout << "  3. 可以用来实现自旋锁\n";

    std::cout << "\n自旋锁实现:\n";
    class SpinLock {
        std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
    public:
        void lock() {
            while (flag_.test_and_set(std::memory_order_acquire)) {
                // 忙等待
            }
        }
        void unlock() {
            flag_.clear(std::memory_order_release);
        }
    };

    SpinLock splock;
    int sp_counter = 0;
    std::vector<std::jthread> sp_threads;
    for (int i = 0; i < 4; ++i) {
        sp_threads.emplace_back([&splock, &sp_counter]() {
            for (int j = 0; j < 10000; ++j) {
                splock.lock();
                ++sp_counter;
                splock.unlock();
            }
        });
    }
    std::cout << "自旋锁计数: " << sp_counter << " (期望40000)\n";
}

int main() {
    std::cout << "========== 原子操作示例 ==========\n";

    demo_atomic_basic();
    demo_fetch_operations();
    demo_exchange();
    demo_compare_exchange();
    demo_atomic_flag();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
