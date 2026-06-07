/**
 * @file 02_deep_dive_thread_pitfalls.cpp
 * @brief 多线程常见陷阱
 * @description 对应文档: 02-CPP/29-thread-basic
 *  @note C 语言中使用 pthread API 实现类似功能, 参见 C 章节 24-进程与线程
 */

#include <iostream>
#include <thread>
#include <vector>
#include <atomic>
#include <chrono>
#include <mutex>

void demo_data_race() {
    std::cout << "\n=== 数据竞争(Data Race) ===\n";

    int counter = 0;
    auto increment = [&counter]() {
        for (int i = 0; i < 100000; ++i) {
            ++counter;
        }
    };

    std::thread t1(increment);
    std::thread t2(increment);
    t1.join();
    t2.join();

    std::cout << "无保护计数: 期望200000, 实际" << counter << "\n";
    std::cout << "原因: ++counter不是原子操作(读-改-写三步)\n";

    std::cout << "\n修复: 使用std::atomic\n";
    std::atomic<int> safe_counter{0};
    auto safe_increment = [&safe_counter]() {
        for (int i = 0; i < 100000; ++i) {
            safe_counter.fetch_add(1, std::memory_order_relaxed);
        }
    };

    std::thread t3(safe_increment);
    std::thread t4(safe_increment);
    t3.join();
    t4.join();
    std::cout << "原子计数: 期望200000, 实际" << safe_counter.load() << "\n";
}

void demo_race_condition() {
    std::cout << "\n=== 竞态条件(Race Condition) ===\n";

    std::cout << "数据竞争 vs 竞态条件:\n";
    std::cout << "  数据竞争: 并发访问同一内存(未定义行为)\n";
    std::cout << "  竞态条件: 时序依赖导致逻辑错误(即使有锁)\n";

    int balance = 1000;
    std::mutex mtx;

    auto withdraw = [&balance, &mtx](int amount) {
        std::lock_guard<std::mutex> lock(mtx);
        if (balance >= amount) {
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
            balance -= amount;
            std::cout << "  取款" << amount << "成功, 余额" << balance << "\n";
        } else {
            std::cout << "  取款" << amount << "失败, 余额不足\n";
        }
    };

    std::thread t1(withdraw, 800);
    std::thread t2(withdraw, 800);
    t1.join();
    t2.join();
    std::cout << "最终余额: " << balance << "\n";
    std::cout << "使用mutex保护后, 不会出现透支\n";

    std::cout << "\n举一反三 - 检查与操作的TOCTOU问题:\n";
    std::cout << "  if (ptr != nullptr) { use(*ptr); }  // 检查与使用之间ptr可能变\n";
    std::cout << "  修复: 将检查和使用放在同一锁内\n";
}

void demo_thread_safety() {
    std::cout << "\n=== 线程安全 ===\n";

    std::cout << "1. const成员函数通常是线程安全的(只读)\n";
    std::cout << "   但如果内部有mutable成员, 仍需注意\n";

    std::cout << "\n2. 标准库容器的线程安全:\n";
    std::cout << "   多读单写: 安全(同一容器的不同元素)\n";
    std::cout << "   多写同一元素: 不安全, 需要同步\n";
    std::cout << "   不同容器: 总是安全\n";

    std::cout << "\n3. 常见线程不安全模式:\n";
    std::cout << "   全局变量/静态变量 + 多线程访问\n";
    std::cout << "   单例模式未加锁\n";
    std::cout << "   std::shared_ptr的引用计数(但本身是原子的)\n";

    std::cout << "\n4. 线程安全设计原则:\n";
    std::cout << "   最小共享: 减少共享数据\n";
    std::cout << "   不可变: 共享不可变数据无需同步\n";
    std::cout << "   线程局部: 使用thread_local\n";
    std::cout << "   消息传递: 用队列代替共享内存\n";
}

void demo_false_sharing() {
    std::cout << "\n=== 伪共享(False Sharing) ===\n";

    std::cout << "缓存行(Cache Line): 通常64字节\n";
    std::cout << "如果两个线程频繁修改同一缓存行中的不同变量,\n";
    std::cout << "会导致缓存行在CPU核心间反复失效(乒乓效应)\n";

    struct alignas(64) AlignedCounter {
        std::atomic<int> value{0};
        char padding[60];
    };

    std::cout << "\n示例: 两个原子变量在同一缓存行\n";
    std::cout << "  struct Counters { atomic<int> a, b; };\n";
    std::cout << "  线程1改a, 线程2改b -> 伪共享!\n";

    std::cout << "\n修复: 对齐到缓存行\n";
    std::cout << "  struct alignas(64) AlignedCounter { atomic<int> value; };\n";
    std::cout << "  每个变量独占一个缓存行\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  高性能计数器: 每线程计数, 最后汇总\n";
    std::cout << "  矩阵运算: 分块处理, 避免同行数据竞争\n";
    std::cout << "  链表: 节点通常在不同缓存行, 天然避免伪共享\n";
}

void demo_oversubscription() {
    std::cout << "\n=== 过度订阅(Oversubscription) ===\n";

    unsigned int cores = std::thread::hardware_concurrency();
    std::cout << "CPU核心数: " << cores << "\n";

    std::cout << "\n过度订阅: 创建的线程数远超CPU核心数\n";
    std::cout << "后果:\n";
    std::cout << "  1. 上下文切换开销增大\n";
    std::cout << "  2. 缓存失效增多\n";
    std::cout << "  3. 调度延迟增加\n";
    std::cout << "  4. 整体性能下降\n";

    std::cout << "\n避免过度订阅:\n";
    std::cout << "  CPU密集型: 线程数 ≈ 核心数\n";
    std::cout << "  I/O密集型: 线程数可以适当超过核心数\n";
    std::cout << "  使用线程池管理线程数量\n";
    std::cout << "  使用std::async让运行时决定并发度\n";

    std::cout << "\n线程数建议:\n";
    std::cout << "  最佳线程数 = 核心数 * (1 + 等待时间/计算时间)\n";
    std::cout << "  纯计算: 等待时间≈0, 最佳≈核心数\n";
    std::cout << "  50%等待: 最佳≈2*核心数\n";
}

void demo_common_mistakes() {
    std::cout << "\n=== 多线程常见错误总结 ===\n";

    std::cout << "1. 忘记join或detach -> std::terminate\n";
    std::cout << "   修复: 使用jthread或RAII守卫\n";

    std::cout << "\n2. 数据竞争 -> 未定义行为\n";
    std::cout << "   修复: 使用mutex或atomic\n";

    std::cout << "\n3. 死锁 -> 程序挂起\n";
    std::cout << "   修复: 固定加锁顺序, 使用scoped_lock\n";

    std::cout << "\n4. 悬垂引用 -> 访问已销毁的变量\n";
    std::cout << "   修复: 确保引用对象的生命周期\n";

    std::cout << "\n5. 异常逃逸 -> std::terminate\n";
    std::cout << "   修复: 在线程函数内捕获所有异常\n";

    std::cout << "\n6. 过度同步 -> 性能下降\n";
    std::cout << "   修复: 减小锁粒度, 使用读写锁\n";

    std::cout << "\n7. 忽略返回值 -> 逻辑错误\n";
    std::cout << "   修复: 检查try_lock等返回值\n";
}

int main() {
    std::cout << "========== 多线程常见陷阱 ==========\n";

    demo_data_race();
    demo_race_condition();
    demo_thread_safety();
    demo_false_sharing();
    demo_oversubscription();
    demo_common_mistakes();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
