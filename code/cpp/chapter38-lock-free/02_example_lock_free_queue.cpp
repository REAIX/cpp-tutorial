/**
 * @file 02_example_lock_free_queue.cpp
 * @brief 无锁队列: Michael-Scott队列, 内存回收挑战
 * @description 对应文档: 02-CPP/33-无锁编程
 */

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <optional>

template<typename T>
class LockFreeQueue {
    struct Node {
        std::atomic<T*> data;
        std::atomic<Node*> next;
        Node() : data(nullptr), next(nullptr) {}
        explicit Node(T val) : data(new T(std::move(val))), next(nullptr) {}
    };

    std::atomic<Node*> head_;
    std::atomic<Node*> tail_;

    // ⚠️ 安全警告: 此 hazard pointer 实现是简化版, 仅供教学演示!
    // 完整实现需要: (1) 线程本地 hazard slot 分配 (2) enqueue/dequeue 中设置/清除 hazard ptr
    // 当前版本未在 enqueue/dequeue 中设置 hazard_ptrs, 因此 is_hazard() 始终返回 false,
    // 多线程下 try_reclaim() 可能删除正在被其他线程访问的节点 (use-after-free)。
    // 单线程使用安全; 多线程演示时需注意此限制。
    static constexpr int max_hazard = 2;
    static constexpr int hazard_count = 16;
    std::atomic<Node*> hazard_ptrs[hazard_count][max_hazard];

    bool is_hazard(Node* p) {
        for (int i = 0; i < hazard_count; ++i) {
            for (int j = 0; j < max_hazard; ++j) {
                if (hazard_ptrs[i][j].load(std::memory_order_acquire) == p) {
                    return true;
                }
            }
        }
        return false;
    }

    void try_reclaim(Node* p) {
        if (!is_hazard(p)) {
            delete p;
        } else {
            p->next.store(nullptr, std::memory_order_relaxed);
        }
    }

public:
    LockFreeQueue() {
        Node* sentinel = new Node();
        head_.store(sentinel, std::memory_order_relaxed);
        tail_.store(sentinel, std::memory_order_relaxed);
        for (int i = 0; i < hazard_count; ++i) {
            for (int j = 0; j < max_hazard; ++j) {
                hazard_ptrs[i][j].store(nullptr, std::memory_order_relaxed);
            }
        }
    }

    ~LockFreeQueue() {
        T val;
        while (dequeue(val)) {}
        Node* sentinel = head_.load(std::memory_order_relaxed);
        delete sentinel;
    }

    void enqueue(T value) {
        Node* new_node = new Node(std::move(value));
        Node* old_tail = tail_.load(std::memory_order_acquire);
        for (;;) {
            Node* next = old_tail->next.load(std::memory_order_acquire);
            if (old_tail == tail_.load(std::memory_order_acquire)) {
                if (next == nullptr) {
                    if (old_tail->next.compare_exchange_weak(
                        next, new_node,
                        std::memory_order_release,
                        std::memory_order_relaxed)) {
                        tail_.compare_exchange_strong(
                            old_tail, new_node,
                            std::memory_order_release,
                            std::memory_order_relaxed);
                        return;
                    }
                } else {
                    tail_.compare_exchange_strong(
                        old_tail, next,
                        std::memory_order_release,
                        std::memory_order_relaxed);
                }
            }
            old_tail = tail_.load(std::memory_order_acquire);
        }
    }

    bool dequeue(T& result) {
        Node* old_head = head_.load(std::memory_order_acquire);
        for (;;) {
            Node* next = old_head->next.load(std::memory_order_acquire);
            if (old_head == head_.load(std::memory_order_acquire)) {
                if (next == nullptr) {
                    return false;
                }
                T* data = next->data.load(std::memory_order_relaxed);
                if (head_.compare_exchange_weak(
                    old_head, next,
                    std::memory_order_release,
                    std::memory_order_relaxed)) {
                    result = std::move(*data);
                    delete data;
                    try_reclaim(old_head);
                    return true;
                }
            }
            old_head = head_.load(std::memory_order_acquire);
        }
    }

    bool empty() const {
        Node* h = head_.load(std::memory_order_acquire);
        Node* next = h->next.load(std::memory_order_acquire);
        return next == nullptr;
    }
};

