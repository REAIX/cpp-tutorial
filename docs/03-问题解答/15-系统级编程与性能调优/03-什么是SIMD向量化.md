# 什么是SIMD向量化
> 📖 相关章节：[内存管理](../../01-C语言/09-内存管理.md)、[高级内存管理](../../09-系统级编程与性能调优/01-高级内存管理.md)、[CPU缓存优化](../../09-系统级编程与性能调优/02-CPU缓存优化.md)

> 核心要义：SIMD（Single Instruction Multiple Data）就是一条指令同时处理多个数据——普通加法一次加两个数，SIMD加法一次加4个、8个甚至16个数，是CPU层面最直接的并行加速手段。

***

### 1. SIMD 的基本原理

#### 1.1 标量 vs 向量

```
标量运算（普通指令）：
ADD r1, r2    →  r1 = r1 + r2    （1次加法）

SIMD运算（向量指令）：
VADDPS ymm1, ymm2, ymm3  →  ymm1[0]=ymm2[0]+ymm3[0]
                             ymm1[1]=ymm2[1]+ymm3[1]
                             ymm1[2]=ymm2[2]+ymm3[2]
                             ymm1[3]=ymm2[3]+ymm3[3]
                             ymm1[4]=ymm2[4]+ymm3[4]
                             ymm1[5]=ymm2[5]+ymm3[5]
                             ymm1[6]=ymm2[6]+ymm3[6]
                             ymm1[7]=ymm2[7]+ymm3[7]
                             （8次加法，1条指令）
```

#### 1.2 类比理解

| 概念 | 类比 |
|------|------|
| 标量运算 | 一个工人一次搬一箱货 |
| SIMD 运算 | 一个工人开着叉车一次搬8箱货 |
| 向量寄存器 | 更宽的叉车（128/256/512位） |
| 自动向量化 | 工厂自动安排叉车（编译器优化） |
| 手动向量化 | 手动驾驶叉车（写 intrinsics） |

***

### 2. SSE/AVX/AVX-512 的区别

#### 2.1 x86 SIMD 指令集演进

| 指令集 | 年份 | 寄存器宽度 | 寄存器名 | 数量 | 同时处理 float | 同时处理 int32 |
|--------|------|-----------|---------|------|---------------|---------------|
| MMX | 1997 | 64位 | MM0-7 | 8 | - | 2 |
| SSE | 1999 | 128位 | XMM0-15 | 16 | 4 | 4 |
| SSE2 | 2001 | 128位 | XMM0-15 | 16 | 4 | 4 |
| AVX | 2011 | 256位 | YMM0-15 | 16 | 8 | - |
| AVX2 | 2013 | 256位 | YMM0-15 | 16 | 8 | 8 |
| AVX-512 | 2015 | 512位 | ZMM0-31 | 32 | 16 | 16 |

#### 2.2 寄存器关系

```
ZMM0（512位）─────────────────────────────────────────
├─────────────────── YMM0（256位）────────────────────┤
├────────── XMM0（128位）──────────┤                   │
├─────────────────────────────────────────────────────┤

ZMM0 = YMM0 + 额外256位
YMM0 = XMM0 + 额外128位
```

#### 2.3 ARM NEON

| 指令集 | 寄存器宽度 | 寄存器名 | 数量 | 同时处理 float |
|--------|-----------|---------|------|---------------|
| NEON | 128位 | Q0-Q31 | 32 | 4 |
| SVE | 可变(128-2048位) | Z0-Z31 | 32 | 可变 |

#### 2.4 各指令集的关键特性

| 特性 | SSE | AVX | AVX2 | AVX-512 |
|------|-----|-----|------|---------|
| 浮点加法 | ✅ | ✅ | ✅ | ✅ |
| 整数运算 | ✅ | ❌ | ✅ | ✅ |
| FMA（融合乘加） | ❌ | 部分 | ✅ | ✅ |
| 掩码操作 | ❌ | ❌ | ❌ | ✅（8个掩码寄存器k0-k7） |
| Gather | ❌ | ❌ | ✅ | ✅ |
| Scatter | ❌ | ❌ | ❌ | ✅ |
| 降频风险 | 无 | 无 | 轻微 | 严重（部分CPU） |

#### 2.5 AVX-512 降频问题

