# 什么是结构化绑定 Structured Binding
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[STL算法](../../02-CPP/15-STL算法与迭代器.md)、[正则表达式](../../02-CPP/16-正则表达式.md)、[文件系统](../../02-CPP/19-文件系统库.md)

> "让多返回值不再痛苦，一行代码解包一切。" —— C++17

***

### 1. 一句话概括

结构化绑定（Structured Binding）是 C++17 引入的特性，允许你用一行声明将数组、tuple-like 对象或结构体的成员直接"解包"到多个独立变量中，告别 `std::get<0>` 和 `std::tie` 的繁琐写法。

***

### 2. 基本语法与三种绑定场景

结构化绑定的声明形式为：

```cpp
auto [a, b, c] = expression;
```

C++17 标准定义了三种可绑定的场景：

| 场景 | 绑定目标 | 示例 |
|------|---------|------|
| 数组 | 原生数组元素 | `int arr[3]; auto [a,b,c] = arr;` |
| Tuple-like | `std::tuple`、`std::pair` 等 | `auto [k,v] = *map.begin();` |
| 结构体/类 | 公有非静态数据成员 | `auto [x,y] = point;` |

```cpp
#include <tuple>
#include <map>
#include <string>
#include <iostream>

struct Point {
    double x;
    double y;
};

int main() {
    int arr[3] = {10, 20, 30};
    auto [a, b, c] = arr;
    std::cout << a << " " << b << " " << c << "\n";

    auto tup = std::make_tuple(1, 3.14, "hello");
    auto [i, d, s] = tup;
    std::cout << i << " " << d << " " << s << "\n";

    Point p{1.0, 2.0};
    auto [x, y] = p;
    std::cout << x << " " << y << "\n";
}
```

***

### 3. 绑定数组

原生数组可以直接解包，绑定数量必须与数组大小完全一致：

```cpp
#include <iostream>

int main() {
    int values[4] = {1, 2, 3, 4};
    auto [a, b, c, d] = values;
    std::cout << a << b << c << d << "\n";

    const char* names[2] = {"Alice", "Bob"};
    auto [first, second] = names;
    std::cout << first << " " << second << "\n";
}
```

需要注意的关键点：

| 要点 | 说明 |
|------|------|
| 数量必须匹配 | `auto [a,b] = arr;` 对 `int[3]` 会编译错误 |
| 绑定的是副本 | `auto [a,b,c] = arr;` 中修改 `a` 不影响 `arr[0]` |
| 引用可修改原值 | `auto& [a,b,c] = arr;` 中修改 `a` 会影响 `arr[0]` |
| `std::array` 属于 tuple-like | 不走数组绑定，走 tuple-like 路径 |

```cpp
#include <iostream>
#include <array>

int main() {
    int arr[3] = {10, 20, 30};
    auto& [a, b, c] = arr;
    a = 100;
    std::cout << arr[0] << "\n";

    std::array<int, 3> sa = {1, 2, 3};
    auto [x, y, z] = sa;
    std::cout << x << y << z << "\n";
}
```

***

### 4. 绑定 Tuple-like 对象

这是最常用的场景，`std::tuple`、`std::pair` 以及任何满足 tuple-like 协议的类型都支持：

```cpp
#include <tuple>
#include <map>
#include <string>
#include <iostream>

int main() {
    std::pair<int, std::string> p{1, "hello"};
    auto [id, name] = p;
    std::cout << id << ": " << name << "\n";

    std::map<std::string, int> scores{
        {"Alice", 95},
        {"Bob", 87},
        {"Charlie", 92}
    };

    for (const auto& [name, score] : scores) {
        std::cout << name << " -> " << score << "\n";
    }
}
```

自定义 tuple-like 类型需要特化 `std::tuple_size`、`std::tuple_element` 并提供 `get` 函数：

