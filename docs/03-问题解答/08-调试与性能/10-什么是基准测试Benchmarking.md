# 什么是基准测试Benchmarking
> 📖 相关章节：[调试技巧](../../04-工程实践/07-调试技巧.md)、[性能优化](../../04-工程实践/09-性能优化.md)

> "过早优化是万恶之源，但没有测量的优化是盲目的。" —— Tony Hoare 改编

***

### 1. 一句话概括

基准测试（Benchmarking）是通过受控实验精确测量代码片段的执行性能（吞吐量、延迟、内存开销等），以数据驱动的方式发现性能瓶颈、验证优化效果，避免凭直觉做性能决策。

***

### 2. 微基准测试 vs 宏基准测试

| 维度 | 微基准测试（Micro） | 宏基准测试（Macro） |
|------|-------------------|-------------------|
| **范围** | 单个函数/代码片段 | 完整系统/子系统 |
| **隔离性** | 高，排除外部干扰 | 低，包含真实环境 |
| **可重复性** | 高 | 低，受系统状态影响 |
| **定位精度** | 精确到函数/循环 | 只能定位到模块 |
| **典型场景** | 算法对比、数据结构选型 | 端到端延迟、QPS 测试 |
| **工具** | Google Benchmark、Catch2 | JMeter、wrk、自定义框架 |
| **运行时间** | 秒级 | 分钟到小时 |

```cpp
#include <vector>
#include <algorithm>

int sum_vector(const std::vector<int> &v) {
    int s = 0;
    for (int x : v) s += x;
    return s;
}

int sum_sorted_vector(std::vector<int> v) {
    std::sort(v.begin(), v.end());
    int s = 0;
    for (int x : v) s += x;
    return s;
}
```

微基准测试关注 `sum_vector` 与 `sum_sorted_vector` 的单次执行耗时差异；宏基准测试则关注整个数据处理流水线的端到端吞吐量。

***

### 3. Google Benchmark 框架详解

Google Benchmark 是 C++ 生态中最主流的微基准测试框架，提供统计采样、结果聚合、防止编译器优化等关键能力。

**安装与集成**：

```bash
# vcpkg
vcpkg install benchmark

# CMake
find_package(benchmark REQUIRED)
target_link_libraries(my_bench benchmark::benchmark)
```

**基础用法**：

```cpp
#include <benchmark/benchmark.h>
#include <vector>
#include <algorithm>
#include <random>

static void BM_VectorSort(benchmark::State &state) {
    size_t n = state.range(0);
    std::vector<int> data(n);

    std::mt19937 rng(42);
    for (auto &x : data) x = rng();

    for (auto _ : state) {
        std::vector<int> copy = data;
        std::sort(copy.begin(), copy.end());
        benchmark::DoNotOptimize(copy.data());
    }
    state.SetComplexityN(n);
}
BENCHMARK(BM_VectorSort)->Range(1 << 6, 1 << 16)->Complexity();

static void BM_VectorPushBack(benchmark::State &state) {
    for (auto _ : state) {
        std::vector<int> v;
        v.reserve(state.range(0));
        for (int i = 0; i < state.range(0); i++) {
            v.push_back(i);
        }
        benchmark::DoNotOptimize(v.data());
    }
}
BENCHMARK(BM_VectorPushBack)->Arg(1000)->Arg(10000)->Arg(100000);

BENCHMARK_MAIN();
```

**编译与运行**：

```bash
g++ -O2 -std=c++17 bench.cpp -lbenchmark -lpthread -o bench

# 运行
./bench

# 输出 JSON 格式
./bench --benchmark_format=json --benchmark_out=result.json

# 过滤特定测试
./bench --benchmark_filter=BM_VectorSort

# 控制采样
./bench --benchmark_min_time=5.0 --benchmark_repetitions=10
```

