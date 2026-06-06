# 什么是AoS与SoA布局
> 📖 相关章节：[内存管理](../../01-C语言/09-内存管理.md)、[高级内存管理](../../09-系统级编程与性能调优/01-高级内存管理.md)、[CPU缓存优化](../../09-系统级编程与性能调优/02-CPU缓存优化.md)

> 要义概览：AoS（Array of Structures）是"一排人站队，每人带着自己的全部行李"，SoA（Structure of Arrays）是"按行李类型分别排队"——SoA让CPU缓存行一次搬运的都是同类数据，SIMD处理效率翻倍，但代码可读性下降。

***

### 1. 什么是 AoS 与 SoA

#### 1.1 AoS：结构体数组

**AoS（Array of Structures）** = 每个对象的所有属性连续存储，然后排成数组。这是最自然的编程方式。

```cpp
// AoS 布局：每个粒子是一个结构体，排成数组
struct Particle {
    float x, y, z;     // 位置
    float vx, vy, vz;  // 速度
    float mass;         // 质量
    float life;         // 生命值
};  // 32 字节

Particle particles[1000];
```

内存布局：

```
对象0                对象1                对象2
[x0 y0 z0 vx0 vy0 vz0 m0 l0] [x1 y1 z1 vx1 vy1 vz1 m1 l1] [x2 y2 z2 ...]
|←──── 32字节 ────→| |←──── 32字节 ────→| |←──── 32字节 ────→|
```

#### 1.2 SoA：数组结构体

**SoA（Structure of Arrays）** = 把所有对象的同一属性连续存储，不同属性分开成不同数组。

```cpp
// SoA 布局：所有属性分开存储
struct Particles {
    float x[1000], y[1000], z[1000];       // 位置
    float vx[1000], vy[1000], vz[1000];    // 速度
    float mass[1000];                       // 质量
    float life[1000];                       // 生命值
};

Particles particles;
```

内存布局：

```
[x0 x1 x2 x3 ... x999] [y0 y1 y2 y3 ... y999] [z0 z1 z2 ...] [vx0 vx1 vx2 ...]
|← 只存x →|            |← 只存y →|            |← 只存z →|    |← 只存vx →|
```

***

### 2. 缓存友好性对比

#### 2.1 AoS 的缓存行为

假设要更新所有粒子的位置（只访问 x, y, z, vx, vy, vz）：

```
缓存行加载（64字节）：
[x0 y0 z0 vx0 vy0 vz0 m0 l0 | x1 y1 z1 vx1 vy1 vz1 m1 l1]
|← 有用的数据 →|  无用  | ← 有用的数据 →|  无用  |

每个缓存行 64 字节中，只有 24 字节是有用的（x,y,z,vx,vy,vz）
缓存利用率：24/64 = 37.5%
```

**问题**：遍历时，每个缓存行有一半是暂时不需要的数据（mass, life），浪费了带宽。

#### 2.2 SoA 的缓存行为

同样更新所有粒子的位置：

```
缓存行加载（64字节）：
[x0 x1 x2 x3 x4 x5 x6 x7 x8 x9 x10 x11 x12 x13 x14 x15]
|← 全部是有用的数据 →|

缓存利用率：64/64 = 100%
```

**优势**：一个缓存行装满了 16 个 x 值，全部有用，缓存利用率 100%。

#### 2.3 量化对比

| 指标 | AoS | SoA |
|------|-----|-----|
| 更新位置的缓存利用率 | 37.5% | 100% |
| 更新位置需要的缓存行数 | ~500 | ~63 |
| 缓存缺失次数 | 多 | 少 |
| SIMD 友好度 | 差（需要 gather/scatter） | 好（连续内存） |

***

### 3. 何时用 AoS，何时用 SoA

#### 3.1 决策矩阵

| 场景 | 推荐布局 | 原因 |
|------|---------|------|
| 遍历所有对象的部分属性 | SoA | 只加载需要的属性，缓存利用率高 |
| 随机访问单个对象的所有属性 | AoS | 一次加载获得全部属性，空间局部性好 |
| 需要添加/删除对象 | AoS | SoA 添加/删除需要操作多个数组 |
| SIMD 向量化处理 | SoA | 连续内存，天然适合 SIMD |
| 对象间有引用关系 | AoS | 指针指向完整对象，更自然 |
| 属性间经常一起访问 | AoS | 同一缓存行中包含所有属性 |

#### 3.2 具体场景推荐

