> **前置知识**：高级内存管理见 [高级内存管理](./01-高级内存管理.md)，内存模型见 [C++内存模型](../02-CPP/32-内存模型.md)。

# CPU缓存优化

> 深入理解CPU缓存机制，编写缓存友好的高性能代码

***

> **With respect to memory, the CPU is like a speed demon sitting at a desk with a very small inbox.** — Ulrich Drepper
> （在内存方面，CPU就像一个坐在办公桌前的急脾气，而收件箱却非常小。）

***

> **🎯 近水楼台先得月。**
>
> （CPU缓存优化的核心思想：让数据尽可能靠近计算单元，减少等待时间。）

## 前置知识
- [高级内存管理](01-高级内存管理.md)
- C++内存模型（第2章）
## 后续内容
- [SIMD与向量化编程](03-SIMD与向量化编程.md)

***

> 💡 **通俗理解 - 为什么CPU缓存如此重要？**

**CPU = 闪电侠，内存 = 蜗牛**

想象CPU是闪电侠，处理速度极快，但他需要从内存（蜗牛）获取数据：
- CPU执行一条指令：1纳秒
- 从L1缓存取数据：1纳秒（就在手边）
- 从L2缓存取数据：4纳秒（隔壁房间）
- 从L3缓存取数据：12纳秒（楼下）
- 从主存取数据：100纳秒（隔壁城市！）

**如果每次都要从主存取数据，闪电侠99%的时间都在等蜗牛！**

> 🔬 **抽象理解 - CPU缓存优化的本质**：
>
> - **核心目标**：最大化缓存命中率，最小化缓存未命中
> - **关键手段**：数据布局优化、访问模式优化、缓存行对齐
> - **性能影响**：缓存命中与未命中的性能差距可达10-100倍
> - **两大原则**：时间局部性（近期用过的数据再用）和空间局部性（相邻数据一起用）

***

## 目录

