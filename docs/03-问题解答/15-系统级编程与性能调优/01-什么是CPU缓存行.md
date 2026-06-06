# 什么是CPU缓存行
> 📖 相关章节：[内存管理](../../01-C语言/09-内存管理.md)、[高级内存管理](../../09-系统级编程与性能调优/01-高级内存管理.md)、[CPU缓存优化](../../09-系统级编程与性能调优/02-CPU缓存优化.md)

> 核心提炼：CPU不是逐字节读写内存的，而是以缓存行（通常64字节）为单位整块搬运——两个不相关的变量如果恰好住在同一缓存行，一个被修改就会导致另一个在别的核心上失效，这就是伪共享（False Sharing），性能杀手之一。

***

### 1. 什么是缓存行

#### 1.1 为什么需要缓存行

CPU 的速度远超内存：

```
CPU 寄存器：    ~1 个时钟周期（0.3ns）
L1 缓存：       ~4 个时钟周期（1ns）
L2 缓存：       ~12 个时钟周期（4ns）
L3 缓存：       ~40 个时钟周期（15ns）
主内存：         ~200+ 个时钟周期（80ns）
```

如果 CPU 每读一个字节都要等 80ns，那 99% 的时间都在等内存。解决方案：**一次多读一些**，利用空间局部性——访问了一个地址，大概率很快会访问附近的地址。

#### 1.2 缓存行的定义

**缓存行（Cache Line）** = CPU 与内存之间数据传输的最小单位，通常为 **64 字节**。

```
内存地址空间：
0x0000 ┌──────────────────────────────────────────────────────────┐
       │  字节0  字节1  字节2  ...  字节63                        │ ← 缓存行0
0x0040 ├──────────────────────────────────────────────────────────┤
       │  字节64 字节65 字节66 ...  字节127                       │ ← 缓存行1
0x0080 ├──────────────────────────────────────────────────────────┤
       │  字节128 ...                                            │ ← 缓存行2
       └──────────────────────────────────────────────────────────┘

CPU 读取地址 0x0020 → 整个缓存行0（0x0000~0x003F）被加载到缓存
```

#### 1.3 缓存行的硬件结构

每个缓存行除了 64 字节数据，还有额外的元信息：

```
┌─────────┬──────────────┬──────────────────────────────────────┐
│ Tag     │ 状态位(MESI) │ 64 字节数据                           │
│ (高位地址)│ (2 bit)      │                                      │
└─────────┴──────────────┴──────────────────────────────────────┘
```

- **Tag**：标识该缓存行对应内存的哪个位置
- **状态位**：MESI 协议状态（Modified/Exclusive/Shared/Invalid）
- **数据**：64 字节的实际数据

***

### 2. 缓存行的大小与对齐

#### 2.1 常见平台的缓存行大小

| 平台 | 缓存行大小 |
|------|-----------|
| x86-64（Intel/AMD） | 64 字节 |
| ARM（Apple M1/M2） | 64 字节 |
| ARM（Cortex-A系列） | 64 字节 |
| POWER（IBM） | 128 字节 |
| 一些旧架构 | 32 字节 |

#### 2.2 运行时获取缓存行大小

```cpp
#include <cstdio>

// 方式1：C++17 std::hardware_destructive_interference_size
#if __cplusplus >= 201703L
#include <new>
constexpr size_t cache_line_size = std::hardware_destructive_interference_size;
#endif

// 方式2：Linux 系统调用
#include <unistd.h>
size_t get_cache_line_size_linux() {
    return sysconf(_SC_LEVEL1_DCACHE_LINESIZE);
}

// 方式3：平台特定 API
#ifdef _WIN32
#include <windows.h>
size_t get_cache_line_size_win() {
    DWORD size = 0;
    GetLogicalProcessorInformation(nullptr, &size);
    std::vector<BYTE> buffer(size);
    auto* ptr = reinterpret_cast<SYSTEM_LOGICAL_PROCESSOR_INFORMATION*>(buffer.data());
    GetLogicalProcessorInformation(ptr, &size);
    for (DWORD i = 0; i < size / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION); ++i) {
        if (ptr[i].Relationship == RelationCache && ptr[i].Cache.Level == 1) {
            return ptr[i].Cache.LineSize;
        }
    }
    return 64;  // 默认值
}
#endif

int main() {
    printf("缓存行大小: %zu 字节\n", get_cache_line_size_linux());
    return 0;
}
```

