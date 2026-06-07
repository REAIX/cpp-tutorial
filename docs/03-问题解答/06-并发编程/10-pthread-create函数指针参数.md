# pthread_create 函数指针参数详解
> 📖 相关章节：[多线程基础](../../02-CPP/27-多线程基础.md)、[线程同步](../../02-CPP/28-线程同步.md)、[原子操作](../../02-CPP/29-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

### 1. 一句话结论

**可以传函数指针，但必须符合固定签名；不匹配的函数需要套一层包装。**

***

### 2. 核心规矩

`pthread_create` 第三个参数本质是：
**传一个指定格式的「函数指针」**

它的类型原型：

```c
void *(*start_routine)(void *)
```

**翻译成人话**：

- 接收 1 个参数：`void*`
- 返回值：`void*`

**只要函数签名对上，就能传；对不上，不能直接传。**

***

### 3. void*(*) (void*) 签名解析

这是 C 语言中最容易让人困惑的类型声明之一。我们逐步拆解：

#### 1. 从内到外阅读

```c
void * (* start_routine) (void *)
 │      │       │           │
 │      │       │           └── 参数类型：void*（万能指针）
 │      │       └────────────── 变量名：函数指针名
 │      └────────────────────── 指针（*表示这是函数指针）
 └─────────────────────────── 返回值类型：void*（万能指针）
```

#### 2. 用 typedef 简化

```c
// 定义函数指针类型
typedef void* (*ThreadFunc)(void*);

// 使用
ThreadFunc func = my_worker;
pthread_create(&tid, NULL, func, NULL);
```

#### 3. 等价写法对比

| 写法 | 含义 |
| --- | --- |
| `void* (*f)(void*)` | 函数指针变量 f |
| `typedef void* (*ThreadFunc)(void*)` | 函数指针类型 ThreadFunc |
| `void* func(void*)` | 普通函数（可隐式转为函数指针） |

**关键点**：函数名本身就是地址，`worker` 和 `&worker` 是等价的。

```c
// 以下两种写法完全等价
pthread_create(&tid, NULL, worker, NULL);
pthread_create(&tid, NULL, &worker, NULL);
```

***

### 4. 三种情况拆解

#### 1. 情况1：普通函数指针（符合签名）

**可以直接传**

```c
void* func(void* arg);

// 直接传，完全合法
pthread_create(&tid, NULL, func, NULL);
```

这是最标准、最安全的用法，签名完全匹配，不需要任何转换。

#### 2. 情况2：信号处理函数

**不能直接传**

信号函数标准签名：

```c
void sig_func(int sig);
```

和线程函数签名**对不上**：

| 维度 | 线程函数 | 信号函数 |
| --- | --- | --- |
| 返回值 | `void*` | `void` |
| 参数 | `void*` | `int` |
| 参数个数 | 1 | 1 |

强行强转编译能过，但**运行必崩、栈乱、寄存器乱**，绝对不能干。

```c
// 危险！绝对不要这样做！
pthread_create(&tid, NULL, (void*(*)(void*))sig_func, NULL);
// 调用时栈帧不匹配，返回值处理错误，直接段错误
```

#### 3. 情况3：任意函数（签名不匹配）

**不可以随便乱传**

比如这些都不行：

```c
void f1();           // 无参无返回
void f2(int a);      // 有int参数
int  f3(void* a);    // 返回int
void f4(int a, int b); // 两个参数
```

签名不匹配，强行塞进去：

- 编译可能警告/强转能过
- **运行栈失衡、调用约定错乱、直接段错误**

#### 4. 签名匹配规则

| 函数签名 | 能否直接传 | 原因 |
| --- | --- | --- |
| `void* f(void*)` | 可以 | 完全匹配 |
| `void f(void*)` | 不可以 | 返回值类型不同 |
| `void* f()` | 不可以 | 参数列表不同（C中()表示任意参数） |
| `int f(void*)` | 不可以 | 返回值类型不同 |
| `void* f(int*)` | 不可以 | 参数类型不同 |

***

### 5. 包装函数解决方案

想用任意格式函数？包一层**包装壳**就行：

#### 1. 基本包装

```c
// 你自己的函数（任意格式）
void my_task(int x) {
    printf("任务执行：%d\n", x);
}

// 包装层：符合线程标准签名
void* wrapper(void* arg) {
    int x = *(int*)arg;
    my_task(x);   // 调用你自己任意格式的函数
    return NULL;
}

// 使用
int data = 100;
pthread_create(&tid, NULL, wrapper, &data);
```

#### 2. 包装带返回值的函数

```c
// 你自己的函数（有返回值）
int compute(int a, int b) {
    return a + b;
}

// 包装层
typedef struct {
    int a;
    int b;
    int result;
} ComputeArg;

void* compute_wrapper(void* arg) {
    ComputeArg* ca = (ComputeArg*)arg;
    ca->result = compute(ca->a, ca->b);
    return NULL;
}

// 使用
ComputeArg ca = {3, 4, 0};
pthread_create(&tid, NULL, compute_wrapper, &ca);
pthread_join(tid, NULL);
printf("结果: %d\n", ca.result);
```

#### 3. 通用包装模板

```c
// 通用包装：可以包装任意函数
typedef struct {
    void (*func)(void*);  // 被包装的函数指针
    void* user_data;      // 传给被包装函数的参数
} WrapperArg;

void* generic_wrapper(void* arg) {
    WrapperArg* wa = (WrapperArg*)arg;
    wa->func(wa->user_data);  // 调用实际函数
    free(wa);
    return NULL;
}

// 使用
void my_func(void* data) {
    printf("执行自定义函数\n");
}

WrapperArg* wa = malloc(sizeof(WrapperArg));
wa->func = (void(*)(void*))my_func;
wa->user_data = NULL;
pthread_create(&tid, NULL, generic_wrapper, wa);
```

***

### 6. 不同类型函数指针的转换

#### 1. 为什么不能直接强转？

C语言的函数调用约定（calling convention）要求：

1. **参数压栈顺序**由调用约定决定
2. **返回值存放位置**由返回类型决定
3. **栈清理责任**由调用约定决定

签名不匹配时，这些全部对不上：

```c
// 假设线程函数期望 void* f(void*)
// 但你传了 void f(int)

// 调用时底层发生的事：
// 1. pthread 库按 void* 参数压栈
// 2. 按 void* 返回值从寄存器取结果
// 3. 但实际函数期望 int 参数，返回 void
// 4. 参数解析错误 + 返回值读取错误 = 灾难
```

#### 2. 强转的风险等级

| 强转方式 | 风险 | 后果 |
| --- | --- | --- |
| `void*(*)(void*)` -> `void(*)(void*)` | 高 | 返回值读取错误 |
| `void*(*)(void*)` -> `void*(*)(int*)` | 高 | 参数类型不匹配 |
| `void*(*)(void*)` -> `int(*)(void*)` | 中 | 返回值截断 |
| 相同签名不同命名 | 无 | 安全 |

**结论**：永远不要强转函数指针来适配 `pthread_create`，用包装函数。

***

### 7. C++ 成员函数与线程

C++ 的非静态成员函数有隐藏的 `this` 指针参数，签名和 `void*(void*)` 不匹配。

#### 1. 错误做法

```cpp
class Worker {
public:
    void* run(void* arg) {
        // 隐藏参数：this 指针
        // 实际签名：void* run(Worker* this, void* arg)
        return NULL;
    }
};

// 错误！成员函数不是普通函数指针
pthread_create(&tid, NULL, &Worker::run, NULL);  // 编译错误
```

#### 2. 正确做法1：静态成员函数包装

```cpp
class Worker {
public:
    static void* thread_entry(void* arg) {
        Worker* self = (Worker*)arg;
        return self->run();  // 调用非静态成员函数
    }

    void* run() {
        printf("成员函数在线程中执行\n");
        return NULL;
    }
};

Worker w;
pthread_create(&tid, NULL, Worker::thread_entry, &w);
```

#### 3. 正确做法2：C++11 std::thread（推荐）

```cpp
#include <thread>

class Worker {
public:
    void run() {
        printf("成员函数在线程中执行\n");
    }
};

Worker w;
std::thread t(&Worker::run, &w);  // 自动处理 this 指针
t.join();
```

***

### 8. Lambda 与线程（C++）

C++11 的 lambda 可以直接用于 `std::thread`，也可以用于 `pthread_create`：

#### 1. 用 std::thread + lambda

```cpp
#include <thread>
#include <cstdio>

int main() {
    int value = 42;

    // lambda 直接传给 std::thread
    std::thread t([&value]() {
        printf("线程中: value=%d\n", value);
    });

    t.join();
    return 0;
}
```

#### 2. 用 pthread_create + lambda

```cpp
#include <pthread.h>
#include <cstdio>

int main() {
    int value = 42;

    // lambda 必须是无捕获的才能转为函数指针
    // 有捕获的 lambda 需要包装
    auto lambda_with_capture = [&value]() {
        printf("线程中: value=%d\n", value);
    };

    // 包装：把 lambda 放到堆上，通过 void* 传递
    auto wrapper = [](void* arg) -> void* {
        auto* func = static_cast<decltype(&lambda_with_capture)>(arg);
        (*func)();
        return nullptr;
    };

    // 注意：这里用到了 lambda 的地址，lambda 必须在堆上或生命周期足够长
    pthread_t tid;
    pthread_create(&tid, nullptr, wrapper, &lambda_with_capture);
    pthread_join(tid, nullptr);

    return 0;
}
```

#### 3. 无捕获 lambda 直接转换

```cpp
// 无捕获的 lambda 可以隐式转为函数指针
auto no_capture = [](void* arg) -> void* {
    printf("无捕获lambda\n");
    return nullptr;
};

// 直接传！因为无捕获lambda可以转为 void*(*)(void*)
pthread_create(&tid, nullptr, no_capture, nullptr);
```

| Lambda 类型 | 能否直接传 pthread_create | 原因 |
| --- | --- | --- |
| 无捕获 | 可以 | 可转为函数指针 |
| 有捕获 | 不可以 | 闭包状态无法通过函数指针传递 |
| 有捕获（包装后） | 可以 | 通过 void* 传递闭包对象 |

***

### 9. 包装任意函数的标准模板

```c
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

// 自定义数据结构（传递多个参数）
typedef struct {
    int arg1;
    const char* arg2;
} MyArgs;

// 你自己的任意函数
void my_custom_function(int a, const char* b) {
    printf("收到参数: a=%d, b=%s\n", a, b);
}

// 包装函数
void* thread_wrapper(void* arg) {
    MyArgs* args = (MyArgs*)arg;
    my_custom_function(args->arg1, args->arg2);
    free(args);
    return NULL;
}

int main() {
    pthread_t tid;

    // 准备参数
    MyArgs* args = malloc(sizeof(MyArgs));
    args->arg1 = 42;
    args->arg2 = "hello";

    // 创建线程
    pthread_create(&tid, NULL, thread_wrapper, args);
    pthread_join(tid, NULL);

    return 0;
}
```

***

### 10. 底层原理

- `pthread_create` 内部按 **固定函数调用栈布局** 跳转到任务函数
- 必须严格：入参 `void*`、返回 `void*`
- 乱传别的函数，**栈帧对不上、寄存器不匹配**，底层执行逻辑直接崩坏

#### 1. 调用过程时序

```
pthread_create 内部:
    │
    ├── 保存 start_routine 函数指针
    ├── 保存 arg 参数指针
    │
    ▼ 新线程开始执行
    │
    ├── 设置新线程的 PC 寄存器 = start_routine 地址
    ├── 设置新线程的参数寄存器 = arg
    ├── 按 void* (*)(void*) 的调用约定调用
    │
    ▼ 函数执行
    │
    ├── 如果签名匹配：正常执行，返回值存入正确寄存器
    └── 如果签名不匹配：参数/返回值错位，栈帧混乱，崩溃
```

***

### 11. 常见陷阱与最佳实践

#### 1. 常见陷阱

| 陷阱 | 后果 | 修复 |
| --- | --- | --- |
| 强转不匹配的函数指针 | 运行时崩溃 | 用包装函数 |
| C++非静态成员函数直接传 | 编译错误 | 用静态成员函数包装 |
| 有捕获lambda直接传pthread | 编译错误 | 用 void* 传递闭包对象 |
| 忘记包装函数中 free 参数 | 内存泄漏 | 包装函数中释放 |
| 包装函数返回栈上指针 | 返回值失效 | 用 malloc 分配返回值 |

#### 2. 最佳实践

1. **永远不要强转函数指针**：签名不匹配就用包装函数
2. **C++优先用 std::thread**：自动处理成员函数和lambda
3. **包装函数只做转发**：不要在包装函数里写业务逻辑
4. **包装函数负责释放参数**：保持"谁分配谁释放"原则
5. **无捕获lambda可以直接用**：最简洁的C++线程写法

***

### 12. 一句话总结

1. 第三个参数**本质就是函数指针**
2. 只能传 **签名 = `void* xxx(void*)`** 的函数
3. 信号函数、普通自定义函数**不能直接传**
4. 想用任意格式函数 -> 写一层**包装函数**中转
5. C++ 成员函数用静态包装，lambda 用无捕获或闭包传递

***

### 相关阅读

- [pthread-create参数与任务函数](09-pthread-create参数与任务函数.md)
- [std-function与函数指针与Lambda](27-std-function与函数指针与Lambda.md)
- [多线程函数接口](24-多线程函数接口.md)

***