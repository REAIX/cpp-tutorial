# 什么是性能剖析 Profiling
> 📖 相关章节：[调试技巧](../../04-工程实践/06-调试技巧.md)、[性能优化](../../04-工程实践/08-性能优化.md)

> "先度量，再优化"——性能工程的第一法则

***

### 1. 先抓核心

性能剖析（Profiling）是通过采样或插桩手段收集程序运行时的 CPU、内存等资源消耗数据，定位性能瓶颈的系统性方法。

***

### 2. 采样与插桩——两大核心方法

| 维度 | 采样（Sampling） | 插桩（Instrumentation） |
|------|-------------------|------------------------|
| 原理 | 周期性中断，记录调用栈 | 在代码中插入测量指令 |
| 开销 | 低（1-5%） | 中到高（10-100%） |
| 精度 | 统计近似，热点准确 | 精确到每次调用 |
| 数据 | 函数/地址的采样频率 | 调用次数、耗时、内存分配 |
| 适用 | 生产环境、长时间运行 | 开发阶段、精确分析 |
| 代表工具 | perf、gprof、Instruments | gprof（编译插桩）、VTune、gperftools |

**采样示意**：

```
时间线:  ─────────────────────────────────────>
采样点:     ▼     ▼     ▼     ▼     ▼     ▼
调用栈:   foo   foo   bar   foo   bar   bar

结果: foo 50% (3/6), bar 50% (3/6)
```

**插桩示意**：

```cpp
// 插桩前
void compute() {
    for (int i = 0; i < N; ++i) result += data[i];
}

// 插桩后（概念性展示）
void compute() {
    __profiler_enter("compute");          // 记录进入时间
    for (int i = 0; i < N; ++i) result += data[i];
    __profiler_exit("compute", elapsed);  // 记录退出时间
}
```

***

### 3. CPU 性能剖析

**Linux perf——最强大的 CPU 剖析工具**：

```bash
# 采样 CPU 热点（基于硬件性能计数器）
perf record -g ./myapp

# 查看报告
perf report

# 采样指定进程
perf record -g -p <PID> sleep 10

# 采样特定事件
perf record -e cache-misses,branch-misses -g ./myapp

# 统计概览
perf stat ./myapp
```

`perf stat` 输出示例：

```
Performance counter stats for './myapp':

        3,542.78 msec  task-clock             #  0.998 CPUs utilized
              12       context-switches       #  3.387 /sec
               2       cpu-migrations         #  0.565 /sec
           8,431       page-faults            #  2.380 K/sec
  12,345,678,901       cycles                 #  3.484 GHz
   2,345,678,901       instructions           #  0.19  insn per cycle
     567,890,123       cache-references       # 160.314 M/sec
      45,678,901       cache-misses           #  8.043% of all cache refs
```

**gprof——经典编译插桩工具**：

```bash
# 编译时加入 -pg 选项
g++ -pg -O2 -o myapp main.cpp utils.cpp

# 运行程序，生成 gmon.out
./myapp

# 分析结果
gprof myapp gmon.out > analysis.txt
```

`gprof` 输出关键部分：

```
Flat profile:

 %   cumulative   self              self     total
time   seconds   seconds    calls  ms/call  ms/call  name
40.00      0.04     0.04   100000     0.00     0.00  sort_data
30.00      0.07     0.03   500000     0.00     0.00  hash_lookup
20.00      0.09     0.02  1000000     0.00     0.00  compare
10.00      0.10     0.01        1    10.00   100.00  main
```

**Intel VTune——深度微架构分析**：

| 分析类型 | 说明 |
|----------|------|
| Hotspots | CPU 热点与调用栈 |
| Microarchitecture Exploration | 流水线利用率、缓存分析 |
| Memory Access | 内存带宽与延迟 |
| Threading | 线程负载均衡 |
| HPC Performance Characterization | FLOPS、向量化率 |

```bash
# VTune 命令行采样
vtune -collect hotspots ./myapp
vtune -collect uarch-exploration ./myapp
vtune -report summary -result-dir r000hs
```

> **平台注意**：perf 仅限 Linux；Windows 上使用 Windows Performance Analyzer (WPA) 或 VTune；macOS 上使用 Instruments。

**Windows 性能分析工具**：

