# 什么是C++20 Concepts
> 📖 相关章节：[模板基础](../../02-CPP/10-模板基础.md)、[模板进阶](../../02-CPP/11-模板进阶.md)、[Concepts](../../02-CPP/23-Concepts.md)

> "Concepts 让编译器的错误信息从天书变成了人话。"

***

### 1. 先抓核心

C++20 Concepts 是对模板参数的命名约束，它用清晰的语法声明模板参数必须满足的语义要求，替代了 SFINAE 的晦涩技巧，并大幅改善错误信息。

***

### 2. Concepts 的基本语法

Concepts 有四种定义和使用方式：

```cpp
#include <iostream>
#include <concepts>
#include <string>
#include <vector>
#include <type_traits>

template <typename T>
concept Integral = std::is_integral_v<T>;

template <typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template <typename T>
concept Container = requires(T c) {
    typename T::value_type;
    { c.begin() } -> std::input_iterator;
    { c.end() } -> std::input_iterator;
    { c.size() } -> std::convertible_to<std::size_t>;
};

template <typename T>
concept Sortable = Container<T> && requires(T& c) {
    { std::sort(c.begin(), c.end()) };
};

template <Integral T>
T add(T a, T b) {
    return a + b;
}

template <Addable T>
T combine(T a, T b) {
    return a + b;
}

template <Container C>
void printSize(const C& c) {
    std::cout << "容器大小: " << c.size() << std::endl;
}

int main() {
    std::cout << add(3, 5) << std::endl;
    std::cout << combine(std::string("hello"), std::string(" world")) << std::endl;

    std::vector<int> v = {1, 2, 3};
    printSize(v);
}
```

四种使用方式对比：

| 语法形式 | 示例 | 特点 |
|---------|------|------|
| `requires` 子句 | `template<typename T> requires Integral<T> void f(T)` | 最灵活 |
| 尾置 `requires` | `void f(T) requires Integral<T>` | 简洁 |
| Concept 名替代 `typename` | `template<Integral T> void f(T)` | 最简洁 |
| 简写函数模板 | `void f(Integral auto x)` | 最简短 |

### 3. requires 表达式详解

`requires` 表达式是 Concepts 的核心，用于检查表达式是否合法：

```cpp
#include <iostream>
#include <concepts>
#include <string>
#include <vector>

template <typename T>
concept HasSize = requires(T t) {
    { t.size() } -> std::convertible_to<std::size_t>;
};

template <typename T>
concept Printable = requires(T t, std::ostream& os) {
    { os << t } -> std::same_as<std::ostream&>;
};

template <typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template <typename T>
concept Hashable = requires(T t) {
    { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
};

template <typename T>
concept Iterable = requires(T t) {
    typename T::iterator;
    { t.begin() } -> std::same_as<typename T::iterator>;
    { t.end() } -> std::same_as<typename T::iterator>;
    requires std::input_iterator<typename T::iterator>;
};

template <Hashable T>
std::size_t getHash(const T& val) {
    return std::hash<T>{}(val);
}

template <Numeric T>
T square(T val) {
    return val * val;
}

int main() {
    std::cout << "hash(42): " << getHash(42) << std::endl;
    std::cout << "hash(3.14): " << getHash(3.14) << std::endl;
    std::cout << "square(5): " << square(5) << std::endl;
    std::cout << "square(2.5): " << square(2.5) << std::endl;

    static_assert(HasSize<std::string>);
    static_assert(HasSize<std::vector<int>>);
    static_assert(Printable<int>);
    static_assert(Printable<std::string>);
}
```

`requires` 表达式的四种检查：

| 检查类型 | 语法 | 含义 |
|---------|------|------|
| 简单要求 | `{ expr }` | 表达式合法即可 |
| 类型要求 | `{ typename T::type }` | 嵌套类型存在 |
| 复合要求 | `{ expr } -> Concept` | 表达式返回类型满足 Concept |
| 嵌套要求 | `{ requires Concept<T> }` | 类型满足另一个 Concept |

