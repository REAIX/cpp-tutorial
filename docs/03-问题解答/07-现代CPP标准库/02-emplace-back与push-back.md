# emplace_back 与 push_back 的区别
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[STL算法](../../02-CPP/15-STL算法与迭代器.md)、[正则表达式](../../02-CPP/16-正则表达式.md)、[文件系统](../../02-CPP/19-文件系统库.md)

> "先在走廊组装家具再搬进房间，还是直接在房间里组装"——emplace_back 原地构造，省去中间搬运。

***

### 1. 核心定义

- **push_back** = 先构造一个临时对象，再移动/拷贝到容器中
- **emplace_back** = 直接在容器的内存位置上构造对象，无需临时对象

关键点：**emplace_back 省去了临时对象的构造和析构开销**。

***

### 2. 生活类比

**搬家具进房间**：

| 操作 | 类比 | 对应代码 |
|------|------|---------|
| push_back | 在走廊组装好家具 → 搬进房间 | 先构造临时对象 → 移动/拷贝到容器 |
| emplace_back | 直接把零件搬进房间 → 在房间里组装 | 直接在容器内存上构造对象 |

走廊组装（push_back）的问题：
1. 走廊空间有限（临时对象生命周期短）
2. 搬运过程可能磕碰（移动/拷贝开销）
3. 组装完还要清理走廊（析构临时对象）

房间内组装（emplace_back）：
1. 一步到位，没有搬运
2. 没有磕碰风险（无移动/拷贝）
3. 没有清理工作（无临时对象析构）

***

### 3. 代码对比

#### 1. 基础对比

```cpp
#include <iostream>
#include <vector>
#include <string>

class Player {
    std::string name_;
    int level_;
public:
    Player(const std::string& name, int level)
        : name_(name), level_(level) {
        std::cout << "  构造: " << name_ << "\n";
    }
    Player(const Player& other)
        : name_(other.name_), level_(other.level_) {
        std::cout << "  拷贝: " << name_ << "\n";
    }
    Player(Player&& other) noexcept
        : name_(std::move(other.name_)), level_(other.level_) {
        std::cout << "  移动: " << name_ << "\n";
    }
    ~Player() {
        std::cout << "  析构: " << name_ << "\n";
    }
};

int main() {
    std::cout << "=== push_back ===\n";
    {
        std::vector<Player> v;
        v.reserve(4);
        std::cout << "[1] push_back(Player(\"Alice\", 1)):\n";
        v.push_back(Player("Alice", 1));
        // 构造临时对象 → 移动到容器 → 析构临时对象

        std::cout << "[2] push_back({\"Bob\", 2}):\n";
        v.push_back({"Bob", 2});
        // 同上：构造临时对象 → 移动 → 析构
    }

    std::cout << "\n=== emplace_back ===\n";
    {
        std::vector<Player> v;
        v.reserve(4);
        std::cout << "[1] emplace_back(\"Charlie\", 3):\n";
        v.emplace_back("Charlie", 3);
        // 直接在容器内存上构造，无临时对象

        std::cout << "[2] emplace_back(std::string(\"Dave\"), 4):\n";
        v.emplace_back(std::string("Dave"), 4);
        // 传了已构造的 string，仍需拷贝/移动 string
    }
}
```

**典型输出**：

```
=== push_back ===
[1] push_back(Player("Alice", 1)):
  构造: Alice       ← 构造临时对象
  移动: Alice       ← 移动到容器
  析构:             ← 析构临时对象（name 已被 move 走）
[2] push_back({"Bob", 2}):
  构造: Bob
  移动: Bob
  析构:

=== emplace_back ===
[1] emplace_back("Charlie", 3):
  构造: Charlie     ← 直接构造，无移动，无临时对象析构
[2] emplace_back(std::string("Dave"), 4):
  构造: Dave        ← Player 直接构造，但 string 参数是传入的
```

#### 2. 参数传递对比

```cpp
std::vector<std::string> v;

v.push_back("Hello");
// "Hello" (const char*) → 构造临时 string → 移动到容器 → 析构临时

v.emplace_back("Hello");
// "Hello" (const char*) → 直接在容器内存上构造 string

v.push_back(std::string("World"));
// 构造临时 string → 移动到容器 → 析构临时

v.emplace_back(std::string("World"));
// 构造临时 string → 移动到容器 → 析构临时
// 效果和 push_back 一样！因为参数已经是 string 对象
```

***

### 4. 性能差异

#### 1. 差异来源

```
push_back 的开销链：
  构造临时对象 → 移动/拷贝到容器 → 析构临时对象
  （3 步）

emplace_back 的开销链：
  直接在容器内存上构造
  （1 步）
```

#### 2. 何时差异明显

| 场景 | 差异程度 | 原因 |
|------|:---:|------|
| 对象有昂贵的移动/拷贝 | 大 | 省去了移动/拷贝开销 |
| 对象移动很廉价（如 int） | 几乎无 | 移动本身就是拷贝几个字节 |
| 参数已经是对象类型 | 几乎无 | emplace_back 也无法避免参数的构造 |
| 大量插入操作 | 累积明显 | 单次差异小，但次数多 |

#### 3. 性能测试

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <chrono>

struct HeavyObject {
    std::string data;
    int id;
    double value;
    HeavyObject(const std::string& s, int i, double v)
        : data(s), id(i), value(v) {}
    HeavyObject(const HeavyObject&) = default;
    HeavyObject(HeavyObject&&) = default;
};