```
AVX-512 在某些 Intel CPU 上会导致 CPU 降频：
- 使用 512 位指令 → CPU 降低频率以控制功耗
- 降频幅度：10%~30%
- 降频影响：即使非 AVX-512 代码也会变慢
- 恢复时间：停止使用后需要几百微秒恢复

因此：短时间使用 AVX-512 可能得不偿失
      长时间密集计算才值得使用 AVX-512
```

***

### 3. Intrinsics 编程基础

#### 3.1 什么是 Intrinsics

Intrinsics 是编译器提供的内置函数，对应特定的 SIMD 指令，比汇编可读性好，比纯 C 性能高。

```cpp
// 命名规则：_mm<位数>_<操作>_<数据类型>
_mm256_add_ps   // 256位，加法，packed single-precision(float)
_mm256_add_pd   // 256位，加法，packed double-precision(double)
_mm_add_epi32   // 128位，加法，extended packed int32
_mm256_mul_ps   // 256位，乘法，packed float

// 后缀含义：
// ps = packed single (多个float)
// pd = packed double (多个double)
// epi32 = extended packed int32
// epi8/epi16/epi64 = packed int8/int16/int64
// ss = scalar single (单个float)
// sd = scalar double (单个double)
```

#### 3.2 头文件

```cpp
#include <xmmintrin.h>   // SSE
#include <emmintrin.h>   // SSE2
#include <pmmintrin.h>   // SSE3
#include <tmmintrin.h>   // SSSE3
#include <smmintrin.h>   // SSE4.1
#include <nmmintrin.h>   // SSE4.2
#include <immintrin.h>   // AVX, AVX2, AVX-512（包含以上所有）
#include <arm_neon.h>    // ARM NEON
```

#### 3.3 基本操作示例

```cpp
#include <immintrin.h>
#include <cstdio>

int main() {
    // ===== 加载与存储 =====

    // 对齐加载（数据必须 32 字节对齐）
    alignas(32) float a[8] = {1, 2, 3, 4, 5, 6, 7, 8};
    alignas(32) float b[8] = {8, 7, 6, 5, 4, 3, 2, 1};
    alignas(32) float result[8];

    __m256 va = _mm256_load_ps(a);   // 对齐加载
    __m256 vb = _mm256_load_ps(b);

    // 非对齐加载（数据不需要对齐，但稍慢）
    // __m256 va = _mm256_loadu_ps(a);

    // ===== 算术运算 =====

    // 加法
    __m256 vsum = _mm256_add_ps(va, vb);
    // 结果：{9, 9, 9, 9, 9, 9, 9, 9}

    // 减法
    __m256 vdiff = _mm256_sub_ps(va, vb);
    // 结果：{-7, -5, -3, -1, 1, 3, 5, 7}

    // 乘法
    __m256 vmul = _mm256_mul_ps(va, vb);
    // 结果：{8, 14, 18, 20, 20, 18, 14, 8}

    // 融合乘加（FMA）：result = a * b + c
    __m256 vfma = _mm256_fmadd_ps(va, vb, vsum);
    // 结果：{8+9, 14+9, 18+9, 20+9, 20+9, 18+9, 14+9, 8+9}

    // 存储
    _mm256_store_ps(result, vsum);

    for (int i = 0; i < 8; ++i) {
        printf("%.0f ", result[i]);
    }
    printf("\n");

    return 0;
}
```

#### 3.4 常用操作速查

```cpp
// ===== 广播（将一个值复制到所有通道）=====
__m256 v = _mm256_set1_ps(3.14f);  // {3.14, 3.14, 3.14, 3.14, 3.14, 3.14, 3.14, 3.14}

// ===== 设置（按顺序设置所有通道）=====
__m256 v2 = _mm256_set_ps(8, 7, 6, 5, 4, 3, 2, 1);  // 注意：参数顺序是反的
// 结果：{1, 2, 3, 4, 5, 6, 7, 8}

// ===== 清零 =====
__m256 zero = _mm256_setzero_ps();

// ===== 比较操作 =====
__m256 cmp = _mm256_cmp_ps(va, vb, _CMP_GT_OS);  // 大于比较
// 结果：全1位(真) 或 全0位(假)

// ===== 位运算 =====
__m256 vand = _mm256_and_ps(va, vb);    // 按位与
__m256 vor = _mm256_or_ps(va, vb);      // 按位或
__m256 vxor = _mm256_xor_ps(va, vb);    // 按位异或

// ===== 水平运算（跨通道）=====
// 注意：水平运算效率较低，尽量避免
__m256 hadd = _mm256_hadd_ps(va, vb);   // 水平加法

// ===== 求水平求和（所有通道之和）=====
// 没有直接指令，需要手动实现
float horizontal_sum(__m256 v) {
    // 高128位 + 低128位
    __m128 hi = _mm256_extractf128_ps(v, 1);
    __m128 lo = _mm256_castps256_ps128(v);
    __m128 sum128 = _mm_add_ps(hi, lo);

    // 逐级归约
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);

    return _mm_cvtss_f32(sum128);
}
```

