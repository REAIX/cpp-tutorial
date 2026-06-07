/**
 * @file 01_deep_dive_memory_model.cpp
 * @brief C++内存模型深入: 数据竞争, 虚无值, 平台差异
 * @description 对应文档: 02-CPP/32-内存模型
 */

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>
#include <cstdint>

void demo_data_race_definition() {
    std::cout << "\n=== demo_data_race_definition ===\n";
    std::cout << "数据竞争(Data Race)定义:\n";
    std::cout << "  两个线程同时访问同一内存位置, 至少一个是写入, 且无同步关系\n";
    std::cout << "  数据竞争 = 未定义行为(UB)!\n\n";

    std::cout << "示例: 非原子变量的并发读写 (数据竞争!)\n";
    int shared = 0;

    auto writer = [&]() {
        for (int i = 0; i < 1000; ++i) {
            shared = i;
        }
    };

    auto reader = [&]() {
        for (int i = 0; i < 1000; ++i) {
            int val = shared;
            (void)val;
        }
    };

    std::thread t1(writer);
    std::thread t2(reader);
    t1.join();
    t2.join();

    std::cout << "  上述代码存在数据竞争, 是未定义行为!\n";
    std::cout << "  可能的结果: 崩溃, 数据损坏, 读取到部分写入的值\n\n";

    std::cout << "修复方法1: 使用atomic\n";
    std::atomic<int> safe_shared{0};
    auto safe_writer = [&]() {
        for (int i = 0; i < 1000; ++i) {
            safe_shared.store(i, std::memory_order_relaxed);
        }
    };
    auto safe_reader = [&]() {
        for (int i = 0; i < 1000; ++i) {
            int val = safe_shared.load(std::memory_order_relaxed);
            (void)val;
        }
    };
    std::thread t3(safe_writer);
    std::thread t4(safe_reader);
    t3.join();
    t4.join();
    std::cout << "  atomic保证无数据竞争\n\n";

    std::cout << "修复方法2: 使用mutex\n";
    std::mutex mtx;
    int mutex_shared = 0;
    auto mutex_writer = [&]() {
        for (int i = 0; i < 1000; ++i) {
            std::lock_guard<std::mutex> lock(mtx);
            mutex_shared = i;
        }
    };
    auto mutex_reader = [&]() {
        for (int i = 0; i < 1000; ++i) {
            std::lock_guard<std::mutex> lock(mtx);
            int val = mutex_shared;
            (void)val;
        }
    };
    std::thread t5(mutex_writer);
    std::thread t6(mutex_reader);
    t5.join();
    t6.join();
    std::cout << "  mutex保证互斥访问, 无数据竞争\n";

    std::cout << "\n常见数据竞争场景:\n";
    std::cout << "  1. 全局/静态变量的并发读写\n";
    std::cout << "  2. 共享容器的并发修改 (如std::vector)\n";
    std::cout << "  3. 对象的并发构造与读取\n";
    std::cout << "  4. 引用计数未用atomic\n";
}

