# pthread_create 参数详解与任务函数设计
> 📖 相关章节：[多线程基础](../../02-CPP/29-多线程基础.md)、[线程同步](../../02-CPP/30-线程同步.md)、[原子操作](../../02-CPP/31-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

### 1. 一句话概括

**`pthread_create` = 招一个新员工，告诉他：去哪上班（任务函数）、带什么工具（参数）。**

***

### 2. 函数原型（记 4 个位置）

```c
int pthread_create(
    pthread_t *thread,              // 参数1：输出线程ID
    const pthread_attr_t *attr,     // 参数2：线程属性
    void *(*start_routine)(void *), // 参数3：线程入口函数
    void *arg                       // 参数4：传给入口函数的参数
);
```

中文翻译版：

```c
pthread_create(
    线程ID地址,    // 输出：新员工工号
    线程属性,      // 输入：员工特殊要求（默认填 NULL）
    任务函数,      // 输入：新员工去哪个办公室干活
    函数参数       // 输入：给员工带什么工具/数据
);
```

***

### 3. 4 个参数逐行翻译

#### 1. 参数1：`&tid` -> 输出：线程ID（工号）

- 作用：创建线程后，系统把**新线程编号**写到这个变量里
- 传变量地址进去，内核帮你填好ID
- 后面用这个ID：等待线程（join）、取消线程（cancel）等
- 类型：`pthread_t*`，本质是一个无符号长整型的指针

**理解**：你给系统一张空白纸，系统写完**新员工工号**还给你。

```c
pthread_t tid;  // 声明一个变量来接收线程ID
pthread_create(&tid, NULL, worker, NULL);
printf("新线程ID: %lu\n", (unsigned long)tid);
```

#### 2. 参数2：`NULL` -> 线程属性（默认配置）

- 栈大小、优先级、分离状态等
- 99% 场景直接写 **NULL**
- 意思：**按系统默认配置创建线程**

**理解**：普通员工，不用特殊待遇 -> NULL

| 属性 | NULL时的默认值 |
| --- | --- |
| 栈大小 | 8MB |
| 分离状态 | JOINABLE（需要join回收） |
| 调度策略 | SCHED_OTHER |
| 优先级 | 0 |
| 继承调度 | 继承父线程 |

#### 3. 参数3：`worker` -> 最重要：线程要跑的函数

- 线程启动后，**第一个执行的就是这个函数**
- 这个函数就是**线程的主逻辑**
- **必须严格按规定格式写**！
- 函数指针类型：`void* (*)(void*)`

**理解**：员工上岗后，**第一件事就是跑这个函数**。

#### 4. 参数4：`NULL` -> 传给 worker 的参数

- 想给线程传什么数据，就传什么
- 可以是 int、结构体、字符串、任何指针
- 不想传就写 **NULL**
- 类型：`void*`，万能指针，什么都能传

**理解**：给员工带的**工具包、资料、参数**。

***

### 4. 任务函数必须长这样

```c
void* worker(void* arg)
{
    // 线程所有代码写这里
    // ...

    return NULL; // 必须返回一个 void*
}
```

**为什么必须长这样？**

因为 `pthread_create` 规定：

> *线程函数必须接收一个 void* 参数，返回一个 void*。*

就像：**新员工必须穿统一制服上岗**。

#### 1. 签名解析

```c
void* worker(void* arg)
//  ↑        ↑       ↑
//  |        |       |
// 返回值   函数名   参数
// void*    自定义   void*（万能指针）
```

| 部分 | 类型 | 含义 |
| --- | --- | --- |
| 返回值 | `void*` | 线程的退出结果，可被 `pthread_join` 获取 |
| 函数名 | 自定义 | 你自己起的名字 |
| 参数 | `void*` | 万能指针，可以指向任何类型的数据 |

#### 2. 返回值的用途

```c
void* worker(void* arg) {
    int* result = malloc(sizeof(int));
    *result = 42;
    return (void*)result;  // 返回结果
}

// 主线程获取返回值
void* retval;
pthread_join(tid, &retval);
printf("线程返回: %d\n", *(int*)retval);
free(retval);  // 记得释放
```

***

### 5. 参数传递方式

#### 1. 方式1：不传参数（NULL）

最简单，线程不需要外部数据。

```c
void* worker(void* arg) {
    // 不使用 arg
    printf("我是子线程\n");
    return NULL;
}

pthread_create(&tid, NULL, worker, NULL);
```

#### 2. 方式2：值传递（传整数）

把整数值强转为 `void*` 传入，不需要分配内存。

```c
void* worker(void* arg) {
    // 方式A：把 void* 当作整数用（仅适用于小整数）
    int num = (int)(intptr_t)arg;
    printf("线程收到数字：%d\n", num);
    return NULL;
}

// 使用：直接把整数强转为 void*
int value = 100;
pthread_create(&tid, NULL, worker, (void*)(intptr_t)value);
```

**注意**：这种方式只适用于小整数（指针能容纳的范围），且不需要解引用。

#### 3. 方式3：指针传递（传变量的地址）

```c
void* worker(void* arg) {
    int* p = (int*)arg;
    printf("线程收到数字：%d\n", *p);
    return NULL;
}

int data = 100;
pthread_create(&tid, NULL, worker, &data);
// 危险！如果主线程修改 data 或 data 是局部变量，线程可能读到错误值
```

**陷阱**：传局部变量地址，函数返回后地址失效！

#### 4. 方式4：堆内存传递（最安全）

```c
void* worker(void* arg) {
    int* p = (int*)arg;
    printf("线程收到数字：%d\n", *p);
    free(p);  // 线程负责释放
    return NULL;
}

int* data = malloc(sizeof(int));
*data = 100;
pthread_create(&tid, NULL, worker, data);
// data 的所有权转移给子线程，主线程不再使用
```

#### 5. 四种传递方式对比

| 方式 | 内存分配 | 安全性 | 适用场景 | 注意事项 |
| --- | --- | --- | --- | --- |
| NULL | 无 | 最安全 | 不需要参数 | - |
| 值强转 | 无 | 安全 | 传递小整数 | 指针大小限制 |
| 传地址 | 无 | 危险 | 仅限全局/静态变量 | 局部变量会失效 |
| 堆内存 | malloc | 安全 | 通用场景 | 线程内必须free |

***

### 6. 结构体传递（传递多个数据）

**最工程化、最实用！** 当需要传递多个参数时，打包成结构体。

```c
typedef struct {
    int id;
    char name[32];
    double score;
} Task;

void* worker(void* arg) {
    Task* task = (Task*)arg;

    printf("id=%d, name=%s, score=%.1f\n",
           task->id, task->name, task->score);

    free(task); // 堆分配记得释放
    return NULL;
}

// 创建线程 + 传任务
Task* t = malloc(sizeof(Task));
t->id = 1;
strcpy(t->name, "测试任务");
t->score = 95.5;

pthread_create(&tid, NULL, worker, t);
```

#### 1. 结构体设计的最佳实践

```c
// 推荐的结构体设计
typedef struct {
    int      thread_id;       // 线程编号
    void*    input_data;      // 输入数据指针
    int      input_size;      // 输入数据大小
    void*    output_data;     // 输出数据指针（线程填充）
    int      output_size;     // 输出数据大小
    pthread_mutex_t* mutex;   // 共享锁（如果需要）
} ThreadArg;
```

**设计原则**：

1. 所有必要数据都放进结构体，不要依赖全局变量
2. 结构体通过 `malloc` 分配，线程用完 `free`
3. 明确"所有权"：谁分配谁释放
4. 如果有输出，预留输出字段

***

### 7. 多参数传递技巧

#### 1. 技巧1：结构体打包（推荐）

```c
typedef struct {
    int a;
    double b;
    const char* c;
} MultiArg;

void* worker(void* arg) {
    MultiArg* ma = (MultiArg*)arg;
    printf("a=%d, b=%.2f, c=%s\n", ma->a, ma->b, ma->c);
    free(ma);
    return NULL;
}

MultiArg* ma = malloc(sizeof(MultiArg));
ma->a = 1;
ma->b = 3.14;
ma->c = "hello";
pthread_create(&tid, NULL, worker, ma);
```

#### 2. 技巧2：数组传递（批量数据）

```c
typedef struct {
    int* data;       // 数据数组
    int  count;      // 数据个数
    int  thread_id;  // 线程编号
} ArrayArg;

void* sum_worker(void* arg) {
    ArrayArg* aa = (ArrayArg*)arg;
    long long sum = 0;
    for (int i = 0; i < aa->count; i++) {
        sum += aa->data[i];
    }
    printf("线程%d: sum=%lld\n", aa->thread_id, sum);
    free(aa);
    return NULL;
}

int data[] = {1, 2, 3, 4, 5, 6, 7, 8};
ArrayArg* aa = malloc(sizeof(ArrayArg));
aa->data = data;      // 数组本身不需要malloc，全局/栈上即可
aa->count = 8;
aa->thread_id = 1;
pthread_create(&tid, NULL, sum_worker, aa);
```

#### 3. 技巧3：分段处理（大数据分片）

```c
typedef struct {
    int* data;       // 数据数组指针
    int  start;      // 起始索引
    int  end;        // 结束索引
    long long* result; // 结果存放位置
} SliceArg;

void* slice_sum(void* arg) {
    SliceArg* sa = (SliceArg*)arg;
    long long sum = 0;
    for (int i = sa->start; i < sa->end; i++) {
        sum += sa->data[i];
    }
    *(sa->result) = sum;
    free(sa);
    return NULL;
}
```

***

### 8. 线程参数的生命周期问题

这是多线程编程中最容易出bug的地方！

#### 1. 问题1：局部变量失效

```c
// 错误示例
void create_thread_wrong() {
    int value = 42;
    pthread_create(&tid, NULL, worker, &value);
    // 函数返回后，value 的栈空间被回收
    // 线程读到的可能是垃圾值
}
```

**修复**：用堆内存。

```c
void create_thread_right() {
    int* value = malloc(sizeof(int));
    *value = 42;
    pthread_create(&tid, NULL, worker, value);
    // 线程内负责 free(value)
}
```

#### 2. 问题2：循环中传同一个变量

```c
// 错误示例
for (int i = 0; i < 5; i++) {
    pthread_create(&tids[i], NULL, worker, &i);
    // 所有线程可能都读到 i=5（循环结束后i的值）
}
```

**修复**：每次循环分配独立的堆内存。

```c
for (int i = 0; i < 5; i++) {
    int* p = malloc(sizeof(int));
    *p = i;
    pthread_create(&tids[i], NULL, worker, p);
}
```

#### 3. 问题3：字符串参数失效

```c
// 错误示例
void create_thread_wrong() {
    char buf[64];
    sprintf(buf, "task_%d", id);
    pthread_create(&tid, NULL, worker, buf);
    // buf 是局部变量，函数返回后失效
}
```

**修复**：用 `strdup` 复制字符串。

```c
char* task_name = strdup(buf);  // 堆上复制一份
pthread_create(&tid, NULL, worker, task_name);
// 线程内 free(task_name)
```

#### 4. 生命周期规则总结

| 数据来源 | 是否安全 | 原因 |
| --- | --- | --- |
| 全局变量 | 安全 | 生命周期贯穿整个进程 |
| 静态变量 | 安全 | 生命周期贯穿整个进程 |
| 堆内存（malloc） | 安全 | 显式free才释放 |
| 局部变量 | 危险 | 函数返回后失效 |
| 字符串字面量 | 安全 | 存储在只读数据段 |
| 栈上数组 | 危险 | 函数返回后失效 |

***

### 9. 完整可运行模板

```c
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int thread_id;
    const char* message;
} ThreadData;

void* worker(void* arg) {
    ThreadData* data = (ThreadData*)arg;

    printf("线程 %d 收到消息: %s\n", data->thread_id, data->message);

    // 线程工作...
    for (int i = 0; i < 3; i++) {
        printf("线程 %d 工作中... %d\n", data->thread_id, i);
    }

    // 返回值可以被 pthread_join 获取
    int* result = malloc(sizeof(int));
    *result = data->thread_id * 10;

    free(data);
    return (void*)result;
}

int main() {
    pthread_t tid1, tid2;

    // 创建第一个线程
    ThreadData* data1 = malloc(sizeof(ThreadData));
    data1->thread_id = 1;
    data1->message = "Hello from Thread 1";
    pthread_create(&tid1, NULL, worker, data1);

    // 创建第二个线程
    ThreadData* data2 = malloc(sizeof(ThreadData));
    data2->thread_id = 2;
    data2->message = "Hello from Thread 2";
    pthread_create(&tid2, NULL, worker, data2);

    // 等待线程结束并获取返回值
    void* ret1;
    pthread_join(tid1, &ret1);
    printf("线程1返回: %d\n", *(int*)ret1);
    free(ret1);

    void* ret2;
    pthread_join(tid2, &ret2);
    printf("线程2返回: %d\n", *(int*)ret2);
    free(ret2);

    return 0;
}
```

***

### 10. 常见陷阱与最佳实践

#### 1. 常见陷阱

| 陷阱 | 后果 | 修复 |
| --- | --- | --- |
| 传局部变量地址 | 读到垃圾值或崩溃 | 用 malloc 分配堆内存 |
| 循环中传 &i | 所有线程读到同一个值 | 每次循环独立 malloc |
| 线程不释放参数内存 | 内存泄漏 | 线程内 free(arg) |
| 返回值指向栈内存 | join 后读到垃圾值 | 返回值用 malloc 分配 |
| 不检查返回值 | 创建失败后操作无效tid | 检查 pthread_create 返回值 |

#### 2. 最佳实践

1. **参数一律用 malloc 分配**：避免所有生命周期问题
2. **明确所有权**：谁分配谁释放，通常"主线程分配，子线程释放"
3. **结构体打包参数**：不要传多个零散指针
4. **返回值也用 malloc**：栈上的返回值在 join 时可能已失效
5. **线程函数开头就转换类型**：避免后续代码忘记转换

```c
// 推荐的线程函数模板
void* worker(void* arg) {
    // 第一步：立即转换类型
    MyArg* my_arg = (MyArg*)arg;

    // 第二步：做工作
    do_something(my_arg);

    // 第三步：释放参数
    free(my_arg);

    // 第四步：返回结果（堆分配）
    int* result = malloc(sizeof(int));
    *result = 0;
    return (void*)result;
}
```

***

### 11. 极简口诀

```
创建线程四参数：
ID、属性、任务函数、带参数
属性默认写 NULL
函数格式固定死：void* func(void*)

参数传递三原则：
1. 局部变量不能传（会失效）
2. 堆内存最安全（malloc + free）
3. 所有权要明确（谁分配谁释放）
```

***

### 相关阅读

- [pthread-create底层原理](./01-pthread-create底层原理.md)
- [pthread-create函数指针参数](./03-pthread-create函数指针参数.md)
- [多线程通讯](./11-多线程通讯.md)

***