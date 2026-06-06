# 什么是RCU
> 📖 相关章节：[内存管理](../../01-C语言/09-内存管理.md)、[高级内存管理](../../09-系统级编程与性能调优/01-高级内存管理.md)、[CPU缓存优化](../../09-系统级编程与性能调优/02-CPU缓存优化.md)

> 精髓速览：RCU（Read-Copy-Update）让读者零开销地访问共享数据——读者直接读不加锁，写者先拷贝一份修改，等所有读者离开后再替换旧数据，读侧几乎零成本，是"读多写少"场景的终极武器。

***

### 1. RCU 的原理

#### 1.1 核心思想

```
传统读写锁：
  读者：获取读锁 → 读取 → 释放读锁（有锁开销）
  写者：获取写锁 → 修改 → 释放写锁（阻塞读者）

RCU：
  读者：直接读（无锁、无原子操作、无内存屏障）
  写者：拷贝 → 修改副本 → 替换指针 → 等待宽限期 → 释放旧数据
```

**三步走**：
1. **Read**：读者直接访问共享数据，不加任何锁
2. **Copy**：写者拷贝一份数据，在副本上修改
3. **Update**：写者原子地替换指针，等待宽限期后释放旧数据

#### 1.2 类比理解

```
类比：修改公告栏

读写锁方式：
  读者：先拿"阅读证" → 阅读 → 还"阅读证"
  写者：等所有人还证 → 挂"维护中"牌 → 修改 → 摘牌
  问题：读者有开销（拿证/还证），写者阻塞读者

RCU方式：
  读者：直接看公告栏（零开销）
  写者：复印一份 → 在复印件上改 → 把公告栏换成复印件 → 等看旧版的人走开 → 销毁旧版
  优势：读者完全不受影响
```

#### 1.3 宽限期（Grace Period）

```
时间线：
  ─────────────────────────────────────────────→ 时间

  读者A: [────读旧数据────]
  读者B:          [────读旧数据────]
  读者C:                    [────读新数据────]  ← 指针替换后进入

  写者:  拷贝→修改→替换指针 ──── 等待宽限期 ──── 释放旧数据
                                    ↑
                              所有旧读者都离开了
                              安全释放旧数据

宽限期 = 从替换指针到所有旧读者离开的时间
```

***

### 2. 读侧零开销

#### 2.1 为什么读侧零开销

```cpp
// 传统读写锁的读操作
pthread_rwlock_rdlock(&lock);    // 原子操作 + 可能的缓存行失效
// 读取数据
pthread_rwlock_unlock(&lock);    // 原子操作 + 可能的缓存行失效
// 开销：~25-50ns

// RCU 的读操作
rcu_read_lock();                 // 仅修改线程局部变量（1条指令）
// 读取数据
rcu_read_unlock();               // 仅修改线程局部变量（1条指令）
// 开销：~1-2ns
```

**关键**：`rcu_read_lock()` 和 `rcu_read_unlock()` 只修改线程局部变量，不涉及任何原子操作、内存屏障或缓存一致性协议。

#### 2.2 Linux 内核中的 RCU 读侧

```c
// Linux 内核 RCU 读侧实现（极简版）
// 非抢占内核：
#define rcu_read_lock()    preempt_disable()   // 禁止抢占
#define rcu_read_unlock()  preempt_enable()    // 允许抢占

// 可抢占内核：
#define rcu_read_lock()    __rcu_read_lock()   // 增加嵌套计数
#define rcu_read_unlock()  __rcu_read_unlock() // 减少嵌套计数
```

**为什么禁止抢占就够了？** 因为如果读者不会被调度走，那么宽限期结束后，一定没有读者还在访问旧数据。

#### 2.3 读侧零开销的前提

| 条件 | 说明 |
|------|------|
| 读者不能阻塞 | 读临界区内不能睡眠/阻塞 |
| 读者不能长时间持有 | 读临界区应该短小 |
| 读者通过指针访问 | 写者替换指针，读者看到新或旧数据 |
| 读者不修改数据 | 读操作完全无副作用 |

***

### 3. 宽限期与回收

