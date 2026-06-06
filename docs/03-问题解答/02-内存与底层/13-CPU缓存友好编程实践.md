# CPU 缓存友好编程实践
> 📖 相关章节：[指针](../../01-C语言/06-指针.md)、[结构体与联合体](../../01-C语言/08-结构体与联合体.md)、[内存管理](../../01-C语言/09-内存管理.md)、[智能指针](../../02-CPP/08-智能指针与内存管理.md)、[内存模型](../../02-CPP/32-内存模型.md)

> "把常用工具放桌面（L1），不常用的放抽屉（L2），偶尔用的放仓库（内存）"——顺应缓存特性写代码，性能翻倍不是梦。

***

### 1. 核心定义

**CPU 缓存友好编程** = 编写利用 CPU 缓存特性的代码，减少缓存未命中（Cache Miss），让 CPU 尽量从高速缓存而非慢速内存中获取数据。

关键点：**CPU 访问缓存比访问内存快 100 倍，缓存友好的代码和缓存不友好的代码性能差距可达数倍甚至数十倍**。

***

### 2. 生活类比

**办公桌理论**：

| 存储层级 | 类比 | 访问时间 | 容量 |
|------|------|------|------|
| L1 缓存 | 桌面，伸手就拿到 | ~1 纳秒 | ~32KB |
| L2 缓存 | 抽屉，转身拉开 | ~4 纳秒 | ~256KB |
| L3 缓存 | 文件柜，走两步 | ~12 纳秒 | ~数MB |
| 内存 | 仓库，下楼去找 | ~100 纳秒 | ~数GB |

**缓存友好 = 把常用东西放桌面，按顺序用，别东翻西找**

- ✅ 顺序翻文件（顺序访问）→ 桌面够用，一扫而过
- ❌ 随机翻文件（随机访问）→ 桌面放不下，反复跑仓库
- ❌ 桌面堆满不用的东西（缓存行浪费）→ 常用的反而没地方放

***

### 3. AoS vs SoA 布局对比

#### 1. AoS（Array of Structures）= 结构体数组

```cpp
struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float mass;
    float color_r, color_g, color_b;
};

Particle particles[10000];
```

内存布局：

```
[x,y,z,vx,vy,vz,mass,r,g,b] [x,y,z,vx,vy,vz,mass,r,g,b] [x,y,z,...]
 ↑ Particle 0                 ↑ Particle 1                  ↑ Particle 2
```

#### 2. SoA（Structure of Arrays）= 数组结构体

```cpp
struct Particles {
    float x[10000], y[10000], z[10000];
    float vx[10000], vy[10000], vz[10000];
    float mass[10000];
    float color_r[10000], color_g[10000], color_b[10000];
};

Particles particles;
```

内存布局：

```
[x0,x1,x2,...,x9999] [y0,y1,y2,...,y9999] [z0,z1,z2,...,z9999] ...
 ↑ 所有 x 连续          ↑ 所有 y 连续          ↑ 所有 z 连续
```

#### 3. 对比

| 维度 | AoS | SoA |
|------|------|------|
| 内存布局 | 一个对象的所有字段连续 | 所有对象的同一字段连续 |
| 访问全部字段 | ✅ 缓存友好 | ❌ 跳跃访问 |
| 访问单个字段 | ❌ 缓存行中有不需要的字段 | ✅ 缓存行全是需要的数据 |
| 代码可读性 | ✅ 直觉 | ⚠️ 稍复杂 |
| SIMD 友好 | ❌ 需要收集 | ✅ 天然连续 |

**原则**：只访问部分字段 → SoA；经常访问全部字段 → AoS

***

### 4. 代码示例：AoS 和 SoA 的性能差异

#### 1. 场景：只更新粒子位置（只访问 x, y, z, vx, vy, vz）

