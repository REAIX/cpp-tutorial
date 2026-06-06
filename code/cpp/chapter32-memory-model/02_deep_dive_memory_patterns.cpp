/**
 * @file 02_deep_dive_memory_patterns.cpp
 * @brief 内存序模式: 发布模式, 标志同步, fence操作, acquire/release链
 * @description 对应文档: 02-CPP/32-内存模型
 */

#include <iostream>
#include <atomic>
#include <thread>
#include <vector>
#include <mutex>

void demo_publication_pattern() {
    std::cout << "\n=== demo_publication_pattern ===\n";
    std::cout << "发布模式(Publication Pattern): 安全地将数据发布给其他线程\n\n";

    struct Config {
        int width;
        int height;
        std::string name;
    };

    std::atomic<Config*> published_config{nullptr};

    auto publisher = [&]() {
        Config* cfg = new Config{1920, 1080, "MainWindow"};
        published_config.store(cfg, std::memory_order_release);
        std::cout << "  发布者: 配置已发布\n";
    };

    auto subscriber = [&]() {
        Config* cfg = nullptr;
        while ((cfg = published_config.load(std::memory_order_acquire)) == nullptr) {}
        std::cout << "  订阅者: 读取配置 " << cfg->name
                  << " " << cfg->width << "x" << cfg->height << "\n";
        std::cout << "  release保证: Config的所有字段对acquire线程可见\n";
    };

    std::thread t1(publisher);
    std::thread t2(subscriber);
    t1.join();
    t2.join();

    delete published_config.load();

    std::cout << "\n发布模式要点:\n";
    std::cout << "  1. 发布者: 先构造对象, 再用release写指针\n";
    std::cout << "  2. 订阅者: 用acquire读指针, 再访问对象\n";
    std::cout << "  3. release/acquire配对保证对象完全构造后才可见\n";
    std::cout << "  4. 常见错误: 用relaxed写指针, 对象可能未完全构造\n";
}

void demo_flag_synchronization() {
    std::cout << "\n=== demo_flag_synchronization ===\n";
    std::cout << "基于标志的同步模式\n\n";

    std::cout << "模式1: 单标志通知\n";
    std::atomic<bool> done{false};
    int result = 0;

    std::thread worker([&]() {
        result = 42 * 2;
        done.store(true, std::memory_order_release);
    });

    while (!done.load(std::memory_order_acquire)) {}
    std::cout << "  主线程: result=" << result << " (通过标志同步)\n";
    worker.join();

    std::cout << "\n模式2: 多阶段标志\n";
    std::atomic<int> phase{0};
    int data1 = 0, data2 = 0, data3 = 0;

    auto stage1 = [&]() {
        data1 = 100;
        phase.store(1, std::memory_order_release);
    };

    auto stage2 = [&]() {
        while (phase.load(std::memory_order_acquire) < 1) {}
        data2 = data1 + 200;
        phase.store(2, std::memory_order_release);
    };

    auto stage3 = [&]() {
        while (phase.load(std::memory_order_acquire) < 2) {}
        data3 = data2 + 300;
        phase.store(3, std::memory_order_release);
    };

    std::thread t1(stage1);
    std::thread t2(stage2);
    std::thread t3(stage3);
    t1.join();
    t2.join();
    t3.join();

    std::cout << "  阶段1: data1=" << data1 << "\n";
    std::cout << "  阶段2: data2=" << data2 << "\n";
    std::cout << "  阶段3: data3=" << data3 << "\n";

    std::cout << "\n模式3: 一次性初始化标志\n";
    std::atomic<bool> initialized{false};
    int expensive_data = 0;

    auto init_if_needed = [&]() {
        if (!initialized.load(std::memory_order_acquire)) {
            expensive_data = 999;
            initialized.store(true, std::memory_order_release);
            std::cout << "  初始化完成\n";
        } else {
            std::cout << "  已初始化, 跳过\n";
        }
    };

    std::thread ta(init_if_needed);
    std::thread tb(init_if_needed);
    ta.join();
    tb.join();

    std::cout << "\n注意: 上述一次性初始化存在竞态!\n";
    std::cout << "  两个线程可能同时进入初始化分支\n";
    std::cout << "  正确做法: 使用std::call_once或atomic的CAS\n";
}