void demo_basic_queue() {
    std::cout << "\n=== demo_basic_queue ===\n";
    std::cout << "Michael-Scott无锁队列\n\n";

    LockFreeQueue<int> queue;

    std::cout << "1. 单线程测试:\n";
    for (int i = 1; i <= 5; ++i) {
        queue.enqueue(i);
        std::cout << "  enqueue(" << i << ")\n";
    }

    int val;
    while (queue.dequeue(val)) {
        std::cout << "  dequeue() = " << val << "\n";
    }
    std::cout << "  队列为空: " << std::boolalpha << queue.empty() << "\n";

    std::cout << "\n2. 多生产者-多消费者测试:\n";
    LockFreeQueue<int> shared_queue;
    constexpr int items_per_producer = 500;
    constexpr int num_producers = 4;
    constexpr int num_consumers = 4;
    std::atomic<int> total_consumed{0};
    std::atomic<int> sum_consumed{0};

    auto producer = [&](int id) {
        for (int i = 0; i < items_per_producer; ++i) {
            shared_queue.enqueue(id * items_per_producer + i);
        }
    };

    auto consumer = [&]() {
        int local_count = 0;
        int local_sum = 0;
        int val;
        while (total_consumed.load(std::memory_order_relaxed) < num_producers * items_per_producer) {
            if (shared_queue.dequeue(val)) {
                local_count++;
                local_sum += val;
                total_consumed.fetch_add(1, std::memory_order_relaxed);
            }
        }
        sum_consumed.fetch_add(local_sum, std::memory_order_relaxed);
    };

    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back(producer, i);
    }
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back(consumer);
    }
    for (auto& t : producers) {
        t.join();
    }
    for (auto& t : consumers) {
        t.join();
    }

    int expected_sum = 0;
    for (int i = 0; i < num_producers * items_per_producer; ++i) {
        expected_sum += i;
    }
    std::cout << "  消费总数: " << total_consumed.load() << "\n";
    std::cout << "  消费总和: " << sum_consumed.load() << " (期望: " << expected_sum << ")\n";
}

void demo_queue_structure() {
    std::cout << "\n=== demo_queue_structure ===\n";
    std::cout << "Michael-Scott队列结构详解\n\n";

    std::cout << "关键设计:\n";
    std::cout << "  1. 哨兵节点(dummy node): head始终指向哨兵节点\n";
    std::cout << "  2. 真正的数据从head->next开始\n";
    std::cout << "  3. tail可能滞后: 不影响正确性, 只影响性能\n\n";

    std::cout << "enqueue操作:\n";
    std::cout << "  1. 读取tail, 检查tail->next\n";
    std::cout << "  2. 如果tail->next为空, CAS设置tail->next为新节点\n";
    std::cout << "  3. 如果tail->next非空, 推进tail (帮助其他线程)\n";
    std::cout << "  4. CAS成功后, 尝试推进tail\n\n";

    std::cout << "dequeue操作:\n";
    std::cout << "  1. 读取head, 检查head->next\n";
    std::cout << "  2. 如果head->next为空, 队列为空\n";
    std::cout << "  3. CAS将head从哨兵推进到head->next\n";
    std::cout << "  4. 旧哨兵节点成为新哨兵, 旧哨兵的数据即为出队值\n\n";

    std::cout << "为什么需要哨兵节点?\n";
    std::cout << "  避免enqueue和dequeue竞争同一节点\n";
    std::cout << "  head和tail始终指向不同节点\n";
}

void demo_memory_reclamation_challenge() {
    std::cout << "\n=== demo_memory_reclamation_challenge ===\n";
    std::cout << "无锁数据结构的内存回收挑战\n\n";

    std::cout << "核心问题:\n";
    std::cout << "  线程A从队列中移除节点后, 如何确认没有其他线程还在访问该节点?\n";
    std::cout << "  过早释放: 其他线程访问已释放内存 (use-after-free)\n";
    std::cout << "  过晚释放: 内存泄漏\n\n";

    std::cout << "主要解决方案:\n\n";

    std::cout << "1. Hazard Pointer (风险指针):\n";
    std::cout << "   每个线程声明正在访问的指针\n";
    std::cout << "   释放前检查是否有线程持有该指针\n";
    std::cout << "   优点: 确定性延迟, 内存回收及时\n";
    std::cout << "   缺点: 每次访问需更新hazard pointer, 开销较大\n\n";

    std::cout << "2. Epoch-based Reclamation (基于时代的回收):\n";
    std::cout << "   全局维护当前时代号\n";
    std::cout << "   线程进入临界区时记录当前时代\n";
    std::cout << "   只有所有线程都离开旧时代后, 才回收旧时代的节点\n";
    std::cout << "   优点: 批量回收, 性能好\n";
    std::cout << "   缺点: 可能延迟回收, 需要线程协作\n\n";

    std::cout << "3. 引用计数:\n";
    std::cout << "   每个节点维护原子引用计数\n";
    std::cout << "   访问时增加计数, 离开时减少\n";
    std::cout << "   优点: 简单直观\n";
    std::cout << "   缺点: 原子计数器开销大, 可能ABA问题\n\n";

    std::cout << "4. Quiescent State-Based Reclamation (QSBR):\n";
    std::cout << "   线程声明进入静默状态(不在临界区)\n";
    std::cout << "   所有线程都经过静默状态后, 回收安全\n";
    std::cout << "   优点: 零开销读取\n";
    std::cout << "   缺点: 需要线程周期性声明静默\n";

    std::cout << "\n本示例使用简化版Hazard Pointer\n";
    std::cout << "生产环境建议使用成熟的库(如libcds, folly)\n";
}

int main() {
    std::cout << "无锁队列实现演示\n";

    demo_basic_queue();
    demo_queue_structure();
    demo_memory_reclamation_challenge();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
