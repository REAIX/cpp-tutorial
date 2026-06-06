# 什么是大页内存HugePages
> 📖 相关章节：[内存管理](../../01-C语言/09-内存管理.md)、[高级内存管理](../../09-系统级编程与性能调优/01-高级内存管理.md)、[CPU缓存优化](../../09-系统级编程与性能调优/02-CPU缓存优化.md)

> 要点直击：大页内存（HugePages）把内存页从4KB增大到2MB或1GB，让TLB（地址翻译缓存）用更少的条目覆盖更多的内存——TLB缺失少了，地址翻译快了，数据库和虚拟化性能直接提升10%-30%。

***

### 1. 大页的原理

#### 1.1 虚拟内存与页表

程序使用的虚拟地址需要翻译成物理地址，这个过程由页表完成：

```
虚拟地址翻译过程：
  虚拟地址 → 查页表 → 物理地址 → 访问内存

4KB 页的页表结构（x86-64）：
  虚拟地址: [63:48] [47:39] [38:30] [29:21] [20:12] [11:0]
            符号位   PML4    PDPT     PD      PT     偏移
                     ↑       ↑       ↑       ↑
                   4级页表  3级页表  2级页表  1级页表

  每级页表有 512 个条目
  4级查找 = 4次内存访问（如果 TLB 未命中）
```

#### 1.2 TLB 的作用

**TLB（Translation Lookaside Buffer）** = 页表条目的缓存，加速地址翻译。

```
没有 TLB：
  每次内存访问需要 4 次额外内存访问（4级页表查找）
  一次内存访问变 5 次 → 5x 慢

有 TLB：
  TLB 命中：1 次访问（直接拿到物理地址）
  TLB 缺失：5 次访问（查页表 + 实际访问）

TLB 的大小有限（典型值）：
  L1 ITLB（指令）：64 条目
  L1 DTLB（数据）：64 条目
  L2 STLB（共享）：1536 条目
```

#### 1.3 4KB 页的 TLB 覆盖范围

```
4KB 页的 TLB 覆盖：
  L1 DTLB:  64 × 4KB = 256 KB
  L2 STLB:  1536 × 4KB = 6 MB

一个数据库的缓冲池可能有几十 GB
6 MB 的 TLB 覆盖远远不够！
→ 大量 TLB 缺失
→ 每次缺失要查 4 级页表
→ 性能严重下降
```

#### 1.4 大页的 TLB 覆盖范围

```
2MB 大页的 TLB 覆盖：
  L1 DTLB:  64 × 2MB = 128 MB
  L2 STLB:  1536 × 2MB = 3 GB

1GB 大页的 TLB 覆盖：
  L1 DTLB:  64 × 1GB = 64 GB
  L2 STLB:  1536 × 1GB = 1.5 TB

对比：
  4KB 页：  L2 覆盖 6 MB
  2MB 大页：L2 覆盖 3 GB   （500x 提升）
  1GB 大页：L2 覆盖 1.5 TB （250000x 提升）
```

#### 1.5 页表层级减少

```
4KB 页：4级页表（PML4 → PDPT → PD → PT → 页）
2MB 页：3级页表（PML4 → PDPT → PD → 页）    少1级
1GB 页：2级页表（PML4 → PDPT → 页）          少2级

页表查找次数：
  4KB 页：4次内存访问
  2MB 页：3次内存访问（少25%）
  1GB 页：2次内存访问（少50%）
```

***

### 2. TLB 缺失的影响

#### 2.1 TLB 缺失的代价

```
TLB 命中：  ~1 个时钟周期
TLB 缺失：  ~20-100 个时钟周期（取决于页表是否在缓存中）

对比：
  L1 缓存命中：  4 个时钟周期
  L2 缓存命中：  12 个时钟周期
  TLB 缺失：    20-100 个时钟周期

TLB 缺失可能比 L2 缓存缺失还慢！
```

#### 2.2 TLB 缺失率对比

