# 什么是WCET最坏执行时间
> 📖 相关章节：[嵌入式概述](../../12-嵌入式与实时系统/00-嵌入式系统概述.md)

> **WCET就是"最坏情况下这代码要跑多久"——不是平均时间，不是典型时间，而是绝对不可能超过的时间。** 在安全关键系统中，你必须证明刹车响应永远不会超过5毫秒，WCET分析就是给出这个保证的方法。

***

### 1. 核心要义

**WCET（Worst-Case Execution Time，最坏执行时间）** 是一段程序在特定硬件平台上可能花费的最长执行时间。它是实时系统的核心指标——不是"通常多久"，而是"绝对不会超过多久"，用于保证系统始终满足截止时间约束。

***

### 2. 生活类比

| 类比 | 说明 |
|------|------|
| 平均执行时间 = 平均通勤时间 | 通常30分钟，但偶尔2小时 |
| WCET = 合同保证的交付时间 | 无论什么情况，不超过5天 |
| WCET = 电梯载重上限 | 标注"最多1000kg"，不是"通常能载1000kg" |
| WCET = 消防通道宽度 | 必须保证最胖的人也能通过 |

**具体场景**：汽车ABS系统——刹车信号处理必须在2ms内完成。平均执行时间1ms不够安全，你需要证明WCET < 2ms，即无论输入如何、缓存状态如何，都不会超过2ms。

***

### 3. WCET分析方法

#### 3.1 静态分析 vs 测量法

| 方法 | 原理 | 优点 | 缺点 |
|------|------|------|------|
| 静态分析 | 从代码和硬件模型推导上界 | 安全（保证上界） | 可能严重高估 |
| 测量法 | 运行程序测量最大时间 | 接近真实值 | 可能遗漏最坏情况 |
| 混合方法 | 测量+静态分析结合 | 平衡精度和安全性 | 实现复杂 |

#### 3.2 静态分析流程

```
源代码
  │
  ↓
┌──────────────┐
│ 控制流分析    │  识别循环、分支、函数调用
└──────────────┘
  │
  ↓
┌──────────────┐
│ 循环界分析    │  确定循环最大迭代次数
└──────────────┘
  │
  ↓
┌──────────────┐
│ 处理器行为分析│  建模流水线、缓存、分支预测
└──────────────┘
  │
  ↓
┌──────────────┐
│ 路径分析      │  找到最长执行路径（IPET/ILP）
└──────────────┘
  │
  ↓
WCET上界
```

#### 3.3 静态分析示例

```cpp
// 需要分析WCET的函数
int process_sensor(int data[N], int threshold) {
    int count = 0;
    for (int i = 0; i < N; i++) {        // 循环：最多N次
        if (data[i] > threshold) {        // 分支
            count += complex_calc(data[i]);  // 较慢路径
        } else {
            count += simple_calc(data[i]);   // 较快路径
        }
    }
    return count;
}

// WCET分析：
// 1. 循环界：N次（N是常量或已知上界）
// 2. 每次迭代的最坏情况：走complex_calc路径
// 3. WCET = N × max(complex_calc时间, simple_calc时间)
//        = N × complex_calc时间

// 假设：
// - complex_calc: 最多500个时钟周期
// - simple_calc: 最多100个时钟周期
// - 循环开销: 10个周期
// - N = 100

// WCET = 100 × (10 + 500) = 51,000个周期
// 在100MHz CPU上 = 510μs
```

#### 3.4 循环界分析