| Google Benchmark API | 说明 |
|---------------------|------|
| `BENCHMARK(func)` | 注册基准测试函数 |
| `BENCHMARK_MAIN()` | 生成 main 函数 |
| `state.range(0)` | 获取参数化输入 |
| `state.SetItemsProcessed(n)` | 设置处理元素数（计算吞吐量） |
| `state.SetBytesProcessed(n)` | 设置处理字节数 |
| `state.PauseTiming()` | 暂停计时 |
| `state.ResumeTiming()` | 恢复计时 |
| `state.SetComplexityN(n)` | 设置复杂度参数 |
| `->Arg(n)` | 传入单个参数 |
| `->Range(lo, hi)` | 传入 2 的幂次范围 |
| `->DenseRange(lo, hi, step)` | 传入密集范围 |

***

### 4. DoNotOptimize 与 ClobberMemory

编译器优化是基准测试的天敌。Google Benchmark 提供两个关键工具防止优化消除被测代码。

**DoNotOptimize 原理**：

```cpp
namespace benchmark {
namespace internal {

template <class Tp>
inline BENCHMARK_ALWAYS_INLINE void DoNotOptimize(Tp const &value) {
    asm volatile("" : : "r,m"(value) : "memory");
}

inline BENCHMARK_ALWAYS_INLINE void ClobberMemory() {
    asm volatile("" : : : "memory");
}

}
}
```

- **DoNotOptimize(x)**：告诉编译器 `x` 被"使用"了，不能消除对 `x` 的计算
- **ClobberMemory()**：告诉编译器内存被"修改"了，不能缓存内存读取

**实战示例**：

```cpp
#include <benchmark/benchmark.h>
#include <vector>
#include <numeric>

static void BM_Accumulate_WithOptimize(benchmark::State &state) {
    std::vector<int> data(state.range(0), 42);

    for (auto _ : state) {
        int result = std::accumulate(data.begin(), data.end(), 0);
        benchmark::DoNotOptimize(result);
    }
}
BENCHMARK(BM_Accumulate_WithOptimize)->Arg(10000);

static void BM_Accumulate_WithoutOptimize(benchmark::State &state) {
    std::vector<int> data(state.range(0), 42);

    for (auto _ : state) {
        int result = std::accumulate(data.begin(), data.end(), 0);
    }
}
BENCHMARK(BM_Accumulate_WithoutOptimize)->Arg(10000);

static void BM_WritePattern(benchmark::State &state) {
    std::vector<int> data(state.range(0));

    for (auto _ : state) {
        for (auto &x : data) x = 0;
        benchmark::ClobberMemory();
    }
}
BENCHMARK(BM_WritePattern)->Arg(10000);

BENCHMARK_MAIN();
```

| 场景 | 应使用 |
|------|-------|
| 防止计算结果被消除 | `DoNotOptimize(result)` |
| 防止写入被消除 | `DoNotOptimize(ptr)` + `ClobberMemory()` |
| 防止读取被缓存 | `ClobberMemory()` |
| 防止整个循环被删除 | `DoNotOptimize` + `ClobberMemory` 组合 |

> ⚠️ **平台注意**：`DoNotOptimize` 使用内联汇编，MSVC 上实现方式不同。Google Benchmark 已做兼容处理，但建议在 GCC/Clang 上进行正式基准测试。

***

### 5. 计时方案：chrono / rdtsc / steady_clock

| 计时方式 | 精度 | 开销 | 可移植性 | 适用场景 |
|---------|------|------|---------|---------|
| `std::chrono::steady_clock` | 纳秒级 | 低 | 完全可移植 | 通用基准测试 |
| `std::chrono::high_resolution_clock` | 纳秒级 | 低 | 平台相关 | 不推荐（可能是 system_clock 别名） |
| `rdtsc`（x86 时间戳计数器） | CPU 周期级 | 极低 | 仅 x86/x86_64 | 超精细测量 |
| `clock_gettime(CLOCK_MONOTONIC)` | 纳秒级 | 低 | POSIX | Linux 基准测试 |
| `QueryPerformanceCounter` | 微秒-纳秒 | 低 | 仅 Windows | Windows 基准测试 |

