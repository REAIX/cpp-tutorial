# 什么是GPU与CPU的区别
> 📖 相关章节：[图形编程概述](../../11-图形与GPU计算/00-图形编程概述.md)

> **CPU是将军，GPU是军队。** 将军擅长运筹帷幄处理复杂决策，军队擅长同时冲锋执行简单任务——理解这个本质差异，就理解了为什么GPU天生适合并行计算。

***

### 1. 本质速解

**GPU（Graphics Processing Unit）** 与 **CPU（Central Processing Unit）** 的根本区别在于设计哲学：**CPU追求单线程极致性能（低延迟），GPU追求大规模吞吐量（高带宽）**。CPU用少量强大的核心处理复杂逻辑，GPU用海量简单的核心同时处理大量相似任务。

***

### 2. 生活类比

| 类比维度 | CPU | GPU |
|---------|-----|-----|
| 交通工具 | 法拉利——速度快，但一次只能载2人 | 公交车——速度一般，但一次能载50人 |
| 厨房 | 一个米其林大厨——能做复杂菜式 | 100个帮厨——同时切100份土豆 |
| 军队 | 一个将军——制定战略决策 | 一支军队——同时执行冲锋命令 |
| 处理方式 | 串行精英——一个一个快速处理 | 并行洪流——一批一批同时处理 |

**具体场景**：渲染一帧1080p画面需要处理约200万个像素。如果用CPU逐个计算，就像一个厨师逐个做200万道菜；用GPU则是让几千个厨师同时做，每人只负责几百道。

***

### 3. 架构差异详解

#### 3.1 核心数量与复杂度

```
CPU 典型架构（以8核为例）：
┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐
│Core 0│ │Core 1│ │Core 2│ │Core 3│   每个核心：
│      │ │      │ │      │ │      │   - 超大缓存（L1/L2/L3）
│ 复杂 │ │ 复杂 │ │ 复杂 │ │ 复杂 │   - 分支预测器
│      │ │      │ │      │ │      │   - 乱序执行引擎
└──────┘ └──────┘ └──────┘ └──────┘   - 深流水线
┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐
│Core 4│ │Core 5│ │Core 6│ │Core 7│
└──────┘ └──────┘ └──────┘ └──────┘

GPU 典型架构（以NVIDIA为例）：
┌─────────────────────────────────────────┐
│              SM (流多处理器) × 数十个     │
│  ┌────┐┌────┐┌────┐┌────┐┌────┐┌────┐  │
│  │CUDA││CUDA││CUDA││CUDA││CUDA││CUDA│  │
│  │Core││Core││Core││Core││Core││Core│  │  每个核心：
│  └────┘└────┘└────┘└────┘└────┘└────┘  │  - 极简设计
│  ┌────┐┌────┐┌────┐┌────┐┌────┐┌────┐  │  - 共享缓存
│  │CUDA││CUDA││CUDA││CUDA││CUDA││CUDA│  │  - SIMT执行
│  │Core││Core││Core││Core││Core││Core│  │  - 无分支预测
│  └────┘└────┘└────┘└────┘└────┘└────┘  │
│          共享内存 / L1 缓存              │
└─────────────────────────────────────────┘
```

#### 3.2 晶体管分配对比

| 组件 | CPU占比 | GPU占比 | 说明 |
|------|--------|--------|------|
| ALU（计算单元） | ~20% | ~80% | GPU把大部分晶体管用于计算 |
| 缓存 | ~30% | ~5% | CPU用大缓存降低延迟 |
| 控制逻辑 | ~30% | ~5% | CPU需要复杂的分支预测和乱序执行 |
| 其他 | ~20% | ~10% | CPU的内存控制器、I/O等 |

#### 3.3 内存子系统

```cpp
// CPU内存访问模式——追求低延迟
// 典型延迟：L1 ~1ns, L2 ~4ns, L3 ~12ns, DDR ~80ns
void cpu_matrix_add(const float* a, const float* b, float* c, int n) {
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            c[i * n + j] = a[i * n + j] + b[i * n + j];  // 逐个计算
        }
    }
}
// 1024×1024 矩阵加法：~100万次迭代，串行执行

// GPU内存访问模式——追求高带宽
// 典型带宽：HBM2e ~2TB/s vs DDR5 ~60GB/s
__global__ void gpu_matrix_add(const float* a, const float* b, float* c, int n) {
    int i = blockIdx.x * blockDim.x + threadIdx.x;  // 每个线程处理一个元素
    int j = blockIdx.y * blockDim.y + threadIdx.y;
    if (i < n && j < n) {
        c[i * n + j] = a[i * n + j] + b[i * n + j];  // 数千线程同时计算
    }
}
// 1024×1024 矩阵加法：~100万个线程并行执行
```