```cpp
// 确定循环的最大迭代次数是WCET分析的关键

// 1. 固定界循环：容易分析
for (int i = 0; i < 100; i++) {  // 精确：100次
    // ...
}

// 2. 变量界循环：需要约束分析
void process(int n) {
    // 需要证明 n <= MAX_N
    for (int i = 0; i < n; i++) {
        // ...
    }
}

// 3. 嵌套循环
for (int i = 0; i < M; i++) {
    for (int j = 0; j < N; j++) {
        // WCET = M × N × 单次迭代时间
    }
}

// 4. 数据依赖循环
for (int i = 0; i < n; i++) {
    while (data[i] > 0) {  // 循环次数取决于数据！
        data[i] = data[i] / 2;
    }
}
// 需要分析data[i]的可能范围来确定while的最大次数
// 如果data[i]是int32_t，最多32次（每次除以2）

// 5. 递归：更难分析
int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n-1) + fibonacci(n-2);  // 指数级调用
}
// WCET分析需要限制递归深度
```

***

### 4. 测量法

#### 4.1 基本测量方法

```cpp
// 使用硬件定时器测量执行时间

// ARM Cortex-M：使用DWT（Data Watchpoint and Trace）
#define DWT_CYCCNT ((volatile uint32_t*)0xE0001004)
#define DWT_CONTROL ((volatile uint32_t*)0xE0001000)

void dwt_init(void) {
    *DWT_CONTROL |= 1;  // 使能CYCCNT计数器
}

uint32_t measure_execution_time(void (*func)(void)) {
    uint32_t start, end;

    start = *DWT_CYCCNT;
    func();
    end = *DWT_CYCCNT;

    return end - start;  // 时钟周期数
}

// 使用示例
void task_under_test(void) {
    process_sensor(data, threshold);
}

int main(void) {
    dwt_init();

    uint32_t max_cycles = 0;
    for (int trial = 0; trial < 10000; trial++) {
        generate_test_data(data);  // 生成不同的测试数据
        uint32_t cycles = measure_execution_time(task_under_test);
        if (cycles > max_cycles) {
            max_cycles = cycles;
        }
    }

    printf("测量到的最大执行时间: %u 周期 (%.1f μs)\n",
           max_cycles, (float)max_cycles / SystemCoreClock * 1e6);

    // 注意：测量值不是WCET！
    // 需要添加安全裕量：WCET估计 = 测量最大值 × 裕量系数
    float wcet_estimate = max_cycles * 1.2f;  // 20%裕量
}
```

#### 4.2 测量法的局限

```
测量法的问题：

1. 输入空间爆炸
   - 函数可能有无数种输入组合
   - 不可能测试所有情况
   - 最坏输入可能被遗漏

2. 硬件状态影响
   - 缓存状态不同，执行时间不同
   - 流水线状态不同，执行时间不同
   - 中断可能在测量时发生

3. 测量干扰
   - 测量本身的开销
   - 中断的影响
   - 动态电压频率调整（DVFS）

结论：测量值是WCET的下界，不是上界！
必须添加安全裕量或结合静态分析
```

***

### 5. WCET与调度分析

#### 5.1 利用率分析

```cpp
// 速率单调调度（Rate Monotonic Scheduling, RMS）

// 任务集合
struct Task {
    uint32_t period;    // 周期（ms）
    uint32_t wcet;      // 最坏执行时间（ms）
    uint32_t deadline;  // 截止时间（ms）
};

// 示例：三个实时任务
Task tasks[] = {
    {10,  2, 10},   // 任务1：周期10ms，WCET 2ms
    {20,  3, 20},   // 任务2：周期20ms，WCET 3ms
    {50, 10, 50},   // 任务3：周期50ms，WCET 10ms
};

// RMS可调度性条件：总利用率 ≤ n(2^(1/n) - 1)
float utilization = 0;
for (const auto& t : tasks) {
    utilization += (float)t.wcet / t.period;
}

// 3个任务的上限 = 3 × (2^(1/3) - 1) ≈ 0.780
float rms_bound = 3.0f * (powf(2.0f, 1.0f/3.0f) - 1.0f);

printf("总利用率: %.3f\n", utilization);        // 0.700
printf("RMS上限: %.3f\n", rms_bound);            // 0.780
printf("可调度: %s\n", utilization <= rms_bound ? "是" : "否");  // 是
```

#### 5.2 响应时间分析

