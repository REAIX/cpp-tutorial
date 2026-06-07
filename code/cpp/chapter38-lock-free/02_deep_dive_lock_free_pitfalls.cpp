/**
 * @file 02_deep_dive_lock_free_pitfalls.cpp
 * @brief 无锁编程陷阱: ABA解决方案, 内存序, 性能陷阱, 何时避免
 * @description 对应文档: 02-CPP/33-无锁编程
 */

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <cstdint>

void demo_aba_solutions() {
    std::cout << "\n=== demo_aba_solutions ===\n";
    std::cout << "ABA问题的解决方案\n\n";

    std::cout << "方案1: 版本号/计数器\n";
    std::cout << "  将指针与版本号打包, CAS同时检查两者\n\n";

    struct TaggedPointer {
        uintptr_t ptr : 48;
        uintptr_t version : 16;

        bool operator==(const TaggedPointer& other) const {
            return ptr == other.ptr && version == other.version;
        }
    };

    static_assert(sizeof(TaggedPointer) == sizeof(uintptr_t), "TaggedPointer大小必须等于指针大小");

    std::atomic<TaggedPointer> tagged_head{TaggedPointer{0, 0}};

    auto tagged_push = [&](uintptr_t node_ptr) {
        TaggedPointer old_head = tagged_head.load(std::memory_order_acquire);
        TaggedPointer new_head;
        new_head.ptr = node_ptr;
        new_head.version = old_head.version + 1;
        while (!tagged_head.compare_exchange_weak(
            old_head, new_head,
            std::memory_order_release,
            std::memory_order_acquire)) {
            new_head.version = old_head.version + 1;
        }
    };

    tagged_push(0x1000);
    tagged_push(0x2000);
    TaggedPointer current = tagged_head.load();
    std::cout << "  当前head: ptr=0x" << std::hex << current.ptr
              << ", version=" << std::dec << current.version << "\n";
    std::cout << "  版本号确保: 即使指针值相同, 版本不同CAS也会失败\n\n";

    std::cout << "方案2: Hazard Pointer (见前一章)\n";
    std::cout << "  通过延迟释放避免ABA\n";
    std::cout << "  节点不会被重用, 直到确认无线程持有引用\n\n";

    std::cout << "方案3: 双宽度CAS (DCAS)\n";
    std::cout << "  在支持128位CAS的平台上, 同时CAS指针和计数器\n";
    std::cout << "  x86-64: CMPXCHG16B指令\n";
    std::cout << "  ARM64: LDXP/STXP (LD/ST exclusive pair)\n\n";

    std::cout << "方案4: 带引用计数的指针\n";
    std::cout << "  每个节点维护外部引用计数和内部引用计数\n";
    std::cout << "  当两者之和为零时才能释放\n";
    std::cout << "  Folly的AtomicUnorderedMap使用此方案\n";

    std::cout << "\n方案选择建议:\n";
    std::cout << "  简单场景: 版本号 (最易实现)\n";
    std::cout << "  生产环境: Hazard Pointer或Epoch\n";
    std::cout << "  高性能场景: DCAS (平台相关)\n";
}

void demo_memory_ordering_for_lock_free() {
    std::cout << "\n=== demo_memory_ordering_for_lock_free ===\n";
    std::cout << "无锁数据结构的内存序选择\n\n";

    std::cout << "1. 栈的push操作:\n";
    std::cout << "   新节点的所有字段必须在CAS之前写入\n";
    std::cout << "   CAS的release语义确保: 节点字段对其他线程可见\n";
    std::cout << "   如果用relaxed CAS, 节点字段可能对pop线程不可见!\n\n";

    std::cout << "2. 栈的pop操作:\n";
    std::cout << "   CAS的acquire语义确保: 读取到节点后, 节点字段可见\n";
    std::cout << "   读取next指针需要与push的release配对\n\n";

    std::cout << "3. 队列的enqueue:\n";
    std::cout << "   设置next的CAS需要release (新节点数据可见)\n";
    std::cout << "   推进tail的CAS可以用relaxed (只是优化)\n\n";

    std::cout << "4. 队列的dequeue:\n";
    std::cout << "   CAS推进head需要acquire (读取节点数据)\n";
    std::cout << "   读取data可以在CAS之后 (由acquire保证)\n\n";

    std::cout << "常见错误:\n";
    std::cout << "  错误1: 所有操作都用relaxed\n";
    std::cout << "    结果: 数据可能对其他线程不可见, 读取到未初始化的值\n\n";
    std::cout << "  错误2: 所有操作都用seq_cst\n";
    std::cout << "    结果: 正确但性能差, 尤其在ARM上\n\n";
    std::cout << "  错误3: CAS的failure内存序太强\n";
    std::cout << "    结果: 不必要的内存屏障, 影响性能\n";
    std::cout << "    建议: failure用acquire或relaxed, 不要用acq_rel\n\n";

    std::cout << "内存序选择原则:\n";
    std::cout << "  1. 发布数据: release写 + acquire读\n";
    std::cout << "  2. 仅修改指针: relaxed可能足够\n";
    std::cout << "  3. 不确定时: 用seq_cst (安全但慢)\n";
    std::cout << "  4. 用TSan验证: 确保无数据竞争\n";
    std::cout << "  5. 在ARM上测试: x86可能掩盖内存序bug\n";
}

