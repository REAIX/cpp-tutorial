# std::function 与函数指针与 Lambda 的区别
> 📖 相关章节：[多线程基础](../../02-CPP/27-多线程基础.md)、[线程同步](../../02-CPP/28-线程同步.md)、[原子操作](../../02-CPP/29-原子操作与异步编程.md)、[无锁编程](../../02-CPP/33-无锁编程.md)

> "函数指针是固定电话，Lambda 是对讲机，std::function 是万能转接器"——三种可调用对象，各有各的用武之地。

***

### 1. 核心定义

| 概念 | 定义 | 一句话 |
|------|------|------|
| **函数指针** | 指向函数的指针，C 风格回调 | 固定电话——只能接一种线 |
| **Lambda** | 匿名函数对象，C++11 引入 | 对讲机——灵活但范围有限 |
| **std::function** | 类型擦除的可调用对象包装器，C++11 引入 | 万能转接器——什么都能接但稍慢 |

***

### 2. 生活类比

**电话系统**：

- **函数指针 = 固定电话**：只能接一种线（函数签名必须完全匹配），接上线就能通话（调用），速度快，但灵活性差
- **Lambda = 对讲机**：自带电池（捕获变量），灵活便携，但每个对讲机型号不同（每个 Lambda 类型唯一），不能混用
- **std::function = 万能转接器**：什么电话都能接（函数指针、Lambda、仿函数），但多了一层转接（类型擦除开销），稍慢

| 类比 | 函数指针 | Lambda | std::function |
|------|------|------|------|
| 设备 | 固定电话 | 对讲机 | 万能转接器 |
| 灵活性 | 低 | 中 | 高 |
| 速度 | 最快 | 快 | 稍慢 |
| 能否捕获变量 | ❌ | ✅ | ✅（间接） |

***

### 3. 同一功能用三种方式实现

**需求**：计算两个数的运算结果，运算方式由调用方决定。

#### 1. 方式1：函数指针

```cpp
#include <iostream>

int add(int a, int b) { return a + b; }
int multiply(int a, int b) { return a * b; }

int compute(int a, int b, int (*op)(int, int)) {
    return op(a, b);
}

int main() {
    std::cout << compute(3, 4, add) << std::endl;
    std::cout << compute(3, 4, multiply) << std::endl;
}
```

#### 2. 方式2：Lambda

```cpp
#include <iostream>
#include <algorithm>

int main() {
    auto add = [](int a, int b) { return a + b; };
    auto multiply = [](int a, int b) { return a * b; };

    int base = 10;
    auto add_with_base = [base](int a, int b) { return a + b + base; };

    std::cout << add(3, 4) << std::endl;
    std::cout << multiply(3, 4) << std::endl;
    std::cout << add_with_base(3, 4) << std::endl;
}
```

#### 3. 方式3：std::function

```cpp
#include <iostream>
#include <functional>
#include <vector>

int compute(int a, int b, const std::function<int(int, int)>& op) {
    return op(a, b);
}

int main() {
    int base = 10;

    std::function<int(int, int)> add = [](int a, int b) { return a + b; };
    std::function<int(int, int)> multiply = [](int a, int b) { return a * b; };
    std::function<int(int, int)> add_with_base = [base](int a, int b) { return a + b + base; };

    std::cout << compute(3, 4, add) << std::endl;
    std::cout << compute(3, 4, multiply) << std::endl;
    std::cout << compute(3, 4, add_with_base) << std::endl;

    std::vector<std::function<int(int, int)>> ops = {add, multiply, add_with_base};
    for (auto& op : ops) {
        std::cout << compute(5, 6, op) << std::endl;
    }
}
```

**关键区别**：`std::function` 能把不同类型的 Lambda 放进同一个容器，函数指针和裸 Lambda 都做不到。

***

### 4. 性能对比

```cpp
#include <iostream>
#include <functional>
#include <chrono>

volatile int sink = 0;

int add_func(int a, int b) { return a + b; }

void bench_function_pointer(int iterations) {
    int (*op)(int, int) = add_func;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        sink = op(i, i + 1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - start).count();
    std::cout << "function pointer: " << ns / iterations << " ns/call" << std::endl;
}

void bench_lambda(int iterations) {
    auto op = [](int a, int b) { return a + b; };
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        sink = op(i, i + 1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::nanoseconds>(end - start).count();
    std::cout << "lambda:          " << ns / iterations << " ns/call" << std::endl;
}

void bench_std_function(int iterations) {
    std::function<int(int, int)> op = [](int a, int b) { return a + b; };
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        sink = op(i, i + 1);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::nanoseconds>(end - start).count();
    std::cout << "std::function:   " << ns / iterations << " ns/call" << std::endl;
}

int main() {
    const int N = 10'000'000;
    bench_function_pointer(N);
    bench_lambda(N);
    bench_std_function(N);
}
```

典型结果（相对值）：

| 方式 | 相对耗时 | 原因 |
|------|:---:|------|
| 函数指针 | 1x（基准） | 直接调用，可内联 |
| Lambda | ~1x | 编译器内联，零开销 |
| std::function | ~2~5x | 类型擦除 + 间接调用 + 可能堆分配 |

