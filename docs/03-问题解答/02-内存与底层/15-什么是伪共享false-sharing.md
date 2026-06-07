# 什么是伪共享 False Sharing
> 📖 相关章节：[指针](../../01-C语言/06-指针.md)、[结构体与联合体](../../01-C语言/08-结构体与联合体.md)、[内存管理](../../01-C语言/09-内存管理.md)、[智能指针](../../02-CPP/08-智能指针与内存管理.md)、[内存模型](../../02-CPP/32-内存模型.md)

> "两个人共用一张桌子，一个人动一下，另一个人就得重新整理"——伪共享让多核互相拖后腿。

***

### 1. 核心定义

**伪共享（False Sharing）** = 不同线程修改同一缓存行上的不同变量，导致缓存行在多核间反复失效，性能骤降。

关键点：**变量之间没有任何逻辑关系，只是恰好住在同一缓存行，就被迫互相拖累**。

***

### 2. 生活类比

**共用办公桌**：

想象两个人（线程0和线程1）共用一张大办公桌（缓存行，64字节）：

- 线程0 用桌子左半边改文件 A
- 线程1 用桌子右半边改文件 B
- 两人改的是**不同的文件**，互不相关

但桌子是**整体移动**的：

- 线程0 改了文件 A → 整张桌子被搬到线程0 的办公室
- 线程1 想改文件 B → 发现桌子不在自己办公室，得从线程0 那搬回来
- 线程1 改了文件 B → 整张桌子又被搬到线程1 的办公室
- 线程0 又要改文件 A → 又得搬……

**结果**：桌子在两个办公室之间疯狂搬运，两人都在等桌子，实际干活的时间很少。

**解决**：给每人一张独立的小桌子（缓存行对齐），互不干扰。

***

### 3. 原理详解

#### 1. CPU 缓存以缓存行为单位

CPU 不按字节读写内存，而是按**缓存行（Cache Line）**读写，通常 **64 字节**。

```
内存地址:  0x00  0x40  0x80  0xC0
           |_____|_____|_____|_____|
           64B   64B   64B   64B
```

一次加载一整行 64 字节，即使你只读 1 字节。

#### 2. 多核各自有 L1/L2 缓存

```
Core 0: L1 缓存（私有）  ←→  L2 缓存（私有）
                                    ↕
                              L3 缓存（共享）
                                    ↕
Core 1: L1 缓存（私有）  ←→  L2 缓存（私有）  ←→  内存
```

每个核的 L1/L2 是私有的，同一份数据可能在多个核的缓存中都有副本。

#### 3. MESI 协议保持一致性

MESI 是缓存一致性协议，每个缓存行有四种状态：

| 状态 | 全称 | 含义 |
|------|------|------|
| M | Modified | 已修改，和内存不一致，只有本核有 |
| E | Exclusive | 独占，和内存一致，只有本核有 |
| S | Shared | 共享，和内存一致，多个核都有 |
| I | Invalid | 无效，数据过期，不能使用 |

**关键规则**：当一个核修改了某个缓存行，其他核中同一缓存行的状态变为 **Invalid**（失效）。

#### 4. 两个不相关的变量恰好同行 → 互相拖累

```
缓存行（64字节）:
┌─────────────────────────────────────────────────────────┐
│ counter_a (8B) │ padding... │ counter_b (8B) │ padding │
└─────────────────────────────────────────────────────────┘
 ↑ Core 0 写                  ↑ Core 1 写
```

- Core 0 写 `counter_a` → 整行标记为 Modified → Core 1 的该行变 Invalid
- Core 1 写 `counter_b` → 要先从 Core 0 拿最新行 → Core 0 的该行变 Invalid
- Core 0 再写 `counter_a` → 又得从 Core 1 拿 → Core 1 的行变 Invalid
- ……

**这就是伪共享：变量没有共享，缓存行被共享了**。

***

### 4. 代码示例：多线程计数器的伪共享问题

#### 1. 有伪共享的版本

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

struct Counters {
    uint64_t counter_a;
    uint64_t counter_b;
};

void worker_a(Counters& c, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        c.counter_a++;
    }
}

void worker_b(Counters& c, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        c.counter_b++;
    }
}

int main() {
    const int N = 100'000'000;
    Counters c{0, 0};

    auto start = std::chrono::high_resolution_clock::now();

    std::thread t1(worker_a, std::ref(c), N);
    std::thread t2(worker_b, std::ref(c), N);
    t1.join();
    t2.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "counter_a = " << c.counter_a << std::endl;
    std::cout << "counter_b = " << c.counter_b << std::endl;
    std::cout << "time: " << ms << "ms (有伪共享)" << std::endl;
}
```

`counter_a` 和 `counter_b` 很可能在同一缓存行（各 8 字节，加起来才 16 字节 < 64 字节），两个核疯狂互踢。

#### 2. 修复版本：缓存行对齐

```cpp
#include <iostream>
#include <thread>
#include <chrono>

struct alignas(64) AlignedCounter {
    uint64_t value;
};