```cpp
// 场景1：粒子系统 → SoA
// 原因：每帧更新位置/速度，不需要 mass/life
// SoA 只加载 x/y/z/vx/vy/vz，缓存效率高

// 场景2：游戏实体管理 → AoS
// 原因：经常按 ID 查询完整实体，需要所有属性

// 场景3：数据库列存储 → SoA
// 原因：分析查询通常只涉及部分列

// 场景4：链表节点 → AoS
// 原因：访问节点时需要 next 指针和数据
```

***

### 4. AoSofA 混合布局

#### 4.1 为什么需要混合布局

纯 SoA 的问题：
- 添加/删除对象需要操作多个数组
- 随机访问单个对象的所有属性需要从多个数组中收集
- 代码复杂度高

纯 AoS 的问题：
- 遍历部分属性时缓存利用率低
- 不利于 SIMD

**AoSofA（Array of Structures of Arrays）** = 把对象分成小组（如 4 个或 8 个一组），组内用 SoA，组间用 AoS。

#### 4.2 AoSofA 的结构

```cpp
#include <cstdint>

// 每组 4 个粒子（适合 SSE 4-wide SIMD）
struct ParticleBlock {
    // 位置：4 个粒子的 x 连续，4 个 y 连续，4 个 z 连续
    float x[4];
    float y[4];
    float z[4];
    float vx[4];
    float vy[4];
    float vz[4];
    float mass[4];
    float life[4];
};  // 128 字节 = 2 个缓存行

// 粒子数组 = 多个 Block 组成的数组
ParticleBlock blocks[250];  // 250 × 4 = 1000 个粒子
```

内存布局：

```
Block 0:
[x0 x1 x2 x3] [y0 y1 y2 y3] [z0 z1 z2 z3] [vx0 vx1 vx2 vx3] [vy0 vy1 vy2 vy3] [vz0 vz1 vz2 vz3] [m0 m1 m2 m3] [l0 l1 l2 l3]
|← 16字节 →|  |← 16字节 →|  ...                                                              |← 16字节 →|  |← 16字节 →|

Block 1:
[x4 x5 x6 x7] [y4 y5 y6 y7] ...
```

#### 4.3 AoSofA 的优势

| 优势 | 说明 |
|------|------|
| SIMD 友好 | 组内同属性连续，直接加载到向量寄存器 |
| 缓存友好 | 处理一个 Block 时，数据集中在 2 个缓存行 |
| 灵活 | Block 大小可调（4 for SSE, 8 for AVX, 16 for AVX-512） |
| 代码简洁 | 比 SoA 更接近面向对象的编程方式 |

#### 4.4 AoSofA 的完整示例

```cpp
#include <cstdio>
#include <immintrin.h>
#include <vector>

// 每组 8 个粒子（适合 AVX 8-wide SIMD）
struct ParticleBlock {
    alignas(32) float x[8];      // 8 个粒子的 x 坐标
    alignas(32) float y[8];      // 8 个粒子的 y 坐标
    alignas(32) float z[8];      // 8 个粒子的 z 坐标
    alignas(32) float vx[8];     // 8 个粒子的 x 速度
    alignas(32) float vy[8];     // 8 个粒子的 y 速度
    alignas(32) float vz[8];     // 8 个粒子的 z 速度
    alignas(32) float mass[8];   // 8 个粒子的质量
    alignas(32) float life[8];   // 8 个粒子的生命值
};

class ParticleSystem {
private:
    std::vector<ParticleBlock> blocks;
    int count;  // 实际粒子数

public:
    ParticleSystem(int n) : count(n) {
        // 向上取整到 Block 大小的倍数
        int block_count = (n + 7) / 8;
        blocks.resize(block_count);
    }

    // 使用 AVX 更新所有粒子的位置
    void update(float dt) {
        for (auto& block : blocks) {
            // 加载 8 个 x 坐标
            __m256 px = _mm256_load_ps(block.x);
            __m256 py = _mm256_load_ps(block.y);
            __m256 pz = _mm256_load_ps(block.z);

            // 加载 8 个速度
            __m256 pvx = _mm256_load_ps(block.vx);
            __m256 pvy = _mm256_load_ps(block.vy);
            __m256 pvz = _mm256_load_ps(block.vz);

            // 位置 += 速度 × 时间
            __m256 sdt = _mm256_set1_ps(dt);
            px = _mm256_add_ps(px, _mm256_mul_ps(pvx, sdt));
            py = _mm256_add_ps(py, _mm256_mul_ps(pvy, sdt));
            pz = _mm256_add_ps(pz, _mm256_mul_ps(pvz, sdt));

            // 存回
            _mm256_store_ps(block.x, px);
            _mm256_store_ps(block.y, py);
            _mm256_store_ps(block.z, pz);
        }
    }

    // 访问单个粒子（比 SoA 方便）
    void get_particle(int index, float& out_x, float& out_y, float& out_z) {
        int block_idx = index / 8;
        int elem_idx = index % 8;
        out_x = blocks[block_idx].x[elem_idx];
        out_y = blocks[block_idx].y[elem_idx];
        out_z = blocks[block_idx].z[elem_idx];
    }

    // 设置单个粒子
    void set_particle(int index, float x, float y, float z,
                      float vx, float vy, float vz, float mass, float life) {
        int block_idx = index / 8;
        int elem_idx = index % 8;
        blocks[block_idx].x[elem_idx] = x;
        blocks[block_idx].y[elem_idx] = y;
        blocks[block_idx].z[elem_idx] = z;
        blocks[block_idx].vx[elem_idx] = vx;
        blocks[block_idx].vy[elem_idx] = vy;
        blocks[block_idx].vz[elem_idx] = vz;
        blocks[block_idx].mass[elem_idx] = mass;
        blocks[block_idx].life[elem_idx] = life;
    }
};

int main() {
    ParticleSystem ps(1000);

    // 初始化
    for (int i = 0; i < 1000; ++i) {
        ps.set_particle(i, i * 1.0f, 0, 0, 1.0f, 0.5f, 0, 1.0f, 100.0f);
    }

    // 更新
    ps.update(0.016f);  // 16ms 一帧

    // 查询
    float x, y, z;
    ps.get_particle(0, x, y, z);
    printf("粒子0位置: (%.2f, %.2f, %.2f)\n", x, y, z);

    return 0;
}
```