```cpp
#include <tuple>
#include <string>
#include <iostream>

class User {
    int id_;
    std::string name_;
    double score_;
public:
    User(int id, std::string name, double score)
        : id_(id), name_(std::move(name)), score_(score) {}

    int id() const { return id_; }
    const std::string& name() const { return name_; }
    double score() const { return score_; }
};

namespace std {
template<>
struct tuple_size<User> : integral_constant<size_t, 3> {};

template<>
struct tuple_element<0, User> { using type = int; };

template<>
struct tuple_element<1, User> { using type = std::string; };

template<>
struct tuple_element<2, User> { using type = double; };
}

template<size_t I>
auto get(const User& u) {
    if constexpr (I == 0) return u.id();
    else if constexpr (I == 1) return u.name();
    else return u.score();
}

int main() {
    User u{1, "Alice", 95.5};
    auto [id, name, score] = u;
    std::cout << id << " " << name << " " << score << "\n";
}
```

***

### 5. 绑定结构体与类

绑定结构体时，所有非静态数据成员必须都是公有的，且不能有位域成员：

```cpp
#include <iostream>
#include <string>

struct Employee {
    int id;
    std::string name;
    double salary;
};

int main() {
    Employee emp{1, "Alice", 50000.0};
    auto [id, name, salary] = emp;
    std::cout << id << ": " << name << " earns " << salary << "\n";

    auto& [rid, rname, rsalary] = emp;
    rsalary = 55000.0;
    std::cout << emp.salary << "\n";
}
```

结构体绑定的限制：

| 限制 | 说明 |
|------|------|
| 必须全部公有 | 有 `private`/`protected` 成员则编译失败 |
| 无位域 | 包含位域成员的结构体不可绑定 |
| 无基类数据成员（C++17） | C++17 不支持绑定带基类的结构体 |
| 聚合类型 | C++17 要求是聚合类型 |
| 继承结构体（C++20） | C++20 放宽了限制，允许绑定有基类的聚合 |

```cpp
#include <iostream>

struct Base {
    int x;
};

struct Derived : Base {
    int y;
};

int main() {
#if __cplusplus >= 202002L
    Derived d{{10}, 20};
    auto [x, y] = d;
    std::cout << x << " " << y << "\n";
#else
    std::cout << "C++17 不支持绑定有基类的结构体\n";
#endif
}
```

***

### 6. auto、auto& 与 const auto& 的选择

引用限定决定了绑定变量与原对象的关系，这是结构化绑定中最容易出错的点：

```cpp
#include <iostream>
#include <tuple>
#include <string>

struct Config {
    int width;
    int height;
};

int main() {
    Config cfg{1920, 1080};

    auto [w1, h1] = cfg;
    w1 = 800;
    std::cout << cfg.width << "\n";

    auto& [w2, h2] = cfg;
    w2 = 800;
    std::cout << cfg.width << "\n";

    const auto& [w3, h3] = cfg;
}
```

三种引用语义对比：

| 声明方式 | 绑定变量类型 | 修改绑定变量 | 修改原对象 | 适用场景 |
|---------|------------|------------|-----------|---------|
| `auto [a,b] = x` | 副本 | ✅ | ❌ | 只读、需要独立副本 |
| `auto& [a,b] = x` | 引用 | ✅ | ✅ | 需要修改原对象 |
| `const auto& [a,b] = x` | const引用 | ❌ | ❌ | 只读遍历，避免拷贝 |
| `auto&& [a,b] = x` | 转发引用 | 视情况 | 视情况 | 泛型编程/完美转发 |

map 遍历中的典型用法：

```cpp
#include <map>
#include <string>
#include <iostream>

int main() {
    std::map<std::string, int> m{{"x", 1}, {"y", 2}};

    for (auto& [key, value] : m) {
        value += 10;
    }

    for (const auto& [key, value] : m) {
        std::cout << key << " = " << value << "\n";
    }
}
```

***

### 7. 底层机制：它到底做了什么？

结构化绑定并非创建新变量，而是为原对象的成员引入别名。编译器大致执行以下步骤：

