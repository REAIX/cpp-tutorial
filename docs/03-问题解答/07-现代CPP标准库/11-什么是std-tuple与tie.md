# 什么是std::tuple与tie
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[STL算法](../../02-CPP/15-STL算法与迭代器.md)、[正则表达式](../../02-CPP/16-正则表达式.md)、[文件系统](../../02-CPP/19-文件系统库.md)

> "A tuple is a fixed-size collection of heterogeneous values — the Swiss Army knife of C++ type machinery." — Stephan T. Lavavej

***

### 1. 先抓核心

`std::tuple` 是固定大小的异构容器，可存储不同类型的元素；`std::tie` 用于解包 tuple 或构建可比较的元组引用，两者配合实现多返回值和字典序比较。

***

### 2. tuple 基础：异构容器与创建方式

`std::tuple` 是标准库提供的固定大小异构容器，每个元素可以是不同类型。

```cpp
#include <tuple>
#include <string>
#include <iostream>

int main() {
    std::tuple<int, double, std::string> t1(42, 3.14, "hello");

    auto t2 = std::make_tuple(1, 2.0, std::string("world"));

    auto t3 = std::make_tuple(100);

    auto t4 = std::tuple<int, int, int>(1, 2, 3);

    std::cout << std::get<0>(t1) << "\n";
    std::cout << std::get<1>(t1) << "\n";
    std::cout << std::get<2>(t1) << "\n";

    std::cout << std::get<std::string>(t1) << "\n";
}
```

创建方式汇总：

| 方式 | 代码 | 说明 |
|------|------|------|
| 构造函数 | `tuple<T1,T2>(v1,v2)` | 显式指定类型 |
| `make_tuple` | `make_tuple(v1,v2)` | 自动推导类型 |
| 花括号初始化 | `tuple{v1,v2}` | C++17 CTAD 推导 |
| `forward_as_tuple` | `forward_as_tuple(v1,v2)` | 创建引用元组（完美转发） |
| `tie` | `tie(a,b,c)` | 创建引用元组（左值引用） |

```cpp
auto t1 = std::tuple(1, 2.0, "hi");

int x = 10;
double y = 3.14;
auto t2 = std::forward_as_tuple(x, y);

std::get<0>(t2) = 99;
std::cout << x << "\n";
```

***

### 3. get\<I\> 访问元素

`std::get<I>` 是按索引访问 tuple 元素的标准方式，编译期检查索引合法性。

```cpp
#include <tuple>
#include <string>
#include <iostream>

int main() {
    auto t = std::make_tuple(42, 3.14, std::string("hello"));

    int& first = std::get<0>(t);
    first = 100;

    double second = std::get<1>(t);

    std::string& third = std::get<2>(t);

    std::cout << first << ", " << second << ", " << third << "\n";
}
```

C++14 起，`std::get` 也支持按类型访问（当类型唯一时）：

```cpp
auto t = std::make_tuple(42, 3.14, std::string("hello"));

int n = std::get<int>(t);
double d = std::get<double>(t);
std::string s = std::get<std::string>(t);

// auto bad = std::get<int>(std::make_tuple(1, 2)); // 编译错误：int 不唯一
```

`get<I>` 的特性：

| 特性 | 说明 |
|------|------|
| 编译期检查 | 索引越界直接编译错误 |
| 返回引用 | `get<I>(t)` 返回 `T&`，`get<I>(const t)` 返回 `const T&` |
| 按类型访问 | C++14 起，类型必须唯一 |
| 移动语义 | `get<I>(std::move(t))` 返回 `T&&` |

```cpp
auto make_data() {
    return std::make_tuple(1, std::string("data"), 3.14);
}

int main() {
    auto t = make_data();
    int n = std::get<0>(std::move(t));
}
```

***

### 4. 结构化绑定 vs get

C++17 的结构化绑定是解包 tuple 的更优雅方式，替代了手动 `get`。