***

### 5. 性能对比实战

#### 5.1 AoS 版本

```cpp
#include <cstdio>
#include <chrono>
#include <vector>

struct ParticleAoS {
    float x, y, z;
    float vx, vy, vz;
    float mass, life;
};

void bench_aos(int n) {
    std::vector<ParticleAoS> particles(n);

    // 初始化
    for (int i = 0; i < n; ++i) {
        particles[i] = {i * 1.0f, 0, 0, 1.0f, 0.5f, 0, 1.0f, 100.0f};
    }

    auto start = std::chrono::high_resolution_clock::now();

    // 更新位置
    for (int frame = 0; frame < 1000; ++frame) {
        float dt = 0.016f;
        for (auto& p : particles) {
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            p.z += p.vz * dt;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printf("AoS: %lld ms\n", ms.count());
}

int main() {
    bench_aos(1000000);
    return 0;
}
```

#### 5.2 SoA 版本

```cpp
#include <cstdio>
#include <chrono>
#include <vector>

struct ParticlesSoA {
    std::vector<float> x, y, z;
    std::vector<float> vx, vy, vz;
    std::vector<float> mass, life;

    ParticlesSoA(int n)
        : x(n), y(n), z(n), vx(n), vy(n), vz(n), mass(n), life(n) {}
};

void bench_soa(int n) {
    ParticlesSoA particles(n);

    // 初始化
    for (int i = 0; i < n; ++i) {
        particles.x[i] = i * 1.0f;
        particles.y[i] = 0;
        particles.z[i] = 0;
        particles.vx[i] = 1.0f;
        particles.vy[i] = 0.5f;
        particles.vz[i] = 0;
        particles.mass[i] = 1.0f;
        particles.life[i] = 100.0f;
    }

    auto start = std::chrono::high_resolution_clock::now();

    // 更新位置
    for (int frame = 0; frame < 1000; ++frame) {
        float dt = 0.016f;
        for (int i = 0; i < n; ++i) {
            particles.x[i] += particles.vx[i] * dt;
            particles.y[i] += particles.vy[i] * dt;
            particles.z[i] += particles.vz[i] * dt;
        }
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printf("SoA: %lld ms\n", ms.count());
}

int main() {
    bench_soa(1000000);
    return 0;
}
```

#### 5.3 典型结果

```
100万个粒子，1000帧更新位置：

AoS:   ~1200 ms
SoA:   ~600 ms
AoSofA:~550 ms（含 SIMD 优化）

SoA 比 AoS 快约 2x，主要因为缓存命中率更高。
```

***

### 6. SoA 与 SIMD 的天然配合

#### 6.1 SoA 直接加载到向量寄存器