```cpp
#include <iostream>
#include <chrono>
#include <cmath>

const int N = 10'000'000;
const float dt = 0.016f;

struct Particle {
    float x, y, z;
    float vx, vy, vz;
    float mass;
    float color_r, color_g, color_b;
};

struct Particles {
    float* x;  float* y;  float* z;
    float* vx; float* vy; float* vz;
    float* mass;
    float* color_r; float* color_g; float* color_b;

    Particles() {
        x = new float[N](); y = new float[N](); z = new float[N]();
        vx = new float[N](); vy = new float[N](); vz = new float[N]();
        mass = new float[N]();
        color_r = new float[N](); color_g = new float[N](); color_b = new float[N]();
    }

    ~Particles() {
        delete[] x; delete[] y; delete[] z;
        delete[] vx; delete[] vy; delete[] vz;
        delete[] mass;
        delete[] color_r; delete[] color_g; delete[] color_b;
    }
};

void bench_aos(Particle* particles) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        particles[i].x += particles[i].vx * dt;
        particles[i].y += particles[i].vy * dt;
        particles[i].z += particles[i].vz * dt;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "AoS: " << ms << "ms" << std::endl;
}

void bench_soa(Particles& p) {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        p.x[i] += p.vx[i] * dt;
        p.y[i] += p.vy[i] * dt;
        p.z[i] += p.vz[i] * dt;
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    std::cout << "SoA: " << ms << "ms" << std::endl;
}

int main() {
    auto* aos = new Particle[N]();
    Particles soa;

    bench_aos(aos);
    bench_soa(soa);

    delete[] aos;
}
```

**典型结果**：

```
AoS: 45ms
SoA: 22ms
```

**原因分析**：

```
AoS 缓存行内容（40字节/对象，缓存行64字节）:
[x,y,z,vx,vy,vz,mass,r,g,b, x,y,...]
                  ^^^^^^^^^^^^^^^^
                  mass, color 不需要但占了缓存行空间

SoA 缓存行内容:
[x0,x1,x2,...,x15]  ← 16个x连续，全是有用数据
```

AoS 每次加载缓存行，有 40% 是不需要的 `mass` 和 `color` 字段，浪费了缓存行空间。SoA 加载的缓存行全是需要的 `x` 值，利用率 100%。

***

### 5. 缓存行对齐

#### 1. 缓存行大小

主流 CPU 缓存行大小为 **64 字节**。内存按缓存行加载，即使只读 1 字节，也会加载整行 64 字节。

#### 2. 对齐的两种用法

**1. 避免伪共享（多线程）**

```cpp
struct alignas(64) PaddedCounter {
    uint64_t value;
};
```

详见 [FAQ 84](../02-内存与底层/11-什么是伪共享false-sharing.md)。

**2. 避免数据跨缓存行（单线程）**

```cpp
struct Bad {
    uint8_t flag;
};

struct Good {
    alignas(64) uint8_t flag;
};
```

如果 `flag` 跨两个缓存行，一次读取要访问两行。对齐后保证在单行内。

#### 3. 检查对齐

```cpp
#include <iostream>
#include <cstddef>

struct alignas(64) AlignedData {
    int values[16];
};

int main() {
    AlignedData d;
    std::cout << "alignment: " << alignof(AlignedData) << std::endl;
    std::cout << "address % 64: " << (reinterpret_cast<uintptr_t>(&d) % 64) << std::endl;
}
```

***

### 6. 数据局部性原则

#### 1. 时间局部性（Temporal Locality）

**最近访问的数据，很可能很快再次访问**。

```cpp
for (int i = 0; i < N; ++i) {
    sum += data[i];
}
```

`sum` 在每次迭代都被访问，编译器通常把它优化到寄存器中。

#### 2. 空间局部性（Spatial Locality）

**访问某个地址后，很可能很快访问附近的地址**。

```cpp
int sum = 0;
for (int i = 0; i < N; ++i) {
    sum += data[i];
}
```

顺序访问 `data[0]`, `data[1]`, `data[2]`... 它们在内存中相邻，加载 `data[0]` 时缓存行已经预取了后续数据。

#### 3. 顺序访问 vs 随机访问

```cpp
#include <iostream>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>

int main() {
    const int N = 10'000'000;
    std::vector<int> data(N);
    std::vector<int> indices(N);

    for (int i = 0; i < N; ++i) {
        data[i] = i;
        indices[i] = i;
    }

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(indices.begin(), indices.end(), g);

    long long sum = 0;

    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        sum += data[i];
    }
    auto end = std::chrono::high_resolution_clock::now();
    std::cout << "sequential: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;

    sum = 0;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        sum += data[indices[i]];
    }
    end = std::chrono::high_resolution_clock::now();
    std::cout << "random: "
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count() << "ms" << std::endl;
}
```

**典型结果**：

```
sequential: 8ms
random: 45ms
```

随机访问比顺序访问慢 5~6 倍，因为缓存预取失效，每次都可能缓存未命中。

