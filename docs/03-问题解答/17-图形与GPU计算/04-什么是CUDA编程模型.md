# 什么是CUDA编程模型
> 📖 相关章节：[图形编程概述](../../11-图形与GPU计算/00-图形编程概述.md)

> **CUDA让普通程序员也能驾驭GPU——你不需要懂图形学，只需要理解"线程层次"和"内存层次"两个核心概念。** Grid→Block→Thread的三级线程组织，加上全局内存→共享内存→寄存器的三级存储，构成了CUDA编程的全部基础。

***

### 1. 本质洞察

**CUDA（Compute Unified Device Architecture）** 是NVIDIA推出的GPU通用计算平台和编程模型。它的核心思想是：**用C/C++风格的语法编写在GPU上并行执行的代码，通过Grid/Block/Thread三级层次组织数百万个线程，利用不同层次的内存实现高效数据访问**。

***

### 2. 生活类比

| CUDA概念 | 类比 | 说明 |
|---------|------|------|
| Grid | 整个工厂 | 包含所有车间 |
| Block | 车间 | 一个车间内的工人可以共享工具箱 |
| Thread | 工人 | 每个工人独立执行任务 |
| 全局内存 | 工厂仓库 | 所有人都能访问，但距离远、速度慢 |
| 共享内存 | 车间工具箱 | 只有本车间的人能用，距离近、速度快 |
| 寄存器 | 工人口袋 | 只有自己能用，最快 |
| Warp | 工作小组 | 32个工人必须同时做同样的动作 |

***

### 3. 线程层次：Grid / Block / Thread

#### 3.1 三级线程组织

```
┌─────────────────────────────────────────────────────────────┐
│                        Grid（网格）                           │
│  gridDim.x × gridDim.y × gridDim.z                          │
│                                                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │   Block(0,0) │  │   Block(1,0) │  │   Block(2,0) │       │
│  │  blockDim.x  │  │  blockDim.x  │  │  blockDim.x  │       │
│  │  × blockDim.y│  │  × blockDim.y│  │  × blockDim.y│       │
│  │              │  │              │  │              │       │
│  │ T(0,0) T(1,0)│  │ T(0,0) T(1,0)│  │ T(0,0) T(1,0)│       │
│  │ T(0,1) T(1,1)│  │ T(0,1) T(1,1)│  │ T(0,1) T(1,1)│       │
│  │ T(0,2) T(1,2)│  │ T(0,2) T(1,2)│  │ T(0,2) T(1,2)│       │
│  └──────────────┘  └──────────────┘  └──────────────┘       │
│                                                               │
│  ┌──────────────┐  ┌──────────────┐  ┌──────────────┐       │
│  │   Block(0,1) │  │   Block(1,1) │  │   Block(2,1) │       │
│  └──────────────┘  └──────────────┘  └──────────────┘       │
└─────────────────────────────────────────────────────────────┘
```

#### 3.2 线程索引计算

```cpp
// 1D网格 + 1D块（最常用）
int globalThreadId = blockIdx.x * blockDim.x + threadIdx.x;

// 2D网格 + 2D块（图像处理常用）
int x = blockIdx.x * blockDim.x + threadIdx.x;
int y = blockIdx.y * blockDim.y + threadIdx.y;

// 3D网格 + 3D块（体数据常用）
int x = blockIdx.x * blockDim.x + threadIdx.x;
int y = blockIdx.y * blockDim.y + threadIdx.y;
int z = blockIdx.z * blockDim.z + threadIdx.z;

// 完整的1D全局索引计算
__global__ void kernel(float* data, int n) {
    // 计算全局线程ID
    int tid = blockIdx.x * blockDim.x + threadIdx.x;

    // 边界检查（重要！线程数可能超过数据量）
    if (tid < n) {
        data[tid] = data[tid] * 2.0f;
    }
}

// 启动核函数
int n = 1000000;
int blockSize = 256;
int numBlocks = (n + blockSize - 1) / blockSize;  // 向上取整
kernel<<<numBlocks, blockSize>>>(d_data, n);
```

#### 3.3 Block大小选择

