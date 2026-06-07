/** @file 01_deep_dive_sync_patterns.cpp @brief 同步模式深入探讨 @description 对应文档: 02-CPP/30-thread-sync
 *  编译命令: g++ -std=c++20 01_deep_dive_sync_patterns.cpp -o 01_deep_dive_sync_patterns
 */

#include <iostream>
#include <thread>
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <vector>
#include <string>

void demo_scoped_lock() {
    std::cout << "\n=== scoped_lock (C++17) ===\n";

    std::mutex mtx1, mtx2;
    int resource1 = 0, resource2 = 0;

    auto safe_update = [&mtx1, &mtx2, &resource1, &resource2](int id) {
        std::scoped_lock lock(mtx1, mtx2);
        resource1 += id;
        resource2 -= id;
        std::cout << "  线程" << id << ": r1=" << resource1 << " r2=" << resource2 << "\n";
    };

    std::vector<std::jthread> threads;
    for (int i = 1; i <= 3; ++i) {
        threads.emplace_back(safe_update, i);
    }

    std::cout << "scoped_lock同时锁定多个互斥量\n";
    std::cout << "使用避免死锁的算法(类似std::lock)\n";
    std::cout << "RAII风格, 构造加锁, 析构解锁\n";
}

void demo_deadlock_avoidance() {
    std::cout << "\n=== 死锁避免 ===\n";

    std::cout << "死锁四个必要条件:\n";
    std::cout << "  1. 互斥: 资源同时只能被一个线程使用\n";
    std::cout << "  2. 持有并等待: 持有资源同时等待其他资源\n";
    std::cout << "  3. 不可抢占: 已获得的资源不能被强制释放\n";
    std::cout << "  4. 循环等待: 线程间形成循环等待关系\n";

    std::cout << "\n避免策略:\n";

    std::cout << "\n1. 固定加锁顺序:\n";
    std::mutex mtx_a, mtx_b;
    auto safe_order = [&mtx_a, &mtx_b]() {
        std::scoped_lock lock(mtx_a, mtx_b);
        std::cout << "  按固定顺序加锁, 避免循环等待\n";
    };
    safe_order();

    std::cout << "\n2. 使用scoped_lock/std::lock:\n";
    std::cout << "  std::scoped_lock(mtx1, mtx2);  // 自动避免死锁\n";
    std::cout << "  std::lock(mtx1, mtx2);         // 同时加锁\n";

    std::cout << "\n3. 限制锁的持有时间:\n";
    std::cout << "  拷贝数据 -> 释放锁 -> 处理数据\n";

    std::cout << "\n4. 避免嵌套锁:\n";
    std::cout << "  如果必须嵌套, 使用scoped_lock\n";

    std::cout << "\n5. 层级锁:\n";
    std::cout << "  给每个锁分配层级号, 只能从低到高加锁\n";
}

void demo_try_lock() {
    std::cout << "\n=== try_lock ===\n";

    std::mutex mtx;

    if (mtx.try_lock()) {
        std::cout << "try_lock成功: 获得锁\n";
        mtx.unlock();
    } else {
        std::cout << "try_lock失败: 锁被占用\n";
    }

    std::cout << "\ntry_lock用途:\n";
    std::cout << "  1. 避免阻塞: 锁被占用时做其他事\n";
    std::cout << "  2. 死锁检测: 定期try_lock检查\n";
    std::cout << "  3. 超时模式: 循环try_lock + sleep\n";

    std::cout << "\nstd::try_lock(多个锁):\n";
    std::mutex m1, m2;
    int result = std::try_lock(m1, m2);
    if (result == -1) {
        std::cout << "  同时获得两个锁\n";
        m1.unlock();
        m2.unlock();
    } else {
        std::cout << "  第" << result << "个锁获取失败\n";
    }
}

void demo_timed_mutex() {
    std::cout << "\n=== 定时互斥量 ===\n";

    std::timed_mutex tmtx;

    if (tmtx.try_lock_for(std::chrono::milliseconds(100))) {
        std::cout << "try_lock_for(100ms)成功\n";
        tmtx.unlock();
    } else {
        std::cout << "try_lock_for(100ms)超时\n";
    }

    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(100);
    if (tmtx.try_lock_until(deadline)) {
        std::cout << "try_lock_until成功\n";
        tmtx.unlock();
    } else {
        std::cout << "try_lock_until超时\n";
    }

    std::cout << "\ntimed_mutex vs mutex:\n";
    std::cout << "  mutex: 只能阻塞等待或try_lock\n";
    std::cout << "  timed_mutex: 支持超时等待\n";
    std::cout << "  recursive_timed_mutex: 可重入 + 超时\n";

    std::cout << "\n使用场景:\n";
    std::cout << "  避免无限等待(如外部系统可能卡住)\n";
    std::cout << "  实现超时重试逻辑\n";
    std::cout << "  优雅关闭(超时后放弃锁)\n";
}

void demo_lock_ordering() {
    std::cout << "\n=== 锁排序策略 ===\n";

    std::cout << "策略1: 地址排序\n";
    std::cout << "  按mutex地址从小到大加锁\n";
    std::cout << "  优点: 通用, 无需手动编号\n";
    std::cout << "  缺点: 可能与代码逻辑不一致\n";

    std::cout << "\n策略2: 层级编号\n";
    std::cout << "  每个锁分配层级号, 只能从低到高加锁\n";
    std::cout << "  运行时检查违规\n";

    std::cout << "\n策略3: 逻辑分组\n";
    std::cout << "  按业务逻辑分组, 同组内固定顺序\n";
    std::cout << "  不同组尽量不交叉加锁\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  数据库: 两阶段锁协议\n";
    std::cout << "  分布式: 全局排序(如资源ID)\n";
    std::cout << "  实时系统: 优先级继承协议\n";
}

int main() {
    std::cout << "========== 同步模式深入探讨 ==========\n";

    demo_scoped_lock();
    demo_deadlock_avoidance();
    demo_try_lock();
    demo_timed_mutex();
    demo_lock_ordering();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
