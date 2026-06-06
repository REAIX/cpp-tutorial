# GPU计算与CUDA

> 掌握CUDA编程模型、线程层次结构、内存管理与C++互操作

***

> **CUDA is C with a few extensions that let you write programs that run on the GPU.** — NVIDIA
> （CUDA是C语言加上一些扩展，让你编写在GPU上运行的程序。）

> **The GPU is the most powerful computing device in your system.** — Unknown
> （GPU是你系统中最强大的计算设备。）

***

> **🎯 授人以鱼不如授人以渔。**
>
> （学习CUDA不仅是学习一个工具，更是掌握并行计算的思维方式。）

> 💡 **通俗理解 - 什么是CUDA？**

想象一下：

- **CPU** 就像一个"数学教授"，能解复杂的微积分题，但一次只能做一道
- **GPU** 就像一个"千人计算团"，每人只会加减乘除，但可以同时做一千道简单题
- **CUDA** 就像"指挥这个计算团的指挥棒"，让你告诉每个人该做什么

当你需要对一百万个数据做同样的运算时，CUDA让你把这百万个任务分配给GPU的数千个核心同时执行，速度比CPU快几十倍！

> 🔬 **抽象理解 - CUDA编程模型**：
>
> - **内核（Kernel）**：在GPU上并行执行的函数，由数千个线程同时运行
> - **线程层次**：Grid→Block→Warp→Thread，从粗到细的组织结构
> - **内存层次**：全局内存、共享内存、常量内存、纹理内存，速度和容量各不同
> - **流与事件**：异步执行和同步机制，实现CPU-GPU并行

***

## 前置知识
- [Vulkan基础](02-Vulkan基础.md)
- C++模板与泛型编程
- 并发编程概念

## 后续内容
- [图形数学与算法](04-图形数学与算法.md)

***

## 目录

