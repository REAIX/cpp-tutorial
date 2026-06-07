# 什么是_Atomic与C11原子操作
> 📖 相关章节：[原子操作与异步编程](../../02-CPP/29-原子操作与异步编程.md)、[多线程基础](../../02-CPP/27-多线程基础.md)

> "_Atomic=给变量加了一把'原子锁'，读写不可分割，不会被其他线程打断。"——C11引入的原子类型和操作，让C语言也能写出正确的并发程序。

***

### 1. 通俗理解

- **_Atomic** = C11标准引入的类型修饰符，让变量的读写操作变成"原子的"（不可分割的）
- 就像银行转账：非原子操作是"先扣钱再加钱"两步，中间可能出问题；原子操作是"瞬间完成"，不可能被打断
- 多线程下，普通变量的读写可能被其他线程看到"半完成"的状态，_Atomic保证操作完整性
- C11还提供了`stdatomic.h`头文件，包含完整的原子操作函数

| 概念 | 类比 | 说明 |
|------|------|------|
| _Atomic | 原子保险箱 | 读写不可分割，其他线程看不到中间状态 |
| atomic_load | 看一眼保险箱 | 原子地读取值 |
| atomic_store | 往保险箱放东西 | 原子地写入值 |
| atomic_fetch_add | 保险箱里加钱 | 原子地读取并加上一个值 |
| memory_order | 保险箱的可见性规则 | 控制操作的可见性顺序 |

***

### 2. 技术说明

#### 1. _Atomic类型修饰符

C11引入了`_Atomic`类型修饰符，可以两种方式使用：

```c
/* 方式1：_Atomic加类型 */
_Atomic int counter;

/* 方式2：_Atomic加括号（更复杂类型时使用） */
_Atomic(int) counter;
```

#### 2. stdatomic.h头文件

```c
#include <stdatomic.h>
```

**常用原子类型别名**：

| 类型别名 | 等价于 | 说明 |
|---------|--------|------|
| atomic_bool | _Atomic _Bool | 原子布尔 |
| atomic_char | _Atomic char | 原子字符 |
| atomic_int | _Atomic int | 原子整数 |
| atomic_uint | _Atomic unsigned int | 原子无符号整数 |
| atomic_long | _Atomic long | 原子长整数 |
| atomic_ulong | _Atomic unsigned long | 原子无符号长整数 |
| atomic_size_t | _Atomic size_t | 原子size_t |
| atomic_ptrdiff_t | _Atomic ptrdiff_t | 原子ptrdiff_t |
| atomic_flag | 特殊类型 | 原子标志位（保证无锁） |

#### 3. 原子操作函数

**基本读写操作**：

| 函数 | 作用 | 示例 |
|------|------|------|
| `atomic_load(&x)` | 原子读取 | `int val = atomic_load(&counter);` |
| `atomic_store(&x, val)` | 原子写入 | `atomic_store(&counter, 0);` |
| `atomic_exchange(&x, val)` | 原子交换 | `int old = atomic_exchange(&counter, 0);` |
| `atomic_compare_exchange_strong(&x, &expected, desired)` | 原子比较并交换（CAS） | 如果x==expected，则x=desired |

**算术操作（仅限整数类型）**：

| 函数 | 作用 | 等价于 |
|------|------|--------|
| `atomic_fetch_add(&x, val)` | 原子加 | 返回旧值，x += val |
| `atomic_fetch_sub(&x, val)` | 原子减 | 返回旧值，x -= val |
| `atomic_fetch_or(&x, val)` | 原子或 | 返回旧值，x \|= val |
| `atomic_fetch_and(&x, val)` | 原子与 | 返回旧值，x &= val |
| `atomic_fetch_xor(&x, val)` | 原子异或 | 返回旧值，x ^= val |

**atomic_flag操作**（最简单的原子类型，保证无锁）：

| 函数 | 作用 |
|------|------|
| `ATOMIC_FLAG_INIT` | 初始化为清除状态 |
| `atomic_flag_test_and_set(&flag)` | 原子地设置并返回旧值 |
| `atomic_flag_clear(&flag)` | 原子地清除 |

#### 4. memory_order枚举

C11定义了6种内存序，控制原子操作的可见性和排序：

| 内存序 | 含义 | 开销 | 说明 |
|--------|------|------|------|
| `memory_order_relaxed` | 无排序约束 | 最低 | 只保证原子性，不保证顺序 |
| `memory_order_acquire` | 获取语义 | 中等 | 后续读写不能重排到此操作之前 |
| `memory_order_release` | 释放语义 | 中等 | 之前的读写不能重排到此操作之后 |
| `memory_order_acq_rel` | 获取+释放 | 较高 | 同时具有acquire和release语义 |
| `memory_order_consume` | 数据依赖获取 | 中等 | 仅保证依赖数据的顺序（极少使用） |
| `memory_order_seq_cst` | 顺序一致性 | 最高 | 全局统一顺序，最安全 |

