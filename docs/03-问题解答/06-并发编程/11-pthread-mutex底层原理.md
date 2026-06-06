# pthread_mutex 底层原理
> 📖 相关章节：[多线程基础](../../02-CPP/29-多线程基础.md)、[线程同步](../../02-CPP/30-线程同步.md)、[原子操作](../../02-CPP/31-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

### 1. 核心速览

`pthread_mutex_t` + `PTHREAD_MUTEX_INITIALIZER` 的**底层本质**：就是一个**内核同步对象 + 一堆状态标记的结构体**。

***

### 2. pthread_mutex_t 底层是什么？

它本质是一个**结构体**，里面大概藏着这些东西（不同系统略有差异，但原理一样）：

| 成员           | 作用                   |
| ------------ | -------------------- |
| **锁状态**      | 空闲 / 已加锁             |
| **持有线程 ID**  | 记录当前哪个线程拿到了锁         |
| **等待队列**     | 没抢到锁的线程，排队休眠挂在这里     |
| **内核句柄/信号量** | 用来跟操作系统内核交互，让线程阻塞、唤醒 |
| **锁类型**      | 普通锁/递归锁/检错锁          |
| **自旋次数**     | 用户态自旋等待的次数           |

**简化想象：**

```c
typedef struct {
    int locked;         // 0 空闲 1 已锁住
    long owner_tid;     // 持有锁的线程ID
    int kernel_fd;      // 内核等待对象
    int type;           // 锁类型
    int spin_count;     // 自旋次数
    // 还有等待队列头指针、递归计数等
} pthread_mutex_t;
```

**glibc 实际结构（简化版）**：

```c
// glibc 中 pthread_mutex_t 的实际内部结构（简化）
struct __pthread_mutex_s {
    int __lock;              // 锁状态：0=空闲, 1=已锁, 2=有等待者
    unsigned int __count;    // 递归锁的递归计数
    int __owner;             // 持有者线程ID
    unsigned int __nusers;   // 使用者计数
    int __kind;              // 锁类型
    int __spins;             // 自旋等待次数
    __pthread_list_t __list; // 等待队列
};
```

***

### 3. PTHREAD_MUTEX_INITIALIZER 底层是什么？

它是一个**宏**，本质是给这个结构体**填好默认初始值**。

大概等价于：

```c
#define PTHREAD_MUTEX_INITIALIZER {0, 0, 0, 0, PTHREAD_MUTEX_DEFAULT, 0, {0, 0}}
```

**作用**：

- 锁状态初始化为**未锁定**
- 没有线程持有
- 等待队列为空
- 内部标志位全部置0
- 锁类型为默认的普通锁

相当于**出厂预设好一把干净、可用的锁**。

#### 1. 初始化方式对比

| 方式 | 代码 | 适用场景 | 是否需要销毁 |
| --- | --- | --- | --- |
| 静态初始化 | `static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;` | 全局/静态变量 | 不需要（静态生命周期） |
| 动态初始化 | `pthread_mutex_init(&mtx, NULL);` | 局部变量/堆上 | 需要 `pthread_mutex_destroy` |

```c
// 方式1：静态初始化（推荐，最简单）
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

// 方式2：动态初始化（需要自定义属性时用）
pthread_mutex_t mtx;
pthread_mutex_init(&mtx, NULL);
// 用完后
pthread_mutex_destroy(&mtx);
```

***

### 4. 互斥锁的本质：内核对象 vs 用户态 futex

Linux 下 `pthread_mutex_t` 的实现采用了 **futex**（Fast Userspace muTEX）技术：

#### 1. futex 原理

```
加锁过程：
    │
    ├── 1. 用户态：原子操作尝试修改锁状态
    │       ├── 成功 → 直接返回（最快路径，不进内核）
    │       └── 失败 → 进入步骤2
    │
    ├── 2. 用户态：自旋等待若干次
    │       ├── 期间锁释放了 → 获取成功（较快路径）
    │       └── 自旋结束仍未获取 → 进入步骤3
    │
    └── 3. 内核态：调用 futex 系统调用
            ├── 线程进入内核等待队列休眠
            ├── 不消耗 CPU
            └── 被唤醒后重新尝试获取锁
```

#### 2. 两种路径对比

| 路径 | 场景 | 是否进内核 | 耗时 |
| --- | --- | --- | --- |
| 快速路径 | 锁空闲，原子操作直接拿到 | 否 | 纳秒级 |
| 自旋路径 | 锁被短暂持有，自旋等待 | 否 | 百纳秒级 |
| 慢速路径 | 锁被长时间持有，进内核休眠 | 是 | 微秒级 |

**关键洞察**：大多数情况下锁竞争不激烈，走快速路径，**根本不进内核**，所以互斥锁很快。

***

### 5. 加锁 pthread_mutex_lock 底层干了啥？

#### 1. 详细流程

```
pthread_mutex_lock(&mtx)
    │
    ├── 1. 原子操作：尝试把 __lock 从 0 改为 1
    │       ├── 成功 → 设置 owner = 当前线程ID → 返回0（加锁成功）
    │       └── 失败 → 进入步骤2
    │
    ├── 2. 自旋等待：循环尝试若干次
    │       ├── 期间 __lock 变为 0 → 原子抢锁 → 成功返回
    │       └── 自旋结束仍为 1 → 进入步骤3
    │
    └── 3. futex 系统调用：进入内核等待
            ├── 把 __lock 设为 2（表示有等待者）
            ├── 当前线程加入等待队列
            ├── 线程状态设为 TASK_INTERRUPTIBLE
            ├── 主动让出 CPU，进入休眠
            └── 被唤醒后 → 回到步骤1重新抢锁
```

**本质：能抢到就立刻进，抢不到就睡觉排队**

#### 2. 代码级理解

```c
// pthread_mutex_lock 的伪代码实现
int pthread_mutex_lock(pthread_mutex_t *mutex) {
    // 快速路径：原子尝试
    if (__sync_bool_compare_and_swap(&mutex->__lock, 0, 1)) {
        mutex->__owner = gettid();
        return 0;  // 加锁成功，没进内核
    }

    // 自旋路径：短暂等待
    for (int i = 0; i < SPIN_COUNT; i++) {
        if (__sync_bool_compare_and_swap(&mutex->__lock, 0, 1)) {
            mutex->__owner = gettid();
            return 0;
        }
        cpu_relax();  // PAUSE 指令，降低功耗
    }

    // 慢速路径：进内核休眠
    while (1) {
        mutex->__lock = 2;  // 标记有等待者
        futex_wait(&mutex->__lock, 2);  // 内核等待
        if (__sync_bool_compare_and_swap(&mutex->__lock, 0, 1)) {
            mutex->__owner = gettid();
            return 0;
        }
    }
}
```

***

### 6. 解锁 pthread_mutex_unlock 底层干了啥？

#### 1. 详细流程

```
pthread_mutex_unlock(&mtx)
    │
    ├── 1. 检查：当前线程是否是锁的持有者
    │       └── 不是 → 返回 EPERM（无权解锁）
    │
    ├── 2. 清除 owner 字段
    │
    ├── 3. 检查 __lock 的值
    │       ├── __lock == 1（没有等待者）
    │       │       └── 原子设为 0 → 返回（最简单情况）
    │       │
    │       └── __lock == 2（有等待者）
    │               ├── 原子设为 0
    │               └── futex_wake 唤醒一个等待线程
    │
    └── 4. 被唤醒的线程重新抢锁
```

**本质：开门 + 叫醒一个排队的线程**

#### 2. 代码级理解

```c
// pthread_mutex_unlock 的伪代码实现
int pthread_mutex_unlock(pthread_mutex_t *mutex) {
    // 检查权限
    if (mutex->__owner != gettid()) {
        return EPERM;  // 不是持有者，不能解锁
    }

    mutex->__owner = 0;

    // 没有等待者：直接释放
    if (mutex->__lock == 1) {
        __sync_lock_release(&mutex->__lock);  // 设为0
        return 0;
    }

    // 有等待者：释放并唤醒
    mutex->__lock = 0;
    futex_wake(&mutex->__lock, 1);  // 唤醒1个等待线程
    return 0;
}
```

***

### 7. 锁的类型

pthread_mutex 支持 4 种锁类型：

| 类型 | 常量 | 行为 |
| --- | --- | --- |
| 普通锁 | `PTHREAD_MUTEX_DEFAULT` | 同一线程重复加锁 -> 死锁 |
| 递归锁 | `PTHREAD_MUTEX_RECURSIVE` | 同一线程可重复加锁，需等次数解锁 |
| 检错锁 | `PTHREAD_MUTEX_ERRORCHECK` | 同一线程重复加锁 -> 返回 EDEADLK |
| 适应锁 | `PTHREAD_MUTEX_ADAPTIVE_NP` | 自旋更久再休眠（Linux特有） |

#### 1. 普通锁（默认）

```c
// 默认类型，最常用
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

// 危险：同一线程重复加锁会死锁！
void dangerous_func() {
    pthread_mutex_lock(&mtx);
    // 如果这里又调用了一个也 lock(&mtx) 的函数...
    another_func();  // 里面也 lock(&mtx) -> 死锁！
    pthread_mutex_unlock(&mtx);
}
```

#### 2. 递归锁

```c
// 创建递归锁
pthread_mutex_t rec_mtx;
pthread_mutexattr_t attr;
pthread_mutexattr_init(&attr);
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
pthread_mutex_init(&rec_mtx, &attr);
pthread_mutexattr_destroy(&attr);

// 安全：同一线程可以重复加锁
void safe_func() {
    pthread_mutex_lock(&rec_mtx);    // 第1次加锁，count=1
    another_safe_func();             // 里面也加锁，count=2
    pthread_mutex_unlock(&rec_mtx);  // count=1
    // 还需要再 unlock 一次才能完全释放
}

void another_safe_func() {
    pthread_mutex_lock(&rec_mtx);    // 第2次加锁，count=2
    // 做事情...
    pthread_mutex_unlock(&rec_mtx);  // count=1
}
```

#### 3. 检错锁

```c
// 创建检错锁
pthread_mutexattr_t attr;
pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_ERRORCHECK);
pthread_mutex_init(&err_mtx, &attr);

// 重复加锁会返回错误而非死锁
int ret = pthread_mutex_lock(&err_mtx);   // 第1次，成功
ret = pthread_mutex_lock(&err_mtx);       // 第2次，返回 EDEADLK
if (ret == EDEADLK) {
    printf("检测到重复加锁！\n");
}
```

#### 4. 四种锁类型对比

| 特性 | 普通锁 | 递归锁 | 检错锁 | 适应锁 |
| --- | --- | --- | --- | --- |
| 同线程重复加锁 | 死锁 | 允许 | 返回错误 | 死锁 |
| 性能 | 高 | 稍低（维护计数） | 稍低（检查错误） | 最高（自旋更久） |
| 调试友好 | 差 | 一般 | 好 | 差 |
| 推荐场景 | 通用 | 需要递归调用 | 调试阶段 | 高性能场景 |

***

### 8. 为什么能用全局 static 直接初始化？

因为：

- `PTHREAD_MUTEX_INITIALIZER` 只是**常量数值填充**
- 全局/静态变量在程序**加载时**就把结构体初始好了
- 不需要运行时再调用 `pthread_mutex_init`

**对比**：

```c
// 全局静态初始化：程序加载时自动完成
static pthread_mutex_t g_mtx = PTHREAD_MUTEX_INITIALIZER;

// 动态初始化：运行时才初始化
void init() {
    pthread_mutex_t* mtx = malloc(sizeof(pthread_mutex_t));
    pthread_mutex_init(mtx, NULL);  // 运行时初始化
    // 用完后
    pthread_mutex_destroy(mtx);
    free(mtx);
}
```

***

### 9. 什么是临界区

就是**不能多线程同时跑的代码**，比如单例里这一段：

```c
if (inst == NULL) {
    inst = malloc(...);
}
```

多线程同时进来会出 bug，这就是**临界区**，必须加锁保护。

#### 1. 临界区的特征

| 特征 | 说明 |
| --- | --- |
| 访问共享数据 | 读写全局变量、堆内存、静态变量 |
| 原子性要求 | 必须作为一个整体执行，不能被打断 |
| 互斥性 | 同一时刻只能有一个线程进入 |

#### 2. 临界区的大小

```c
void* worker(void* arg) {
    // 非临界区：不需要锁
    int local_var = compute_something();

    // ---- 临界区开始 ----
    pthread_mutex_lock(&mtx);
    shared_counter++;       // 读写共享变量
    shared_result = local_var;
    pthread_mutex_unlock(&mtx);
    // ---- 临界区结束 ----

    // 非临界区：不需要锁
    process_result(shared_result);
}
```

**原则**：临界区越小越好，只包住必须串行的代码。

***

### 10. 性能分析

#### 1. 锁的开销来源

| 开销 | 耗时 | 触发条件 |
| --- | --- | --- |
| 原子操作 | ~10ns | 每次加锁/解锁 |
| 自旋等待 | ~100ns/次 | 锁被短暂持有时 |
| futex系统调用 | ~1-2us | 锁被长时间持有时 |
| 缓存行失效 | ~50-100ns | 多核间缓存一致性协议 |
| 上下文切换 | ~5-10us | 线程休眠/唤醒 |

#### 2. 锁的性能优化建议

```c
// 差：锁粒度太大
void* worker(void* arg) {
    pthread_mutex_lock(&mtx);
    // 大量计算（不需要锁）
    int result = heavy_computation();
    // 只有这一行需要锁
    shared_data = result;
    // 大量I/O（不需要锁）
    write_to_file(shared_data);
    pthread_mutex_unlock(&mtx);
}

// 好：锁粒度最小化
void* worker(void* arg) {
    int result = heavy_computation();   // 不加锁
    pthread_mutex_lock(&mtx);
    shared_data = result;               // 只锁必要的操作
    pthread_mutex_unlock(&mtx);
    write_to_file(shared_data);         // 不加锁
}
```

#### 3. 锁竞争程度对比

| 竞争程度 | 走的路径 | 性能影响 | 优化方向 |
| --- | --- | --- | --- |
| 无竞争 | 快速路径（原子操作） | 极小 | 无需优化 |
| 低竞争 | 偶尔自旋 | 小 | 减小临界区 |
| 中竞争 | 频繁自旋+偶尔休眠 | 中 | 减少锁持有时间 |
| 高竞争 | 频繁休眠+唤醒 | 大 | 重新设计数据结构/减少共享 |

***

### 11. 完整示例：互斥锁保护共享数据

```c
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#define THREAD_COUNT 4
#define ITERATIONS   1000000

// 共享数据
static int shared_counter = 0;
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

// 不加锁的线程（结果错误）
void* unsafe_increment(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        shared_counter++;  // 数据竞争！
    }
    return NULL;
}

// 加锁的线程（结果正确）
void* safe_increment(void* arg) {
    for (int i = 0; i < ITERATIONS; i++) {
        pthread_mutex_lock(&counter_mutex);
        shared_counter++;
        pthread_mutex_unlock(&counter_mutex);
    }
    return NULL;
}

int main() {
    pthread_t threads[THREAD_COUNT];

    // 测试1：不加锁
    shared_counter = 0;
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, unsafe_increment, NULL);
    }
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("不加锁: counter=%d (期望: %d)\n",
           shared_counter, THREAD_COUNT * ITERATIONS);

    // 测试2：加锁
    shared_counter = 0;
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, safe_increment, NULL);
    }
    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }
    printf("加锁后: counter=%d (期望: %d)\n",
           shared_counter, THREAD_COUNT * ITERATIONS);

    pthread_mutex_destroy(&counter_mutex);
    return 0;
}
```

**典型输出**：

```
不加锁: counter=2847193 (期望: 4000000)
加锁后: counter=4000000 (期望: 4000000)
```

***

### 12. 生活场景类比

把「临界区代码」当成**单人卫生间**：

- `lock` = 你进去反锁门
- 别人来想进：发现锁了，**排队站边上等着**
- `unlock` = 你出来开门
- 自动叫醒下一个排队的人进去

永远**同一时间只一个人在里面**。

#### 1. 不同锁类型的卫生间类比

| 锁类型 | 类比 |
| --- | --- |
| 普通锁 | 进门后门自动锁，再推门推不开（死锁） |
| 递归锁 | 同一个人可以反复进出，但要进出次数匹配 |
| 检错锁 | 同一个人再推门，门会提示"你已经进来了" |
| 适应锁 | 推不开门时先在门口等一会儿，实在不行再去排队 |

***

### 13. 常见陷阱与最佳实践

#### 1. 常见陷阱

| 陷阱 | 后果 | 修复 |
| --- | --- | --- |
| 忘记 unlock | 死锁，所有线程卡住 | lock/unlock 严格配对 |
| 临界区内 return | 锁未释放，死锁 | 用 goto 或确保所有路径都 unlock |
| 临界区内调用加锁函数 | 死锁（普通锁） | 用递归锁或重构代码 |
| 不同线程用不同锁 | 等于没锁 | 所有线程必须用同一把锁 |
| 锁粒度过大 | 性能退化为串行 | 只锁必要的最小代码段 |

#### 2. 最佳实践

1. **lock/unlock 配对**：像 malloc/free 一样严格
2. **临界区最小化**：只锁共享数据读写，不锁计算和I/O
3. **避免嵌套加锁**：容易死锁，必须嵌套时保证加锁顺序一致
4. **优先用静态初始化**：简单且不容易出错
5. **调试时用检错锁**：帮助发现重复加锁等问题

```c
// 安全的临界区写法：用 goto 确保解锁
void* worker(void* arg) {
    pthread_mutex_lock(&mtx);

    if (error_condition) {
        goto cleanup;  // 跳到解锁处
    }

    if (another_error) {
        goto cleanup;
    }

    // 正常逻辑
    shared_data = new_value;

cleanup:
    pthread_mutex_unlock(&mtx);  // 确保任何路径都解锁
    return NULL;
}
```

***

### 14. 极简总结

| 概念 | 底层本质 |
| --- | --- |
| `pthread_mutex_t` | 带状态、带等待队列、关联内核阻塞能力的结构体 |
| `PTHREAD_MUTEX_INITIALIZER` | 给结构体清零赋初始值的宏 |
| `lock` | 原子抢锁 -> 自旋等待 -> futex内核休眠 |
| `unlock` | 放锁 + 唤醒排队线程 |
| futex | 用户态快速路径 + 内核态慢速路径的混合实现 |
| 作用 | 保证一段代码同一时间只能一个线程跑 |

***

### 相关阅读

- [加锁解锁](./05-加锁解锁.md)
- [scoped-lock与lock-guard](./15-scoped-lock与lock-guard.md)
- [pthread-create底层原理](./01-pthread-create底层原理.md)

***