***

### 4. 设计目标差异

| 维度 | CPU | GPU |
|------|-----|-----|
| **首要目标** | 最小化单任务延迟 | 最大化总吞吐量 |
| **核心策略** | 深流水线 + 乱序执行 + 大缓存 | 大量简单核心 + SIMT |
| **分支处理** | 复杂分支预测器，预测准确率>95% | 所有线程必须走同一分支，分歧导致串行化 |
| **缓存设计** | 大容量多级缓存，减少内存访问 | 小缓存，依赖高带宽隐藏延迟 |
| **时钟频率** | 4-6 GHz | 1.5-2.5 GHz |
| **核心数量** | 4-64核 | 数千到数万核 |
| **内存带宽** | ~50-100 GB/s | ~500-2000 GB/s |

#### 为什么CPU不能替代GPU？

```cpp
// CPU的困境：分支密集型任务
int compute(int x) {
    if (x > 100) return x * x;        // 分支1
    else if (x > 50) return x + 10;   // 分支2
    else if (x > 0) return x - 5;     // 分支3
    else return 0;                     // 分支4
}
// CPU：分支预测器高效处理，每个分支只需1-2个周期
// GPU：一个warp内32个线程如果走不同分支，需要串行执行所有分支路径
```

#### 为什么GPU不能替代CPU？

```cpp
// GPU的困境：复杂控制流
void complex_logic(Database& db, Query& q) {
    // 递归查询
    auto result = db.query(q);
    if (result.empty()) {
        // 动态调整策略
        q.rewrite();
        result = db.query(q);
        if (result.size() > 1000) {
            // 分页处理
            for (int page = 0; page < result.pages(); page++) {
                process_page(result[page]);
            }
        }
    }
}
// CPU：擅长这种复杂的、不可预测的控制流
// GPU：分支分歧严重，大量核心空闲等待
```

***

### 5. 适用场景对比

| 场景 | CPU更优 | GPU更优 |
|------|--------|--------|
| 操作系统调度 | ✅ | ❌ |
| 数据库查询 | ✅ | ❌ |
| Web服务器 | ✅ | ❌ |
| 编译器 | ✅ | ❌ |
| 图形渲染 | ❌ | ✅ |
| 矩阵运算 | ❌ | ✅ |
| 深度学习训练 | ❌ | ✅ |
| 密码学暴力破解 | ❌ | ✅ |
| 视频编码 | ❌ | ✅ |
| 科学模拟 | ❌ | ✅ |

#### 判断标准：数据并行度

```cpp
// 低并行度——适合CPU
// 每次处理一个元素，逻辑复杂
void process_order(Order& order) {
    validate(order);
    calculate_price(order);
    apply_discount(order);
    update_inventory(order);
    send_confirmation(order);
}

// 高并行度——适合GPU
// 处理百万元素，每个元素逻辑相同
__global__ void apply_filter(float* image, int width, int height) {
    int x = blockIdx.x * blockDim.x + threadIdx.x;
    int y = blockIdx.y * blockDim.y + threadIdx.y;
    if (x < width && y < height) {
        // 对每个像素执行相同的3×3卷积
        float sum = 0.0f;
        for (int dy = -1; dy <= 1; dy++) {
            for (int dx = -1; dx <= 1; dx++) {
                sum += image[(y+dy) * width + (x+dx)];
            }
        }
        image[y * width + x] = sum / 9.0f;
    }
}
```

***

### 6. 为什么GPU适合并行计算

#### 6.1 SIMT执行模型

SIMT（Single Instruction, Multiple Threads）是GPU的核心执行模型：

```
一个Warp（32个线程）的执行过程：

时刻1: 所有32个线程执行 ADD 指令
时刻2: 所有32个线程执行 MUL 指令
时刻3: 所有32个线程执行 LD 指令（从内存加载数据）

如果遇到分支：
┌──────────────────────────────────────────┐
│ if (threadIdx.x < 16)                    │
│     路径A: result = a + b;  // 线程0-15  │
│ else                                     │
│     路径B: result = a * b;  // 线程16-31  │
└──────────────────────────────────────────┘

实际执行：
时刻3: 执行路径A（线程0-15工作，线程16-31掩码空闲）
时刻4: 执行路径B（线程16-31工作，线程0-15掩码空闲）
→ 分支分歧导致性能减半！
```

#### 6.2 延迟隐藏