#### 3.1 宽限期的确定

```
宽限期结束的条件：
  所有在宽限期开始前进入读临界区的读者都已经退出

检测方法：
  1. 静止状态（Quiescent State）：线程不在 RCU 读临界区内
  2. 宽限期：所有线程都至少经历了一次静止状态
```

#### 3.2 宽限期的等待方式

```c
// 方式1：同步等待（synchronize_rcu）
// 阻塞直到宽限期结束
void writer_update() {
    // 1. 拷贝并修改
    struct data* new_data = copy_and_modify(old_data);

    // 2. 替换指针
    rcu_assign_pointer(global_ptr, new_data);

    // 3. 等待宽限期
    synchronize_rcu();  // 阻塞，直到所有旧读者退出

    // 4. 释放旧数据
    kfree(old_data);
}

// 方式2：异步回调（call_rcu）
// 注册回调，宽限期结束后自动调用
void writer_update_async() {
    struct data* new_data = copy_and_modify(old_data);
    rcu_assign_pointer(global_ptr, new_data);

    // 异步回收：宽限期结束后调用 free_old_data
    call_rcu(&old_data->rcu_head, free_old_data);
}

// 回调函数
void free_old_data(struct rcu_head* head) {
    struct data* old = container_of(head, struct data, rcu_head);
    kfree(old);
}
```

#### 3.3 宽限期的时间

```
典型宽限期长度：
  最短：几微秒（所有线程快速通过静止状态）
  典型：几十微秒
  最长：几毫秒（有线程长时间在读临界区内）

影响因素：
  - 读临界区的长度
  - 线程数量
  - 内核调度频率
```

***

### 4. RCU 在 Linux 内核中的应用

#### 4.1 内核中的典型用法

```c
// 示例1：RCU 保护的全局配置
struct config {
    int max_connections;
    int timeout_ms;
    struct rcu_head rcu;
};

struct config __rcu* global_config;

// 读者：零开销读取配置
int get_timeout() {
    int timeout;
    rcu_read_lock();
    struct config* cfg = rcu_dereference(global_config);
    timeout = cfg->timeout_ms;
    rcu_read_unlock();
    return timeout;
}

// 写者：更新配置
void update_config(int new_timeout) {
    struct config* old = rcu_dereference(global_config);
    struct config* new_cfg = kmalloc(sizeof(*new_cfg), GFP_KERNEL);
    *new_cfg = *old;                     // 拷贝
    new_cfg->timeout_ms = new_timeout;   // 修改副本

    rcu_assign_pointer(global_config, new_cfg);  // 替换指针
    kfree_rcu(old, rcu);               // 宽限期后释放旧配置
}

// 示例2：RCU 保护的链表遍历
// 读者遍历链表不需要锁
void for_each_process_rcu() {
    rcu_read_lock();
    struct task_struct* p;
    for (p = rcu_dereference(task_list); p;
         p = rcu_dereference(p->next)) {
        // 安全地读取 p 的信息
        // 即使其他线程在删除节点，当前节点在宽限期内不会被释放
    }
    rcu_read_unlock();
}
```

#### 4.2 内核 RCU 的变体

| 变体 | 说明 | 适用场景 |
|------|------|---------|
| RCU | 经典 RCU，读侧禁止抢占 | 通用场景 |
| SRCU | 可睡眠的 RCU | 读临界区可能睡眠 |
| RCU-sched | 基于调度的 RCU | 不关心睡眠的场景 |
| RCU-bh | 基于下半部的 RCU | 网络软中断场景 |
| Tiny RCU | 单核专用 RCU | 嵌入式单核系统 |
| Tasks RCU | 基于任务的 RCU | 跟踪用户态任务 |

***

### 5. 用户态 RCU

#### 5.1 为什么需要用户态 RCU

- Linux 内核的 RCU 不能直接在用户态使用
- 用户态程序也需要"读多写少"的高性能并发
- 数据库、网络服务器等场景非常适合 RCU

#### 5.2 liburcu（用户态 RCU 库）