- [1. CPU缓存层次结构](#1-cpu缓存层次结构)
- [2. 缓存行与对齐](#2-缓存行与对齐)
- [3. 数据布局优化(AoS/SoA)](#3-数据布局优化aossa)
- [4. 缓存友好的数据结构](#4-缓存友好的数据结构)
- [5. 缓存预取](#5-缓存预取)
- [6. 缓存一致性协议(MESI)](#6-缓存一致性协议mesi)
- [7. false sharing深度分析与消除](#7-false-sharing深度分析与消除)
- [8. 小结](#8-小结)

***

## 1. CPU缓存层次结构

### 1.1 现代CPU缓存架构

```
┌──────────────────────────────────────────────────────────┐
│                    CPU缓存层次                             │
├──────────────────────────────────────────────────────────┤
│                                                          │
│  ┌─────────┐  ┌─────────┐                               │
│  │  Core 0 │  │  Core 1 │  ...                           │
│  │ ┌─────┐ │  │ ┌─────┐ │                               │
│  │ │ L1D │ │  │ │ L1D │ │  32-48KB  延迟 ~1ns          │
│  │ │32KB │ │  │ │32KB │ │  带宽 ~1TB/s                  │
│  │ └─────┘ │  │ └─────┘ │                               │
│  │ ┌─────┐ │  │ ┌─────┐ │                               │
│  │ │ L1I │ │  │ │ L1I │ │  32KB    延迟 ~1ns           │
│  │ │32KB │ │  │ │32KB │ │                               │
│  │ └─────┘ │  │ └─────┘ │                               │
│  │ ┌─────┐ │  │ ┌─────┐ │                               │
│  │ │ L2  │ │  │ │ L2  │ │  256KB-1MB 延迟 ~4ns         │
│  │ │512KB│ │  │ │512KB│ │  带宽 ~500GB/s                │
│  │ └─────┘ │  │ └─────┘ │                               │
│  └─────────┘  └─────────┘                               │
│                                                          │
│  ┌──────────────────────────────────────────────────┐   │
│  │                    L3 Cache                       │   │
│  │                  8-64MB 共享                       │   │
│  │              延迟 ~12ns  带宽 ~200GB/s             │   │
│  └──────────────────────────────────────────────────┘   │
│                                                          │
│  ┌──────────────────────────────────────────────────┐   │
│  │                  主存 (DRAM)                       │   │
│  │              延迟 ~100ns  带宽 ~50GB/s             │   │
│  └──────────────────────────────────────────────────┘   │
└──────────────────────────────────────────────────────────┘
```

### 1.2 缓存参数查询

```cpp
#include <iostream>
#include <cstdint>

// ============================================
// 在Linux上查询CPU缓存信息
// ============================================

// 方法1：通过cpuid指令查询（x86架构）
struct CacheInfo {
    int level;
    enum Type { DATA, INSTRUCTION, UNIFIED } type;
    size_t size;
    size_t line_size;
    size_t associativity;
};

void query_cache_info_cpuid() {
    // 使用__cpuid内置函数
    uint32_t eax, ebx, ecx, edx;

    // CPUID leaf 4: 缓存参数
    for (int i = 0; i < 10; ++i) {
        __cpuid_count(4, i, eax, ebx, ecx, edx);

        int cache_type = eax & 0x1F;
        if (cache_type == 0) break;  // 没有更多缓存

        int cache_level = (eax >> 5) & 0x7;
        size_t line_size = (ebx & 0xFFF) + 1;
        size_t partitions = ((ebx >> 12) & 0x3FF) + 1;
        size_t ways = ((ebx >> 22) & 0x3FF) + 1;
        size_t sets = ecx + 1;
        size_t total_size = ways * partitions * line_size * sets;

        const char* type_names[] = {"", "数据", "指令", "统一"};
        std::cout << "L" << cache_level << " "
                  << type_names[cache_type] << " 缓存: "
                  << total_size / 1024 << "KB, "
                  << "行大小: " << line_size << "B, "
                  << ways << "路组相联" << std::endl;
    }
}

// 方法2：在Linux上读取sysfs
void query_cache_info_sysfs() {
    std::cout << "\n=== 通过sysfs查询缓存信息 ===" << std::endl;
    // 可以读取以下文件：
    // /sys/devices/system/cpu/cpu0/cache/index0/size
    // /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size
    // /sys/devices/system/cpu/cpu0/cache/index0/number_of_sets
    // /sys/devices/system/cpu/cpu0/cache/index0/ways_of_associativity

    // 示例：读取L1D缓存行大小
    // cat /sys/devices/system/cpu/cpu0/cache/index0/coherency_line_size
    // 通常输出: 64
}

// ============================================
// 缓存延迟测量
// ============================================

#include <chrono>
#include <vector>
#include <random>
#include <algorithm>

void measure_cache_latency() {
    std::cout << "\n=== 缓存延迟测量 ===" << std::endl;

    // 创建链表结构来测量不同大小下的访问延迟
    struct Node {
        Node* next;
        char padding[56];  // 填充到64字节（一个缓存行）
    };

    // 测试不同大小范围
    size_t sizes[] = {
        4 * 1024,          // 4KB - L1内
        32 * 1024,         // 32KB - L1边界
        256 * 1024,        // 256KB - L2内
        1024 * 1024,       // 1MB - L2边界
        8 * 1024 * 1024,   // 8MB - L3内
        64 * 1024 * 1024,  // 64MB - 超出L3
    };

    for (size_t size : sizes) {
        size_t num_nodes = size / sizeof(Node);
        std::vector<Node> nodes(num_nodes);

        // 创建随机访问链（避免预取器干扰）
        std::vector<size_t> indices(num_nodes);
        for (size_t i = 0; i < num_nodes; ++i) indices[i] = i;
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(indices.begin(), indices.end(), g);

        for (size_t i = 0; i < num_nodes - 1; ++i) {
            nodes[indices[i]].next = &nodes[indices[i + 1]];
        }
        nodes[indices[num_nodes - 1]].next = nullptr;

        // 遍历链表并计时
        const int iterations = 10;
        auto start = std::chrono::high_resolution_clock::now();

        volatile Node* current = &nodes[indices[0]];
        for (int iter = 0; iter < iterations; ++iter) {
            current = &nodes[indices[0]];
            while (current) {
                current = current->next;
            }
        }

        auto end = std::chrono::high_resolution_clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
        double ns_per_access = static_cast<double>(ns) / (num_nodes * iterations);

        std::cout << "大小 " << size / 1024 << "KB: "
                  << ns_per_access << " ns/访问" << std::endl;
    }
}
```

***

## 2. 缓存行与对齐

### 2.1 缓存行原理

缓存行（Cache Line）是缓存与主存之间数据传输的最小单位，通常为64字节。

```
┌──────────────────────────────────────────────────────────┐
│                缓存行与内存映射                           │
├──────────────────────────────────────────────────────────┤
│                                                          │
│  内存地址空间                                             │
│  ┌──────────────────────────────────────────────────┐   │
│  │ 0x0000 │ 0x0040 │ 0x0080 │ 0x00C0 │ 0x0100 │...│   │
│  │ 64字节 │ 64字节 │ 64字节 │ 64字节 │ 64字节 │   │   │
│  └────┬───┴────┬───┴────┬───┴────┬───┴────┬───┘   │   │
│       │        │        │        │        │        │   │
│  缓存行映射（每个缓存行64字节）                          │
│  ┌────▼───┐┌────▼───┐┌────▼───┐┌────▼───┐┌────▼───┐│   │
│  │ Line 0 ││ Line 1 ││ Line 2 ││ Line 3 ││ Line 4 ││   │
│  │  有效  ││  有效  ││  有效  ││  无效  ││  有效  ││   │
│  │  脏   ││  干净  ││  干净  ││       ││  脏   ││   │
│  └────────┘└────────┘└────────┘└────────┘└────────┘│   │
│                                                          │
│  关键概念：                                               │
│  - 读取1字节 → 整个64字节缓存行被加载                     │
│  - 修改1字节 → 整个64字节缓存行需要写回                   │
│  - 对齐访问 → 一次缓存行加载即可                          │
│  - 跨行访问 → 需要两次缓存行加载                          │
└──────────────────────────────────────────────────────────┘
```

### 2.2 缓存行对齐实践

```cpp
#include <iostream>
#include <cstdint>
#include <atomic>
#include <thread>
#include <chrono>

// ============================================
// 缓存行大小定义
// ============================================

#ifndef CACHE_LINE_SIZE
#if defined(__x86_64__) || defined(_M_X64) || defined(__i386__)
    #define CACHE_LINE_SIZE 64
#elif defined(__arm64__) || defined(__aarch64__)
    #define CACHE_LINE_SIZE 64
#elif defined(__powerpc64__)
    #define CACHE_LINE_SIZE 128
#else
    #define CACHE_LINE_SIZE 64
#endif
#endif

// ============================================
// 对齐宏
// ============================================

#if defined(__GNUC__) || defined(__clang__)
    #define ALIGNAS(x) __attribute__((aligned(x)))
#elif defined(_MSC_VER)
    #define ALIGNAS(x) __declspec(align(x))
#endif

// ============================================
// 缓存行对齐的结构体
// ============================================

// 错误示例：未对齐，可能产生false sharing
struct BadCounter {
    std::atomic<uint64_t> count1;  // 线程1使用
    std::atomic<uint64_t> count2;  // 线程2使用
    // 两个变量可能在同一个缓存行！
};

// 正确示例：缓存行对齐，避免false sharing
struct ALIGNAS(CACHE_LINE_SIZE) GoodCounter {
    std::atomic<uint64_t> count1;
    char padding1[CACHE_LINE_SIZE - sizeof(std::atomic<uint64_t>)];

    std::atomic<uint64_t> count2;
    char padding2[CACHE_LINE_SIZE - sizeof(std::atomic<uint64_t>)];
};

// 更优雅的方式：使用alignas
struct AlignedCounter {
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> count1;
    alignas(CACHE_LINE_SIZE) std::atomic<uint64_t> count2;
};

// ============================================
// 对比测试：false sharing的性能影响
// ============================================

void benchmark_false_sharing() {
    const int ITERATIONS = 10000000;

    // 测试1：未对齐（可能false sharing）
    {
        BadCounter counter;
        counter.count1 = 0;
        counter.count2 = 0;

        auto start = std::chrono::high_resolution_clock::now();

        std::thread t1([&]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                counter.count1.fetch_add(1, std::memory_order_relaxed);
            }
        });

        std::thread t2([&]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                counter.count2.fetch_add(1, std::memory_order_relaxed);
            }
        });

        t1.join();
        t2.join();

        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "未对齐: " << ms << " ms (可能false sharing)" << std::endl;
    }

    // 测试2：缓存行对齐（无false sharing）
    {
        AlignedCounter counter;
        counter.count1 = 0;
        counter.count2 = 0;

        auto start = std::chrono::high_resolution_clock::now();

        std::thread t1([&]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                counter.count1.fetch_add(1, std::memory_order_relaxed);
            }
        });

        std::thread t2([&]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                counter.count2.fetch_add(1, std::memory_order_relaxed);
            }
        });

        t1.join();
        t2.join();

        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "对齐后: " << ms << " ms (无false sharing)" << std::endl;
    }
}

// ============================================
// 结构体字段排列优化
// ============================================

// 糟糕的布局：频繁访问的字段分散，缓存行浪费
struct BadLayout {
    bool active;           // 1字节 + 7字节填充
    double value;          // 8字节
    char name[3];          // 3字节 + 5字节填充
    int id;                // 4字节 + 4字节填充
    double* ptr;           // 8字节
    // 总大小: 40字节，但访问模式不友好
};

// 优化的布局：按访问频率分组，减少缓存行占用
struct GoodLayout {
    // 热路径字段（频繁访问，放在前面）
    double value;          // 8字节
    double* ptr;           // 8字节
    int id;                // 4字节
    bool active;           // 1字节 + 3字节填充
    // 前24字节在一个缓存行中，热路径只需一次缓存加载

    // 冷路径字段（不频繁访问，放在后面）
    char name[3];          // 3字节 + 填充
};

void layout_comparison() {
    std::cout << "BadLayout大小:  " << sizeof(BadLayout) << " 字节" << std::endl;
    std::cout << "GoodLayout大小: " << sizeof(GoodLayout) << " 字节" << std::endl;

    // 更重要的是访问模式
    GoodLayout obj;
    // 热路径只访问前24字节，一个缓存行就够了
    obj.value = 3.14;
    obj.ptr = nullptr;
    obj.id = 42;
    obj.active = true;
}
```

***

## 3. 数据布局优化(AoS/SoA)

### 3.1 AoS vs SoA

```
┌──────────────────────────────────────────────────────────┐
│              AoS (Array of Structures)                    │
├──────────────────────────────────────────────────────────┤
│                                                          │
│  Particle[0]  │  Particle[1]  │  Particle[2]  │  ...   │
│  ┌──────────┐ │  ┌──────────┐ │  ┌──────────┐ │        │
│  │ x,y,z    │ │  │ x,y,z    │ │  │ x,y,z    │ │        │
│  │ vx,vy,vz │ │  │ vx,vy,vz │ │  │ vx,vy,vz │ │        │
│  │ mass     │ │  │ mass     │ │  │ mass     │ │        │
│  │ color    │ │  │ color    │ │  │ color    │ │        │
│  └──────────┘ │  └──────────┘ │  └──────────┘ │        │
│                                                          │
│  只需要x时，加载了整个Particle → 缓存行浪费               │
├──────────────────────────────────────────────────────────┤
│              SoA (Structure of Arrays)                    │
├──────────────────────────────────────────────────────────┤
│                                                          │
│  x:    [x0, x1, x2, x3, ...]  ← 连续存储，缓存友好     │
│  y:    [y0, y1, y2, y3, ...]                            │
│  z:    [z0, z1, z2, z3, ...]                            │
│  vx:   [vx0, vx1, vx2, vx3, ...]                       │
│  vy:   [vy0, vy1, vy2, vy3, ...]                       │
│  vz:   [vz0, vz1, vz2, vz3, ...]                       │
│  mass: [m0, m1, m2, m3, ...]                            │
│                                                          │
│  只需要x时，只加载x数组 → 缓存行利用率高                 │
└──────────────────────────────────────────────────────────┘
```

### 3.2 AoS与SoA实现对比

```cpp
#include <iostream>
#include <vector>
#include <chrono>
#include <cmath>
#include <numbers>

// ============================================
// AoS实现：传统的面向对象方式
// ============================================

struct ParticleAoS {
    float x, y, z;        // 位置
    float vx, vy, vz;     // 速度
    float mass;            // 质量
    float charge;          // 电荷
};

class ParticleSystemAoS {
public:
    std::vector<ParticleAoS> particles;

    void update(float dt) {
        for (auto& p : particles) {
            // 只更新位置，但加载了整个ParticleAoS
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            p.z += p.vz * dt;
        }
    }

    // 计算总动能
    float totalKineticEnergy() const {
        float sum = 0.0f;
        for (const auto& p : particles) {
            float v2 = p.vx * p.vx + p.vy * p.vy + p.vz * p.vz;
            sum += 0.5f * p.mass * v2;
        }
        return sum;
    }
};

// ============================================
// SoA实现：数据导向设计
// ============================================

class ParticleSystemSoA {
public:
    size_t count = 0;

    // 每个属性单独存储
    std::vector<float> x, y, z;
    std::vector<float> vx, vy, vz;
    std::vector<float> mass;
    std::vector<float> charge;

    void resize(size_t n) {
        count = n;
        x.resize(n); y.resize(n); z.resize(n);
        vx.resize(n); vy.resize(n); vz.resize(n);
        mass.resize(n); charge.resize(n);
    }

    void update(float dt) {
        // 只访问位置和速度，缓存行利用率高
        for (size_t i = 0; i < count; ++i) {
            x[i] += vx[i] * dt;
            y[i] += vy[i] * dt;
            z[i] += vz[i] * dt;
        }
    }

    float totalKineticEnergy() const {
        float sum = 0.0f;
        for (size_t i = 0; i < count; ++i) {
            float v2 = vx[i] * vx[i] + vy[i] * vy[i] + vz[i] * vz[i];
            sum += 0.5f * mass[i] * v2;
        }
        return sum;
    }
};

// ============================================
// 性能对比
// ============================================

void benchmark_aos_vs_soa() {
    const size_t N = 10000000;  // 1000万粒子
    const float dt = 0.016f;    // 16ms时间步

    // AoS测试
    {
        ParticleSystemAoS sys;
        sys.particles.resize(N);
        for (size_t i = 0; i < N; ++i) {
            sys.particles[i] = {
                static_cast<float>(i), static_cast<float>(i * 2), static_cast<float>(i * 3),
                1.0f, 0.5f, 0.3f,
                1.0f, 0.0f
            };
        }

        auto start = std::chrono::high_resolution_clock::now();
        for (int iter = 0; iter < 100; ++iter) {
            sys.update(dt);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "AoS更新: " << ms << " ms" << std::endl;
    }

    // SoA测试
    {
        ParticleSystemSoA sys;
        sys.resize(N);
        for (size_t i = 0; i < N; ++i) {
            sys.x[i] = static_cast<float>(i);
            sys.y[i] = static_cast<float>(i * 2);
            sys.z[i] = static_cast<float>(i * 3);
            sys.vx[i] = 1.0f; sys.vy[i] = 0.5f; sys.vz[i] = 0.3f;
            sys.mass[i] = 1.0f; sys.charge[i] = 0.0f;
        }

        auto start = std::chrono::high_resolution_clock::now();
        for (int iter = 0; iter < 100; ++iter) {
            sys.update(dt);
        }
        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "SoA更新: " << ms << " ms" << std::endl;
    }
}
```

### 3.3 混合布局(AoSoA)

```cpp
#include <iostream>
#include <vector>
#include <cstdint>

// ============================================
// AoSoA (Array of Structures of Arrays)
// ============================================
// 结合AoS和SoA的优点：块内SoA，块间AoS
// 特别适合SIMD向量化

// 每个块包含8个粒子的数据（适合AVX2的256位寄存器）
static constexpr size_t VECTOR_WIDTH = 8;

struct ParticleBlock {
    // 每个属性8个float = 32字节 = 半个缓存行
    float x[VECTOR_WIDTH];   // 32字节
    float y[VECTOR_WIDTH];   // 32字节
    float z[VECTOR_WIDTH];   // 32字节
    float vx[VECTOR_WIDTH];  // 32字节
    float vy[VECTOR_WIDTH];  // 32字节
    float vz[VECTOR_WIDTH];  // 32字节
    float mass[VECTOR_WIDTH]; // 32字节
    float charge[VECTOR_WIDTH]; // 32字节
    // 总大小: 256字节 = 4个缓存行
};

class ParticleSystemAoSoA {
public:
    std::vector<ParticleBlock> blocks;
    size_t total_count;

    void resize(size_t n) {
        total_count = n;
        blocks.resize((n + VECTOR_WIDTH - 1) / VECTOR_WIDTH);
    }

    void update(float dt) {
        for (auto& block : blocks) {
            // 块内数据连续，适合SIMD
            for (size_t i = 0; i < VECTOR_WIDTH; ++i) {
                block.x[i] += block.vx[i] * dt;
                block.y[i] += block.vy[i] * dt;
                block.z[i] += block.vz[i] * dt;
            }
        }
    }

    // 获取第n个粒子的位置
    void getPosition(size_t n, float& out_x, float& out_y, float& out_z) const {
        size_t block_idx = n / VECTOR_WIDTH;
        size_t elem_idx = n % VECTOR_WIDTH;
        out_x = blocks[block_idx].x[elem_idx];
        out_y = blocks[block_idx].y[elem_idx];
        out_z = blocks[block_idx].z[elem_idx];
    }
};

void aosoa_demo() {
    ParticleSystemAoSoA sys;
    sys.resize(100);

    // 初始化
    for (size_t i = 0; i < sys.total_count; ++i) {
        size_t block_idx = i / VECTOR_WIDTH;
        size_t elem_idx = i % VECTOR_WIDTH;
        sys.blocks[block_idx].x[elem_idx] = static_cast<float>(i);
        sys.blocks[block_idx].vx[elem_idx] = 1.0f;
    }

    // 更新
    sys.update(0.016f);

    // 查询
    float x, y, z;
    sys.getPosition(0, x, y, z);
    std::cout << "粒子0位置: (" << x << ", " << y << ", " << z << ")" << std::endl;

    std::cout << "\n布局对比:" << std::endl;
    std::cout << "AoS:   面向对象友好，缓存不友好" << std::endl;
    std::cout << "SoA:   缓存友好，单粒子访问不友好" << std::endl;
    std::cout << "AoSoA: 兼顾缓存友好和SIMD，推荐" << std::endl;
}
```

***

## 4. 缓存友好的数据结构

### 4.1 缓存友好的树结构

传统指针式二叉树节点分散在堆中，缓存不友好。可以使用数组表示的完全二叉树。

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <cassert>

// ============================================
// 缓存友好的优先队列（基于数组的二叉堆）
// ============================================

template<typename T, typename Compare = std::less<T>>
class CacheFriendlyPriorityQueue {
public:
    explicit CacheFriendlyPriorityQueue(size_t reserve = 0) {
        if (reserve > 0) data_.reserve(reserve);
    }

    void push(const T& value) {
        data_.push_back(value);
        siftUp(data_.size() - 1);
    }

    void pop() {
        assert(!data_.empty());
        data_[0] = std::move(data_.back());
        data_.pop_back();
        if (!data_.empty()) {
            siftDown(0);
        }
    }

    const T& top() const {
        assert(!data_.empty());
        return data_[0];
    }

    bool empty() const { return data_.empty(); }
    size_t size() const { return data_.size(); }

private:
    // 父子关系通过索引计算，无需指针
    // 父节点: (i - 1) / 2
    // 左子:   2 * i + 1
    // 右子:   2 * i + 2

    void siftUp(size_t i) {
        while (i > 0) {
            size_t parent = (i - 1) / 2;
            if (comp_(data_[i], data_[parent])) {
                std::swap(data_[i], data_[parent]);
                i = parent;
            } else {
                break;
            }
        }
    }

    void siftDown(size_t i) {
        size_t n = data_.size();
        while (true) {
            size_t smallest = i;
            size_t left = 2 * i + 1;
            size_t right = 2 * i + 2;

            if (left < n && comp_(data_[left], data_[smallest])) {
                smallest = left;
            }
            if (right < n && comp_(data_[right], data_[smallest])) {
                smallest = right;
            }

            if (smallest != i) {
                std::swap(data_[i], data_[smallest]);
                i = smallest;
            } else {
                break;
            }
        }
    }

    std::vector<T> data_;  // 连续存储，缓存友好
    Compare comp_;
};

// ============================================
// B树：缓存友好的搜索树
// ============================================

// B树的每个节点包含多个键值，减少树高度
// 节点大小设计为接近缓存行大小

static constexpr size_t BTREE_ORDER = 16;  // B树的阶

template<typename Key, typename Value>
class BTreeNode {
public:
    // 节点内数据连续存储，一次缓存行加载可访问多个键
    Key keys[BTREE_ORDER - 1];
    Value values[BTREE_ORDER - 1];
    BTreeNode* children[BTREE_ORDER];
    size_t num_keys;
    bool is_leaf;

    BTreeNode(bool leaf = true) : num_keys(0), is_leaf(leaf) {
        for (size_t i = 0; i < BTREE_ORDER; ++i) {
            children[i] = nullptr;
        }
    }
};

// ============================================
// 缓存友好的哈希表（开放寻址法）
// ============================================

template<typename Key, typename Value>
class CacheFriendlyHashMap {
public:
    explicit CacheFriendlyHashMap(size_t capacity = 64)
        : capacity_(capacity), size_(0) {
        // 容量对齐到2的幂，方便位运算取模
        size_t cap = 1;
        while (cap < capacity) cap <<= 1;
        capacity_ = cap;

        slots_.resize(capacity_);
        // 初始化所有槽位为空
        for (auto& slot : slots_) {
            slot.occupied = false;
        }
    }

    bool insert(const Key& key, const Value& value) {
        if (size_ * 2 >= capacity_) {
            rehash(capacity_ * 2);
        }

        size_t idx = hash(key);
        size_t probe_count = 0;

        while (slots_[idx].occupied) {
            if (slots_[idx].key == key) {
                slots_[idx].value = value;  // 更新
                return true;
            }
            // 线性探测：连续内存访问，缓存友好
            idx = (idx + 1) & (capacity_ - 1);
            probe_count++;
            if (probe_count >= capacity_) return false;  // 表满
        }

        slots_[idx].key = key;
        slots_[idx].value = value;
        slots_[idx].occupied = true;
        size_++;
        return true;
    }

    Value* find(const Key& key) {
        size_t idx = hash(key);
        size_t probe_count = 0;

        while (slots_[idx].occupied) {
            if (slots_[idx].key == key) {
                return &slots_[idx].value;
            }
            idx = (idx + 1) & (capacity_ - 1);
            probe_count++;
            if (probe_count >= capacity_) break;
        }
        return nullptr;
    }

    size_t size() const { return size_; }

private:
    struct Slot {
        Key key;
        Value value;
        bool occupied;
    };

    std::vector<Slot> slots_;  // 连续存储
    size_t capacity_;
    size_t size_;

    size_t hash(const Key& key) const {
        // 简单的FNV哈希
        size_t h = 14695981039346656037ULL;
        const uint8_t* bytes = reinterpret_cast<const uint8_t*>(&key);
        for (size_t i = 0; i < sizeof(Key); ++i) {
            h ^= bytes[i];
            h *= 1099511628211ULL;
        }
        return h & (capacity_ - 1);
    }

    void rehash(size_t new_capacity) {
        std::vector<Slot> old_slots = std::move(slots_);
        capacity_ = new_capacity;
        slots_.resize(capacity_);
        for (auto& slot : slots_) slot.occupied = false;
        size_ = 0;

        for (auto& slot : old_slots) {
            if (slot.occupied) {
                insert(slot.key, slot.value);
            }
        }
    }
};
```

***

## 5. 缓存预取

### 5.1 软件预取

```cpp
#include <iostream>
#include <vector>
#include <cstdint>

// ============================================
// 软件预取指令
// ============================================

// GCC/Clang内置预取函数
// __builtin_prefetch(addr, rw, locality)
// rw: 0=读, 1=写
// locality: 0=不用保留, 1=L3, 2=L2, 3=L1

// 链表遍历中的预取
template<typename Node>
void prefetch_list_traverse(Node* head) {
    Node* current = head;
    while (current) {
        // 预取下一个节点到L1缓存
        if (current->next) {
            __builtin_prefetch(current->next, 0, 3);
        }
        // 处理当前节点
        process(current);
        current = current->next;
    }
}

// ============================================
// 数组遍历中的预取
// ============================================

// 预取距离：提前多少个元素开始预取
// 计算公式：预取距离 = (内存延迟 / 处理每个元素的时间)
// 例如：内存延迟100ns，每元素处理5ns → 预取距离=20

template<typename T, typename Func>
void prefetch_array_process(const T* data, size_t count, Func process) {
    // 预取距离（需要根据实际硬件调优）
    const size_t PREFETCH_DISTANCE = 16;

    for (size_t i = 0; i < count; ++i) {
        // 预取远处的数据
        if (i + PREFETCH_DISTANCE < count) {
            __builtin_prefetch(&data[i + PREFETCH_DISTANCE], 0, 1);
        }
        // 处理当前元素
        process(data[i]);
    }
}

// ============================================
// 链表节点的缓存友好设计
// ============================================

// 问题：传统链表节点分散在堆中，遍历缓存不友好
template<typename T>
struct ListNode {
    T data;
    ListNode* next;
};

// 解决方案1：预取下一个节点
template<typename T>
void traverse_with_prefetch(ListNode<T>* head) {
    ListNode<T>* current = head;
    while (current) {
        // 提前预取下一个节点
        if (current->next) {
            __builtin_prefetch(current->next, 0, 3);
        }
        // 处理当前节点（这段时间内下一个节点被加载到缓存）
        processNode(current->data);
        current = current->next;
    }
}

// 解决方案2：使用连续内存的链表
template<typename T>
class CompactList {
public:
    struct Node {
        T data;
        int32_t next_idx;  // 用索引代替指针，节省空间
    };

    void push_back(const T& value) {
        int32_t new_idx = static_cast<int32_t>(nodes_.size());
        nodes_.push_back({value, -1});

        if (tail_idx_ >= 0) {
            nodes_[tail_idx_].next_idx = new_idx;
        } else {
            head_idx_ = new_idx;
        }
        tail_idx_ = new_idx;
    }

    // 遍历时节点在连续内存中，缓存友好
    template<typename Func>
    void for_each(Func func) {
        int32_t idx = head_idx_;
        while (idx >= 0) {
            func(nodes_[idx].data);
            idx = nodes_[idx].next_idx;
        }
    }

private:
    std::vector<Node> nodes_;  // 连续存储
    int32_t head_idx_ = -1;
    int32_t tail_idx_ = -1;
};

// ============================================
// 矩阵乘法的缓存优化
// ============================================

// 朴素实现：缓存不友好
void matrix_mul_naive(const double* A, const double* B, double* C,
                      size_t N) {
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            double sum = 0.0;
            for (size_t k = 0; k < N; ++k) {
                sum += A[i * N + k] * B[k * N + j];  // B的访问不连续
            }
            C[i * N + j] = sum;
        }
    }
}

// 优化1：交换循环顺序（i-k-j），使B的访问连续
void matrix_mul_reordered(const double* A, const double* B, double* C,
                          size_t N) {
    // 先清零
    for (size_t i = 0; i < N * N; ++i) C[i] = 0.0;

    for (size_t i = 0; i < N; ++i) {
        for (size_t k = 0; k < N; ++k) {
            double a_ik = A[i * N + k];
            for (size_t j = 0; j < N; ++j) {
                C[i * N + j] += a_ik * B[k * N + j];  // B和C都连续访问
            }
        }
    }
}

// 优化2：分块矩阵乘法（缓存友好）
void matrix_mul_blocked(const double* A, const double* B, double* C,
                        size_t N, size_t BLOCK = 64) {
    // 先清零
    for (size_t i = 0; i < N * N; ++i) C[i] = 0.0;

    for (size_t ii = 0; ii < N; ii += BLOCK) {
        for (size_t kk = 0; kk < N; kk += BLOCK) {
            for (size_t jj = 0; jj < N; jj += BLOCK) {
                // 处理一个块
                size_t i_end = std::min(ii + BLOCK, N);
                size_t k_end = std::min(kk + BLOCK, N);
                size_t j_end = std::min(jj + BLOCK, N);

                for (size_t i = ii; i < i_end; ++i) {
                    for (size_t k = kk; k < k_end; ++k) {
                        double a_ik = A[i * N + k];
                        for (size_t j = jj; j < j_end; ++j) {
                            C[i * N + j] += a_ik * B[k * N + j];
                        }
                    }
                }
            }
        }
    }
}
```

***

## 6. 缓存一致性协议(MESI)

### 6.1 MESI协议原理

MESI是x86架构使用的缓存一致性协议，定义了缓存行的四种状态：

```
┌──────────────────────────────────────────────────────────┐
│                MESI协议状态                               │
├──────────────────────────────────────────────────────────┤
│                                                          │
│  ┌──────────┐                                            │
│  │ Modified │  已修改：数据已修改，只在本缓存中            │
│  │   (M)    │  必须写回主存，其他缓存必须失效              │
│  └────┬─────┘                                            │
│       │                                                  │
│  ┌────▼─────┐                                            │
│  │ Exclusive│  独占：数据与主存一致，只在本缓存中          │
│  │   (E)    │  可以直接修改，无需通知其他缓存              │
│  └────┬─────┘                                            │
│       │                                                  │
│  ┌────▼─────┐                                            │
│  │  Shared  │  共享：数据与主存一致，多个缓存可持有        │
│  │   (S)    │  修改时需要通知其他缓存失效                  │
│  └────┬─────┘                                            │
│       │                                                  │
│  ┌────▼─────┐                                            │
│  │ Invalid  │  无效：缓存行无效，需要从主存重新加载        │
│  │   (I)    │                                            │
│  └──────────┘                                            │
│                                                          │
│  状态转换：                                               │
│  本地读(M)→M  本地读(E)→E  本地读(S)→S  本地读(I)→E/S    │
│  本地写(M)→M  本地写(E)→M  本地写(S)→M  本地写(I)→M      │
│  远程读(M)→S  远程读(E)→S  远程读(S)→S  远程读(I)→I      │
│  远程写(M)→I  远程写(E)→I  远程写(S)→I  远程写(I)→I      │
└──────────────────────────────────────────────────────────┘
```

### 6.2 MESI对编程的影响

```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>

// ============================================
// MESI协议下的缓存行状态变化
// ============================================

// 示例1：独占状态的高效修改
void exclusive_state_demo() {
    // 线程独占的数据可以无开销修改
    alignas(64) int exclusive_data = 0;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i) {
        exclusive_data++;  // E状态 → M状态，无总线通信
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();

    std::cout << "独占修改: " << ns / 100000000 << " ns/次" << std::endl;
}

// 示例2：共享状态的修改开销
void shared_state_demo() {
    // 两个线程修改同一缓存行的不同变量
    alignas(64) struct {
        std::atomic<int> counter1{0};
        char padding[56];  // 填充到缓存行大小
        std::atomic<int> counter2{0};
    } shared;

    auto start = std::chrono::high_resolution_clock::now();

    std::thread t1([&]() {
        for (int i = 0; i < 10000000; ++i) {
            shared.counter1.fetch_add(1, std::memory_order_relaxed);
        }
    });

    std::thread t2([&]() {
        for (int i = 0; i < 10000000; ++i) {
            shared.counter2.fetch_add(1, std::memory_order_relaxed);
        }
    });

    t1.join();
    t2.join();

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "共享修改(有填充): " << ms << " ms" << std::endl;
}

// ============================================
// 缓存行乒乓效应（Cache Line Ping-Pong）
// ============================================

void cache_ping_pong_demo() {
    // 两个变量在同一个缓存行
    struct BadShared {
        std::atomic<int> a;  // 线程1修改
        std::atomic<int> b;  // 线程2修改
    };

    // 两个变量在不同缓存行
    struct GoodShared {
        alignas(64) std::atomic<int> a;
        alignas(64) std::atomic<int> b;
    };

    const int ITERATIONS = 10000000;

    // 测试乒乓效应
    {
        BadShared bad;
        bad.a = 0; bad.b = 0;

        auto start = std::chrono::high_resolution_clock::now();

        std::thread t1([&]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                bad.a.fetch_add(1, std::memory_order_relaxed);
            }
        });

        std::thread t2([&]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                bad.b.fetch_add(1, std::memory_order_relaxed);
            }
        });

        t1.join();
        t2.join();

        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "同缓存行(乒乓): " << ms << " ms" << std::endl;
    }

    {
        GoodShared good;
        good.a = 0; good.b = 0;

        auto start = std::chrono::high_resolution_clock::now();

        std::thread t1([&]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                good.a.fetch_add(1, std::memory_order_relaxed);
            }
        });

        std::thread t2([&]() {
            for (int i = 0; i < ITERATIONS; ++i) {
                good.b.fetch_add(1, std::memory_order_relaxed);
            }
        });

        t1.join();
        t2.join();

        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "不同缓存行:     " << ms << " ms" << std::endl;
    }
}
```

***

## 7. false sharing深度分析与消除

### 7.1 false sharing原理

false sharing发生在多个线程修改同一缓存行中的不同变量时，虽然逻辑上没有共享数据，但硬件层面产生了缓存一致性流量。

```
┌──────────────────────────────────────────────────────────┐
│              false sharing过程                            │
├──────────────────────────────────────────────────────────┤
│                                                          │
│  初始状态：两个核心都缓存了同一缓存行                      │
│                                                          │
│  Core 0 缓存行:  [ var_a | .... | var_b | .... ]  S状态  │
│  Core 1 缓存行:  [ var_a | .... | var_b | .... ]  S状态  │
│                                                          │
│  Step 1: Core 0 修改 var_a                               │
│  Core 0:  [ var_a*| .... | var_b | .... ]  M状态         │
│  Core 1:  [         无效                   ]  I状态       │
│                    ↑ 发送Invalidate消息                    │
│                                                          │
│  Step 2: Core 1 修改 var_b                               │
│  Core 1:  [ var_a | .... | var_b*| .... ]  M状态         │
│  Core 0:  [         无效                   ]  I状态       │
│                    ↑ 发送Invalidate消息                    │
│                                                          │
│  结果：虽然修改的是不同变量，但缓存行不断失效→性能灾难    │
└──────────────────────────────────────────────────────────┘
```

### 7.2 false sharing检测与消除

```cpp
#include <iostream>
#include <atomic>
#include <thread>
#include <chrono>
#include <vector>
#include <algorithm>