int main() {
    const int N = 1'000'000;

    auto start = std::chrono::high_resolution_clock::now();
    std::vector<HeavyObject> v1;
    v1.reserve(N);
    for (int i = 0; i < N; ++i) {
        v1.push_back(HeavyObject("test", i, i * 3.14));
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto push_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    start = std::chrono::high_resolution_clock::now();
    std::vector<HeavyObject> v2;
    v2.reserve(N);
    for (int i = 0; i < N; ++i) {
        v2.emplace_back("test", i, i * 3.14);
    }
    end = std::chrono::high_resolution_clock::now();
    auto emplace_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "push_back:   " << push_ms << "ms\n";
    std::cout << "emplace_back: " << emplace_ms << "ms\n";
}
```

**典型结果**：`emplace_back` 比 `push_back` 快 10%~30%（取决于对象复杂度和编译器优化）。

***

### 5. 何时用哪个

| 情况 | 推荐 | 原因 |
|------|------|------|
| 有构造函数参数，想直接构造 | `emplace_back` | 省去临时对象 |
| 已有现成对象要插入 | 都可以 | 差异极小 |
| 插入 `int`、`double` 等基本类型 | 都可以 | 无移动/拷贝开销差异 |
| 插入 `std::string` 字面量 | `emplace_back` | 省去临时 string 构造 |
| 需要显式类型转换 | `push_back` | 更安全，避免隐式转换 |
| 代码可读性优先 | `push_back` | 类型明确，意图清晰 |

**简单原则**：能用 `emplace_back` 就用，除非需要防止隐式转换。

***

### 6. 常见误区

#### 1. 误区1：emplace_back 永远不会触发拷贝

```cpp
std::vector<std::string> v;
std::string s = "Hello";

v.emplace_back(s);  // 触发拷贝！s 是左值
v.emplace_back(std::move(s));  // 移动，s 变为空
```

`emplace_back` 只是"原地构造"，但如果传入的是左值，构造函数仍然走拷贝语义。

**emplace_back 省的是"临时对象的构造+移动+析构"，不是省拷贝本身**。

#### 2. 误区2：emplace_back 总是更快

```cpp
std::vector<int> v;
v.push_back(42);      // 拷贝 int，4 字节
v.emplace_back(42);   // 也是拷贝 int，4 字节
// 性能完全一样
```

对于平凡类型（int、double、指针等），两者没有性能差异。

#### 3. 误区3：emplace_back 的参数会自动推导类型

```cpp
class Widget {
public:
    Widget(int x);
    explicit Widget(std::string s);  // explicit!
};

std::vector<Widget> v;

v.push_back("Hello");   // 编译错误！push_back 也需要显式转换
v.emplace_back("Hello"); // 编译通过！emplace_back 直接传参给构造函数
// 但这可能不是你想要的——隐式 const char* → string → Widget
```

`emplace_back` 会尝试所有构造函数（包括隐式转换），可能导致意外的构造函数调用。

**安全做法**：

```cpp
v.emplace_back(std::string("Hello"));  // 显式构造 string
```

#### 4. 误区4：emplace_back 可以替代 push_back 的所有场景

```cpp
std::vector<std::vector<int>> v;

v.push_back({1, 2, 3});  // OK：初始化列表
v.emplace_back({1, 2, 3}); // 编译错误！初始化列表不能直接转发
```

`emplace_back` 使用完美转发，初始化列表 `{1, 2, 3}` 无法被推导为 `std::initializer_list`。

**修复**：

```cpp
v.emplace_back(std::vector<int>{1, 2, 3});  // 显式构造
```

***

### 7. 与 RVO/NRVO 的关系

[RVO与NRVO返回值优化](../04-CPP核心特性/16-RVO与NRVO返回值优化.md) 讲过 RVO/NRVO 的原理。两者都致力于消除不必要的拷贝，但作用层面不同：

| 维度 | RVO/NRVO | emplace_back |
|------|----------|-------------|
| 消除的拷贝 | 函数返回值 → 调用者 | 临时对象 → 容器内部 |
| 作用时机 | 函数返回时 | 容器插入时 |
| 实现方式 | 编译器在目标位置直接构造 | 容器在预留位置直接构造 |
| 保证级别 | C++17 保证 RVO | C++11 起始终有效 |

**它们可以叠加**：

```cpp
std::vector<std::string> v;

std::string create_string();  // RVO 优化返回值

v.push_back(create_string());
// RVO: 返回值直接构造在临时位置 → 移动到容器

v.emplace_back(create_string());
// RVO: 返回值直接构造在临时位置 → 移动到容器
// 效果一样！因为参数已经是 string 对象

v.emplace_back("Direct");
// 无 RVO 参与，但直接在容器位置构造 string
// 这才是 emplace_back 的优势场景
```

**结论**：RVO 消除"返回值拷贝"，emplace_back 消除"插入时拷贝"，两者互补。

***

### 8. 极简总结

**emplace_back = 原地构造，push_back = 先构造再搬入**

| 要点 | 说明 |
|------|------|
| 核心区别 | emplace_back 直接传构造参数，push_back 传已构造的对象 |
| 性能收益 | 省去临时对象的构造+移动+析构，对复杂对象明显 |
| 误区 | emplace_back 传左值仍会拷贝；不是所有场景都快 |
| 安全提示 | emplace_back 可能触发隐式转换；initializer_list 需显式构造 |
| 一句话 | 有构造参数用 emplace_back，已有对象用 push_back 也行 |

***

### 相关阅读

- [STL容器底层实现](./00-STL容器底层实现.md)
- [optional与nullptr](./03-optional与nullptr.md)
- [variant与union](./04-variant与union.md)

***