### 4. 标准库 Concepts

C++20 `<concepts>` 头文件提供了丰富的预定义 Concepts：

```cpp
#include <iostream>
#include <concepts>
#include <type_traits>

void demoCoreConcepts() {
    static_assert(std::same_as<int, int>);
    static_assert(std::derived_from<std::runtime_error, std::exception>);
    static_assert(std::convertible_to<int, double>);
    static_assert(std::common_with<int, double>);
    static_assert(std::integral<int>);
    static_assert(std::floating_point<double>);
    static_assert(std::signed_integral<int>);
    static_assert(std::unsigned_integral<unsigned>);

    std::cout << "核心语言 Concepts 验证通过" << std::endl;
}

void demoComparisonConcepts() {
    static_assert(std::equality_comparable<int>);
    static_assert(std::totally_ordered<double>);
    static_assert(std::three_way_comparable<int>);

    std::cout << "比较 Concepts 验证通过" << std::endl;
}

void demoObjectConcepts() {
    static_assert(std::movable<int>);
    static_assert(std::copyable<int>);
    static_assert(std::semiregular<int>);
    static_assert(std::regular<int>);
    static_assert(std::invocable<void(*)()>);

    std::cout << "对象 Concepts 验证通过" << std::endl;
}

void demoCallableConcepts() {
    auto lambda = [](int x) { return x * 2; };
    static_assert(std::invocable<decltype(lambda), int>);
    static_assert(std::predicate<decltype(lambda), int>);

    std::cout << "可调用 Concepts 验证通过" << std::endl;
}

int main() {
    demoCoreConcepts();
    demoComparisonConcepts();
    demoObjectConcepts();
    demoCallableConcepts();
}
```

标准 Concepts 分类：

| 类别 | 代表 Concepts | 用途 |
|-----|-------------|------|
| 语言核心 | `same_as`, `derived_from`, `convertible_to` | 类型关系 |
| 算术 | `integral`, `floating_point`, `signed_integral` | 数值类型 |
| 比较 | `equality_comparable`, `totally_ordered` | 可比较类型 |
| 对象 | `movable`, `copyable`, `semiregular`, `regular` | 值语义 |
| 可调用 | `invocable`, `predicate`, `regular_invocable` | 函数对象 |
| 迭代器 | `input_iterator`, `forward_iterator` 等 | 迭代器 |
| 范围 | `range`, `sized_range`, `view` 等 | 范围抽象 |

### 5. Concepts vs SFINAE

Concepts 是 SFINAE 的直接替代品，对比鲜明：

```cpp
#include <iostream>
#include <type_traits>
#include <concepts>
#include <vector>
#include <list>

template <typename T>
typename std::enable_if<std::is_integral_v<T>, T>::type
sfinaeAbs(T val) {
    return val < 0 ? -val : val;
}

template <typename T>
typename std::enable_if<std::is_floating_point_v<T>, T>::type
sfinaeAbs(T val) {
    return val < 0 ? -val : val;
}

template <std::integral T>
T conceptAbs(T val) {
    return val < 0 ? -val : val;
}

template <std::floating_point T>
T conceptAbs(T val) {
    return val < 0 ? -val : val;
}

template <typename T>
requires std::is_integral_v<T>
void sfinaeProcess(T val) {
    std::cout << "SFINAE: 整数处理 " << val << std::endl;
}

template <typename T>
requires std::integral<T>
void conceptProcess(T val) {
    std::cout << "Concept: 整数处理 " << val << std::endl;
}

int main() {
    std::cout << sfinaeAbs(-5) << std::endl;
    std::cout << sfinaeAbs(-3.14) << std::endl;
    std::cout << conceptAbs(-5) << std::endl;
    std::cout << conceptAbs(-3.14) << std::endl;

    sfinaeProcess(42);
    conceptProcess(42);
}
```