void demo_performance_pitfalls() {
    std::cout << "\n=== demo_performance_pitfalls ===\n";
    std::cout << "无锁编程的性能陷阱\n\n";

    std::cout << "陷阱1: 伪共享(False Sharing)\n";
    std::cout << "  同一缓存行上的不同原子变量, 相互导致缓存失效\n";
    std::cout << "  解决: 对齐到缓存行 (alignas(64))\n\n";

    struct alignas(64) PaddedCounter {
        std::atomic<int> value{0};
        char padding[60];
    };

    PaddedCounter counters[4];
    auto padded_worker = [&](int id) {
        for (int i = 0; i < 1000000; ++i) {
            counters[id].value.fetch_add(1, std::memory_order_relaxed);
        }
    };

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(padded_worker, i);
    }
    for (auto& t : threads) {
        t.join();
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto padded_us = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    std::cout << "  对齐后4线程递增: " << padded_us << " us\n";
    std::cout << "  如果不对齐, 性能可能差5-10倍!\n\n";

    std::cout << "陷阱2: 高竞争下的CAS风暴\n";
    std::cout << "  多线程同时CAS同一变量, 大量失败重试\n";
    std::cout << "  浪费CPU带宽, 吞吐量反而下降\n";
    std::cout << "  解决: 分片(每线程独立计数), 退避策略\n\n";

    std::cout << "陷阱3: 内存回收开销\n";
    std::cout << "  Hazard Pointer: 每次访问需更新, O(T*H)检查\n";
    std::cout << "  Epoch: 需要线程协作, 可能延迟回收\n";
    std::cout << "  引用计数: 原子操作开销大\n\n";

    std::cout << "陷阱4: 过度优化\n";
    std::cout << "  为了relaxed的微小性能提升, 引入难以调试的bug\n";
    std::cout << "  实际性能瓶颈往往不在内存序\n";
    std::cout << "  建议: 先用seq_cst, 确认瓶颈后再优化\n\n";

    std::cout << "陷阱5: 忽略编译器优化\n";
    std::cout << "  编译器可能优化掉看似'多余'的原子操作\n";
    std::cout << "  volatile不能替代atomic\n";
    std::cout << "  data race是UB, 编译器可以任意假设\n";

    std::cout << "\n性能优化检查清单:\n";
    std::cout << "  [ ] 是否存在伪共享?\n";
    std::cout << "  [ ] CAS竞争是否过高?\n";
    std::cout << "  [ ] 内存序是否过强?\n";
    std::cout << "  [ ] 内存回收开销是否合理?\n";
    std::cout << "  [ ] 是否有更简单的替代方案?\n";
}

void demo_when_to_avoid_lock_free() {
    std::cout << "\n=== demo_when_to_avoid_lock_free ===\n";
    std::cout << "何时应该避免无锁编程\n\n";

    std::cout << "应该使用mutex的场景:\n\n";

    std::cout << "1. 临界区较长 (>100条指令)\n";
    std::cout << "   自旋等待的CPU消耗 > 上下文切换开销\n";
    std::cout << "   mutex会让出CPU, 更高效\n\n";

    std::cout << "2. 低竞争场景\n";
    std::cout << "   mutex的开销很小 (fast path只需一次原子操作)\n";
    std::cout << "   无锁的复杂性不值得\n\n";

    std::cout << "3. 需要同时修改多个变量\n";
    std::cout << "   无锁难以保证多个变量的一致性\n";
    std::cout << "   mutex天然保证互斥\n\n";

    std::cout << "4. 团队维护性\n";
    std::cout << "   无锁代码难以理解和维护\n";
    std::cout << "   需要专门的并发专家review\n\n";

    std::cout << "5. 调试困难\n";
    std::cout << "   数据竞争的bug难以重现\n";
    std::cout << "   mutex的bug更容易定位\n\n";

    std::cout << "应该使用无锁的场景:\n\n";

    std::cout << "1. 极低延迟要求 (纳秒级)\n";
    std::cout << "   高频交易, 实时系统\n\n";

    std::cout << "2. 优先级反转不可接受\n";
    std::cout << "   实时系统, 嵌入式系统\n\n";

    std::cout << "3. 读多写少\n";
    std::cout << "   RCU模式, 读写锁的替代\n\n";

    std::cout << "4. 信号处理程序中\n";
    std::cout << "   信号处理程序不能调用mutex\n";
    std::cout << "   只能使用atomic操作\n\n";

    std::cout << "5. 已有成熟实现\n";
    std::cout << "   Folly, libcds, boost::lockfree\n";
    std::cout << "   不要自己实现, 用经过验证的库\n";

    std::cout << "\n实用建议:\n";
    std::cout << "  1. 先用mutex, 有性能问题再考虑无锁\n";
    std::cout << "  2. 用性能测试证明mutex是瓶颈\n";
    std::cout << "  3. 优先使用成熟的无锁库\n";
    std::cout << "  4. 自实现时必须有充分的测试和review\n";
    std::cout << "  5. 使用TSan, Helgrind等工具验证\n";
}

int main() {
    std::cout << "无锁编程陷阱深入分析\n";

    demo_aba_solutions();
    demo_memory_ordering_for_lock_free();
    demo_performance_pitfalls();
    demo_when_to_avoid_lock_free();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
