# jthread 与 thread 的区别
> 📖 相关章节：[多线程基础](../../02-CPP/29-多线程基础.md)、[线程同步](../../02-CPP/30-线程同步.md)、[原子操作](../../02-CPP/31-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

> "借了书必须自己记得还，还是离开时自动归还"——jthread 是自动 join 的线程，离开作用域就不会被遗忘。

***

### 1. 核心定义

- **std::thread** = C++11 的线程类，析构时若未 join/detach 则调用 `std::terminate` 终止程序
- **std::jthread** = C++20 的线程类，析构时自动 join，并内置协作式取消机制（stop_token）

关键点：**jthread = thread + 自动 join + stop_token 协作式取消**。

***

### 2. 生活类比

**借书与还书**：

| 概念 | 类比 | 对应代码 |
|------|------|---------|
| std::thread | 借了书必须自己记得还，忘了还就罚款（terminate） | 析构前必须手动 join/detach |
| std::jthread | 自动还书系统，离开图书馆自动归还 | 析构时自动 join |
| stop_token | 还书提醒通知，告诉对方"该停了" | 请求线程停止执行的信号 |
| stop_source | 还书按钮，按下触发提醒 | 发出停止请求的源头 |

thread 的痛点：

```
1. 忘了还书 → 罚款（std::terminate 终止程序）
2. 不知道书什么时候看完 → 无法优雅地通知对方停止
3. 异常路径容易遗漏 → try-catch 里也要记得 join
```

jthread 的改善：

```
1. 自动还书 → 离开作用域自动 join，不会忘记
2. 有还书提醒 → stop_token 通知线程该停了
3. 异常安全 → 析构函数保证 join，无需手动处理
```

***

### 3. 自动 join 机制

#### 1. thread 的析构陷阱

```cpp
#include <iostream>
#include <thread>

void worker() {
    std::cout << "working...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "done\n";
}

int main() {
    {
        std::thread t(worker);
    }
    // t 析构，但未 join/detach → std::terminate()！程序崩溃
    std::cout << "after scope\n";
}
```

#### 2. 正确的 thread 用法（手动 join）

```cpp
int main() {
    {
        std::thread t(worker);
        t.join();
    }
    std::cout << "after scope\n";
}
```

#### 3. jthread 的自动 join

```cpp
#include <iostream>
#include <thread>

void worker() {
    std::cout << "working...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "done\n";
}

int main() {
    {
        std::jthread t(worker);
    }
    // t 析构时自动 join，无需手动调用
    std::cout << "after scope\n";
}
```

#### 4. 异常安全对比

```cpp
#include <iostream>
#include <thread>
#include <stdexcept>

void worker() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
}

// thread：异常路径容易遗漏 join
void unsafe_version() {
    std::thread t(worker);
    throw std::runtime_error("oops");
    t.join();  // 永远到不了这里 → terminate！
}

// thread：需要 try-catch 保证 join
void safe_version() {
    std::thread t(worker);
    try {
        throw std::runtime_error("oops");
    } catch (...) {
        t.join();
        throw;
    }
}

// jthread：异常安全，无需额外处理
void jthread_version() {
    std::jthread t(worker);
    throw std::runtime_error("oops");
    // t 析构时自动 join，异常安全
}
```

***

### 4. stop_token 协作式取消

jthread 的另一个核心特性是内置了协作式取消机制，由三个组件协作：

| 组件 | 角色 | 说明 |
|------|------|------|
| `std::stop_source` | 发出停止请求 | 拥有停止请求的所有权，可以请求停止 |
| `std::stop_token` | 查询停止请求 | 只读查询，检查是否收到停止请求 |
| `std::stop_callback` | 注册停止回调 | 停止请求发出时自动执行回调 |

#### 1. 基本用法

```cpp
#include <iostream>
#include <thread>

void worker(std::stop_token token) {
    int count = 0;
    while (!token.stop_requested()) {
        std::cout << "working... " << count++ << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "received stop request, exiting gracefully\n";
}

int main() {
    std::jthread t(worker);

    std::this_thread::sleep_for(std::chrono::seconds(1));
    t.request_stop();

    t.join();
    std::cout << "thread stopped\n";
}
```

#### 2. stop_callback 注册回调

```cpp
#include <iostream>
#include <thread>
#include <stop_token>

void worker(std::stop_token token) {
    std::stop_callback cb(token, [] {
        std::cout << "cleanup callback triggered!\n";
    });

    while (!token.stop_requested()) {
        std::cout << "working...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    std::cout << "exiting\n";
}

int main() {
    std::jthread t(worker);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    t.request_stop();
    t.join();
}
```

#### 3. thread 如何实现类似功能（C++11 手动版）

```cpp
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<bool> should_stop{false};

void worker() {
    int count = 0;
    while (!should_stop.load()) {
        std::cout << "working... " << count++ << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "received stop signal, exiting\n";
}

int main() {
    std::thread t(worker);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    should_stop.store(true);
    t.join();
}
```

**对比**：jthread 的 stop_token 是标准化的、类型安全的，而手动 atomic bool 需要自己管理生命周期。

***

### 5. API 差异

#### 1. 构造函数差异

```cpp
// thread：线程函数不接受 stop_token
std::thread t([] { /* ... */ });

// jthread：线程函数可以接受 stop_token 作为第一个参数
std::jthread jt([](std::stop_token token) { /* ... */ });
// jthread 内部自动传入 stop_token

// jthread 也可以不接受 stop_token
std::jthread jt2([] { /* ... */ });
// 此时只是自动 join，没有取消功能
```

#### 2. 成员函数差异

```cpp
std::thread t;
std::jthread jt;

// 两者共有
t.join();
t.detach();
t.joinable();
t.get_id();
t.native_handle();

// jthread 独有
jt.request_stop();           // 请求停止
jt.get_stop_source();        // 获取 stop_source
jt.get_stop_token();         // 获取 stop_token

// 注意：jthread 没有 detach 的常见需求
// 因为自动 join 已经解决了资源管理问题
```

#### 3. 析构行为对比

```cpp
{
    std::thread t([] { /* ... */ });
    // 析构：未 join/detach → std::terminate()
}

{
    std::jthread jt([] { /* ... */ });
    // 析构：
    // 1. 调用 request_stop()（如果有 stop_token）
    // 2. 调用 join()
    // 线程安全退出，不会 terminate
}
```

***

### 6. 对比表格

| 特性 | std::thread | std::jthread |
|------|:---:|:---:|
| C++ 版本 | C++11 | C++20 |
| 析构行为 | 未 join/detach → terminate | 自动 request_stop + join |
| 取消机制 | 无（需手动实现） | 内置 stop_token/stop_source |
| 线程函数签名 | 任意可调用对象 | 第一个参数可选 stop_token |
| request_stop | 无 | 有 |
| get_stop_token | 无 | 有 |
| get_stop_source | 无 | 有 |
| join | 手动调用 | 析构自动调用 |
| detach | 支持 | 支持（但通常不需要） |
| 头文件 | `<thread>` | `<thread>` |
| 异常安全 | 需手动保证 | 自动保证 |

***

### 7. 何时用哪个

| 场景 | 推荐 | 原因 |
|------|------|------|
| C++20+ 新项目 | `jthread` | 自动 join + 取消机制，更安全更方便 |
| 需要优雅停止线程 | `jthread` | stop_token 标准化协作式取消 |
| C++11/14/17 环境 | `thread` | jthread 不可用 |
| 需要 detach 语义 | `thread` | jthread 也可 detach，但语义不匹配自动 join 的设计意图 |
| 后台守护线程 | `thread` + detach | 守护线程通常不需要 join |

**简单原则**：

- C++20+ 优先用 `jthread`，告别忘记 join 的噩梦
- 需要取消线程功能时，`jthread` + `stop_token` 是标准方案
- C++11 环境只能用 `thread`，配合 RAII 包装器模拟自动 join

***

### 8. 极简总结

**jthread = thread + 自动 join + stop_token**

| 要点 | 说明 |
|------|------|
| 核心区别 | thread 析构可能 terminate，jthread 析构自动 join |
| 取消机制 | jthread 内置 stop_token 协作式取消，thread 需手动实现 |
| C++ 版本 | thread: C++11，jthread: C++20 |
| 异常安全 | jthread 析构保证 join，thread 需 try-catch |
| 选择原则 | C++20+ 优先 jthread，C++11 只能用 thread |
| 一句话 | thread 要你记得还书，jthread 自动还书还带提醒 |

***

### 相关阅读

- [C与CPP多线程](./07-C与CPP多线程.md)
- [多线程底层原理与通信](./00-多线程底层原理与通信.md)
- [协程与线程](./21-协程与线程.md)