***

### 4. 自动向量化

#### 4.1 编译器自动向量化

编译器在 `-O2`/`-O3` 下会自动将简单循环转换为 SIMD 指令。

```cpp
// 编译器可以自动向量化的循环
void add_arrays(float* a, const float* b, int n) {
    for (int i = 0; i < n; ++i) {
        a[i] += b[i];
    }
}
// 编译选项：g++ -O3 -mavx2 main.cpp
// 编译器会自动生成 AVX2 向量指令
```

#### 4.2 自动向量化条件

| 条件 | 说明 |
|------|------|
| 循环次数已知 | 编译器需要知道迭代次数 |
| 无数据依赖 | 迭代之间没有依赖关系 |
| 无指针别名 | 编译器确定指针不重叠 |
| 无分支 | 循环内没有 if/switch |
| 数据对齐 | 数据对齐到向量宽度 |
| 简单操作 | 循环内只有算术/逻辑运算 |

#### 4.3 帮助编译器自动向量化

```cpp
// ❌ 编译器不敢向量化：指针可能重叠
void add(float* a, float* b, float* c, int n) {
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

// ✅ 使用 restrict 告诉编译器指针不重叠
void add_restrict(float* restrict a, float* restrict b,
                  float* restrict c, int n) {
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

// ✅ C++ 使用 __restrict__
void add_cpp(float* __restrict__ a, float* __restrict__ b,
             float* __restrict__ c, int n) {
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

// ✅ 使用 OpenMP simd 指令
void add_omp(float* a, float* b, float* c, int n) {
    #pragma omp simd
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}

// ✅ 使用 GCC/Clang 的向量化提示
void add_pragma(float* __restrict__ a, float* __restrict__ b,
                float* __restrict__ c, int n) {
    #pragma GCC ivdep  // 告诉编译器没有数据依赖
    for (int i = 0; i < n; ++i) {
        c[i] = a[i] + b[i];
    }
}
```

#### 4.4 查看向量化报告

```bash
# GCC 查看向量化信息
g++ -O3 -fopt-info-vec main.cpp
# 输出：main.cpp:5: note: loop vectorized

# GCC 查看未向量化的原因
g++ -O3 -fopt-info-vec-missed main.cpp
# 输出：main.cpp:10: missed: not vectorized: control flow in loop

# Clang 查看向量化信息
clang++ -O3 -Rpass=loop-vectorize main.cpp

# Clang 查看未向量化的原因
clang++ -O3 -Rpass-missed=loop-vectorize main.cpp
```

***

### 5. SIMD 的适用场景

#### 5.1 适合 SIMD 的场景

| 场景 | 说明 | 加速比 |
|------|------|--------|
| 图像处理 | 像素操作（亮度、对比度、滤镜） | 4-8x |
| 音视频编解码 | DCT变换、运动估计 | 4-8x |
| 科学计算 | 矩阵运算、向量运算 | 4-8x |
| 机器学习推理 | 全连接层、卷积 | 4-16x |
| 字符串处理 | 查找、比较、拷贝 | 4-8x |
| 数据库 | 列扫描、过滤 | 4-8x |
| 游戏物理 | 粒子系统、碰撞检测 | 4-8x |

#### 5.2 不适合 SIMD 的场景

| 场景 | 原因 |
|------|------|
| 大量分支 | SIMD 所有通道执行相同操作，分支导致部分通道空闲 |
| 数据依赖强 | 后续计算依赖前序结果，无法并行 |
| 数据不连续 | gather/scatter 效率低 |
| 操作复杂 | 没有对应的 SIMD 指令 |
| 数据量小 | 向量化开销大于收益 |

#### 5.3 实战：图像亮度调整

