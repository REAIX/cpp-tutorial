# 什么是NUMA架构
> 📖 相关章节：[内存管理](../../01-C语言/09-内存管理.md)、[高级内存管理](../../09-系统级编程与性能调优/01-高级内存管理.md)、[CPU缓存优化](../../09-系统级编程与性能调优/02-CPU缓存优化.md)

> 核心速览：NUMA（Non-Uniform Memory Access）架构下，CPU访问自己本地的内存飞快，访问其他CPU的内存要走互联总线——就像你从自家冰箱拿饮料秒拿，去邻居家冰箱拿饮料要敲门等开门，慢得多。

***

### 1. NUMA 的原理

#### 1.1 从 UMA 到 NUMA

**UMA（Uniform Memory Access）** = 所有 CPU 访问任何内存的延迟相同

```
UMA 架构（传统多核）：
  CPU0 ──┐
  CPU1 ──┤── 总线 ──┬── 内存
  CPU2 ──┤          │
  CPU3 ──┘          └── 内存

问题：CPU 越多，总线争用越严重，扩展性差
```

**NUMA（Non-Uniform Memory Access）** = 每个 CPU 有自己的本地内存，访问本地快，访问远端慢

```
NUMA 架构：
  Node 0                    Node 1
  ┌──────────┐             ┌──────────┐
  │ CPU0 CPU1│──互联总线──│ CPU2 CPU3│
  │  内存0   │  (QPI/InfinityFabric) │  内存1   │
  └──────────┘             └──────────┘

  CPU0 访问内存0：本地访问，~80ns
  CPU0 访问内存1：远端访问，~120ns（慢 50%）
```

#### 1.2 NUMA 节点（Node）

```
典型服务器 NUMA 拓扑：

双路服务器（2个CPU）：
  Node 0: CPU 0-15, 内存 0-64GB
  Node 1: CPU 16-31, 内存 64-128GB

四路服务器（4个CPU）：
  Node 0: CPU 0-15, 内存 0-64GB
  Node 1: CPU 16-31, 内存 64-128GB
  Node 2: CPU 32-47, 内存 128-192GB
  Node 3: CPU 48-63, 内存 192-256GB
```

#### 1.3 NUMA 的访问延迟

```
访问延迟对比（典型 x86 服务器）：

本地内存访问：   ~80ns    ████████████
跨1跳远端访问：  ~120ns   ██████████████████  (+50%)
跨2跳远端访问：  ~160ns   ████████████████████████  (+100%)

类比：
  本地内存 = 自己家的冰箱（1秒拿到）
  跨1跳   = 隔壁邻居的冰箱（1.5秒，要敲门）
  跨2跳   = 对面楼的冰箱（2秒，要下楼过马路）
```

#### 1.4 NUMA 的带宽

```
内存带宽对比（DDR4-2666，双路服务器）：

本地带宽：  ~60 GB/s  ████████████████████
远端带宽：  ~40 GB/s  ████████████         (-33%)

关键：远端访问不仅延迟高，带宽也低
      互联总线（QPI/UPI/Infinity Fabric）是瓶颈
```

***

### 2. NUMA 感知内存分配

#### 2.1 查看 NUMA 拓扑

```bash
# Linux 查看 NUMA 信息
numactl --hardware

# 输出示例：
# available: 2 nodes (0-1)
# node 0 size: 65536 MB
# node 1 size: 65536 MB
# node 0 free: 32000 MB
# node 1 free: 28000 MB

# 查看进程的 NUMA 内存分布
numastat -p <pid>

# 查看每个节点的内存统计
cat /sys/devices/system/node/node0/meminfo
cat /sys/devices/system/node/node1/meminfo
```

#### 2.2 NUMA 内存分配策略

| 策略 | 说明 | 适用场景 |
|------|------|---------|
| default | 从当前线程所在的节点分配 | 通用 |
| bind | 强制从指定节点分配 | 延迟敏感 |
| interleave | 交替从所有节点分配 | 带宽敏感 |
| preferred | 优先从指定节点分配，不够则从其他节点 | 尽量本地 |

#### 2.3 numactl 命令行工具

```bash
# 强制在 Node 0 上分配内存
numactl --membind=0 ./my_program

# 优先在 Node 0 上分配内存
numactl --preferred=0 ./my_program

# 交替从所有节点分配内存（带宽优先）
numactl --interleave=all ./my_program

# 绑定到 Node 0 的 CPU 和内存
numactl --cpunodebind=0 --membind=0 ./my_program
```

#### 2.4 编程接口