#### 2.3 缓存行对齐

数据对齐到缓存行边界，可以避免一个变量跨越两个缓存行：

```cpp
#include <cstdio>

// 未对齐：可能跨越两个缓存行
struct BadAlign {
    char data[65];  // 65 字节，跨越两个缓存行
};

// 对齐到缓存行
struct alignas(64) GoodAlign {
    char data[65];  // 起始地址对齐到 64 字节边界
};

int main() {
    BadAlign bad;
    GoodAlign good;

    printf("BadAlign 对齐: %zu\n", alignof(BadAlign));   // 1
    printf("GoodAlign 对齐: %zu\n", alignof(GoodAlign));  // 64

    printf("BadAlign 大小: %zu\n", sizeof(BadAlign));     // 65
    printf("GoodAlign 大小: %zu\n", sizeof(GoodAlign));   // 128（填充到 64 的倍数）

    return 0;
}
```

***

### 3. False Sharing 的原理与消除

#### 3.1 False Sharing 是什么

**False Sharing（伪共享）** = 不同核心修改同一缓存行上的不同变量，导致缓存行在核心间反复失效。

```
时间线：
  Core 0 修改变量 A ──→ 缓存行失效（Core 1 的副本）
  Core 1 修改变量 B ──→ 缓存行失效（Core 0 的副本）
  Core 0 修改变量 A ──→ 缓存行失效（Core 1 的副本）
  ...无限循环...

变量 A 和 B 毫无关系，只是恰好住在同一缓存行！
```

#### 3.2 False Sharing 的性能影响

```cpp
#include <cstdio>
#include <chrono>
#include <thread>

// ❌ 有 False Sharing 的版本
struct CounterBad {
    int count1;  // 线程1 修改
    int count2;  // 线程2 修改
    // count1 和 count2 可能在同一缓存行！
};

void bench_false_sharing() {
    CounterBad counter{0, 0};

    auto start = std::chrono::high_resolution_clock::now();

    std::thread t1([&]() {
        for (int i = 0; i < 100000000; ++i) {
            counter.count1++;
        }
    });

    std::thread t2([&]() {
        for (int i = 0; i < 100000000; ++i) {
            counter.count2++;
        }
    });

    t1.join();
    t2.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printf("False Sharing 版本: %lld ms\n", ms.count());
}

int main() {
    bench_false_sharing();
    return 0;
}
```

#### 3.3 消除 False Sharing 的方法

**方法1：缓存行对齐（最常用）**

```cpp
#include <cstdio>
#include <chrono>
#include <thread>

// ✅ 消除 False Sharing：缓存行对齐
struct CounterGood {
    alignas(64) int count1;  // 独占一个缓存行
    alignas(64) int count2;  // 独占另一个缓存行
};

void bench_no_false_sharing() {
    CounterGood counter{0, 0};

    auto start = std::chrono::high_resolution_clock::now();

    std::thread t1([&]() {
        for (int i = 0; i < 100000000; ++i) {
            counter.count1++;
        }
    });

    std::thread t2([&]() {
        for (int i = 0; i < 100000000; ++i) {
            counter.count2++;
        }
    });

    t1.join();
    t2.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printf("无 False Sharing 版本: %lld ms\n", ms.count());
}

int main() {
    bench_no_false_sharing();
    return 0;
}
```

**方法2：手动填充（Padding）**

```cpp
// 手动填充到缓存行大小
struct CounterPadded {
    int count1;
    char padding1[60];  // 填充到 64 字节
    int count2;
    char padding2[60];  // 填充到 64 字节
};
```

**方法3：线程局部存储**

```cpp
#include <cstdio>
#include <thread>

// 每个线程用自己的计数器，最后合并
void bench_tls() {
    thread_local int local_count = 0;

    auto func = []() {
        for (int i = 0; i < 100000000; ++i) {
            local_count++;
        }
    };

    std::thread t1(func);
    std::thread t2(func);
    t1.join();
    t2.join();
}
```

#### 3.4 性能对比

典型结果（8线程，各递增1亿次）：

```
有 False Sharing：  ~3000 ms
无 False Sharing：  ~300 ms
加速比：            ~10x
```

***

