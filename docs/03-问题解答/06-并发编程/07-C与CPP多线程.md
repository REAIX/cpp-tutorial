# C与C++多线程的区别
> 📖 相关章节：[多线程基础](../../02-CPP/27-多线程基础.md)、[线程同步](../../02-CPP/28-线程同步.md)、[原子操作](../../02-CPP/29-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

### 1. 一句话结论

**C 和 C++ 的多线程不是同一个！它们是两套完全独立的标准、两套独立的库、两套独立的 API！**

***

### 2. 先分清 3 个层面

| 层面 | 说明 |
| --- | --- |
| **操作系统原生线程** | Linux `pthread` / Windows `CreateThread`，真正干活的底层实现 |
| **C11 标准线程库** | `<threads.h>`，C语言官方标准，很少使用 |
| **C++11 标准线程库** | `<thread>`、`<mutex>`、`<future>`，C++官方标准，工业界主流 |

**关键理解**：C 和 C++ 的标准线程库最终都调用操作系统原生线程 API，底层是同一个内核实现。

***

### 3. C 语言的 pthread API

#### 1. 核心 API 一览

| API | 功能 | 头文件 |
| --- | --- | --- |
| `pthread_create()` | 创建线程 | `<pthread.h>` |
| `pthread_join()` | 等待线程结束 | `<pthread.h>` |
| `pthread_detach()` | 分离线程 | `<pthread.h>` |
| `pthread_exit()` | 线程退出 | `<pthread.h>` |
| `pthread_self()` | 获取线程ID | `<pthread.h>` |
| `pthread_cancel()` | 取消线程 | `<pthread.h>` |
| `pthread_mutex_init()` | 初始化互斥锁 | `<pthread.h>` |
| `pthread_mutex_lock()` | 加锁 | `<pthread.h>` |
| `pthread_mutex_unlock()` | 解锁 | `<pthread.h>` |
| `pthread_cond_wait()` | 条件变量等待 | `<pthread.h>` |
| `pthread_cond_signal()` | 条件变量唤醒 | `<pthread.h>` |

#### 2. pthread 基本用法

```c
#include <stdio.h>
#include <pthread.h>

void* worker(void* arg) {
    int id = *(int*)arg;
    printf("线程 %d 正在工作\n", id);
    return NULL;
}

int main() {
    pthread_t tid;
    int id = 1;
    pthread_create(&tid, NULL, worker, &id);
    pthread_join(tid, NULL);
    return 0;
}
```

***

### 4. C++ 的 std::thread

#### 1. 核心 API 一览

| API | 功能 | 头文件 |
| --- | --- | --- |
| `std::thread` | 创建线程 | `<thread>` |
| `std::thread::join()` | 等待线程结束 | `<thread>` |
| `std::thread::detach()` | 分离线程 | `<thread>` |
| `std::thread::get_id()` | 获取线程ID | `<thread>` |
| `std::mutex` | 互斥锁 | `<mutex>` |
| `std::lock_guard` | RAII锁管理 | `<mutex>` |
| `std::unique_lock` | 灵活锁管理 | `<mutex>` |
| `std::condition_variable` | 条件变量 | `<condition_variable>` |
| `std::future` | 异步结果 | `<future>` |
| `std::promise` | 异步赋值 | `<future>` |
| `std::async` | 异步启动 | `<future>` |

#### 2. std::thread 基本用法

```cpp
#include <iostream>
#include <thread>

void worker(int id) {
    std::cout << "线程 " << id << " 正在工作" << std::endl;
}

int main() {
    std::thread t(worker, 1);  // 直接传参数，不需要 void*
    t.join();
    return 0;
}
```

***

### 5. 核心区别对比

| 维度 | C 语言多线程 | C++ 多线程 |
| --- | --- | --- |
| 头文件 | `<pthread.h>` / `<threads.h>` | `<thread>`, `<mutex>`, `<future>` |
| 标准 | POSIX / C11 | C++11 及以上 |
| 写法 | 函数式（C风格） | 面向对象（类、封装） |
| 线程函数签名 | `void* (*)(void*)` | 任意可调用对象 |
| 参数传递 | `void*` 手动转换 | 模板自动推导 |
| 锁管理 | 手动 lock/unlock | RAII 自动管理 |
| 条件变量 | `pthread_cond_t` | `std::condition_variable` |
| 异步结果 | 无标准支持 | `std::future/promise` |
| 底层实现 | 调用操作系统 API | 调用操作系统 API |
| 流行度 | C 项目用 pthread | C++ 项目用 std::thread |
| 易用性 | 麻烦、容易错 | 简单、安全 |
| 跨平台 | pthread 仅 POSIX | 标准库跨平台 |

***

### 6. 互斥锁对比

#### 1. C 语言互斥锁

```c
#include <pthread.h>

static int counter = 0;
static pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;

void* worker(void* arg) {
    pthread_mutex_lock(&mtx);
    counter++;
    pthread_mutex_unlock(&mtx);
    return NULL;
}
```

#### 2. C++ 互斥锁

```cpp
#include <mutex>

int counter = 0;
std::mutex mtx;

void worker() {
    {
        std::lock_guard<std::mutex> lock(mtx);  // RAII：构造加锁，析构解锁
        counter++;
    }  // 离开作用域自动解锁
}
```

#### 3. 互斥锁对比

| 维度 | C (pthread_mutex) | C++ (std::mutex) |
| --- | --- | --- |
| 声明 | `pthread_mutex_t mtx` | `std::mutex mtx` |
| 初始化 | `PTHREAD_MUTEX_INITIALIZER` 或 `pthread_mutex_init` | 构造函数自动初始化 |
| 加锁 | `pthread_mutex_lock(&mtx)` | `mtx.lock()` 或 `std::lock_guard` |
| 解锁 | `pthread_mutex_unlock(&mtx)` | `mtx.unlock()` 或自动（RAII） |
| 销毁 | `pthread_mutex_destroy(&mtx)` | 析构函数自动销毁 |
| 异常安全 | 不安全（异常时锁不释放） | 安全（RAII保证释放） |
| 递归锁 | `PTHREAD_MUTEX_RECURSIVE` | `std::recursive_mutex` |

#### 4. RAII 的威力

```cpp
// C++ 的 lock_guard：异常安全
void safe_func() {
    std::lock_guard<std::mutex> lock(mtx);
    // 即使这里抛出异常，锁也会在 lock 析构时自动释放
    if (error) {
        throw std::runtime_error("error");  // 锁自动释放！
    }
    counter++;
}

// C 语言的锁：异常不安全
void unsafe_func() {
    pthread_mutex_lock(&mtx);
    if (error) {
        // 如果这里 return 或 longjmp，锁不会释放！
        return;  // 死锁！
    }
    counter++;
    pthread_mutex_unlock(&mtx);
}
```

***

### 7. 条件变量对比

#### 1. C 语言条件变量

```c
#include <pthread.h>

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  cond = PTHREAD_COND_INITIALIZER;
int ready = 0;

// 等待方
void* waiter(void* arg) {
    pthread_mutex_lock(&mtx);
    while (!ready) {
        pthread_cond_wait(&cond, &mtx);
    }
    pthread_mutex_unlock(&mtx);
    printf("收到通知\n");
    return NULL;
}

// 通知方
void notifier() {
    pthread_mutex_lock(&mtx);
    ready = 1;
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mtx);
}
```

#### 2. C++ 条件变量

```cpp
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

// 等待方
void waiter() {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, []{ return ready; });  // lambda 条件，自动处理虚假唤醒
    std::cout << "收到通知" << std::endl;
}

// 通知方
void notifier() {
    {
        std::lock_guard<std::mutex> lock(mtx);
        ready = true;
    }  // 解锁在 notify 之前也可以
    cv.notify_one();
}
```

#### 3. 条件变量对比

| 维度 | C (pthread_cond) | C++ (std::condition_variable) |
| --- | --- | --- |
| 声明 | `pthread_cond_t cond` | `std::condition_variable cv` |
| 初始化 | `PTHREAD_COND_INITIALIZER` | 构造函数自动 |
| 等待 | `pthread_cond_wait(&cond, &mtx)` | `cv.wait(lock)` 或 `cv.wait(lock, pred)` |
| 唤醒一个 | `pthread_cond_signal(&cond)` | `cv.notify_one()` |
| 唤醒全部 | `pthread_cond_broadcast(&cond)` | `cv.notify_all()` |
| 带谓词等待 | 手动 while 循环 | `cv.wait(lock, predicate)` |
| 销毁 | `pthread_cond_destroy(&cond)` | 析构函数自动 |

***

### 8. 线程局部存储对比

#### 1. C 语言线程局部存储

```c
// GCC 扩展
__thread int tls_var = 0;

// C11 标准
_Thread_local int tls_var = 0;

// pthread 专用
pthread_key_t key;

void init_key() {
    pthread_key_create(&key, NULL);
}

void set_tls(void* value) {
    pthread_setspecific(key, value);
}

void* get_tls() {
    return pthread_getspecific(key);
}
```

#### 2. C++ 线程局部存储

```cpp
// C++11 标准
thread_local int tls_var = 0;

// 使用示例
thread_local std::string thread_name;

void worker() {
    thread_name = "Thread-" + std::to_string(std::hash<std::thread::id>{}(std::this_thread::get_id()));
    std::cout << "我是 " << thread_name << std::endl;
    // thread_name 每个线程独立，不需要锁
}
```

#### 3. 线程局部存储对比

| 维度 | C | C++ |
| --- | --- | --- |
| 关键字 | `__thread` / `_Thread_local` | `thread_local` |
| 动态创建 | `pthread_key_create/setspecific/getspecific` | 不需要，直接用 `thread_local` |
| 析构回调 | `pthread_key_create` 的第二个参数 | 自动调用析构函数 |
| 支持类型 | 仅 POD 类型（`__thread`） | 任意类型（含类对象） |
| 初始化 | 仅常量表达式 | 支持动态初始化 |

***

### 9. C++ 的独有优势

#### 1. RAII 自动锁管理

```cpp
// C++：自动加锁解锁，异常安全
void safe_increment() {
    std::lock_guard<std::mutex> lock(mtx);
    counter++;  // 即使抛异常，锁也会释放
}

// C：手动管理，容易出错
void unsafe_increment() {
    pthread_mutex_lock(&mtx);
    counter++;
    // 如果忘记 unlock，或者异常跳转，死锁
    pthread_mutex_unlock(&mtx);
}
```

#### 2. 任意可调用对象

```cpp
// C++：函数、lambda、成员函数、仿函数都能用
std::thread t1(some_function);                    // 普通函数
std::thread t2([](){ cout << "lambda"; });        // lambda
std::thread t3(&MyClass::run, &obj);              // 成员函数
std::thread t4(MyFunctor());                      // 仿函数
```

#### 3. future/promise 异步编程

```cpp
#include <future>
#include <iostream>

int compute(int x) {
    return x * x;
}

int main() {
    // 异步启动任务
    std::future<int> f = std::async(std::launch::async, compute, 42);

    // 做其他事情...

    // 获取结果（如果没完成会阻塞）
    int result = f.get();
    std::cout << "结果: " << result << std::endl;  // 1764

    return 0;
}
```

#### 4. promise 实现线程间传值

```cpp
#include <future>
#include <thread>
#include <iostream>

void producer(std::promise<int> prom) {
    // 做一些计算...
    prom.set_value(42);  // 设置结果
}

int main() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();

    std::thread t(producer, std::move(prom));

    int result = fut.get();  // 等待并获取结果
    std::cout << "收到: " << result << std::endl;

    t.join();
    return 0;
}
```

#### 5. packaged_task 包装任务

```cpp
#include <future>
#include <iostream>

int main() {
    std::packaged_task<int(int)> task([](int x) {
        return x * 2;
    });

    std::future<int> fut = task.get_future();

    // 在另一个线程中执行
    std::thread t(std::move(task), 21);

    std::cout << "结果: " << fut.get() << std::endl;  // 42

    t.join();
    return 0;
}
```

#### 6. C++ 独有优势总结

| 优势 | 说明 |
| --- | --- |
| RAII | 锁自动管理，异常安全 |
| 任意可调用对象 | 函数、lambda、成员函数、仿函数 |
| future/promise | 异步结果获取，无需手动同步 |
| packaged_task | 任务封装，灵活调度 |
| async | 一行代码启动异步任务 |
| 类型安全 | 模板推导参数类型，不需要 void* 转换 |
| 异常传播 | 线程异常可以跨线程捕获 |

***

### 10. 完整对比示例：生产者-消费者

#### 1. C 语言版本

```c
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUF_SIZE 8

int buffer[BUF_SIZE];
int buf_head = 0, buf_tail = 0, buf_count = 0;

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t  not_full  = PTHREAD_COND_INITIALIZER;
pthread_cond_t  not_empty = PTHREAD_COND_INITIALIZER;

void* producer(void* arg) {
    for (int i = 1; i <= 20; i++) {
        pthread_mutex_lock(&mtx);
        while (buf_count >= BUF_SIZE) {
            pthread_cond_wait(&not_full, &mtx);
        }
        buffer[buf_tail] = i;
        buf_tail = (buf_tail + 1) % BUF_SIZE;
        buf_count++;
        printf("[生产] %d, 缓冲区=%d\n", i, buf_count);
        pthread_cond_signal(&not_empty);
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

void* consumer(void* arg) {
    for (int i = 1; i <= 20; i++) {
        pthread_mutex_lock(&mtx);
        while (buf_count <= 0) {
            pthread_cond_wait(&not_empty, &mtx);
        }
        int item = buffer[buf_head];
        buf_head = (buf_head + 1) % BUF_SIZE;
        buf_count--;
        printf("[消费] %d, 缓冲区=%d\n", item, buf_count);
        pthread_cond_signal(&not_full);
        pthread_mutex_unlock(&mtx);
    }
    return NULL;
}

int main() {
    pthread_t prod, cons;
    pthread_create(&prod, NULL, producer, NULL);
    pthread_create(&cons, NULL, consumer, NULL);
    pthread_join(prod, NULL);
    pthread_join(cons, NULL);
    return 0;
}
```

#### 2. C++ 版本

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

const int BUF_SIZE = 8;

int buffer[BUF_SIZE];
int buf_head = 0, buf_tail = 0, buf_count = 0;

std::mutex mtx;
std::condition_variable not_full, not_empty;

void producer() {
    for (int i = 1; i <= 20; i++) {
        std::unique_lock<std::mutex> lock(mtx);
        not_full.wait(lock, []{ return buf_count < BUF_SIZE; });
        buffer[buf_tail] = i;
        buf_tail = (buf_tail + 1) % BUF_SIZE;
        buf_count++;
        std::cout << "[生产] " << i << ", 缓冲区=" << buf_count << std::endl;
        not_empty.notify_one();
    }
}

void consumer() {
    for (int i = 1; i <= 20; i++) {
        std::unique_lock<std::mutex> lock(mtx);
        not_empty.wait(lock, []{ return buf_count > 0; });
        int item = buffer[buf_head];
        buf_head = (buf_head + 1) % BUF_SIZE;
        buf_count--;
        std::cout << "[消费] " << item << ", 缓冲区=" << buf_count << std::endl;
        not_full.notify_one();
    }
}

int main() {
    std::thread prod(producer);
    std::thread cons(consumer);
    prod.join();
    cons.join();
    return 0;
}
```

#### 3. 两个版本对比

| 维度 | C 版本 | C++ 版本 |
| --- | --- | --- |
| 代码行数 | 较多 | 较少 |
| 锁管理 | 手动 lock/unlock | RAII 自动管理 |
| 条件等待 | 手动 while 循环 | `wait(lock, predicate)` |
| 参数传递 | `void*` 手动转换 | 模板自动推导 |
| 异常安全 | 不安全 | 安全 |
| 跨平台 | 仅 POSIX | 全平台 |

***

### 11. 现实开发真实情况

- **C 程序**：基本不用 C11 `<threads.h>`，直接用操作系统原生 API（Linux `pthread` / Windows `CreateThread`）
- **C++ 程序**：全部用 C++11 `std::thread` 这套标准库
- **混合项目**：C++ 代码中可以调用 pthread API，但不推荐混用

#### 1. 为什么 C 不用 C11 threads.h？

| 原因 | 说明 |
| --- | --- |
| 支持太晚 | C11 标准发布于2011年，pthread 早已根深蒂固 |
| 功能太少 | C11 线程库功能远不如 pthread 丰富 |
| 实现不全 | 很多编译器至今未完整实现 C11 threads.h |
| 生态惯性 | 大量现有 C 代码使用 pthread，迁移成本高 |

#### 2. 为什么 C++ 用 std::thread？

| 原因 | 说明 |
| --- | --- |
| RAII | 自动锁管理，避免忘记解锁 |
| 类型安全 | 不需要 void* 转换 |
| 跨平台 | 同一套代码在 Linux/Windows/Mac 上编译运行 |
| 功能丰富 | future/promise/async 等高级特性 |
| 标准保证 | 所有符合 C++11 的编译器都必须支持 |

***

### 12. 常见陷阱与最佳实践

#### 1. 常见陷阱

| 陷阱 | 后果 | 修复 |
| --- | --- | --- |
| C++ 中混用 pthread 和 std::thread | 死锁、未定义行为 | 统一用一套 API |
| std::thread 忘记 join/detach | 程序终止（std::terminate） | 创建后立即 join 或 detach |
| C++ 传引用不用 std::ref | 参数被拷贝而非引用 | 用 `std::ref` 包装 |
| pthread 传局部变量地址 | 数据错乱或崩溃 | 用 malloc 分配堆内存 |

```cpp
// C++ 传引用的正确方式
void func(int& x) {
    x = 42;
}

int value = 0;
std::thread t(func, std::ref(value));  // 必须用 std::ref
t.join();
// value 现在是 42
```

#### 2. 最佳实践

1. **C 项目**：统一用 pthread，不用 C11 threads.h
2. **C++ 项目**：统一用 std::thread 系列，不用 pthread
3. **锁管理**：C++ 永远用 `std::lock_guard` 或 `std::unique_lock`
4. **条件变量**：C++ 用 `wait(lock, predicate)` 替代手动 while 循环
5. **异步任务**：C++ 用 `std::async` 替代手动创建线程

***

### 13. 极简总结

1. **语言层面**：C 和 C++ 多线程是**两套不同标准、不同 API**，代码不能混用
2. **底层层面**：最终都调用操作系统线程，内核层面是同一个
3. **主流选择**：
   - **C++**：只用 `std::thread` / `std::mutex` / `std::lock_guard`
   - **C**：直接用系统原生线程 API（pthread / CreateThread）
4. **C++ 优势**：RAII 自动锁管理、类型安全、future/promise、异常安全
5. **不要混用**：一个项目中统一用一套 API，不要 pthread 和 std::thread 混用

***

### 相关阅读

- [多线程底层原理与通信](./00-多线程底层原理与通信.md)
- [jthread与thread](25-jthread与thread.md)
- [scoped-lock与lock-guard](./15-scoped-lock与lock-guard.md)

***