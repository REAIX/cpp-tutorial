# pthread_create 底层原理
> 📖 相关章节：[进程与线程](../../01-C语言/23-进程与线程.md)、[多线程基础](../../02-CPP/27-多线程基础.md)

### 1. 一句话概括

**`pthread_create` 一行代码背后，是操作系统内核完成的：分配栈内存、创建线程控制块、复用进程资源、入调度队列、上下文切换的完整流程。**

***

### 2. 为什么代码看着就一行？

```c
pthread_create(&tid, NULL, worker, NULL);
```

**原因就 3 句：**

1. **库函数把所有复杂底层操作全部封装藏起来了**
2. 真正干活的是 **操作系统内核**，不是你写的代码
3. CPU 创建、栈分配、寄存器上下文、调度队列入队，**全是内核默默秒级搞定**

你只调用一个库接口，脏活累活全被 **glibc + Linux内核** 替你干完了。

**生活类比**：就像你**按一下电梯按钮**就一行动作，但背后电机、控制板、钢丝绳、门锁、传感器忙得要死，**你完全无感**。

***

### 3. 一行代码背后，内核偷偷干了十几件大事

#### 1. 分配线程私有栈内存

每个线程都要有**私有栈**：

- 一般默认 8MB 栈
- 内核帮你虚拟内存映射、分配页表
- 你**完全看不到**，但内存实实在在划出去了

#### 2. 生成线程控制块 TCB

内核里每个线程都有一个 `task_struct`，存：

- 线程ID
- 寄存器上下文（eax/ebp/程序计数器PC）
- 状态：就绪、运行、阻塞、睡眠
- 调度优先级、时间片统计

这是线程的**身份证+档案**。

#### 3. 复用进程全部资源

新线程不重新创建进程：

- 复用同一个虚拟地址空间
- 复用全局变量、堆、文件描述符、打开的文件

所以**创建线程极快**，不用像进程那样重新拷贝整个地址空间。

#### 4. 入调度就绪队列

把新线程丢进 **CPU 调度就绪队列**，告诉调度器：

> "这个线程准备好了，有空就给它分配CPU时间片"

#### 5. 上下文切换

内核瞬间做一次 **上下文切换**：

- 保存主线程寄存器 -> 加载子线程寄存器 -> 跳到 `worker` 函数执行
- **全程微秒级**，你代码层面完全感知不到

#### 6. 子线程开始执行

```c
void* worker(void *arg)
{
    // 这已经是新线程在CPU上跑了
}
```

***

### 4. pthread_create 的4个参数详解

```c
int pthread_create(
    pthread_t *thread,              // 参数1：输出线程ID
    const pthread_attr_t *attr,     // 参数2：线程属性
    void *(*start_routine)(void *), // 参数3：线程入口函数
    void *arg                       // 参数4：传给入口函数的参数
);
```

| 参数 | 类型 | 方向 | 说明 |
| --- | --- | --- | --- |
| `thread` | `pthread_t*` | 输出 | 创建成功后，新线程的ID写入此变量 |
| `attr` | `const pthread_attr_t*` | 输入 | 线程属性，NULL表示默认 |
| `start_routine` | `void*(*)(void*)` | 输入 | 线程启动后执行的函数 |
| `arg` | `void*` | 输入 | 传给 start_routine 的参数 |

**返回值**：

| 返回值 | 含义 |
| --- | --- |
| 0 | 创建成功 |
| EAGAIN | 系统资源不足（线程数超限） |
| EINVAL | 属性参数无效 |
| EPERM | 没有设置调度参数的权限 |

***

### 5. 线程创建过程：系统调用 clone

Linux 下 `pthread_create` 最终调用的是 `clone()` 系统调用：

#### 1. clone 系统调用的关键标志

```c
clone(child_stack, flags, ...)
```