### 4. `__cacheline_aligned` 与编译器支持

#### 4.1 各编译器的缓存行对齐语法

```cpp
// 方式1：C++11 alignas（推荐，可移植）
struct alignas(64) AlignedData {
    int value;
};

// 方式2：GCC/Clang __attribute__
struct __attribute__((aligned(64))) AlignedDataGCC {
    int value;
};

// 方式3：MSVC __declspec
__declspec(align(64)) struct AlignedDataMSVC {
    int value;
};

// 方式4：C++17 标准常量
#include <new>
struct alignas(std::hardware_destructive_interference_size) AlignedDataStd {
    int value;
};
```

#### 4.2 Linux 内核中的 `____cacheline_aligned`

```c
// Linux 内核定义（include/linux/cache.h）
#define ____cacheline_aligned __attribute__((__aligned__(SMP_CACHE_BYTES)))

// 使用示例
struct per_cpu_data {
    int counter;
} ____cacheline_aligned;

// 在多核环境中，每个 CPU 的数据独占缓存行
DEFINE_PER_CPU(struct per_cpu_data, cpu_data);
```

#### 4.3 封装可移植的缓存行对齐宏

```cpp
// cache_line.h - 可移植的缓存行对齐
#ifndef CACHE_LINE_H
#define CACHE_LINE_H

#include <cstddef>

// 缓存行大小
#ifdef __cpp_lib_hardware_interference_size
    constexpr size_t CACHE_LINE_SIZE = std::hardware_destructive_interference_size;
#else
    constexpr size_t CACHE_LINE_SIZE = 64;
#endif

// 对齐宏
#if defined(__GNUC__) || defined(__clang__)
    #define CACHE_LINE_ALIGN alignas(CACHE_LINE_SIZE)
#elif defined(_MSC_VER)
    #define CACHE_LINE_ALIGN __declspec(align(CACHE_LINE_SIZE))
#else
    #define CACHE_LINE_ALIGN alignas(CACHE_LINE_SIZE)
#endif

// 缓存行对齐的结构体
#define CACHE_LINE_ALIGNED struct CACHE_LINE_ALIGN

#endif // CACHE_LINE_H
```

```cpp
// 使用示例
#include "cache_line.h"

CACHE_LINE_ALIGNED Counter {
    int count1;
    // 自动对齐到缓存行，大小为 64 字节
};

// 多个变量各自对齐
struct MultiCounter {
    CACHE_LINE_ALIGN int count1;  // 独占缓存行
    CACHE_LINE_ALIGN int count2;  // 独占缓存行
    CACHE_LINE_ALIGN int count3;  // 独占缓存行
};
```

***

### 5. 缓存行对齐的性能影响

#### 5.1 正面影响：消除 False Sharing

```
对齐前（False Sharing）：
Core 0: [count1|count2|padding...] ← 修改 count1
Core 1: [count1|count2|padding...] ← 修改 count2，导致 Core 0 缓存行失效

对齐后：
Core 0: [count1|padding...]        ← 修改 count1
Core 1: [count2|padding...]        ← 修改 count2，互不影响
```

#### 5.2 负面影响：内存浪费

```cpp
// 过度对齐的例子
struct OverAligned {
    alignas(64) int a;    // 64 字节（实际只用 4 字节）
    alignas(64) int b;    // 64 字节
    alignas(64) int c;    // 64 字节
};
// 总大小：192 字节，实际数据只有 12 字节，浪费 93.75%
```

#### 5.3 何时该对齐，何时不该

| 场景 | 是否对齐 | 原因 |
|------|---------|------|
| 多线程各自修改的变量 | ✅ 对齐 | 消除 False Sharing |
| 只读共享变量 | ❌ 不需要 | 多核共享读不失效 |
| 单线程访问的变量 | ❌ 不需要 | 没有跨核争用 |
| 数组元素 | ❌ 通常不需要 | 对齐会破坏空间局部性 |
| 热点计数器 | ✅ 对齐 | 高频修改，争用严重 |

#### 5.4 缓存行对齐与缓存命中率

