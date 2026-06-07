/**
 * @file 01_deep_dive_lock_free_design.cpp
 * @brief 无锁设计原则: 进度保证, 内存回收(Hazard Pointer, Epoch)
 * @description 对应文档: 02-CPP/33-无锁编程
 */

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>

void demo_progress_guarantees() {
    std::cout << "\n=== demo_progress_guarantees ===\n";
    std::cout << "并发数据结构的进度保证\n\n";

    std::cout << "1. 阻塞(Blocking):\n";
    std::cout << "   一个线程的延迟可能阻止其他线程前进\n";
    std::cout << "   例: mutex保护的容器\n";
    std::cout << "   风险: 死锁, 优先级反转, 挂起线程阻塞所有人\n\n";

    std::cout << "2. 无阻碍(Obstruction-Free):\n";
    std::cout << "   如果所有其他线程暂停, 当前线程能在有限步内完成\n";
    std::cout << "   最弱的非阻塞保证\n\n";

    std::cout << "3. 无锁(Lock-Free):\n";
    std::cout << "   系统整体保证前进: 至少一个线程能在有限步内完成\n";
    std::cout << "   不保证每个线程都能前进 (可能有饥饿)\n";
    std::cout << "   例: CAS循环实现的数据结构\n\n";

    std::cout << "4. 无等待(Wait-Free):\n";
    std::cout << "   每个线程都能在有限步内完成操作\n";
    std::cout << "   最强的进度保证, 无饥饿\n";
    std::cout << "   例: atomic的fetch_add (硬件保证)\n\n";

    std::atomic<int> counter{0};
    constexpr int n = 10000;

    std::cout << "无等待操作示例 (fetch_add):\n";
    auto wait_free_worker = [&]() {
        for (int i = 0; i < n; ++i) {
            counter.fetch_add(1, std::memory_order_relaxed);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(wait_free_worker);
    }
    for (auto& t : threads) {
        t.join();
    }
    std::cout << "  fetch_add结果: " << counter.load() << " (wait-free, 有限步完成)\n";

    std::cout << "\n无锁操作示例 (CAS循环):\n";
    counter.store(0);
    auto lock_free_worker = [&]() {
        for (int i = 0; i < n; ++i) {
            int old = counter.load(std::memory_order_relaxed);
            while (!counter.compare_exchange_weak(
                old, old + 1,
                std::memory_order_relaxed,
                std::memory_order_relaxed)) {
            }
        }
    };

    threads.clear();
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(lock_free_worker);
    }
    for (auto& t : threads) {
        t.join();
    }
    std::cout << "  CAS循环结果: " << counter.load() << " (lock-free, 但单线程可能无限重试)\n";

    std::cout << "\n进度保证对比:\n";
    std::cout << "  ┌──────────────┬────────────┬──────────┬──────────┐\n";
    std::cout << "  │ 保证级别     │ 单线程延迟 │ 饥饿可能 │ 实现难度 │\n";
    std::cout << "  ├──────────────┼────────────┼──────────┼──────────┤\n";
    std::cout << "  │ Blocking     │ 阻塞所有人 │ 是       │ 低       │\n";
    std::cout << "  │ Obstruction  │ 阻塞自己   │ 是       │ 中       │\n";
    std::cout << "  │ Lock-Free    │ 不阻塞他人 │ 可能     │ 高       │\n";
    std::cout << "  │ Wait-Free    │ 不阻塞他人 │ 否       │ 极高     │\n";
    std::cout << "  └──────────────┴────────────┴──────────┴──────────┘\n";
}

