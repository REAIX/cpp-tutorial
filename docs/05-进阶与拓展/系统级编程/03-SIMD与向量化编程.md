> **前置知识**：CPU缓存优化见 [CPU缓存优化](./02-CPU缓存优化.md)，内存模型见 C++内存模型。

# SIMD与向量化编程

---

## 📚 难度分级与推荐阅读

> **本文档采用三级难度标注：**
> - 🟢 **入门级**：基础概念，适合初学者
> - 🟡 **进阶级**：需要一定基础，适合有经验的开发者
> - 🔴 **高阶级**：深入底层原理，适合高级开发者

### 推荐阅读范围

| 读者类型 | 建议阅读范围 | 跳过内容 |
|---------|------------|---------|
| **初学者** | 🟢 第1章（SIMD指令集概览，了解概念即可）、🟢 第3章（自动向量化） | 第2章及以后 |
| **中级开发者** | 🟢 第1章、🟡 第2章（intrinsics编程）、🟡 第3章（自动向量化）、🟡 第5章（SIMD应用实例） | 🔴 第4章（跨平台SIMD抽象） |
| **高级开发者/专家** | 全文阅读，重点关注 🔴 第2.3节（AVX-512 intrinsics）、🔴 第4章（跨平台SIMD抽象） | — |

---

> 利用CPU的并行计算能力，实现数据级并行处理

***

> **SIMD is the secret weapon of high-performance computing.** — Agner Fog
> （SIMD是高性能计算的秘密武器。）

***

> **🎯 众擎易举，独木难支。**
>
> （SIMD让一条指令同时处理多个数据，将CPU的并行计算能力发挥到极致。）

## 前置知识
- [CPU缓存优化](02-CPU缓存优化.md)
- C++内存模型
## 后续内容
- 分支预测与流水线优化

***

> 💡 **通俗理解 - 什么是SIMD？**

**标量处理 = 一个厨师一次炒一盘菜**
- 厨师（CPU核心）一次只能处理一盘菜（一个数据）
- 要炒100盘菜，需要炒100次

**SIMD处理 = 一个厨师同时炒8盘菜**
- 厨师用一个大锅（宽寄存器），一次可以放8盘菜的食材
- 一把火（一条指令）同时炒8盘
- 100盘菜只需要炒13次！

> 🔬 **抽象理解 - SIMD的本质**：
>
> - **SIMD**（Single Instruction Multiple Data）：一条指令同时处理多个数据
> - **核心思想**：数据级并行，将相同的操作应用于多个数据元素
> - **性能提升**：理论加速比 = 寄存器宽度 / 数据宽度（如AVX2处理8个float = 8倍）
> - **关键条件**：数据连续、操作相同、无数据依赖

***

## 目录

