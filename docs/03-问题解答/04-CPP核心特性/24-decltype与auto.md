# 什么是 decltype 与 auto 的区别
> 📖 相关章节：[类与对象](../../02-CPP/03-类与对象.md)、[继承与多态](../../02-CPP/04-继承与多态.md)、[核心机制](../../02-CPP/05-核心机制.md)、[异常处理](../../02-CPP/07-异常处理.md)、[移动语义](../../02-CPP/09-移动语义与完美转发.md)

> "auto 让编译器替你写类型，decltype 让编译器告诉你类型是什么。" —— C++11

***

### 1. 先抓核心

`auto` 根据初始化表达式推导类型并执行退化（decay，去掉引用和顶层 const），`decltype` 检查表达式的精确类型而不做退化；`decltype(auto)` 结合两者优势，在泛型编程中实现完美类型转发。

***

### 2. auto 的推导规则

`auto` 的推导规则基于模板参数推导（与函数模板参数推导规则一致）：

```cpp
#include <iostream>
#include <type_traits>

int main() {
    int x = 42;
    const int cx = x;
    int& rx = x;
    const int& crx = x;
    int* px = &x;

    auto a = x;
    auto b = cx;
    auto c = rx;
    auto d = crx;
    auto e = px;

    std::cout << std::is_same_v<decltype(a), int> << "\n";
    std::cout << std::is_same_v<decltype(b), int> << "\n";
    std::cout << std::is_same_v<decltype(c), int> << "\n";
    std::cout << std::is_same_v<decltype(d), int> << "\n";
    std::cout << std::is_same_v<decltype(e), int*> << "\n";
}
```

auto 退化规则（Decay）：

| 源类型 | auto 推导结果 | 说明 |
|-------|-------------|------|
| `int` | `int` | 无变化 |
| `const int` | `int` | 丢弃顶层 const |
| `int&` | `int` | 丢弃引用 |
| `const int&` | `int` | 丢弃引用和顶层 const |
| `int*` | `int*` | 指针保留 |
| `const int*` | `const int*` | 底层 const 保留 |
| `int[5]` | `int*` | 数组退化为指针 |
| `int(int)` | `int(*)(int)` | 函数退化为函数指针 |

用 `auto&` 保留引用和 const：

```cpp
#include <iostream>
#include <type_traits>

int main() {
    int x = 42;
    const int cx = x;

    auto& a = x;
    auto& b = cx;
    const auto& c = x;

    std::cout << std::is_same_v<decltype(a), int&> << "\n";
    std::cout << std::is_same_v<decltype(b), const int&> << "\n";
    std::cout << std::is_same_v<decltype(c), const int&> << "\n";
}
```

| 声明 | 源类型 | 推导结果 | 说明 |
|------|-------|---------|------|
| `auto& a = x` | `int` | `int&` | 保留引用 |
| `auto& b = cx` | `const int` | `const int&` | 保留引用和 const |
| `const auto& c = x` | `int` | `const int&` | 添加 const |

***

### 3. decltype 的推导规则

`decltype` 有两套推导规则，取决于其参数是标识符还是表达式：

**规则一：标识符（无括号）**——返回声明类型

```cpp
#include <iostream>
#include <type_traits>

int main() {
    int x = 42;
    const int cx = x;
    int& rx = x;
    const int& crx = x;
    int arr[5] = {};

    std::cout << std::is_same_v<decltype(x), int> << "\n";
    std::cout << std::is_same_v<decltype(cx), const int> << "\n";
    std::cout << std::is_same_v<decltype(rx), int&> << "\n";
    std::cout << std::is_same_v<decltype(crx), const int&> << "\n";
    std::cout << std::is_same_v<decltype(arr), int[5]> << "\n";
}
```

**规则二：表达式（有括号或非标识符）**——返回值类别决定的类型

```cpp
#include <iostream>
#include <type_traits>

int g_val = 100;

int main() {
    int x = 42;

    std::cout << std::is_same_v<decltype((x)), int&> << "\n";
    std::cout << std::is_same_v<decltype(x + 1), int> << "\n";
    std::cout << std::is_same_v<decltype(g_val), int> << "\n";
    std::cout << std::is_same_v<decltype((g_val)), int&> << "\n";
}
```

decltype 表达式规则：

| 表达式 | 值类别 | decltype 结果 |
|-------|--------|-------------|
| 标识符 `x`（`int`） | — | `int`（声明类型） |
| `(x)` | 左值 | `int&` |
| `x + 1` | 纯右值 | `int` |
| `x = 10` | 左值 | `int&` |
| `*px` | 左值 | `int&` |
| `std::move(x)` | 右值 | `int&&` |
| 字面量 `42` | 纯右值 | `int` |