void demo_hazard_pointer_concept() {
    std::cout << "\n=== demo_hazard_pointer_concept ===\n";
    std::cout << "Hazard Pointer (风险指针) 概念\n\n";

    std::cout << "核心思想:\n";
    std::cout << "  1. 每个线程拥有少量'hazard pointer'槽位\n";
    std::cout << "  2. 访问共享指针前, 先将其存入hazard slot\n";
    std::cout << "  3. 删除节点前, 检查是否被任何hazard pointer引用\n";
    std::cout << "  4. 如果被引用, 放入待删除列表, 延后回收\n\n";

    constexpr int max_hazards = 2;
    constexpr int max_threads = 8;

    struct HazardRecord {
        std::atomic<void*> hazards[max_hazards];
    };

    HazardRecord records[max_threads];
    for (int i = 0; i < max_threads; ++i) {
        for (int j = 0; j < max_hazards; ++j) {
            records[i].hazards[j].store(nullptr, std::memory_order_relaxed);
        }
    }

    struct Node {
        int value;
        Node* next;
        Node(int v) : value(v), next(nullptr) {}
    };

    auto is_hazardous = [&](Node* p) {
        for (int i = 0; i < max_threads; ++i) {
            for (int j = 0; j < max_hazards; ++j) {
                if (records[i].hazards[j].load(std::memory_order_acquire) == p) {
                    return true;
                }
            }
        }
        return false;
    };

    std::vector<Node*> retired_list;

    auto retire_node = [&](Node* p) {
        retired_list.push_back(p);
    };

    auto reclaim_retired = [&]() {
        auto it = retired_list.begin();
        while (it != retired_list.end()) {
            if (!is_hazardous(*it)) {
                delete *it;
                it = retired_list.erase(it);
            } else {
                ++it;
            }
        }
    };

    std::cout << "Hazard Pointer使用流程:\n";
    Node* node = new Node(42);
    records[0].hazards[0].store(node, std::memory_order_release);
    std::cout << "  1. 线程0将节点存入hazard slot\n";
    std::cout << "  2. 其他线程检查: is_hazardous=" << std::boolalpha << is_hazardous(node) << "\n";
    std::cout << "  3. 因为被hazard引用, 不能删除\n";

    retire_node(node);
    std::cout << "  4. 节点放入待删除列表\n";
    reclaim_retired();
    std::cout << "  5. 回收检查: 仍有hazard引用, 未删除\n";

    records[0].hazards[0].store(nullptr, std::memory_order_release);
    std::cout << "  6. 线程0清除hazard slot\n";
    reclaim_retired();
    std::cout << "  7. 再次回收: 无hazard引用, 已删除\n";

    std::cout << "\nHazard Pointer优缺点:\n";
    std::cout << "  优点: 确定性回收, 无批量延迟\n";
    std::cout << "  缺点: 每次访问需更新hazard, 检查开销O(T*H)\n";
    std::cout << "  适用: 读写都频繁的场景\n";
}