SFINAE 错误信息示例（概念前）：

```
error: no matching function for call to 'sort(std::vector<MyType>::iterator, std::vector<MyType>::iterator)'
note: candidate template ignored: substitution failure [with _Iter = std::vector<MyType>::iterator]:
      no type named 'difference_type' in 'std::iterator_traits<std::vector<MyType>::iterator>'
```

Concepts 错误信息示例（概念后）：

```
error: constraint 'std::sortable<std::vector<MyType>&>' not satisfied
note: the required expression 'std::sort(c.begin(), c.end())' is invalid
```

| 特性 | SFINAE | Concepts |
|-----|--------|----------|
| 语法 | `enable_if`, `void_t` | `concept`, `requires` |
| 可读性 | ❌ 晦涩 | ✅ 清晰 |
| 错误信息 | ❌ 几十行模板展开 | ✅ 直接指出约束不满足 |
| 约束组合 | 嵌套 `enable_if` | `&&`, `||` |
| 语义表达 | 隐式（通过替换失败） | ✅ 显式（命名约束） |
| 诊断能力 | 差 | ✅ 精确定位 |
| C++ 版本 | C++11 | C++20 |

### 6. Concepts vs 标签分发 vs if constexpr

三种编译期分支选择技术的全面对比：

```cpp
#include <iostream>
#include <concepts>
#include <iterator>
#include <vector>
#include <list>

template <typename Iter, typename Distance>
void advanceTagDispatch(Iter& it, Distance n, std::input_iterator_tag) {
    for (Distance i = 0; i < n; ++i) ++it;
}

template <typename Iter, typename Distance>
void advanceTagDispatch(Iter& it, Distance n, std::random_access_iterator_tag) {
    it += n;
}

template <typename Iter, typename Distance>
void advanceTag(Iter& it, Distance n) {
    advanceTagDispatch(it, n, typename std::iterator_traits<Iter>::iterator_category{});
}

template <typename Iter, typename Distance>
void advanceIfConstexpr(Iter& it, Distance n) {
    if constexpr (std::random_access_iterator<Iter>) {
        it += n;
    } else {
        for (Distance i = 0; i < n; ++i) ++it;
    }
}

template <std::random_access_iterator Iter, typename Distance>
void advanceConcept(Iter& it, Distance n) {
    it += n;
}

template <std::input_iterator Iter, typename Distance>
requires (!std::random_access_iterator<Iter>)
void advanceConcept(Iter& it, Distance n) {
    for (Distance i = 0; i < n; ++i) ++it;
}

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto vit = v.begin();
    advanceTag(vit, 3);
    advanceIfConstexpr(vit, 1);
    advanceConcept(vit, 1);
    std::cout << "vector: " << *vit << std::endl;

    std::list<int> lst = {10, 20, 30};
    auto lit = lst.begin();
    advanceTag(lit, 2);
    advanceIfConstexpr(lit, 1);
    advanceConcept(lit, 1);
    std::cout << "list: " << *lit << std::endl;
}
```

| 特性 | 标签分发 | if constexpr | Concepts |
|-----|---------|-------------|----------|
| C++ 版本 | C++98 | C++17 | C++20 |
| 代码位置 | 多个重载 | 单函数体内 | 多个重载 |
| 可读性 | 中 | 高 | ✅ 最高 |
| 错误信息 | 中 | 好 | ✅ 最佳 |
| 扩展性 | 开放封闭原则 | 修改已有代码 | 开放封闭原则 |
| 约束表达 | 类型标签 | 布尔条件 | ✅ 语义化约束 |
| 组合能力 | 继承链 | `&&`, `||` | `&&`, `||`, 包含 |
| 推荐度 | 向后兼容 | 简单分支 | ✅ C++20 首选 |

### 7. 受约束的 auto