```cpp
#include <tuple>
#include <string>
#include <iostream>

auto get_person() {
    return std::make_tuple(std::string("Alice"), 25, std::string("Engineer"));
}

int main() {
    // 方式一：逐个 get
    auto t = get_person();
    std::string name1 = std::get<0>(t);
    int age1 = std::get<1>(t);
    std::string job1 = std::get<2>(t);

    // 方式二：结构化绑定（推荐）
    auto [name2, age2, job2] = get_person();
    std::cout << name2 << ", " << age2 << ", " << job2 << "\n";

    // 结构化绑定 + const
    const auto [cname, cage, cjob] = get_person();

    // 结构化绑定 + 引用
    auto& [rname, rage, rjob] = t;
    rage = 26;
}
```

结构化绑定 vs `get` 对比：

| 对比维度 | `std::get<I>` | 结构化绑定 |
|----------|--------------|-----------|
| 语法 | `get<0>(t)`, `get<1>(t)` | `auto [a, b, c] = t` |
| 可读性 | 需记住索引 | 语义化命名 |
| 代码量 | 多 | 少 |
| 灵活性 | 可单独访问某个 | 一次解包全部 |
| 修改元素 | `get<0>(t) = val` | `auto& [a,b,c] = t; a = val` |
| C++ 版本 | C++11 | C++17 |

```cpp
// 结构化绑定在循环中的使用
#include <map>

int main() {
    std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}};

    for (const auto& [name, score] : scores) {
        std::cout << name << ": " << score << "\n";
    }
}
```

***

### 5. std::tie —— 解包与字典序比较

`std::tie` 创建一个左值引用的 tuple，主要用于解包返回值和实现字典序比较。

**解包 tuple 返回值：**

```cpp
#include <tuple>
#include <string>
#include <iostream>

auto get_info() {
    return std::make_tuple(std::string("Alice"), 25, 3.8);
}

int main() {
    std::string name;
    int age;
    double gpa;

    std::tie(name, age, gpa) = get_info();
    std::cout << name << ", " << age << ", " << gpa << "\n";

    // 忽略某些返回值
    std::string name2;
    std::tie(name2, std::ignore, std::ignore) = get_info();
    std::cout << name2 << "\n";
}
```

**字典序比较：**

```cpp
#include <tuple>
#include <string>

struct Person {
    std::string name;
    int age;
    double height;

    bool operator<(const Person& other) const {
        return std::tie(name, age, height) < std::tie(other.name, other.age, other.height);
    }

    bool operator==(const Person& other) const {
        return std::tie(name, age, height) == std::tie(other.name, other.age, other.height);
    }
};

int main() {
    Person a{"Alice", 25, 1.65};
    Person b{"Alice", 25, 1.70};
    Person c{"Bob", 20, 1.60};

    bool r1 = a < b;  // true（height 1.65 < 1.70）
    bool r2 = a < c;  // true（name "Alice" < "Bob"）
    bool r3 = a == b; // false
}
```

`tie` vs 结构化绑定对比：

| 用途 | `std::tie` | 结构化绑定 |
|------|-----------|-----------|
| 解包 | `tie(a,b,c) = t` | `auto [a,b,c] = t` |
| 忽略值 | `tie(a,ignore,c)` | ❌ 不支持 |
| 字典序比较 | `tie(x,y) < tie(ox,oy)` | ❌ 不适用 |
| 需要预声明 | ✅ 需要 | ❌ 不需要 |
| C++ 版本 | C++11 | C++17 |

***

### 6. tuple_cat —— 拼接多个 tuple

`std::tuple_cat` 将多个 tuple 拼接为一个新 tuple。

