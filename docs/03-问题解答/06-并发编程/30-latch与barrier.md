# latch 与 barrier 的区别
> 📖 相关章节：[多线程基础](../../02-CPP/27-多线程基础.md)、[线程同步](../../02-CPP/28-线程同步.md)、[原子操作](../../02-CPP/29-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

### 1. 先抓核心

**latch 是一次性倒计数同步点（等所有人到齐就开跑），barrier 是可重用的多阶段同步点（每轮都要等齐再继续）。一次性等待用 latch，多轮同步用 barrier。**

***

### 2. 核心定义

| | std::latch | std::barrier |
|---|---|---|
| 是什么 | 一次性倒计数同步原语 | 可重用的多阶段同步原语 |
| 计数方向 | 只能递减（count_down） | 自动重置（每轮自动恢复计数） |
| 可重用性 | 一次性（计数到 0 后不可重用） | 可重用（每轮结束后自动重置） |
| C++ 版本 | C++20 | C++20 |

**本质区别**：

```cpp
// latch：一次性倒计数
std::latch done(3);     // 计数器初始为 3
done.count_down();      // 3 → 2
done.count_down();      // 2 → 1
done.count_down();      // 1 → 0，等待的线程被释放
// done 已经"用完"，不能再重置

// barrier：可重用同步点
std::barrier sync(3);   // 需要 3 个线程到达
// 第1轮：3个线程都到达 → 全部释放 → 计数自动重置
// 第2轮：3个线程都到达 → 全部释放 → 计数自动重置
// ...可以无限轮次使用
```

***

### 3. 生活类比

| | std::latch | std::barrier |
|---|---|---|
| 类比 | 考试开考前等所有人到齐（只等一次） | 接力赛每棒交接处（每轮都要等） |
| 说明 | 所有人到齐后开考，考完就散了 | 每一棒的选手都要等前一棒交棒，然后自己跑，再到下一棒交接处等 |
| 关键区别 | 一次性事件 | 重复性事件 |

**具体场景**：

- **latch**：考试开考前，老师等所有学生到齐。30 个学生到齐后，考试开始。考试结束后大家各走各的，不需要再集合。这是一次性的同步。
- **barrier**：接力赛中，每一棒都有一个交接区。第 1 棒选手跑完后在交接区等第 2 棒选手，交接后第 2 棒跑，跑到下一个交接区再等第 3 棒。每一轮都要在交接区等齐，这是可重用的同步。

***

### 4. latch 的用法

```cpp
#include <latch>
#include <thread>
#include <vector>
#include <iostream>

// count_down：递减计数器
// wait：阻塞直到计数器到 0
// arrive_and_wait：递减 + 等待（合二为一）
// try_wait：非阻塞检查计数器是否到 0

int main() {
    const int n = 5;
    std::latch start(n);       // 所有线程等这个信号再开始
    std::latch done(n);        // 主线程等所有线程完成

    std::vector<std::thread> threads;
    for (int i = 0; i < n; ++i) {
        threads.emplace_back([&start, &done, i]() {
            // 每个线程先等待"开始"信号
            start.arrive_and_wait();   // 递减计数 + 等待计数到 0

            std::cout << "Thread " << i << " working...\n";

            // 完成后通知主线程
            done.count_down();         // 只递减，不等待
        });
    }

    // 主线程也可以参与倒计数
    start.arrive_and_wait();     // 主线程也到齐了，所有线程同时开始

    // 等待所有线程完成
    done.wait();
    std::cout << "All threads done!\n";

    for (auto& t : threads) t.join();
    return 0;
}
```

**latch 典型模式：主线程等待子线程完成**

```cpp
#include <latch>
#include <thread>
#include <vector>

void parallelInit(std::latch& done) {
    // 做初始化工作...
    done.count_down();   // 完成后通知
}

int main() {
    const int n = 10;
    std::latch done(n);
    std::vector<std::thread> threads;

    for (int i = 0; i < n; ++i) {
        threads.emplace_back(parallelInit, std::ref(done));
    }

    done.wait();   // 主线程等待所有初始化完成

    // 所有初始化完成后，继续主逻辑
    // ...

    for (auto& t : threads) t.join();
    return 0;
}
```

***

### 5. barrier 的用法

```cpp
#include <barrier>
#include <thread>
#include <vector>
#include <iostream>

// arrive_and_wait：到达并等待所有线程到达
// arrive_and_drop：到达后永久减少参与计数（退出同步）
// arrive：只到达不等待

int main() {
    const int n = 3;
    int iteration = 0;

    // barrier 可以带一个完成函数，每轮所有线程到达后执行
    std::barrier sync(n, [&iteration]() noexcept {
        ++iteration;
        std::cout << "--- Phase " << iteration << " complete ---\n";
    });

    auto worker = [&sync](int id) {
        for (int phase = 0; phase < 3; ++phase) {
            std::cout << "Thread " << id << " phase " << phase << " working\n";
            sync.arrive_and_wait();   // 每轮结束后同步
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < n; ++i) {
        threads.emplace_back(worker, i);
    }

    for (auto& t : threads) t.join();
    return 0;
}
```

**barrier 的 arrive_and_drop：线程退出同步**

```cpp
#include <barrier>
#include <thread>
#include <iostream>

int main() {
    const int n = 4;
    std::barrier sync(n);

    auto worker = [&sync](int id, bool permanent) {
        std::cout << "Thread " << id << " phase 1\n";
        sync.arrive_and_wait();

        if (!permanent) {
            std::cout << "Thread " << id << " exiting\n";
            sync.arrive_and_drop();   // 退出，后续轮次少一个参与者
            return;
        }

        std::cout << "Thread " << id << " phase 2\n";
        sync.arrive_and_wait();       // 剩余线程继续同步

        std::cout << "Thread " << id << " phase 3\n";
        sync.arrive_and_wait();
    };

    std::thread t1(worker, 1, false);   // 临时线程，phase 1 后退出
    std::thread t2(worker, 2, true);    // 永久线程
    std::thread t3(worker, 3, true);
    std::thread t4(worker, 4, true);

    t1.join(); t2.join(); t3.join(); t4.join();
    return 0;
}
```

***

### 6. latch vs barrier 选择指南

```
需要线程同步？
├── 只需要同步一次？
│   ├── 等所有线程完成？→ latch（主线程 wait，子线程 count_down）
│   ├── 等所有线程到齐再开始？→ latch（所有线程 arrive_and_wait）
│   └── 需要完成函数？→ barrier（虽然只用一次，但需要完成函数时）
└── 需要多轮同步？
    ├── 每轮参与线程数相同？→ barrier
    ├── 某些线程会退出？→ barrier + arrive_and_drop
    └── 需要每轮执行额外逻辑？→ barrier + 完成函数
```

**具体场景**：

```cpp
// 场景1：并行初始化，等所有完成后继续 → latch
std::latch initDone(numWorkers);
// 每个 worker: initDone.count_down();
// 主线程: initDone.wait();

// 场景2：并行计算的多轮迭代 → barrier
std::barrier iterSync(numWorkers);
// 每个 worker 每轮: iterSync.arrive_and_wait();

// 场景3：生产者-消费者的分阶段处理 → barrier
std::barrier phase(numWorkers, []() noexcept {
    // 每轮结束后交换缓冲区等
});

// 场景4：线程池中任务分阶段执行 → barrier
std::barrier stage(numThreads);
```

***

### 7. 对比表格

| 特性 | std::latch | std::barrier |
|------|:---:|:---:|
| 可重用性 | 一次性（用完即弃） | 可重用（自动重置） |
| 计数方向 | 只能递减 | 自动重置 |
| 参与者变化 | 不支持 | 支持（arrive_and_drop） |
| 完成函数 | 无 | 支持（每轮结束后执行） |
| 典型操作 | count_down / wait / arrive_and_wait | arrive_and_wait / arrive_and_drop |
| 适用场景 | 一次性同步（初始化、等待完成） | 多轮同步（迭代计算、流水线） |
| C++ 版本 | C++20 | C++20 |
| 头文件 | \<latch\> | \<barrier\> |

***

### 8. 完整示例

```cpp
#include <iostream>
#include <latch>
#include <barrier>
#include <thread>
#include <vector>
#include <atomic>
using namespace std;

void demoLatch() {
    cout << "=== Latch Demo: One-time synchronization ===\n";
    const int n = 4;
    atomic<int> counter{0};
    latch ready(n);
    latch done(n);

    vector<thread> threads;
    for (int i = 0; i < n; ++i) {
        threads.emplace_back([&ready, &done, &counter, i]() {
            cout << "  Worker " << i << " preparing...\n";
            ready.arrive_and_wait();

            ++counter;
            cout << "  Worker " << i << " done (counter=" << counter << ")\n";
            done.count_down();
        });
    }

    ready.wait();   // 等所有线程就绪
    cout << "  All workers ready!\n";

    done.wait();    // 等所有线程完成
    cout << "  All workers finished! counter=" << counter << "\n";

    for (auto& t : threads) t.join();
}

void demoBarrier() {
    cout << "\n=== Barrier Demo: Multi-phase synchronization ===\n";
    const int n = 3;
    int phase = 0;
    barrier sync(n, [&phase]() noexcept {
        ++phase;
        cout << "  --- Phase " << phase << " complete ---\n";
    });

    vector<thread> threads;
    for (int i = 0; i < n; ++i) {
        threads.emplace_back([&sync, i]() {
            for (int p = 0; p < 3; ++p) {
                cout << "  Thread " << i << " working on phase " << p << "\n";
                sync.arrive_and_wait();
            }
        });
    }

    for (auto& t : threads) t.join();
    cout << "  All phases complete!\n";
}

int main() {
    demoLatch();
    demoBarrier();
    return 0;
}
```

***

### 9. 极简总结

**latch = 一次性倒计数同步 | barrier = 可重用多阶段同步 | latch 用 count_down/wait/arrive_and_wait | barrier 用 arrive_and_wait/arrive_and_drop | latch 适合初始化等待 | barrier 适合迭代计算 | barrier 支持完成函数和参与者退出 | 都是 C++20**

***

### 相关阅读

- [什么是条件变量condition-variable](16-什么是条件变量condition-variable.md)
- [什么是信号量Semaphore](17-什么是信号量Semaphore.md)
- [jthread与thread](25-jthread与thread.md)

***