void demo_out_of_thin_air() {
    std::cout << "\n=== demo_out_of_thin_air ===\n";
    std::cout << "Out-of-thin-air (虚无值)问题:\n";
    std::cout << "  某些内存序组合可能导致循环依赖, 产生'凭空出现'的值\n";
    std::cout << "  C++11标准未完全解决此问题, C++17进一步澄清\n\n";

    std::cout << "经典例子 (理论性, 实际编译器不会这样):\n";
    std::cout << "  线程1: r1 = x.load(relaxed); y.store(r1, relaxed);\n";
    std::cout << "  线程2: r2 = y.load(relaxed); x.store(r2, relaxed);\n";
    std::cout << "  如果x=y=0, 可能r1=42, r2=42 (凭空出现!)\n\n";

    std::cout << "C++标准的规定:\n";
    std::cout << "  1. 禁止out-of-thin-air值\n";
    std::cout << "  2. 编译器必须确保不会产生循环依赖的推测值\n";
    std::cout << "  3. 实际主流编译器(MSVC/GCC/Clang)不会产生此问题\n\n";

    std::cout << "避免虚无值的建议:\n";
    std::cout << "  1. 避免纯relaxed的循环依赖\n";
    std::cout << "  2. 至少在一侧使用acquire/release\n";
    std::cout << "  3. 对需要同步的数据使用seq_cst\n";
    std::cout << "  4. 使用TSan等工具检测\n";

    std::atomic<int> x{0}, y{0};
    int r1 = 0, r2 = 0;

    auto t1_func = [&]() {
        r1 = x.load(std::memory_order_relaxed);
        y.store(r1, std::memory_order_relaxed);
    };

    auto t2_func = [&]() {
        r2 = y.load(std::memory_order_relaxed);
        x.store(r2, std::memory_order_relaxed);
    };

    std::thread t1(t1_func);
    std::thread t2(t2_func);
    t1.join();
    t2.join();

    std::cout << "\n实际运行结果: r1=" << r1 << ", r2=" << r2 << "\n";
    std::cout << "(正常编译器不会产生虚无值, 但理论上relaxed序不排除)\n";
}

void demo_platform_differences() {
    std::cout << "\n=== demo_platform_differences ===\n";
    std::cout << "不同CPU架构的内存模型差异\n\n";

    std::cout << "x86/x86-64 (TSO - Total Store Order):\n";
    std::cout << "  特点: 强内存模型, 写操作不会重排\n";
    std::cout << "  acquire: 几乎免费 (普通load即可)\n";
    std::cout << "  release: 几乎免费 (普通store即可)\n";
    std::cout << "  seq_cst: 需要MFENCE或locked指令\n";
    std::cout << "  relaxed: 与seq_cst差别不大\n\n";

    std::cout << "ARM/ARM64 (Weakly Ordered):\n";
    std::cout << "  特点: 弱内存模型, 读写都可能重排\n";
    std::cout << "  acquire: 需要LDAR指令\n";
    std::cout << "  release: 需要STLR指令\n";
    std::cout << "  seq_cst: 需要DMB + LDAR/STLR\n";
    std::cout << "  relaxed: 普通LDR/STR, 性能优势明显\n\n";

    std::cout << "POWER/SPARC等:\n";
    std::cout << "  更弱的内存模型, 需要更重的同步指令\n\n";

    std::cout << "实际影响:\n";
    std::cout << "  x86上: relaxed vs seq_cst 性能差异小\n";
    std::cout << "  ARM上: relaxed vs seq_cst 性能差异大\n";
    std::cout << "  因此: 在ARM上优化内存序更有价值\n";
    std::cout << "  但: 优化必须基于正确性分析, 不能盲目\n";

    std::cout << "\n各内存序在不同架构的指令映射:\n";
    std::cout << "  ┌─────────────┬──────────────┬──────────────┐\n";
    std::cout << "  │ 操作        │ x86-64       │ ARM64        │\n";
    std::cout << "  ├─────────────┼──────────────┼──────────────┤\n";
    std::cout << "  │ load relaxed│ MOV          │ LDR          │\n";
    std::cout << "  │ load acquire│ MOV          │ LDAR         │\n";
    std::cout << "  │ load seq_cst│ MOV+MFENCE   │ LDAR+DMB     │\n";
    std::cout << "  │ store relaxed│ MOV         │ STR          │\n";
    std::cout << "  │ store release│ MOV         │ STLR         │\n";
    std::cout << "  │ store seq_cst│ MFENCE+MOV  │ DMB+STLR     │\n";
    std::cout << "  │ RMW seq_cst  │ LOCK CMPXCHG│ LDAXR+STLXR  │\n";
    std::cout << "  └─────────────┴──────────────┴──────────────┘\n";
}