```cpp
#include <immintrin.h>

// SoA 布局下，x 数组是连续的，直接加载到 AVX 寄存器
void update_soa_simd(float* x, float* y, float* z,
                     const float* vx, const float* vy, const float* vz,
                     int n, float dt) {
    __m256 sdt = _mm256_set1_ps(dt);
    int i = 0;

    // 每次处理 8 个粒子
    for (; i + 8 <= n; i += 8) {
        __m256 px = _mm256_loadu_ps(&x[i]);
        __m256 py = _mm256_loadu_ps(&y[i]);
        __m256 pz = _mm256_loadu_ps(&z[i]);

        __m256 pvx = _mm256_loadu_ps(&vx[i]);
        __m256 pvy = _mm256_loadu_ps(&vy[i]);
        __m256 pvz = _mm256_loadu_ps(&vz[i]);

        px = _mm256_add_ps(px, _mm256_mul_ps(pvx, sdt));
        py = _mm256_add_ps(py, _mm256_mul_ps(pvy, sdt));
        pz = _mm256_add_ps(pz, _mm256_mul_ps(pvz, sdt));

        _mm256_storeu_ps(&x[i], px);
        _mm256_storeu_ps(&y[i], py);
        _mm256_storeu_ps(&z[i], pz);
    }

    // 处理剩余元素
    for (; i < n; ++i) {
        x[i] += vx[i] * dt;
        y[i] += vy[i] * dt;
        z[i] += vz[i] * dt;
    }
}
```

#### 6.2 AoS 需要 gather/scatter

```cpp
// AoS 布局下，需要 gather 指令收集分散的数据
void update_aos_simd(ParticleAoS* particles, int n, float dt) {
    __m256 sdt = _mm256_set1_ps(dt);

    for (int i = 0; i + 8 <= n; i += 8) {
        // 需要收集 8 个不连续的 x 值（步长 32 字节）
        // AVX2 的 gather 指令效率远低于连续加载
        __m256i indices = _mm256_setr_epi32(
            i * 8, (i + 1) * 8, (i + 2) * 8, (i + 3) * 8,
            (i + 4) * 8, (i + 5) * 8, (i + 6) * 8, (i + 7) * 8
        );

        // gather 比连续加载慢 3-5 倍
        __m256 px = _mm256_i32gather_ps(
            reinterpret_cast<const float*>(particles), indices, 4);

        // ... 类似地收集 vx, y, vy, z, vz

        // 更新后还需要 scatter（AVX-512 才有硬件支持）
        // 没有 scatter 就只能逐个写回
    }
}
```

***

### 7. 实际项目中的选择策略

#### 7.1 游戏引擎中的混合策略

```cpp
// ECS（Entity Component System）天然就是 SoA 思想
// 每种 Component 独立存储，系统只访问需要的 Component

class PositionComponent {
    std::vector<float> x, y, z;  // SoA 布局
};

class VelocityComponent {
    std::vector<float> vx, vy, vz;  // SoA 布局
};

class RenderComponent {
    std::vector<int> mesh_id;       // SoA 布局
    std::vector<int> texture_id;
};

// 移动系统只访问 Position + Velocity
void move_system(PositionComponent& pos, const VelocityComponent& vel, float dt) {
    int n = pos.x.size();
    for (int i = 0; i < n; ++i) {
        pos.x[i] += vel.vx[i] * dt;
        pos.y[i] += vel.vy[i] * dt;
        pos.z[i] += vel.vz[i] * dt;
    }
}
```

#### 7.2 数据库中的列存储

```
行存储（AoS）：适合 OLTP（事务处理）
  | id | name | age | salary |
  | 1  | Alice| 30  | 5000   |
  | 2  | Bob  | 25  | 4000   |

列存储（SoA）：适合 OLAP（分析查询）
  | id: 1, 2, 3, ...        |
  | name: Alice, Bob, ...    |
  | age: 30, 25, 35, ...    |
  | salary: 5000, 4000, ...  |

查询 "SELECT AVG(salary)" → 列存储只需扫描 salary 列
行存储需要扫描所有列，浪费 I/O
```

***

### 8. 常见误区

| 误区 | 事实 |
|------|------|
| "SoA 总是比 AoS 快" | 随机访问所有属性时，AoS 更快 |
| "SoA 代码一定很丑" | 用模板和宏可以封装得很优雅 |
| "AoSofA 太复杂不值得" | 在 SIMD 场景下，性能提升显著 |
| "ECS 就是 SoA" | ECS 是 SoA 思想的一种实现，但不完全等同 |
| "改布局就能解决所有性能问题" | 布局优化是重要手段，但不是银弹 |

***

### 9. 总结

| 要点 | 说明 |
|------|------|
| AoS | 对象连续存储，面向对象自然，随机访问快 |
| SoA | 属性连续存储，缓存友好，SIMD 友好，遍历快 |
| AoSofA | 混合布局，兼顾两者优势，SIMD 最佳搭档 |
| 选 AoS | 随机访问、属性总是一起用、需要增删对象 |
| 选 SoA | 批量遍历部分属性、SIMD 向量化、数据量大 |
| 选 AoSofA | 需要 SIMD + 适度灵活性 |