```cpp
#include <tuple>
#include <string>
#include <iostream>

int main() {
    auto t1 = std::make_tuple(1, 2);
    auto t2 = std::make_tuple(3.0, 4.0);
    auto t3 = std::make_tuple(std::string("five"));

    auto combined = std::tuple_cat(t1, t2, t3);

    std::cout << std::get<0>(combined) << "\n";
    std::cout << std::get<2>(combined) << "\n";
    std::cout << std::get<4>(combined) << "\n";

    auto [a, b, c, d, e] = combined;
    std::cout << a << ", " << c << ", " << e << "\n";
}
```

实际应用——合并查询结果：

```cpp
auto query_user(int id) {
    return std::make_tuple(std::string("Alice"), 25);
}

auto query_orders(int id) {
    return std::make_tuple(1001, 1002);
}

auto query_settings(int id) {
    return std::make_tuple(true, std::string("dark"));
}

int main() {
    auto user = query_user(1);
    auto orders = query_orders(1);
    auto settings = query_settings(1);

    auto all = std::tuple_cat(user, orders, settings);
    // all: tuple<string, int, int, int, bool, string>
}
```

***

### 7. std::apply —— 将 tuple 展开为函数参数

`std::apply` 将 tuple 的元素展开为函数的参数列表调用。

```cpp
#include <tuple>
#include <iostream>
#include <string>

int add(int a, int b, int c) {
    return a + b + c;
}

void print_info(const std::string& name, int age, double score) {
    std::cout << name << ", " << age << ", " << score << "\n";
}

int main() {
    auto args = std::make_tuple(1, 2, 3);
    int result = std::apply(add, args);
    std::cout << result << "\n";

    auto info = std::make_tuple(std::string("Bob"), 30, 95.5);
    std::apply(print_info, info);
}
```

与成员函数结合：

```cpp
#include <tuple>
#include <iostream>

class Calculator {
public:
    double compute(double a, double b, double c) const {
        return a * b + c;
    }
};

int main() {
    Calculator calc;
    auto args = std::make_tuple(2.0, 3.0, 1.0);

    double result = std::apply(&Calculator::compute, std::tuple_cat(std::make_tuple(&calc), args));
    std::cout << result << "\n";
}
```

`apply` 的特性：

| 特性 | 说明 |
|------|------|
| C++ 版本 | C++17 |
| 参数来源 | tuple 的元素按顺序展开 |
| 返回值 | 返回函数的返回值 |
| 成员函数 | 需要将对象指针作为第一个参数 |
| 完美转发 | 内部使用 `std::invoke` |

***

### 8. tuple_size 与 tuple_element —— 编译期元信息

`std::tuple_size` 和 `std::tuple_element` 提供编译期查询 tuple 大小和元素类型的能力。

```cpp
#include <tuple>
#include <string>
#include <type_traits>
#include <iostream>

int main() {
    using MyTuple = std::tuple<int, double, std::string>;

    constexpr std::size_t size = std::tuple_size<MyTuple>::value;
    std::cout << "元素个数: " << size << "\n";

    using FirstType = std::tuple_element<0, MyTuple>::type;
    using SecondType = std::tuple_element<1, MyTuple>::type;
    using ThirdType = std::tuple_element<2, MyTuple>::type;

    static_assert(std::is_same_v<FirstType, int>);
    static_assert(std::is_same_v<SecondType, double>);
    static_assert(std::is_same_v<ThirdType, std::string>);

    constexpr std::size_t size_v = std::tuple_size_v<MyTuple>;
    using FirstType_t = std::tuple_element_t<0, MyTuple>;
}
```

编译期遍历 tuple 的经典模式：

```cpp
#include <tuple>
#include <iostream>
#include <string>

template <typename Tuple, std::size_t... Is>
void print_tuple_impl(const Tuple& t, std::index_sequence<Is...>) {
    ((std::cout << (Is == 0 ? "" : ", ") << std::get<Is>(t)), ...);
    std::cout << "\n";
}

template <typename... Args>
void print_tuple(const std::tuple<Args...>& t) {
    print_tuple_impl(t, std::index_sequence_for<Args...>{});
}

int main() {
    auto t = std::make_tuple(42, 3.14, std::string("hello"));
    print_tuple(t);
}
```

