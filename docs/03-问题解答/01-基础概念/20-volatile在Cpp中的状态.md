# volatile 在 C++ 中的状态
> 📖 相关章节：[运算符与表达式](../../01-C语言/02-运算符与表达式.md)、[控制结构](../../01-C语言/03-控制结构.md)、[函数](../../01-C语言/04-函数.md)、[基础特性](../../02-CPP/01-基础特性.md)、[命名空间](../../02-CPP/02-命名空间与编码规范.md)

### 1. 一句话概括

**volatile 在 C++ 中没有被删除**，但 C++20 弃用了部分用法。多线程同步应使用 `std::atomic`，volatile 主要用于硬件交互。

***

### 2. C++20 弃用了什么

```cpp
volatile int x = 0;

// C++20 弃用的操作：
x++;              // 弃用：复合赋值
x--;              // 弃用
x += 1;           // 弃用
x &= 0xFF;        // 弃用

// 仍然合法的操作：
x = 1;            // OK：简单赋值
int y = x;        // OK：简单读取
```

#### 1. 弃用的完整列表

| 操作 | 示例 | 状态 |
|:---|:---|:---:|
| 前置自增/自减 | `++x; --x;` | 弃用 |
| 后置自增/自减 | `x++; x--;` | 弃用 |
| 复合赋值 | `+= -= *= &= \|= ^=` | 弃用 |
| 简单赋值 | `x = v;` | 仍合法 |
| 左值到右值转换 | `int y = x;` | 仍合法 |

### 3. 为什么弃用

- volatile 的复合操作不是原子的
- 程序员误以为 volatile 能保证线程安全（其实不能）
- `std::atomic` 才是正确的多线程同步工具

#### 1. 典型误用示例

```cpp
volatile int counter = 0;

// ❌ 错误的并发用法
void thread1() { for (int i = 0; i < 1000; i++) counter++; }
void thread2() { for (int i = 0; i < 1000; i++) counter++; }
// 期望：2000，实际可能远小于 2000（数据竞争）

// ✅ 正确的并发写法
std::atomic<int> counter{0};
void thread1() { for (int i = 0; i < 1000; i++) counter++; }
void thread2() { for (int i = 0; i < 1000; i++) counter++; }
// 结果：始终为 2000
```

### 4. volatile vs atomic

| 特性 | volatile | std::atomic |
|:---|:---:|:---:|
| 禁止编译器优化 | ✅ | ✅ |
| 原子性 | ❌ | ✅ |
| 内存序控制 | ❌ | ✅（relaxed/acquire/release/seq_cst） |
| 多线程安全 | ❌ | ✅ |
| 硬件寄存器访问 | ✅ | ❌ |
| 信号处理安全 | ✅（部分） | ❌（通常不能用于信号处理） |
| 禁止指令重排（编译器） | ✅（仅 volatile 操作间） | ✅（可配置内存序） |
| 禁止指令重排（CPU） | ❌ | ✅ |

#### 1. 内存序对比示例

```cpp
std::atomic<int> flag{0};
int data = 0;

// 线程1
data = 42;
flag.store(1, std::memory_order_release);
// release 语义：之前的所有写操作在 flag 写入前对其他线程可见

// 线程2
while (flag.load(std::memory_order_acquire) == 0);
int x = data;  // 保证读到 42
// acquire 语义：flag 读取之后，能看到 release 之前的所有写操作

// 用 volatile 做不到这种内存序保证！
```

### 5. 什么时候还用 volatile

#### 1. 硬件寄存器映射

```cpp
// 内存映射 I/O（MMIO）是 volatile 的核心用途
volatile uint32_t* GPIO = (volatile uint32_t*)0x40020000;

// 连续读取状态寄存器（每次必须真正读硬件）
while (GPIO[0] & 0x01) {  // volatile 确保每次都读
    // 等待硬件置位
}
```

#### 2. 信号处理

```cpp
#include <csignal>
#include <atomic>

// 信号处理函数中正确的做法
volatile std::sig_atomic_t signal_received = 0;

extern "C" void handler(int sig) {
    signal_received = 1;  // 信号处理中只能修改 volatile sig_atomic_t
}

int main() {
    std::signal(SIGINT, handler);
    while (!signal_received) {
        // 等待信号
    }
}
```

#### 3. setjmp/longjmp 跨跳转变量

```cpp
#include <csetjmp>
jmp_buf env;

void func() {
    volatile int local_var = 42;  // volatile 防止 longjmp 后值不确定
    if (setjmp(env) == 0) {
        local_var = 100;
        longjmp(env, 1);  // 跳回 setjmp
    }
    // 没有 volatile，local_var 的值在 longjmp 后是未定义的
    int x = local_var;  // volatile 保证读到正确的值
}
```

### 6. C++11 后的变化总结

| 版本 | 变化 |
|:---|:---|
| C++98/03 | volatile 用于多线程和硬件 |
| C++11 | 引入 `std::atomic`，多线程推荐用 atomic |
| C++14 | 无显著变化 |
| C++17 | 强化 inline 变量，volatile 不变 |
| C++20 | 弃用 volatile 复合赋值/自增自减 |

### 7. 极简总结

**volatile 没被删除 → C++20 弃用复合操作 → 多线程用 atomic → 硬件交互用 volatile**

***

### 相关阅读

- [什么是未定义行为](./07-什么是未定义行为.md)
- [volatile关键字](./12-volatile关键字.md)
- [_Atomic与C11原子操作](./27-什么是-Atomic与C11原子操作.md)