```powershell
# Windows Performance Recorder
wpr -start CPU
./myapp.exe
wpr -stop output.etl

# 使用 Xperf 分析
xperf -i output.etl -o report.txt -a stats

# Visual Studio 性能探查器（IDE 内置）
# 调试 → 性能探查器 → CPU 采样
```

***

### 4. 内存性能剖析

**Valgrind Memcheck——内存错误检测**：

```bash
# 检测内存泄漏、越界访问等
valgrind --leak-check=full --show-leak-kinds=all ./myapp
```

输出示例：

```
==12345== HEAP SUMMARY:
==12345==     in use at exit: 1,024 bytes in 1 blocks
==12345==   total heap usage: 100 allocs, 99 frees, 10,240 bytes allocated
==12345==
==12345== 1,024 bytes in 1 blocks are definitely lost in loss record 1 of 1
==12345==    at 0x4C2FB0F: malloc (in /usr/lib/valgrind/vgpreload_memcheck-amd64-linux.so)
==12345==    by 0x401156: create_buffer (main.cpp:42)
==12345==    by 0x401189: main (main.cpp:58)
```

**Valgrind Massif——堆内存剖析**：

```bash
# 分析堆内存增长趋势
valgrind --tool=massif ./myapp
ms_print massif.out.12345
```

**gperftools（tcmalloc + 剖析器）**：

```cpp
#include <gperftools/profiler.h>
#include <gperftools/heap-profiler.h>

int main() {
    ProfilerStart("cpu_profile.prof");
    HeapProfilerStart("heap_profile");

    run_workload();

    HeapProfilerStop();
    ProfilerStop();
    return 0;
}
```

```bash
# 编译
g++ -o myapp main.cpp -lprofiler -ltcmalloc

# 查看结果
pprof --text myapp cpu_profile.prof
pprof --web myapp cpu_profile.prof
```

**AddressSanitizer——编译器内置内存检测**：

```bash
# GCC/Clang 内置，无需外部工具
g++ -fsanitize=address -fno-omit-frame-pointer -g -o myapp main.cpp
./myapp
```

**Sanitizer 对比**：

| Sanitizer | 检测内容 | 开销 |
|-----------|---------|------|
| AddressSanitizer (ASan) | 越界、UAF、泄漏 | ~2x 内存，~2x 速度 |
| MemorySanitizer (MSan) | 未初始化读取 | ~3x 内存，~3x 速度 |
| ThreadSanitizer (TSan) | 数据竞争 | ~5-10x 内存，~5-15x 速度 |
| UndefinedBehaviorSanitizer | 未定义行为 | ~1.5x 速度 |

***

### 5. 火焰图（Flame Graph）

火焰图是性能剖析数据最直观的可视化方式，由 Brendan Gregg 发明。

**生成火焰图流程**：

```bash
# 1. 使用 perf 采集数据
perf record -F 99 -g ./myapp
# 或附加到运行中的进程
perf record -F 99 -g -p <PID> sleep 30

# 2. 折叠调用栈
perf script | stackcollapse-perf.pl > out.folded

# 3. 生成火焰图 SVG
flamegraph.pl out.folded > flamegraph.svg
```

**火焰图解读**：

```
                    ┌──────────┐
                    │  main()  │    ← 栈顶（叶子函数）
                    ├──────────┤
              ┌─────┤ process() │
              │     ├──────────┤
        ┌─────┤     │  read()   │    ← 宽度 = 采样占比
        │     │     └──────────┘
  ┌─────┤     │
  │init()│     │
  └─────┘     │
              └─────────────────
```

- **x 轴**：函数的采样占比（宽度越大越耗时）
- **y 轴**：调用栈深度（从下往上）
- **颜色**：通常无特殊含义，随机区分不同函数

**不同类型的火焰图**：

| 类型 | 数据来源 | 用途 |
|------|---------|------|
| CPU 火焰图 | perf record | CPU 热点 |
| Off-CPU 火焰图 | perf record -e sched:sched_stat_sleep | I/O 等待、锁竞争 |
| 内存火焰图 | brk/mmap 追踪 | 内存分配热点 |
| 差分火焰图 | 两次采样对比 | 版本间性能变化 |

**Off-CPU 分析**：