- [1. CUDA编程模型](#1-cuda编程模型)
- [2. 线程层次结构](#2-线程层次结构)
- [3. 内存管理](#3-内存管理)
- [4. 流与事件](#4-流与事件)
- [5. CUDA与C++互操作](#5-cuda与c互操作)
- [6. cuBLAS与cuDNN](#6-cublas与cudnn)
- [7. 常见陷阱与性能优化](#7-常见陷阱与性能优化)
- [8. 跨平台与替代方案](#8-跨平台与替代方案)
- [9. 本章小结](#9-本章小结)

***

## 1. CUDA编程模型

### 1.1 CUDA程序结构

一个典型的CUDA程序包含以下部分：

```cpp
// CUDA程序基本结构
#include <cuda_runtime.h>
#include <cstdio>
#include <vector>

// 1. CUDA内核函数（在GPU上执行）
__global__ void vectorAdd(const float* a, const float* b, float* c, int n) {
    // 计算当前线程处理的数据索引
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        c[idx] = a[idx] + b[idx];
    }
}

// 2. 主机代码（在CPU上执行）
int main() {
    const int N = 1024 * 1024;  // 一百万个元素
    size_t size = N * sizeof(float);

    // 分配主机内存
    std::vector<float> h_a(N), h_b(N), h_c(N);
    for (int i = 0; i < N; ++i) {
        h_a[i] = static_cast<float>(i);
        h_b[i] = static_cast<float>(i * 2);
    }

    // 分配设备内存
    float *d_a, *d_b, *d_c;
    cudaMalloc(&d_a, size);
    cudaMalloc(&d_b, size);
    cudaMalloc(&d_c, size);

    // 将数据从主机复制到设备
    cudaMemcpy(d_a, h_a.data(), size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, h_b.data(), size, cudaMemcpyHostToDevice);

    // 配置内核启动参数
    int threadsPerBlock = 256;
    int blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;

    // 启动内核
    vectorAdd<<<blocksPerGrid, threadsPerBlock>>>(d_a, d_b, d_c, N);

    // 等待GPU完成
    cudaDeviceSynchronize();

    // 将结果从设备复制回主机
    cudaMemcpy(h_c.data(), d_c, size, cudaMemcpyDeviceToHost);

    // 验证结果
    for (int i = 0; i < 10; ++i) {
        printf("c[%d] = %f (期望: %f)\n", i, h_c[i], h_a[i] + h_b[i]);
    }

    // 释放设备内存
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);

    return 0;
}
```

### 1.2 CUDA函数限定符

```cpp
// CUDA函数限定符

// __global__ - 内核函数，在GPU上执行，从CPU调用
// 必须返回void，可从主机或设备（动态并行）调用
__global__ void kernelFunction() {
    // 每个线程执行此函数
}

// __device__ - 设备函数，在GPU上执行，只能从GPU调用
__device__ float deviceFunction(float x) {
    return x * x + 1.0f;
}

// __host__ - 主机函数，在CPU上执行，只能从CPU调用（默认）
__host__ void hostFunction() {
    // 普通CPU代码
}

// __host__ __device__ - 同时可在CPU和GPU上执行
__host__ __device__ float sharedFunction(float x) {
    return sqrtf(x);
}

// 示例：在内核中调用设备函数
__global__ void computeKernel(float* output, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        float val = static_cast<float>(idx);
        output[idx] = deviceFunction(val);  // 调用__device__函数
    }
}
```

### 1.3 CUDA错误处理

```cpp
// CUDA错误处理宏
#include <cuda_runtime.h>
#include <cstdio>
#include <cstdlib>

// 检查CUDA运行时API调用的错误
#define CUDA_CHECK(call)                                                \
    do {                                                                \
        cudaError_t err = call;                                         \
        if (err != cudaSuccess) {                                       \
            fprintf(stderr, "CUDA错误 %s:%d: %s\n",                    \
                    __FILE__, __LINE__, cudaGetErrorString(err));       \
            exit(EXIT_FAILURE);                                         \
        }                                                               \
    } while (0)

// 检查内核执行后的错误
#define CUDA_CHECK_LAST_ERROR()                                         \
    do {                                                                \
        cudaError_t err = cudaGetLastError();                           \
        if (err != cudaSuccess) {                                       \
            fprintf(stderr, "CUDA内核错误 %s:%d: %s\n",                 \
                    __FILE__, __LINE__, cudaGetErrorString(err));       \
            exit(EXIT_FAILURE);                                         \
        }                                                               \
    } while (0)

// 使用示例
void exampleWithErrorHandling() {
    float* d_data;
    CUDA_CHECK(cudaMalloc(&d_data, 1024 * sizeof(float)));

    // 启动内核
    kernelFunction<<<10, 256>>>(d_data, 1024);
    CUDA_CHECK_LAST_ERROR();

    // 同步并检查错误
    CUDA_CHECK(cudaDeviceSynchronize());

    CUDA_CHECK(cudaFree(d_data));
}
```

### 1.4 更多CUDA内核示例

#### 1.4.1 并行归约（Reduction）

```cpp
// 并行归约：求大数组的和
// 这是CUDA编程中最经典的优化案例之一

// 版本1：朴素实现（存在Warp分化问题）
__global__ void reduceNaive(float* input, float* output, int n) {
    extern __shared__ float sdata[];

    int tid = threadIdx.x;
    int idx = blockIdx.x * blockDim.x * 2 + threadIdx.x;

    // 加载两个元素
    sdata[tid] = (idx < n) ? input[idx] : 0.0f;
    sdata[tid] += (idx + blockDim.x < n) ? input[idx + blockDim.x] : 0.0f;
    __syncthreads();

    // 逐步归约
    for (int stride = blockDim.x; stride > 0; stride >>= 1) {
        if (tid < stride) {
            sdata[tid] += sdata[tid + stride];
        }
        __syncthreads();
    }

    if (tid == 0) {
        output[blockIdx.x] = sdata[0];
    }
}

// 版本2：优化Warp分化（最后32个线程不需要同步）
__global__ void reduceOptimized(float* input, float* output, int n) {
    extern __shared__ float sdata[];

    int tid = threadIdx.x;
    int idx = blockIdx.x * blockDim.x * 2 + threadIdx.x;

    sdata[tid] = (idx < n) ? input[idx] : 0.0f;
    sdata[tid] += (idx + blockDim.x < n) ? input[idx + blockDim.x] : 0.0f;
    __syncthreads();

    // 手动展开最后6轮（Warp内无需__syncthreads）
    for (int stride = blockDim.x / 2; stride > 32; stride >>= 1) {
        if (tid < stride) {
            sdata[tid] += sdata[tid + stride];
        }
        __syncthreads();
    }

    // Warp内归约（无需同步，Warp内指令是同步的）
    if (tid < 32) {
        volatile float* vsmem = sdata;
        vsmem[tid] += vsmem[tid + 32];
        vsmem[tid] += vsmem[tid + 16];
        vsmem[tid] += vsmem[tid + 8];
        vsmem[tid] += vsmem[tid + 4];
        vsmem[tid] += vsmem[tid + 2];
        vsmem[tid] += vsmem[tid + 1];
    }

    if (tid == 0) {
        output[blockIdx.x] = sdata[0];
    }
}

// 完整的归约流程（多轮归约直到最终结果）
float fullReduction(float* d_input, int n) {
    int threads = 256;
    int blocks = (n + threads * 2 - 1) / (threads * 2);
    size_t sharedMemSize = threads * sizeof(float);

    float* d_temp;
    CUDA_CHECK(cudaMalloc(&d_temp, blocks * sizeof(float)));

    // 第一轮归约
    reduceOptimized<<<blocks, threads, sharedMemSize>>>(d_input, d_temp, n);

    // 如果结果仍然很多，继续归约
    while (blocks > 1) {
        int newBlocks = (blocks + threads * 2 - 1) / (threads * 2);
        reduceOptimized<<<newBlocks, threads, sharedMemSize>>>(
            d_temp, d_temp, blocks);
        blocks = newBlocks;
    }

    float result;
    CUDA_CHECK(cudaMemcpy(&result, d_temp, sizeof(float), cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaFree(d_temp));
    return result;
}
```

#### 1.4.2 并行前缀和（Scan / Prefix Sum）

```cpp
// 并行前缀和：输出每个位置的累加和
// 输入: [1, 3, 5, 2, 8, 4, 6, 7]
// 输出: [1, 4, 9, 11, 19, 23, 29, 36] (inclusive scan)
// 输出: [0, 1, 4, 9, 11, 19, 23, 29]  (exclusive scan)

// Blelloch工作高效前缀和（Exclusive Scan）
__global__ void exclusiveScan(float* input, float* output, float* blockSums,
                               int n) {
    extern __shared__ float sdata[];

    int tid = threadIdx.x;
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    // 加载数据到共享内存
    sdata[tid] = (idx < n) ? input[idx] : 0.0f;
    __syncthreads();

    // 上扫（Up-Sweep / Reduce阶段）
    int offset = 1;
    for (int d = blockDim.x >> 1; d > 0; d >>= 1) {
        __syncthreads();
        if (tid < d) {
            int ai = offset * (2 * tid + 1) - 1;
            int bi = offset * (2 * tid + 2) - 1;
            sdata[bi] += sdata[ai];
        }
        offset *= 2;
    }

    // 保存Block总和并清零最后一个元素
    __syncthreads();
    if (tid == 0) {
        blockSums[blockIdx.x] = sdata[blockDim.x - 1];
        sdata[blockDim.x - 1] = 0.0f;
    }

    // 下扫（Down-Sweep阶段）
    for (int d = 1; d < blockDim.x; d *= 2) {
        offset >>= 1;
        __syncthreads();
        if (tid < d) {
            int ai = offset * (2 * tid + 1) - 1;
            int bi = offset * (2 * tid + 2) - 1;
            float temp = sdata[ai];
            sdata[ai] = sdata[bi];
            sdata[bi] += temp;
        }
    }
    __syncthreads();

    // 写回结果
    if (idx < n) {
        output[idx] = sdata[tid];
    }
}

// 将Block前缀和加到每个元素上
__global__ void addBlockSums(float* output, const float* blockSums, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n && blockIdx.x > 0) {
        output[idx] += blockSums[blockIdx.x - 1];
    }
}
```

#### 1.4.3 图像处理内核

```cpp
// CUDA图像处理示例：Sobel边缘检测

// 读取像素（带边界检查）
__device__ float readPixel(const unsigned char* image,
                            int width, int height, int x, int y) {
    x = max(0, min(x, width - 1));
    y = max(0, min(y, height - 1));
    return static_cast<float>(image[y * width + x]) / 255.0f;
}

__global__ void sobelEdgeDetection(const unsigned char* input,
                                    unsigned char* output,
                                    int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) return;

    // Sobel X核
    // -1  0  1
    // -2  0  2
    // -1  0  1
    float gx = -readPixel(input, width, height, x-1, y-1)
               + readPixel(input, width, height, x+1, y-1)
               - 2.0f * readPixel(input, width, height, x-1, y)
               + 2.0f * readPixel(input, width, height, x+1, y)
               - readPixel(input, width, height, x-1, y+1)
               + readPixel(input, width, height, x+1, y+1);

    // Sobel Y核
    // -1 -2 -1
    //  0  0  0
    //  1  2  1
    float gy = -readPixel(input, width, height, x-1, y-1)
               - 2.0f * readPixel(input, width, height, x, y-1)
               - readPixel(input, width, height, x+1, y-1)
               + readPixel(input, width, height, x-1, y+1)
               + 2.0f * readPixel(input, width, height, x, y+1)
               + readPixel(input, width, height, x+1, y+1);

    float magnitude = sqrtf(gx * gx + gy * gy);
    output[y * width + x] = static_cast<unsigned char>(
        min(magnitude * 255.0f, 255.0f));
}

void launchSobel(const unsigned char* h_input, unsigned char* h_output,
                 int width, int height) {
    size_t imageSize = width * height * sizeof(unsigned char);

    unsigned char *d_input, *d_output;
    CUDA_CHECK(cudaMalloc(&d_input, imageSize));
    CUDA_CHECK(cudaMalloc(&d_output, imageSize));
    CUDA_CHECK(cudaMemcpy(d_input, h_input, imageSize, cudaMemcpyHostToDevice));

    dim3 blockSize(16, 16);
    dim3 gridSize((width + 15) / 16, (height + 15) / 16);
    sobelEdgeDetection<<<gridSize, blockSize>>>(d_input, d_output, width, height);

    CUDA_CHECK(cudaMemcpy(h_output, d_output, imageSize, cudaMemcpyDeviceToHost));
    CUDA_CHECK(cudaFree(d_input));
    CUDA_CHECK(cudaFree(d_output));
}
```

## 2. 线程层次结构

### 2.1 Grid、Block与Thread

```
┌──────────────────── Grid ────────────────────┐
│                                               │
│  ┌─── Block(0,0) ───┐  ┌─── Block(1,0) ───┐ │
│  │ (0,0) (1,0) (2,0)│  │ (0,0) (1,0) (2,0)│ │
│  │ (0,1) (1,1) (2,1)│  │ (0,1) (1,1) (2,1)│ │
│  │ (0,2) (1,2) (2,2)│  │ (0,2) (1,2) (2,2)│ │
│  └───────────────────┘  └───────────────────┘ │
│                                               │
│  ┌─── Block(0,1) ───┐  ┌─── Block(1,1) ───┐ │
│  │ (0,0) (1,0) (2,0)│  │ (0,0) (1,0) (2,0)│ │
│  │ (0,1) (1,1) (2,1)│  │ (0,1) (1,1) (2,1)│ │
│  │ (0,2) (1,2) (2,2)│  │ (0,2) (1,2) (2,2)│ │
│  └───────────────────┘  └───────────────────┘ │
│                                               │
└───────────────────────────────────────────────┘

每个小格子是一个Thread
每个大方框是一个Block
所有方框组成一个Grid
```

### 2.2 线程索引计算

```cpp
// 一维、二维、三维线程索引计算

// 一维索引（最常用）
__global__ void kernel1D(float* data, int n) {
    // 全局一维索引
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        data[idx] *= 2.0f;
    }
}

// 二维索引（图像处理常用）
__global__ void kernel2D(float* image, int width, int height) {
    // 全局二维索引
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x < width && y < height) {
        int idx = y * width + x;  // 行优先线性索引
        image[idx] = processPixel(image[idx]);
    }
}

// 三维索引（体数据处理）
__global__ void kernel3D(float* volume, int dimX, int dimY, int dimZ) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    int z = blockIdx.z * blockDim.z + threadIdx.z;

    if (x < dimX && y < dimY && z < dimZ) {
        int idx = z * dimY * dimX + y * dimX + x;
        volume[idx] = processVoxel(volume[idx]);
    }
}

// 启动配置示例
void launchKernels() {
    // 一维：处理100万个元素
    int N = 1000000;
    int threadsPerBlock = 256;
    int blocksPerGrid = (N + threadsPerBlock - 1) / threadsPerBlock;
    kernel1D<<<blocksPerGrid, threadsPerBlock>>>(d_data, N);

    // 二维：处理1920x1080图像
    dim3 blockSize2D(16, 16);  // 每个Block 16x16=256个线程
    dim3 gridSize2D(
        (1920 + blockSize2D.x - 1) / blockSize2D.x,
        (1080 + blockSize2D.y - 1) / blockSize2D.y
    );
    kernel2D<<<gridSize2D, blockSize2D>>>(d_image, 1920, 1080);

    // 三维：处理128x128x128体数据
    dim3 blockSize3D(8, 8, 8);  // 每个Block 8x8x8=512个线程
    dim3 gridSize3D(
        (128 + blockSize3D.x - 1) / blockSize3D.x,
        (128 + blockSize3D.y - 1) / blockSize3D.y,
        (128 + blockSize3D.z - 1) / blockSize3D.z
    );
    kernel3D<<<gridSize3D, blockSize3D>>>(d_volume, 128, 128, 128);
}
```

### 2.3 Warp与分支发散

```cpp
// Warp（线程束）是GPU调度的基本单位，包含32个线程
// 同一Warp中的线程必须执行相同的指令（SIMT模型）

// ❌ 分支发散：同一Warp中的线程走不同分支，导致性能下降
__global__ void branchDivergence(float* data, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        if (idx % 2 == 0) {
            // 偶数线程执行复杂计算
            data[idx] = expensiveOperation(data[idx]);
        } else {
            // 奇数线程执行简单计算
            data[idx] = simpleOperation(data[idx]);
        }
        // 两个分支都会被所有线程执行，不走的分支被屏蔽
        // 性能损失约50%
    }
}

// ✅ 消除分支发散：重新组织数据，让同一Warp走同一分支
__global__ void noBranchDivergence(float* data, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        // 前半部分线程做复杂计算，后半部分做简单计算
        // 同一Warp中的线程走同一分支
        if (idx < n / 2) {
            data[idx] = expensiveOperation(data[idx]);
        } else {
            data[idx] = simpleOperation(data[idx]);
        }
    }
}

// 辅助函数声明
__device__ float expensiveOperation(float x) { return sinf(x) * cosf(x); }
__device__ float simpleOperation(float x) { return x * 2.0f; }
```

## 3. 内存管理

### 3.1 全局内存

```cpp
// 全局内存（Global Memory）- 所有线程可访问，延迟最高

// 基本的设备内存操作
void globalMemoryExample() {
    const int N = 1024;
    size_t size = N * sizeof(float);

    // 1. 分配设备内存
    float* d_data;
    CUDA_CHECK(cudaMalloc(&d_data, size));

    // 2. 从主机复制数据到设备
    float h_data[N];
    for (int i = 0; i < N; ++i) h_data[i] = static_cast<float>(i);
    CUDA_CHECK(cudaMemcpy(d_data, h_data, size, cudaMemcpyHostToDevice));

    // 3. 执行内核
    processKernel<<<(N + 255) / 256, 256>>>(d_data, N);

    // 4. 从设备复制数据回主机
    CUDA_CHECK(cudaMemcpy(h_data, d_data, size, cudaMemcpyDeviceToHost));

    // 5. 释放设备内存
    CUDA_CHECK(cudaFree(d_data));
}

// 统一内存（Unified Memory）- CPU和GPU共享同一地址空间
void unifiedMemoryExample() {
    const int N = 1024;
    size_t size = N * sizeof(float);

    // 分配统一内存（CPU和GPU都可以访问）
    float* data;
    CUDA_CHECK(cudaMallocManaged(&data, size));

    // 在CPU上初始化
    for (int i = 0; i < N; ++i) {
        data[i] = static_cast<float>(i);
    }

    // 预取数据到GPU（优化提示）
    int device = 0;
    CUDA_CHECK(cudaMemPrefetchAsync(data, size, device, 0));

    // 在GPU上处理
    processKernel<<<(N + 255) / 256, 256>>>(data, N);
    CUDA_CHECK(cudaDeviceSynchronize());

    // 在CPU上读取结果（自动迁移）
    printf("data[0] = %f\n", data[0]);

    // 释放
    CUDA_CHECK(cudaFree(data));
}
```

### 3.2 共享内存

```cpp
// 共享内存（Shared Memory）- 同一Block内线程共享，速度接近寄存器

// 经典示例：矩阵乘法使用共享内存优化
__global__ void matrixMulShared(const float* A, const float* B, float* C,
                                 int M, int N, int K) {
    // 每个Block计算一个TILE_SIZE x TILE_SIZE的子矩阵
    const int TILE_SIZE = 16;

    // 声明共享内存（Block内所有线程共享）
    __shared__ float s_A[TILE_SIZE][TILE_SIZE];
    __shared__ float s_B[TILE_SIZE][TILE_SIZE];

    // 当前线程负责计算的输出元素
    int row = blockIdx.y * TILE_SIZE + threadIdx.y;
    int col = blockIdx.x * TILE_SIZE + threadIdx.x;

    float sum = 0.0f;

    // 分块计算（Tiling）
    for (int t = 0; t < (K + TILE_SIZE - 1) / TILE_SIZE; ++t) {
        // 协作加载A和B的子块到共享内存
        int aCol = t * TILE_SIZE + threadIdx.x;
        int bRow = t * TILE_SIZE + threadIdx.y;

        // 边界检查后加载
        s_A[threadIdx.y][threadIdx.x] =
            (row < M && aCol < K) ? A[row * K + aCol] : 0.0f;
        s_B[threadIdx.y][threadIdx.x] =
            (bRow < K && col < N) ? B[bRow * N + col] : 0.0f;

        // 同步：确保所有线程完成加载
        __syncthreads();

        // 计算部分点积
        for (int k = 0; k < TILE_SIZE; ++k) {
            sum += s_A[threadIdx.y][k] * s_B[k][threadIdx.x];
        }

        // 同步：确保计算完成后再加载下一块
        __syncthreads();
    }

    // 写入结果
    if (row < M && col < N) {
        C[row * N + col] = sum;
    }
}

// 动态共享内存
__global__ void kernelDynamicShared(float* output, int n) {
    // 动态分配共享内存（大小在内核启动时指定）
    extern __shared__ float dynamicShared[];

    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    int tid = threadIdx.x;

    if (idx < n) {
        dynamicShared[tid] = output[idx];
    }
    __syncthreads();

    // 在共享内存中进行规约操作
    for (int stride = blockDim.x / 2; stride > 0; stride >>= 1) {
        if (tid < stride) {
            dynamicShared[tid] += dynamicShared[tid + stride];
        }
        __syncthreads();
    }

    // Block的第一个线程写入结果
    if (tid == 0) {
        output[blockIdx.x] = dynamicShared[0];
    }
}

// 启动动态共享内存内核
void launchDynamicShared() {
    int n = 1024;
    int threadsPerBlock = 256;
    int blocksPerGrid = (n + threadsPerBlock - 1) / threadsPerBlock;

    // 第三个参数指定动态共享内存大小
    size_t sharedMemSize = threadsPerBlock * sizeof(float);
    kernelDynamicShared<<<blocksPerGrid, threadsPerBlock, sharedMemSize>>>(d_output, n);
}
```

### 3.3 常量内存与纹理内存

```cpp
// 常量内存（Constant Memory）- 只读，广播给所有线程，适合统一读取

// 声明常量内存（最大64KB）
__constant__ float c_kernel[9];  // 3x3卷积核
__constant__ float c_params[16]; // 参数数组

// 在主机端初始化常量内存
void initConstantMemory() {
    float h_kernel[9] = {
        1.0f/16, 2.0f/16, 1.0f/16,
        2.0f/16, 4.0f/16, 2.0f/16,
        1.0f/16, 2.0f/16, 1.0f/16
    };
    CUDA_CHECK(cudaMemcpyToSymbol(c_kernel, h_kernel, sizeof(h_kernel)));
}

// 使用常量内存的卷积内核
__global__ void convolutionKernel(const float* input, float* output,
                                   int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;

    if (x >= width || y >= height) return;

    float sum = 0.0f;
    for (int ky = -1; ky <= 1; ++ky) {
        for (int kx = -1; kx <= 1; ++kx) {
            int sx = min(max(x + kx, 0), width - 1);
            int sy = min(max(y + ky, 0), height - 1);
            sum += input[sy * width + sx] * c_kernel[(ky + 1) * 3 + (kx + 1)];
        }
    }
    output[y * width + x] = sum;
}

// 纹理内存（Texture Memory）- 只读，缓存优化，适合空间局部性访问
// 使用CUDA Surface或Texture Object

// 纹理对象方式（现代CUDA推荐）
void textureMemoryExample() {
    // 创建纹理对象
    cudaResourceDesc resDesc = {};
    resDesc.resType = cudaResourceTypeLinear;
    resDesc.res.linear.devPtr = d_data;
    resDesc.res.linear.desc = cudaCreateChannelDesc<float>();
    resDesc.res.linear.sizeInBytes = N * sizeof(float);

    cudaTextureDesc texDesc = {};
    texDesc.addressMode[0] = cudaAddressModeClamp;
    texDesc.filterMode = cudaFilterModePoint;
    texDesc.readMode = cudaReadModeElementType;

    cudaTextureObject_t texObj;
    CUDA_CHECK(cudaCreateTextureObject(&texObj, &resDesc, &texDesc, nullptr));

    // 在内核中使用纹理对象
    textureKernel<<<grid, block>>>(texObj, d_output, N);

    CUDA_CHECK(cudaDestroyTextureObject(texObj));
}

__global__ void textureKernel(cudaTextureObject_t texObj, float* output, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        // 通过纹理对象读取数据（自动缓存优化）
        output[idx] = tex1Dfetch<float>(texObj, idx) * 2.0f;
    }
}
```

### 3.4 内存优化深入

#### 3.4.1 占用率与寄存器使用

```cpp
// 占用率（Occupancy）是衡量GPU利用率的关键指标
// 占用率 = 活跃Warp数 / SM最大Warp数

// 影响占用率的因素：
// 1. 每个Block的线程数
// 2. 每个线程使用的寄存器数
// 3. 每个Block使用的共享内存量

// 查询和计算占用率
void calculateOccupancy() {
    int device;
    CUDA_CHECK(cudaGetDevice(&device));

    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, device));

    printf("设备: %s\n", prop.name);
    printf("SM数量: %d\n", prop.multiProcessorCount);
    printf("每个SM最大线程数: %d\n", prop.maxThreadsPerMultiProcessor);
    printf("每个SM最大Block数: %d\n", prop.maxBlocksPerMultiProcessor);
    printf("每个SM最大Warp数: %d\n", prop.maxThreadsPerMultiProcessor / 32);
    printf("每个Block最大线程数: %d\n", prop.maxThreadsPerBlock);
    printf("每个SM共享内存: %zu KB\n", prop.sharedMemPerMultiprocessor / 1024);
    printf("每个SM寄存器数: %d\n", prop.regsPerMultiprocessor);

    // 计算特定内核的占用率
    int blockSize = 256;  // 每个Block 256个线程
    int numWarps = blockSize / 32;

    // 假设每个线程使用32个寄存器
    int regsPerThread = 32;
    int regsPerBlock = regsPerThread * blockSize;

    // 受寄存器限制的Block数
    int maxBlocksByRegs = prop.regsPerMultiprocessor / regsPerBlock;

    // 受Warp限制的Block数
    int maxBlocksByWarps = (prop.maxThreadsPerMultiProcessor / 32) / numWarps;

    int maxBlocks = min(maxBlocksByRegs, maxBlocksByWarps);
    maxBlocks = min(maxBlocks, prop.maxBlocksPerMultiProcessor);

    float occupancy = static_cast<float>(maxBlocks * numWarps) /
                      (prop.maxThreadsPerMultiProcessor / 32) * 100.0f;
    printf("理论占用率: %.1f%%\n", occupancy);
}

// 使用CUDA运行时API计算占用率
void occupancyWithAPI() {
    int blockSize = 256;
    int minGridSize, optimalBlockSize;

    // 自动计算最优Block大小
    CUDA_CHECK(cudaOccupancyMaxPotentialBlockSize(
        &minGridSize, &optimalBlockSize, myKernel, 0, 0));

    printf("最优Block大小: %d\n", optimalBlockSize);
    printf("最小Grid大小: %d\n", minGridSize);

    // 计算占用率
    float occupancy;
    CUDA_CHECK(cudaOccupancyMaxActiveBlocksPerMultiprocessor(
        &minGridSize, myKernel, blockSize, 0));
    cudaOccupancyMaxActiveBlocksPerMultiprocessor(&minGridSize, myKernel, blockSize, 0);

    int device;
    CUDA_CHECK(cudaGetDevice(&device));
    cudaDeviceProp prop;
    CUDA_CHECK(cudaGetDeviceProperties(&prop, device));

    occupancy = static_cast<float>(minGridSize * blockSize) /
               prop.maxThreadsPerMultiProcessor * 100.0f;
    printf("占用率: %.1f%%\n", occupancy);
}
```

> 💡 **占用率不是越高越好**：100%占用率并不意味着最高性能。高占用率只是提供了更多Warp来隐藏延迟，但如果内核本身计算密集而非内存密集，较低的占用率也可能达到相同性能。通常50%以上的占用率就足以隐藏大部分延迟。

#### 3.4.2 内存对齐与Padding

```cpp
// 内存对齐对GPU性能的影响

// ❌ 未对齐的结构体（可能导致非合并访问和bank conflict）
struct UnalignedParticle {
    float x, y, z;      // 12字节
    float vx, vy;       // 8字节
    uint8_t type;       // 1字节
    // 编译器可能添加padding，但访问模式不佳
};

// ✅ 对齐的结构体
struct __align__(16) AlignedParticle {
    float x, y, z;      // 12字节
    float vx, vy;       // 8字节
    uint8_t type;       // 1字节
    uint8_t padding[3]; // 3字节对齐填充
    // 总大小: 24字节，16字节对齐
};

// ✅ 更优的SoA布局（避免所有对齐问题）
struct ParticleSoA {
    float* x;    // 独立数组，完美对齐
    float* y;
    float* z;
    float* vx;
    float* vy;
    uint8_t* type;
    int count;
};

// 共享内存Bank冲突
// 共享内存被组织为32个Bank，每个Bank 4字节宽
// 同一Warp中两个线程访问同一Bank的不同地址 → Bank冲突（串行化）

// ❌ Bank冲突示例
__global__ void bankConflictExample(float* output, int n) {
    __shared__ float sdata[256];
    int tid = threadIdx.x;

    // 步长为2的访问 → 2-way bank conflict
    // 因为 sdata[tid*2] 和 sdata[tid*2+32] 在同一个Bank
    sdata[tid] = output[tid * 2];  // 2-way bank conflict
    __syncthreads();
}

// ✅ 消除Bank冲突：使用Padding
__global__ void noBankConflictExample(float* output, int n) {
    // 添加一列padding，改变Bank映射
    __shared__ float sdata[257];  // 257 = 256 + 1 padding
    int tid = threadIdx.x;

    sdata[tid] = output[tid * 2];  // 无bank conflict
    __syncthreads();
}
```

#### 3.4.3 页锁定内存（Pinned Memory）

```cpp
// 页锁定内存（Pinned Memory / Page-Locked Memory）
// 阻止操作系统将内存页换出到磁盘，提高DMA传输效率

void pinnedMemoryExample() {
    const int N = 1024 * 1024;
    size_t size = N * sizeof(float);

    // ❌ 普通内存：cudaMemcpy需要先将其复制到暂存区
    // float* h_data = new float[N];  // 可被换出
    // cudaMemcpy(d_data, h_data, size, cudaMemcpyHostToDevice);
    // → 实际执行：h_data → 暂存区(pinned) → GPU（两次复制）

    // ✅ 页锁定内存：直接DMA传输
    float* h_pinned;
    CUDA_CHECK(cudaMallocHost(&h_pinned, size));  // 分配页锁定内存
    // cudaMemcpy(d_data, h_pinned, size, cudaMemcpyHostToDevice);
    // → 实际执行：h_pinned → GPU（一次DMA传输，快2-3倍）

    // 初始化数据
    for (int i = 0; i < N; ++i) {
        h_pinned[i] = static_cast<float>(i);
    }

    // ✅ 异步复制（页锁定内存 + 流 = 真正的异步）
    float* d_data;
    CUDA_CHECK(cudaMalloc(&d_data, size));

    cudaStream_t stream;
    CUDA_CHECK(cudaStreamCreate(&stream));

    // 页锁定内存 + 异步复制 = 真正的异步（CPU不等待）
    CUDA_CHECK(cudaMemcpyAsync(d_data, h_pinned, size,
                                cudaMemcpyHostToDevice, stream));

    // 可以继续在CPU上做其他工作...
    kernelFunction<<<blocks, threads, 0, stream>>>(d_data, N);

    CUDA_CHECK(cudaStreamSynchronize(stream));

    // 清理
    CUDA_CHECK(cudaFreeHost(h_pinned));  // 注意：用cudaFreeHost而非delete
    CUDA_CHECK(cudaFree(d_data));
    CUDA_CHECK(cudaStreamDestroy(stream));
}

// 可移植的页锁定内存（多GPU场景）
void portablePinnedMemory() {
    float* h_portable;
    unsigned int flags = cudaHostAllocPortable;  // 所有GPU可访问
    CUDA_CHECK(cudaHostAlloc(&h_portable, size, flags));

    // 其他可用标志：
    // cudaHostAllocWriteCombined  - 写合并内存（CPU读慢，GPU读快）
    // cudaHostAllocMapped         - 映射到GPU地址空间（零拷贝）
}
```

> ⚠️ **页锁定内存陷阱**：页锁定内存不会被换出到磁盘，过度使用会导致系统可用物理内存减少，严重时影响系统性能。建议仅在需要高性能DMA传输的场景使用，且总量不要超过系统物理内存的1/4。

## 4. 流与事件

### 4.1 CUDA流

```cpp
// CUDA流（Stream）- 异步执行，多个流可以并行

void streamExample() {
    const int N = 1024 * 1024;
    size_t size = N * sizeof(float);

    // 创建多个流
    const int numStreams = 4;
    cudaStream_t streams[numStreams];
    for (int i = 0; i < numStreams; ++i) {
        CUDA_CHECK(cudaStreamCreate(&streams[i]));
    }

    // 为每个流分配独立的内存
    float* d_data[numStreams];
    float* d_result[numStreams];
    for (int i = 0; i < numStreams; ++i) {
        CUDA_CHECK(cudaMalloc(&d_data[i], size));
        CUDA_CHECK(cudaMalloc(&d_result[i], size));
    }

    // 在不同流中异步执行操作
    float* h_data = new float[N];
    for (int i = 0; i < N; ++i) h_data[i] = static_cast<float>(i);

    for (int i = 0; i < numStreams; ++i) {
        // 异步内存复制（在流中排队）
        CUDA_CHECK(cudaMemcpyAsync(d_data[i], h_data, size,
                                    cudaMemcpyHostToDevice, streams[i]));

        // 异步内核执行
        int blocks = (N + 255) / 256;
        processKernel<<<blocks, 256, 0, streams[i]>>>(d_data[i], d_result[i], N);

        // 异步结果复制
        CUDA_CHECK(cudaMemcpyAsync(h_data, d_result[i], size,
                                    cudaMemcpyDeviceToHost, streams[i]));
    }

    // 等待所有流完成
    CUDA_CHECK(cudaDeviceSynchronize());

    // 清理
    for (int i = 0; i < numStreams; ++i) {
        CUDA_CHECK(cudaStreamDestroy(streams[i]));
        CUDA_CHECK(cudaFree(d_data[i]));
        CUDA_CHECK(cudaFree(d_result[i]));
    }
    delete[] h_data;
}
```

### 4.2 CUDA事件

```cpp
// CUDA事件（Event）- 精确计时和流间同步

void eventExample() {
    const int N = 1024 * 1024;
    size_t size = N * sizeof(float);

    float *d_in, *d_out;
    CUDA_CHECK(cudaMalloc(&d_in, size));
    CUDA_CHECK(cudaMalloc(&d_out, size));

    // 创建事件
    cudaEvent_t start, stop;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));

    // 记录开始时间
    CUDA_CHECK(cudaEventRecord(start, 0));

    // 执行内核
    int blocks = (N + 255) / 256;
    for (int i = 0; i < 100; ++i) {  // 执行100次取平均
        processKernel<<<blocks, 256>>>(d_in, d_out, N);
    }

    // 记录结束时间
    CUDA_CHECK(cudaEventRecord(stop, 0));
    CUDA_CHECK(cudaEventSynchronize(stop));

    // 计算耗时
    float milliseconds = 0;
    CUDA_CHECK(cudaEventElapsedTime(&milliseconds, start, stop));
    printf("内核执行100次总耗时: %.3f ms\n", milliseconds);
    printf("平均每次: %.3f ms\n", milliseconds / 100.0f);

    // 清理
    CUDA_CHECK(cudaEventDestroy(start));
    CUDA_CHECK(cudaEventDestroy(stop));
    CUDA_CHECK(cudaFree(d_in));
    CUDA_CHECK(cudaFree(d_out));
}

// 使用事件进行流间同步
void eventSyncExample() {
    cudaStream_t stream1, stream2;
    CUDA_CHECK(cudaStreamCreate(&stream1));
    CUDA_CHECK(cudaStreamCreate(&stream2));

    cudaEvent_t event;
    CUDA_CHECK(cudaEventCreate(&event));

    // stream1中执行操作
    kernelA<<<grid, block, 0, stream1>>>(...);

    // stream1记录事件
    CUDA_CHECK(cudaEventRecord(event, stream1));

    // stream2等待stream1的事件
    CUDA_CHECK(cudaStreamWaitEvent(stream2, event, 0));

    // stream2中的操作会在stream1完成后执行
    kernelB<<<grid, block, 0, stream2>>>(...);

    CUDA_CHECK(cudaDeviceSynchronize());

    CUDA_CHECK(cudaStreamDestroy(stream1));
    CUDA_CHECK(cudaStreamDestroy(stream2));
    CUDA_CHECK(cudaEventDestroy(event));
}
```

### 4.3 流式处理深入：流水线并行

```cpp
// 流水线并行（Pipeline Parallelism）
// 将数据传输和计算重叠，最大化GPU利用率

// 经典的三缓冲流水线模式
void pipelineParallel() {
    const int N = 10 * 1024 * 1024;  // 10M元素
    const int numChunks = 10;          // 分10个块
    const int chunkSize = N / numChunks;
    size_t chunkBytes = chunkSize * sizeof(float);

    // 创建3个流（三缓冲）
    const int NUM_STREAMS = 3;
    cudaStream_t streams[NUM_STREAMS];
    for (int i = 0; i < NUM_STREAMS; ++i) {
        CUDA_CHECK(cudaStreamCreate(&streams[i]));
    }

    // 分配页锁定主机内存（3个缓冲区）
    float* h_input[NUM_STREAMS];
    float* h_output[NUM_STREAMS];
    for (int i = 0; i < NUM_STREAMS; ++i) {
        CUDA_CHECK(cudaMallocHost(&h_input[i], chunkBytes));
        CUDA_CHECK(cudaMallocHost(&h_output[i], chunkBytes));
    }

    // 分配设备内存（3个缓冲区）
    float* d_input[NUM_STREAMS];
    float* d_output[NUM_STREAMS];
    for (int i = 0; i < NUM_STREAMS; ++i) {
        CUDA_CHECK(cudaMalloc(&d_input[i], chunkBytes));
        CUDA_CHECK(cudaMalloc(&d_output[i], chunkBytes));
    }

    // 流水线执行
    for (int chunk = 0; chunk < numChunks + NUM_STREAMS; ++chunk) {
        int sid = chunk % NUM_STREAMS;  // 当前流索引

        // 等待当前流完成之前的工作
        CUDA_CHECK(cudaStreamSynchronize(streams[sid]));

        if (chunk >= NUM_STREAMS) {
            // 取回上一轮的结果
            int resultChunk = chunk - NUM_STREAMS;
            // h_output[sid] 中已有结果，可以处理...
            printf("处理完成: chunk %d\n", resultChunk);
        }

        if (chunk < numChunks) {
            // 准备输入数据（CPU端）
            for (int i = 0; i < chunkSize; ++i) {
                h_input[sid][i] = static_cast<float>(chunk * chunkSize + i);
            }

            // 异步上传 → 计算 → 异步下载
            CUDA_CHECK(cudaMemcpyAsync(d_input[sid], h_input[sid],
                                        chunkBytes, cudaMemcpyHostToDevice,
                                        streams[sid]));

            int blocks = (chunkSize + 255) / 256;
            processKernel<<<blocks, 256, 0, streams[sid]>>>(
                d_input[sid], d_output[sid], chunkSize);

            CUDA_CHECK(cudaMemcpyAsync(h_output[sid], d_output[sid],
                                        chunkBytes, cudaMemcpyDeviceToHost,
                                        streams[sid]));
        }
    }

    // 清理
    for (int i = 0; i < NUM_STREAMS; ++i) {
        CUDA_CHECK(cudaStreamDestroy(streams[i]));
        CUDA_CHECK(cudaFreeHost(h_input[i]));
        CUDA_CHECK(cudaFreeHost(h_output[i]));
        CUDA_CHECK(cudaFree(d_input[i]));
        CUDA_CHECK(cudaFree(d_output[i]));
    }
}
```

> 🔬 **流水线并行的时序分析**：
> ```
> 无流水线：  [H2D][Compute][D2H]  [H2D][Compute][D2H]  [H2D][Compute][D2H]
>            ←──── chunk 0 ────→  ←──── chunk 1 ────→  ←──── chunk 2 ────→
>
> 有流水线：  [H2D0][Compute0][D2H0]
>                  [H2D1][Compute1][D2H1]
>                       [H2D2][Compute2][D2H2]
>            ← 传输与计算重叠 →
> ```
> 流水线模式下，数据传输和计算可以并行执行，总时间接近于 max(传输时间, 计算时间) × chunk数，而非 (传输时间 + 计算时间) × chunk数。

### 4.4 多GPU编程

```cpp
// 多GPU编程：在多个GPU上并行执行任务

void multiGpuExample() {
    int numDevices;
    CUDA_CHECK(cudaGetDeviceCount(&numDevices));
    printf("检测到 %d 个GPU\n", numDevices);

    if (numDevices < 2) {
        printf("需要至少2个GPU\n");
        return;
    }

    const int N = 10 * 1024 * 1024;
    size_t size = N * sizeof(float);
    int chunkSize = N / numDevices;

    // 每个GPU分配独立的内存和流
    struct GpuContext {
        float* d_input;
        float* d_output;
        float* h_input;
        float* h_output;
        cudaStream_t stream;
    };

    std::vector<GpuContext> contexts(numDevices);

    for (int dev = 0; dev < numDevices; ++dev) {
        CUDA_CHECK(cudaSetDevice(dev));

        CUDA_CHECK(cudaMalloc(&contexts[dev].d_input, chunkSize * sizeof(float)));
        CUDA_CHECK(cudaMalloc(&contexts[dev].d_output, chunkSize * sizeof(float)));
        CUDA_CHECK(cudaMallocHost(&contexts[dev].h_input, chunkSize * sizeof(float)));
        CUDA_CHECK(cudaMallocHost(&contexts[dev].h_output, chunkSize * sizeof(float)));
        CUDA_CHECK(cudaStreamCreate(&contexts[dev].stream));

        // 初始化输入数据
        for (int i = 0; i < chunkSize; ++i) {
            contexts[dev].h_input[i] = static_cast<float>(dev * chunkSize + i);
        }
    }

    // 在每个GPU上异步执行
    for (int dev = 0; dev < numDevices; ++dev) {
        CUDA_CHECK(cudaSetDevice(dev));

        CUDA_CHECK(cudaMemcpyAsync(contexts[dev].d_input, contexts[dev].h_input,
                                    chunkSize * sizeof(float),
                                    cudaMemcpyHostToDevice, contexts[dev].stream));

        int blocks = (chunkSize + 255) / 256;
        processKernel<<<blocks, 256, 0, contexts[dev].stream>>>(
            contexts[dev].d_input, contexts[dev].d_output, chunkSize);

        CUDA_CHECK(cudaMemcpyAsync(contexts[dev].h_output, contexts[dev].d_output,
                                    chunkSize * sizeof(float),
                                    cudaMemcpyDeviceToHost, contexts[dev].stream));
    }

    // 等待所有GPU完成
    for (int dev = 0; dev < numDevices; ++dev) {
        CUDA_CHECK(cudaSetDevice(dev));
        CUDA_CHECK(cudaStreamSynchronize(contexts[dev].stream));
    }

    // GPU间直接通信（需要NVLink或PCIe P2P）
    // 启用P2P访问
    for (int dev = 0; dev < numDevices; ++dev) {
        for (int other = 0; other < numDevices; ++other) {
            if (dev != other) {
                int canAccess;
                CUDA_CHECK(cudaDeviceCanAccessPeer(&canAccess, dev, other));
                if (canAccess) {
                    CUDA_CHECK(cudaSetDevice(dev));
                    CUDA_CHECK(cudaDeviceEnablePeerAccess(other, 0));
                    printf("启用GPU %d → GPU %d P2P访问\n", dev, other);
                }
            }
        }
    }

    // 清理
    for (int dev = 0; dev < numDevices; ++dev) {
        CUDA_CHECK(cudaSetDevice(dev));
        CUDA_CHECK(cudaFree(contexts[dev].d_input));
        CUDA_CHECK(cudaFree(contexts[dev].d_output));
        CUDA_CHECK(cudaFreeHost(contexts[dev].h_input));
        CUDA_CHECK(cudaFreeHost(contexts[dev].h_output));
        CUDA_CHECK(cudaStreamDestroy(contexts[dev].stream));
    }
}
```

## 5. CUDA与C++互操作

### 5.1 Thrust库

```cpp
// Thrust - CUDA的C++ STL风格并行算法库
#include <thrust/device_vector.h>
#include <thrust/host_vector.h>
#include <thrust/sort.h>
#include <thrust/reduce.h>
#include <thrust/transform.h>
#include <thrust/functional.h>
#include <thrust/sequence.h>

void thrustExample() {
    const int N = 1000000;

    // 创建设备向量（类似std::vector）
    thrust::device_vector<int> d_vec(N);

    // 填充序列 0, 1, 2, ..., N-1
    thrust::sequence(d_vec.begin(), d_vec.end());

    // 变换：每个元素平方
    thrust::transform(d_vec.begin(), d_vec.end(), d_vec.begin(),
                      thrust::placeholders::_1 * thrust::placeholders::_1);

    // 规约：求和
    int sum = thrust::reduce(d_vec.begin(), d_vec.end(), 0, thrust::plus<int>());
    printf("0到%d的平方和: %d\n", N - 1, sum);

    // 排序
    thrust::device_vector<int> d_random(N);
    thrust::sequence(d_random.begin(), d_random.end());
    thrust::sort(d_random.begin(), d_random.end(), thrust::greater<int>());

    // 复制到主机
    thrust::host_vector<int> h_vec = d_vec;
    printf("前10个元素: ");
    for (int i = 0; i < 10; ++i) {
        printf("%d ", h_vec[i]);
    }
    printf("\n");
}
```

### 5.2 自定义内核与C++结合

```cpp
// CUDA内核与C++类结合

// 设备端可用的数学工具
class DeviceMath {
public:
    __host__ __device__
    static float lerp(float a, float b, float t) {
        return a + t * (b - a);
    }

    __host__ __device__
    static float clamp(float x, float lo, float hi) {
        return fmaxf(lo, fminf(hi, x));
    }
};

// 在内核中使用C++类
__global__ void processKernel(float* data, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        float val = data[idx];
        val = DeviceMath::clamp(val, 0.0f, 1.0f);
        val = DeviceMath::lerp(val, 0.5f, 0.1f);
        data[idx] = val;
    }
}

// RAII封装CUDA资源
class CudaBuffer {
public:
    CudaBuffer() = default;

    explicit CudaBuffer(size_t size) {
        allocate(size);
    }

    ~CudaBuffer() {
        free();
    }

    // 禁止拷贝
    CudaBuffer(const CudaBuffer&) = delete;
    CudaBuffer& operator=(const CudaBuffer&) = delete;

    // 支持移动
    CudaBuffer(CudaBuffer&& other) noexcept
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    CudaBuffer& operator=(CudaBuffer&& other) noexcept {
        if (this != &other) {
            free();
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }

    void allocate(size_t size) {
        free();
        size_ = size;
        CUDA_CHECK(cudaMalloc(&data_, size));
    }

    void upload(const void* hostData, size_t size) {
        CUDA_CHECK(cudaMemcpy(data_, hostData, size, cudaMemcpyHostToDevice));
    }

    void download(void* hostData, size_t size) const {
        CUDA_CHECK(cudaMemcpy(hostData, data_, size, cudaMemcpyDeviceToHost));
    }

    void free() {
        if (data_) {
            cudaFree(data_);
            data_ = nullptr;
            size_ = 0;
        }
    }

    float* get() { return static_cast<float*>(data_); }
    const float* get() const { return static_cast<const float*>(data_); }
    size_t size() const { return size_; }

private:
    void* data_ = nullptr;
    size_t size_ = 0;
};
```

### 5.3 CUDA与C++模板深度集成

```cpp
// CUDA内核与C++模板结合，实现通用并行算法

// 通用并行变换内核
template<typename T, typename TransformOp>
__global__ void transformKernel(const T* input, T* output, int n,
                                 TransformOp op) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        output[idx] = op(input[idx]);
    }
}

// 通用并行归约内核
template<typename T, typename ReduceOp>
__global__ void reduceKernel(const T* input, T* output, int n,
                              ReduceOp op, T identity) {
    extern __shared__ char sharedMem[];
    T* sdata = reinterpret_cast<T*>(sharedMem);

    int tid = threadIdx.x;
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    sdata[tid] = (idx < n) ? input[idx] : identity;
    __syncthreads();

    for (int stride = blockDim.x / 2; stride > 0; stride >>= 1) {
        if (tid < stride) {
            sdata[tid] = op(sdata[tid], sdata[tid + stride]);
        }
        __syncthreads();
    }

    if (tid == 0) {
        output[blockIdx.x] = sdata[0];
    }
}

// C++风格的高层封装
template<typename T>
class GpuVector {
public:
    explicit GpuVector(size_t size) : size_(size) {
        CUDA_CHECK(cudaMalloc(&data_, size * sizeof(T)));
    }

    ~GpuVector() {
        if (data_) cudaFree(data_);
    }

    // 禁止拷贝，允许移动
    GpuVector(const GpuVector&) = delete;
    GpuVector& operator=(const GpuVector&) = delete;
    GpuVector(GpuVector&& other) noexcept
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    // 从主机数据上传
    void upload(const T* hostData, size_t count) {
        CUDA_CHECK(cudaMemcpy(data_, hostData, count * sizeof(T),
                               cudaMemcpyHostToDevice));
    }

    // 下载到主机
    void download(T* hostData, size_t count) const {
        CUDA_CHECK(cudaMemcpy(hostData, data_, count * sizeof(T),
                               cudaMemcpyDeviceToHost));
    }

    // 通用变换
    template<typename TransformOp>
    void transform(GpuVector<T>& output, TransformOp op) {
        int threads = 256;
        int blocks = (size_ + threads - 1) / threads;
        transformKernel<<<blocks, threads>>>(data_, output.data(), size_, op);
    }

    // 通用归约
    template<typename ReduceOp>
    T reduce(ReduceOp op, T identity) {
        int threads = 256;
        int blocks = (size_ + threads - 1) / threads;
        size_t sharedMemSize = threads * sizeof(T);

        GpuVector<T> d_partial(blocks);
        reduceKernel<<<blocks, threads, sharedMemSize>>>(
            data_, d_partial.data(), size_, op, identity);

        // 继续归约直到只剩一个元素
        while (blocks > 1) {
            int newBlocks = (blocks + threads - 1) / threads;
            GpuVector<T> d_temp(newBlocks);
            reduceKernel<<<newBlocks, threads, sharedMemSize>>>(
                d_partial.data(), d_temp.data(), blocks, op, identity);
            blocks = newBlocks;
        }

        T result;
        CUDA_CHECK(cudaMemcpy(&result, d_partial.data(), sizeof(T),
                               cudaMemcpyDeviceToHost));
        return result;
    }

    T* data() { return data_; }
    const T* data() const { return data_; }
    size_t size() const { return size_; }

private:
    T* data_ = nullptr;
    size_t size_ = 0;
};

// 使用示例
void gpuVectorExample() {
    const int N = 1000000;
    std::vector<float> h_data(N);
    for (int i = 0; i < N; ++i) h_data[i] = static_cast<float>(i);

    GpuVector<float> gpuVec(N);
    gpuVec.upload(h_data.data(), N);

    // 变换：每个元素平方
    GpuVector<float> gpuResult(N);
    gpuVec.transform(gpuResult, [] __device__ (float x) {
        return x * x;
    });

    // 归约：求和
    float sum = gpuResult.reduce(
        [] __device__ (float a, float b) { return a + b; }, 0.0f);
    printf("平方和: %f\n", sum);

    // 归约：求最大值
    float maxVal = gpuVec.reduce(
        [] __device__ (float a, float b) { return fmaxf(a, b); },
        -FLT_MAX);
    printf("最大值: %f\n", maxVal);
}
```

### 5.4 CUDA动态并行

```cpp
// CUDA动态并行（Dynamic Parallelism）
// 允许GPU内核在运行时启动新的内核（从Volta架构开始支持）

// 示例：递归归约
__global__ void recursiveReduce(float* data, float* result, int n) {
    extern __shared__ float sdata[];

    int tid = threadIdx.x;
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    sdata[tid] = (idx < n) ? data[idx] : 0.0f;
    __syncthreads();

    // Block内归约
    for (int stride = blockDim.x / 2; stride > 0; stride >>= 1) {
        if (tid < stride) {
            sdata[tid] += sdata[tid + stride];
        }
        __syncthreads();
    }

    if (tid == 0) {
        data[blockIdx.x] = sdata[0];  // 写回部分结果

        // 如果这是最后一个Block，递归启动新内核
        if (gridDim.x > 1 && blockIdx.x == gridDim.x - 1) {
            int newBlocks = (gridDim.x + blockDim.x - 1) / blockDim.x;
            recursiveReduce<<<newBlocks, blockDim.x, blockDim.x * sizeof(float)>>>(
                data, result, gridDim.x);
        } else if (gridDim.x == 1) {
            *result = sdata[0];
        }
    }
}

// ⚠️ 动态并行的注意事项：
// 1. 需要额外的设备端内存用于启动队列
// 2. 子内核启动有额外开销（约10-50μs）
// 3. 递归深度有限制（通常64层）
// 4. 调试更困难
// 5. 优先使用迭代方式而非递归
```

### 5.5 CUDA与OpenGL/Vulkan互操作

```cpp
// CUDA与图形API互操作：在CUDA和图形渲染之间共享GPU缓冲区

// CUDA-OpenGL互操作
#include <cuda_gl_interop.h>

class CudaGLInterop {
public:
    bool init(GLuint vbo, GLuint texture) {
        // 注册OpenGL缓冲区对象
        CUDA_CHECK(cudaGraphicsGLRegisterBuffer(
            &vboResource_, vbo,
            cudaGraphicsMapFlagsWriteDiscard));

        // 注册OpenGL纹理
        CUDA_CHECK(cudaGraphicsGLRegisterImage(
            &textureResource_, texture, GL_TEXTURE_2D,
            cudaGraphicsMapFlagsWriteDiscast));

        return true;
    }

    // 在CUDA中修改VBO数据
    void updateVBO(float* cudaOutput, size_t size) {
        // 映射OpenGL缓冲区到CUDA
        CUDA_CHECK(cudaGraphicsMapResources(1, &vboResource_, 0));

        float* d_ptr;
        size_t numBytes;
        CUDA_CHECK(cudaGraphicsResourceGetMappedPointer(
            reinterpret_cast<void**>(&d_ptr), &numBytes, vboResource_));

        // 在CUDA中处理数据
        processKernel<<<blocks, threads>>>(d_ptr, numBytes / sizeof(float));

        // 取消映射（OpenGL可以继续使用）
        CUDA_CHECK(cudaGraphicsUnmapResources(1, &vboResource_, 0));
    }

    void destroy() {
        if (vboResource_) {
            cudaGraphicsUnregisterResource(vboResource_);
        }
        if (textureResource_) {
            cudaGraphicsUnregisterResource(textureResource_);
        }
    }

private:
    cudaGraphicsResource* vboResource_ = nullptr;
    cudaGraphicsResource* textureResource_ = nullptr;
};

// 典型应用场景：
// 1. CUDA粒子模拟 → OpenGL渲染
// 2. CUDA图像处理 → OpenGL纹理显示
// 3. CUDA物理模拟 → Vulkan渲染
// 4. CUDA视频解码 → OpenGL纹理显示
```

## 6. cuBLAS与cuDNN

### 6.1 cuBLAS矩阵运算

```cpp
// cuBLAS - CUDA基础线性代数子程序库
#include <cublas_v2.h>

class CuBLASContext {
public:
    bool init() {
        cublasStatus_t status = cublasCreate(&handle_);
        if (status != CUBLAS_STATUS_SUCCESS) {
            fprintf(stderr, "cuBLAS初始化失败\n");
            return false;
        }
        return true;
    }

    void destroy() {
        if (handle_) {
            cublasDestroy(handle_);
            handle_ = nullptr;
        }
    }

    cublasHandle_t getHandle() const { return handle_; }

private:
    cublasHandle_t handle_ = nullptr;
};

// 矩阵乘法：C = alpha * A * B + beta * C
void matrixMultiplyCuBLAS(const CuBLASContext& ctx,
                          const float* d_A, const float* d_B, float* d_C,
                          int M, int N, int K,
                          float alpha = 1.0f, float beta = 0.0f) {
    // cuBLAS使用列优先存储，需要注意转置
    // C = A * B 在cuBLAS中表示为：
    // C = alpha * op(A) * op(B) + beta * C
    // op(A) = A^T (行优先转列优先)

    cublasSgemm(ctx.getHandle(),
                CUBLAS_OP_N, CUBLAS_OP_N,  // 不转置
                N, M, K,                     // 输出维度
                &alpha,
                d_B, N,                      // B矩阵（列优先）
                d_A, K,                      // A矩阵（列优先）
                &beta,
                d_C, N);                     // C矩阵（列优先）
}

// 向量点积
float dotProductCuBLAS(const CuBLASContext& ctx,
                       const float* d_a, const float* d_b, int n) {
    float result = 0.0f;
    cublasSdot(ctx.getHandle(), n, d_a, 1, d_b, 1, &result);
    return result;
}

// 向量范数
float normCuBLAS(const CuBLASContext& ctx, const float* d_x, int n) {
    float result = 0.0f;
    cublasSnrm2(ctx.getHandle(), n, d_x, 1, &result);
    return result;
}
```

### 6.2 cuDNN深度学习

```cpp
// cuDNN - CUDA深度神经网络库
#include <cudnn.h>

class CuDNNContext {
public:
    bool init() {
        cudnnStatus_t status = cudnnCreate(&handle_);
        if (status != CUDNN_STATUS_SUCCESS) {
            fprintf(stderr, "cuDNN初始化失败\n");
            return false;
        }
        return true;
    }

    void destroy() {
        if (handle_) {
            cudnnDestroy(handle_);
            handle_ = nullptr;
        }
    }

    cudnnHandle_t getHandle() const { return handle_; }

private:
    cudnnHandle_t handle_ = nullptr;
};

// 卷积前向传播
class ConvolutionLayer {
public:
    bool init(const CuDNNContext& ctx,
              int batchSize, int inputChannels, int inputHeight, int inputWidth,
              int outputChannels, int kernelSize, int padding, int stride) {
        // 创建输入张量描述符
        cudnnCreateTensorDescriptor(&inputDesc_);
        cudnnSetTensor4dDescriptor(inputDesc_, CUDNN_TENSOR_NCHW,
                                    CUDNN_DATA_FLOAT,
                                    batchSize, inputChannels,
                                    inputHeight, inputWidth);

        // 创建卷积核描述符
        cudnnCreateFilterDescriptor(&filterDesc_);
        cudnnSetFilter4dDescriptor(filterDesc_, CUDNN_DATA_FLOAT,
                                    CUDNN_TENSOR_NCHW,
                                    outputChannels, inputChannels,
                                    kernelSize, kernelSize);

        // 创建卷积描述符
        cudnnCreateConvolutionDescriptor(&convDesc_);
        cudnnSetConvolution2dDescriptor(convDesc_,
                                         padding, padding,    // padding
                                         stride, stride,      // stride
                                         1, 1,                // dilation
                                         CUDNN_CROSS_CORRELATION,
                                         CUDNN_DATA_FLOAT);

        // 计算输出维度
        cudnnGetConvolution2dForwardOutputDim(convDesc_, inputDesc_, filterDesc_,
                                               &batchSize_, &outputChannels_,
                                               &outputHeight_, &outputWidth_);

        // 创建输出张量描述符
        cudnnCreateTensorDescriptor(&outputDesc_);
        cudnnSetTensor4dDescriptor(outputDesc_, CUDNN_TENSOR_NCHW,
                                    CUDNN_DATA_FLOAT,
                                    batchSize_, outputChannels_,
                                    outputHeight_, outputWidth_);

        // 选择最优算法
        int algoCount = 0;
        cudnnGetConvolutionForwardAlgorithmMaxCount(ctx.getHandle(), &algoCount);

        cudnnConvolutionFwdAlgoPerf_t algoPerf;
        cudnnFindConvolutionForwardAlgorithm(ctx.getHandle(),
                                              inputDesc_, filterDesc_, convDesc_, outputDesc_,
                                              1, &algoCount, &algoPerf);
        algo_ = algoPerf.algo;

        // 分配工作空间
        size_t workspaceSize = 0;
        cudnnGetConvolutionForwardWorkspaceSize(ctx.getHandle(),
                                                 inputDesc_, filterDesc_, convDesc_, outputDesc_,
                                                 algo_, &workspaceSize);
        if (workspaceSize > 0) {
            cudaMalloc(&workspace_, workspaceSize);
        }
        workspaceSize_ = workspaceSize;

        return true;
    }

    void forward(const CuDNNContext& ctx,
                 const float* input, const float* filter, float* output) {
        float alpha = 1.0f, beta = 0.0f;
        cudnnConvolutionForward(ctx.getHandle(),
                                &alpha,
                                inputDesc_, input,
                                filterDesc_, filter,
                                convDesc_, algo_,
                                workspace_, workspaceSize_,
                                &beta,
                                outputDesc_, output);
    }

    void destroy() {
        if (inputDesc_) cudnnDestroyTensorDescriptor(inputDesc_);
        if (outputDesc_) cudnnDestroyTensorDescriptor(outputDesc_);
        if (filterDesc_) cudnnDestroyFilterDescriptor(filterDesc_);
        if (convDesc_) cudnnDestroyConvolutionDescriptor(convDesc_);
        if (workspace_) cudaFree(workspace_);
    }

private:
    cudnnTensorDescriptor_t inputDesc_ = nullptr;
    cudnnTensorDescriptor_t outputDesc_ = nullptr;
    cudnnFilterDescriptor_t filterDesc_ = nullptr;
    cudnnConvolutionDescriptor_t convDesc_ = nullptr;
    cudnnConvolutionFwdAlgo_t algo_;
    void* workspace_ = nullptr;
    size_t workspaceSize_ = 0;
    int batchSize_ = 0, outputChannels_ = 0, outputHeight_ = 0, outputWidth_ = 0;
};
```

## 7. 常见陷阱与性能优化

### 7.1 CUDA编程常见陷阱

#### 7.1.1 忽视错误检查

```cpp
// ❌ 不检查任何CUDA错误
cudaMalloc(&d_data, size);
myKernel<<<blocks, threads>>>(d_data, N);
cudaMemcpy(h_result, d_data, size, cudaMemcpyDeviceToHost);
// 如果内核出错，后续所有CUDA调用都可能失败，但你看不到任何错误信息

// ✅ 每个CUDA调用都检查错误
CUDA_CHECK(cudaMalloc(&d_data, size));
myKernel<<<blocks, threads>>>(d_data, N);
CUDA_CHECK_LAST_ERROR();  // 检查内核启动错误
CUDA_CHECK(cudaDeviceSynchronize());  // 确保内核执行完成
CUDA_CHECK(cudaMemcpy(h_result, d_data, size, cudaMemcpyDeviceToHost));
```

#### 7.1.2 过度同步

```cpp
// ❌ 频繁同步，破坏异步性
for (int i = 0; i < 100; ++i) {
    kernelA<<<blocks, threads>>>(d_data, N);
    cudaDeviceSynchronize();  // 每次都同步，GPU空闲等待
    kernelB<<<blocks, threads>>>(d_data, N);
    cudaDeviceSynchronize();  // 又同步
}

// ✅ 只在必要时同步
for (int i = 0; i < 100; ++i) {
    kernelA<<<blocks, threads>>>(d_data, N);
    kernelB<<<blocks, threads>>>(d_data, N);
    // 让GPU异步执行，不需要CPU干预
}
cudaDeviceSynchronize();  // 最后同步一次即可
```

#### 7.1.3 不合理的Block大小

```cpp
// ❌ Block大小不是32的倍数（Warp不完整）
myKernel<<<grid, 100>>>(...);  // 100不是32的倍数，最后一个Warp只有4个线程

// ❌ Block太小（占用率低）
myKernel<<<grid, 32>>>(...);  // 每个Block只有1个Warp，占用率极低

// ❌ Block太大（超过限制或占用率低）
myKernel<<<grid, 1024>>>(...);  // 可能超过寄存器/共享内存限制

// ✅ Block大小为32的倍数，通常128-512
// 256是最常用的Block大小
myKernel<<<grid, 256>>>(...);
myKernel<<<grid, 128>>>(...);  // 需要更多寄存器时用较小的Block
```

#### 7.1.4 原子操作滥用

```cpp
// ❌ 大量线程竞争同一个原子变量
__global__ void badAtomic(float* input, float* result, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        // 所有线程竞争同一个内存地址，串行化
        atomicAdd(result, input[idx]);
    }
}

// ✅ 使用共享内存进行Block内归约，最后只做一次原子操作
__global__ void goodAtomic(float* input, float* result, int n) {
    __shared__ float sdata[256];
    int tid = threadIdx.x;
    int idx = blockIdx.x * blockDim.x + threadIdx.x;

    sdata[tid] = (idx < n) ? input[idx] : 0.0f;
    __syncthreads();

    // Block内归约（无原子操作）
    for (int stride = blockDim.x / 2; stride > 0; stride >>= 1) {
        if (tid < stride) {
            sdata[tid] += sdata[tid + stride];
        }
        __syncthreads();
    }

    // 每个Block只做一次原子操作
    if (tid == 0) {
        atomicAdd(result, sdata[0]);
    }
}
```

### 7.2 性能优化清单

```
1. 最大化并行度
   - Block大小为32的倍数（128-512最佳）
   - 确保足够的Block数（至少是SM数的4倍）
   - 使用cudaOccupancyMaxPotentialBlockSize计算最优配置

2. 优化内存访问
   - 全局内存：确保合并访问（连续线程访问连续地址）
   - 共享内存：避免Bank冲突（使用Padding）
   - 使用SoA而非AoS布局
   - 利用常量内存缓存只读数据
   - 使用纹理内存缓存空间局部性数据

3. 减少数据传输
   - 尽量在GPU上完成所有计算
   - 使用页锁定内存提高传输速度
   - 使用统一内存简化编程（性能可能不如手动管理）
   - 流水线化数据传输和计算

4. 避免分支发散
   - 同一Warp内的线程尽量走相同分支
   - 使用条件赋值替代条件分支
   - 对数据进行排序/重排，使相同条件的线程在同一Warp

5. 合理使用同步
   - 最小化__syncthreads调用
   - 避免不必要的cudaDeviceSynchronize
   - 使用流和事件进行细粒度同步

6. 使用高性能库
   - cuBLAS：矩阵运算
   - cuDNN：深度学习
   - cuFFT：快速傅里叶变换
   - Thrust：STL风格并行算法
   - cuRAND：随机数生成
   - NPP：图像处理原语
```

### 7.3 性能分析工具

```cpp
// 使用NVIDIA Nsight Compute分析内核性能
// 命令行：ncu --set full -o report ./my_program

// 使用NVIDIA Nsight Systems分析整体时间线
// 命令行：nsys profile -o report ./my_program

// 在代码中使用性能计数器
void profileWithEvents() {
    cudaEvent_t start, stop;
    CUDA_CHECK(cudaEventCreate(&start));
    CUDA_CHECK(cudaEventCreate(&stop));

    // 预热（避免首次执行的额外开销）
    warmupKernel<<<blocks, threads>>>(d_data, N);
    CUDA_CHECK(cudaDeviceSynchronize());

    // 正式计时
    CUDA_CHECK(cudaEventRecord(start));
    for (int i = 0; i < 100; ++i) {
        myKernel<<<blocks, threads>>>(d_data, N);
    }
    CUDA_CHECK(cudaEventRecord(stop));
    CUDA_CHECK(cudaEventSynchronize(stop));

    float ms;
    CUDA_CHECK(cudaEventElapsedTime(&ms, start, stop));
    printf("平均内核耗时: %.3f ms\n", ms / 100.0f);

    // 计算带宽利用率
    size_t bytesRead = N * sizeof(float);
    size_t bytesWritten = N * sizeof(float);
    float bandwidth = (bytesRead + bytesWritten) / (ms / 100.0f * 1e-3) / 1e9;
    printf("有效带宽: %.2f GB/s\n", bandwidth);
    // 与理论带宽比较，判断是否接近内存瓶颈
}
```

## 8. 跨平台与替代方案

### 8.1 CUDA的局限性

CUDA是NVIDIA专有技术，仅在NVIDIA GPU上可用。如果需要支持AMD、Intel或其他GPU，需要考虑替代方案：

| 方案 | 描述 | 性能 | 易用性 | 跨平台 |
|------|------|------|--------|--------|
| CUDA | NVIDIA专有 | 最高 | 高 | 仅NVIDIA |
| HIP | AMD的CUDA兼容层 | 高 | 高（与CUDA几乎相同） | NVIDIA + AMD |
| OpenCL | 开放标准 | 中 | 中 | 广泛 |
| SYCL | 基于C++的开放标准 | 中高 | 高 | 广泛 |
| Vulkan Compute | 图形API的计算着色器 | 中高 | 低 | 广泛 |
| oneAPI/DPC++ | Intel的SYCL实现 | 中高 | 高 | Intel + NVIDIA + AMD |

### 8.2 HIP：CUDA代码移植到AMD

```cpp
// HIP（Heterogeneous-Computing Interface for Portability）
// AMD开发的CUDA兼容层，可以将CUDA代码几乎无损地移植到AMD GPU

// CUDA代码：
// __global__ void myKernel(float* data, int n) { ... }
// myKernel<<<blocks, threads>>>(d_data, N);
// cudaMalloc(&d_data, size);
// cudaMemcpy(d_data, h_data, size, cudaMemcpyHostToDevice);

// HIP代码（几乎相同）：
// __global__ void myKernel(float* data, int n) { ... }
// hipLaunchKernelGGL(myKernel, dim3(blocks), dim3(threads), 0, 0, d_data, N);
// hipMalloc(&d_data, size);
// hipMemcpy(d_data, h_data, size, hipMemcpyHostToDevice);

// 使用hipify工具自动转换：
// hipify-perl my_cuda_code.cu > my_hip_code.hip
```

### 8.3 SYCL：现代C++跨平台GPU编程

```cpp
// SYCL是基于纯C++的异构编程模型
// 无需特殊编译器扩展，使用标准C++语法

#include <sycl/sycl.hpp>

void syclExample() {
    // 创建队列（选择设备）
    sycl::queue q(sycl::gpu_selector_v);

    const int N = 1024 * 1024;
    std::vector<float> h_a(N), h_b(N), h_c(N);

    // 分配设备内存
    float *d_a = sycl::malloc_device<float>(N, q);
    float *d_b = sycl::malloc_device<float>(N, q);
    float *d_c = sycl::malloc_device<float>(N, q);

    // 上传数据
    q.memcpy(d_a, h_a.data(), N * sizeof(float));
    q.memcpy(d_b, h_b.data(), N * sizeof(float));

    // 执行内核
    q.parallel_for(sycl::range<1>(N), [=](sycl::id<1> idx) {
        d_c[idx] = d_a[idx] + d_b[idx];
    }).wait();

    // 下载结果
    q.memcpy(h_c.data(), d_c, N * sizeof(float)).wait();

    // 释放
    sycl::free(d_a, q);
    sycl::free(d_b, q);
    sycl::free(d_c, q);
}
```

> 💡 **选择建议**：
> - 仅需支持NVIDIA GPU → CUDA（生态最完善，性能最高）
> - 需要支持NVIDIA + AMD → HIP（代码改动最小）
> - 需要广泛跨平台 → SYCL/DPC++（现代C++风格，Intel主推）
> - 图形渲染中的GPU计算 → Vulkan Compute / 计算着色器

## 9. 本章小结

本章深入讲解了CUDA编程的核心知识，要点如下：

| 主题 | 核心要点 |
|------|---------|
| 编程模型 | Kernel函数、函数限定符、错误处理、并行归约、前缀和、图像处理 |
| 线程层次 | Grid→Block→Warp→Thread、索引计算、分支发散 |
| 内存管理 | 全局/共享/常量/纹理内存、统一内存、Tiling优化、占用率、Bank冲突、页锁定内存 |
| 流与事件 | 异步执行、精确计时、流间同步、流水线并行、多GPU编程 |
| C++互操作 | Thrust库、RAII封装、`__host__ __device__`函数、模板内核、动态并行、图形API互操作 |
| cuBLAS/cuDNN | 矩阵乘法、向量运算、卷积前向传播 |
| 性能优化 | 错误检查、避免过度同步、合理Block大小、原子操作优化、性能分析工具 |
| 跨平台 | HIP移植、SYCL替代方案、选择建议 |

**关键理解**：

1. **CUDA是C的GPU扩展**：通过少量关键字扩展（`__global__`等）实现GPU编程
2. **线程层次是核心概念**：合理组织Grid/Block大小直接影响性能
3. **共享内存是性能关键**：利用共享内存减少全局内存访问，实现Tiling优化
4. **流实现异步并行**：多个流可以重叠数据传输和计算
5. **cuBLAS/cuDNN是工业级库**：直接使用高度优化的库，避免重复造轮子
6. **内存访问模式决定性能**：合并访问、Bank冲突避免、SoA布局是优化的核心
7. **流水线并行最大化GPU利用率**：将数据传输和计算重叠，减少GPU空闲时间
8. **CUDA不是唯一选择**：HIP、SYCL等替代方案提供了跨平台GPU编程的可能性

> **下一步**：在[图形数学与算法](04-图形数学与算法.md)中，我们将学习图形编程所需的数学基础和算法。
