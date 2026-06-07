# 双重检查锁(DCL)为什么有风险
> 📖 相关章节：[线程同步](../../02-CPP/28-线程同步.md)、[原子操作与异步编程](../../02-CPP/29-原子操作与异步编程.md)

### 1. 一句话结论

**DCL 有风险，但仍被广泛使用**，原因是 x86 平台上基本不出问题、老代码太多、程序员不知道风险，且加内存屏障后可变安全。

***

### 2. DCL 是什么？

DCL（Double-Checked Locking）双重检查锁，是一种"优化"单例模式的手法：

```c
Singleton* get_instance(void) {
    if (instance == NULL) {              // 第1次检查（不加锁）
        pthread_mutex_lock(&mtx);
        if (instance == NULL) {          // 第2次检查（加锁后）
            instance = create_singleton();
        }
        pthread_mutex_unlock(&mtx);
    }
    return instance;
}
```

**目的**：避免每次调用都加锁，只在第一次创建时加锁。

#### 1. DCL 的执行流程

```
get_instance() 被调用
    │
    ├── 第1次检查：instance == NULL ?
    │       ├── 不为NULL → 直接返回（快速路径，不加锁）
    │       └── 为NULL → 进入加锁流程
    │
    ├── 加锁
    │
    ├── 第2次检查：instance == NULL ?
    │       ├── 不为NULL → 别的线程已创建，解锁返回
    │       └── 为NULL → 创建实例
    │
    ├── 创建实例
    │
    ├── 解锁
    │
    └── 返回 instance
```

#### 2. 为什么需要 DCL？

| 方案 | 每次调用开销 | 线程安全 | 懒加载 |
| --- | --- | --- | --- |
| 不加锁 | 无 | 不安全 | 是 |
| 全函数加锁 | 有（每次加锁/解锁） | 安全 | 是 |
| DCL | 无（创建后不加锁） | 理论上安全 | 是 |
| 饿汉式 | 无 | 安全 | 否 |

**DCL 的吸引力**：既有懒加载，又几乎无性能开销。

***

### 3. 核心风险：CPU/编译器乱序执行

#### 1. 代码顺序 vs 执行顺序

你写的代码顺序：

```c
instance = malloc(...);    // 1. 分配内存
instance->counter = 0;     // 2. 初始化成员
instance->name[0] = '\0';  // 3. 初始化成员
```

**CPU/编译器可能偷偷重排为**：

```c
instance = malloc(...);    // 1. 分配内存
instance = 地址;           // 2. 先赋值给指针！
instance->counter = 0;     // 3. 后初始化
```

#### 2. 灾难场景

| 时序 | 线程A | 线程B |
| --- | --- | --- |
| T1 | 执行 `instance = malloc(...)` | - |
| T2 | 执行 `instance = 地址`（未初始化） | - |
| T3 | - | 检查 `instance != NULL` -> 为真 |
| T4 | - | **直接使用未初始化的对象** -> 崩溃！ |

**根本原因**：线程B在第1次检查时，看到了非NULL的指针，但对象可能还没初始化完成。

#### 3. 为什么会乱序？

| 原因 | 说明 |
| --- | --- |
| 编译器优化 | 编译器为了性能可能重排指令 |
| CPU乱序执行 | 现代CPU的超标量流水线会动态调度 |
| 写缓冲区 | CPU写操作先进入缓冲区，不保证立即对其他核可见 |
| 缓存一致性 | 多核CPU各自有L1/L2缓存，写入传播有延迟 |

***

### 4. 为什么大家还在用 DCL？

#### 1. 原因1：x86/x86_64 平台读/写不乱序

- x86 CPU **不会对读/写操作重排序**（Store-Load 除外，但 DCL 场景中不触发）
- 所以 DCL 在 PC、服务器上 **99% 没问题**

| CPU架构 | 是否允许 Store-Store 重排 | DCL 是否安全 |
| --- | --- | --- |
| x86/x86_64 | 不允许 | 基本安全 |
| ARM | 允许 | 不安全 |
| PowerPC | 允许 | 不安全 |
| MIPS | 允许 | 不安全 |
| RISC-V | 允许 | 不安全 |