**内存序选择指南**：

| 场景 | 推荐内存序 | 原因 |
|------|-----------|------|
| 简单计数器 | relaxed | 只需要原子性，不需要同步 |
| 发布/订阅标志 | release/acquire | 确保发布方写入对订阅方可见 |
| 读写锁实现 | acq_rel | 获取锁需要acquire，释放锁需要release |
| 默认选择 | seq_cst | 最安全，不容易出错 |

**带内存序的原子操作**：

```c
atomic_load_explicit(&x, memory_order_acquire);
atomic_store_explicit(&x, val, memory_order_release);
atomic_fetch_add_explicit(&x, 1, memory_order_relaxed);
```

#### 5. 与C++ std::atomic的对比

| 维度 | C11 _Atomic | C++ std::atomic |
|------|-------------|-----------------|
| 声明方式 | `_Atomic int x;` | `std::atomic<int> x;` |
| 头文件 | `<stdatomic.h>` | `<atomic>` |
| 读取 | `atomic_load(&x)` | `x.load()` |
| 写入 | `atomic_store(&x, val)` | `x.store(val)` |
| CAS | `atomic_compare_exchange_strong(&x, &exp, des)` | `x.compare_exchange_strong(exp, des)` |
| 自增 | `atomic_fetch_add(&x, 1)` | `x.fetch_add(1)` 或 `x++` |
| 运算符重载 | 无（只能用函数） | 有（支持`++`、`+=`等） |
| 内存序 | 同样的6种 | 同样的6种 |
| 是否保证无锁 | 不保证（`atomic_is_lock_free`） | 不保证（`is_lock_free`） |
| atomic_flag | 保证无锁 | 保证无锁 |

#### 6. C语言原子计数器示例

```c
#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>

#define THREAD_COUNT 4
#define INCREMENTS_PER_THREAD 1000000

atomic_int counter = 0;

void* increment(void* arg) {
    (void)arg;
    for (int i = 0; i < INCREMENTS_PER_THREAD; i++) {
        atomic_fetch_add(&counter, 1);
    }
    return NULL;
}

int main(void) {
    pthread_t threads[THREAD_COUNT];

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_create(&threads[i], NULL, increment, NULL);
    }

    for (int i = 0; i < THREAD_COUNT; i++) {
        pthread_join(threads[i], NULL);
    }

    printf("期望值: %d\n", THREAD_COUNT * INCREMENTS_PER_THREAD);
    printf("实际值: %d\n", atomic_load(&counter));

    return 0;
}
```

**编译**：

```bash
gcc -std=c11 -pthread atomic_counter.c -o atomic_counter
./atomic_counter
```

**输出**：

```
期望值: 4000000
实际值: 4000000
```

如果不用`atomic_int`而用普通`int`，实际值会远小于期望值（竞态条件）。

#### 7. 用atomic_flag实现自旋锁

```c
#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>

typedef struct {
    atomic_flag flag;
} spinlock_t;

void spinlock_init(spinlock_t* lock) {
    atomic_flag_clear(&lock->flag);
}

void spinlock_lock(spinlock_t* lock) {
    while (atomic_flag_test_and_set(&lock->flag)) {
        /* 自旋等待 */
    }
}

void spinlock_unlock(spinlock_t* lock) {
    atomic_flag_clear(&lock->flag);
}

spinlock_t lock;
int shared_counter = 0;

void* worker(void* arg) {
    (void)arg;
    for (int i = 0; i < 100000; i++) {
        spinlock_lock(&lock);
        shared_counter++;
        spinlock_unlock(&lock);
    }
    return NULL;
}

int main(void) {
    spinlock_init(&lock);

    pthread_t t1, t2;
    pthread_create(&t1, NULL, worker, NULL);
    pthread_create(&t2, NULL, worker, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    printf("结果: %d（期望200000）\n", shared_counter);
    return 0;
}
```

***

### 3. 代码示例：生产者-消费者模式（release/acquire语义）