1. 用 `auto` 声明一个匿名变量（称为 `e`），用右侧表达式初始化
2. 将每个绑定名声明为指向 `e` 对应成员的引用（或类引用实体）

```cpp
struct Point {
    int x, y;
};

Point p{1, 2};
auto [a, b] = p;
```

等价于：

```cpp
Point p{1, 2};
auto e = p;
auto& a = e.x;
auto& b = e.y;
```

对于 tuple-like 类型：

```cpp
auto t = std::make_tuple(1, 2.0);
auto [a, b] = t;
```

等价于：

```cpp
auto e = t;
auto& a = std::get<0>(e);
auto& b = std::get<1>(e);
```

| 绑定场景 | 引用指向 | 实际类型 |
|---------|---------|---------|
| 数组 | `e[i]` | 元素类型的引用 |
| Tuple-like | `get<I>(e)` | `tuple_element` 指定类型的引用 |
| 结构体 | `e.member` | 成员类型的引用 |

关键理解：绑定名本身不是变量，而是"类引用"的别名。这意味着 `decltype` 对绑定名返回的是成员类型而非引用类型：

```cpp
#include <tuple>
#include <iostream>
#include <type_traits>

struct S { int x; };

int main() {
    S s{42};
    auto [a] = s;
    std::cout << std::is_same_v<decltype(a), int> << "\n";
    std::cout << std::is_same_v<decltype(a), int&> << "\n";
}
```

***

### 8. 与 std::tie 的对比

C++17 之前，解包 tuple 通常用 `std::tie`：

```cpp
#include <tuple>
#include <iostream>

std::tuple<int, double, std::string> get_data() {
    return {1, 3.14, "hello"};
}

int main() {
    int i;
    double d;
    std::string s;
    std::tie(i, d, s) = get_data();
    std::cout << i << " " << d << " " << s << "\n";

    auto [ai, ad, as] = get_data();
    std::cout << ai << " " << ad << " " << as << "\n";
}
```

| 对比维度 | `std::tie` | 结构化绑定 |
|---------|-----------|-----------|
| 标准 | C++11 | C++17 |
| 声明方式 | 先声明变量再绑定 | 声明即绑定 |
| 忽略值 | `std::ignore` | 无直接等价（用 `[[maybe_unused]]`） |
| 代码行数 | 多（需先声明） | 少（一行搞定） |
| 可读性 | 一般 | 优秀 |
| 绑定结构体 | ❌ | ✅ |
| 绑定数组 | ❌ | ✅ |
| 重新绑定 | ✅（可多次赋值） | ❌（只初始化一次） |
| 忽略部分值 | `std::ignore` | 需用 `[[maybe_unused]]` |

```cpp
#include <tuple>
#include <iostream>

int main() {
    auto t = std::make_tuple(1, 2, 3);

    int a, c;
    std::tie(a, std::ignore, c) = t;
    std::cout << a << " " << c << "\n";

    [[maybe_unused]] auto [x, _, z] = t;
    std::cout << x << " " << z << "\n";
}
```

***

### 9. 限制与常见陷阱

结构化绑定虽然强大，但有一些重要限制：

**1. 不能在条件语句中使用**

```cpp
#include <map>
#include <string>

int main() {
    std::map<std::string, int> m;
    if (auto [it, ok] = m.insert({"key", 1}); ok) {
    }
}
```

**2. 绑定名不能重新绑定**

```cpp
#include <tuple>

int main() {
    auto [a, b] = std::make_pair(1, 2);
}
```

**3. 不能用于位域**

```cpp
struct Flags {
    unsigned int a : 3;
    unsigned int b : 5;
};

int main() {
    Flags f{1, 2};
}
```

**4. 不支持嵌套绑定（C++17/20）**

```cpp
#include <tuple>

int main() {
    auto [a, [b, c]] = std::make_pair(1, std::make_pair(2, 3));
}
```

**5. lambda 捕获的注意事项**