#### 2. 原因2：程序员认知盲区

大部分程序员以为：

```
代码顺序 = 执行顺序
```

实际上现代 CPU 和编译器为了性能会乱序执行。

#### 3. 原因3：历史遗留代码

Linux、Apache、Nginx 等很多库几十年前写的 DCL，**不敢轻易改动**。

#### 4. 原因4：C 语言的哲学

C 语言不阻止你写"有风险"的代码：

- 允许野指针、越界访问、空指针解引用
- **相信程序员知道自己在干嘛**

***

### 5. 如何让 DCL 变安全？

#### 1. 方案1：添加内存屏障

```c
static Singleton *instance = NULL;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

Singleton* get_instance(void) {
    if (instance == NULL) {
        pthread_mutex_lock(&mtx);
        if (instance == NULL) {
            Singleton* tmp = (Singleton*)malloc(sizeof(Singleton));
            tmp->counter = 0;
            tmp->name[0] = '\0';
            __sync_synchronize();  // 保证初始化写入对其他核可见
            instance = tmp;         // 屏障之后再写入共享指针
        }
        pthread_mutex_unlock(&mtx);
    }
    return instance;
}
```

**内存屏障的作用**：

| 屏障类型 | 作用 | GCC 内建函数 |
| --- | --- | --- |
| 全屏障 | 阻止所有重排 | `__sync_synchronize()` |
| 写屏障 | 阻止写操作重排 | `__atomic_thread_fence(__ATOMIC_RELEASE)` |
| 读屏障 | 阻止读操作重排 | `__atomic_thread_fence(__ATOMIC_ACQUIRE)` |

> **注意**：`__sync_synchronize()` 始终是全屏障（Full Fence），若需细粒度屏障应使用 C11 的 `atomic_thread_fence` 或 GCC 的 `__atomic_thread_fence`。

#### 2. 方案2：C11 局部静态变量（推荐）

```c
Singleton* get_instance(void) {
    static Singleton instance = {0};  // C11 保证线程安全初始化
    return &instance;
}
```

**优点**：

- 编译器自动加锁、双重检查、内存屏障
- 线程安全
- 极简写法（1行搞定）
- 高性能

#### 3. 方案3：C11 atomic 操作

```c
#include <stdatomic.h>

static _Atomic(Singleton*) instance = NULL;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

Singleton* get_instance(void) {
    Singleton* tmp = atomic_load(&instance);
    if (tmp == NULL) {
        pthread_mutex_lock(&mtx);
        tmp = atomic_load(&instance);
        if (tmp == NULL) {
            tmp = malloc(sizeof(Singleton));
            tmp->counter = 0;
            atomic_store(&instance, tmp);  // 原子写入，自带内存屏障
        }
        pthread_mutex_unlock(&mtx);
    }
    return tmp;
}
```

#### 4. 方案4：C++11 的 std::call_once（C++推荐）

```cpp
#include <mutex>

class Singleton {
    static Singleton* instance;
    static std::once_flag flag;

    Singleton() {}

public:
    static Singleton* get_instance() {
        std::call_once(flag, []() {
            instance = new Singleton();
        });
        return instance;
    }
};

Singleton* Singleton::instance = nullptr;
std::once_flag Singleton::flag;
```

#### 5. 方案5：C++11 的 Meyer's Singleton（C++最推荐）

```cpp
class Singleton {
    Singleton() {}
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;

public:
    static Singleton& get_instance() {
        static Singleton instance;  // C++11 保证线程安全
        return instance;
    }
};
```

***

### 6. 方案对比

| 方案 | 线程安全 | 懒加载 | 性能 | 复杂度 | 风险 |
| --- | --- | --- | --- | --- | --- |
| 普通懒汉（无锁） | 不安全 | 是 | 高 | 低 | 高 |
| 饿汉式 | 安全 | 否 | 高 | 低 | 无 |
| 懒汉式加锁 | 安全 | 是 | 中 | 低 | 无 |
| C11 静态局部 | 安全 | 是 | 高 | 低 | 无 |
| DCL（无屏障） | 有风险 | 是 | 高 | 中 | 中 |
| DCL（有屏障） | 安全 | 是 | 高 | 中 | 无 |
| C11 atomic | 安全 | 是 | 高 | 中 | 无 |
| C++ call_once | 安全 | 是 | 高 | 低 | 无 |
| C++ Meyer's | 安全 | 是 | 高 | 低 | 无 |