Concepts 可以约束 `auto`，让自动推导更有安全保障：

```cpp
#include <iostream>
#include <concepts>
#include <vector>

std::integral auto getIntegral() {
    return 42;
}

std::floating_point auto getFloat() {
    return 3.14;
}

template <typename T>
std::same_as<T> auto identity(T val) {
    return val;
}

template <typename C>
requires std::ranges::range<C>
std::ranges::range auto getRange(C& c) {
    return c;
}

void demoConstrainedAuto() {
    std::integral auto x = 10;
    std::floating_point auto y = 2.5;
    std::same_as<int> auto z = 42;

    auto val1 = getIntegral();
    auto val2 = getFloat();

    std::cout << "x=" << x << " y=" << y << " z=" << z << std::endl;
    std::cout << "val1=" << val1 << " val2=" << val2 << std::endl;

    std::vector<int> v = {1, 2, 3};
    for (std::integral auto elem : v) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;
}

int main() {
    demoConstrainedAuto();
}
```

| auto 形式 | 约束 | 示例 |
|-----------|------|------|
| `auto` | 无约束 | `auto x = 42;` |
| `std::integral auto` | 必须是整数 | `std::integral auto x = 42;` |
| `std::floating_point auto` | 必须是浮点 | `std::floating_point auto y = 3.14;` |
| `std::same_as<T> auto` | 必须是 T 类型 | `std::same_as<int> auto z = 42;` |
| `Concept auto` | 自定义约束 | `MyConcept auto v = ...;` |

### 8. Concept 包含（Subsumption）

Concepts 支持"包含"关系：更严格的 Concept 优先匹配，实现自动重载排序：

```cpp
#include <iostream>
#include <concepts>
#include <iterator>
#include <vector>
#include <list>

template <typename T>
concept BasicIterator = requires(T it) {
    { *it };
    { ++it };
};

template <typename T>
concept ForwardIter = BasicIterator<T> && requires(T it) {
    { it++ };
    typename T::value_type;
};

template <typename T>
concept RandomIter = ForwardIter<T> && requires(T it, T it2, int n) {
    { it += n };
    { it - it2 } -> std::convertible_to<int>;
};

template <BasicIterator Iter>
int classify(Iter) {
    return 1;
}

template <ForwardIter Iter>
int classify(Iter) {
    return 2;
}

template <RandomIter Iter>
int classify(Iter) {
    return 3;
}

int main() {
    std::vector<int> v = {1, 2, 3};
    std::list<int> lst = {1, 2, 3};

    std::cout << "vector 迭代器级别: " << classify(v.begin()) << std::endl;
    std::cout << "list 迭代器级别: " << classify(lst.begin()) << std::endl;
}
```

输出：

```
vector 迭代器级别: 3
list 迭代器级别: 2
```

包含关系图：

```
BasicIterator
    └── ForwardIter (包含 BasicIterator)
            └── RandomIter (包含 ForwardIter)
```

重载选择规则：

| 传入类型 | 满足的 Concepts | 选择的重载 | 原因 |
|---------|----------------|-----------|------|
| `vector::iterator` | Basic, Forward, Random | Random (3) | 最严格 |
| `list::iterator` | Basic, Forward | Forward (2) | 最严格 |
| `istream_iterator` | Basic | Basic (1) | 唯一匹配 |

**关键**：当 Concept A 包含 Concept B（即 A = B && 额外约束），A 的重载比 B 更优先。编译器自动选择最严格的匹配。

### 9. SFINAE 迁移到 Concepts 指南

逐步将现有 SFINAE 代码迁移到 Concepts：