```cpp
#include <tuple>
#include <iostream>

int main() {
    auto [a, b] = std::make_pair(1, 2);
    auto f = [&a, &b]() {
        std::cout << a << " " << b << "\n";
    };
    f();
}
```

> 注意：某些编译器版本中，在 lambda 中捕获结构化绑定名可能产生警告或错误，建议捕获整个匿名对象。

```cpp
#include <tuple>
#include <iostream>

int main() {
    auto t = std::make_pair(1, 2);
    auto [a, b] = t;
    auto f = [&t]() {
        std::cout << std::get<0>(t) << " " << std::get<1>(t) << "\n";
    };
    f();
}
```

***

### 10. 实战：map 遍历与多返回值

结构化绑定最常见的两个实战场景：

**场景一：优雅遍历 map**

```cpp
#include <map>
#include <string>
#include <iostream>

int main() {
    std::map<std::string, std::vector<int>> data{
        {"Alice", {90, 85, 88}},
        {"Bob", {78, 92, 80}},
        {"Charlie", {95, 89, 93}}
    };

    for (const auto& [name, scores] : data) {
        double avg = 0;
        for (int s : scores) avg += s;
        avg /= scores.size();
        std::cout << name << ": avg=" << avg << "\n";
    }

    for (auto& [name, scores] : data) {
        scores.push_back(100);
    }
}
```

**场景二：函数多返回值**

```cpp
#include <tuple>
#include <string>
#include <iostream>

enum class ParseStatus { OK, Error, Empty };

std::tuple<ParseStatus, int, std::string> parse_input(const std::string& input) {
    if (input.empty()) return {ParseStatus::Empty, 0, ""};
    try {
        int val = std::stoi(input);
        return {ParseStatus::OK, val, "success"};
    } catch (...) {
        return {ParseStatus::Error, 0, "invalid number"};
    }
}

int main() {
    auto [status, value, msg] = parse_input("42");
    std::cout << static_cast<int>(status) << " " << value << " " << msg << "\n";

    auto [s2, v2, m2] = parse_input("abc");
    std::cout << static_cast<int>(s2) << " " << v2 << " " << m2 << "\n";
}
```

**场景三：insert/emplace 返回值处理**

```cpp
#include <map>
#include <string>
#include <iostream>

int main() {
    std::map<std::string, int> cache;

    auto [it1, ok1] = cache.insert({"key1", 100});
    std::cout << "insert key1: " << (ok1 ? "new" : "exists") << "\n";

    auto [it2, ok2] = cache.insert({"key1", 200});
    std::cout << "insert key1 again: " << (ok2 ? "new" : "exists") << "\n";

    auto [it3, ok3] = cache.emplace("key2", 300);
    std::cout << "emplace key2: " << (ok3 ? "new" : "exists") << "\n";
}
```

***

### 11. 极简总结

| 特性 | 说明 |
|------|------|
| 标准 | C++17 |
| 语法 | `auto [a, b, c] = expr;` |
| 三种场景 | 数组、tuple-like、结构体/类 |
| 引用修饰 | `auto`（副本）、`auto&`（引用）、`const auto&`（只读引用） |
| 底层机制 | 引入匿名变量 + 绑定名为类引用别名 |
| 替代 tie | 更简洁、更安全、支持更多类型 |
| 限制 | 无嵌套绑定、无位域绑定、不能重新赋值 |

核心要点：

- 优先使用 `const auto&` 遍历 map，需要修改时用 `auto&`
- 结构体绑定要求所有成员公有
- 绑定名是别名而非独立变量，`decltype` 返回成员类型
- 在 lambda 中捕获绑定名需注意编译器兼容性
- C++20 对有基类的聚合类型放宽了绑定限制

***

### 相关阅读

- [什么是std-tuple与tie](./17-什么是std-tuple与tie.md)
- [STL容器底层实现](./00-STL容器底层实现.md)
- [什么是C++23新特性](./15-什么是C++23新特性.md)

***