```bash
# 追踪进程等待时间
perf record -e sched:sched_stat_sleep -e sched:sched_stat_iowait -g -p <PID> sleep 30
perf script | stackcollapse-perf.pl | flamegraph.pl --countname=ms > offcpu.svg
```

> **平台注意**：火焰图工具链主要在 Linux 上使用。Windows 可用 Windows Performance Toolkit 生成类似图表；macOS 可用 Instruments 的 Call Tree 视图。

***

### 6. 性能剖析工作流

**标准性能优化工作流**：

```
1. 建立基准 → 2. 剖析定位 → 3. 分析原因 → 4. 实施优化 → 5. 验证效果 → 回到 1
```

**详细步骤**：

```bash
# 步骤 1：建立可重复的基准
# 使用稳定的测试数据集，关闭省电模式，绑核运行
taskset -c 0 ./myapp --benchmark

# 步骤 2：CPU 采样
perf record -F 999 -g ./myapp --benchmark
perf report --no-children

# 步骤 3：生成火焰图
perf script | stackcollapse-perf.pl | flamegraph.pl > before.svg

# 步骤 4：针对热点优化代码
# （修改源码）

# 步骤 5：重新基准测试
taskset -c 0 ./myapp --benchmark

# 步骤 6：对比火焰图
perf record -F 999 -g ./myapp --benchmark
perf script | stackcollapse-perf.pl | flamegraph.pl > after.svg
difffolded.pl before.folded after.folded | flamegraph.pl > diff.svg
```

**C++ 微基准测试**：

```cpp
#include <chrono>
#include <iostream>

class Timer {
public:
    Timer() : start_(std::chrono::high_resolution_clock::now()) {}

    double elapsed_ms() const {
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start_).count();
    }

private:
    std::chrono::high_resolution_clock::time_point start_;
};

void benchmark_sort() {
    std::vector<int> data(1000000);
    for (int i = 0; i < 1000000; ++i) data[i] = rand();

    Timer t;
    std::sort(data.begin(), data.end());
    std::cout << "sort: " << t.elapsed_ms() << " ms\n";
}
```

**Google Benchmark 集成**：

```cpp
#include <benchmark/benchmark.h>

static void BM_Sort(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    for (auto& x : data) x = rand();

    for (auto _ : state) {
        state.PauseTiming();
        auto copy = data;
        state.ResumeTiming();
        std::sort(copy.begin(), copy.end());
    }
    state.SetComplexityN(state.range(0));
}
BENCHMARK(BM_Sort)->Range(1<<10, 1<<20)->Complexity();
BENCHMARK_MAIN();
```

***

### 7. 观测者效应

观测者效应指剖析工具本身会影响程序行为，导致测量结果偏离真实情况。

**观测者效应来源**：

| 来源 | 影响 | 量化 |
|------|------|------|
| 采样中断 | 改变缓存状态 | 1-5% CPU 开销 |
| 插桩代码 | 增加指令数、改变分支预测 | 10-100% 开销 |
| Valgrind | 模拟执行，JIT 开销 | 20-50x 慢 |
| Sanitizer | 额外内存和检查 | 2-10x 开销 |
| I/O 写入 | 剖析数据写入磁盘 | 取决于采样频率 |

**减轻观测者效应的策略**：

```bash
# 1. 降低采样频率（默认 999Hz，可降低）
perf record -F 99 -g ./myapp

# 2. 限制采样范围
perf record -g -p <PID> sleep 5

# 3. 使用硬件性能计数器而非软件中断
perf record -e cycles:pp -g ./myapp

# 4. 远程采集，减少 I/O 影响
perf record -g -o /tmp/perf.data ./myapp

# 5. 使用统计方法多次采样取平均
for i in $(seq 1 10); do
    perf stat -x, ./myapp 2>&1 | grep "cycles"
done | awk -F, '{sum+=$1; count++} END {print sum/count}'
```

**Heisenbug 风险**：

```cpp
// 剖析可能改变时序，掩盖竞态条件
// 线程 A
data_ready = true;

// 线程 B
while (!data_ready) {}
process(data);

// 加上 printf 或 Sanitizer 后时序改变
// 竞态可能消失或改变表现
```

***

### 8. Amdahl 定律与优化优先级

**Amdahl 定律**：加速比受限于串行部分的比例。

$$S = \frac{1}{(1 - p) + \frac{p}{s}}$$