```cpp
#include <immintrin.h>
#include <cstdio>
#include <chrono>
#include <vector>

// 标量版本
void adjust_brightness_scalar(uint8_t* pixels, int n, float factor) {
    for (int i = 0; i < n; ++i) {
        float val = pixels[i] * factor;
        if (val > 255.0f) val = 255.0f;
        if (val < 0.0f) val = 0.0f;
        pixels[i] = static_cast<uint8_t>(val);
    }
}

// AVX2 向量化版本
void adjust_brightness_avx2(uint8_t* pixels, int n, float factor) {
    __m256 vfactor = _mm256_set1_ps(factor);
    __m256 vzero = _mm256_setzero_ps();
    __m256 vmax = _mm256_set1_ps(255.0f);

    int i = 0;
    for (; i + 32 <= n; i += 32) {
        // 加载 32 字节（32 个像素）
        __m256i data = _mm256_loadu_si256(
            reinterpret_cast<const __m256i*>(&pixels[i]));

        // 将低 128 位和高 128 位分别解包为 16 位
        __m256i zero256 = _mm256_setzero_si256();
        __m256i lo16 = _mm256_unpacklo_epi8(data, zero256);  // 低16个像素→16位
        __m256i hi16 = _mm256_unpackhi_epi8(data, zero256);  // 高16个像素→16位

        // 将 16 位转换为 float（每次处理 8 个）
        __m128i lo_lo16 = _mm256_castsi256_si128(lo16);
        __m128i hi_lo16 = _mm256_extractf128_si256(lo16, 1);
        __m128i lo_hi16 = _mm256_castsi256_si128(hi16);
        __m128i hi_hi16 = _mm256_extractf128_si256(hi16, 1);

        __m256 f0 = _mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(lo_lo16));
        __m256 f1 = _mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(hi_lo16));
        __m256 f2 = _mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(lo_hi16));
        __m256 f3 = _mm256_cvtepi32_ps(_mm256_cvtepu16_epi32(hi_hi16));

        // 乘以亮度因子
        f0 = _mm256_mul_ps(f0, vfactor);
        f1 = _mm256_mul_ps(f1, vfactor);
        f2 = _mm256_mul_ps(f2, vfactor);
        f3 = _mm256_mul_ps(f3, vfactor);

        // 钳制到 [0, 255]
        f0 = _mm256_min_ps(_mm256_max_ps(f0, vzero), vmax);
        f1 = _mm256_min_ps(_mm256_max_ps(f1, vzero), vmax);
        f2 = _mm256_min_ps(_mm256_max_ps(f2, vzero), vmax);
        f3 = _mm256_min_ps(_mm256_max_ps(f3, vzero), vmax);

        // 转回 uint8_t 并打包（省略，逻辑类似，反向操作）
        // 实际项目中推荐使用 _mm256_cvtps_epi32 + pack 操作
    }

    // 处理剩余像素
    adjust_brightness_scalar(&pixels[i], n - i, factor);
}

int main() {
    const int N = 1920 * 1080 * 3;  // 1080p 图像
    std::vector<uint8_t> img1(N, 128);
    std::vector<uint8_t> img2(N, 128);

    auto t1 = std::chrono::high_resolution_clock::now();
    adjust_brightness_scalar(img1.data(), N, 1.5f);
    auto t2 = std::chrono::high_resolution_clock::now();
    printf("标量版本: %lld us\n",
           std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count());

    auto t3 = std::chrono::high_resolution_clock::now();
    adjust_brightness_avx2(img2.data(), N, 1.5f);
    auto t4 = std::chrono::high_resolution_clock::now();
    printf("AVX2版本: %lld us\n",
           std::chrono::duration_cast<std::chrono::microseconds>(t4 - t3).count());

    return 0;
}
```

#### 5.4 实战：向量点积

```cpp
#include <immintrin.h>
#include <cstdio>

// 标量版本
float dot_product_scalar(const float* a, const float* b, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        sum += a[i] * b[i];
    }
    return sum;
}

// AVX2 向量化版本
float dot_product_avx2(const float* a, const float* b, int n) {
    __m256 vsum = _mm256_setzero_ps();

    int i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        vsum = _mm256_fmadd_ps(va, vb, vsum);  // vsum += va * vb
    }

    // 水平求和
    __m128 hi = _mm256_extractf128_ps(vsum, 1);
    __m128 lo = _mm256_castps256_ps128(vsum);
    __m128 sum128 = _mm_add_ps(hi, lo);
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);
    float result = _mm_cvtss_f32(sum128);

    // 处理剩余元素
    for (; i < n; ++i) {
        result += a[i] * b[i];
    }

    return result;
}

int main() {
    alignas(32) float a[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    alignas(32) float b[16] = {1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1};

    printf("标量: %.2f\n", dot_product_scalar(a, b, 16));
    printf("AVX2: %.2f\n", dot_product_avx2(a, b, 16));

    return 0;
}
```