```cpp
// Block大小必须是32的倍数（Warp大小）
// 常见选择：128, 256, 512

// 选择原则：
// 1. 必须是32的倍数（避免Warp分歧浪费）
// 2. 不能超过1024（硬件限制）
// 3. 需要考虑寄存器和共享内存的使用量

// 查询设备限制
cudaDeviceProp prop;
cudaGetDeviceProperties(&prop, 0);
printf("每个Block最大线程数: %d\n", prop.maxThreadsPerBlock);       // 通常1024
printf("每个SM最大线程数: %d\n", prop.maxThreadsPerMultiProcessor); // 通常1536-2048
printf("每个Block最大共享内存: %zu KB\n", prop.sharedMemPerBlock);  // 通常48KB

// 计算占用率
int blockSize = 256;
int numWarps = blockSize / 32;
int regsPerThread = 32;  // 从编译信息获取
int sharedMemPerBlock = 0;

cudaOccupancyMaxActiveBlocksPerMultiprocessor(
    &numBlocks, kernel, blockSize, sharedMemPerBlock
);
float occupancy = (numBlocks * blockSize) / (float)prop.maxThreadsPerMultiProcessor;
printf("占用率: %.1f%%\n", occupancy * 100);
```

***

### 4. 内存层次

#### 4.1 CUDA内存层次结构

```
┌─────────────────────────────────────────────────────┐
│                    GPU 芯片                          │
│                                                       │
│  ┌─────────────────────────────────────────────────┐ │
│  │              L2 缓存（数MB）                      │ │
│  │  ┌───────────┐  ┌───────────┐  ┌───────────┐   │ │
│  │  │   SM 0    │  │   SM 1    │  │   SM N    │   │ │
│  │  │ ┌───────┐ │  │ ┌───────┐ │  │ ┌───────┐ │   │ │
│  │  │ │寄存器 │ │  │ │寄存器 │ │  │ │寄存器 │ │   │ │
│  │  │ │(最快) │ │  │ │(最快) │ │  │ │(最快) │ │   │ │
│  │  │ └───────┘ │  │ └───────┘ │  │ └───────┘ │   │ │
│  │  │ ┌───────┐ │  │ ┌───────┐ │  │ ┌───────┐ │   │ │
│  │  │ │共享内存│ │  │ │共享内存│ │  │ │共享内存│ │   │ │
│  │  │ │(快)   │ │  │ │(快)   │ │  │ │(快)   │ │   │ │
│  │  │ │Block内 │ │  │ │Block内 │ │  │ │Block内 │ │   │ │
│  │  │ │共享    │ │  │ │共享    │ │  │ │共享    │ │   │ │
│  │  │ └───────┘ │  │ └───────┘ │  │ └───────┘ │   │ │
│  │  │ ┌───────┐ │  │ ┌───────┐ │  │ ┌───────┐ │   │ │
│  │  │ │L1缓存 │ │  │ │L1缓存 │ │  │ │L1缓存 │ │   │ │
│  │  │ └───────┘ │  │ └───────┘ │  │ └───────┘ │   │ │
│  │  └───────────┘  └───────────┘  └───────────┘   │ │
│  └─────────────────────────────────────────────────┘ │
│                                                       │
│  ┌─────────────────────────────────────────────────┐ │
│  │           全局内存 / 显存（数GB-数十GB）          │ │
│  │           所有SM共享，带宽高但延迟大              │ │
│  └─────────────────────────────────────────────────┘ │
└─────────────────────────────────────────────────────┘
```

#### 4.2 各级内存对比

| 内存类型 | 作用域 | 生命周期 | 延迟 | 带宽 | 大小 |
|---------|--------|---------|------|------|------|
| 寄存器 | 单线程 | 函数内 | ~1周期 | 极高 | ~256KB/SM |
| 共享内存 | Block内 | Block生命周期 | ~20周期 | ~1.5TB/s | 48-96KB/SM |
| L1缓存 | Block内 | 自动管理 | ~30周期 | ~1TB/s | 128KB/SM |
| L2缓存 | 全局 | 自动管理 | ~200周期 | ~1TB/s | 数MB |
| 全局内存 | 全局 | 应用程序控制 | ~400周期 | ~900GB/s | 数-数十GB |
| 常量内存 | 全局 | 应用程序控制 | ~5周期(缓存命中) | ~1TB/s | 64KB |
| 纹理内存 | 全局 | 应用程序控制 | ~100周期(缓存命中) | ~1TB/s | 与全局内存共享 |

#### 4.3 共享内存的使用

```cpp
// 共享内存：矩阵转置优化
__global__ void matrixTranspose(float* input, float* output, int width, int height) {
    // 声明共享内存（Block内所有线程共享）
    __shared__ float tile[32][33];  // 注意：33列避免bank冲突

    // 读取全局内存到共享内存
    int x = blockIdx.x * 32 + threadIdx.x;
    int y = blockIdx.y * 32 + threadIdx.y;

    if (x < width && y < height) {
        tile[threadIdx.y][threadIdx.x] = input[y * width + x];
    }

    // 同步：确保所有线程都写完共享内存
    __syncthreads();

    // 从共享内存写回全局内存（转置）
    int outX = blockIdx.y * 32 + threadIdx.x;
    int outY = blockIdx.x * 32 + threadIdx.y;

    if (outX < height && outY < width) {
        output[outY * height + outX] = tile[threadIdx.x][threadIdx.y];
    }
}
```