**chrono 方案**：

```cpp
#include <chrono>
#include <iostream>
#include <vector>
#include <algorithm>

class Timer {
    using clock = std::chrono::steady_clock;
    clock::time_point start_;

public:
    Timer() : start_(clock::now()) {}

    double elapsed_ns() const {
        auto end = clock::now();
        return std::chrono::duration<double, std::nano>(end - start_).count();
    }

    double elapsed_us() const {
        return elapsed_ns() / 1000.0;
    }

    double elapsed_ms() const {
        return elapsed_ns() / 1000000.0;
    }
};

int main() {
    std::vector<int> data(1000000);
    for (auto &x : data) x = rand();

    Timer t;
    std::sort(data.begin(), data.end());
    std::cout << "sort elapsed: " << t.elapsed_ms() << " ms\n";

    return 0;
}
```

**rdtsc 方案**（x86 专用）：

```cpp
#include <cstdint>
#include <intrin.h>

inline uint64_t rdtsc_start() {
    unsigned int aux;
    return __rdtscp(&aux);
}

inline uint64_t rdtsc_end() {
    unsigned int aux;
    return __rdtscp(&aux);
}

double cycles_to_ns(uint64_t cycles, double ghz) {
    return static_cast<double>(cycles) / ghz;
}
```

> ⚠️ **平台注意**：`__rdtscp` 在 MSVC 中声明于 `<intrin.h>`，在 GCC/Clang 中声明于 `<x86intrin.h>`。ARM 平台需使用 `cntvct_el0` 寄存器。rdtsc 在频率缩放（SpeedStep/Turbo Boost）环境下可能不准确，建议固定 CPU 频率后使用。

***

### 6. 统计方法与结果分析

单次运行结果不可靠，必须使用统计方法消除噪声。

**关键统计指标**：

| 指标 | 含义 | 用途 |
|------|------|------|
| 均值（Mean） | 所有样本平均值 | 整体趋势 |
| 中位数（Median） | 排序后中间值 | 抗异常值 |
| 标准差（StdDev） | 离散程度 | 结果稳定性 |
| 置信区间（CI） | 真实均值的估计范围 | 结果可信度 |
| 百分位数（P99/P999） | 99%/99.9% 分位值 | 尾延迟评估 |

**Google Benchmark 内置统计**：

```bash
# 输出示例
----------------------------------------------------------
Benchmark                Time             CPU   Iterations
----------------------------------------------------------
BM_VectorSort/64      289 ns          289 ns      2411724
BM_VectorSort/128     612 ns          612 ns      1128205
BM_VectorSort/256    1345 ns         1345 ns       518731
```

**自定义统计分析**：

```cpp
#include <vector>
#include <algorithm>
#include <numeric>
#include <cmath>
#include <iostream>
#include <chrono>

struct BenchResult {
    double mean;
    double median;
    double stddev;
    double p99;
    double min_val;
    double max_val;
};

BenchResult analyze(std::vector<double> samples) {
    std::sort(samples.begin(), samples.end());

    BenchResult r;
    r.mean = std::accumulate(samples.begin(), samples.end(), 0.0) / samples.size();
    r.median = samples[samples.size() / 2];
    r.min_val = samples.front();
    r.max_val = samples.back();
    r.p99 = samples[static_cast<size_t>(samples.size() * 0.99)];

    double sum_sq = 0;
    for (double s : samples) {
        sum_sq += (s - r.mean) * (s - r.mean);
    }
    r.stddev = std::sqrt(sum_sq / samples.size());

    return r;
}

void run_benchmark(const char *name, auto func, int iterations = 100) {
    std::vector<double> times;
    times.reserve(iterations);

    for (int i = 0; i < iterations; i++) {
        auto start = std::chrono::steady_clock::now();
        func();
        auto end = std::chrono::steady_clock::now();
        double ns = std::chrono::duration<double, std::nano>(end - start).count();
        times.push_back(ns);
    }

    auto r = analyze(times);
    std::cout << name << ":\n"
              << "  mean=" << r.mean << "ns median=" << r.median << "ns\n"
              << "  stddev=" << r.stddev << "ns p99=" << r.p99 << "ns\n"
              << "  min=" << r.min_val << "ns max=" << r.max_val << "ns\n";
}
```