其中 $p$ 是可加速部分的比例，$s$ 是加速倍数。

| 串行比例 | 2x 加速 | 4x 加速 | 8x 加速 | 16x 加速 | ∞x 加速 |
|----------|---------|---------|---------|----------|---------|
| 5% | 1.05x | 1.05x | 1.05x | 1.05x | 1.05x |
| 10% | 1.11x | 1.18x | 1.22x | 1.23x | 1.11x |
| 25% | 1.33x | 1.60x | 1.78x | 1.88x | 1.33x |
| 50% | 1.33x | 1.60x | 1.78x | 1.88x | 2.00x |
| 75% | 1.60x | 2.28x | 2.91x | 3.37x | 4.00x |
| 90% | 1.82x | 3.07x | 4.71x | 6.40x | 10.0x |

**优化优先级策略**：

```
1. 剖析定位最大热点（占 CPU 时间最多的函数）
2. 优化该热点（算法 > 数据结构 > 微优化）
3. 重新剖析，找到下一个热点
4. 重复直到收益递减
```

**实际案例**：

```cpp
// 优化前：热点占 80% 时间
void process(std::vector<Item>& items) {
    for (auto& item : items) {
        if (find(cache.begin(), cache.end(), item.key) != cache.end()) {
            item.cached = true;
        }
    }
}

// 优化：O(n*m) → O(n)，热点从 80% 降到 5%
void process(std::vector<Item>& items) {
    std::unordered_set<std::string> cache_set(cache.begin(), cache.end());
    for (auto& item : items) {
        if (cache_set.count(item.key)) {
            item.cached = true;
        }
    }
}

// 整体加速: 1 / (0.2 + 0.8/20) ≈ 3.57x
```

***

### 9. 剖析工具速查对比

| 工具 | 平台 | 方法 | CPU | 内存 | 火焰图 | 开销 |
|------|------|------|-----|------|--------|------|
| perf | Linux | 采样 | ✅ | ✅ | ✅ | 低 |
| gprof | 全平台 | 插桩 | ✅ | ❌ | ❌ | 中 |
| Valgrind | Linux/macOS | 模拟 | ✅ | ✅ | ❌ | 极高 |
| VTune | Linux/Windows | 采样+插桩 | ✅ | ✅ | ✅ | 低-中 |
| Instruments | macOS | 采样+插桩 | ✅ | ✅ | ✅ | 低-中 |
| WPA | Windows | 采样 | ✅ | ✅ | ❌ | 低 |
| gperftools | Linux | 插桩 | ✅ | ✅ | ✅ | 中 |
| Sanitizers | 全平台 | 编译插桩 | 部分 | ✅ | ❌ | 中-高 |
| Tracy | 全平台 | 插桩 | ✅ | ✅ | ✅ | 低 |

**按场景选择工具**：

| 场景 | 推荐工具 |
|------|---------|
| 生产环境 CPU 热点 | perf + 火焰图 |
| 开发阶段内存泄漏 | ASan / Valgrind |
| 微架构瓶颈（缓存/分支） | VTune / perf stat |
| 多线程数据竞争 | TSan / Helgrind |
| 游戏实时帧分析 | Tracy / Pix |
| 快速验证优化效果 | Google Benchmark + perf stat |

***

### 10. 极简总结

| 要点 | 内容 |
|------|------|
| 两大方法 | 采样（低开销、统计近似）、插桩（高精度、高开销） |
| CPU 剖析 | perf（Linux）、VTune（跨平台）、Instruments（macOS） |
| 内存剖析 | ASan（编译器内置）、Valgrind（模拟执行）、gperftools |
| 可视化 | 火焰图——宽度=占比、高度=栈深 |
| 工作流 | 基准 → 剖析 → 分析 → 优化 → 验证 → 循环 |
| 观测者效应 | 工具本身影响结果，降低采样频率可缓解 |
| Amdahl 定律 | 优化最热点的收益最大，串行部分决定上限 |
| 核心原则 | 先度量再优化，用数据驱动决策 |

***

### 相关阅读

- [什么是模糊测试Fuzzing](12-什么是模糊测试Fuzzing.md)
- [什么是基准测试Benchmarking](./10-什么是基准测试Benchmarking.md)
- [CPP工具链](03-CPP工具链.md)

***