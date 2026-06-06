# scoped_lock 与 lock_guard 的区别
> 📖 相关章节：[多线程基础](../../02-CPP/29-多线程基础.md)、[线程同步](../../02-CPP/30-线程同步.md)、[原子操作](../../02-CPP/31-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

> "一个人拿一把钥匙，还是同时拿多把钥匙按固定顺序开锁"——scoped_lock 一次安全地锁住多个互斥量，避免死锁。

***

### 1. 核心定义

- **lock_guard** = RAII 包装器，管理**单个**互斥量的加锁/解锁
- **scoped_lock** = RAII 包装器，可同时管理**多个**互斥量的加锁/解锁，且使用 `std::lock` 避免死锁

关键点：**lock_guard 管一把锁，scoped_lock 管多把锁还防死锁**。

***

### 2. 生活类比

**钥匙与门锁**：

| 概念 | 类比 | 对应代码 |
|------|------|---------|
| lock_guard | 一个人拿一把钥匙开一扇门 | 管理一个 mutex |
| scoped_lock（单锁） | 一个人拿一把钥匙开一扇门 | 管理一个 mutex（和 lock_guard 等价） |
| scoped_lock（多锁） | 一个人同时拿多把钥匙，按固定顺序开门 | 管理多个 mutex，按安全顺序加锁 |

**死锁场景**：

```
线程A：先拿钥匙1，再拿钥匙2
线程B：先拿钥匙2，再拿钥匙1

线程A 拿到钥匙1，等钥匙2
线程B 拿到钥匙2，等钥匙1
→ 两人互相等，永远等下去 = 死锁
```

**scoped_lock 的解决方案**：

```
scoped_lock(mux1, mux2) 内部调用 std::lock(mux1, mux2)
std::lock 尝试所有加锁顺序，找到不死锁的组合
→ 两个线程都按相同顺序拿钥匙，不会死锁
```

***

### 3. 代码示例

#### 1. lock_guard：单锁场景

```cpp
#include <iostream>
#include <mutex>
#include <thread>

int counter = 0;
std::mutex counter_mtx;

void increment(int n) {
    for (int i = 0; i < n; ++i) {
        std::lock_guard<std::mutex> lock(counter_mtx);
        ++counter;
    }
}

int main() {
    std::thread t1(increment, 100000);
    std::thread t2(increment, 100000);
    t1.join();
    t2.join();
    std::cout << "counter = " << counter << "\n";
}
```

#### 2. scoped_lock：多锁场景

```cpp
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx_a;
std::mutex mtx_b;
int data_a = 0;
int data_b = 0;

void worker1() {
    for (int i = 0; i < 100000; ++i) {
        std::scoped_lock lock(mtx_a, mtx_b);
        ++data_a;
        ++data_b;
    }
}

void worker2() {
    for (int i = 0; i < 100000; ++i) {
        std::scoped_lock lock(mtx_b, mtx_a);
        // 即使参数顺序不同，scoped_lock 内部也会用 std::lock
        // 避免死锁
        ++data_b;
        ++data_a;
    }
}

int main() {
    std::thread t1(worker1);
    std::thread t2(worker2);
    t1.join();
    t2.join();
    std::cout << "data_a = " << data_a << ", data_b = " << data_b << "\n";
}
```

#### 3. 不用 scoped_lock 的死锁风险

```cpp
std::mutex mtx_a, mtx_b;

void worker1() {
    std::lock_guard<std::mutex> lock_a(mtx_a);  // 先锁 mtx_a
    std::lock_guard<std::mutex> lock_b(mtx_b);  // 再锁 mtx_b
}

void worker2() {
    std::lock_guard<std::mutex> lock_b(mtx_b);  // 先锁 mtx_b
    std::lock_guard<std::mutex> lock_a(mtx_a);  // 再锁 mtx_a
}
// 死锁！worker1 持有 mtx_a 等 mtx_b，worker2 持有 mtx_b 等 mtx_a
```

#### 4. 用 std::lock + lock_guard 手动防死锁（C++11 方式）

```cpp
void safe_worker() {
    std::lock(mtx_a, mtx_b);  // 原子地锁住两个 mutex，避免死锁
    std::lock_guard<std::mutex> lock_a(mtx_a, std::adopt_lock);
    std::lock_guard<std::mutex> lock_b(mtx_b, std::adopt_lock);
    // std::adopt_lock 表示 mutex 已被锁住，lock_guard 只负责解锁
}
```

**scoped_lock 就是上面这种模式的封装**，一行代码搞定。

***

### 4. 多锁场景的死锁问题

#### 1. 死锁的四个必要条件

| 条件 | 说明 |
|------|------|
| 互斥 | 每个互斥量同一时刻只能被一个线程持有 |
| 持有并等待 | 线程持有至少一个互斥量，同时等待其他互斥量 |
| 不可抢占 | 互斥量不能被强制从持有者手中夺走 |
| 循环等待 | 线程间形成环形等待链 |

**破坏"循环等待"是最实用的防死锁策略**：所有线程按相同顺序加锁。

#### 2. scoped_lock 如何避免死锁

`scoped_lock` 内部使用 `std::lock` 算法：

1. 尝试锁定第一个 mutex
2. 成功后尝试锁定第二个 mutex
3. 如果第二个锁定失败，释放第一个，重新尝试
4. 反复尝试，直到所有 mutex 同时锁定成功

```
线程A: scoped_lock(m1, m2)
  → std::lock 尝试: lock m1 → lock m2 → 成功

线程B: scoped_lock(m2, m1)
  → std::lock 尝试: lock m2 → lock m1 → 如果 m1 被线程A持有
  → 释放 m2 → lock m1 → lock m2 → 成功

结果：两个线程最终都按相同顺序持有锁，不会死锁
```

***

### 5. 对比表格

| 特性 | lock_guard | scoped_lock |
|------|:---:|:---:|
| C++ 版本 | C++11 | C++17 |
| 模板参数 | `lock_guard<Mutex>` | `scoped_lock<Mutex...>`（可变参数） |
| 管理互斥量数量 | 1 个 | 1 个或多个 |
| 死锁预防 | 无（单锁无需预防） | 有（多锁时使用 `std::lock` 算法） |
| adopt_lock 支持 | 支持 | 支持 |
| defer_lock 支持 | 不支持 | 不支持（用 `unique_lock` 代替） |
| 单锁时等价性 | — | 与 `lock_guard` 功能等价 |
| 头文件 | `<mutex>` | `<mutex>` |

#### 1. 单锁场景对比

```cpp
std::mutex m;

std::lock_guard<std::mutex> lg(m);     // C++11
std::scoped_lock<std::mutex> sl(m);    // C++17，等价
std::scoped_lock sl2(m);               // C++17，CTAD 推导，更简洁
```

C++17 起，单锁场景用 `scoped_lock` 也可以，且写法更简洁（CTAD 自动推导模板参数）。

#### 2. 多锁场景对比

```cpp
std::mutex m1, m2;

// C++11 方式：手动 std::lock + adopt_lock
std::lock(m1, m2);
std::lock_guard<std::mutex> lg1(m1, std::adopt_lock);
std::lock_guard<std::mutex> lg2(m2, std::adopt_lock);

// C++17 方式：一行搞定
std::scoped_lock sl(m1, m2);
```

***

### 6. 何时用哪个

| 场景 | 推荐 | 原因 |
|------|------|------|
| 只需要锁一个 mutex | `lock_guard` 或 `scoped_lock` | 功能等价，`scoped_lock` 写法稍简洁 |
| 需要锁多个 mutex | `scoped_lock` | 自动防死锁，一行代码搞定 |
| C++11 环境，多锁 | `std::lock` + `lock_guard` | 手动防死锁 |
| 需要延迟加锁/条件变量 | `unique_lock` | `lock_guard` 和 `scoped_lock` 都不支持 |
| 需要提前解锁 | `unique_lock` | `lock_guard` 和 `scoped_lock` 析构时才解锁 |

**简单原则**：

- C++17+：统一用 `scoped_lock`，单锁多锁都适用
- C++11：单锁用 `lock_guard`，多锁用 `std::lock` + `lock_guard`
- 需要更灵活的控制（延迟加锁、条件变量、提前解锁）用 `unique_lock`

***

### 7. 极简总结

**lock_guard 管一把锁，scoped_lock 管多把锁还防死锁**

| 要点 | 说明 |
|------|------|
| 核心区别 | lock_guard 单锁，scoped_lock 多锁 + 死锁预防 |
| C++ 版本 | lock_guard: C++11，scoped_lock: C++17 |
| 死锁预防 | scoped_lock 内部用 std::lock 算法，自动避免循环等待 |
| 单锁场景 | 两者等价，scoped_lock 写法更简洁（CTAD） |
| 选择原则 | C++17+ 统一用 scoped_lock；需灵活控制用 unique_lock |
| 一句话 | 一把锁用谁都行，多把锁用 scoped_lock |

***

### 相关阅读

- [加锁解锁](./05-加锁解锁.md)
- [锁的粒度与性能](./19-锁的粒度与性能.md)
- [pthread-mutex底层原理](./04-pthread-mutex底层原理.md)

***