| 标志 | 含义 |
| --- | --- |
| `CLONE_VM` | 共享虚拟地址空间（线程最核心的标志） |
| `CLONE_FS` | 共享文件系统信息 |
| `CLONE_FILES` | 共享文件描述符表 |
| `CLONE_SIGHAND` | 共享信号处理函数 |
| `CLONE_THREAD` | 放入同一线程组 |

**对比 fork()**：`fork()` 不带 `CLONE_VM`，所以子进程有独立的地址空间拷贝。

#### 2. 创建流程时序

```
用户态: pthread_create()
    │
    ├── 1. 分配线程栈（mmap）
    ├── 2. 初始化线程描述符
    ├── 3. 设置线程入口函数和参数
    │
    ▼ 系统调用
内核态: clone()
    │
    ├── 4. 创建新的 task_struct
    ├── 5. 复制/共享进程资源（按flags）
    ├── 6. 分配线程PID
    ├── 7. 初始化寄存器上下文
    ├── 8. 加入调度就绪队列
    │
    ▼ 返回用户态
    ├── 9. 主线程继续执行（返回0）
    └── 10. 新线程跳转到 start_routine 执行
```

***

### 6. 线程属性设置

默认传 NULL 使用系统默认属性，但可以通过 `pthread_attr_t` 自定义：

#### 1. 常用属性

| 属性 | 默认值 | 说明 |
| --- | --- | --- |
| 栈大小 | 8MB | 每个线程的私有栈空间 |
| 栈地址 | 系统自动分配 | 可以手动指定栈内存位置 |
| 分离状态 | PTHREAD_CREATE_JOINABLE | 线程结束后是否自动回收 |
| 调度策略 | SCHED_OTHER | 调度算法 |
| 优先级 | 0 | 调度优先级 |
| 继承属性 | PTHREAD_INHERIT_SCHED | 继承父线程调度参数 |

#### 2. 属性设置示例

```c
#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

void* worker(void* arg) {
    int id = *(int*)arg;
    printf("线程 %d 正在运行\n", id);

    // 获取自身栈信息
    pthread_attr_t attr;
    void* stack_addr;
    size_t stack_size;
    pthread_getattr_np(pthread_self(), &attr);
    pthread_attr_getstack(&attr, &stack_addr, &stack_size);
    printf("线程 %d 栈大小: %zu MB\n", id, stack_size / (1024*1024));
    pthread_attr_destroy(&attr);

    return NULL;
}

int main() {
    pthread_t tid;
    pthread_attr_t attr;

    // 初始化属性对象
    pthread_attr_init(&attr);

    // 设置栈大小为 4MB（默认8MB太大，可按需缩小）
    pthread_attr_setstacksize(&attr, 4 * 1024 * 1024);

    // 设置为分离线程（结束后自动回收，不需要join）
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    int id = 1;
    int ret = pthread_create(&tid, &attr, worker, &id);
    if (ret != 0) {
        fprintf(stderr, "创建线程失败: %s\n", strerror(ret));
        return 1;
    }

    // 分离线程不能 join，主线程等待一下
    sleep(1);

    // 销毁属性对象
    pthread_attr_destroy(&attr);

    return 0;
}
```

***

### 7. 线程ID获取

#### 1. 三种"线程ID"的区别

| 函数 | 类型 | 含义 | 作用域 |
| --- | --- | --- | --- |
| `pthread_self()` | `pthread_t` | POSIX线程库的线程ID | 进程内 |
| `syscall(SYS_gettid)` | `pid_t` | 内核分配的线程ID | 系统全局 |
| `getpid()` | `pid_t` | 进程ID（主线程和子线程相同） | 系统全局 |