```cpp
#include <numa.h>
#include <numaif.h>
#include <cstdio>
#include <cstdlib>

int main() {
    if (numa_available() < 0) {
        printf("系统不支持 NUMA\n");
        return 1;
    }

    // 查询 NUMA 信息
    int num_nodes = numa_num_configured_nodes();
    printf("NUMA 节点数: %d\n", num_nodes);

    // 在指定节点上分配内存
    void* ptr = numa_alloc_onnode(1024 * 1024, 0);  // 在 Node 0 分配 1MB
    if (ptr) {
        printf("在 Node 0 分配了 1MB 内存\n");
        numa_free(ptr, 1024 * 1024);
    }

    // 查询内存所在的节点
    ptr = malloc(4096);
    int node = -1;
    if (get_mempolicy(&node, nullptr, 0, ptr, MPOL_F_ADDR) == 0) {
        printf("分配的内存在 Node %d\n", node);
    }
    free(ptr);

    // 设置内存分配策略
    struct bitmask* nodes = numa_parse_nodestring("0");
    set_mempolicy(MPOL_BIND, nodes->maskp, nodes->size);
    // 后续的 malloc 会优先从 Node 0 分配
    numa_bitmask_free(nodes);

    return 0;
}
```

#### 2.5 C++ NUMA 感知分配器

```cpp
#include <numa.h>
#include <numaif.h>
#include <cstddef>
#include <cstdio>
#include <new>

// NUMA 感知的 C++ 分配器
template<typename T>
class NumaAllocator {
private:
    int preferred_node_;

public:
    using value_type = T;

    explicit NumaAllocator(int node) : preferred_node_(node) {}

    template<typename U>
    NumaAllocator(const NumaAllocator<U>& other) noexcept
        : preferred_node_(other.node()) {}

    T* allocate(std::size_t n) {
        void* ptr = numa_alloc_onnode(n * sizeof(T), preferred_node_);
        if (!ptr) throw std::bad_alloc();
        return static_cast<T*>(ptr);
    }

    void deallocate(T* ptr, std::size_t n) noexcept {
        numa_free(ptr, n * sizeof(T));
    }

    int node() const { return preferred_node_; }
};

template<typename T, typename U>
bool operator==(const NumaAllocator<T>& a, const NumaAllocator<U>& b) {
    return a.node() == b.node();
}

template<typename T, typename U>
bool operator!=(const NumaAllocator<T>& a, const NumaAllocator<U>& b) {
    return a.node() != b.node();
}

// 使用示例
#include <vector>

int main() {
    // 在 Node 0 上分配 vector 的内存
    NumaAllocator<int> alloc(0);
    std::vector<int, NumaAllocator<int>> vec(alloc);
    vec.reserve(10000);

    // 在 Node 1 上分配另一个 vector
    NumaAllocator<int> alloc1(1);
    std::vector<int, NumaAllocator<int>> vec1(alloc1);
    vec1.reserve(10000);

    return 0;
}
```

***

### 3. 线程亲和性

#### 3.1 什么是线程亲和性

**线程亲和性（CPU Affinity）** = 将线程绑定到特定的 CPU 核心，避免线程在核心间迁移。

```
为什么需要亲和性：
1. 缓存失效：线程迁移到新核心，L1/L2 缓存全部失效
2. NUMA 惩罚：线程迁移到另一个 NUMA 节点，访问本地内存变远端
3. 抖动：多个线程争抢同一核心，频繁上下文切换
```

#### 3.2 设置线程亲和性

```cpp
#include <sched.h>
#include <pthread.h>
#include <cstdio>
#include <thread>

// 方式1：pthread_setaffinity_np
void set_affinity_pthread(std::thread& t, int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    int rc = pthread_setaffinity_np(t.native_handle(), sizeof(cpuset), &cpuset);
    if (rc != 0) {
        printf("设置亲和性失败\n");
    }
}

// 方式2：sched_setaffinity（当前线程）
void set_affinity_current(int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);
}

// 方式3：绑定到整个 NUMA 节点
void set_affinity_numa_node(int node_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    // 获取该节点的所有 CPU
    struct bitmask* cpus = numa_allocate_cpumask();
    numa_node_to_cpus(node_id, cpus);

    for (int i = 0; i < numa_num_configured_cpus(); ++i) {
        if (numa_bitmask_isbitset(cpus, i)) {
            CPU_SET(i, &cpuset);
        }
    }

    sched_setaffinity(0, sizeof(cpuset), &cpuset);
    numa_free_cpumask(cpus);
}

int main() {
    std::thread t([]() {
        set_affinity_current(0);  // 绑定到 CPU 0
        // ... 工作代码 ...
    });

    set_affinity_pthread(t, 0);
    t.join();
    return 0;
}
```