// ============================================
// 消除方案1：缓存行填充
// ============================================

// 通用缓存行填充模板
template<typename T>
struct CacheLinePadded {
    alignas(64) T value;
    // 编译器自动填充到64字节

    CacheLinePadded() = default;
    CacheLinePadded(const T& v) : value(v) {}

    T* operator&() { return &value; }
    T& operator*() { return value; }
    T* operator->() { return &value; }
};

// ============================================
// 消除方案2：线程本地存储
// ============================================

class ThreadLocalCounter {
public:
    void increment() {
        // 每个线程修改自己的计数器，无false sharing
        local_count_++;
    }

    int64_t getTotal() {
        // 汇总时才访问其他线程的数据
        int64_t total = local_count_;
        return total;
    }

private:
    static thread_local int64_t local_count_;
};

thread_local int64_t ThreadLocalCounter::local_count_ = 0;

// ============================================
// 消除方案3：分片计数器
// ============================================

class ShardedCounter {
public:
    explicit ShardedCounter(size_t num_shards)
        : shards_(num_shards) {}

    void increment(size_t shard_id) {
        // 每个分片在不同缓存行，无false sharing
        shards_[shard_id].count.fetch_add(1, std::memory_order_relaxed);
    }

    int64_t getTotal() const {
        int64_t total = 0;
        for (const auto& shard : shards_) {
            total += shard.count.load(std::memory_order_relaxed);
        }
        return total;
    }

private:
    struct Shard {
        alignas(64) std::atomic<int64_t> count{0};
    };