***

### 7. 常见陷阱：缓存预热与分支预测

基准测试最大的敌人不是计时精度，而是**微架构状态污染**。

**陷阱 1：缓存预热效应**

```cpp
#include <benchmark/benchmark.h>
#include <vector>
#include <numeric>

static void BM_ColdCache(benchmark::State &state) {
    std::vector<int> data(state.range(0), 42);
    const int stride = 64 / sizeof(int);

    for (auto _ : state) {
        state.PauseTiming();
        std::vector<int> fresh_data = data;
        benchmark::ClobberMemory();
        state.ResumeTiming();

        long long sum = 0;
        for (size_t i = 0; i < fresh_data.size(); i += stride) {
            sum += fresh_data[i];
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_ColdCache)->Arg(1000000);

static void BM_WarmCache(benchmark::State &state) {
    std::vector<int> data(state.range(0), 42);
    const int stride = 64 / sizeof(int);

    long long warmup = 0;
    for (size_t i = 0; i < data.size(); i += stride) warmup += data[i];
    benchmark::DoNotOptimize(warmup);

    for (auto _ : state) {
        long long sum = 0;
        for (size_t i = 0; i < data.size(); i += stride) {
            sum += data[i];
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_WarmCache)->Arg(1000000);

BENCHMARK_MAIN();
```

**陷阱 2：分支预测影响**

```cpp
#include <benchmark/benchmark.h>
#include <vector>
#include <algorithm>
#include <random>

static void BM_BranchPredictable(benchmark::State &state) {
    std::vector<int> data(state.range(0));
    for (size_t i = 0; i < data.size(); i++) data[i] = i % 2;

    for (auto _ : state) {
        int count = 0;
        for (int x : data) {
            if (x) count++;
        }
        benchmark::DoNotOptimize(count);
    }
}
BENCHMARK(BM_BranchPredictable)->Arg(1000000);

static void BM_BranchUnpredictable(benchmark::State &state) {
    std::vector<int> data(state.range(0));
    std::mt19937 rng(42);
    for (auto &x : data) x = rng() & 1;

    for (auto _ : state) {
        int count = 0;
        for (int x : data) {
            if (x) count++;
        }
        benchmark::DoNotOptimize(count);
    }
}
BENCHMARK(BM_BranchUnpredictable)->Arg(1000000);

BENCHMARK_MAIN();
```

| 陷阱 | 表现 | 对策 |
|------|------|------|
| 缓存预热 | 首次运行远快于后续 | 区分冷/热缓存场景分别测试 |
| 分支预测 | 有规律数据远快于随机数据 | 使用随机数据测试，或用位运算替代分支 |
| 编译器内联 | 小函数被内联后测量不准 | 使用 `__attribute__((noinline))` |
| 死代码消除 | 计算结果未使用被删除 | 使用 `DoNotOptimize` |
| 常量折叠 | 编译期已知值被预计算 | 运行时生成输入数据 |
| 频率缩放 | CPU 动态调频影响结果 | 固定 CPU 频率（Linux: `cpupower frequency-set`） |

***

### 8. 内存操作基准测试

内存操作是性能优化的核心战场，理解不同内存访问模式的性能差异至关重要。

**内存层次延迟参考**：

| 层级 | 延迟 | 大小 |
|------|------|------|
| L1 Cache | ~1 ns | 32-64 KB |
| L2 Cache | ~4 ns | 256 KB - 1 MB |
| L3 Cache | ~10-30 ns | 4-64 MB |
| 主存 DDR4 | ~60-100 ns | GB 级 |
| NVMe SSD | ~10,000 ns | TB 级 |

