# 什么是SIMD向量化编程
> 📖 相关章节：[性能优化](../../04-工程实践/08-性能优化.md)

> "SIMD=一个指令同时搬4箱货，而不是一箱一箱搬。"——单指令多数据，一条指令同时处理多个数据，是CPU级并行加速的核心技术。

***

### 1. 通俗理解

- **SIMD** = Single Instruction Multiple Data（单指令多数据），一条指令同时处理多个数据
- 普通指令一次处理1个数，SIMD指令一次处理4/8/16甚至64个数
- 就像超市收银：普通方式逐个扫码，SIMD方式一扫就是一整排

| 概念 | 类比 | 说明 |
|------|------|------|
| 标量运算 | 一次搬一箱货 | 普通指令，一次处理1个数据 |
| SIMD运算 | 一次搬4/8箱货 | 一条指令同时处理多个数据 |
| 向量寄存器 | 更宽的推车 | 128/256/512位寄存器，容纳多个数据 |
| 自动向量化 | 搬货机器人自动判断 | 编译器自动把循环变成SIMD指令 |

***

### 2. 技术说明

#### 1. SSE/AVX/AVX2/AVX-512/NEON指令集对比

| 指令集 | 平台 | 寄存器宽度 | 寄存器数量 | 同时处理float数 | 同时处理int32数 |
|--------|------|-----------|-----------|----------------|----------------|
| SSE | x86 | 128位 | 16个(XMM0-15) | 4个 | 4个 |
| AVX | x86 | 256位 | 16个(YMM0-15) | 8个 | 8个 |
| AVX2 | x86 | 256位 | 16个(YMM0-15) | 8个 | 8个（整数支持） |
| AVX-512 | x86 | 512位 | 32个(ZMM0-31) | 16个 | 16个 |
| NEON | ARM | 128位 | 32个(Q0-31) | 4个 | 4个 |

**关键区别**：

| 维度 | SSE | AVX | AVX2 | AVX-512 |
|------|-----|-----|------|---------|
| 发布年份 | 1999 | 2011 | 2013 | 2015 |
| 整数运算 | 支持 | 不支持 | 支持 | 支持 |
| 浮点运算 | 支持 | 支持 | 支持 | 支持 |
| 掩码操作 | 无 | 无 | 无 | 支持（8个掩码寄存器） |
| 降频问题 | 无 | 无 | 轻微 | 严重（部分CPU降频） |

#### 2. 自动向量化

编译器可以自动把循环转换为SIMD指令。

**编译选项**：

| 选项 | 作用 |
|------|------|
| `-O2` | 启用基本自动向量化 |
| `-O3` | 更激进的自动向量化 |
| `-ftree-vectorize` | 显式启用循环向量化 |
| `-mavx2` | 允许使用AVX2指令 |
| `-mavx512f` | 允许使用AVX-512指令 |
| `-fopt-info-vec` | 输出向量化信息 |
| `-fopt-info-vec-missed` | 输出未向量化的原因 |

**查看编译器是否向量化**：

```bash
gcc -O3 -ftree-vectorize -fopt-info-vec main.c -o main
# 输出类似：main.c:8: note: loop vectorized
```

**阻止自动向量化的因素**：

| 因素 | 说明 |
|------|------|
| 数据依赖 | 循环中后一次迭代依赖前一次结果 |
| 指针别名 | 编译器不确定指针是否重叠 |
| 分支 | 循环中有条件判断 |
| 非对齐访问 | 数据未对齐到向量宽度 |
| 函数调用 | 循环中调用了非内联函数 |

#### 3. 手动向量化：intrinsics函数

**SSE intrinsics头文件**：

```c
#include <xmmintrin.h>   /* SSE  */
#include <emmintrin.h>   /* SSE2 */
#include <pmmintrin.h>   /* SSE3 */
#include <smmintrin.h>   /* SSE4.1 */
#include <immintrin.h>   /* AVX/AVX2/AVX-512 */
```

**常用SSE intrinsics**：

| 函数 | 作用 | 数据类型 |
|------|------|---------|
| `_mm_load_ps` | 加载4个对齐float | `__m128` |
| `_mm_loadu_ps` | 加载4个非对齐float | `__m128` |
| `_mm_add_ps` | 4个float同时加 | `__m128` |
| `_mm_mul_ps` | 4个float同时乘 | `__m128` |
| `_mm_store_ps` | 存储4个对齐float | `__m128` |
| `_mm_set_ps` | 从4个float构造向量 | `__m128` |
| `_mm_set1_ps` | 广播1个float到4个位置 | `__m128` |

**命名规则**：`_mm_操作_后缀`
- `ps` = packed single（4个float）
- `pd` = packed double（2个double）
- `ss` = scalar single（1个float）
- `epi32` = 4个int32

#### 4. 何时使用SIMD

| 场景 | 适合SIMD | 原因 |
|------|---------|------|
| 图像处理 | ✅ | 像素操作高度并行 |
| 矩阵运算 | ✅ | 大量相同操作 |
| 字符串操作 | ✅ | memcmp/strlen等可并行 |
| 音视频编解码 | ✅ | 信号处理天然并行 |
| 条件分支多 | ❌ | 分支导致掩码开销 |
| 数据量小 | ❌ | 加载/存储开销抵消收益 |