    std::vector<Shard> shards_;
};

// ============================================
// false sharing性能对比
// ============================================

void false_sharing_benchmark() {
    const int THREADS = 8;
    const int ITERATIONS = 10000000;

    // 测试1：有false sharing
    {
        struct SharedData {
            std::atomic<int64_t> counters[8];
        };

        SharedData data;
        for (int i = 0; i < THREADS; ++i) data.counters[i] = 0;

        auto start = std::chrono::high_resolution_clock::now();

        std::vector<std::thread> threads;
        for (int t = 0; t < THREADS; ++t) {
            threads.emplace_back([&data, t]() {
                for (int i = 0; i < ITERATIONS; ++i) {
                    data.counters[t].fetch_add(1, std::memory_order_relaxed);
                }
            });
        }

        for (auto& th : threads) th.join();

        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "有false sharing: " << ms << " ms" << std::endl;
    }

    // 测试2：使用分片计数器消除false sharing
    {
        ShardedCounter counter(THREADS);

        auto start = std::chrono::high_resolution_clock::now();

        std::vector<std::thread> threads;
        for (int t = 0; t < THREADS; ++t) {
            threads.emplace_back([&counter, t]() {
                for (int i = 0; i < ITERATIONS; ++i) {
                    counter.increment(t);
                }
            });
        }

        for (auto& th : threads) th.join();

        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "分片计数器:     " << ms << " ms" << std::endl;
        std::cout << "总计: " << counter.getTotal() << std::endl;
    }

    // 测试3：使用线程本地存储
    {
        auto start = std::chrono::high_resolution_clock::now();

        std::vector<std::thread> threads;
        std::vector<int64_t> results(THREADS);

        for (int t = 0; t < THREADS; ++t) {
            threads.emplace_back([&results, t]() {
                int64_t local = 0;  // 纯栈变量，无共享
                for (int i = 0; i < ITERATIONS; ++i) {
                    local++;
                }
                results[t] = local;
            });
        }

        for (auto& th : threads) th.join();

        auto end = std::chrono::high_resolution_clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        std::cout << "线程本地变量:   " << ms << " ms" << std::endl;
    }
}