```cpp
// 响应时间分析（Response Time Analysis, RTA）
// 计算每个任务的最坏响应时间

// 高优先级任务i对低优先级任务j的干扰：
// 干扰 = ceil(Rj / Ti) × Ci
// Rj = Cj + Σ ceil(Rj / Ti) × Ci

// 迭代计算响应时间
float compute_response_time(const Task* tasks, int n, int target) {
    float R = tasks[target].wcet;  // 初始值

    for (int iter = 0; iter < 100; iter++) {
        float R_new = tasks[target].wcet;

        // 加上所有高优先级任务的干扰
        for (int i = 0; i < target; i++) {
            float interference = ceilf(R / tasks[i].period) * tasks[i].wcet;
            R_new += interference;
        }

        if (R_new == R) {
            // 收敛
            return R_new;
        }
        R = R_new;
    }

    return -1;  // 不收敛，不可调度
}

// 验证所有任务是否满足截止时间
bool is_schedulable(const Task* tasks, int n) {
    for (int i = 0; i < n; i++) {
        float response_time = compute_response_time(tasks, n, i);
        if (response_time < 0 || response_time > tasks[i].deadline) {
            return false;  // 任务i不可调度
        }
    }
    return true;
}
```

***

### 6. 安全关键系统的时序保证

#### 6.1 DO-178C中的时序要求

```
航空软件标准DO-178C对时序的要求：

1. 必须证明所有实时任务满足截止时间
2. WCET分析必须考虑最坏情况
3. 必须考虑中断的影响
4. 必须考虑缓存和流水线的影响
5. 必须有可追溯性：从需求到分析到验证

认证等级：
A级：灾难性（飞机失控）→ 最严格
B级：危险（严重降低安全裕度）
C级：重大（显著增加工作量）
D级：轻微（轻微不便）
```

#### 6.2 ISO 26262中的时序要求

```
汽车功能安全标准ISO 26262：

ASIL等级：
ASIL-D：最高安全等级（转向、刹车）
ASIL-C：高安全等级
ASIL-B：中等安全等级
ASIL-A：最低安全等级

时序要求：
1. 必须定义所有实时任务的截止时间
2. 必须通过WCET分析证明满足截止时间
3. 必须考虑最坏情况下的系统负载
4. 必须验证中断延迟和任务切换时间
```

***

### 7. WCET分析工具

| 工具 | 类型 | 支持架构 | 特点 |
|------|------|---------|------|
| aiT (AbsInt) | 静态分析 | ARM/PPC/TriCore等 | 工业级，认证支持 |
| OTAWA | 静态分析 | ARM/PPC | 开源学术工具 |
| Heptane | 静态分析 | ARM/MIPS | 开源 |
| RapiTime | 混合 | 多架构 | 测量+分析 |
| Chronos | 静态分析 | ARM | 开源学术工具 |

***

### 8. 常见误区

| 误区 | 事实 |
|------|------|
| WCET = 平均执行时间 | WCET是最坏情况，远大于平均值 |
| 测量最大值就是WCET | 测量值是下界，可能遗漏最坏情况 |
| 现代CPU缓存让WCET不可分析 | 可以通过缓存分析技术建模 |
| WCET分析只用于航空航天 | 汽车、医疗、工业控制都需要 |
| WCET分析太保守没用 | 保守但安全，可以结合测量降低保守度 |

***

### 9. 总结

| 维度 | 核心要点 |
|------|---------|
| 本质 | 程序在最坏情况下的最长执行时间 |
| 静态分析 | 从代码推导上界，安全但可能高估 |
| 测量法 | 运行测量最大值，接近真实但不保证 |
| 调度分析 | 用WCET验证所有任务满足截止时间 |
| 安全关键 | DO-178C/ISO 26262要求WCET分析 |

**核心记忆**：WCET = 最坏情况执行时间，不是平均值。静态分析给出安全上界，测量法给出参考值。在安全关键系统中，WCET分析是证明"绝对不会超时"的数学依据。