void demo_fence_operations() {
    std::cout << "\n=== demo_fence_operations ===\n";
    std::cout << "Fence (栅栏/内存屏障) 操作\n\n";

    std::cout << "std::atomic_thread_fence: 独立于原子变量的内存屏障\n";
    std::cout << "  atomic_thread_fence(memory_order_acquire)\n";
    std::cout << "  atomic_thread_fence(memory_order_release)\n";
    std::cout << "  atomic_thread_fence(memory_order_acq_rel)\n";
    std::cout << "  atomic_thread_fence(memory_order_seq_cst)\n\n";

    std::cout << "Fence vs 原子操作上的内存序:\n";
    std::cout << "  原子操作上的内存序: 只影响该操作的排序\n";
    std::cout << "  Fence: 影响所有相邻的内存操作\n\n";

    std::atomic<bool> flag{false};
    int data = 0;

    auto fence_writer = [&]() {
        data = 42;
        std::atomic_thread_fence(std::memory_order_release);
        flag.store(true, std::memory_order_relaxed);
    };

    auto fence_reader = [&]() {
        while (!flag.load(std::memory_order_relaxed)) {}
        std::atomic_thread_fence(std::memory_order_acquire);
        std::cout << "  通过fence同步: data=" << data << " (保证为42)\n";
    };

    std::thread t1(fence_writer);
    std::thread t2(fence_reader);
    t1.join();
    t2.join();

    std::cout << "\nFence的语义:\n";
    std::cout << "  release fence: fence之前的写操作不会被重排到fence之后\n";
    std::cout << "  acquire fence: fence之后的读操作不会被重排到fence之前\n";
    std::cout << "  配对条件: release fence之后的原子写 + acquire fence之前的原子读\n\n";

    std::cout << "Fence使用场景:\n";
    std::cout << "  1. 需要对非原子变量建立同步时\n";
    std::cout << "  2. 需要对多个原子变量统一排序时\n";
    std::cout << "  3. 性能敏感代码中减少原子操作的开销\n";
    std::cout << "  4. 实现自定义同步原语时\n\n";

    std::cout << "Fence vs 原子操作内存序的选择:\n";
    std::cout << "  优先使用原子操作上的内存序 (更安全, 更易理解)\n";
    std::cout << "  只在确实需要时使用fence (如实现底层原语)\n";
}

void demo_acquire_release_chain() {
    std::cout << "\n=== demo_acquire_release_chain ===\n";
    std::cout << "Acquire/Release链: 多线程间的传递同步\n\n";

    constexpr int num_threads = 5;
    std::atomic<int> barrier{0};
    std::vector<int> results(num_threads, 0);

    auto chain_worker = [&](int id) {
        while (barrier.load(std::memory_order_acquire) < id) {}
        results[id] = id * 100;
        barrier.store(id + 1, std::memory_order_release);
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(chain_worker, i);
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "链式执行结果:\n";
    for (int i = 0; i < num_threads; ++i) {
        std::cout << "  线程" << i << ": result=" << results[i] << "\n";
    }

    std::cout << "\nAcquire/Release链的典型应用:\n";
    std::cout << "  1. 生产者-消费者链: 数据经过多阶段处理\n";
    std::cout << "  2. 流水线: 每个阶段依赖前一阶段的输出\n";
    std::cout << "  3. 顺序初始化: 模块按依赖顺序初始化\n\n";

    std::cout << "常见陷阱:\n";
    std::cout << "  1. 忘记配对: release写必须与acquire读配对\n";
    std::cout << "  2. 用错方向: release用于写端, acquire用于读端\n";
    std::cout << "  3. 遗漏fence: 需要fence时只用relaxed\n";
    std::cout << "  4. 过度优化: 不必要地使用relaxed\n";

    std::cout << "\n最佳实践:\n";
    std::cout << "  1. 默认使用seq_cst, 有性能需求再优化\n";
    std::cout << "  2. acquire/release配对使用, 不要单独使用\n";
    std::cout << "  3. 用注释标注每个原子操作的同步意图\n";
    std::cout << "  4. 用TSan验证并发正确性\n";
    std::cout << "  5. 在ARM设备上测试relaxed优化是否正确\n";
}

int main() {
    std::cout << "内存序模式深入分析\n";

    demo_publication_pattern();
    demo_flag_synchronization();
    demo_fence_operations();
    demo_acquire_release_chain();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