```cpp
// CPU策略：通过缓存减少延迟
// 如果数据在L1缓存，1ns就能拿到
// 命中率目标：>95%

// GPU策略：通过并发线程隐藏延迟
// 当一个warp等待内存时，切换到另一个warp执行
// 不需要数据在缓存中，只需要有足够多的warp可以切换

// 示例：GPU延迟隐藏
__global__ void latency_hiding(float* data, float* result, int n) {
    int idx = blockIdx.x * blockDim.x + threadIdx.x;
    if (idx < n) {
        float val = data[idx];       // 这个内存访问可能需要数百个周期
        result[idx] = val * 2.0f;    // 但其他warp可以在等待期间执行
    }
}
// 当warp 0等待数据时，SM切换到warp 1执行
// 当warp 1也等待时，切换到warp 2...
// 只要warp数量足够，内存延迟就被完全隐藏
```

#### 6.3 计算与通信比

```
衡量GPU效率的关键指标：计算与通信比（Arithmetic Intensity）

计算与通信比 = FLOPS / 字节访问量

高计算密度（适合GPU）：
- 矩阵乘法：O(n³) 计算量 / O(n²) 数据量 → 比值 = O(n)
- 卷积运算：O(k²×n²) 计算量 / O(n²) 数据量 → 比值 = O(k²)

低计算密度（不适合GPU）：
- 向量加法：O(n) 计算量 / O(n) 数据量 → 比值 = O(1)
- 内存拷贝：0 计算量 / O(n) 数据量 → 比值 = 0
```

***

### 7. GPU编程模型演进

| 时代 | API/框架 | 特点 | 编程难度 |
|------|---------|------|---------|
| 固定管线时代 | OpenGL 1.x / DirectX 7 | 硬件固定功能，不可编程 | 低 |
| 可编程管线 | OpenGL 2.x / DirectX 9 | 顶点/片段着色器可编程 | 中 |
| 通用计算 | CUDA / OpenCL | GPU用于非图形计算 | 中高 |
| 现代图形 | Vulkan / DirectX 12 | 显式控制，低开销 | 高 |
| AI加速 | Tensor Core / CUDA | 混合精度矩阵运算 | 中 |

```cpp
// CUDA编程的基本模式
#include <cuda_runtime.h>

// 第一步：在GPU上分配内存
float* d_data;
cudaMalloc(&d_data, size * sizeof(float));

// 第二步：将数据从CPU拷贝到GPU
cudaMemcpy(d_data, h_data, size * sizeof(float), cudaMemcpyHostToDevice);

// 第三步：启动GPU核函数
int blockSize = 256;
int numBlocks = (size + blockSize - 1) / blockSize;
my_kernel<<<numBlocks, blockSize>>>(d_data, size);

// 第四步：将结果从GPU拷贝回CPU
cudaMemcpy(h_result, d_data, size * sizeof(float), cudaMemcpyDeviceToHost);

// 第五步：释放GPU内存
cudaFree(d_data);
```

***

### 8. 性能对比实例

```
矩阵乘法 4096×4096（单精度浮点）：

| 实现方式 | 执行时间 | GFLOPS | 加速比 |
|---------|---------|--------|-------|
| CPU单线程（朴素实现） | ~45秒 | ~1.5 | 1× |
| CPU OpenMP 8线程 | ~7秒 | ~10 | 6.4× |
| GPU CUDA（朴素实现） | ~0.8秒 | ~85 | 56× |
| GPU CUDA（优化后） | ~0.05秒 | ~1400 | 900× |
| GPU cuBLAS（库函数） | ~0.03秒 | ~2300 | 1500× |

注意：实际加速比取决于问题规模和优化程度
```

***

### 9. 常见误区

| 误区 | 事实 |
|------|------|
| GPU核心比CPU核心强 | GPU单个核心远弱于CPU核心，优势在数量 |
| GPU可以替代CPU | 两者互补，不能互相替代 |
| 更多核心=更快 | 如果数据并行度不够，更多核心只会空闲 |
| GPU编程很简单 | GPU编程的难点在内存管理和优化 |
| GPU只用于图形 | 现在GPU广泛用于AI、科学计算、金融等 |

***

### 10. 总结

| 维度 | CPU | GPU |
|------|-----|-----|
| 设计哲学 | 低延迟 | 高吞吐 |
| 核心数量 | 少而精（4-64） | 多而简（数千-数万） |
| 适用任务 | 复杂控制流、串行逻辑 | 大规模数据并行 |
| 编程模型 | 顺序执行 | SIMT并行执行 |
| 内存重点 | 低延迟缓存 | 高带宽显存 |
| 选择原则 | 需要低延迟响应 | 需要高吞吐量计算 |

**核心记忆**：CPU像法拉利——快但座位少；GPU像公交车——单次慢但总运力大。选择谁取决于你要运人还是赛车。