#### 4.4 共享内存Bank冲突

```cpp
// 共享内存被分为32个bank
// 每个bank宽度4字节，每个周期每个bank只能服务一个地址

// 无冲突访问：每个线程访问不同的bank
__shared__ float data[256];
// 线程i访问data[i] → 无冲突（每个线程访问不同bank）

// 2路bank冲突：两个线程访问同一bank的不同地址
// 线程0访问data[0]，线程1访问data[32] → 2路冲突（bank 0被两个线程同时访问）

// 解决方法：填充（Padding）
__shared__ float tile[32][33];  // 33列而非32列
// tile[y][x] 和 tile[y+1][x] 不在同一bank

// 示例：矩阵乘法中的bank冲突优化
__global__ void matrixMul(float* A, float* B, float* C, int N) {
    __shared__ float sA[32][33];  // 填充一列避免bank冲突
    __shared__ float sB[32][33];

    int row = blockIdx.y * 32 + threadIdx.y;
    int col = blockIdx.x * 32 + threadIdx.x;

    float sum = 0.0f;

    for (int t = 0; t < (N + 31) / 32; t++) {
        // 加载到共享内存
        if (row < N && t * 32 + threadIdx.x < N)
            sA[threadIdx.y][threadIdx.x] = A[row * N + t * 32 + threadIdx.x];
        else
            sA[threadIdx.y][threadIdx.x] = 0.0f;

        if (col < N && t * 32 + threadIdx.y < N)
            sB[threadIdx.y][threadIdx.x] = B[(t * 32 + threadIdx.y) * N + col];
        else
            sB[threadIdx.y][threadIdx.x] = 0.0f;

        __syncthreads();  // 确保数据加载完成

        // 计算
        for (int k = 0; k < 32; k++) {
            sum += sA[threadIdx.y][k] * sB[k][threadIdx.x];
        }

        __syncthreads();  // 确保计算完成后再加载下一块
    }

    if (row < N && col < N) {
        C[row * N + col] = sum;
    }
}
```

***

### 5. SIMT执行模型

#### 5.1 Warp的概念

```
一个Warp = 32个连续线程
同一Warp内的线程必须执行相同的指令（SIMT）

Warp 0: Thread 0-31  → 同时执行 ADD
Warp 1: Thread 32-63 → 同时执行 MUL
Warp 2: Thread 64-95 → 同时执行 LD

如果Warp内线程走不同分支：
┌──────────────────────────────────────────┐
│ if (threadIdx.x < 16)                    │
│     路径A: x = a + b;                    │
│ else                                     │
│     路径B: x = a * b;                    │
└──────────────────────────────────────────┘

执行过程：
时刻1: 线程0-15执行路径A，线程16-31空闲（掩码）
时刻2: 线程16-31执行路径B，线程0-15空闲（掩码）
→ 性能减半！
```

#### 5.2 分支分歧的影响

```cpp
// 坏例子：严重的分支分歧
__global__ void bad_branch(int* data, int n) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid < n) {
        if (data[tid] > 0) {
            data[tid] = data[tid] * 2;   // 路径A
        } else {
            data[tid] = data[tid] + 1;   // 路径B
        }
    }
}
// 如果一个warp内数据有正有负，两条路径都要执行

// 好例子：减少分支分歧
__global__ void good_branch(int* data, int n) {
    int tid = blockIdx.x * blockDim.x + threadIdx.x;
    if (tid < n) {
        // 用条件赋值替代分支
        int val = data[tid];
        data[tid] = (val > 0) ? (val * 2) : (val + 1);
    }
}
// 编译器可能将三元运算符转换为无分支指令（SEL）
```

***

### 6. CUDA与OpenCL的对比

| 维度 | CUDA | OpenCL |
|------|------|--------|
| 开发者 | NVIDIA | Khronos（行业标准） |
| 支持硬件 | 仅NVIDIA GPU | NVIDIA/AMD/Intel/CPU/FPGA |
| 语言 | CUDA C/C++（类C++扩展） | OpenCL C（类C子集） |
| 编译 | NVCC离线编译 | 运行时编译 |
| 生态 | 成熟，库丰富 | 较弱，库较少 |
| 调试工具 | Nsight/cuPrintf | 较少 |
| 性能 | 在NVIDIA上最优 | 跨平台但可能次优 |
| 学习曲线 | 较平缓 | 较陡峭 |
| 商业模式 | 闭源，NVIDIA独占 | 开源标准 |