// ============================================
// 使用perf检测false sharing
// ============================================

void detect_false_sharing_tips() {
    std::cout << "\n=== false sharing检测方法 ===" << std::endl;
    std::cout << "1. perf stat -e cache-misses ./program" << std::endl;
    std::cout << "   高缓存未命中率可能表示false sharing" << std::endl;
    std::cout << std::endl;
    std::cout << "2. perf record -e r10d1 ./program" << std::endl;
    std::cout << "   r10d1 = MEM_LOAD_UOPS_L3_HIT_RETIRED.XSNP_HITM" << std::endl;
    std::cout << "   高XSNP_HITM表示跨核缓存行修改（false sharing标志）" << std::endl;
    std::cout << std::endl;
    std::cout << "3. Intel VTune: 查看Cache Misses和False Sharing指标" << std::endl;
    std::cout << std::endl;
    std::cout << "4. 代码审查：检查多线程共享的原子变量是否对齐到缓存行" << std::endl;
}
```

***

## 8. 小结

### 核心要点回顾

| 优化技术 | 核心思想 | 性能提升 |
|---------|---------|---------|
| **缓存行对齐** | 避免跨缓存行访问 | 2-10倍 |
| **SoA/AoSoA** | 按访问模式分离数据 | 2-5倍 |
| **分块算法** | 让工作集适配缓存 | 3-10倍 |
| **软件预取** | 提前加载数据到缓存 | 1.5-3倍 |
| **消除false sharing** | 缓存行填充/分片 | 2-10倍 |
| **连续存储** | 用数组代替链表 | 5-50倍 |

### 缓存优化检查清单

```
□ 数据是否按访问频率分组（热/冷分离）？
□ 多线程共享变量是否缓存行对齐？
□ 是否使用SoA布局优化顺序访问？
□ 矩阵/数组操作是否分块？
□ 链表遍历是否使用预取？
□ 是否用连续存储替代指针链式结构？
□ 是否检测并消除了false sharing？
□ 是否利用了空间局部性（顺序访问）？
```

### 性能优化优先级

1. **数据布局** → SoA/AoSoA、字段排列
2. **算法分块** → 让工作集适配缓存
3. **消除false sharing** → 缓存行填充、分片
4. **预取优化** → 软件预取、访问模式
5. **连续存储** → 数组替代链表