```
场景：遍历 1GB 数组

4KB 页：
  TLB 条目需要：1GB / 4KB = 262144 个
  L2 STLB 容量：1536 个
  TLB 缺失率：  ~99.4%
  每次缺失代价：~50ns
  总 TLB 惩罚：  1GB / 64B × 99.4% × 50ns ≈ 800ms

2MB 大页：
  TLB 条目需要：1GB / 2MB = 512 个
  L2 STLB 容量：1536 个
  TLB 缺失率：  ~0%
  总 TLB 惩罚：  ≈ 0ms

差距：800ms vs 0ms！
```

#### 2.3 测量 TLB 缺失

```bash
# Linux perf 工具
perf stat -e dTLB-load-misses,dTLB-loads ./my_program

# 输出示例：
#   dTLB-loads:        100,000,000
#   dTLB-load-misses:   50,000,000  (50% 缺失率！)
#                              ↑ 超过 1% 就需要关注
```

```cpp
// 程序内测量 TLB 缺失
#include <linux/perf_event.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <cstdio>

long perf_event_open(struct perf_event_attr* attr, pid_t pid,
                     int cpu, int group_fd, unsigned long flags) {
    return syscall(__NR_perf_event_open, attr, pid, cpu, group_fd, flags);
}

int main() {
    struct perf_event_attr attr{};
    attr.type = PERF_TYPE_HW_CACHE;
    attr.size = sizeof(attr);
    attr.config = PERF_COUNT_HW_CACHE_DTLB |
                  (PERF_COUNT_HW_CACHE_OP_READ << 8) |
                  (PERF_COUNT_HW_CACHE_RESULT_MISS << 16);
    attr.disabled = 0;
    attr.exclude_kernel = 1;

    int fd = perf_event_open(&attr, 0, -1, -1, 0);
    if (fd == -1) {
        perror("perf_event_open");
        return 1;
    }

    long long misses;
    read(fd, &misses, sizeof(misses));
    printf("DTLB 缺失次数: %lld\n", misses);

    close(fd);
    return 0;
}
```

***

### 3. 透明大页与显式大页

#### 3.1 显式大页（HugePages）

**显式大页** = 需要系统管理员预先配置，应用程序显式申请使用。

```bash
# 配置显式大页
# 1. 查看当前大页配置
cat /proc/meminfo | grep Huge

# 输出示例：
# HugePages_Total:    0
# HugePages_Free:     0
# Hugepagesize:    2048 kB

# 2. 分配大页（需要 root）
echo 1024 > /proc/sys/vm/nr_hugepages  # 分配 1024 个 2MB 大页 = 2GB

# 3. 查看分配结果
cat /proc/meminfo | grep Huge
# HugePages_Total: 1024
# HugePages_Free:  1024
# Hugepagesize:    2048 kB

# 4. 持久化配置
echo "vm.nr_hugepages = 1024" >> /etc/sysctl.conf
sysctl -p
```

**显式大页的编程接口**：

```cpp
#include <sys/mman.h>
#include <cstdio>
#include <cstring>

int main() {
    size_t huge_page_size = 2 * 1024 * 1024;  // 2MB

    // 方式1：使用 MAP_HUGETLB 标志
    void* ptr = mmap(nullptr, huge_page_size,
                     PROT_READ | PROT_WRITE,
                     MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB,
                     -1, 0);
    if (ptr == MAP_FAILED) {
        perror("mmap HUGETLB 失败");
        return 1;
    }
    printf("使用大页分配了 %zu 字节\n", huge_page_size);

    // 使用内存
    memset(ptr, 0, huge_page_size);

    // 释放
    munmap(ptr, huge_page_size);

    // 方式2：使用 hugetlbfs 文件系统
    // mount -t hugetlbfs nodev /mnt/hugepages
    int fd = open("/mnt/hugepages/my_data", O_CREAT | O_RDWR, 0666);
    ptr = mmap(nullptr, huge_page_size,
               PROT_READ | PROT_WRITE,
               MAP_SHARED, fd, 0);

    // 使用...

    munmap(ptr, huge_page_size);
    close(fd);
    unlink("/mnt/hugepages/my_data");

    return 0;
}
```