```c
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/syscall.h>

void* worker(void* arg) {
    pthread_t ptid = pthread_self();         // POSIX 线程ID
    pid_t     ktid = syscall(SYS_gettid);    // 内核线程ID
    pid_t     pid = getpid();                // 进程ID

    printf("子线程: pthread_t=%lu, 内核tid=%d, 进程pid=%d\n",
           (unsigned long)ptid, ktid, pid);
    return NULL;
}

int main() {
    printf("主线程: pthread_t=%lu, 内核tid=%d, 进程pid=%d\n",
           (unsigned long)pthread_self(),
           (int)syscall(SYS_gettid),
           getpid());

    pthread_t tid;
    pthread_create(&tid, NULL, worker, NULL);
    pthread_join(tid, NULL);

    return 0;
}
```

**输出示例**：

```
主线程: pthread_t=140123456789120, 内核tid=12345, 进程pid=12345
子线程: pthread_t=140123448396416, 内核tid=12346, 进程pid=12345
```

注意：主线程的内核tid == 进程pid，子线程的内核tid不同但进程pid相同。

***

### 8. 创建线程的完整示例

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

typedef struct {
    int thread_id;
    int loop_count;
} ThreadConfig;

// 线程工作函数
void* worker(void* arg) {
    ThreadConfig* config = (ThreadConfig*)arg;

    printf("[线程%d] 启动, 内核tid=%d\n",
           config->thread_id, (int)syscall(SYS_gettid));

    for (int i = 0; i < config->loop_count; i++) {
        printf("[线程%d] 工作进度: %d/%d\n",
               config->thread_id, i + 1, config->loop_count);
        usleep(100000);  // 模拟工作耗时
    }

    printf("[线程%d] 完成\n", config->thread_id);

    // 返回结果
    int* result = malloc(sizeof(int));
    *result = config->thread_id * 100;
    free(config);

    return (void*)result;
}