```cpp
// 安装：sudo apt install liburcu-dev
// 编译：g++ -lurcu main.cpp -o main

#include <urcu.h>
#include <urcu/rculfhash.h>
#include <cstdio>
#include <cstdlib>

struct config {
    int value;
    struct rcu_head rcu;
};

struct config* global_cfg;

// 读者：零开销读取
int read_config() {
    int val;
    rcu_read_lock();
    struct config* cfg = reinterpret_cast<struct config*>(
        rcu_dereference(global_cfg));
    val = cfg->value;
    rcu_read_unlock();
    return val;
}

// 写者：拷贝-修改-替换
void update_config(int new_value) {
    struct config* old = reinterpret_cast<struct config*>(
        rcu_dereference(global_cfg));
    struct config* new_cfg = new config();
    new_cfg->value = new_value;

    rcu_assign_pointer(global_cfg, new_cfg);
    synchronize_rcu();  // 等待宽限期
    delete old;
}

int main() {
    // 初始化
    global_cfg = new config{42, {}};

    // 读取
    printf("配置值: %d\n", read_config());

    // 更新
    update_config(100);
    printf("更新后: %d\n", read_config());

    return 0;
}
```

#### 5.3 QSBR（Quiescent-State-Based Reclamation）

```cpp
// QSBR 是用户态 RCU 的一种实现方式
// 每个线程定期报告自己处于静止状态

#include <atomic>
#include <vector>
#include <cstdio>

class QSBR {
private:
    std::atomic<uint64_t> global_epoch{0};
    std::vector<std::atomic<uint64_t>> thread_epochs;
    int thread_count;

public:
    QSBR(int n) : thread_epochs(n), thread_count(n) {
        for (auto& e : thread_epochs) e.store(0);
    }

    // 线程报告静止状态
    void quiescent_state(int thread_id) {
        uint64_t current = global_epoch.load(std::memory_order_acquire);
        thread_epochs[thread_id].store(current, std::memory_order_release);
    }

    // 等待宽限期：所有线程都进入了当前 epoch
    void wait_for_grace_period() {
        uint64_t new_epoch = global_epoch.fetch_add(1, std::memory_order_acq_rel) + 1;

        // 等待所有线程进入新 epoch
        for (int i = 0; i < thread_count; ++i) {
            while (thread_epochs[i].load(std::memory_order_acquire) < new_epoch) {
                // 自旋等待或让出 CPU
                std::this_thread::yield();
            }
        }
    }
};
```

#### 5.4 简单的用户态 RCU 实现

```cpp
#include <atomic>
#include <vector>
#include <thread>
#include <cstdio>

class SimpleUserRCU {
private:
    static constexpr int MAX_THREADS = 64;
    std::atomic<int> reader_count[MAX_THREADS]{};
    std::atomic<bool> reader_active[MAX_THREADS]{};

public:
    // 注册线程
    int register_thread() {
        static std::atomic<int> next_id{0};
        int id = next_id.fetch_add(1);
        reader_active[id].store(true, std::memory_order_release);
        return id;
    }

    // 读侧：进入读临界区
    void read_lock(int thread_id) {
        reader_count[thread_id].fetch_add(1, std::memory_order_acquire);
    }

    // 读侧：退出读临界区
    void read_unlock(int thread_id) {
        reader_count[thread_id].fetch_add(1, std::memory_order_release);
    }

    // 写侧：等待宽限期
    void synchronize() {
        // 记录所有线程的计数器快照
        int snapshots[MAX_THREADS];
        for (int i = 0; i < MAX_THREADS; ++i) {
            if (reader_active[i].load(std::memory_order_acquire)) {
                snapshots[i] = reader_count[i].load(std::memory_order_acquire);
            } else {
                snapshots[i] = -1;  // 线程未注册
            }
        }

        // 等待所有活跃线程的计数器变化（说明它们退出了读临界区）
        for (int i = 0; i < MAX_THREADS; ++i) {
            if (snapshots[i] < 0) continue;
            while (true) {
                int current = reader_count[i].load(std::memory_order_acquire);
                // 计数器变了，说明线程退出了读临界区
                if (current != snapshots[i]) break;
                std::this_thread::yield();
            }
        }
    }
};
```