#### 3.2 透明大页（THP）

**透明大页（Transparent Huge Pages, THP）** = 操作系统自动将连续的 4KB 页合并为 2MB 大页，对应用程序透明。

```bash
# 查看透明大页状态
cat /sys/kernel/mm/transparent_hugepage/enabled

# 输出：
# always [madvise] never
#   always:  总是尝试使用大页
#   madvise: 只对 MADV_HUGEPAGE 的内存区域使用大页
#   never:   不使用透明大页

# 设置透明大页模式
echo always > /sys/kernel/mm/transparent_hugepage/enabled
echo madvise > /sys/kernel/mm/transparent_hugepage/enabled
echo never > /sys/kernel/mm/transparent_hugepage/enabled
```

**使用 madvise 指定区域使用大页**：

```cpp
#include <sys/mman.h>
#include <cstdlib>
#include <cstdio>

int main() {
    size_t size = 100 * 1024 * 1024;  // 100MB

    // 分配普通内存
    void* ptr = malloc(size);
    if (!ptr) return 1;

    // 建议内核使用大页
    int ret = madvise(ptr, size, MADV_HUGEPAGE);
    if (ret != 0) {
        perror("madvise 失败");
    } else {
        printf("建议使用大页成功\n");
    }

    // 使用内存...
    memset(ptr, 0, size);

    // 取消大页建议
    madvise(ptr, size, MADV_NOHUGEPAGE);

    free(ptr);
    return 0;
}
```

#### 3.3 显式大页 vs 透明大页

| 维度 | 显式大页 | 透明大页 |
|------|---------|---------|
| 配置 | 需要预先配置 | 自动管理 |
| 内存使用 | 预分配，可能浪费 | 按需合并，更灵活 |
| 碎片 | 无碎片（预分配连续内存） | 可能无法合并（内存碎片） |
| 可靠性 | 高（保证有大页可用） | 不保证（可能退化为 4KB） |
| 适用场景 | 数据库、虚拟化 | 通用应用 |
| 代码修改 | 需要修改代码 | 无需修改（透明） |
| 页面大小 | 2MB 或 1GB | 仅 2MB |

***

### 4. 大页在数据库中的应用

#### 4.1 为什么数据库需要大页

```
数据库的内存访问模式：
1. 缓冲池（Buffer Pool）可能有几十 GB
2. 频繁随机访问缓冲池中的页面
3. 工作集远大于 TLB 覆盖范围
4. TLB 缺失率极高 → 性能严重下降

使用大页后：
1. TLB 覆盖范围扩大 500 倍
2. TLB 缺失率接近 0
3. 地址翻译开销大幅降低
4. 整体性能提升 10%-30%
```

#### 4.2 MySQL 使用大页

```bash
# 1. 配置系统大页
echo 5120 > /proc/sys/vm/nr_hugepages  # 10GB 大页

# 2. MySQL 配置
# my.cnf
[mysqld]
large-pages                  # 启用大页
innodb-buffer-pool-size=8G   # 缓冲池大小

# 3. 确保 MySQL 用户有权限使用大页
# /etc/security/limits.conf
mysql soft memlock unlimited
mysql hard memlock unlimited

# 4. 重启 MySQL
systemctl restart mysql

# 5. 验证
cat /proc/meminfo | grep Huge
# HugePages_Total: 5120
# HugePages_Free:  1024  ← 已使用 4096 个大页
```

#### 4.3 PostgreSQL 使用大页

```bash
# 1. 计算需要的大页数量
# shared_buffers = 4GB
# 需要的大页数 = 4GB / 2MB = 2048

echo 2048 > /proc/sys/vm/nr_hugepages

# 2. PostgreSQL 配置
# postgresql.conf
huge_pages = try    # 尝试使用大页，失败则回退
shared_buffers = 4GB

# 3. 重启 PostgreSQL
pg_ctl restart
```

#### 4.4 Redis 使用大页

