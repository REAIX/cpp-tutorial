/** @file 01_deep_dive_atomic_patterns.cpp @brief 原子操作模式深入探讨 @description 对应文档: 02-CPP/31-atomic-async
 *  编译命令: g++ -std=c++20 01_deep_dive_atomic_patterns.cpp -o 01_deep_dive_atomic_patterns
 */

#include <iostream>
#include <thread>
#include <atomic>
#include <vector>
#include <chrono>
#include <cstdint>

void demo_lock_free_counter() {
    std::cout << "\n=== 无锁计数器 ===\n";

    std::atomic<int> counter{0};

    auto increment = [&counter](int count) {
        for (int i = 0; i < count; ++i) {
            counter.fetch_add(1, std::memory_order_relaxed);
        }
    };

    std::vector<std::jthread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(increment, 100000);
    }

    std::cout << "无锁计数: " << counter.load() << " (期望400000)\n";
    std::cout << "is_lock_free: " << counter.is_lock_free() << "\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  fetch_add: 原子加, 返回旧值\n";
    std::cout << "  fetch_sub: 原子减, 返回旧值\n";
    std::cout << "  ++counter: 等价于fetch_add(1) + 1\n";
    std::cout << "  counter++: 等价于fetch_add(1)\n";
}

template<typename T>
class LockFreeStack {
    struct Node {
        T data;
        Node* next;
        Node(T val) : data(std::move(val)), next(nullptr) {}
    };

    std::atomic<Node*> head_{nullptr};

public:
    void push(T value) {
        Node* new_node = new Node(std::move(value));
        new_node->next = head_.load(std::memory_order_relaxed);

        while (!head_.compare_exchange_weak(
            new_node->next,
            new_node,
            std::memory_order_release,
            std::memory_order_relaxed)) {
            // CAS失败, new_node->next已被更新, 重试
        }
    }

    bool pop(T& result) {
        Node* old_head = head_.load(std::memory_order_relaxed);

        while (old_head && !head_.compare_exchange_weak(
            old_head,
            old_head->next,
            std::memory_order_acquire,
            std::memory_order_relaxed)) {
            // CAS失败, old_head已被更新, 重试
        }

        if (old_head) {
            result = std::move(old_head->data);
            delete old_head;
            return true;
        }
        return false;
    }

    ~LockFreeStack() {
        T val;
        while (pop(val)) {}
    }
};

void demo_lock_free_stack() {
    std::cout << "\n=== 无锁栈概念 ===\n";

    LockFreeStack<int> stack;

    auto producer = [&stack](int start, int count) {
        for (int i = 0; i < count; ++i) {
            stack.push(start + i);
        }
    };

    std::vector<std::jthread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(producer, i * 100, 50);
    }

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int val;
    int popped = 0;
    while (stack.pop(val)) {
        ++popped;
    }
    std::cout << "弹出元素数: " << popped << "\n";

    std::cout << "\n注意: 这个简单实现有ABA问题(见下文)\n";
    std::cout << "生产环境应使用hazard pointer或epoch-based回收\n";
}

void demo_aba_problem() {
    std::cout << "\n=== ABA问题 ===\n";

    std::cout << "ABA问题描述:\n";
    std::cout << "  1. 线程1读取head=A, A->next=B\n";
    std::cout << "  2. 线程2弹出A, 弹出B, 再推入A\n";
    std::cout << "  3. 线程1执行CAS: head==A? 是! 成功\n";
    std::cout << "  4. 但A->next已经不是B了(可能已释放)\n";

    std::cout << "\nABA问题的解决方案:\n";

    std::cout << "\n1. 版本号/标签:\n";
    std::cout << "   将指针与版本号打包, 每次修改递增版本号\n";
    std::cout << "   CAS同时比较指针和版本号\n";

    struct TaggedPtr {
        uintptr_t ptr : 48;
        uintptr_t tag : 16;
    };
    std::cout << "   TaggedPtr: 48位指针 + 16位版本号\n";

    std::cout << "\n2. Hazard Pointer:\n";
    std::cout << "   线程声明正在访问的指针\n";
    std::cout << "   回收前检查是否有其他线程正在使用\n";

    std::cout << "\n3. Epoch-Based Reclamation:\n";
    std::cout << "   全局epoch计数器\n";
    std::cout << "   每个线程记录自己的epoch\n";
    std::cout << "   只有所有线程都离开旧epoch后才回收\n";

    std::cout << "\n4. 引用计数:\n";
    std::cout << "   类似shared_ptr, 但需要特别注意性能\n";
}