**顺序 vs 随机访问**：

```cpp
#include <benchmark/benchmark.h>
#include <vector>
#include <random>

static void BM_SequentialAccess(benchmark::State &state) {
    std::vector<int> data(state.range(0), 1);

    for (auto _ : state) {
        long long sum = 0;
        for (int x : data) {
            sum += x;
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetBytesProcessed(state.iterations() * state.range(0) * sizeof(int));
}
BENCHMARK(BM_SequentialAccess)->Arg(1 << 20);

static void BM_RandomAccess(benchmark::State &state) {
    size_t n = state.range(0);
    std::vector<int> data(n, 1);
    std::vector<size_t> indices(n);

    std::mt19937 rng(42);
    for (auto &idx : indices) idx = rng() % n;

    for (auto _ : state) {
        long long sum = 0;
        for (size_t idx : indices) {
            sum += data[idx];
        }
        benchmark::DoNotOptimize(sum);
    }
    state.SetBytesProcessed(state.iterations() * n * sizeof(int));
}
BENCHMARK(BM_RandomAccess)->Arg(1 << 20);

BENCHMARK_MAIN();
```

**malloc vs 内存池**：

```cpp
#include <benchmark/benchmark.h>
#include <vector>
#include <cstdlib>

struct Object {
    int a, b, c, d;
    double e, f;
};

static void BM_MallocFree(benchmark::State &state) {
    for (auto _ : state) {
        Object *obj = static_cast<Object *>(malloc(sizeof(Object)));
        benchmark::DoNotOptimize(obj);
        free(obj);
    }
}
BENCHMARK(BM_MallocFree);

static void BM_NewDelete(benchmark::State &state) {
    for (auto _ : state) {
        Object *obj = new Object();
        benchmark::DoNotOptimize(obj);
        delete obj;
    }
}
BENCHMARK(BM_NewDelete);

class ObjectPool {
    static constexpr size_t POOL_SIZE = 1024;
    Object pool_[POOL_SIZE];
    bool used_[POOL_SIZE] = {};

public:
    Object *alloc() {
        for (size_t i = 0; i < POOL_SIZE; i++) {
            if (!used_[i]) {
                used_[i] = true;
                return &pool_[i];
            }
        }
        return nullptr;
    }

    void dealloc(Object *obj) {
        size_t idx = obj - pool_;
        if (idx < POOL_SIZE) used_[idx] = false;
    }
};

static void BM_ObjectPool(benchmark::State &state) {
    ObjectPool pool;

    for (auto _ : state) {
        Object *obj = pool.alloc();
        benchmark::DoNotOptimize(obj);
        pool.dealloc(obj);
    }
}
BENCHMARK(BM_ObjectPool);

BENCHMARK_MAIN();
```

***

### 9. 多线程基准测试

多线程基准测试需要特别关注线程创建开销、锁竞争和伪共享等问题。