```bash
# Redis 配置
# redis.conf
# 注意：Redis 的透明大页可能导致 fork 后的 COW 问题
# 建议：禁用透明大页，使用显式大页

echo never > /sys/kernel/mm/transparent_hugepage/enabled

# 启动 Redis 时使用大页
echo 512 > /proc/sys/vm/nr_hugepages
redis-server --save "" --maxmemory 1gb
```

***

### 5. 大页在虚拟化中的应用

#### 5.1 虚拟机为什么需要大页

```
虚拟机的内存翻译：
  虚拟机虚拟地址 → 虚拟机物理地址 → 宿主机虚拟地址 → 宿主机物理地址
                    二次翻译！

4KB 页的 TLB 压力：
  虚拟机有 8GB 内存 → 需要 2M 个页表条目
  宿主机 TLB 远远不够 → 大量 TLB 缺失

2MB 大页：
  虚拟机有 8GB 内存 → 只需要 4K 个页表条目
  TLB 完全覆盖 → 几乎零缺失
```

#### 5.2 KVM 使用大页

```bash
# 1. 配置大页
echo 4096 > /proc/sys/vm/nr_hugepages  # 8GB 大页

# 2. 启动虚拟机使用大页
qemu-system-x86_64 \
    -m 8G \
    -mem-path /dev/hugepages \  # 使用大页
    -smp 4 \
    -hda disk.qcow2

# 3. libvirt 配置
# XML 配置
# <memory backing>
#   <hugepages/>
# </memory backing>
```

#### 5.3 1GB 大页

```bash
# 1GB 大页需要特殊配置
# GRUB 配置：default_hugepagesz=1G hugepagesz=1G hugepages=4

# 更新 GRUB
grubby --update-kernel=ALL --args="default_hugepagesz=1G hugepagesz=1G hugepages=4"

# 重启后验证
cat /proc/meminfo | grep Huge
# HugePages_Total: 4
# Hugepagesize:  1048576 kB  ← 1GB

# 启动虚拟机使用 1GB 大页
qemu-system-x86_64 \
    -m 4G \
    -mem-path /dev/hugepages1G \
    -smp 4 \
    -hda disk.qcow2
```

***

### 6. 大页的性能测试

#### 6.1 顺序访问测试

```cpp
#include <sys/mman.h>
#include <chrono>
#include <cstdio>
#include <cstring>

void bench_sequential_access(bool use_hugepage) {
    const size_t SIZE = 512 * 1024 * 1024;  // 512MB
    const int ITERATIONS = 10;

    void* ptr;
    if (use_hugepage) {
        ptr = mmap(nullptr, SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
        if (ptr == MAP_FAILED) {
            printf("大页分配失败，回退到普通页\n");
            ptr = mmap(nullptr, SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        }
    } else {
        ptr = mmap(nullptr, SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }

    // 初始化
    memset(ptr, 1, SIZE);

    // 顺序读取测试
    auto start = std::chrono::high_resolution_clock::now();
    volatile char sink;  // 防止编译器优化掉读取
    for (int iter = 0; iter < ITERATIONS; ++iter) {
        char* p = static_cast<char*>(ptr);
        for (size_t i = 0; i < SIZE; i += 64) {  // 每缓存行读一次
            sink = p[i];
        }
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    printf("%s: %lld ms\n",
           use_hugepage ? "大页" : "普通页", ms.count());

    munmap(ptr, SIZE);
}

int main() {
    bench_sequential_access(false);  // 普通页
    bench_sequential_access(true);   // 大页
    return 0;
}
```

#### 6.2 随机访问测试