***

### 7. C 语言中的 DCL 完整示例

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

typedef struct {
    int counter;
    char name[64];
} Singleton;

// 方式1：全函数加锁（安全首选）
static Singleton* instance_v1 = NULL;
static pthread_mutex_t mutex_v1 = PTHREAD_MUTEX_INITIALIZER;

Singleton* singleton_get_v1(void) {
    pthread_mutex_lock(&mutex_v1);
    if (instance_v1 == NULL) {
        instance_v1 = malloc(sizeof(Singleton));
        instance_v1->counter = 0;
        memset(instance_v1->name, 0, sizeof(instance_v1->name));
    }
    pthread_mutex_unlock(&mutex_v1);
    return instance_v1;
}

// 方式2：DCL + 内存屏障（高性能）
static Singleton* instance_v2 = NULL;
static pthread_mutex_t mutex_v2 = PTHREAD_MUTEX_INITIALIZER;

Singleton* singleton_get_v2(void) {
    Singleton* tmp = instance_v2;
    if (tmp == NULL) {
        pthread_mutex_lock(&mutex_v2);
        tmp = instance_v2;
        if (tmp == NULL) {
            tmp = malloc(sizeof(Singleton));
            tmp->counter = 0;
            memset(tmp->name, 0, sizeof(tmp->name));
            __sync_synchronize();  // 内存屏障
            instance_v2 = tmp;
        }
        pthread_mutex_unlock(&mutex_v2);
    }
    return tmp;
}

// 方式3：C11 静态局部变量（最简洁）
Singleton* singleton_get_v3(void) {
    static Singleton instance_v3 = {0, {0}};
    return &instance_v3;
}

// 测试
void* thread_func(void* arg) {
    int id = *(int*)arg;
    Singleton* s1 = singleton_get_v1();
    Singleton* s2 = singleton_get_v2();
    Singleton* s3 = singleton_get_v3();

    printf("线程%d: v1=%p, v2=%p, v3=%p\n", id, (void*)s1, (void*)s2, (void*)s3);
    return NULL;
}

int main() {
    pthread_t threads[4];
    int ids[4] = {1, 2, 3, 4};

    for (int i = 0; i < 4; i++) {
        pthread_create(&threads[i], NULL, thread_func, &ids[i]);
    }
    for (int i = 0; i < 4; i++) {
        pthread_join(threads[i], NULL);
    }

    return 0;
}
```

***

### 8. Java 的 DCL

Java 的 DCL 问题与 C 语言类似，但 Java 有了更优雅的解决方案：

#### 1. Java DCL（volatile 修复）

```java
public class Singleton {
    private static volatile Singleton instance;  // volatile 防止指令重排

    public static Singleton getInstance() {
        if (instance == null) {              // 第1次检查
            synchronized (Singleton.class) {
                if (instance == null) {      // 第2次检查
                    instance = new Singleton();
                }
            }
        }
        return instance;
    }
}
```

**Java volatile 的作用**：

| 特性 | Java volatile | C volatile |
| --- | --- | --- |
| 可见性 | 保证 | 不保证 |
| 禁止指令重排 | 保证 | 不保证 |
| 原子性 | 不保证（long/double除外） | 不保证 |

**注意**：C 语言的 `volatile` 和 Java 的 `volatile` 完全不同！C 的 `volatile` 只防止编译器优化，不提供内存屏障。

#### 2. Java 推荐方案：枚举单例

```java
public enum Singleton {
    INSTANCE;