#### 3.3 NUMA 感知的线程池

```cpp
#include <vector>
#include <thread>
#include <functional>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <numa.h>

class NumaThreadPool {
private:
    struct Worker {
        std::thread thread;
        std::queue<std::function<void()>> tasks;
        std::mutex mtx;
        std::condition_variable cv;
        bool stop = false;
    };

    std::vector<std::unique_ptr<Worker>> workers_;
    int num_nodes_;

public:
    NumaThreadPool() : num_nodes_(numa_num_configured_nodes()) {
        int cpus_per_node = numa_num_configured_cpus() / num_nodes_;

        for (int node = 0; node < num_nodes_; ++node) {
            for (int i = 0; i < cpus_per_node; ++i) {
                auto worker = std::make_unique<Worker>();
                int core_id = node * cpus_per_node + i;

                worker->thread = std::thread([this, node, core_id]() {
                    // 绑定到指定核心和 NUMA 节点
                    set_affinity_current(core_id);

                    // 设置内存分配策略为本地优先
                    struct bitmask* nodes = numa_parse_nodestring(
                        std::to_string(node).c_str());
                    set_mempolicy(MPOL_PREFERRED, nodes->maskp, nodes->size);
                    numa_bitmask_free(nodes);

                    // 工作循环
                    Worker* w = workers_[core_id].get();
                    while (true) {
                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(w->mtx);
                            w->cv.wait(lock, [w] {
                                return w->stop || !w->tasks.empty();
                            });
                            if (w->stop && w->tasks.empty()) return;
                            task = std::move(w->tasks.front());
                            w->tasks.pop();
                        }
                        task();
                    }
                });

                workers_.push_back(std::move(worker));
            }
        }
    }

    // 在指定 NUMA 节点上执行任务
    void submit(int node, std::function<void()> task) {
        int cpus_per_node = numa_num_configured_cpus() / num_nodes_;
        int base_core = node * cpus_per_node;
        // 简单轮询选择该节点上的工作线程
        static std::atomic<int> counter{0};
        int core = base_core + (counter++ % cpus_per_node);

        {
            std::lock_guard<std::mutex> lock(workers_[core]->mtx);
            workers_[core]->tasks.push(std::move(task));
        }
        workers_[core]->cv.notify_one();
    }

    ~NumaThreadPool() {
        for (auto& w : workers_) {
            {
                std::lock_guard<std::mutex> lock(w->mtx);
                w->stop = true;
            }
            w->cv.notify_all();
            w->thread.join();
        }
    }
};
```

***

### 4. NUMA 对性能的影响

#### 4.1 内存访问延迟影响

```cpp
#include <numa.h>
#include <chrono>
#include <cstdio>
#include <thread>

void bench_numa_access() {
    const int SIZE = 64 * 1024 * 1024;  // 64MB
    const int ITERATIONS = 100;

    for (int local_node = 0; local_node < numa_num_configured_nodes(); ++local_node) {
        for (int mem_node = 0; mem_node < numa_num_configured_nodes(); ++mem_node) {
            // 在 mem_node 上分配内存
            int* data = static_cast<int*>(numa_alloc_onnode(SIZE, mem_node));

            // 在 local_node 的 CPU 上运行
            auto start = std::chrono::high_resolution_clock::now();

            // 顺序读取
            long long sum = 0;
            for (int iter = 0; iter < ITERATIONS; ++iter) {
                for (int i = 0; i < SIZE / sizeof(int); ++i) {
                    sum += data[i];
                }
            }

            auto end = std::chrono::high_resolution_clock::now();
            auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

            printf("CPU Node %d → 内存 Node %d: %lld ms (sum=%lld)\n",
                   local_node, mem_node, ms.count(), sum);

            numa_free(data, SIZE);
        }
    }
}
```

#### 4.2 典型性能数据

```
双路 Intel Xeon 服务器（2个NUMA节点）：

顺序读取 64MB 数据：
  本地访问：  120 ms  ████████████████████
  远端访问：  180 ms  ██████████████████████████████  (+50%)

随机读取：
  本地访问：  800 ms  ████████████████████
  远端访问：  1200 ms ██████████████████████████████  (+50%)

数据库查询（TPC-C）：
  NUMA 优化前：  10000 tpmC
  NUMA 优化后：  14000 tpmC  (+40%)
```