元信息工具汇总：

| 工具 | 作用 | C++ 版本 |
|------|------|----------|
| `tuple_size<T>::value` | 元素个数 | C++11 |
| `tuple_size_v<T>` | 元素个数（变量模板） | C++17 |
| `tuple_element<I,T>::type` | 第 I 个元素的类型 | C++11 |
| `tuple_element_t<I,T>` | 第 I 个元素的类型（别名模板） | C++14 |

***

### 9. 多返回值的最佳实践

tuple 最常见的用途是函数返回多个值。以下是各种方式的对比和推荐。

```cpp
#include <tuple>
#include <string>
#include <optional>
#include <iostream>

// 方式一：tuple 返回
std::tuple<int, std::string, double> get_data_tuple() {
    return {42, "hello", 3.14};
}

// 方式二：结构体返回（推荐）
struct Data {
    int id;
    std::string name;
    double score;
};

Data get_data_struct() {
    return {42, "hello", 3.14};
}

// 方式三：输出参数
void get_data_out(int& id, std::string& name, double& score) {
    id = 42;
    name = "hello";
    score = 3.14;
}

int main() {
    // tuple + 结构化绑定
    auto [id1, name1, score1] = get_data_tuple();

    // 结构体（最推荐）
    auto d = get_data_struct();
    std::cout << d.id << ", " << d.name << ", " << d.score << "\n";

    // 输出参数（不推荐）
    int id2;
    std::string name2;
    double score2;
    get_data_out(id2, name2, score2);
}
```

多返回值方式对比：

| 方式 | 可读性 | 类型安全 | 性能 | 推荐度 |
|------|--------|---------|------|--------|
| 结构体 | ⭐⭐⭐ | ✅ | 最优 | ⭐⭐⭐ |
| tuple + 结构化绑定 | ⭐⭐ | ✅ | 优 | ⭐⭐ |
| tuple + get | ⭐ | ✅ | 优 | ⭐ |
| 输出参数 | ⭐ | ✅ | 优 | ❌ |

选择指南：

```
返回值有明确语义？── 是 → 用结构体，字段有名字
                    └── 否 → 用 tuple + 结构化绑定
仅内部临时使用？── 是 → tuple 足够
                  └── 否 → 结构体更清晰
需要序列化/文档化？── 是 → 结构体
                      └── 否 → 两者皆可
```

***

### 10. 极简总结

| 概念 | 关键语法 | 核心用途 |
|------|----------|----------|
| `std::tuple` | `tuple<T1,T2,...>` | 异构固定大小容器 |
| `std::get<I>` | `get<0>(t)` / `get<int>(t)` | 按索引或类型访问元素 |
| 结构化绑定 | `auto [a,b,c] = t` | C++17 优雅解包 |
| `std::tie` | `tie(a,b,c) = t` | 解包到已有变量、字典序比较 |
| `std::ignore` | `tie(a,ignore,c)` | 忽略某个返回值 |
| `std::tuple_cat` | `tuple_cat(t1,t2)` | 拼接多个 tuple |
| `std::apply` | `apply(func, t)` | 将 tuple 展开为函数参数 |
| `tuple_size` | `tuple_size_v<T>` | 编译期获取元素个数 |
| `tuple_element` | `tuple_element_t<I,T>` | 编译期获取元素类型 |
| 多返回值 | `auto [a,b] = f()` | 函数返回多个值 |

核心记忆：**tuple 存异构值，get 取元素，tie 做比较，apply 做分发，结构化绑定最优雅**。

***

### 相关阅读

- [什么是结构化绑定Structured-Binding](./12-什么是结构化绑定Structured-Binding.md)
- [variant与union](./04-variant与union.md)
- [STL容器底层实现](./00-STL容器底层实现.md)

***