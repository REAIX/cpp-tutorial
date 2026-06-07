# 什么是自旋锁 Spin Lock
> 📖 相关章节：[多线程基础](../../02-CPP/27-多线程基础.md)、[线程同步](../../02-CPP/28-线程同步.md)、[原子操作](../../02-CPP/29-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

> 自旋锁是最原始的互斥原语——等不到锁就不停转，直到锁到手为止。

***

### 1. 要义概览

自旋锁（Spin Lock）是一种忙等待（Busy-Wait）锁，当线程无法获取锁时不进入睡眠，而是循环检测锁状态，适用于临界区极短、持锁时间极少的场景。

***

### 2. 忙等待的本质

自旋锁的核心是忙等待：线程在获取锁失败后不放弃 CPU，而是反复尝试（自旋），直到锁被释放。

```
线程A (持锁)          线程B (等锁)
┌──────────┐         ┌──────────────┐
│ lock()   │         │ lock()       │
│ 临界区   │         │  → 失败      │
│ ...      │         │  → 自旋...   │
│ ...      │         │  → 自旋...   │
│ unlock() │         │  → 成功!     │
└──────────┘         │  临界区      │
                     │  unlock()    │
                     └──────────────┘
```

```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <vector>

class NaiveSpinLock {
    std::atomic<bool> locked_{false};
public:
    void lock() {
        while (locked_.exchange(true, std::memory_order_acquire)) {
        }
    }
    void unlock() {
        locked_.store(false, std::memory_order_release);
    }
};

int main() {
    NaiveSpinLock spin;
    int counter = 0;

    auto worker = [&]() {
        for (int i = 0; i < 100000; ++i) {
            spin.lock();
            ++counter;
            spin.unlock();
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(worker);
    }
    for (auto& t : threads) {
        t.join();
    }

    std::cout << "counter = " << counter << " (期望 400000)\n";
    return 0;
}
```

> 这个朴素实现虽然正确，但在高竞争下会浪费大量 CPU 周期，且对缓存不友好。

***

### 3. 自旋锁 vs 互斥锁

| 特性 | 自旋锁 | 互斥锁 (Mutex) |
|------|--------|---------------|
| 等待方式 | 忙等待（循环检测） | 睡眠等待（内核调度） |
| CPU 占用 | 等待时 100% 占用 | 等待时释放 CPU |
| 上下文切换 | 无 | 有（两次切换约数 μs） |
| 适用场景 | 临界区极短（< 上下文切换开销） | 临界区较长 |
| 实现层级 | 纯用户态 | 涉及内核态 |
| 公平性 | 无保证（可能饿死） | 通常有保证 |
| 可中断性 | 不可中断（不响应信号） | 可中断 |

```cpp
#include <iostream>
#include <mutex>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>

class SpinLock {
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
public:
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
        }
    }
    void unlock() {
        flag_.clear(std::memory_order_release);
    }
};

template<typename LockType>
void benchmark(const char* name, int iterations) {
    LockType lock;
    int counter = 0;

    auto start = std::chrono::high_resolution_clock::now();

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back([&]() {
            for (int j = 0; j < iterations; ++j) {
                lock.lock();
                ++counter;
                lock.unlock();
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << name << ": " << ms << " ms, counter=" << counter << "\n";
}

int main() {
    const int iters = 1000000;
    benchmark<SpinLock>("SpinLock", iters);
    benchmark<std::mutex>("std::mutex", iters);
    return 0;
}
```

> 当临界区只有 `++counter` 一条指令时，自旋锁通常更快；但如果临界区包含 I/O 或长时间计算，互斥锁更优。

***

### 4. C++ 标准实现：atomic_flag

C++11 提供了 `std::atomic_flag`，是标准保证 lock-free 的原子类型，是构建自旋锁的基础。

```cpp
#include <atomic>

class SpinLock {
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
public:
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
        }
    }

    bool try_lock() {
        return !flag_.test_and_set(std::memory_order_acquire);
    }

    void unlock() {
        flag_.clear(std::memory_order_release);
    }
};
```

| atomic_flag 操作 | 说明 | 内存序 |
|-----------------|------|--------|
| `test_and_set` | 原子地设为 true 并返回旧值 | acquire（lock 时） |
| `clear` | 原子地设为 false | release（unlock 时） |
| `ATOMIC_FLAG_INIT` | 静态初始化为 false | — |

**C++20 新增 `test` 方法**：

```cpp
#include <atomic>
#include <thread>

class SpinLock20 {
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
public:
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
            while (flag_.test(std::memory_order_relaxed)) {
                std::this_thread::yield();
            }
        }
    }

    void unlock() {
        flag_.clear(std::memory_order_release);
    }
};
```

> C++20 的 `atomic_flag::test()` 允许在不修改原子变量的情况下读取状态，配合双层循环减少总线流量。

***

### 5. 指数退避策略

朴素自旋锁在锁竞争激烈时会导致严重的缓存行争用（Cache Line Bouncing）。指数退避在自旋失败后逐步增加等待间隔，减少总线流量。

```
自旋次数与退避:
尝试 1:  无等待
尝试 2:  pause × 1
尝试 3:  pause × 2
尝试 4:  pause × 4
...
尝试 N:  pause × 2^(N-1)  (上限 max_backoff)
```

```cpp
#include <atomic>
#include <thread>

class BackoffSpinLock {
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;

    static void cpu_relax() {
#if defined(_MSC_VER)
        _mm_pause();
#elif defined(__x86_64__) || defined(__i386__)
        __asm__ __volatile__("pause" ::: "memory");
#elif defined(__aarch64__)
        __asm__ __volatile__("yield" ::: "memory");
#else
        std::this_thread::yield();
#endif
    }

public:
    void lock() {
        int backoff = 1;
        const int max_backoff = 64;

        while (flag_.test_and_set(std::memory_order_acquire)) {
            for (int i = 0; i < backoff; ++i) {
                cpu_relax();
            }
            if (backoff < max_backoff) {
                backoff *= 2;
            }
        }
    }

    void unlock() {
        flag_.clear(std::memory_order_release);
    }
};
```

| 策略 | CPU 占用 | 总线流量 | 延迟 | 适用场景 |
|------|---------|---------|------|---------|
| 朴素自旋 | 最高 | 最高 | 最低 | 无竞争 |
| 固定退避 | 中 | 中 | 中 | 一般 |
| 指数退避 | 低 | 低 | 中高 | 高竞争 |
| yield 自旋 | 最低 | 最低 | 最高 | 长临界区 |

> `PAUSE` 指令（x86）不仅降低 CPU 功耗，还提示处理器这是自旋等待，改善内存序检测性能。

***

### 6. Ticket Lock——公平自旋锁

朴素自旋锁不公平，后到的线程可能先获取锁（饥饿问题）。Ticket Lock 模仿银行叫号，保证先到先服务。

```
Ticket Lock 原理:
now_serving = 0, next_ticket = 0

线程A 取号: ticket=0, now_serving=0 → 立即获得锁
线程B 取号: ticket=1, now_serving=0 → 等待
线程C 取号: ticket=2, now_serving=0 → 等待

线程A 释放: now_serving=1 → 线程B 获得
线程B 释放: now_serving=2 → 线程C 获得
```

```cpp
#include <atomic>
#include <thread>

class TicketLock {
    std::atomic<unsigned int> now_serving_{0};
    std::atomic<unsigned int> next_ticket_{0};

    static void cpu_relax() {
#if defined(_MSC_VER)
        _mm_pause();
#elif defined(__x86_64__) || defined(__i386__)
        __asm__ __volatile__("pause" ::: "memory");
#elif defined(__aarch64__)
        __asm__ __volatile__("yield" ::: "memory");
#else
        std::this_thread::yield();
#endif
    }

public:
    unsigned int lock() {
        unsigned int my_ticket = next_ticket_.fetch_add(1, std::memory_order_acquire);
        while (now_serving_.load(std::memory_order_acquire) != my_ticket) {
            cpu_relax();
        }
        return my_ticket;
    }

    void unlock() {
        unsigned int serving = now_serving_.load(std::memory_order_relaxed);
        now_serving_.store(serving + 1, std::memory_order_release);
    }
};
```

| 特性 | 朴素自旋锁 | Ticket Lock |
|------|-----------|-------------|
| 公平性 | 不公平 | FIFO 公平 |
| 饥饿 | 可能 | 不会 |
| 缓存争用 | 高（同一原子变量） | 中（释放时才通知） |
| 额外开销 | 无 | 多一个原子变量 |
| 适用 | 低竞争 | 需要公平性 |

> Ticket Lock 的 `now_serving_` 在释放时才修改，等待线程各自读取同一缓存行，但只在释放时产生一次缓存失效。

***

### 7. MCS Lock——可扩展自旋锁

MCS Lock（John Mellor-Crummey & Michael Scott）通过链表结构让每个等待线程在自己的本地变量上自旋，彻底消除缓存行争用。

```
MCS Lock 结构:

tail → [Thread C] → [Thread B] → [Thread A] (持锁)
        next=false    next=false    next=null
        locked=true   locked=true   locked=false

Thread A 释放:
  设置 A.next->locked = false
  → Thread B 在自己的 locked 上检测到变化，获得锁
```

```cpp
#include <atomic>
#include <thread>

class MCSLock {
    struct Node {
        std::atomic<Node*> next_{nullptr};
        std::atomic<bool> locked_{false};
    };

    std::atomic<Node*> tail_{nullptr};

public:
    MCSLock() = default;

    Node* lock(Node& my_node) {
        my_node.next_.store(nullptr, std::memory_order_relaxed);
        my_node.locked_.store(true, std::memory_order_relaxed);

        Node* prev = tail_.exchange(&my_node, std::memory_order_acquire);

        if (prev != nullptr) {
            prev->next_.store(&my_node, std::memory_order_release);
            while (my_node.locked_.load(std::memory_order_acquire)) {
#if defined(_MSC_VER)
                _mm_pause();
#elif defined(__x86_64__) || defined(__i386__)
                __asm__ __volatile__("pause" ::: "memory");
#elif defined(__aarch64__)
                __asm__ __volatile__("yield" ::: "memory");
#else
                std::this_thread::yield();
#endif
            }
        }
        return prev;
    }

    void unlock(Node& my_node) {
        Node* next = my_node.next_.load(std::memory_order_acquire);

        if (next == nullptr) {
            Node* expected = &my_node;
            if (tail_.compare_exchange_strong(expected, nullptr,
                                               std::memory_order_release,
                                               std::memory_order_relaxed)) {
                return;
            }
            while ((next = my_node.next_.load(std::memory_order_acquire)) == nullptr) {
#if defined(_MSC_VER)
                _mm_pause();
#elif defined(__x86_64__) || defined(__i386__)
                __asm__ __volatile__("pause" ::: "memory");
#elif defined(__aarch64__)
                __asm__ __volatile__("yield" ::: "memory");
#else
                std::this_thread::yield();
#endif
            }
        }

        next->locked_.store(false, std::memory_order_release);
    }
};
```

| 特性 | Ticket Lock | MCS Lock |
|------|-----------|----------|
| 缓存争用 | 中（共享 now_serving） | 极低（各自自旋本地变量） |
| 可扩展性 | 中等 | 极好 |
| 实现复杂度 | 低 | 高 |
| 内存开销 | 2 个原子变量 | 每线程一个 Node |
| 适用核心数 | < 32 | > 32 |

> MCS Lock 在 NUMA 系统上优势更明显，因为每个线程只在自己的缓存行上自旋。

***

### 8. 内核自旋锁

操作系统内核中的自旋锁有特殊约束：持锁期间不能睡眠、不能被抢占、不能触发调度。

| 约束 | 原因 |
|------|------|
| 不能睡眠 | 睡眠会触发调度器，若另一线程也请求同一锁则死锁 |
| 不能被抢占 | 防止持锁线程被换出，其他 CPU 无限自旋 |
| 禁用中断（部分场景） | 中断处理程序可能请求同一锁，导致死锁 |
| 尽量短 | 长时间持锁浪费其他 CPU 时间 |

```cpp
#include <iostream>

int main() {
    std::cout << "Linux 内核自旋锁 API:\n";
    std::cout << "  spin_lock(&lock)        / spin_unlock(&lock)\n";
    std::cout << "  spin_lock_irq(&lock)    / spin_unlock_irq(&lock)\n";
    std::cout << "  spin_lock_irqsave(&lock, flags) / spin_unlock_irqrestore(&lock, flags)\n";
    std::cout << "  spin_lock_bh(&lock)     / spin_unlock_bh(&lock)\n";
    std::cout << "\n";
    std::cout << "Windows 内核自旋锁 API:\n";
    std::cout << "  KeAcquireSpinLock(&lock, &irql) / KeReleaseSpinLock(&lock, irql)\n";
    std::cout << "  KeAcquireInStackQueuedSpinLock  / KeReleaseInStackQueuedSpinLock\n";
    return 0;
}
```

| Linux API | 禁用中断 | 禁用软中断 | 适用场景 |
|-----------|---------|-----------|---------|
| `spin_lock` | ❌ | ❌ | 不与中断共享的锁 |
| `spin_lock_irq` | ✅ | ✅ | 与中断处理程序共享 |
| `spin_lock_irqsave` | ✅ | ✅ | 不确定中断状态时 |
| `spin_lock_bh` | ❌ | ✅ | 与软中断共享 |

> 内核自旋锁在单核（UP）系统上通常退化为禁用抢占/中断，因为自旋无意义。

***

### 9. 实际应用场景

| 场景 | 使用自旋锁的原因 |
|------|----------------|
| 内核中断处理 | 不能睡眠，必须自旋 |
| 数据库 Buffer Pool Latch | 临界区极短（修改指针） |
| 内存分配器（jemalloc/tcmalloc） | tcache 操作极快 |
| 引用计数更新 | 单条原子指令+短暂持锁 |
| RCU 读侧 | 实际无锁，但写侧可能用自旋等待读者退出 |
| 游戏引擎帧同步 | 每帧固定时间，不允许睡眠延迟 |

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

class SpinLock {
    std::atomic_flag flag_ = ATOMIC_FLAG_INIT;
public:
    void lock() {
        while (flag_.test_and_set(std::memory_order_acquire)) {
#if defined(_MSC_VER)
            _mm_pause();
#elif defined(__x86_64__) || defined(__i386__)
            __asm__ __volatile__("pause" ::: "memory");
#elif defined(__aarch64__)
            __asm__ __volatile__("yield" ::: "memory");
#else
            std::this_thread::yield();
#endif
        }
    }
    void unlock() {
        flag_.clear(std::memory_order_release);
    }
};

struct FreeList {
    struct Node {
        Node* next;
        int data;
    };

    Node* head_{nullptr};
    SpinLock lock_;

    void push(Node* n) {
        lock_.lock();
        n->next = head_;
        head_ = n;
        lock_.unlock();
    }

    Node* pop() {
        lock_.lock();
        Node* n = head_;
        if (n) {
            head_ = n->next;
        }
        lock_.unlock();
        return n;
    }
};

int main() {
    FreeList list;
    std::vector<FreeList::Node> nodes(10000);

    auto producer = [&](int start) {
        for (int i = start; i < start + 2500; ++i) {
            nodes[i].data = i;
            list.push(&nodes[i]);
        }
    };

    std::vector<std::thread> threads;
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(producer, i * 2500);
    }
    for (auto& t : threads) {
        t.join();
    }

    int count = 0;
    while (auto* n = list.pop()) {
        ++count;
    }
    std::cout << "从 FreeList 弹出 " << count << " 个节点\n";
    return 0;
}
```

> 内存分配器的 free list 操作（push/pop）只需修改一两个指针，持锁时间极短，是自旋锁的典型应用。

***

### 10. 自旋锁的选择指南

```
                    临界区长度
                    ↓
        ┌───────────┼───────────┐
        │ < 100ns   │ 100ns~10μs│ > 10μs
        │           │           │
  低    │ 朴素自旋   │ 退避自旋   │ 互斥锁
  竞    │           │           │
  争 ───┼───────────┼───────────┤
        │ Ticket    │ MCS/CLH   │ 互斥锁
  高    │ Lock      │ Lock      │
  争    │           │           │
        └───────────┴───────────┘