***

### 6. 跨平台 SIMD 编程

#### 6.1 使用 xsimd 库（推荐）

```cpp
// xsimd 提供跨平台的 SIMD 抽象
#include <xsimd/xsimd.hpp>
#include <vector>

void add_arrays_xsimd(std::vector<float>& a,
                       const std::vector<float>& b) {
    size_t n = a.size();
    size_t vec_size = xsimd::batch<float>::size;
    size_t aligned_n = n - n % vec_size;

    for (size_t i = 0; i < aligned_n; i += vec_size) {
        auto va = xsimd::load_unaligned(&a[i]);
        auto vb = xsimd::load_unaligned(&b[i]);
        auto vr = va + vb;
        xsimd::store_unaligned(&a[i], vr);
    }

    for (size_t i = aligned_n; i < n; ++i) {
        a[i] += b[i];
    }
}
```

#### 6.2 编译选项

```bash
# 启用 SSE4.2
g++ -O3 -msse4.2 main.cpp

# 启用 AVX2
g++ -O3 -mavx2 main.cpp

# 启用 AVX-512
g++ -O3 -mavx512f main.cpp

# 运行时检测（推荐）
g++ -O3 -mavx2 main.cpp  # 编译时允许使用 AVX2
# 程序中运行时检测 CPU 是否支持，选择对应代码路径
```

#### 6.3 运行时 CPU 特性检测

```cpp
#include <cstdio>

// GCC/Clang 内置函数
void detect_cpu_features() {
    #ifdef __GNUC__
    printf("SSE:     %s\n", __builtin_cpu_supports("sse") ? "yes" : "no");
    printf("SSE2:    %s\n", __builtin_cpu_supports("sse2") ? "yes" : "no");
    printf("SSE4.2:  %s\n", __builtin_cpu_supports("sse4.2") ? "yes" : "no");
    printf("AVX:     %s\n", __builtin_cpu_supports("avx") ? "yes" : "no");
    printf("AVX2:    %s\n", __builtin_cpu_supports("avx2") ? "yes" : "no");
    printf("AVX512F: %s\n", __builtin_cpu_supports("avx512f") ? "yes" : "no");
    #endif
}

// 使用函数指针实现运行时分发
using AddFunc = void(*)(float*, const float*, int);

void add_scalar(float* a, const float* b, int n) {
    for (int i = 0; i < n; ++i) a[i] += b[i];
}

void add_avx2(float* a, const float* b, int n) {
    #ifdef __GNUC__
    for (int i = 0; i + 8 <= n; i += 8) {
        __m256 va = _mm256_loadu_ps(&a[i]);
        __m256 vb = _mm256_loadu_ps(&b[i]);
        _mm256_storeu_ps(&a[i], _mm256_add_ps(va, vb));
    }
    #endif
}

AddFunc get_best_add() {
    #ifdef __GNUC__
    if (__builtin_cpu_supports("avx2")) return add_avx2;
    #endif
    return add_scalar;
}

int main() {
    detect_cpu_features();
    auto add = get_best_add();
    return 0;
}
```

***

### 7. 常见误区

| 误区 | 事实 |
|------|------|
| "SIMD 一定比标量快" | 数据量小时，SIMD 初始化开销可能抵消收益 |
| "AVX-512 总是最好的" | 降频问题可能导致整体变慢 |
| "手动 intrinsics 总比自动向量化好" | 编译器自动向量化越来越强，手动代码难维护 |
| "SIMD 能解决所有性能问题" | SIMD 只加速数据并行部分，算法优化更重要 |
| "NEON 和 SSE 代码可以通用" | 指令集不同，需要抽象层或条件编译 |

***

### 8. 总结

| 要点 | 说明 |
|------|------|
| SIMD | 单指令多数据，一条指令同时处理多个数据 |
| SSE/AVX/AVX-512 | x86 SIMD 指令集，128/256/512位宽 |
| Intrinsics | 编译器内置函数，比汇编可读，比纯C高效 |
| 自动向量化 | 编译器自动将循环转为SIMD，需要满足条件 |
| 适用场景 | 图像处理、科学计算、机器学习、字符串处理 |
| 不适用 | 大量分支、强数据依赖、数据不连续 |