#### 4. 实践原则

| 做法 | 缓存效果 | 说明 |
|------|:---:|------|
| 顺序遍历数组 | ✅ 好 | 空间局部性，预取器高效工作 |
| 随机访问数组 | ❌ 差 | 预取器无法预测，频繁缓存未命中 |
| 链表遍历 | ❌ 差 | 节点不连续，每次跳转都可能未命中 |
| 连续容器（vector） | ✅ 好 | 内存连续，缓存友好 |
| 分层/分块处理 | ✅ 好 | 数据分块，每块适配缓存大小 |

***

### 7. 分支预测友好

CPU 有分支预测器，预测 `if/else` 走哪条路。预测正确几乎零开销，预测错误要清空流水线，代价约 **15~20 个时钟周期**。

#### 1. 不友好的写法：随机分支

```cpp
void process_random(std::vector<int>& data) {
    for (auto& x : data) {
        if (x > threshold) {
            x = expensive_op_a(x);
        } else {
            x = expensive_op_b(x);
        }
    }
}
```

数据随机分布，分支预测器频繁猜错。

#### 2. 友好的写法：先排序再分支

```cpp
void process_sorted(std::vector<int>& data) {
    std::sort(data.begin(), data.end(),
        [](int a, int b) { return a <= threshold && b > threshold; });

    for (auto& x : data) {
        if (x > threshold) {
            x = expensive_op_a(x);
        } else {
            x = expensive_op_b(x);
        }
    }
}
```

排序后，前半段全走 else，后半段全走 if，分支预测几乎 100% 正确。

#### 3. 更好的写法：消除分支

```cpp
void process_branchless(std::vector<int>& data) {
    for (auto& x : data) {
        int mask = -(x > threshold);
        x = (expensive_op_a(x) & mask) | (expensive_op_b(x) & ~mask);
    }
}
```

没有分支，没有预测失败。但两个操作都会执行，只适合操作本身很轻量的场景。

#### 4. 分支预测性能对比

| 方式 | 预测失败率 | 性能 |
|------|:---:|:---:|
| 随机数据 + if/else | ~50% | 慢 |
| 排序数据 + if/else | ~0% | 快 |
| 无分支（branchless） | 0% | 最快（操作轻量时） |

***

### 8. 与缓存命中率和伪共享的关系

[FAQ 04](../01-基础概念/04-什么是缓存命中率.md) 讲了缓存命中率的概念，[FAQ 84](../02-内存与底层/11-什么是伪共享false-sharing.md) 讲了伪共享问题。本篇是它们的**实践指南**：

```
缓存命中率（FAQ 04）← 这是结果指标
       ↑
       |  影响因素
       |
伪共享（FAQ 84）← 多核场景的缓存杀手
       ↑
       |  解决方案
       |
缓存友好编程（本篇）← 从代码层面提升命中率的实践
```

| 实践 | 解决什么问题 | 对应章节 |
|------|------|------|
| SoA 布局 | 缓存行利用率低 | 97.3 ~ 97.4 |
| 缓存行对齐 | 伪共享 / 跨行访问 | 97.5 |
| 顺序访问 | 缓存预取失效 | 97.6 |
| 分支预测友好 | 流水线清空 | 97.7 |
| 数据分块 | 工作集超出缓存容量 | 97.6 |

**一句话**：缓存友好编程 = 让数据在缓存中待得更久 + 让 CPU 预取更准 + 让分支预测更对。

***

### 9. 极简总结

**缓存友好 = 顺序访问、紧凑布局、对齐对行、分支可预测**

| 要点 | 做法 | 效果 |
|------|------|------|
| 数据布局 | 只访问部分字段时用 SoA | 缓存行利用率 ↑ |
| 访问模式 | 顺序遍历 > 随机访问 | 预取器命中率 ↑ |
| 缓存行对齐 | alignas(64) / padding | 避免伪共享、跨行访问 |
| 分支预测 | 排序数据 / 消除分支 | 流水线冲刷 ↓ |
| 容器选择 | vector > list | 内存连续，缓存友好 |
| 一句话 | 让 CPU 尽量从缓存拿数据，少跑内存 | 性能 ↑ 数倍 |

***

### 相关阅读

- [什么是伪共享false-sharing](./11-什么是伪共享false-sharing.md)
- [结构体内存对齐](./01-结构体内存对齐.md)