- [1. SIMD指令集概览](#1-simd指令集概览)
- [2. intrinsics编程](#2-intrinsics编程)
- [3. 自动向量化](#3-自动向量化)
- [4. 跨平台SIMD抽象](#4-跨平台simd抽象)
- [5. SIMD应用实例](#5-simd应用实例)
- [6. 小结](#6-小结)

***

## 🟢 1. SIMD指令集概览

### 1.1 x86 SIMD指令集演进

```
┌──────────────────────────────────────────────────────────┐
│              x86 SIMD指令集演进                           │
├──────────────────────────────────────────────────────────┤
│                                                          │
│  1999  SSE      128位  4×float / 2×double               │
│   │                                                      │
│  2001  SSE2     128位  整数+浮点完整支持                  │
│   │                                                      │
│  2004  SSE3     128位  水平加减、复数运算                  │
│   │                                                      │
│  2006  SSSE3    128位  字节混洗、绝对值                   │
│   │                                                      │
│  2007  SSE4.1   128位  点积、混合运算                     │
│   │                                                      │
│  2008  SSE4.2   128位  字符串比较、CRC                   │
│   │                                                      │
│  2011  AVX      256位  8×float / 4×double               │
│   │            VEX编码，3操作数格式                        │
│   │                                                      │
│  2013  AVX2     256位  整数SIMD + FMA(融合乘加)          │
│   │                                                      │
│  2017  AVX-512  512位  16×float / 8×double              │
│   │            掩码寄存器、新增指令                        │
│   │                                                      │
│  ▼  理论加速比: SSE=4x, AVX=8x, AVX-512=16x (float)    │
└──────────────────────────────────────────────────────────┘
```

### 1.2 各指令集寄存器与数据容量

| 指令集 | 寄存器 | 宽度 | float数 | double数 | int32数 | int8数 |
|--------|--------|------|---------|----------|---------|--------|
| **SSE** | xmm0-15 | 128位 | 4 | 2 | 4 | 16 |
| **AVX** | ymm0-15 | 256位 | 8 | 4 | 8 | 32 |
| **AVX-512** | zmm0-31 | 512位 | 16 | 8 | 16 | 64 |

### 1.3 检测CPU支持的SIMD指令集

```cpp
#include <iostream>
#include <cstdint>

// ============================================
// 使用CPUID检测SIMD支持
// ============================================

struct SIMDInfo {
    bool sse;
    bool sse2;
    bool sse3;
    bool ssse3;
    bool sse41;
    bool sse42;
    bool avx;
    bool avx2;
    bool avx512f;
    bool avx512bw;
    bool avx512vl;
    bool fma;
};

SIMDInfo detect_simd() {
    SIMDInfo info = {};

    uint32_t eax, ebx, ecx, edx;

    // CPUID leaf 1
    __cpuid(1, eax, ebx, ecx, edx);
    info.sse   = (edx >> 25) & 1;
    info.sse2  = (edx >> 26) & 1;
    info.sse3  = ecx & 1;
    info.ssse3 = (ecx >> 9) & 1;
    info.sse41 = (ecx >> 19) & 1;
    info.sse42 = (ecx >> 20) & 1;
    info.avx   = (ecx >> 28) & 1;
    info.fma   = (ecx >> 12) & 1;

    // CPUID leaf 7 (需要先检查是否支持)
    uint32_t max_leaf;
    __cpuid(0, max_leaf, ebx, ecx, edx);
    if (max_leaf >= 7) {
        __cpuid_count(7, 0, eax, ebx, ecx, edx);
        info.avx2    = (ebx >> 5) & 1;
        info.avx512f = (ebx >> 16) & 1;
        info.avx512bw = (ebx >> 30) & 1;
        info.avx512vl = (ebx >> 31) & 1;
    }

    return info;
}

void print_simd_info() {
    SIMDInfo info = detect_simd();

    std::cout << "=== CPU SIMD支持 ===" << std::endl;
    std::cout << "SSE:      " << (info.sse ? "是" : "否") << std::endl;
    std::cout << "SSE2:     " << (info.sse2 ? "是" : "否") << std::endl;
    std::cout << "SSE3:     " << (info.sse3 ? "是" : "否") << std::endl;
    std::cout << "SSSE3:    " << (info.ssse3 ? "是" : "否") << std::endl;
    std::cout << "SSE4.1:   " << (info.sse41 ? "是" : "否") << std::endl;
    std::cout << "SSE4.2:   " << (info.sse42 ? "是" : "否") << std::endl;
    std::cout << "AVX:      " << (info.avx ? "是" : "否") << std::endl;
    std::cout << "AVX2:     " << (info.avx2 ? "是" : "否") << std::endl;
    std::cout << "FMA:      " << (info.fma ? "是" : "否") << std::endl;
    std::cout << "AVX-512F: " << (info.avx512f ? "是" : "否") << std::endl;
    std::cout << "AVX-512BW:" << (info.avx512bw ? "是" : "否") << std::endl;
    std::cout << "AVX-512VL:" << (info.avx512vl ? "是" : "否") << std::endl;

    // 确定最佳可用指令集
    if (info.avx512f) {
        std::cout << "\n推荐使用: AVX-512 (16×float)" << std::endl;
    } else if (info.avx2) {
        std::cout << "\n推荐使用: AVX2 (8×float)" << std::endl;
    } else if (info.avx) {
        std::cout << "\n推荐使用: AVX (8×float)" << std::endl;
    } else if (info.sse41) {
        std::cout << "\n推荐使用: SSE4.1 (4×float)" << std::endl;
    }
}
```

***

## 🟡 2. intrinsics编程

### 2.1 SSE intrinsics基础

```cpp
#include <iostream>
#include <xmmintrin.h>   // SSE
#include <emmintrin.h>   // SSE2
#include <pmmintrin.h>   // SSE3
#include <smmintrin.h>   // SSE4.1
#include <immintrin.h>   // AVX, AVX2, AVX-512
#include <cmath>

// ============================================
// SSE基础操作
// ============================================

void sse_basic_operations() {
    // __m128: 4个float的SSE寄存器
    // __m128d: 2个double
    // __m128i: 整数（各种宽度）

    // 1. 加载和存储
    alignas(16) float a[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    alignas(16) float b[4] = {5.0f, 6.0f, 7.0f, 8.0f};
    alignas(16) float result[4];

    // 加载（要求数据16字节对齐）
    __m128 va = _mm_load_ps(a);    // 对齐加载4个float
    __m128 vb = _mm_load_ps(b);

    // 未对齐加载（稍慢）
    // __m128 va = _mm_loadu_ps(a);

    // 2. 算术运算
    __m128 vadd = _mm_add_ps(va, vb);       // 加法
    __m128 vsub = _mm_sub_ps(va, vb);       // 减法
    __m128 vmul = _mm_mul_ps(va, vb);       // 乘法
    __m128 vdiv = _mm_div_ps(va, vb);       // 除法

    // 3. 存储
    _mm_store_ps(result, vadd);  // 对齐存储

    std::cout << "加法结果: ";
    for (int i = 0; i < 4; ++i) {
        std::cout << result[i] << " ";
    }
    std::cout << std::endl;

    // 4. 其他常用操作
    __m128 vmin = _mm_min_ps(va, vb);       // 逐元素取最小
    __m128 vmax = _mm_max_ps(va, vb);       // 逐元素取最大
    __m128 vsqrt = _mm_sqrt_ps(va);         // 逐元素平方根
    __m128 vrcp = _mm_rcp_ps(va);           // 快速倒数（近似）
    __m128 vrsqrt = _mm_rsqrt_ps(va);       // 快速平方根倒数（近似）

    // 5. 比较操作
    __m128 vcmp = _mm_cmplt_ps(va, vb);     // a < b 的掩码

    // 6. 水平运算（SSE3+）
    __m128 vhadd = _mm_hadd_ps(va, vb);     // 水平加法
}

// ============================================
// SSE向量点积
// ============================================

float sse_dot_product(const float* a, const float* b, size_t n) {
    __m128 sum = _mm_setzero_ps();  // 初始化为0

    size_t i = 0;
    for (; i + 4 <= n; i += 4) {
        __m128 va = _mm_loadu_ps(a + i);
        __m128 vb = _mm_loadu_ps(b + i);
        // 乘加: sum += va * vb
        sum = _mm_add_ps(sum, _mm_mul_ps(va, vb));
    }

    // 水平求和
    alignas(16) float temp[4];
    _mm_store_ps(temp, sum);
    float result = temp[0] + temp[1] + temp[2] + temp[3];

    // 处理剩余元素
    for (; i < n; ++i) {
        result += a[i] * b[i];
    }

    return result;
}
```

### 2.2 AVX/AVX2 intrinsics

```cpp
#include <iostream>
#include <immintrin.h>

// ============================================
// AVX基础操作（256位）
// ============================================

void avx_basic_operations() {
    // __m256: 8个float
    // __m256d: 4个double
    // __m256i: 整数

    alignas(32) float a[8] = {1,2,3,4,5,6,7,8};
    alignas(32) float b[8] = {8,7,6,5,4,3,2,1};
    alignas(32) float result[8];

    __m256 va = _mm256_load_ps(a);
    __m256 vb = _mm256_load_ps(b);

    // 算术运算（与SSE类似，但处理8个float）
    __m256 vadd = _mm256_add_ps(va, vb);
    __m256 vmul = _mm256_mul_ps(va, vb);

    _mm256_store_ps(result, vadd);
    std::cout << "AVX加法: ";
    for (int i = 0; i < 8; ++i) std::cout << result[i] << " ";
    std::cout << std::endl;

    // AVX特有的操作
    // 256位排列
    __m256 vperm = _mm256_permute_ps(va, _MM_SHUFFLE(3,1,2,0));

    // 高低128位混合
    __m256 vblend = _mm256_blend_ps(va, vb, 0x0F);  // 低4个取a，高4个取b
}

// ============================================
// AVX2 FMA（融合乘加）
// ============================================

void avx2_fma_operations() {
    alignas(32) float a[8] = {1,2,3,4,5,6,7,8};
    alignas(32) float b[8] = {2,3,4,5,6,7,8,9};
    alignas(32) float c[8] = {1,1,1,1,1,1,1,1};

    __m256 va = _mm256_load_ps(a);
    __m256 vb = _mm256_load_ps(b);
    __m256 vc = _mm256_load_ps(c);

    // FMA: a = a * b + c （一条指令完成，精度更高）
    __m256 vfma = _mm256_fmadd_ps(va, vb, vc);   // a*b + c
    __m256 vfms = _mm256_fmsub_ps(va, vb, vc);   // a*b - c
    __m256 vfnma = _mm256_fnmadd_ps(va, vb, vc);  // -a*b + c
    __m256 vfnms = _mm256_fnmsub_ps(va, vb, vc);  // -a*b - c

    // FMA的优势：
    // 1. 只需一条指令，减少延迟
    // 2. 中间结果不截断，精度更高
    // 3. 在矩阵乘法、多项式求值中特别有用
}

// ============================================
// AVX2向量点积（比SSE快2倍）
// ============================================

float avx2_dot_product(const float* a, const float* b, size_t n) {
    __m256 sum = _mm256_setzero_ps();

    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        sum = _mm256_fmadd_ps(va, vb, sum);  // FMA: sum += va * vb
    }

    // 水平求和：将256位缩减为1个float
    // 方法：高低128位相加，然后SSE水平求和
    __m128 hi128 = _mm256_extractf128_ps(sum, 1);  // 高128位
    __m128 lo128 = _mm256_castps256_ps128(sum);     // 低128位
    __m128 sum128 = _mm_add_ps(hi128, lo128);
    sum128 = _mm_hadd_ps(sum128, sum128);
    sum128 = _mm_hadd_ps(sum128, sum128);

    float result = _mm_cvtss_f32(sum128);

    // 处理剩余元素
    for (; i < n; ++i) {
        result += a[i] * b[i];
    }

    return result;
}
```

### 2.3 AVX-512 intrinsics

```cpp
#include <iostream>
#include <immintrin.h>

// ============================================
// AVX-512操作（512位 + 掩码）
// ============================================

void avx512_basic_operations() {
    // __m512: 16个float
    // __m512d: 8个double
    // __mmask16: 16位掩码寄存器

    alignas(64) float a[16] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16};
    alignas(64) float b[16] = {16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1};

    __m512 va = _mm512_load_ps(a);
    __m512 vb = _mm512_load_ps(b);

    // 基本运算
    __m512 vadd = _mm512_add_ps(va, vb);
    __m512 vmul = _mm512_mul_ps(va, vb);

    // AVX-512掩码操作：只对满足条件的元素执行
    // 比较并生成掩码
    __mmask16 mask = _mm512_cmp_ps_mask(va, vb, _MM_CMPINT_LT);  // a < b

    // 使用掩码的加法：只对mask=1的位置执行加法
    __m512 vadd_masked = _mm512_mask_add_ps(va, mask, va, vb);
    // mask=1的位置: a+b, mask=0的位置: 保持a不变

    // 掩码加载：只加载mask=1的位置
    alignas(64) float src[16] = {0};
    __m512 vload_masked = _mm512_mask_load_ps(_mm512_setzero_ps(), mask, a);

    // 掩码存储：只存储mask=1的位置
    alignas(64) float result[16] = {0};
    _mm512_mask_store_ps(result, mask, vadd);

    std::cout << "AVX-512掩码加法结果: ";
    for (int i = 0; i < 16; ++i) {
        std::cout << result[i] << " ";
    }
    std::cout << std::endl;
}

// ============================================
// AVX-512水平求和
// ============================================

float avx512_reduce_add_ps(__m512 v) {
    // AVX-512提供了便捷的规约指令
    return _mm512_reduce_add_ps(v);
}

// ============================================
// AVX-512逐元素绝对值
// ============================================

void avx512_abs_example() {
    alignas(64) float a[16] = {-1,2,-3,4,-5,6,-7,8,-9,10,-11,12,-13,14,-15,16};
    __m512 va = _mm512_load_ps(a);

    // 逐元素绝对值
    __m512 vabs = _mm512_abs_ps(va);

    alignas(64) float result[16];
    _mm512_store_ps(result, vabs);

    std::cout << "绝对值: ";
    for (int i = 0; i < 16; ++i) std::cout << result[i] << " ";
    std::cout << std::endl;
}
```

***

## 🟢 3. 自动向量化

### 3.1 编译器自动向量化

```cpp
#include <iostream>
#include <vector>

// ============================================
// 编译器可以自动向量化的代码模式
// ============================================

// 模式1：简单的循环（最容易向量化）
void auto_vectorizable_add(float* __restrict__ a,
                           const float* __restrict__ b,
                           const float* __restrict__ c,
                           size_t n) {
    // __restrict__ 告诉编译器a/b/c不重叠
    // 编译器可以安全地向量化
    for (size_t i = 0; i < n; ++i) {
        a[i] = b[i] + c[i];
    }
}

// 模式2：带常量系数的乘加
void auto_vectorizable_fma(float* __restrict__ a,
                           const float* __restrict__ b,
                           size_t n, float alpha) {
    for (size_t i = 0; i < n; ++i) {
        a[i] = alpha * b[i] + a[i];  // 可能生成FMA指令
    }
}

// 模式3：条件运算（可能向量化为blend/掩码）
void auto_vectorizable_conditional(float* __restrict__ a,
                                   const float* __restrict__ b,
                                   size_t n, float threshold) {
    for (size_t i = 0; i < n; ++i) {
        a[i] = (b[i] > threshold) ? b[i] : 0.0f;  // 可能向量化
    }
}

// ============================================
// 阻止自动向量化的代码模式
// ============================================

// 阻碍1：数据依赖（循环携带依赖）
void not_vectorizable_dependency(float* a, size_t n) {
    for (size_t i = 1; i < n; ++i) {
        a[i] = a[i - 1] + 1.0f;  // a[i]依赖a[i-1]，无法并行
    }
}

// 阻碍2：未知指针是否重叠
void maybe_not_vectorizable(float* a, float* b, size_t n) {
    // 编译器不确定a和b是否重叠，可能不向量化
    for (size_t i = 0; i < n; ++i) {
        a[i] = b[i] * 2.0f;
    }
}

// 阻碍3：复杂的控制流
void not_vectorizable_control_flow(float* a, const int* indices,
                                   size_t n) {
    for (size_t i = 0; i < n; ++i) {
        // 间接访问，编译器难以分析
        a[indices[i]] *= 2.0f;
    }
}

// ============================================
// 帮助编译器自动向量化的技巧
// ============================================

// 技巧1：使用__restrict__消除指针别名
// 技巧2：使用#pragma提示编译器
void vectorization_hints(float* __restrict__ a,
                         const float* __restrict__ b,
                         size_t n) {
    // GCC/Clang: 提示编译器循环没有数据依赖
    #pragma GCC ivdep
    for (size_t i = 0; i < n; ++i) {
        a[i] = b[i] * 2.0f;
    }
}

// 技巧3：使用#pragma omp simd
void openmp_simd(float* __restrict__ a,
                 const float* __restrict__ b,
                 size_t n) {
    #pragma omp simd
    for (size_t i = 0; i < n; ++i) {
        a[i] = b[i] * 2.0f;
    }
}

// 技巧4：确保数据对齐
void aligned_vectorization(float* __restrict__ a,
                           const float* __restrict__ b,
                           size_t n) {
    // 告诉编译器数据是对齐的
    // GCC: __builtin_assume_aligned
    float* aa = static_cast<float*>(__builtin_assume_aligned(a, 32));
    const float* bb = static_cast<const float*>(__builtin_assume_aligned(b, 32));

    for (size_t i = 0; i < n; ++i) {
        aa[i] = bb[i] * 2.0f;
    }
}
```

### 3.2 检查自动向量化结果

```bash
# GCC: 查看向量化报告
g++ -O3 -ftree-vectorize -fopt-info-vec-optimized main.cpp

# Clang: 查看向量化报告
clang++ -O3 -Rpass=loop-vectorize main.cpp

# 查看未向量化的原因
g++ -O3 -ftree-vectorize -fopt-info-vec-missed main.cpp
clang++ -O3 -Rpass-missed=loop-vectorize main.cpp

# 生成汇编代码检查
g++ -O3 -S -masm=intel main.cpp
# 查找 vmovaps, vaddps 等SIMD指令
```

***

## 🔴 4. 跨平台SIMD抽象

### 4.1 编译期SIMD分发

```cpp
#include <iostream>
#include <cstdint>
#include <cmath>

// ============================================
// 跨平台SIMD抽象层
// ============================================

// 检测最佳SIMD宽度
namespace simd {

// 编译期确定SIMD宽度
#if defined(__AVX512F__)
    constexpr size_t VECTOR_WIDTH = 16;  // 16个float
    constexpr size_t ALIGNMENT = 64;
#elif defined(__AVX__) || defined(__AVX2__)
    constexpr size_t VECTOR_WIDTH = 8;   // 8个float
    constexpr size_t ALIGNMENT = 32;
#elif defined(__SSE__)
    constexpr size_t VECTOR_WIDTH = 4;   // 4个float
    constexpr size_t ALIGNMENT = 16;
#else
    constexpr size_t VECTOR_WIDTH = 1;   // 标量回退
    constexpr size_t ALIGNMENT = 4;
#endif

// ============================================
// 通用SIMD向量类型
// ============================================

template<size_t N>
struct Vec {
    float data[N];

    float& operator[](size_t i) { return data[i]; }
    const float& operator[](size_t i) const { return data[i]; }
};

// ============================================
// 通用SIMD操作（编译期分发到具体实现）
// ============================================

// 加载
inline Vec<VECTOR_WIDTH> load(const float* ptr) {
    Vec<VECTOR_WIDTH> v;
#if defined(__AVX512F__)
    _mm512_store_ps(v.data, _mm512_load_ps(ptr));
#elif defined(__AVX__)
    _mm256_store_ps(v.data, _mm256_load_ps(ptr));
#elif defined(__SSE__)
    _mm_store_ps(v.data, _mm_load_ps(ptr));
#else
    for (size_t i = 0; i < VECTOR_WIDTH; ++i) v.data[i] = ptr[i];
#endif
    return v;
}

// 存储
inline void store(float* ptr, const Vec<VECTOR_WIDTH>& v) {
#if defined(__AVX512F__)
    _mm512_store_ps(ptr, _mm512_load_ps(v.data));
#elif defined(__AVX__)
    _mm256_store_ps(ptr, _mm256_load_ps(v.data));
#elif defined(__SSE__)
    _mm_store_ps(ptr, _mm_load_ps(v.data));
#else
    for (size_t i = 0; i < VECTOR_WIDTH; ++i) ptr[i] = v.data[i];
#endif
}

// 加法
inline Vec<VECTOR_WIDTH> add(const Vec<VECTOR_WIDTH>& a,
                             const Vec<VECTOR_WIDTH>& b) {
    Vec<VECTOR_WIDTH> result;
#if defined(__AVX512F__)
    _mm512_store_ps(result.data,
        _mm512_add_ps(_mm512_load_ps(a.data), _mm512_load_ps(b.data)));
#elif defined(__AVX__)
    _mm256_store_ps(result.data,
        _mm256_add_ps(_mm256_load_ps(a.data), _mm256_load_ps(b.data)));
#elif defined(__SSE__)
    _mm_store_ps(result.data,
        _mm_add_ps(_mm_load_ps(a.data), _mm_load_ps(b.data)));
#else
    for (size_t i = 0; i < VECTOR_WIDTH; ++i) result.data[i] = a.data[i] + b.data[i];
#endif
    return result;
}

// 乘法
inline Vec<VECTOR_WIDTH> mul(const Vec<VECTOR_WIDTH>& a,
                             const Vec<VECTOR_WIDTH>& b) {
    Vec<VECTOR_WIDTH> result;
#if defined(__AVX512F__)
    _mm512_store_ps(result.data,
        _mm512_mul_ps(_mm512_load_ps(a.data), _mm512_load_ps(b.data)));
#elif defined(__AVX__)
    _mm256_store_ps(result.data,
        _mm256_mul_ps(_mm256_load_ps(a.data), _mm256_load_ps(b.data)));
#elif defined(__SSE__)
    _mm_store_ps(result.data,
        _mm_mul_ps(_mm_load_ps(a.data), _mm_load_ps(b.data)));
#else
    for (size_t i = 0; i < VECTOR_WIDTH; ++i) result.data[i] = a.data[i] * b.data[i];
#endif
    return result;
}

// 设置零
inline Vec<VECTOR_WIDTH> setzero() {
    Vec<VECTOR_WIDTH> v;
#if defined(__AVX512F__)
    _mm512_store_ps(v.data, _mm512_setzero_ps());
#elif defined(__AVX__)
    _mm256_store_ps(v.data, _mm256_setzero_ps());
#elif defined(__SSE__)
    _mm_store_ps(v.data, _mm_setzero_ps());
#else
    for (size_t i = 0; i < VECTOR_WIDTH; ++i) v.data[i] = 0.0f;
#endif
    return v;
}

// 水平求和
inline float hsum(const Vec<VECTOR_WIDTH>& v) {
#if defined(__AVX512F__)
    return _mm512_reduce_add_ps(_mm512_load_ps(v.data));
#elif defined(__AVX__)
    __m256 x = _mm256_load_ps(v.data);
    __m128 hi = _mm256_extractf128_ps(x, 1);
    __m128 lo = _mm256_castps256_ps128(x);
    __m128 s = _mm_add_ps(hi, lo);
    s = _mm_hadd_ps(s, s);
    s = _mm_hadd_ps(s, s);
    return _mm_cvtss_f32(s);
#elif defined(__SSE__)
    __m128 x = _mm_load_ps(v.data);
    x = _mm_hadd_ps(x, x);
    x = _mm_hadd_ps(x, x);
    return _mm_cvtss_f32(x);
#else
    float sum = 0;
    for (size_t i = 0; i < VECTOR_WIDTH; ++i) sum += v.data[i];
    return sum;
#endif
}

} // namespace simd

// ============================================
// 使用跨平台SIMD抽象
// ============================================

float portable_dot_product(const float* a, const float* b, size_t n) {
    using namespace simd;

    Vec<VECTOR_WIDTH> sum = setzero();

    size_t i = 0;
    for (; i + VECTOR_WIDTH <= n; i += VECTOR_WIDTH) {
        Vec<VECTOR_WIDTH> va = load(a + i);
        Vec<VECTOR_WIDTH> vb = load(b + i);
        sum = add(sum, mul(va, vb));
    }

    float result = hsum(sum);

    // 处理剩余元素
    for (; i < n; ++i) {
        result += a[i] * b[i];
    }

    return result;
}
```

***

## 🟡 5. SIMD应用实例

### 5.1 图像处理中的SIMD

```cpp
#include <iostream>
#include <cstdint>
#include <immintrin.h>

// ============================================
// 图像灰度转换（RGB → 灰度）
// ============================================

// 标量版本
void rgb_to_gray_scalar(const uint8_t* rgb, uint8_t* gray,
                        size_t pixel_count) {
    // 灰度公式: gray = 0.299*R + 0.587*G + 0.114*B
    for (size_t i = 0; i < pixel_count; ++i) {
        float r = rgb[i * 3 + 0];
        float g = rgb[i * 3 + 1];
        float b = rgb[i * 3 + 2];
        float gray_val = 0.299f * r + 0.587f * g + 0.114f * b;
        gray[i] = static_cast<uint8_t>(gray_val + 0.5f);
    }
}

// AVX2版本：一次处理8个像素
void rgb_to_gray_avx2(const uint8_t* rgb, uint8_t* gray,
                      size_t pixel_count) {
    // 预设权重
    const __m256 wr = _mm256_set1_ps(0.299f);
    const __m256 wg = _mm256_set1_ps(0.587f);
    const __m256 wb = _mm256_set1_ps(0.114f);

    size_t i = 0;
    for (; i + 8 <= pixel_count; i += 8) {
        // 加载24字节的RGB数据（8个像素 × 3通道）
        // 需要手动解交织RGB数据

        // 简化方案：逐像素加载但批量计算
        alignas(32) float r[8], g[8], b[8];
        for (int j = 0; j < 8; ++j) {
            r[j] = static_cast<float>(rgb[(i + j) * 3 + 0]);
            g[j] = static_cast<float>(rgb[(i + j) * 3 + 1]);
            b[j] = static_cast<float>(rgb[(i + j) * 3 + 2]);
        }

        __m256 vr = _mm256_load_ps(r);
        __m256 vg = _mm256_load_ps(g);
        __m256 vb = _mm256_load_ps(b);

        // gray = wr*R + wg*G + wb*B
        __m256 vgray = _mm256_fmadd_ps(vr, wr, _mm256_fmadd_ps(vg, wg, _mm256_mul_ps(vb, wb)));

        // 转换回uint8_t
        alignas(32) float gray_f[8];
        _mm256_store_ps(gray_f, vgray);

        for (int j = 0; j < 8; ++j) {
            gray[i + j] = static_cast<uint8_t>(gray_f[j] + 0.5f);
        }
    }

    // 处理剩余像素
    for (; i < pixel_count; ++i) {
        float r = rgb[i * 3 + 0];
        float g = rgb[i * 3 + 1];
        float b_val = rgb[i * 3 + 2];
        gray[i] = static_cast<uint8_t>(0.299f * r + 0.587f * g + 0.114f * b_val + 0.5f);
    }
}

// ============================================
// 图像亮度调整
// ============================================

void adjust_brightness_avx2(uint8_t* pixels, size_t count,
                            float factor) {
    const __m256 vfactor = _mm256_set1_ps(factor);
    const __m256 vzero = _mm256_setzero_ps();
    const __m256 vmax = _mm256_set1_ps(255.0f);

    size_t i = 0;
    for (; i + 8 <= count; i += 8) {
        // 加载8个像素
        __m128i v8 = _mm_loadl_epi64(
            reinterpret_cast<const __m128i*>(pixels + i));
        __m256i v32 = _mm256_cvtepu8_epi32(v8);

        // 转换为float
        __m256 vf = _mm256_cvtepi32_ps(v32);

        // 乘以亮度因子
        vf = _mm256_mul_ps(vf, vfactor);

        // 钳制到[0, 255]
        vf = _mm256_max_ps(vf, vzero);
        vf = _mm256_min_ps(vf, vmax);

        // 转换回uint8_t
        __m256i vi = _mm256_cvtps_epi32(vf);
        __m128i result = _mm256_cvtepi32_epi8(vi);
        _mm_storel_epi64(reinterpret_cast<__m128i*>(pixels + i), result);
    }

    // 处理剩余
    for (; i < count; ++i) {
        float v = pixels[i] * factor;
        pixels[i] = static_cast<uint8_t>(std::min(255.0f, std::max(0.0f, v)));
    }
}
```

### 5.2 字符串操作中的SIMD

```cpp
#include <iostream>
#include <cstdint>
#include <cstring>
#include <immintrin.h>

// ============================================
// SIMD加速的strlen
// ============================================

size_t simd_strlen(const char* str) {
    const char* ptr = str;

    // 对齐到16字节边界
    while (reinterpret_cast<uintptr_t>(ptr) & 0xF) {
        if (*ptr == '\0') return ptr - str;
        ptr++;
    }

    // SSE版本：一次检查16个字节
    const __m128i vzero = _mm_set1_epi8(0);

    while (true) {
        __m128i vchunk = _mm_load_si128(reinterpret_cast<const __m128i*>(ptr));

        // 比较是否等于0
        __m128i vcmp = _mm_cmpeq_epi8(vchunk, vzero);
        int mask = _mm_movemask_epi8(vcmp);

        if (mask != 0) {
            // 找到零字节，计算位置
            unsigned long offset;
            _BitScanForward(&offset, mask);
            return (ptr - str) + offset;
        }

        ptr += 16;
    }
}

// ============================================
// SIMD加速的memcmp
// ============================================

int simd_memcmp(const void* s1, const void* s2, size_t n) {
    const char* p1 = static_cast<const char*>(s1);
    const char* p2 = static_cast<const char*>(s2);

    size_t i = 0;

    // AVX2版本：一次比较32字节
#if defined(__AVX2__)
    for (; i + 32 <= n; i += 32) {
        __m256i v1 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p1 + i));
        __m256i v2 = _mm256_loadu_si256(reinterpret_cast<const __m256i*>(p2 + i));

        __m256i vcmp = _mm256_cmpeq_epi8(v1, v2);
        int mask = _mm256_movemask_epi8(vcmp);

        if (mask != 0xFFFFFFFF) {
            // 发现不匹配
            unsigned long offset;
            _BitScanForward(&offset, ~mask);
            return static_cast<unsigned char>(p1[i + offset]) -
                   static_cast<unsigned char>(p2[i + offset]);
        }
    }
#endif

    // SSE版本：一次比较16字节
    for (; i + 16 <= n; i += 16) {
        __m128i v1 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p1 + i));
        __m128i v2 = _mm_loadu_si128(reinterpret_cast<const __m128i*>(p2 + i));

        __m128i vcmp = _mm_cmpeq_epi8(v1, v2);
        int mask = _mm_movemask_epi8(vcmp);

        if (mask != 0xFFFF) {
            unsigned long offset;
            _BitScanForward(&offset, ~mask);
            return static_cast<unsigned char>(p1[i + offset]) -
                   static_cast<unsigned char>(p2[i + offset]);
        }
    }

    // 标量处理剩余
    for (; i < n; ++i) {
        if (p1[i] != p2[i]) {
            return static_cast<unsigned char>(p1[i]) -
                   static_cast<unsigned char>(p2[i]);
        }
    }

    return 0;
}

// ============================================
// SIMD加速的字符查找
// ============================================

const char* simd_strchr(const char* str, char ch) {
    const __m128i vch = _mm_set1_epi8(ch);
    const __m128i vzero = _mm_set1_epi8(0);
    const char* ptr = str;

    while (true) {
        __m128i vchunk = _mm_loadu_si128(reinterpret_cast<const __m128i*>(ptr));

        __m128i vcmp_ch = _mm_cmpeq_epi8(vchunk, vch);
        __m128i vcmp_zero = _mm_cmpeq_epi8(vchunk, vzero);

        int mask_ch = _mm_movemask_epi8(vcmp_ch);
        int mask_zero = _mm_movemask_epi8(vcmp_zero);

        if (mask_ch) {
            unsigned long offset;
            _BitScanForward(&offset, mask_ch);
            return ptr + offset;
        }

        if (mask_zero) {
            return nullptr;  // 到达字符串末尾
        }

        ptr += 16;
    }
}
```

### 5.3 数学计算中的SIMD

```cpp
#include <iostream>
#include <cmath>
#include <immintrin.h>

// ============================================
// SIMD加速的向量运算
// ============================================

// 向量加法
void vector_add_avx(float* __restrict__ result,
                    const float* __restrict__ a,
                    const float* __restrict__ b,
                    size_t n) {
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 va = _mm256_loadu_ps(a + i);
        __m256 vb = _mm256_loadu_ps(b + i);
        __m256 vr = _mm256_add_ps(va, vb);
        _mm256_storeu_ps(result + i, vr);
    }
    for (; i < n; ++i) {
        result[i] = a[i] + b[i];
    }
}

// ============================================
// SIMD加速的SAXPY (Y = alpha*X + Y)
// ============================================

void saxpy_avx(float alpha, const float* x, float* y, size_t n) {
    __m256 valpha = _mm256_set1_ps(alpha);

    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 vx = _mm256_loadu_ps(x + i);
        __m256 vy = _mm256_loadu_ps(y + i);
        // y = alpha * x + y
        vy = _mm256_fmadd_ps(valpha, vx, vy);
        _mm256_storeu_ps(y + i, vy);
    }
    for (; i < n; ++i) {
        y[i] = alpha * x[i] + y[i];
    }
}

// ============================================
// SIMD加速的矩阵乘法
// ============================================

void matmul_avx(const float* A, const float* B, float* C,
                size_t M, size_t N, size_t K) {
    // C[M×N] = A[M×K] × B[K×N]
    for (size_t i = 0; i < M; ++i) {
        for (size_t j = 0; j < N; j += 8) {
            __m256 sum0 = _mm256_setzero_ps();
            __m256 sum1 = _mm256_setzero_ps();

            for (size_t k = 0; k < K; k += 2) {
                // 展开2倍
                __m256 a0 = _mm256_set1_ps(A[i * K + k]);
                __m256 b0 = _mm256_loadu_ps(&B[k * N + j]);
                sum0 = _mm256_fmadd_ps(a0, b0, sum0);

                if (k + 1 < K) {
                    __m256 a1 = _mm256_set1_ps(A[i * K + k + 1]);
                    __m256 b1 = _mm256_loadu_ps(&B[(k + 1) * N + j]);
                    sum1 = _mm256_fmadd_ps(a1, b1, sum1);
                }
            }

            __m256 sum = _mm256_add_ps(sum0, sum1);

            size_t remaining = std::min(size_t(8), N - j);
            if (remaining == 8) {
                _mm256_storeu_ps(&C[i * N + j], sum);
            } else {
                alignas(32) float temp[8];
                _mm256_store_ps(temp, sum);
                for (size_t r = 0; r < remaining; ++r) {
                    C[i * N + j + r] = temp[r];
                }
            }
        }
    }
}

// ============================================
// SIMD快速求倒数（比1.0f/x快3-4倍，精度足够）
// ============================================

void fast_reciprocal_avx(const float* input, float* output, size_t n) {
    size_t i = 0;
    for (; i + 8 <= n; i += 8) {
        __m256 v = _mm256_loadu_ps(input + i);
        // 第一步：近似倒数（14位精度）
        __m256 vrcp = _mm256_rcp_ps(v);
        // 第二步：Newton-Raphson迭代提高精度
        // rcp = rcp * (2 - v * rcp)
        __m256 vprod = _mm256_mul_ps(v, vrcp);
        __m256 vtwo = _mm256_set1_ps(2.0f);
        __m256 vdiff = _mm256_sub_ps(vtwo, vprod);
        vrcp = _mm256_mul_ps(vrcp, vdiff);
        _mm256_storeu_ps(output + i, vrcp);
    }
    for (; i < n; ++i) {
        output[i] = 1.0f / input[i];
    }
}
```

***

## 🟢 6. 小结

### 核心要点回顾

| 技术 | 寄存器宽度 | float并行数 | 适用场景 |
|------|-----------|------------|---------|
| **SSE4.1** | 128位 | 4 | 基础向量化，兼容性好 |
| **AVX** | 256位 | 8 | 主流服务器，2倍SSE |
| **AVX2** | 256位 | 8 | 整数+浮点+FMA |
| **AVX-512** | 512位 | 16 | 最新服务器，掩码操作 |

### SIMD编程最佳实践

```
1. 优先让编译器自动向量化
   - 简单循环、无依赖、数据连续
   - 使用__restrict__、#pragma omp simd

2. 必要时使用intrinsics
   - 编译器无法向量化的复杂逻辑
   - 需要特定SIMD指令（如FMA、掩码）

3. 数据布局适配SIMD
   - SoA布局优于AoS
   - 确保数据对齐（16/32/64字节）

4. 处理尾部元素
   - 循环剩余部分用标量处理
   - 或使用掩码操作（AVX-512）

5. 性能验证
   - 检查汇编输出是否使用了SIMD指令
   - 对比标量版本的加速比
```

### SIMD优化检查清单

```
□ 数据是否连续存储？
□ 数据是否正确对齐？
□ 循环是否有数据依赖？
□ 是否使用了__restrict__？
□ 是否处理了尾部元素？
□ 是否选择了正确的指令集？
□ 是否验证了向量化效果？
```
