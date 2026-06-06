/**
 * @file 02_example_happens_before.cpp
 * @brief Happens-before关系演示: synchronizes-with, inter-thread happens-before
 * @description 对应文档: 02-CPP/32-内存模型
 */

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <chrono>

void demo_happens_before_basic() {
    std::cout << "\n=== demo_happens_before_basic ===\n";
    std::cout << "Happens-before (先于关系): C++内存模型的核心概念\n";
    std::cout << "如果 A happens-before B, 则A的效果对B可见\n\n";

    std::cout << "1. 单线程内的happens-before (程序序):\n";
    std::cout << "   int a = 1;      // A\n";
    std::cout << "   int b = a + 1;  // B  (A happens-before B)\n";
    std::cout << "   编译器不能将B重排到A之前\n\n";

    std::cout << "2. 多线程的happens-before (通过同步建立):\n";
    std::cout << "   线程1: data=42; ready.store(true, release)  // A\n";
    std::cout << "   线程2: while(!ready.load(acquire)); use(data) // B\n";
    std::cout << "   A happens-before B (通过synchronizes-with)\n\n";

    int data = 0;
    std::atomic<bool> ready{false};

    std::thread producer([&]() {
        data = 42;
        ready.store(true, std::memory_order_release);
    });

    std::thread consumer([&]() {
        while (!ready.load(std::memory_order_acquire)) {}
        std::cout << "消费者读取 data=" << data << " (happens-before保证可见)\n";
    });

    producer.join();
    consumer.join();
}

void demo_synchronizes_with() {
    std::cout << "\n=== demo_synchronizes_with ===\n";
    std::cout << "Synchronizes-with (同步于): 建立线程间happens-before的桥梁\n";
    std::cout << "条件: 线程A的release写 与 线程B的acquire读 读取了该写值\n\n";

    std::cout << "synchronizes-with的几种形式:\n";
    std::cout << "  1. atomic的release写 + acquire读\n";
    std::cout << "  2. mutex的unlock + lock\n";
    std::cout << "  3. thread的创建 (父线程创建子线程)\n";
    std::cout << "  4. thread的join (子线程结束 + 父线程join返回)\n\n";

    std::atomic<int> sync_flag{0};
    int payload = 0;

    auto worker = [&](int id) {
        payload = id * 100;
        sync_flag.store(id, std::memory_order_release);
        std::cout << "  线程" << id << ": 写入payload=" << payload << ", 设置flag=" << id << "\n";
    };

    std::thread t1(worker, 1);
    t1.join();

    int flag_val = sync_flag.load(std::memory_order_acquire);
    std::cout << "  主线程: 读取flag=" << flag_val << ", payload=" << payload << "\n";
    std::cout << "  join操作也建立了synchronizes-with关系\n";

    std::cout << "\nthread创建的happens-before:\n";
    std::atomic<int> created_flag{0};
    int before_create = 999;

    std::thread t2([&]() {
        int val = created_flag.load(std::memory_order_acquire);
        std::cout << "  子线程看到 before_create=" << before_create
                  << ", created_flag=" << val << "\n";
        std::cout << "  父线程在创建子线程之前的写入, 对子线程可见\n";
    });

    created_flag.store(1, std::memory_order_release);
    t2.join();
}

void demo_inter_thread_happens_before() {
    std::cout << "\n=== demo_inter_thread_happens_before ===\n";
    std::cout << "Inter-thread happens-before: 跨线程的传递性先于关系\n";
    std::cout << "如果 A happens-before B 且 B happens-before C, 则 A happens-before C\n\n";

    std::atomic<int> flag_a{0}, flag_b{0};
    int shared = 0;

    auto thread1 = [&]() {
        shared = 777;
        flag_a.store(1, std::memory_order_release);
        std::cout << "  线程1: 写入shared=777, 设置flag_a=1\n";
    };

    auto thread2 = [&]() {
        while (flag_a.load(std::memory_order_acquire) != 1) {}
        std::cout << "  线程2: 看到flag_a=1, 此时shared=" << shared << "\n";
        flag_b.store(1, std::memory_order_release);
        std::cout << "  线程2: 设置flag_b=1\n";
    };

    auto thread3 = [&]() {
        while (flag_b.load(std::memory_order_acquire) != 1) {}
        std::cout << "  线程3: 看到flag_b=1, 此时shared=" << shared << "\n";
        std::cout << "  传递性: 线程1的写入 -> 线程2 -> 线程3, shared保证为777\n";
    };

    std::thread t1(thread1);
    std::thread t2(thread2);
    std::thread t3(thread3);
    t1.join();
    t2.join();
    t3.join();

    std::cout << "\n传递链: shared=777 (线程1) --release/acquire--> flag_a=1 (线程2) --release/acquire--> flag_b=1 (线程3)\n";
    std::cout << "线程1的写入通过中间线程2传递到线程3\n";
}

void demo_sequential_consistency() {
    std::cout << "\n=== demo_sequential_consistency ===\n";
    std::cout << "顺序一致性(Sequential Consistency): 最直观的内存模型\n";
    std::cout << "所有线程看到相同的操作顺序, 等价于所有操作交错执行\n\n";

    std::cout << "Dekker算法示例 (seq_cst保证正确性):\n";
    std::atomic<bool> x_wants{false}, y_wants{false};
    std::atomic<int> critical_count{0};

    auto process_x = [&]() {
        x_wants.store(true, std::memory_order_seq_cst);
        if (y_wants.load(std::memory_order_seq_cst)) {
            x_wants.store(false, std::memory_order_seq_cst);
            std::cout << "  X让步, 避免死锁\n";
        } else {
            critical_count.fetch_add(1, std::memory_order_relaxed);
            std::cout << "  X进入临界区\n";
            x_wants.store(false, std::memory_order_seq_cst);
        }
    };

    auto process_y = [&]() {
        y_wants.store(true, std::memory_order_seq_cst);
        if (x_wants.load(std::memory_order_seq_cst)) {
            y_wants.store(false, std::memory_order_seq_cst);
            std::cout << "  Y让步, 避免死锁\n";
        } else {
            critical_count.fetch_add(1, std::memory_order_relaxed);
            std::cout << "  Y进入临界区\n";
            y_wants.store(false, std::memory_order_seq_cst);
        }
    };

    for (int i = 0; i < 5; ++i) {
        x_wants = false;
        y_wants = false;
        std::thread t1(process_x);
        std::thread t2(process_y);
        t1.join();
        t2.join();
    }

    std::cout << "\nseq_cst确保: 不会两个线程同时认为对方不想进入临界区\n";
    std::cout << "如果用relaxed, 两个线程可能同时进入临界区!\n";

    std::cout << "\nhappens-before关系总结:\n";
    std::cout << "  1. 程序序: 单线程内语句顺序\n";
    std::cout << "  2. synchronizes-with: release写+acquire读\n";
    std::cout << "  3. 传递性: A->B, B->C => A->C\n";
    std::cout << "  4. join: 子线程所有操作 happens-before join返回\n";
    std::cout << "  5. mutex: unlock happens-before lock\n";
}

int main() {
    std::cout << "Happens-before关系演示\n";

    demo_happens_before_basic();
    demo_synchronizes_with();
    demo_inter_thread_happens_before();
    demo_sequential_consistency();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