    public void doSomething() {
        // ...
    }
}
```

#### 3. 各语言 DCL 对比

| 语言 | DCL 是否安全 | 推荐方案 |
| --- | --- | --- |
| C | 不安全（无屏障时） | C11 静态局部 / 全函数加锁 |
| C++ | 不安全（C++11前） | Meyer's Singleton / call_once |
| Java | volatile 修复后安全 | volatile DCL / 枚举单例 |
| Go | 不安全 | sync.Once |

***

### 9. 全函数加锁单例 vs DCL

#### 1. 全函数加锁单例（安全首选）

```c
Singleton* singleton_get_instance(void) {
    pthread_mutex_lock(&singleton_mutex);  // 一进来就加锁

    if (instance == NULL) {
        instance = malloc(sizeof(Singleton));
        instance->counter = 0;
        instance->name[0] = '\0';
    }

    pthread_mutex_unlock(&singleton_mutex);
    return instance;
}
```

**为什么它绝对安全？**

| 原因 | 说明 |
| --- | --- |
| **锁自带内存屏障** | `pthread_mutex_lock/unlock` 本身自带完整内存屏障 |
| **加锁保证读完成** | 加锁时确保所有读写都先完成 |
| **解锁保证写同步** | 解锁时确保所有修改都同步到主存 |
| **锁内代码不乱序** | 锁内部的代码绝对不会乱序暴露给其他线程 |
| **状态完整可见** | 所有线程看到的 `instance` 状态一定是完整、正确、初始化完成的 |

**结论**：只要是在同一个锁内完成的读写，就绝对安全，不需要额外内存屏障！

#### 2. DCL 为什么有风险？

```c
if (instance == NULL) {  // 外层读不受锁保护！
    pthread_mutex_lock(&mtx);
    if (instance == NULL) {
        instance = malloc(...);
        ...
    }
    pthread_mutex_unlock(&mtx);
}
```

**风险点**：外层读不在锁内，可能读到**半初始化、乱序、不完整**的对象。

#### 3. 方案对比

| 方案 | 安全性 | 性能 | 复杂度 | 是否需要屏障 |
| --- | --- | --- | --- | --- |
| **全函数加锁** | 绝对安全 | 稍慢（每次加锁） | 低 | 不需要 |
| **DCL（无屏障）** | 有风险 | 快 | 中 | 需要 |
| **DCL（有屏障）** | 安全 | 快 | 中 | 需要 |
| **C11 静态局部** | 安全 | 快 | 低 | 不需要 |

#### 4. 选型建议

- **追求绝对安全、简单** -> 全函数加锁或 C11 静态局部
- **追求极致性能** -> DCL + 内存屏障（需熟悉内存模型）
- **新手、团队协作** -> 优先全函数加锁或 C11 静态局部

***

### 10. 常见陷阱与最佳实践

#### 1. 常见陷阱

| 陷阱 | 后果 | 修复 |
| --- | --- | --- |
| DCL 不加内存屏障 | ARM/PowerPC上读到半初始化对象 | 加 `__sync_synchronize()` |
| 用 C 的 volatile 替代内存屏障 | 无效，C volatile 不提供内存屏障 | 用 `__sync_synchronize` 或 atomic |
| DCL 中只检查一次 | 失去DCL意义或引入竞争 | 必须双重检查 |
| 忘记锁内第二次检查 | 多线程可能同时创建实例 | 锁内必须再检查一次 |

#### 2. 最佳实践

1. **C 语言**：优先用 C11 静态局部变量，简单安全
2. **C++**：优先用 Meyer's Singleton 或 `std::call_once`
3. **必须用 DCL**：一定要加内存屏障或用 atomic
4. **不要用 C volatile**：它不提供内存屏障，不能替代 atomic
5. **跨平台代码**：必须考虑 ARM 等弱内存序架构

***

### 11. 极简总结

1. **DCL 风险根源**：CPU/编译器乱序执行，导致指针先赋值、对象后初始化
2. **x86 平台特殊**：读/写不乱序，所以 DCL 在 x86 上基本安全
3. **老代码兼容**：历史遗留代码多，不敢改动
4. **正确做法**：要么加内存屏障，要么用 C11 静态局部变量
5. **C 语言哲学**：不禁止危险代码，只提供能力
6. **C volatile 不等于 Java volatile**：C 的 volatile 不提供内存屏障

***

### 相关阅读

- [内存屏障与乱序执行](21-内存屏障与乱序执行.md)
- [原子操作与原子变量](20-原子操作与原子变量.md)
- [volatile关键字](../01-基础概念/18-volatile关键字.md)

***