void demo_epoch_based_reclamation_concept() {
    std::cout << "\n=== demo_epoch_based_reclamation_concept ===\n";
    std::cout << "Epoch-based Reclamation (基于时代的回收) 概念\n\n";

    std::cout << "核心思想:\n";
    std::cout << "  1. 全局维护一个递增的时代号(epoch)\n";
    std::cout << "  2. 线程进入临界区时, 记录当前时代号\n";
    std::cout << "  3. 删除节点时, 标记删除时的时代号\n";
    std::cout << "  4. 当所有线程都离开旧时代后, 回收旧时代的节点\n\n";

    constexpr int num_threads = 4;
    std::atomic<uint64_t> global_epoch{1};
    std::atomic<uint64_t> thread_epochs[num_threads];
    for (int i = 0; i < num_threads; ++i) {
        thread_epochs[i].store(0, std::memory_order_relaxed);
    }

    struct RetiredNode {
        void* node;
        uint64_t epoch;
        RetiredNode* next;
    };

    std::atomic<RetiredNode*> retired_list{nullptr};

    auto enter_critical = [&](int tid) {
        thread_epochs[tid].store(global_epoch.load(std::memory_order_acquire),
                                 std::memory_order_release);
    };

    auto leave_critical = [&](int tid) {
        thread_epochs[tid].store(0, std::memory_order_release);
    };

    auto try_advance_epoch = [&]() {
        uint64_t current = global_epoch.load(std::memory_order_acquire);
        bool can_advance = true;
        for (int i = 0; i < num_threads; ++i) {
            uint64_t te = thread_epochs[i].load(std::memory_order_acquire);
            if (te != 0 && te < current) {
                can_advance = false;
                break;
            }
        }
        if (can_advance) {
            global_epoch.store(current + 1, std::memory_order_release);
        }
    };

    auto can_reclaim = [&](uint64_t node_epoch) {
        uint64_t current = global_epoch.load(std::memory_order_acquire);
        if (current < node_epoch + 2) return false;
        for (int i = 0; i < num_threads; ++i) {
            uint64_t te = thread_epochs[i].load(std::memory_order_acquire);
            if (te != 0 && te <= node_epoch) return false;
        }
        return true;
    };

    std::cout << "Epoch回收流程演示:\n";
    enter_critical(0);
    enter_critical(1);
    std::cout << "  线程0,1进入临界区, epoch="
              << global_epoch.load() << "\n";

    try_advance_epoch();
    std::cout << "  尝试推进epoch (线程0,1仍在旧时代)\n";
    std::cout << "  当前epoch=" << global_epoch.load() << "\n";

    leave_critical(0);
    try_advance_epoch();
    std::cout << "  线程0离开, 再次推进: epoch=" << global_epoch.load() << "\n";

    leave_critical(1);
    try_advance_epoch();
    std::cout << "  线程1离开, 再次推进: epoch=" << global_epoch.load() << "\n";

    std::cout << "\n  节点在epoch 1删除, 当前epoch 3\n";
    std::cout << "  can_reclaim(epoch=1): " << std::boolalpha << can_reclaim(1) << "\n";

    std::cout << "\nEpoch-based回收优缺点:\n";
    std::cout << "  优点: 读取开销极低(仅写thread_epoch), 批量回收高效\n";
    std::cout << "  缺点: 回收有延迟(需等2个epoch), 内存占用可能较高\n";
    std::cout << "  适用: 读多写少的场景\n";
    std::cout << "  代表实现: crossbeam (Rust), Folly (Facebook)\n";
}

void demo_lock_free_design_principles() {
    std::cout << "\n=== demo_lock_free_design_principles ===\n";
    std::cout << "无锁设计原则\n\n";

    std::cout << "原则1: 最小化共享状态\n";
    std::cout << "  共享状态越少, 竞争越少\n";
    std::cout << "  使用线程局部存储减少共享\n";
    std::cout << "  例: 每线程计数器, 最终合并\n\n";

    std::cout << "原则2: 读写分离\n";
    std::cout << "  读操作不应阻塞写操作\n";
    std::cout << "  使用发布模式: 写端release, 读端acquire\n\n";

    std::cout << "原则3: 避免复杂同步\n";
    std::cout << "  同时修改多个变量很难无锁实现\n";
    std::cout << "  考虑将多个变量合并为一个结构体\n";
    std::cout << "  或使用单一原子变量作为协调点\n\n";

    std::cout << "原则4: 帮助机制(Helping)\n";
    std::cout << "  线程A的操作被线程B观察到未完成时\n";
    std::cout << "  线程B可以帮助完成A的操作\n";
    std::cout << "  例: Michael-Scott队列中, 帮助推进tail\n\n";

    std::cout << "原则5: 容忍不一致\n";
    std::cout << "  某些场景可以容忍短暂的不一致\n";
    std::cout << "  例: size()返回近似值\n";
    std::cout << "  例: 遍历时可能看到部分更新\n\n";

    std::cout << "设计决策树:\n";
    std::cout << "  需要并发访问?\n";
    std::cout << "    否 -> 不需要无锁\n";
    std::cout << "    是 -> 读写模式?\n";
    std::cout << "      读多写少 -> RCU或Epoch\n";
    std::cout << "      读写均衡 -> Hazard Pointer\n";
    std::cout << "      写多读少 -> 考虑mutex\n";
    std::cout << "      极低延迟 -> Wait-free\n";
}

int main() {
    std::cout << "无锁设计原则深入\n";

    demo_progress_guarantees();
    demo_hazard_pointer_concept();
    demo_epoch_based_reclamation_concept();
    demo_lock_free_design_principles();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