```cpp
#include <iostream>
#include <type_traits>
#include <concepts>
#include <vector>
#include <string>

template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
T sfinaeSquare(T x) {
    return x * x;
}

template <std::integral T>
T conceptSquare(T x) {
    return x * x;
}

template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
void sfinaeProcess(T val) {
    std::cout << "SFINAE 整数: " << val << std::endl;
}

template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
void sfinaeProcess(T val) {
    std::cout << "SFINAE 浮点: " << val << std::endl;
}

template <std::integral T>
void conceptProcess(T val) {
    std::cout << "Concept 整数: " << val << std::endl;
}

template <std::floating_point T>
void conceptProcess(T val) {
    std::cout << "Concept 浮点: " << val << std::endl;
}

template <typename T, typename = void>
struct HasSizeSfinae : std::false_type {};

template <typename T>
struct HasSizeSfinae<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};

template <typename T>
concept HasSizeConcept = requires(T t) {
    { t.size() } -> std::convertible_to<std::size_t>;
};

template <typename T>
constexpr bool hasSizeSfinae = HasSizeSfinae<T>::value;

int main() {
    std::cout << "SFINAE square: " << sfinaeSquare(5) << std::endl;
    std::cout << "Concept square: " << conceptSquare(5) << std::endl;

    sfinaeProcess(42);
    sfinaeProcess(3.14);
    conceptProcess(42);
    conceptProcess(3.14);

    static_assert(hasSizeSfinae<std::vector<int>>);
    static_assert(!hasSizeSfinae<int>);
    static_assert(HasSizeConcept<std::vector<int>>);
    static_assert(!HasSizeConcept<int>);

    std::cout << "迁移验证通过" << std::endl;
}
```

迁移对照表：

| SFINAE 模式 | Concepts 等价 |
|-------------|--------------|
| `enable_if_t<cond, T>` | `requires cond` 或 Concept 名 |
| `void_t<decltype(expr)>` | `requires { expr; }` |
| `is_integral_v<T>` | `std::integral<T>` |
| `is_floating_point_v<T>` | `std::floating_point<T>` |
| `is_same_v<T, U>` | `std::same_as<T, U>` |
| `is_base_of_v<Base, Derived>` | `std::derived_from<Derived, Base>` |
| `is_convertible_v<From, To>` | `std::convertible_to<From, To>` |
| `is_invocable_v<F, Args...>` | `std::invocable<F, Args...>` |
| `enable_if_t<cond>* = nullptr` | `requires cond` |
| `std::void_t<...>` | `requires { ... }` |

迁移步骤：

1. **识别 SFINAE 模式**：找到 `enable_if`、`void_t`、`decltype` 等用法
2. **提取语义**：将类型约束抽象为命名 Concept
3. **替换声明**：用 `requires` 子句或 Concept 名替代 `enable_if`
4. **验证编译**：确保约束语义不变
5. **改善错误信息**：Concept 自动提供更好的诊断

### 10. 极简总结

| 要点 | 说明 |
|-----|------|
| **定义** | 对模板参数的命名约束，声明语义要求 |
| **核心语法** | `concept`, `requires`, 受约束 `auto` |
| **requires 四种** | 简单要求、类型要求、复合要求、嵌套要求 |
| **标准 Concepts** | `<concepts>` 提供核心/比较/对象/可调用/迭代器概念 |
| **vs SFINAE** | 更可读、错误信息更好、约束可组合 |
| **vs 标签分发** | 语义更清晰、不需要空标签类型 |
| **vs if constexpr** | 遵循开放封闭原则、支持重载 |
| **Subsumption** | 更严格的 Concept 自动优先匹配 |
| **受约束 auto** | `std::integral auto x = ...;` |
| **C++ 版本** | C++20 |

**口诀**：Concepts 命约束，requires 做检查；SFINAE 退场，错误信息说人话。

***

### 相关阅读

- [SFINAE与TypeTraits](./00-SFINAE与TypeTraits.md)
- [CRTP模式与静态多态](../04-CPP核心特性/17-CRTP模式与静态多态.md)
- [什么是标签分发Tag-Dispatch](./05-什么是标签分发Tag-Dispatch.md)

***