int main() {
    const int THREAD_COUNT = 3;
    pthread_t threads[THREAD_COUNT];

    // 创建多个线程
    for (int i = 0; i < THREAD_COUNT; i++) {
        ThreadConfig* config = malloc(sizeof(ThreadConfig));
        config->thread_id = i + 1;
        config->loop_count = 3 + i;

        int ret = pthread_create(&threads[i], NULL, worker, config);
        if (ret != 0) {
            fprintf(stderr, "创建线程%d失败: %s\n", i + 1, strerror(ret));
            free(config);
        }
    }

    // 等待所有线程完成并收集结果
    for (int i = 0; i < THREAD_COUNT; i++) {
        void* retval;
        pthread_join(threads[i], &retval);
        if (retval != NULL) {
            printf("[主线程] 线程%d返回值: %d\n", i + 1, *(int*)retval);
            free(retval);
        }
    }

    printf("[主线程] 所有线程已完成\n");
    return 0;
}
```

***

### 9. 常见错误

#### 1. 错误1：创建线程后不 join 也不 detach

```c
// 错误：线程资源泄漏
pthread_create(&tid, NULL, worker, NULL);
// 主线程直接退出，子线程被杀，且线程资源未回收
```

**后果**：线程结束后资源不释放，内存泄漏。

**修复**：要么 `pthread_join()`，要么创建时设 `PTHREAD_CREATE_DETACHED`。

#### 2. 错误2：传递局部变量地址给线程

```c
// 错误：局部变量在线程使用前可能已失效
void create_threads() {
    int value = 42;
    pthread_create(&tid, NULL, worker, &value);  // value 在函数返回后失效！
}
```

**修复**：用 `malloc` 分配堆内存传递。

```c
int* value = malloc(sizeof(int));
*value = 42;
pthread_create(&tid, NULL, worker, value);  // 线程内 free
```

#### 3. 错误3：忽略 pthread_create 的返回值

```c
// 错误：不检查返回值
pthread_create(&tid, NULL, worker, &data);
// 如果创建失败，tid 无效，后续 join 会出错
```

**修复**：

```c
int ret = pthread_create(&tid, NULL, worker, &data);
if (ret != 0) {
    fprintf(stderr, "创建线程失败: %s\n", strerror(ret));
    exit(1);
}
```

#### 4. 错误4：主线程先于子线程退出

```c
int main() {
    pthread_create(&tid, NULL, worker, NULL);
    return 0;  // 主线程退出，整个进程结束，子线程被杀
}
```

**修复**：

```c
int main() {
    pthread_create(&tid, NULL, worker, NULL);
    pthread_join(tid, NULL);  // 等待子线程完成
    return 0;
}
```

#### 5. 常见错误汇总

| 错误 | 后果 | 修复方案 |
| --- | --- | --- |
| 不 join/detach | 资源泄漏 | 必须 join 或 detach |
| 传局部变量地址 | 悬空指针、数据错乱 | 用 malloc 分配堆内存 |
| 不检查返回值 | 创建失败后继续操作无效tid | 检查返回值 |
| 主线程先退出 | 子线程被强制终止 | 用 join 等待 |
| 线程数过多 | 资源耗尽、调度开销大 | 控制线程数量 |

***

### 10. 为什么你感觉"平平无奇、没动静、没延迟"？

#### 1. 速度太快：微秒级完成

创建线程 + 分配栈 + 入调度队列 + 第一次切换，**总共就几微秒到几十微秒**，人类感官完全察觉不到延迟。

#### 2. 内核静默完成

没有弹窗、没有日志、没有额外代码，全在内核态无声处理，用户层代码就一句函数调用。

#### 3. 串行代码看起来顺序执行

你写：

```c
printf("创建线程前\n");
pthread_create(...);
printf("创建线程后\n");
```

你视觉上就是**从上往下顺序跑**，但此时 CPU 已经**两个线程来回切换**并发执行了。

> **你看到的是代码顺序，底层早已并行。**

#### 4. 语言库屏蔽了所有细节

C 标准库 / pthread 库，把：

- 系统调用
- 内存分配
- 内核交互
- 调度入队

全部封装成**一个简单函数**，对你暴露的只有一行接口。

***

### 11. 形象比喻

你在办公室（进程），只做了**一个动作**：

> **喊了一声：招个新员工**（pthread_create）

背后行政、人事、后勤默默瞬间干完：

- 给新员工配办公桌（线程栈）
- 建员工档案（TCB）
- 给他安排工位、排班（进调度队列）
- 直接上岗开始干活（切换CPU执行）

**你就一句话，背后整套流程全自动瞬间完成，你当然没感觉。**

***

### 12. 进程创建 vs 线程创建对比

| 维度 | fork() 创建进程 | pthread_create() 创建线程 |
| --- | --- | --- |
| 系统调用 | fork() | clone() |
| 地址空间 | 完整复制（写时拷贝） | 共享，不复制 |
| 栈 | 新进程独立栈 | 新分配独立栈 |
| 文件描述符 | 复制一份 | 共享同一份 |
| 创建耗时 | 毫秒级 | 微秒级 |
| 内存开销 | 大（页表、TLB） | 小（只分配栈） |
| 通信方式 | 管道/共享内存/socket | 直接读写共享变量 |
| 崩溃影响 | 子进程崩溃不影响父进程 | 线程崩溃整个进程挂掉 |

***

### 13. 极简总结

| 层面 | 真相 |
| --- | --- |
| **代码层面** | 只有一行，是库和内核把巨复杂的底层封装了 |
| **底层实现** | 分配栈、建TCB、内核调度、上下文切换、寄存器保存加载，十几步重操作 |
| **用户感受** | 微秒级速度、内核静默后台完成、代码看上去还是顺序执行 |
| **核心系统调用** | clone()，带 CLONE_VM 等标志实现资源共享 |
| **必须做的事** | 创建后必须 join 或 detach，参数必须用堆内存传递 |

***

### 相关阅读

- [多线程底层原理与通信](./00-多线程底层原理与通信.md)
- [pthread-create参数与任务函数](09-pthread-create参数与任务函数.md)
- [pthread-mutex底层原理](11-pthread-mutex底层原理.md)

***