***

### 6. RCU 的性能特征

#### 6.1 读侧性能

```
读操作开销（纳秒）：
  互斥锁：        ~50 ns（获取+释放）
  读写锁（读）：   ~30 ns
  RCU：           ~1-2 ns（仅线程局部变量操作）

读侧扩展性：
  互斥锁：        N线程读 → 串行化
  读写锁：        N线程读 → 原子操作争用
  RCU：           N线程读 → 完全并行，零争用
```

#### 6.2 写侧性能

```
写操作开销：
  互斥锁：        ~50 ns（获取+释放+修改）
  读写锁：        ~50 ns + 读者唤醒延迟
  RCU：           ~50 ns（拷贝+修改）+ 宽限期等待（~10-100μs）

关键：RCU 写侧更慢（需要拷贝+等待宽限期）
      但写侧慢是可接受的，因为写操作少
```

#### 6.3 何时选择 RCU

```
选择 RCU 的条件：
  ✅ 读多写少（读:写 > 100:1）
  ✅ 读侧延迟要求高
  ✅ 读临界区短小
  ✅ 数据通过指针访问
  ✅ 可以容忍旧数据（最终一致性）

不选择 RCU 的条件：
  ❌ 写操作频繁
  ❌ 读临界区可能阻塞
  ❌ 需要强一致性
  ❌ 数据很大（拷贝代价高）
```

***

### 7. RCU 的常见陷阱

#### 7.1 读临界区内阻塞

```c
// ❌ 错误：RCU 读临界区内睡眠
rcu_read_lock();
// ... 读取数据 ...
msleep(100);  // 危险！可能导致宽限期无法结束
rcu_read_unlock();

// ✅ 正确：读临界区短小，不阻塞
rcu_read_lock();
ptr = rcu_dereference(global_ptr);
value = ptr->value;  // 快速读取
rcu_read_unlock();
// 在锁外处理数据
process_value(value);
```

#### 7.2 写者不等待宽限期就释放

```c
// ❌ 错误：立即释放旧数据
rcu_assign_pointer(global_ptr, new_data);
kfree(old_data);  // 危险！读者可能还在访问

// ✅ 正确：等待宽限期后释放
rcu_assign_pointer(global_ptr, new_data);
synchronize_rcu();  // 等待宽限期
kfree(old_data);

// ✅ 正确：异步回调释放
call_rcu(&old_data->rcu_head, free_callback);
```

#### 7.3 读者修改数据

```c
// ❌ 错误：读者修改 RCU 保护的数据
rcu_read_lock();
struct data* p = rcu_dereference(global_ptr);
p->counter++;  // 危险！多个读者可能同时修改
rcu_read_unlock();

// ✅ 正确：读者只读，写者通过 COW 修改
rcu_read_lock();
struct data* p = rcu_dereference(global_ptr);
int val = p->counter;  // 只读
rcu_read_unlock();
```

***

### 8. 常见误区

| 误区 | 事实 |
|------|------|
| "RCU 没有锁" | RCU 读侧无锁，写侧可能用锁保护拷贝过程 |
| "RCU 读侧真的零开销" | 有极小的线程局部变量操作开销（~1ns） |
| "RCU 适合所有并发场景" | 只适合读多写少场景 |
| "RCU 保证读到最新数据" | RCU 只保证不读到半更新数据，可能读到旧数据 |
| "用户态不能用 RCU" | liburcu 提供了用户态 RCU 实现 |

***

### 9. 总结

| 要点 | 说明 |
|------|------|
| RCU | Read-Copy-Update，读者零开销，写者拷贝后替换 |
| 读侧零开销 | 仅修改线程局部变量，无原子操作和内存屏障 |
| 宽限期 | 所有旧读者退出后，安全释放旧数据 |
| 内核应用 | 全局配置、链表遍历、路由表等 |
| 用户态 RCU | liburcu 库，QSBR 等实现 |
| 适用场景 | 读多写少、读侧延迟敏感、可容忍旧数据 |
| 不适用 | 写多读少、需要强一致性、读临界区可能阻塞 |