```cpp
#include <cstdio>
#include <chrono>
#include <vector>

// 场景1：紧凑布局，缓存行利用率高
struct Compact {
    int id;
    float x, y, z;
};  // 16 字节，4个元素共享一个缓存行

// 场景2：过度对齐，缓存行利用率低
struct OverAligned {
    alignas(64) int id;
    alignas(64) float x, y, z;
};  // 每个 64 字节，一个缓存行只放一个元素

void bench_sequential_access() {
    const int N = 10000000;

    // 紧凑布局：遍历快（缓存命中率高）
    {
        std::vector<Compact> data(N);
        auto start = std::chrono::high_resolution_clock::now();
        for (auto& d : data) {
            d.x += 1.0f;
            d.y += 1.0f;
            d.z += 1.0f;
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        printf("紧凑布局顺序访问: %lld ms\n", ms.count());
    }

    // 过度对齐：遍历慢（缓存命中率低）
    {
        std::vector<OverAligned> data(N);
        auto start = std::chrono::high_resolution_clock::now();
        for (auto& d : data) {
            d.x += 1.0f;
            d.y += 1.0f;
            d.z += 1.0f;
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        printf("过度对齐顺序访问: %lld ms\n", ms.count());
    }
}

int main() {
    bench_sequential_access();
    return 0;
}
```

***

### 6. 缓存行感知的数据结构设计

#### 6.1 分离热数据与冷数据

```cpp
// ❌ 热数据和冷数据混在一起
struct GameObjectBad {
    // 热数据（每帧访问）
    float x, y, z;          // 位置
    float vx, vy, vz;       // 速度
    // 冷数据（偶尔访问）
    std::string name;        // 名称
    int level;               // 等级
    int experience;          // 经验值
    std::string description; // 描述
};

// ✅ 分离热数据与冷数据
struct GameObjectHot {
    float x, y, z;          // 位置
    float vx, vy, vz;       // 速度
    int id;                  // 指向冷数据的索引
};  // 28 字节，两个对象共享一个缓存行

struct GameObjectCold {
    std::string name;
    int level;
    int experience;
    std::string description;
};
```

#### 6.2 Per-CPU 数据

```cpp
// 每个 CPU 核心独占一个缓存行
struct alignas(64) PerCPUData {
    long count;
    long sum;
    long min_val;
    long max_val;
    // 填充到 64 字节
    char padding[64 - 4 * sizeof(long)];
};

// 数组大小等于 CPU 核心数
PerCPUData per_cpu[128];  // 最多支持 128 核

// 每个线程只访问自己的数据，无 False Sharing
void increment(int cpu_id) {
    per_cpu[cpu_id].count++;
}
```

***

### 7. 检测 False Sharing 的工具

#### 7.1 perf（Linux）

```bash
# 统计缓存失效次数
perf stat -e cache-misses,cache-references ./my_program

# 查看具体哪些地址导致缓存失效
perf record -e mem-loads,mem-stores ./my_program
perf report
```

#### 7.2 Intel VTune

```
1. 运行 VTune 的 "Microarchitecture Exploration" 分析
2. 查看 "False Sharing" 指标
3. 定位到具体代码行
```

#### 7.3 perf 伪共享检测（Linux 5.9+）

```bash
# 使用 c2c（Cache to Cache）工具检测伪共享
perf c2c record ./my_program
perf c2c report
# 输出会显示哪些缓存行在多个核心间频繁失效
```

***

### 8. 常见误区

| 误区 | 事实 |
|------|------|
| "对齐到缓存行总是更快" | 顺序访问场景，对齐反而降低缓存利用率 |
| "False Sharing 只影响多线程" | 确实如此，单线程不存在此问题 |
| "64 字节对齐就够了" | 某些平台缓存行是 128 字节（如 IBM POWER） |
| "编译器会自动避免 False Sharing" | 编译器不知道运行时哪些变量会被不同线程访问 |
| "atomic 变量不需要对齐" | atomic 变量恰恰是最需要对齐的，因为高频修改 |

***

### 9. 总结

| 要点 | 说明 |
|------|------|
| 缓存行 | CPU 读写内存的最小单位，通常 64 字节 |
| False Sharing | 不同核心修改同一缓存行的不同变量，导致缓存反复失效 |
| 消除方法 | 缓存行对齐（alignas(64)）、手动填充、线程局部存储 |
| 对齐代价 | 内存浪费，可能降低缓存利用率 |
| 选择原则 | 多线程写场景对齐，单线程/只读场景不需要 |
| 检测工具 | perf c2c、VTune、perf stat |