```cpp
#include <iostream>
#include <type_traits>
#include <utility>

int main() {
    int x = 42;
    int* px = &x;

    std::cout << std::is_same_v<decltype(*px), int&> << "\n";
    std::cout << std::is_same_v<decltype(std::move(x)), int&&> << "\n";
    std::cout << std::is_same_v<decltype(x = 10), int&> << "\n";
}
```

***

### 4. auto 与 decltype 的核心区别

```cpp
#include <iostream>
#include <type_traits>

int main() {
    int x = 42;
    const int cx = x;
    int& rx = x;

    auto a = rx;
    decltype(rx) d = rx;

    std::cout << std::is_same_v<decltype(a), int> << "\n";
    std::cout << std::is_same_v<decltype(d), int&> << "\n";

    auto b = cx;
    decltype(cx) dc = cx;

    std::cout << std::is_same_v<decltype(b), int> << "\n";
    std::cout << std::is_same_v<decltype(dc), const int> << "\n";
}
```

完整对比表：

| 对比维度 | `auto` | `decltype` |
|---------|--------|-----------|
| 标准版本 | C++11 | C++11 |
| 核心用途 | 让编译器推导变量类型 | 检查表达式的精确类型 |
| 退化（decay） | ✅ 执行退化 | ❌ 不退化 |
| 引用保留 | ❌ 丢弃（除非 `auto&`） | ✅ 保留 |
| 顶层 const | ❌ 丢弃（除非 `const auto`） | ✅ 保留 |
| 数组 | 退化为指针 | 保留数组类型 |
| 函数 | 退化为函数指针 | 保留函数类型 |
| 使用场景 | 变量声明 | 类型检查、返回类型、模板 |
| 需要初始化 | ✅ 必须 | ❌ 不需要 |
| 值类别感知 | ❌ 不感知 | ✅ 感知（左值→引用） |

```cpp
#include <iostream>
#include <type_traits>

void func(int) {}

int main() {
    int arr[5] = {};

    auto a = arr;
    decltype(arr) d = {1, 2, 3, 4, 5};

    auto f = func;
    decltype(func)* fp = func;

    std::cout << std::is_same_v<decltype(a), int*> << "\n";
    std::cout << std::is_same_v<decltype(d), int[5]> << "\n";
    std::cout << std::is_same_v<decltype(f), void(*)(int)> << "\n";
}
```

***

### 5. decltype(auto) 的妙用

C++14 引入的 `decltype(auto)` 结合了 `auto` 的便利性和 `decltype` 的精确性：

```cpp
#include <iostream>
#include <type_traits>
#include <vector>

template<typename Container>
decltype(auto) get_element(Container& c, size_t i) {
    return c[i];
}

template<typename Container>
decltype(auto) get_element_const(const Container& c, size_t i) {
    return c[i];
}

int main() {
    std::vector<int> v{10, 20, 30};

    decltype(auto) elem = get_element(v, 0);
    elem = 100;
    std::cout << v[0] << "\n";

    const std::vector<int> cv{10, 20, 30};
    decltype(auto) celem = get_element_const(cv, 0);
    std::cout << celem << "\n";
}
```

三种返回类型写法对比：

```cpp
#include <iostream>
#include <type_traits>

int g = 100;

template<typename T>
auto f_auto(T&& x) -> decltype(x) {
    return std::forward<T>(x);
}

template<typename T>
decltype(auto) f_decltype_auto(T&& x) {
    return std::forward<T>(x);
}

int main() {
    int x = 42;

    auto&& a = f_auto(x);
    decltype(auto) b = f_decltype_auto(x);

    std::cout << std::is_same_v<decltype(a), int&> << "\n";
    std::cout << std::is_same_v<decltype(b), int&> << "\n";
}
```

| 写法 | 推导行为 | 适用场景 |
|------|---------|---------|
| `auto` | 退化，返回值类型 | 大多数情况 |
| `decltype(expr)` | 精确匹配表达式类型 | 需要精确控制返回类型 |
| `decltype(auto)` | 用 decltype 规则推导 auto | 泛型转发函数 |
| `auto -> decltype(expr)` | 尾置返回类型 | C++11 需要先声明参数 |

***

### 6. 尾置返回类型 Trailing Return Type

C++11 引入尾置返回类型，使得返回类型可以依赖函数参数：

```cpp
#include <iostream>
#include <vector>
#include <type_traits>

template<typename Container>
auto at(Container& c, size_t i) -> decltype(c[i]) {
    return c[i];
}

template<typename Container>
auto at_const(const Container& c, size_t i) -> decltype(c[i]) {
    return c[i];
}

int main() {
    std::vector<int> v{1, 2, 3};
    at(v, 0) = 100;
    std::cout << v[0] << "\n";

    const std::vector<int> cv{1, 2, 3};
    std::cout << at_const(cv, 0) << "\n";
}
```

C++11 vs C++14 写法对比：