struct Counters {
    AlignedCounter counter_a;
    AlignedCounter counter_b;
};

void worker_a(Counters& c, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        c.counter_a.value++;
    }
}

void worker_b(Counters& c, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        c.counter_b.value++;
    }
}

int main() {
    const int N = 100'000'000;
    Counters c{{0}, {0}};

    auto start = std::chrono::high_resolution_clock::now();

    std::thread t1(worker_a, std::ref(c), N);
    std::thread t2(worker_b, std::ref(c), N);
    t1.join();
    t2.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "counter_a = " << c.counter_a.value << std::endl;
    std::cout << "counter_b = " << c.counter_b.value << std::endl;
    std::cout << "time: " << ms << "ms (无伪共享)" << std::endl;
}
```

`alignas(64)` 让每个 `AlignedCounter` 独占一个缓存行，两个核互不干扰。

**典型性能差距**：伪共享版本可能慢 3~10 倍。

***

### 5. 检测方法

#### 1. Linux：perf 工具

```bash
# 统计缓存未命中
perf stat -e cache-misses,cache-references ./your_program

# 实时观察热点
perf top -e cache-misses

# 查看具体事件的详细统计
perf stat -e L1-dcache-load-misses,L1-dcache-loads ./your_program
```

**伪共享的信号**：

- `cache-misses` 异常高
- 多线程程序的性能不随核数增加而提升（甚至下降）
- `perf top` 显示热点在频繁写入的变量附近

#### 2. 代码层面：检查变量地址

```cpp
#include <iostream>
#include <cstdint>

struct Counters {
    uint64_t counter_a;
    uint64_t counter_b;
};

int main() {
    Counters c;
    std::cout << "counter_a address: " << &c.counter_a << std::endl;
    std::cout << "counter_b address: " << &c.counter_b << std::endl;

    uintptr_t addr_a = reinterpret_cast<uintptr_t>(&c.counter_a);
    uintptr_t addr_b = reinterpret_cast<uintptr_t>(&c.counter_b);

    bool same_line = (addr_a / 64) == (addr_b / 64);
    std::cout << "同一缓存行? " << (same_line ? "是(有伪共享风险)" : "否(安全)") << std::endl;
}
```

如果两个变量的地址除以 64 的商相同，就在同一缓存行。

***

### 6. 解决方案

#### 1. 方案1：alignas 缓存行对齐（推荐）

```cpp
struct alignas(64) PaddedCounter {
    uint64_t value;
};
```

优点：编译器自动处理，代码简洁
缺点：每个变量占 64 字节，浪费内存

#### 2. 方案2：手动 padding 填充

```cpp
struct CounterA {
    uint64_t value;
    char padding[56];
};

struct CounterB {
    uint64_t value;
    char padding[56];
};
```

或者放在一起：

```cpp
struct Counters {
    uint64_t counter_a;
    char padding1[56];
    uint64_t counter_b;
    char padding2[56];
};
```

优点：兼容 C++11 之前
缺点：手动算字节数，容易算错

#### 3. 方案3：结构体字段重排

把只读字段和读写字段分开放，减少伪共享概率：

```cpp
struct BadLayout {
    uint64_t read_only_1;
    uint64_t write_heavy_1;
    uint64_t read_only_2;
    uint64_t write_heavy_2;
};

struct GoodLayout {
    uint64_t read_only_1;
    uint64_t read_only_2;
    char padding[48];
    uint64_t write_heavy_1;
    uint64_t write_heavy_2;
    char padding2[48];
};
```

**原则**：频繁写入的变量不要放在同一缓存行。

***

### 7. 与缓存命中率的关系

[什么是缓存命中率](../01-基础概念/04-什么是缓存命中率.md) 讲过缓存命中率对性能的影响。伪共享本质上是在**破坏缓存命中率**：

```
伪共享 → 缓存行反复失效 → L1 缓存未命中 → 跑去 L3/内存拿数据 → 命中率暴跌 → 性能骤降
```

| 情况 | 缓存命中率 | 性能 |
|------|:---:|:---:|
| 无伪共享，各核独立 | 高（>95%） | 快 |
| 有伪共享，缓存行互踢 | 低（可能 <50%） | 慢 3~10 倍 |

**缓存命中率是结果，伪共享是原因之一**。排查缓存命中率低的问题时，伪共享是重点嫌疑对象。

***

### 8. 极简总结

**伪共享 = 不同变量恰好在同一缓存行 → 多核互相踢缓存行 → 性能暴跌**

| 要点 | 说明 |
|------|------|
| 根因 | 缓存行（64B）是缓存最小单位，同行变量一损俱损 |
| 信号 | 多核程序性能不随核数增加，cache-misses 异常高 |
| 检测 | perf stat 看 cache-misses，或检查变量地址是否同行 |
| 解决 | alignas(64) 对齐 / padding 填充 / 字段重排 |
| 一句话 | 频繁写的变量别放同一缓存行 |

***

### 相关阅读

- [CPU缓存友好编程实践](./13-CPU缓存友好编程实践.md)
- [结构体内存对齐](04-结构体内存对齐.md)