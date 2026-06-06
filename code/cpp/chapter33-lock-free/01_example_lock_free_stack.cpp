/**
 * @file 01_example_lock_free_stack.cpp
 * @brief 无锁栈: CAS实现push/pop, ABA问题引入
 * @description 对应文档: 02-CPP/33-无锁编程
 */

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <cassert>

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
        }
    }

    // ⚠️ 安全警告: 此 pop() 实现存在 use-after-free 风险!
    // 当 CAS 失败后 old_head 被更新为新的 head 值, 再次循环时访问 old_head->next,
    // 若另一线程已 delete 该节点, 则解引用悬垂指针, 属于未定义行为。
    // 生产环境需使用 Hazard Pointer 或 Epoch-based 回收等延迟释放机制。
    // 此处仅作为 ABA 问题的教学引入, 不应用于生产代码。
    bool pop(T& result) {
        Node* old_head = head_.load(std::memory_order_relaxed);
        while (old_head) {
            if (head_.compare_exchange_weak(
                old_head,
                old_head->next,
                std::memory_order_acquire,
                std::memory_order_relaxed)) {
                result = std::move(old_head->data);
                delete old_head;
                return true;
            }
        }
        return false;
    }

    bool empty() const {
        return head_.load(std::memory_order_relaxed) == nullptr;
    }

    ~LockFreeStack() {
        T val{};
        while (pop(val)) {}
    }
};

void demo_basic_lock_free_stack() {
    std::cout << "\n=== demo_basic_lock_free_stack ===\n";
    std::cout << "无锁栈: 使用CAS(Compare-And-Swap)实现\n\n";

    LockFreeStack<int> stack;

    std::cout << "1. 单线程测试:\n";
    for (int i = 1; i <= 5; ++i) {
        stack.push(i);
        std::cout << "  push(" << i << ")\n";
    }

    int val;
    while (stack.pop(val)) {
        std::cout << "  pop() = " << val << "\n";
    }
    std::cout << "  栈为空: " << std::boolalpha << stack.empty() << "\n";

    std::cout << "\n2. 多线程测试:\n";
    LockFreeStack<int> shared_stack;
    constexpr int items_per_thread = 1000;

    auto pusher = [&](int start) {
        for (int i = 0; i < items_per_thread; ++i) {
            shared_stack.push(start + i);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(pusher, i * items_per_thread);
    }
    for (auto& t : threads) {
        t.join();
    }

    int count = 0;
    int v;
    while (shared_stack.pop(v)) {
        ++count;
    }
    std::cout << "  4个线程各push " << items_per_thread << " 个元素\n";
    std::cout << "  pop总数: " << count << " (期望: " << 4 * items_per_thread << ")\n";
}

void demo_cas_loop_explained() {
    std::cout << "\n=== demo_cas_loop_explained ===\n";
    std::cout << "CAS循环详解\n\n";

    std::cout << "push操作的CAS循环:\n";
    std::cout << "  1. 创建新节点, next指向当前head\n";
    std::cout << "  2. CAS: 如果head没变, 将head更新为新节点\n";
    std::cout << "  3. 如果head变了(其他线程修改), 重试\n\n";

    std::cout << "pop操作的CAS循环:\n";
    std::cout << "  1. 读取当前head\n";
    std::cout << "  2. CAS: 如果head没变, 将head更新为head->next\n";
    std::cout << "  3. 如果head变了, 重试\n\n";

    std::atomic<int> counter{0};
    auto cas_increment = [&counter]() {
        int old_val = counter.load(std::memory_order_relaxed);
        while (!counter.compare_exchange_weak(
            old_val,
            old_val + 1,
            std::memory_order_release,
            std::memory_order_relaxed)) {
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < 1000; ++j) {
                cas_increment();
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    std::cout << "CAS递增结果: " << counter.load() << " (期望: 10000)\n";

    std::cout << "\ncompare_exchange_weak vs strong:\n";
    std::cout << "  weak: 可能虚假失败(值相等也返回false), 但在某些架构上更快\n";
    std::cout << "  strong: 仅当值不等时失败, 但可能更慢\n";
    std::cout << "  建议: 循环中使用weak, 非循环使用strong\n";
}

void demo_aba_problem_introduction() {
    std::cout << "\n=== demo_aba_problem_introduction ===\n";
    std::cout << "ABA问题: 无锁编程的经典陷阱\n\n";

    std::cout << "ABA问题场景:\n";
    std::cout << "  栈: A->B->C, 线程1读取head=A\n";
    std::cout << "  线程2: pop A, pop B, push A (栈变为 A->C)\n";
    std::cout << "  线程1: CAS(A, B)成功! 但B可能已被删除或重用\n";
    std::cout << "  结果: 栈变为B(已失效), 丢失了C\n\n";

    std::cout << "ABA问题的本质:\n";
    std::cout << "  CAS只检查值是否相同, 不检查值是否被修改过\n";
    std::cout << "  值从A变成B再变回A, CAS无法区分\n\n";

    std::cout << "模拟ABA问题:\n";
    std::atomic<int> val{1};
    bool aba_detected = false;

    auto thread1 = [&]() {
        int old = val.load(std::memory_order_relaxed);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        if (val.compare_exchange_strong(old, 100, std::memory_order_relaxed)) {
            std::cout << "  线程1: CAS成功 (1->100), 但值可能经历了1->2->1\n";
            if (old != 1) {
                aba_detected = true;
            }
        } else {
            std::cout << "  线程1: CAS失败, 当前值=" << old << "\n";
        }
    };

    auto thread2 = [&]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(2));
        val.store(2, std::memory_order_relaxed);
        std::cout << "  线程2: 1->2\n";
        val.store(1, std::memory_order_relaxed);
        std::cout << "  线程2: 2->1 (ABA!)\n";
    };

    std::thread t1(thread1);
    std::thread t2(thread2);
    t1.join();
    t2.join();

    std::cout << "\nABA问题的解决方案:\n";
    std::cout << "  1. 版本号/计数器: CAS时同时检查版本号\n";
    std::cout << "  2. Hazard Pointer: 延迟释放, 确保无线程持有旧指针\n";
    std::cout << "  3. Epoch-based回收: 分代回收机制\n";
    std::cout << "  4. 双宽度CAS: 128位CAS同时比较指针和计数器\n";

    std::cout << "\n版本号方案示例:\n";
    std::atomic<uint64_t> tagged{0};
    auto make_tagged = [](uint16_t version, uint64_t ptr) -> uint64_t {
        return (static_cast<uint64_t>(version) << 48) | (ptr & 0x0000FFFFFFFFFFFF);
    };
    auto get_version = [](uint64_t val) -> uint16_t {
        return static_cast<uint16_t>(val >> 48);
    };
    auto get_ptr = [](uint64_t val) -> uint64_t {
        return val & 0x0000FFFFFFFFFFFF;
    };

    uint64_t old_val = tagged.load(std::memory_order_relaxed);
    uint16_t old_version = get_version(old_val);
    uint64_t new_val = make_tagged(old_version + 1, 42);
    if (tagged.compare_exchange_strong(old_val, new_val, std::memory_order_release)) {
        std::cout << "  带版本号的CAS成功, 版本=" << get_version(new_val)
                  << ", ptr=" << get_ptr(new_val) << "\n";
    }
}

int main() {
    std::cout << "无锁栈实现演示\n";

    demo_basic_lock_free_stack();
    demo_cas_loop_explained();
    demo_aba_problem_introduction();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