```c
#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>

#define BUFFER_SIZE 8

typedef struct {
    int data[BUFFER_SIZE];
    atomic_int write_pos;
    atomic_int read_pos;
} ring_buffer_t;

ring_buffer_t buffer;

void buffer_init(ring_buffer_t* buf) {
    for (int i = 0; i < BUFFER_SIZE; i++) {
        buf->data[i] = 0;
    }
    atomic_store(&buf->write_pos, 0);
    atomic_store(&buf->read_pos, 0);
}

int buffer_write(ring_buffer_t* buf, int value) {
    int wp = atomic_load_explicit(&buf->write_pos, memory_order_relaxed);
    int rp = atomic_load_explicit(&buf->read_pos, memory_order_acquire);

    if ((wp + 1) % BUFFER_SIZE == rp) {
        return -1;
    }

    buf->data[wp] = value;
    atomic_store_explicit(&buf->write_pos, (wp + 1) % BUFFER_SIZE,
                          memory_order_release);
    return 0;
}

int buffer_read(ring_buffer_t* buf, int* value) {
    int rp = atomic_load_explicit(&buf->read_pos, memory_order_relaxed);
    int wp = atomic_load_explicit(&buf->write_pos, memory_order_acquire);

    if (rp == wp) {
        return -1;
    }

    *value = buf->data[rp];
    atomic_store_explicit(&buf->read_pos, (rp + 1) % BUFFER_SIZE,
                          memory_order_release);
    return 0;
}

void* producer(void* arg) {
    (void)arg;
    for (int i = 1; i <= 20; i++) {
        while (buffer_write(&buffer, i) != 0) {
            /* 缓冲区满，等待 */
        }
        printf("生产: %d\n", i);
    }
    return NULL;
}

void* consumer(void* arg) {
    (void)arg;
    int value;
    int count = 0;
    while (count < 20) {
        if (buffer_read(&buffer, &value) == 0) {
            printf("消费: %d\n", value);
            count++;
        }
    }
    return NULL;
}

int main(void) {
    buffer_init(&buffer);

    pthread_t prod, cons;
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);

    return 0;
}
```

**编译**：

```bash
gcc -std=c11 -pthread ring_buffer.c -o ring_buffer
```

**关键点**：
- `memory_order_release`确保写入数据后，才更新写位置
- `memory_order_acquire`确保读取写位置后，才读取数据
- 这保证了生产者写入的数据对消费者可见

***

### 4. 常见问题

#### 1. 问题1：_Atomic和volatile有什么区别

volatile防止编译器优化（每次都从内存读取），但不保证原子性。_Atomic既保证原子性，又提供内存序控制。多线程下volatile不能替代_Atomic。

| 维度 | volatile | _Atomic |
|------|----------|---------|
| 原子性 | 不保证 | 保证 |
| 可见性 | 保证（不缓存到寄存器） | 保证（通过内存序） |
| 多线程安全 | 不安全 | 安全 |
| 适用场景 | 硬件寄存器、信号处理 | 多线程共享变量 |

#### 2. 问题2：所有_Atomic类型都是无锁的吗

不一定。`atomic_flag`保证无锁。其他类型是否无锁取决于硬件平台和类型大小，可通过`atomic_is_lock_free(&x)`检查。例如，在大多数平台上`atomic_int`是无锁的，但`atomic_long long`可能不是。

#### 3. 问题3：memory_order_relaxed安全吗

`memory_order_relaxed`只保证单个变量的原子性，不保证操作顺序。适用于不需要同步的场景（如简单计数器）。如果需要"先写数据再设标志"这种顺序关系，必须使用release/acquire。

#### 4. 问题4：C11原子操作和GCC内建原子操作有什么区别

GCC之前提供了`__sync_fetch_and_add`等内建函数，C11的`<stdatomic.h>`是标准化的替代。C11版本支持内存序参数，更灵活。新代码应优先使用C11标准接口。

***

### 5. 极简总结

**C11的_Atomic类型修饰符和stdatomic.h头文件提供了标准化的原子操作支持。核心类型有atomic_int/atomic_flag等，核心操作有atomic_load/atomic_store/atomic_fetch_add等。6种memory_order控制可见性顺序：relaxed最轻量、seq_cst最安全。C++的std::atomic功能等价但接口更友好（支持运算符重载）。多线程共享变量必须使用原子操作或锁。**

| 要点 | 一句话 |
|------|--------|
| _Atomic | C11原子类型修饰符——读写不可分割 |
| stdatomic.h | C11原子操作头文件——提供类型别名和操作函数 |
| atomic_int | _Atomic int的别名——最常用的原子整数类型 |
| atomic_flag | 保证无锁的原子标志位——可实现自旋锁 |
| memory_order | 6种内存序——控制可见性和排序 |
| relaxed | 只保证原子性——简单计数器够用 |
| release/acquire | 发布-获取语义——保证写入对读取可见 |
| seq_cst | 顺序一致性——最安全，默认选择 |

***

### 相关阅读

- [什么是未定义行为](../11-常见错误与陷阱/08-未定义行为大全.md)
- [volatile关键字](18-volatile关键字.md)
- [volatile在Cpp中的状态](18-volatile关键字.md)