void demo_atomic_operations_types() {
    std::cout << "\n=== demo_atomic_operations_types ===\n";
    std::cout << "原子操作的分类与保证\n\n";

    std::cout << "1. 原子读(Read): load\n";
    std::atomic<int> a{10};
    int val = a.load(std::memory_order_acquire);
    std::cout << "  load: " << val << "\n";

    std::cout << "\n2. 原子写(Write): store\n";
    a.store(20, std::memory_order_release);
    std::cout << "  store: " << a.load() << "\n";

    std::cout << "\n3. 读-修改-写(RMW): fetch_add, fetch_sub, exchange, CAS\n";
    int old = a.fetch_add(5, std::memory_order_seq_cst);
    std::cout << "  fetch_add(5): 旧值=" << old << ", 新值=" << a.load() << "\n";

    old = a.fetch_sub(3, std::memory_order_seq_cst);
    std::cout << "  fetch_sub(3): 旧值=" << old << ", 新值=" << a.load() << "\n";

    old = a.exchange(100, std::memory_order_acq_rel);
    std::cout << "  exchange(100): 旧值=" << old << ", 新值=" << a.load() << "\n";

    std::cout << "\n4. CAS (Compare-And-Swap): compare_exchange_weak/strong\n";
    int expected = 100;
    bool success = a.compare_exchange_strong(expected, 200,
        std::memory_order_acq_rel, std::memory_order_acquire);
    std::cout << "  CAS(100->200): 成功=" << std::boolalpha << success
              << ", 值=" << a.load() << "\n";

    expected = 999;
    success = a.compare_exchange_strong(expected, 300,
        std::memory_order_acq_rel, std::memory_order_acquire);
    std::cout << "  CAS(999->300): 成功=" << success
              << ", expected=" << expected << " (被更新为当前值)\n";

    std::cout << "\n5. weak vs strong CAS:\n";
    std::cout << "  weak: 可能spurious failure (即使值相等也返回false)\n";
    std::cout << "  strong: 仅当值不等时返回false\n";
    std::cout << "  建议: 循环中使用weak(减少一次比较), 非循环用strong\n";

    std::cout << "\n6. 特殊原子类型: atomic_flag\n";
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    bool was_set = flag.test_and_set(std::memory_order_acquire);
    std::cout << "  test_and_set: 之前是否设置=" << std::boolalpha << was_set << "\n";
    flag.clear(std::memory_order_release);
    std::cout << "  clear后, test=" << flag.test_and_set() << "\n";
    flag.clear();
}

void demo_memory_model_rules() {
    std::cout << "\n=== demo_memory_model_rules ===\n";
    std::cout << "C++内存模型的核心规则\n\n";

    std::cout << "规则1: 无数据竞争 = 无UB\n";
    std::cout << "  只要不发生数据竞争, 程序就有明确语义\n\n";

    std::cout << "规则2: 原子操作保证原子性\n";
    std::cout << "  读取要么看到修改前的值, 要么看到修改后的值\n";
    std::cout << "  不会看到'部分修改'的值\n\n";

    std::cout << "规则3: 修改顺序(Modification Order)\n";
    std::cout << "  对同一原子变量的所有修改, 存在全局一致的顺序\n";
    std::cout << "  所有线程对同一原子变量看到的修改顺序一致\n\n";

    std::atomic<int> counter{0};
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&counter, i]() {
            for (int j = 0; j < 5; ++j) {
                int old = counter.fetch_add(1, std::memory_order_relaxed);
                std::cout << "  线程" << i << ": fetch_add, 旧值=" << old << "\n";
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    std::cout << "  最终值: " << counter.load() << " (修改顺序保证一致性)\n\n";

    std::cout << "规则4: happens-before是可传递的\n";
    std::cout << "  A happens-before B, B happens-before C => A happens-before C\n\n";

    std::cout << "规则5: seq_cst提供全局顺序\n";
    std::cout << "  所有seq_cst操作存在一个全局一致的总顺序\n";
    std::cout << "  这个顺序与happens-before一致\n";
}

int main() {
    std::cout << "C++内存模型深入分析\n";

    demo_data_race_definition();
    demo_out_of_thin_air();
    demo_platform_differences();
    demo_atomic_operations_types();
    demo_memory_model_rules();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