```cpp
#include <type_traits>

template<typename T, typename U>
auto add_cpp11(T t, U u) -> decltype(t + u) {
    return t + u;
}

template<typename T, typename U>
auto add_cpp14(T t, U u) {
    return t + u;
}

template<typename T, typename U>
decltype(auto) add_exact(T t, U u) {
    return t + u;
}
```

| 版本 | 写法 | 特点 |
|------|------|------|
| C++11 | `auto f() -> decltype(expr)` | 必须尾置才能引用参数 |
| C++14 | `auto f()` | 自动推导返回类型 |
| C++14 | `decltype(auto) f()` | 精确推导（保留引用） |

非模板场景也适用尾置返回类型：

```cpp
#include <iostream>
#include <vector>

auto create_vector() -> std::vector<int> {
    return {1, 2, 3};
}

auto main() -> int {
    auto v = create_vector();
    std::cout << v.size() << "\n";
    return 0;
}
```

***

### 7. 在模板中的应用

`decltype` 和 `auto` 在模板编程中是不可或缺的工具：

```cpp
#include <iostream>
#include <vector>
#include <type_traits>

template<typename Container>
decltype(auto) front(Container& c) {
    return c.front();
}

template<typename Container>
decltype(auto) front_const(const Container& c) {
    return c.front();
}

template<typename Container>
decltype(auto) safe_at(Container& c, size_t i) {
    if (i >= c.size()) throw std::out_of_range("index out of range");
    return c[i];
}

int main() {
    std::vector<int> v{10, 20, 30};

    front(v) = 100;
    std::cout << v[0] << "\n";

    safe_at(v, 1) = 200;
    std::cout << v[1] << "\n";

    const std::vector<int> cv{5, 6, 7};
    std::cout << front_const(cv) << "\n";
}
```

SFINAE 与类型检测：

```cpp
#include <iostream>
#include <type_traits>
#include <vector>

template<typename T, typename = void>
struct has_size : std::false_type {};

template<typename T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};

template<typename T, typename = void>
struct has_begin_end : std::false_type {};

template<typename T>
struct has_begin_end<T, std::void_t<
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end())>> : std::true_type {};

int main() {
    std::cout << has_size<std::vector<int>>::value << "\n";
    std::cout << has_size<int>::value << "\n";
    std::cout << has_begin_end<std::vector<int>>::value << "\n";
    std::cout << has_begin_end<int>::value << "\n";
}
```

C++17 `if constexpr` 结合 `decltype`：

```cpp
#include <iostream>
#include <type_traits>
#include <vector>

template<typename T>
auto process(T& container) {
    if constexpr (std::is_same_v<decltype(container.size()), size_t>) {
        return container.size();
    } else {
        return 0;
    }
}

int main() {
    std::vector<int> v{1, 2, 3};
    std::cout << process(v) << "\n";

    int x = 42;
    std::cout << process(x) << "\n";
}
```

***

### 8. decltype 与类成员访问

`decltype` 在类定义中访问成员时有特殊规则：

```cpp
#include <iostream>
#include <type_traits>

struct S {
    int x;
    double y;
};

int main() {
    S s{1, 2.5};
    const S cs{3, 4.5};

    decltype(S::x) a = 10;
    decltype(s.x) b = s.x;
    decltype(cs.x) c = cs.x;
    decltype((s.x)) d = s.x;

    std::cout << std::is_same_v<decltype(a), int> << "\n";
    std::cout << std::is_same_v<decltype(b), int> << "\n";
    std::cout << std::is_same_v<decltype(c), int> << "\n";
    std::cout << std::is_same_v<decltype(d), int&> << "\n";
}
```

| 表达式 | 类型 | 说明 |
|-------|------|------|
| `S::x` | `int` | 成员声明类型 |
| `s.x` | `int` | 非标识符表达式但 .x 是成员访问，返回声明类型 |
| `cs.x` | `int` | const 对象的成员，但 decltype 不加 const |
| `(s.x)` | `int&` | 加括号变为左值表达式，推导为引用 |

在成员函数中的 `decltype`：

```cpp
#include <iostream>
#include <type_traits>
#include <vector>

class Wrapper {
    std::vector<int> data_;
public:
    decltype(auto) operator[](size_t i) {
        return data_[i];
    }

    decltype(auto) operator[](size_t i) const {
        return data_[i];
    }

    void push(int val) {
        data_.push_back(val);
    }

    size_t size() const { return data_.size(); }
};

int main() {
    Wrapper w;
    w.push(10);
    w.push(20);
    w[0] = 100;

    const Wrapper& cw = w;
    std::cout << cw[0] << "\n";
    std::cout << w.size() << "\n";
}
```

***

### 9. 常见陷阱与误区

**陷阱一：auto 与花括号**