```cpp
// CUDA核函数
__global__ void add(float* a, float* b, float* c, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) c[i] = a[i] + b[i];
}
// 启动：add<<<numBlocks, blockSize>>>(d_a, d_b, d_c, n);

// OpenCL等价核函数
__kernel void add(__global float* a, __global float* b, __global float* c, int n) {
    int i = get_global_id(0);
    if (i < n) c[i] = a[i] + b[i];
}
// 启动需要：创建kernel、设置参数、入队执行（更多步骤）
```

***

### 7. CUDA编程完整示例

```cpp
#include <cuda_runtime.h>
#include <stdio.h>
#include <stdlib.h>

// 核函数：向量加法
__global__ void vectorAdd(const float* a, const float* b, float* c, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;
    if (i < n) {
        c[i] = a[i] + b[i];
    }
}

// 核函数：归约求和
__global__ void reduceSum(float* data, float* result, int n) {
    __shared__ float sdata[256];

    int tid = threadIdx.x;
    int i = blockIdx.x * blockDim.x + threadIdx.x;

    // 加载数据到共享内存
    sdata[tid] = (i < n) ? data[i] : 0.0f;
    __syncthreads();

    // 树形归约
    for (int stride = blockDim.x / 2; stride > 0; stride >>= 1) {
        if (tid < stride) {
            sdata[tid] += sdata[tid + stride];
        }
        __syncthreads();
    }

    // Block的线程0写入结果
    if (tid == 0) {
        result[blockIdx.x] = sdata[0];
    }
}

int main() {
    int n = 1 << 20;  // 100万元素
    size_t size = n * sizeof(float);

    // 分配主机内存
    float *h_a = (float*)malloc(size);
    float *h_b = (float*)malloc(size);
    float *h_c = (float*)malloc(size);

    // 初始化数据
    for (int i = 0; i < n; i++) {
        h_a[i] = 1.0f;
        h_b[i] = 2.0f;
    }

    // 分配设备内存
    float *d_a, *d_b, *d_c;
    cudaMalloc(&d_a, size);
    cudaMalloc(&d_b, size);
    cudaMalloc(&d_c, size);

    // 拷贝数据到设备
    cudaMemcpy(d_a, h_a, size, cudaMemcpyHostToDevice);
    cudaMemcpy(d_b, h_b, size, cudaMemcpyHostToDevice);

    // 启动核函数
    int blockSize = 256;
    int numBlocks = (n + blockSize - 1) / blockSize;
    vectorAdd<<<numBlocks, blockSize>>>(d_a, d_b, d_c, n);

    // 拷贝结果回主机
    cudaMemcpy(h_c, d_c, size, cudaMemcpyDeviceToHost);

    // 验证结果
    float maxError = 0.0f;
    for (int i = 0; i < n; i++) {
        maxError = fmax(maxError, fabs(h_c[i] - 3.0f));
    }
    printf("最大误差: %f\n", maxError);

    // 释放内存
    cudaFree(d_a);
    cudaFree(d_b);
    cudaFree(d_c);
    free(h_a);
    free(h_b);
    free(h_c);

    return 0;
}
```

***

### 8. 常见误区

| 误区 | 事实 |
|------|------|
| CUDA只能在NVIDIA上用 | 是的，CUDA是NVIDIA专有技术 |
| 更多线程=更快 | 需要足够的占用率和内存带宽 |
| 共享内存总是比全局内存快 | 需要数据复用才值得 |
| CUDA C++ = 标准C++ | CUDA有扩展关键字和限制 |
| GPU计算没有精度问题 | FP16/TF32有精度损失 |

***

### 9. 总结

| 概念 | 核心要点 |
|------|---------|
| 线程层次 | Grid→Block→Thread，三级组织 |
| 内存层次 | 寄存器→共享内存→全局内存，三级存储 |
| SIMT | 32线程一个Warp，同指令不同数据 |
| 同步 | `__syncthreads()`同步Block内线程 |
| 优化目标 | 最大化占用率、最小化bank冲突、隐藏延迟 |

**核心记忆**：CUDA = Grid/Block/Thread线程组织 + 全局/共享/寄存器内存层次 + SIMT执行模型。理解这三层，就理解了CUDA编程的基础。