**性能排序**：Lambda ≈ 函数指针 >> std::function

**Lambda 为什么和函数指针一样快？** 因为 Lambda 的类型在编译期确定，编译器可以完全内联。`std::function` 运行时才知道具体类型，无法内联。

***

### 5. 灵活性对比

#### 1. 捕获变量的能力

```cpp
int threshold = 42;

auto lambda = [threshold](int x) { return x > threshold; };

bool (*fp)(int) = [](int x) { return x > 42; };

std::function<bool(int)> func = [threshold](int x) { return x > threshold; };
```

| 能力 | 函数指针 | Lambda | std::function |
|------|:---:|:---:|:---:|
| 捕获局部变量 | ❌ | ✅ | ✅ |
| 无捕获时转函数指针 | — | ✅ | ❌ |
| 存储不同类型可调用对象 | ❌ | ❌ | ✅ |
| 运行时替换回调 | ❌ | ❌ | ✅ |
| 放入容器 | ❌（类型相同才行） | ❌（每个 Lambda 类型不同） | ✅ |

#### 2. 异构容器：std::function 的杀手锏

```cpp
#include <iostream>
#include <functional>
#include <vector>

int main() {
    int x = 10;

    std::vector<std::function<void()>> tasks;

    tasks.push_back([] { std::cout << "task 1" << std::endl; });
    tasks.push_back([x] { std::cout << "task 2, x=" << x << std::endl; });
    tasks.push_back([] { std::cout << "task 3" << std::endl; });

    for (auto& t : tasks) {
        t();
    }
}
```

函数指针做不到：有捕获的 Lambda 不能转函数指针。裸 Lambda 做不到：每个 Lambda 类型唯一，不能放进同一容器。

***

### 6. 对比表格

| 维度 | 函数指针 | Lambda | std::function |
|------|------|------|------|
| **类型安全** | ⚠️ 弱（只有签名约束） | ✅ 强（编译器完整检查） | ✅ 强（签名约束） |
| **捕获变量** | ❌ 不能 | ✅ 能（值/引用捕获） | ✅ 能（间接） |
| **性能开销** | ✅ 零（直接调用） | ✅ 零（可内联） | ⚠️ 有（间接调用+可能堆分配） |
| **灵活性** | ❌ 低 | ⚠️ 中（类型唯一） | ✅ 高（类型擦除） |
| **C++ 版本** | C 语言就有 | C++11 | C++11 |
| **可内联** | ✅ 可能 | ✅ 通常可以 | ❌ 不能 |
| **异构容器** | ⚠️ 同签名可以 | ❌ 每个 Lambda 类型不同 | ✅ 可以 |
| **运行时替换** | ✅ 可以（改指针） | ❌ 不可以（auto 固定类型） | ✅ 可以（重新赋值） |
| **适合场景** | C 接口回调 | 局部算法、STL 谓词 | 事件系统、回调注册 |

***

### 7. 适用场景选择

#### 1. 用函数指针

- C 语言接口回调（如 `qsort` 的比较函数）
- 函数签名固定、不需要捕获变量
- 性能极度敏感的热路径

```cpp
void c_api_callback(int event, void (*handler)(int));

void my_handler(int event) {
    std::cout << "event: " << event << std::endl;
}

c_api_callback(1, my_handler);
```

#### 2. 用 Lambda

- STL 算法的谓词/比较器
- 局部一次性逻辑
- 需要捕获上下文变量
- 性能敏感场景

```cpp
std::vector<int> v = {5, 2, 8, 1, 9};
int threshold = 5;

auto it = std::find_if(v.begin(), v.end(),
    [threshold](int x) { return x > threshold; });
```

#### 3. 用 std::function

- 事件系统/信号槽
- 回调注册（运行时动态替换）
- 需要存储不同类型的可调用对象
- 插件/策略模式

```cpp
class Button {
public:
    void set_on_click(std::function<void()> callback) {
        on_click_ = std::move(callback);
    }

    void click() {
        if (on_click_) on_click_();
    }

private:
    std::function<void()> on_click_;
};

int main() {
    Button btn;
    int count = 0;
    btn.set_on_click([&count] { std::cout << "clicked " << ++count << std::endl; });
    btn.click();
    btn.click();
}
```

***

### 8. 极简总结

**函数指针 = C 风格回调，Lambda = 匿名函数对象，std::function = 万能包装器**

| 选择原则 | 推荐 |
|------|------|
| C 接口 / 不需要捕获 / 极致性能 | 函数指针 |
| STL 算法 / 局部逻辑 / 需要捕获 | Lambda |
| 事件系统 / 异构容器 / 运行时替换 | std::function |
| 一句话 | 默认用 Lambda，需要统一类型时用 std::function，C 接口用函数指针 |

***

### 相关阅读

- [pthread-create函数指针参数](10-pthread-create函数指针参数.md)
- [多线程函数接口](24-多线程函数接口.md)
- [什么是回调函数](../04-CPP核心特性/27-什么是回调函数.md)

***