#### 4.3 NUMA 对不同应用的影响

| 应用类型 | NUMA 影响程度 | 原因 |
|---------|-------------|------|
| 数据库 | 严重 | 大量内存访问，延迟敏感 |
| Web 服务器 | 中等 | 请求可以被绑定到本地节点 |
| 科学计算 | 严重 | 大规模数组访问 |
| 虚拟化 | 严重 | 虚拟机跨节点访问 |
| 消息队列 | 中等 | 队列数据在节点间传递 |
| 缓存服务 | 严重 | 内存访问密集 |

***

### 5. NUMA 优化策略

#### 5.1 数据分区

```cpp
// 每个NUMA节点有自己的数据分区，线程只访问本地数据
class NumaPartitionedData {
private:
    int num_nodes_;
    std::vector<int*> partitions_;  // 每个节点一个数据分区

public:
    NumaPartitionedData(int size_per_node) {
        num_nodes_ = numa_num_configured_nodes();
        for (int node = 0; node < num_nodes_; ++node) {
            int* data = static_cast<int*>(
                numa_alloc_onnode(size_per_node * sizeof(int), node));
            partitions_.push_back(data);
        }
    }

    // 获取当前线程所在节点的数据
    int* get_local_data() {
        int cpu = sched_getcpu();
        int node = numa_node_of_cpu(cpu);
        return partitions_[node];
    }

    ~NumaPartitionedData() {
        for (int i = 0; i < num_nodes_; ++i) {
            numa_free(partitions_[i], 0);
        }
    }
};
```

#### 5.2 交叉分配（Interleaving）

```cpp
// 对于带宽敏感的场景，交叉分配可以平衡带宽
void allocate_interleaved(size_t size) {
    // 使用 interleave 策略分配内存
    struct bitmask* all_nodes = numa_all_nodes_ptr;
    set_mempolicy(MPOL_INTERLEAVE, all_nodes->maskp, all_nodes->size);

    void* ptr = malloc(size);

    // 恢复默认策略
    set_mempolicy(MPOL_DEFAULT, nullptr, 0);

    // 使用 ptr ...
    free(ptr);
}
```

#### 5.3 第一触碰策略（First-Touch Policy）

```cpp
// Linux 默认策略：内存页面分配在首次访问它的线程所在的 NUMA 节点
// 这意味着：谁先触碰数据，数据就在谁的本地节点

void first_touch_init(int* data, int size, int node) {
    // 在指定节点上初始化数据，确保内存分配在该节点
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);

    struct bitmask* cpus = numa_allocate_cpumask();
    numa_node_to_cpus(node, cpus);
    for (int i = 0; i < numa_num_configured_cpus(); ++i) {
        if (numa_bitmask_isbitset(cpus, i)) {
            CPU_SET(i, &cpuset);
        }
    }

    // 临时绑定到该节点
    cpu_set_t old_set;
    sched_getaffinity(0, sizeof(old_set), &old_set);
    sched_setaffinity(0, sizeof(cpuset), &cpuset);

    // 初始化数据（触碰页面，触发内存分配）
    for (int i = 0; i < size; ++i) {
        data[i] = 0;
    }

    // 恢复原亲和性
    sched_setaffinity(0, sizeof(old_set), &old_set);
    numa_free_cpumask(cpus);
}
```

***

### 6. 常见误区

| 误区 | 事实 |
|------|------|
| "NUMA 只影响大型服务器" | 消费级 AMD Ryzen 也有 NUMA（Chiplet 设计） |
| "NUMA 优化就是绑核" | 绑核只是手段之一，数据局部性更重要 |
| "所有程序都需要 NUMA 优化" | 小程序和低内存程序影响不大 |
| "interleave 总是好的" | interleave 提高带宽但增加延迟，不适合延迟敏感场景 |
| "虚拟机不需要关心 NUMA" | 虚拟机跨节点访问性能下降严重 |

***

### 7. 总结

| 要点 | 说明 |
|------|------|
| NUMA | 非一致内存访问，本地快远端慢 |
| 延迟差异 | 远端比本地慢 50%-100% |
| 内存分配 | 本地优先、绑定、交叉分配三种策略 |
| 线程亲和性 | 绑定线程到核心，避免迁移和远端访问 |
| 数据分区 | 每个节点独立数据，减少跨节点访问 |
| 第一触碰 | 谁先访问数据，数据就在谁的节点 |
| 适用场景 | 数据库、科学计算、虚拟化等内存密集型应用 |