void demo_memory_order() {
    std::cout << "\n=== 内存序(Memory Order) ===\n";

    std::cout << "六种内存序:\n\n";

    std::cout << "1. memory_order_relaxed:\n";
    std::cout << "   只保证原子性, 不保证顺序\n";
    std::cout << "   适用: 计数器, 统计信息\n";
    {
        std::atomic<int> counter{0};
        counter.fetch_add(1, std::memory_order_relaxed);
        std::cout << "   relaxed计数: " << counter.load(std::memory_order_relaxed) << "\n";
    }

    std::cout << "\n2. memory_order_acquire:\n";
    std::cout << "   读操作: 后续读写不能重排到此操作之前\n";
    std::cout << "   与release配对, 获取release写入的数据\n";

    std::cout << "\n3. memory_order_release:\n";
    std::cout << "   写操作: 之前的读写不能重排到此操作之后\n";
    std::cout << "   与acquire配对, 释放数据给acquire的线程\n";

    {
        std::atomic<bool> ready{false};
        int data = 0;

        std::jthread producer([&ready, &data]() {
            data = 42;
            ready.store(true, std::memory_order_release);
        });

        std::jthread consumer([&ready, &data]() {
            while (!ready.load(std::memory_order_acquire)) {}
            std::cout << "   acquire-release: data=" << data << "\n";
        });
    }

    std::cout << "\n4. memory_order_acq_rel:\n";
    std::cout << "   同时具有acquire和release语义\n";
    std::cout << "   适用: read-modify-write操作(如fetch_add)\n";

    std::cout << "\n5. memory_order_seq_cst (默认):\n";
    std::cout << "   最强的顺序保证\n";
    std::cout << "   所有线程看到相同的操作顺序\n";
    std::cout << "   性能开销最大, 但最安全\n";

    std::cout << "\n6. memory_order_consume:\n";
    std::cout << "   依赖数据的acquire(目前不推荐使用)\n";
    std::cout << "   C++标准建议避免使用, 改用acquire\n";

    std::cout << "\n选择建议:\n";
    std::cout << "  默认: seq_cst (最安全)\n";
    std::cout << "  性能敏感: acquire-release\n";
    std::cout << "  简单计数: relaxed\n";
    std::cout << "  除非有明确性能需求, 否则不要用relaxed\n";
}

void demo_acquire_release_pattern() {
    std::cout << "\n=== Acquire-Release模式 ===\n";

    std::atomic<int> flag{0};
    int shared_data = 0;

    auto writer = [&flag, &shared_data]() {
        shared_data = 100;
        flag.store(1, std::memory_order_release);
    };

    auto reader = [&flag, &shared_data]() {
        while (flag.load(std::memory_order_acquire) != 1) {}
        std::cout << "  读取: shared_data=" << shared_data << "\n";
    };

    std::jthread w(writer);
    std::jthread r(reader);

    std::cout << "\n举一反三:\n";
    std::cout << "  Double-Checked Locking:\n";
    std::cout << "    if (!initialized.load(acquire)) {\n";
    std::cout << "      lock();\n";
    std::cout << "      if (!initialized.load(relaxed)) {\n";
    std::cout << "        init();\n";
    std::cout << "        initialized.store(true, release);\n";
    std::cout << "      }\n";
    std::cout << "      unlock();\n";
    std::cout << "    }\n";
}

int main() {
    std::cout << "========== 原子操作模式深入探讨 ==========\n";

    demo_lock_free_counter();
    demo_lock_free_stack();
    demo_aba_problem();
    demo_memory_order();
    demo_acquire_release_pattern();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