#### 5. SIMD的局限

| 局限 | 说明 | 解决方案 |
|------|------|---------|
| 对齐要求 | SSE要16字节对齐，AVX要32字节 | `alignas(32)` 或 `_mm_loadu_ps` |
| 分支不友好 | 条件判断打断流水线 | 用掩码操作代替分支 |
| 数据布局 | AoS结构不利于SIMD | 改用SoA布局 |
| gather/scatter | 非连续内存访问效率低 | AVX2/512支持但慢，尽量连续访问 |
| 降频 | AVX-512可能导致CPU降频 | 评估实际场景是否值得 |

**AoS vs SoA**：

```c
/* AoS: Array of Structures —— 对SIMD不友好 */
struct Particle {
    float x, y, z;    /* 位置 */
    float vx, vy, vz; /* 速度 */
};
struct Particle particles[1000];
/* 想对x坐标做SIMD加法？数据不连续，效率低 */

/* SoA: Structure of Arrays —— 对SIMD友好 */
struct Particles {
    float x[1000];    /* 所有x连续存放 */
    float y[1000];    /* 所有y连续存放 */
    float z[1000];    /* 所有z连续存放 */
    float vx[1000];
    float vy[1000];
    float vz[1000];
};
/* 对x坐标做SIMD加法？数据连续，一次处理4个 */
```

***

### 3. 代码示例：用SSE加速数组求和

```c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <xmmintrin.h>

#define SIZE 4000000
#define ALIGN 16

float scalar_sum(const float* data, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += data[i];
    }
    return sum;
}

float sse_sum(const float* data, int n) {
    __m128 vsum = _mm_set1_ps(0.0f);

    int i;
    for (i = 0; i + 3 < n; i += 4) {
        __m128 vdata = _mm_load_ps(data + i);
        vsum = _mm_add_ps(vsum, vdata);
    }

    float partial[4];
    _mm_store_ps(partial, vsum);
    float sum = partial[0] + partial[1] + partial[2] + partial[3];

    for (; i < n; i++) {
        sum += data[i];
    }

    return sum;
}

int main(void) {
    float* data = aligned_alloc(ALIGN, SIZE * sizeof(float));
    if (!data) {
        perror("aligned_alloc");
        return 1;
    }

    srand((unsigned)time(NULL));
    for (int i = 0; i < SIZE; i++) {
        data[i] = (float)rand() / RAND_MAX;
    }

    clock_t start, end;

    start = clock();
    float s1 = scalar_sum(data, SIZE);
    end = clock();
    printf("标量求和: %.6f, 耗时: %.3f ms\n", s1, (double)(end - start) * 1000 / CLOCKS_PER_SEC);

    start = clock();
    float s2 = sse_sum(data, SIZE);
    end = clock();
    printf("SSE求和:  %.6f, 耗时: %.3f ms\n", s2, (double)(end - start) * 1000 / CLOCKS_PER_SEC);

    free(data);
    return 0;
}
```

**编译**：

```bash
gcc -O2 -msse main.c -o main
./main
```

**典型结果**：

```
标量求和: 2000123.500000, 耗时: 4.230 ms
SSE求和:  2000123.500000, 耗时: 1.120 ms
```

SSE版本约为标量版本的3-4倍加速。

***

### 4. 常见问题

#### 1. 问题1：自动向量化够用吗，还需要手写intrinsics吗

大部分情况下`-O3`自动向量化就够用了。需要手写的场景：自动向量化失败、需要特殊指令（如`_mm_shuffle`）、性能关键路径需要精细控制。

#### 2. 问题2：AVX-512降频问题严重吗

取决于CPU型号和工作负载。部分Intel CPU在执行AVX-512指令时会降低频率，短时间使用可能得不偿失。建议先测试实际性能。Ice Lake及之后的CPU降频问题已大幅改善。

#### 3. 问题3：ARM NEON和x86 SSE有什么区别

功能类似（都是128位SIMD），但NEON寄存器更多（32个 vs 16个），指令命名不同。跨平台代码建议用编译器intrinsics或SIMD库（如xsimd、highway）。

***

### 5. 极简总结

**SIMD是一条指令同时处理多个数据的并行技术。x86有SSE(128位)/AVX(256位)/AVX-512(512位)，ARM有NEON(128位)。编译器-O3可自动向量化，手写intrinsics可精细控制。适合图像/矩阵/字符串等数据并行场景，对齐要求和分支是主要限制。**

| 要点 | 一句话 |
|------|--------|
| SIMD | 单指令多数据——一条指令同时处理4/8/16个数据 |
| SSE/AVX/AVX-512 | x86 SIMD指令集——128/256/512位 |
| NEON | ARM SIMD指令集——128位 |
| 自动向量化 | `-O3`让编译器自动转换——大部分场景够用 |
| intrinsics | 手动SIMD函数——精细控制，性能关键路径使用 |
| 对齐 | SSE要16字节，AVX要32字节——`alignas`或loadu |
| AoS vs SoA | SoA布局对SIMD友好——连续数据才能高效加载 |

***

### 相关阅读

- [什么是性能剖析Profiling](09-什么是性能剖析Profiling.md)
- [什么是基准测试Benchmarking](./10-什么是基准测试Benchmarking.md)
- [CPP工具链](03-CPP工具链.md)

***