```cpp
#include <benchmark/benchmark.h>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <numeric>

static void BM_SingleThreadSum(benchmark::State &state) {
    std::vector<int> data(state.range(0), 1);

    for (auto _ : state) {
        long long sum = std::accumulate(data.begin(), data.end(), 0LL);
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_SingleThreadSum)->Arg(1 << 20);

static void BM_MutexSum(benchmark::State &state) {
    std::vector<int> data(state.range(0), 1);
    int num_threads = state.range(1);

    for (auto _ : state) {
        long long sum = 0;
        std::mutex mtx;

        std::vector<std::thread> threads;
        size_t chunk = data.size() / num_threads;

        for (int t = 0; t < num_threads; t++) {
            threads.emplace_back([&, t]() {
                size_t start = t * chunk;
                size_t end = (t == num_threads - 1) ? data.size() : start + chunk;
                long long local = 0;
                for (size_t i = start; i < end; i++) local += data[i];
                std::lock_guard<std::mutex> lock(mtx);
                sum += local;
            });
        }

        for (auto &th : threads) th.join();
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_MutexSum)->Args({1 << 20, 2})->Args({1 << 20, 4})->Args({1 << 20, 8});

static void BM_AtomicSum(benchmark::State &state) {
    std::vector<int> data(state.range(0), 1);
    int num_threads = state.range(1);

    for (auto _ : state) {
        std::atomic<long long> sum{0};

        std::vector<std::thread> threads;
        size_t chunk = data.size() / num_threads;

        for (int t = 0; t < num_threads; t++) {
            threads.emplace_back([&, t]() {
                size_t start = t * chunk;
                size_t end = (t == num_threads - 1) ? data.size() : start + chunk;
                for (size_t i = start; i < end; i++) {
                    sum.fetch_add(data[i], std::memory_order_relaxed);
                }
            });
        }

        for (auto &th : threads) th.join();
        benchmark::DoNotOptimize(sum.load());
    }
}
BENCHMARK(BM_AtomicSum)->Args({1 << 20, 2})->Args({1 << 20, 4})->Args({1 << 20, 8});

static void BM_LocalSum(benchmark::State &state) {
    std::vector<int> data(state.range(0), 1);
    int num_threads = state.range(1);

    for (auto _ : state) {
        std::vector<long long> local_sums(num_threads, 0);

        std::vector<std::thread> threads;
        size_t chunk = data.size() / num_threads;

        for (int t = 0; t < num_threads; t++) {
            threads.emplace_back([&, t]() {
                size_t start = t * chunk;
                size_t end = (t == num_threads - 1) ? data.size() : start + chunk;
                for (size_t i = start; i < end; i++) {
                    local_sums[t] += data[i];
                }
            });
        }

        for (auto &th : threads) th.join();
        long long sum = std::accumulate(local_sums.begin(), local_sums.end(), 0LL);
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_LocalSum)->Args({1 << 20, 2})->Args({1 << 20, 4})->Args({1 << 20, 8});

BENCHMARK_MAIN();
```

**多线程基准测试注意事项**：

| 问题 | 说明 | 对策 |
|------|------|------|
| 伪共享 | 不同线程修改同一缓存行 | 对齐到缓存行（alignas(64)） |
| 锁竞争 | 多线程争抢同一把锁 | 减小锁粒度或用无锁结构 |
| 线程创建开销 | 每次迭代创建线程 | 使用线程池 |
| 负载不均 | 线程间工作量差异大 | 动态任务分配 |
| NUMA 效应 | 跨 NUMA 节点访问内存 | 绑定 NUMA 节点 |

> ⚠️ **平台注意**：Linux 上可用 `taskset` 或 `numactl` 绑定 CPU 亲和性。Windows 上使用 `SetThreadAffinityMask`。多线程基准测试结果在不同操作系统上差异较大。

***

### 10. 实战：对比不同容器操作性能