```cpp
#include <sys/mman.h>
#include <chrono>
#include <cstdio>
#include <random>
#include <vector>

void bench_random_access(bool use_hugepage) {
    const size_t SIZE = 512 * 1024 * 1024;  // 512MB
    const int NUM_ACCESSES = 10000000;

    void* ptr;
    if (use_hugepage) {
        ptr = mmap(nullptr, SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS | MAP_HUGETLB, -1, 0);
        if (ptr == MAP_FAILED) {
            ptr = mmap(nullptr, SIZE, PROT_READ | PROT_WRITE,
                       MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
        }
    } else {
        ptr = mmap(nullptr, SIZE, PROT_READ | PROT_WRITE,
                   MAP_PRIVATE | MAP_ANONYMOUS, -1, 0);
    }

    // 生成随机地址序列
    std::mt19937_64 rng(42);
    std::vector<size_t> offsets(NUM_ACCESSES);
    for (auto& off : offsets) {
        off = (rng() % (SIZE / 64)) * 64;  // 对齐到缓存行
    }

    // 随机访问测试
    char* data = static_cast<char*>(ptr);
    auto start = std::chrono::high_resolution_clock::now();

    volatile char sink;
    for (auto off : offsets) {
        sink = data[off];
    }

    auto end = std::chrono::high_resolution_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);

    printf("%s随机访问: %lld ms\n",
           use_hugepage ? "大页" : "普通页", ms.count());

    munmap(ptr, SIZE);
}

int main() {
    bench_random_access(false);
    bench_random_access(true);
    return 0;
}
```

#### 6.3 典型性能数据

```
512MB 数据，随机访问：

普通页（4KB）：  2500 ms  ████████████████████
大页（2MB）：    1800 ms  ██████████████       (+28% 提升)

1GB 数据，随机访问：
普通页（4KB）：  6000 ms  ████████████████████
大页（2MB）：    3800 ms  ████████████         (+37% 提升)
大页（1GB）：    3200 ms  ██████████           (+47% 提升)
```

***

### 7. 大页的注意事项

#### 7.1 透明大页的 COW 问题

```
Redis 的 fork + COW 场景：
1. Redis fork 子进程做 RDB 持久化
2. fork 后父子进程共享内存页（COW）
3. 父进程修改某个 4KB 页 → 只复制这一个页
4. 如果使用 2MB 大页 → 修改一个字节要复制整个 2MB 页！

结果：使用透明大页时，Redis fork 后的内存使用量暴增
解决：Redis 建议禁用透明大页
```

```bash
# Redis 推荐配置
echo never > /sys/kernel/mm/transparent_hugepage/enabled
```

#### 7.2 大页的内存浪费

```
大页的内部碎片：
  程序需要 3MB 内存
  使用 2MB 大页 → 需要 2 个大页 = 4MB
  浪费 1MB（25%）

  程序需要 100KB 内存
  使用 2MB 大页 → 需要 1 个大页 = 2MB
  浪费 1.9MB（95%！）

结论：小内存分配不适合使用大页
      大页适合大内存、长期驻留的场景
```

#### 7.3 大页的预留问题

```
显式大页是预分配的：
  配置 1024 个大页 → 系统启动时预留 2GB 内存
  这 2GB 不能被其他进程使用
  即使没有进程使用大页，这 2GB 也不可用

问题：过度预留导致普通内存不足
解决：根据实际需求精确配置大页数量
```

***

### 8. 常见误区

| 误区 | 事实 |
|------|------|
| "大页总是更快" | 小内存场景，大页反而浪费内存和 TLB 条目 |
| "透明大页完全透明" | 可能导致 COW 问题（如 Redis fork） |
| "1GB 大页比 2MB 大页好" | 1GB 大页浪费更多内存，适合超大内存场景 |
| "大页能减少内存使用" | 大页减少的是 TLB 缺失，不是内存使用 |
| "所有程序都应该用大页" | 只有工作集远大于 TLB 覆盖范围的程序才需要 |

***

### 9. 总结

| 要点 | 说明 |
|------|------|
| 大页 | 2MB 或 1GB 的内存页，替代 4KB 页 |
| TLB | 地址翻译缓存，大页让 TLB 覆盖范围扩大 500 倍 |
| TLB 缺失 | 代价 20-100 时钟周期，大页几乎消除缺失 |
| 显式大页 | 预分配，可靠但需手动配置 |
| 透明大页 | 自动管理，方便但可能有问题 |
| 数据库 | 大页提升 10%-30% 性能 |
| 虚拟化 | 大页减少二次翻译的 TLB 压力 |
| 注意事项 | COW 问题、内存浪费、预留管理 |