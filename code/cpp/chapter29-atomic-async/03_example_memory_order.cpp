/**
 * @file 01_example_memory_order.cpp
 * @brief 内存序演示: relaxed, acquire/release, seq_cst
 * @description 对应文档: 02-CPP/32-内存模型
 */

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <cassert>

void demo_memory_order_relaxed() {
    std::cout << "\n=== demo_memory_order_relaxed ===\n";
    std::cout << "memory_order_relaxed: 仅保证原子性, 不保证顺序\n";
    std::cout << "适用场景: 计数器、统计信息等不需要同步的数据\n\n";

    std::atomic<int> counter{0};
    constexpr int iterations = 10000;

    auto increment = [&counter]() {
        for (int i = 0; i < iterations; ++i) {
            counter.fetch_add(1, std::memory_order_relaxed);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(increment);
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "4个线程各递增10000次, 结果: " << counter.load(std::memory_order_relaxed) << "\n";
    std::cout << "relaxed保证原子性(结果正确=40000), 但不保证操作顺序\n";

    std::atomic<int> x{0}, y{0};
    std::atomic<int> rx{0}, ry{0};

    auto write_x_then_y = [&]() {
        x.store(1, std::memory_order_relaxed);
        y.store(1, std::memory_order_relaxed);
    };

    auto read_y_then_x = [&]() {
        while (y.load(std::memory_order_relaxed) != 1) {}
        rx.store(x.load(std::memory_order_relaxed), std::memory_order_relaxed);
    };

    std::thread t1(write_x_then_y);
    std::thread t2(read_y_then_x);
    t1.join();
    t2.join();

    std::cout << "relaxed序下, rx=" << rx.load() << " (可能为0, 因为x的写入对t2不一定可见)\n";
    std::cout << "关键: relaxed不建立happens-before关系, 写入顺序不可预测\n";
}

void demo_memory_order_acquire_release() {
    std::cout << "\n=== demo_memory_order_acquire_release ===\n";
    std::cout << "memory_order_acquire: 读操作, 后续读写不能重排到此操作之前\n";
    std::cout << "memory_order_release: 写操作, 之前的读写不能重排到此操作之后\n";
    std::cout << "acquire+release配对: 建立synchronizes-with关系\n\n";

    std::atomic<bool> ready{false};
    int data = 0;

    auto producer = [&]() {
        data = 42;
        ready.store(true, std::memory_order_release);
    };

    auto consumer = [&]() {
        while (!ready.load(std::memory_order_acquire)) {}
        std::cout << "消费者读取 data=" << data << " (保证看到42)\n";
    };

    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join();
    t2.join();

    std::cout << "\nacquire/release链式传递示例:\n";
    std::atomic<int> flag1{0}, flag2{0};
    int shared_data = 0;

    auto thread_a = [&]() {
        shared_data = 100;
        flag1.store(1, std::memory_order_release);
    };

    auto thread_b = [&]() {
        while (flag1.load(std::memory_order_acquire) != 1) {}
        flag2.store(1, std::memory_order_release);
    };

    auto thread_c = [&]() {
        while (flag2.load(std::memory_order_acquire) != 1) {}
        std::cout << "线程C读取 shared_data=" << shared_data << " (通过链式传递保证看到100)\n";
    };

    std::thread ta(thread_a);
    std::thread tb(thread_b);
    std::thread tc(thread_c);
    ta.join();
    tb.join();
    tc.join();

    std::cout << "\nrelease语义确保: release之前的所有写入对acquire线程可见\n";
    std::cout << "acquire语义确保: acquire之后的所有读取能看到release前的写入\n";
}

void demo_memory_order_seq_cst() {
    std::cout << "\n=== demo_memory_order_seq_cst ===\n";
    std::cout << "memory_order_seq_cst (默认): 顺序一致性, 全局统一顺序\n";
    std::cout << "最严格的内存序, 所有线程看到相同的操作顺序\n\n";

    std::atomic<bool> x{false}, y{false};
    std::atomic<int> z{0};

    auto write_x = [&]() {
        x.store(true, std::memory_order_seq_cst);
    };

    auto write_y = [&]() {
        y.store(true, std::memory_order_seq_cst);
    };

    auto read_x_then_y = [&]() {
        while (!x.load(std::memory_order_seq_cst)) {}
        if (y.load(std::memory_order_seq_cst)) {
            ++z;
        }
    };

    auto read_y_then_x = [&]() {
        while (!y.load(std::memory_order_seq_cst)) {}
        if (x.load(std::memory_order_seq_cst)) {
            ++z;
        }
    };

    for (int i = 0; i < 10; ++i) {
        x = false;
        y = false;
        z = 0;

        std::thread t1(write_x);
        std::thread t2(write_y);
        std::thread t3(read_x_then_y);
        std::thread t4(read_y_then_x);
        t1.join();
        t2.join();
        t3.join();
        t4.join();

        std::cout << "第" << (i + 1) << "次运行, z=" << z.load() << " (seq_cst保证z>=1)\n";
    }

    std::cout << "\nseq_cst保证: 不可能出现r1=false且r2=false的情况\n";
    std::cout << "代价: 在x86上几乎无额外开销, 在ARM上需要dmb指令\n";

    std::cout << "\n三种内存序对比:\n";
    std::cout << "  relaxed:  仅原子性, 无顺序保证, 性能最好\n";
    std::cout << "  acquire/release: 配对使用, 建立同步关系, 适中开销\n";
    std::cout << "  seq_cst:  全局一致顺序, 最强保证, 可能影响性能\n";
}

void demo_memory_order_comparison() {
    std::cout << "\n=== demo_memory_order_comparison ===\n";
    std::cout << "不同内存序的性能对比 (简单计数器场景)\n\n";

    constexpr int iterations = 1000000;

    auto bench_relaxed = [&]() {
        std::atomic<int> val{0};
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            val.fetch_add(1, std::memory_order_relaxed);
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };

    auto bench_seq_cst = [&]() {
        std::atomic<int> val{0};
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < iterations; ++i) {
            val.fetch_add(1, std::memory_order_seq_cst);
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };

    auto relaxed_us = bench_relaxed();
    auto seq_cst_us = bench_seq_cst();

    std::cout << iterations << "次 fetch_add:\n";
    std::cout << "  relaxed: " << relaxed_us << " us\n";
    std::cout << "  seq_cst: " << seq_cst_us << " us\n";
    std::cout << "  差异: " << (seq_cst_us > relaxed_us ? seq_cst_us - relaxed_us : relaxed_us - seq_cst_us) << " us\n";
    std::cout << "注意: x86上差异较小(TSO模型), ARM上差异更明显\n";
}

int main() {
    std::cout << "C++内存序(memory order)演示\n";

    demo_memory_order_relaxed();
    demo_memory_order_acquire_release();
    demo_memory_order_seq_cst();
    demo_memory_order_comparison();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