```cpp
#include <benchmark/benchmark.h>
#include <vector>
#include <list>
#include <set>
#include <unordered_set>
#include <algorithm>
#include <random>

static void BM_VectorInsert(benchmark::State &state) {
    for (auto _ : state) {
        std::vector<int> v;
        for (int i = 0; i < state.range(0); i++) v.push_back(i);
        benchmark::DoNotOptimize(v.data());
    }
}
BENCHMARK(BM_VectorInsert)->Arg(10000);

static void BM_ListInsert(benchmark::State &state) {
    for (auto _ : state) {
        std::list<int> l;
        for (int i = 0; i < state.range(0); i++) l.push_back(i);
        benchmark::DoNotOptimize(&l);
    }
}
BENCHMARK(BM_ListInsert)->Arg(10000);

static void BM_SetInsert(benchmark::State &state) {
    for (auto _ : state) {
        std::set<int> s;
        for (int i = 0; i < state.range(0); i++) s.insert(i);
        benchmark::DoNotOptimize(&s);
    }
}
BENCHMARK(BM_SetInsert)->Arg(10000);

static void BM_UnorderedSetInsert(benchmark::State &state) {
    for (auto _ : state) {
        std::unordered_set<int> s;
        s.reserve(state.range(0));
        for (int i = 0; i < state.range(0); i++) s.insert(i);
        benchmark::DoNotOptimize(&s);
    }
}
BENCHMARK(BM_UnorderedSetInsert)->Arg(10000);

static void BM_VectorFind(benchmark::State &state) {
    std::vector<int> v(state.range(0));
    for (int i = 0; i < state.range(0); i++) v[i] = i;

    for (auto _ : state) {
        bool found = std::binary_search(v.begin(), v.end(), state.range(0) / 2);
        benchmark::DoNotOptimize(found);
    }
}
BENCHMARK(BM_VectorFind)->Arg(10000);

static void BM_SetFind(benchmark::State &state) {
    std::set<int> s;
    for (int i = 0; i < state.range(0); i++) s.insert(i);

    for (auto _ : state) {
        bool found = s.find(state.range(0) / 2) != s.end();
        benchmark::DoNotOptimize(found);
    }
}
BENCHMARK(BM_SetFind)->Arg(10000);

static void BM_UnorderedSetFind(benchmark::State &state) {
    std::unordered_set<int> s;
    for (int i = 0; i < state.range(0); i++) s.insert(i);

    for (auto _ : state) {
        bool found = s.find(state.range(0) / 2) != s.end();
        benchmark::DoNotOptimize(found);
    }
}
BENCHMARK(BM_UnorderedSetFind)->Arg(10000);

BENCHMARK_MAIN();
```

**预期性能对比**：

| 操作 | vector | list | set | unordered_set |
|------|--------|------|-----|---------------|
| 尾部插入 | O(1) 均摊 | O(1) | O(log n) | O(1) 均摊 |
| 查找 | O(log n) 二分 | O(n) | O(log n) | O(1) 均摊 |
| 顺序遍历 | 最快（缓存友好） | 慢（缓存不友好） | 中等 | 中等 |
| 内存开销 | 最小 | 大（每节点2指针） | 大（红黑树） | 中等（哈希表） |

***

### 11. 极简总结

| 概念 | 要点 |
|------|------|
| **微基准测试** | 测量单个函数/片段，隔离性好，精度高 |
| **宏基准测试** | 测量完整系统，贴近真实，噪声大 |
| **Google Benchmark** | C++ 主流框架，内置统计、防优化、参数化 |
| **DoNotOptimize** | 防止编译器消除被测代码的计算结果 |
| **ClobberMemory** | 防止编译器缓存内存读取 |
| **chrono** | 可移植计时方案，steady_clock 最可靠 |
| **rdtsc** | x86 周期级计时，需固定 CPU 频率 |
| **统计方法** | 必须多次采样，关注中位数和标准差 |
| **缓存预热** | 冷/热缓存性能差异可达 10x |
| **分支预测** | 规律数据与随机数据性能差异显著 |
| **多线程** | 注意伪共享、锁竞争、NUMA 效应 |

**关键记忆**：
- 基准测试三原则：**隔离、重复、防优化**
- 永远不要相信单次运行结果，统计才有意义
- `DoNotOptimize` 和 `ClobberMemory` 是防止编译器"聪明反被聪明误"的利器
- 缓存是性能之王，顺序访问比随机访问快 10-50 倍
- 多线程基准测试要绑核、固定频率、关注伪共享

***

### 相关阅读

- [什么是模糊测试Fuzzing](./09-什么是模糊测试Fuzzing.md)
- [什么是性能剖析Profiling](./08-什么是性能剖析Profiling.md)
- [什么是SIMD向量化编程](./03-什么是SIMD向量化编程.md)