```cpp
#include <iostream>
#include <type_traits>
#include <initializer_list>

int main() {
    auto x1 = 42;
    auto x2 = {42};
    auto x3{42};

    std::cout << std::is_same_v<decltype(x1), int> << "\n";
    std::cout << std::is_same_v<decltype(x2), std::initializer_list<int>> << "\n";
    std::cout << std::is_same_v<decltype(x3), int> << "\n";
}
```

**陷阱二：decltype 加括号**

```cpp
#include <iostream>
#include <type_traits>

int main() {
    int x = 42;

    decltype(auto) a = x;
    decltype(auto) b = (x);

    std::cout << std::is_same_v<decltype(a), int> << "\n";
    std::cout << std::is_same_v<decltype(b), int&> << "\n";

    a = 100;
    b = 200;

    std::cout << "a=" << a << " x=" << x << "\n";
}
```

**陷阱三：decltype(auto) 返回局部变量引用**

```cpp
#include <iostream>

decltype(auto) dangerous() {
    int x = 42;
    return (x);
}

decltype(auto) safe() {
    int x = 42;
    return x;
}

int main() {
    auto val = safe();
    std::cout << val << "\n";
}
```

**陷阱四：vector<bool> 的代理对象**

```cpp
#include <iostream>
#include <vector>
#include <type_traits>

int main() {
    std::vector<bool> bv{true, false, true};

    auto ref = bv[0];
    decltype(auto) dref = bv[0];

    std::cout << std::is_same_v<decltype(ref), std::vector<bool>::reference> << "\n";
    std::cout << std::is_same_v<decltype(dref), std::vector<bool>::reference> << "\n";
}
```

***

### 10. 实战：泛型函数返回类型推导

综合运用 `auto`、`decltype`、`decltype(auto)` 编写泛型代码：

```cpp
#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <type_traits>

template<typename Map, typename Key>
decltype(auto) at_or_default(Map& m, const Key& k) {
    if (auto it = m.find(k); it != m.end()) {
        return it->second;
    }
    static typename Map::mapped_type default_val{};
    return default_val;
}

template<typename Container>
decltype(auto) first(Container& c) {
    return *c.begin();
}

template<typename Container>
decltype(auto) first_const(const Container& c) {
    return *c.begin();
}

template<typename T>
decltype(auto) forward_value(T&& x) {
    return std::forward<T>(x);
}

int main() {
    std::map<std::string, int> scores{
        {"Alice", 95}, {"Bob", 87}
    };

    at_or_default(scores, "Alice") = 100;
    std::cout << scores["Alice"] << "\n";

    auto& val = at_or_default(scores, "Charlie");
    std::cout << val << "\n";

    std::vector<int> v{10, 20, 30};
    first(v) = 99;
    std::cout << v[0] << "\n";
}
```

链式调用中的返回类型推导：

```cpp
#include <iostream>
#include <string>

class Builder {
    std::string result_;
public:
    Builder() = default;

    decltype(auto) add(const std::string& s) {
        result_ += s;
        return *this;
    }

    decltype(auto) add_space() {
        result_ += " ";
        return *this;
    }

    const std::string& build() const {
        return result_;
    }
};

int main() {
    auto result = Builder()
        .add("Hello")
        .add_space()
        .add("World")
        .build();

    std::cout << result << "\n";
}
```

***

### 11. 极简总结

| 特性 | `auto` | `decltype(expr)` | `decltype(auto)` |
|------|--------|-----------------|------------------|
| 退化 | ✅ 退化 | ❌ 不退化 | ❌ 不退化 |
| 引用 | 丢弃 | 保留 | 保留 |
| 顶层 const | 丢弃 | 保留 | 保留 |
| 值类别 | 不感知 | 感知 | 感知 |
| 需要初始化 | 是 | 否 | 是 |
| 标准 | C++11 | C++11 | C++14 |
| 典型用途 | 变量声明 | 类型检查 | 函数返回类型 |

核心记忆点：

- `auto` 总是退化：去掉引用、顶层 const，数组变指针
- `decltype(变量名)` 返回声明类型，`decltype((变量名))` 返回引用
- `decltype(auto)` = 用 `decltype` 规则推导 `auto`，完美保留类型信息
- 尾置返回类型 `auto f() -> decltype(expr)` 是 C++11 的写法
- C++14 的 `decltype(auto)` 可直接用于函数返回类型
- 警惕 `decltype(auto) return (local_var)` 返回局部变量引用
- 模板中优先使用 `decltype(auto)` 实现完美转发返回类型

***

### 相关阅读

- [SFINAE与TypeTraits](../05-模板与泛型/00-SFINAE与TypeTraits.md)
- [什么是完美转发Perfect-Forwarding](20-什么是完美转发Perfect-Forwarding.md)
- [std-move与std-forward](19-std-move与std-forward.md)