```

| 条件 | 推荐 | 原因 |
|------|------|------|
| 临界区 < 100 条指令，低竞争 | 朴素自旋锁 | 开销最小 |
| 临界区短，高竞争 | 指数退避自旋锁 | 减少总线流量 |
| 需要公平性 | Ticket Lock | FIFO 保证 |
| 大量 CPU 核心，NUMA | MCS Lock | 本地自旋，可扩展 |
| 临界区含 I/O 或长计算 | `std::mutex` | 避免浪费 CPU |
| 内核态，不能睡眠 | 内核自旋锁 | 唯一选择 |
| 不确定 | `std::mutex` | 安全默认选项 |

```cpp
#include <iostream>
#include <mutex>
#include <atomic>
#include <thread>

int main() {
    std::cout << "选择建议:\n";
    std::cout << "1. 默认使用 std::mutex，除非有明确性能瓶颈\n";
    std::cout << "2. 仅在临界区 < 上下文切换时间时考虑自旋锁\n";
    std::cout << "3. 上下文切换时间约 1-10 μs（可用 sched_latency 测量）\n";
    std::cout << "4. 先 profile，再优化锁策略\n";
    std::cout << "5. 考虑 lock-free 数据结构作为替代方案\n";
    return 0;
}
```

***

### 11. 极简总结

| 概念 | 核心要点 |
|------|---------|
| 忙等待 | 自旋锁不释放 CPU，循环检测锁状态 |
| vs 互斥锁 | 自旋锁无上下文切换，但浪费 CPU；互斥锁反之 |
| `atomic_flag` | C++ 标准保证 lock-free 的原子类型，自旋锁基础 |
| 指数退避 | 逐步增加等待间隔，减少缓存行争用和总线流量 |
| Ticket Lock | 取号排队，FIFO 公平，避免饥饿 |
| MCS Lock | 链表结构，每个线程在本地变量自旋，可扩展性最佳 |
| 内核自旋锁 | 持锁不能睡眠/被抢占，需配合中断禁用 |
| 实际应用 | 中断处理、内存分配器、短临界区 |
| 选择指南 | 临界区极短用自旋，否则用互斥锁；不确定就用互斥锁 |

**核心原则**：
- 自旋锁仅适用于持锁时间 < 上下文切换开销的场景
- 高竞争下必须使用退避策略或 MCS Lock
- 内核态自旋锁有特殊约束（不能睡眠）
- 默认选择 `std::mutex`，仅在 profile 后确认需要时才换自旋锁

***

### 相关阅读

- [原子操作与原子变量](20-原子操作与原子变量.md)
- [锁的粒度与性能](13-锁的粒度与性能.md)
- [什么是读写锁Read-Write-Lock](18-什么是读写锁Read-Write-Lock.md)

***