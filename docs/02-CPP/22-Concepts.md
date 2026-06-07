> **前置知识**：模板基础见 [模板基础](./10-模板基础.md)，模板进阶见 [模板进阶](./11-模板进阶.md)。

# Concepts深入

> 掌握C++20 Concepts，编写更安全的模板代码

***

> **Concepts make templates easier to use and harder to misuse.** — Bjarne Stroustrup
> （Concepts让模板更容易使用，更难误用。）

***

> **🎯 没有规矩，不成方圆。**
>
> （Concepts为模板参数设定约束条件，确保类型符合预期，避免编译错误难以理解。）

> 💡 **通俗理解 - 什么是Concepts？**

想象你开了一家工厂，招聘工人时需要满足一定条件：
- 没有 `Concepts` 时：谁来都能上岗，出了问题才发现他不合适——错误信息又长又看不懂
- 有了 `Concepts`：招聘时就写清楚要求（会电焊、有驾照），不满足的人直接拒绝——错误信息短而明确

**Concepts就像"岗位说明书"！**
- 告诉编译器：模板参数必须满足什么条件
- 不满足条件时：直接告诉你"缺少XX能力"，而不是报一堆内部错误
- 让代码意图更清晰：读代码的人一眼就知道类型需要什么能力

> 🔬 **抽象理解 - Concepts的本质**：
> - **Concepts**：是对模板参数的命名约束（named constraint），本质上是一个编译期谓词，返回 `true` 或 `false`
> - **`requires`**：是定义约束的关键字，既可以定义 `concept`，也可以直接约束模板
> - **约束（constraint）**：是 `concept` 的核心机制，编译器在实例化模板前先检查约束是否满足
> - **subsumption（包含）**：更严格的 `concept` 可以"包含"更宽松的 `concept`，用于重载决议和偏特化选择

***

## 前置知识
- [模板进阶](./11-模板进阶.md)（第11章）
## 后续内容
- [C++20与23新特性](./24-C++20与23新特性.md)（第24章）

***

## 目录

- [1. Concepts基础回顾](#1-concepts基础回顾)
- [2. 什么是Concepts以及为什么需要它们](#2-什么是concepts以及为什么需要它们)
- [3. 自定义Concept](#3-自定义concept)
- [4. requires子句与requires表达式](#4-requires子句与requires表达式)
- [5. 约束auto](#5-约束auto)
- [6. 约束模板](#6-约束模板)
- [7. 标准库Concept](#7-标准库concept)
- [8. 基于Concept的重载](#8-基于concept的重载)
- [9. 基于Concept的偏特化](#9-基于concept的偏特化)
- [10. 约束与继承](#10-约束与继承)
- [11. Concepts与SFINAE对比](#11-concepts与sfinae对比)
- [12. 最佳实践与常见陷阱](#12-最佳实践与常见陷阱)

***

## 1. Concepts基础回顾

### 1. 四种使用方式

```cpp
// 1. requires子句
template<typename T> requires std::integral<T>
T add(T a, T b) { return a + b; }

// 2. 尾置requires
template<typename T>
T add(T a, T b) requires std::integral<T> { return a + b; }

// 3. 约束auto
std::integral auto add(std::integral auto a, std::integral auto b) {
    return a + b;
}

// 4. 模板参数约束
template<std::integral T>
T add(T a, T b) { return a + b; }
```

> 💡 **四种方式本质相同**，只是语法位置不同。选择哪种取决于代码可读性和个人/团队偏好。

***

## 2. 什么是Concepts以及为什么需要它们

### 1. 模板的痛点

在C++20之前，模板编程面临几个核心问题：

```cpp
// C++17：没有Concepts时的模板代码
template<typename T>
T find_max(const std::vector<T>& vec) {
    // 如果T不支持<比较，这里会报错
    // 但错误信息可能出现在std::max的内部实现中
    // 几十行模板实例化错误，根本找不到原因
    return *std::max_element(vec.begin(), vec.end());
}

// 调用
struct Point { int x, y; };  // 没有operator<
std::vector<Point> points;
find_max(points);  // 💥 报错信息可能长达几十行
```

典型错误信息可能像这样：

```
error: no match for 'operator<' (operand types are 'Point' and 'Point')
  in instantiation of function template 'const T& std::max<const T&>(const T&, const T&)'
  required from 'T find_max(const std::vector<T>&) [with T = Point]'
  ...（还有更多嵌套信息）
```

### 2. Concepts如何解决这些问题

```cpp
// C++20：使用Concepts
template<typename T>
concept Comparable = requires(T a, T b) {
    { a < b } -> std::convertible_to<bool>;
};

template<Comparable T>
T find_max(const std::vector<T>& vec) {
    return *std::max_element(vec.begin(), vec.end());
}

// 调用
struct Point { int x, y; };  // 没有operator<
std::vector<Point> points;
find_max(points);  // ✅ 错误信息清晰：
// error: constraint 'Comparable<Point>' not satisfied
// note: 'Point' does not satisfy 'Comparable'
```

### 3. Concepts的三大价值

| 价值 | 说明 | 示例 |
|------|------|------|
| **更好的错误信息** | 约束不满足时，直接指出哪个 `concept` 失败 | `constraint 'Comparable<Point>' not satisfied` |
| **更清晰的接口** | 代码即文档，模板参数的要求一目了然 | `template<std::integral T>` 比 `template<typename T>` 清晰 |
| **更好的重载决议** | 基于 `concept` 的重载比 SFINAE 更直观 | 多个重载按约束严格程度自动选择 |

### 4. Concept的本质

`concept` 在编译期求值为 `bool` 常量：

```cpp
static_assert(std::integral<int>);       // true
static_assert(std::integral<double>);    // 编译错误：double不是整数类型
static_assert(!std::integral<double>);   // true

// concept可以用于constexpr if
template<typename T>
void process(T value) {
    if constexpr (std::integral<T>) {
        // 整数路径
    } else if constexpr (std::floating_point<T>) {
        // 浮点路径
    }
}
```

***

## 3. 自定义Concept

### 1. 基本定义

```cpp
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template<typename T>
concept Printable = requires(T t, std::ostream& os) {
    { os << t } -> std::same_as<std::ostream&>;
};

template<typename T>
concept Container = requires(T t) {
    typename T::value_type;
    { t.begin() } -> std::input_iterator;
    { t.end() } -> std::sentinel_for<decltype(t.begin())>;
    { t.size() } -> std::convertible_to<std::size_t>;
};
```

### 2. 组合Concept

```cpp
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept Sortable = requires(T& t) {
    { t.begin() } -> std::random_access_iterator;
    { t.end() } -> std::random_access_iterator;
    requires std::totally_ordered<typename T::value_type>;
};

// 使用
template<Sortable C>
void sort_container(C& c) {
    std::sort(c.begin(), c.end());
}
```

### 3. 多参数Concept

`concept` 可以接受多个模板参数，用于约束类型之间的关系：

```cpp
// 约束：T可以转换为U
template<typename T, typename U>
concept ConvertibleTo = std::convertible_to<T, U>;

// 约束：T是U的容器
template<typename T, typename U>
concept ContainerOf = requires(T t) {
    typename T::value_type;
    requires std::same_as<typename T::value_type, U>;
    { t.begin() } -> std::input_iterator;
    { t.end() } -> std::sentinel_for<decltype(t.begin())>;
};

// 使用
template<typename U, ContainerOf<U> C>
U sum(const C& container) {
    U result{};
    for (const auto& elem : container) {
        result += elem;
    }
    return result;
}
```

### 4. Concept的合取与析取

`concept` 可以用 `&&` 和 `||` 组合，形成更复杂的约束：

```cpp
// 合取（AND）：必须同时满足
template<typename T>
concept IntegralAndSigned = std::integral<T> && std::signed_integral<T>;

// 析取（OR）：满足其一即可
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

// 混合使用
template<typename T>
concept NumericContainer = requires(T t) {
    typename T::value_type;
} && (std::integral<typename T::value_type> || std::floating_point<typename T::value_type>);

// 实际应用
template<NumericContainer C>
auto average(const C& c) {
    using V = typename C::value_type;
    V sum{};
    for (const auto& v : c) sum += v;
    return sum / static_cast<V>(c.size());
}
```

***

## 4. requires子句与requires表达式

`requires` 关键字有两种不同的用法，容易混淆，需要仔细区分。

### 1. requires子句（requires clause）

`requires` 子句出现在模板参数列表之后，用于**约束模板**，结果是一个 `bool` 值：

```cpp
// requires子句：约束T必须是整数类型
template<typename T>
requires std::integral<T>     // ← 这是requires子句
T add(T a, T b) { return a + b; }

// requires子句可以包含任意bool常量表达式
template<typename T>
requires (sizeof(T) <= 8 && std::is_trivially_copyable_v<T>)
void fast_copy(T* dst, const T* src, size_t n) {
    std::memcpy(dst, src, n * sizeof(T));
}
```

### 2. requires表达式（requires expression）

`requires` 表达式是一个**编译期谓词**，用于**检查一组表达式是否合法**，返回 `bool`：

```cpp
// requires表达式：检查T是否支持加法
template<typename T>
concept Addable = requires(T a, T b) {   // ← 这是requires表达式
    a + b;                                 // 简单要求
};

// requires表达式可以出现在任何需要bool值的地方
template<typename T>
void process(T val) {
    static_assert(requires(T v) { v.serialize(); },
                  "T must have a serialize() method");
}
```

### 3. 两者对比

| 特性 | `requires` 子句 | `requires` 表达式 |
|------|-----------------|-------------------|
| **位置** | 模板参数列表之后 | 可以出现在任何需要 `bool` 的地方 |
| **作用** | 约束模板的实例化 | 检查表达式是否合法 |
| **返回值** | `bool`（约束是否满足） | `bool`（表达式是否合法） |
| **典型用途** | 限制模板参数 | 定义 `concept` 或 `static_assert` |

### 4. 组合使用

`requires` 子句中可以包含 `requires` 表达式：

```cpp
// requires子句中包含requires表达式
template<typename T>
requires requires(T t) { t.serialize(); }  // 外层requires是子句，内层是表达式
void save(T obj) {
    obj.serialize();
}

// 更常见的写法：先定义concept，再用requires子句
template<typename T>
concept Serializable = requires(T t) {
    { t.serialize() } -> std::convertible_to<std::string>;
};

template<typename T>
requires Serializable<T>   // requires子句引用concept
void save(T obj) {
    obj.serialize();
}
```

> ⚠️ **注意**：`requires requires` 看起来奇怪，但语法上完全合法——外层是子句，内层是表达式。不过为了可读性，建议先定义 `concept` 再引用。

***

## 5. 约束auto

C++20允许在 `auto` 前面加 `concept` 约束，称为**约束auto（constrained auto）**。

### 1. 基本用法

```cpp
// 约束auto变量
std::integral auto x = 42;       // ✅ int是整数类型
std::integral auto y = 3.14;     // ❌ double不是整数类型

std::floating_point auto f = 3.14;  // ✅
std::floating_point auto g = 42;    // ❌
```

### 2. 函数返回值约束

```cpp
// 约束auto返回值
std::integral auto get_value() {
    return 42;  // ✅ 返回int，满足integral
}

std::floating_point auto get_value2() {
    return 42;  // ❌ 返回int，不满足floating_point
}
```

### 3. 函数参数约束

```cpp
// 约束auto函数参数（缩写函数模板）
void print(std::integral auto value) {
    std::cout << value << std::endl;
}

// 等价于
template<std::integral T>
void print(T value) {
    std::cout << value << std::endl;
}
```

### 4. 范围for中的约束auto

```cpp
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

void process(const std::vector<int>& vec) {
    for (Numeric auto v : vec) {  // ✅ int满足Numeric
        std::cout << v << ' ';
    }
}
```

### 5. 约束auto与decltype

```cpp
std::integral auto x = 42;
// decltype(x) 是 int（不是"std::integral auto"）
static_assert(std::same_as<decltype(x), int>);
```

> 💡 **约束auto的本质**：`std::integral auto` 等价于声明一个模板参数 `T`，并加上 `requires std::integral<T>`。编译器推导出实际类型后，检查是否满足约束。

***

## 6. 约束模板

### 1. 约束类模板

```cpp
// 约束类模板参数
template<std::integral T>
class NumericArray {
    std::vector<T> data_;
public:
    void push_back(T val) { data_.push_back(val); }
    T sum() const { return std::accumulate(data_.begin(), data_.end(), T{}); }
};

NumericArray<int> arr1;       // ✅
NumericArray<double> arr2;    // ❌ double不满足integral
```

### 2. 约束成员函数

```cpp
template<typename T>
class Wrapper {
    T value_;
public:
    Wrapper(T v) : value_(v) {}

    // 仅当T可打印时才提供print方法
    void print() const requires std::printable<T> {
        std::cout << value_ << std::endl;
    }

    // 仅当T可比较时才提供compare方法
    int compare(const Wrapper& other) const requires std::totally_ordered<T> {
        if (value_ < other.value_) return -1;
        if (value_ > other.value_) return 1;
        return 0;
    }
};
```

### 3. 约束非模板函数（requires子句中的常量表达式）

```cpp
// 约束非模板函数（通过requires子句中的编译期条件）
void process(void* ptr)
requires (sizeof(void*) == 4)  // 仅在32位平台可用
{
    std::cout << "32-bit mode\n";
}

void process(void* ptr)
requires (sizeof(void*) == 8)  // 仅在64位平台可用
{
    std::cout << "64-bit mode\n";
}
```

### 4. 约束别名模板

```cpp
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<Numeric T>
using NumericVector = std::vector<T>;

NumericVector<int> vi;       // ✅
NumericVector<double> vd;    // ✅
NumericVector<std::string> vs;  // ❌ string不满足Numeric
```

***

## 7. 标准库Concept

### 1. 类型类别

| Concept | 说明 |
|---------|------|
| `std::integral` | 整数类型 |
| `std::signed_integral` | 有符号整数 |
| `std::unsigned_integral` | 无符号整数 |
| `std::floating_point` | 浮点类型 |
| `std::same_as<T,U>` | T和U相同类型 |
| `std::derived_from<T,U>` | T派生自U |
| `std::convertible_to<T,U>` | T可隐式转换为U |
| `std::common_reference_with<T,U>` | T和U有公共引用类型 |
| `std::common_with<T,U>` | T和U有公共类型 |

### 2. 比较概念

| Concept | 说明 |
|---------|------|
| `std::equality_comparable` | 可用==比较 |
| `std::totally_ordered` | 完全有序（<, <=, >, >=） |
| `std::three_way_comparable` | 支持三向比较（C++20） |
| `std::equality_comparable_with<T,U>` | T和U之间可用==比较 |
| `std::totally_ordered_with<T,U>` | T和U之间完全有序 |

### 3. 对象概念

| Concept | 说明 |
|---------|------|
| `std::movable` | 可移动 |
| `std::copyable` | 可拷贝 |
| `std::semiregular` | 可默认构造+可拷贝+可移动 |
| `std::regular` | semiregular + equality_comparable |

> 💡 **对象概念的层次关系**：`regular` ⊃ `semiregular` ⊃ `copyable` ⊃ `movable`。`regular` 是最严格的概念，表示类型"像int一样行为"。

### 4. 可调用概念

| Concept | 说明 |
|---------|------|
| `std::invocable<F, Args...>` | F可用Args调用 |
| `std::predicate<F, Args...>` | F返回bool |
| `std::regular_invocable` | 纯函数式调用（无副作用） |
| `std::relation<F, T, U>` | F是T和U上的二元关系 |
| `std::strict_weak_order<F, T, U>` | F是严格弱序 |

```cpp
// 使用可调用concept
template<typename F, typename... Args>
requires std::invocable<F, Args...>
auto safe_invoke(F&& f, Args&&... args) {
    return std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
}

// 使用predicate concept
template<typename T, std::predicate<T> Pred>
size_t count_if(const std::vector<T>& vec, Pred pred) {
    size_t cnt = 0;
    for (const auto& v : vec)
        if (pred(v)) ++cnt;
    return cnt;
}
```

### 5. 迭代器概念

| Concept | 说明 |
|---------|------|
| `std::input_iterator` | 只读前向迭代器 |
| `std::output_iterator` | 只写迭代器 |
| `std::forward_iterator` | 多遍前向迭代器 |
| `std::bidirectional_iterator` | 双向迭代器 |
| `std::random_access_iterator` | 随机访问迭代器 |
| `std::contiguous_iterator` | 连续内存迭代器 |
| `std::sentinel_for<S,I>` | S是I的哨兵 |
| `std::sized_sentinel_for<S,I>` | S是I的大小哨兵（支持减法） |

### 6. 范围概念

| Concept | 说明 |
|---------|------|
| `std::ranges::range` | 有begin和end |
| `std::ranges::sized_range` | 知道大小的range |
| `std::ranges::input_range` | 输入range |
| `std::ranges::forward_range` | 前向range |
| `std::ranges::bidirectional_range` | 双向range |
| `std::ranges::random_access_range` | 随机访问range |
| `std::ranges::contiguous_range` | 连续内存range |
| `std::ranges::view` | 轻量级视图 |

***

## 8. 基于Concept的重载

`concept` 可以用于函数重载，编译器根据约束的**严格程度**选择最匹配的重载——这称为 **subsumption（包含）** 关系。

### 1. 基本重载

```cpp
// 通用版本
template<typename T>
void process(T val) {
    std::cout << "Generic: " << val << std::endl;
}

// 整数版本（更严格，优先选择）
template<std::integral T>
void process(T val) {
    std::cout << "Integral: " << val << std::endl;
}

// 有符号整数版本（最严格，最优先）
template<std::signed_integral T>
void process(T val) {
    std::cout << "Signed integral: " << val << std::endl;
}

process(3.14);   // Generic: 3.14
process(42u);    // Integral: 42（unsigned不满足signed_integral）
process(-42);    // Signed integral: -42
```

### 2. Subsumption（包含）规则

当 `concept A` 蕴含 `concept B` 时，称 A **subsumes** B。编译器优先选择约束更严格（被包含）的版本：

```cpp
// 包含关系：signed_integral ⊂ integral ⊂ 任意类型
// signed_integral subsumes integral
// integral subsumes 无约束

// 自定义的包含关系
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template<typename T>
concept NumericAddable = (std::integral<T> || std::floating_point<T>) && Addable<T>;
// NumericAddable subsumes Addable

template<Addable T>
void compute(T a, T b) {
    std::cout << "Addable\n";
}

template<NumericAddable T>
void compute(T a, T b) {
    std::cout << "NumericAddable\n";
}

compute(std::string("a"), std::string("b"));  // Addable（string不是Numeric）
compute(1, 2);                                 // NumericAddable（int是Numeric且Addable）
```

### 3. 无包含关系时的歧义

如果两个 `concept` 之间没有包含关系，同时满足时会产生歧义：

```cpp
template<typename T>
concept HasA = requires(T t) { t.a(); };

template<typename T>
concept HasB = requires(T t) { t.b(); };

template<HasA T> void func(T t);   // 重载1
template<HasB T> void func(T t);   // 重载2

struct S { void a() {} void b() {} };
S s;
func(s);  // ❌ 歧义！HasA和HasB之间没有包含关系

// 解决方案：添加更严格的重载
template<HasA T> requires HasB<T> void func(T t);  // 同时满足HasA和HasB
```

### 4. 基于Concept的标签分发替代

```cpp
// C++17：标签分发
template<typename T>
void process_impl(T val, std::true_type) { /* integral */ }

template<typename T>
void process_impl(T val, std::false_type) { /* non-integral */ }

template<typename T>
void process(T val) {
    process_impl(val, std::is_integral<T>{});
}

// C++20：Concept重载（更简洁）
template<typename T>
void process(T val) { /* non-integral */ }

template<std::integral T>
void process(T val) { /* integral */ }
```

***

## 9. 基于Concept的偏特化

`concept` 可以用于类模板的偏特化，编译器根据约束严格程度选择最匹配的特化版本。

### 1. 基本偏特化

```cpp
// 主模板
template<typename T>
struct Serializer {
    static std::string serialize(const T& val) {
        return "Unknown type";
    }
};

// 整数偏特化
template<std::integral T>
struct Serializer<T> {
    static std::string serialize(T val) {
        return std::to_string(val);
    }
};

// 浮点偏特化
template<std::floating_point T>
struct Serializer<T> {
    static std::string serialize(T val) {
        std::ostringstream oss;
        oss << std::setprecision(6) << val;
        return oss.str();
    }
};

// 字符串偏特化
template<>
struct Serializer<std::string> {
    static std::string serialize(const std::string& val) {
        return "\"" + val + "\"";
    }
};

Serializer<int>::serialize(42);          // "42"
Serializer<double>::serialize(3.14);     // "3.14"
Serializer<std::string>::serialize("hi"); // "\"hi\""
Serializer<Point>::serialize(p);         // "Unknown type"
```

### 2. 多参数偏特化

```cpp
// 主模板
template<typename T, typename U>
struct PairTraits {
    static constexpr const char* name = "generic pair";
};

// 两个都是数值类型的偏特化
template<typename T, typename U>
requires (std::integral<T> || std::floating_point<T>) &&
         (std::integral<U> || std::floating_point<U>)
struct PairTraits<T, U> {
    static constexpr const char* name = "numeric pair";
};

// 两个都是整数类型的偏特化（更严格）
template<std::integral T, std::integral U>
struct PairTraits<T, U> {
    static constexpr const char* name = "integral pair";
};
```

### 3. 偏特化与subsumption

偏特化选择遵循与函数重载相同的 subsumption 规则：

```cpp
template<typename T>
struct Wrapper;                     // 声明

template<std::copyable T>           // 特化1
struct Wrapper<T> { /* ... */ };

template<std::semiregular T>        // 特化2（semiregular subsumes copyable）
struct Wrapper<T> { /* ... */ };

template<std::regular T>            // 特化3（regular subsumes semiregular）
struct Wrapper<T> { /* ... */ };

// 编译器选择最严格的匹配
Wrapper<int> w;       // 选择特化3（int是regular）
Wrapper<std::string> w2;  // 选择特化3（string是regular）
```

***

## 10. 约束与继承

### 1. 虚函数约束

```cpp
template<typename T>
concept Drawable = requires(T t) {
    { t.draw() } -> std::same_as<void>;
};

template<Drawable T>
class Shape {
public:
    virtual void draw() = 0;
    virtual ~Shape() = default;
};
```

### 2. 模板特化约束

```cpp
template<typename T>
struct Serializer;

template<std::integral T>
struct Serializer<T> {
    static std::string serialize(T val) { return std::to_string(val); }
};

template<std::floating_point T>
struct Serializer<T> {
    static std::string serialize(T val) {
        std::ostringstream oss;
        oss << std::setprecision(6) << val;
        return oss.str();
    }
};
```

### 3. CRTP与Concepts

```cpp
template<typename Derived>
concept Hashable = requires(const Derived& d) {
    { d.hash() } -> std::convertible_to<std::size_t>;
};

// CRTP基类，要求Derived满足Hashable
template<Hashable Derived>
class HashBase {
public:
    std::size_t hash_code() const {
        return static_cast<const Derived*>(this)->hash();
    }
};

class MyType : public HashBase<MyType> {
    int data_;
public:
    explicit MyType(int d) : data_(d) {}
    std::size_t hash() const { return std::hash<int>{}(data_); }
};

static_assert(Hashable<MyType>);
```

### 4. 接口约束与多态

```cpp
// 用Concept替代虚函数实现"静态多态"
template<typename T>
concept Shape = requires(T t) {
    { t.area() } -> std::convertible_to<double>;
    { t.perimeter() } -> std::convertible_to<double>;
};

struct Circle {
    double radius;
    double area() const { return 3.14159 * radius * radius; }
    double perimeter() const { return 2 * 3.14159 * radius; }
};

struct Rectangle {
    double width, height;
    double area() const { return width * height; }
    double perimeter() const { return 2 * (width + height); }
};

static_assert(Shape<Circle>);
static_assert(Shape<Rectangle>);

// 泛型函数，无需虚函数
template<Shape S>
void print_info(const S& s) {
    std::cout << "Area: " << s.area()
              << ", Perimeter: " << s.perimeter() << std::endl;
}
```

***

## 11. Concepts与SFINAE对比

### 1. SFINAE方式（C++17）

```cpp
template<typename T>
std::enable_if_t<std::is_integral_v<T>, T>
add(T a, T b) { return a + b; }
```

### 2. Concepts方式（C++20）

```cpp
template<std::integral T>
T add(T a, T b) { return a + b; }
```

### 3. 详细对比

| 特性 | SFINAE | Concepts |
|------|--------|----------|
| 可读性 | 差 | 好 |
| 错误信息 | 长且难懂 | 短且明确 |
| 组合性 | 困难 | 简单（&&, \|\|） |
| 表达力 | 有限 | 强大 |
| 推荐度 | 不推荐 | 推荐 |
| 学习曲线 | 陡峭 | 平缓 |
| 调试难度 | 高 | 低 |

### 4. 典型场景对比

#### 场景一：条件编译

```cpp
// SFINAE
template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
void process(T val) { /* integral */ }

template<typename T, std::enable_if_t<!std::is_integral_v<T>, int> = 0>
void process(T val) { /* non-integral */ }

// Concepts
template<std::integral T>
void process(T val) { /* integral */ }

template<typename T>
requires (!std::integral<T>)
void process(T val) { /* non-integral */ }
```

#### 场景二：检测成员函数

```cpp
// SFINAE：需要写一大堆辅助模板
template<typename T, typename = void>
struct has_serialize : std::false_type {};

template<typename T>
struct has_serialize<T, std::void_t<decltype(std::declval<T>().serialize())>>
    : std::true_type {};

template<typename T, std::enable_if_t<has_serialize<T>::value, int> = 0>
void save(T obj) { obj.serialize(); }

// Concepts：简洁明了
template<typename T>
concept Serializable = requires(T t) {
    { t.serialize() } -> std::convertible_to<std::string>;
};

template<Serializable T>
void save(T obj) { obj.serialize(); }
```

#### 场景三：多条件组合

```cpp
// SFINAE：组合条件非常痛苦
template<typename T, std::enable_if_t<
    std::is_integral_v<T> && std::is_signed_v<T> && (sizeof(T) >= 4),
    int> = 0>
void process(T val) { /* ... */ }

// Concepts：清晰自然
template<typename T>
requires std::signed_integral<T> && (sizeof(T) >= 4)
void process(T val) { /* ... */ }
```

### 5. 迁移建议

| SFINAE模式 | Concepts替代 |
|-----------|-------------|
| `std::enable_if_t<cond, T>` | `requires cond` 或 `concept` |
| `std::void_t<...>` | `requires` 表达式 |
| `std::is_detected<...>` | `requires` 表达式 |
| 标签分发 | `concept` 重载 |
| `decltype(...), void_t<...>` | `requires` 表达式中的复合要求 |

***

## 12. 最佳实践与常见陷阱

### 1. 最佳实践

#### 实践一：优先使用标准库Concept

```cpp
// ✅ 好的做法：使用标准concept
template<std::integral T>
T add(T a, T b) { return a + b; }

// ❌ 不好的做法：重复造轮子
template<typename T>
requires requires(T a, T b) { a + b; } && std::is_integral_v<T>
T add(T a, T b) { return a + b; }
```

#### 实践二：给Concept取有意义的名字

```cpp
// ✅ 好的做法：名字表达语义
template<typename T>
concept Sortable = requires(T& t) {
    { t.begin() } -> std::random_access_iterator;
    { t.end() } -> std::random_access_iterator;
    requires std::totally_ordered<typename T::value_type>;
};

// ❌ 不好的做法：名字过于技术化
template<typename T>
concept HasBeginEndAndTotallyOrderedValueType = requires(T& t) {
    // ...
};
```

#### 实践三：Concept应该表达语义，而非语法

```cpp
// ✅ 好的做法：表达语义——"可哈希"
template<typename T>
concept Hashable = requires(T t) {
    { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
};

// ❌ 不好的做法：只检查语法——"有hash_code方法"
template<typename T>
concept HasHashCode = requires(T t) {
    { t.hash_code() } -> std::convertible_to<std::size_t>;
};
// 问题：有hash_code()不一定意味着类型可哈希（可能返回值不合理）
```

#### 实践四：约束应尽量宽松

```cpp
// ✅ 好的做法：只要求必要的操作
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

// ❌ 不好的做法：过度约束
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::same_as<T>;   // 不需要完全相同，能转换即可
    { a - b } -> std::same_as<T>;   // 不需要减法
    { a * b } -> std::same_as<T>;   // 不需要乘法
};
```

#### 实践五：使用尾置requires提高可读性

```cpp
// 当约束很长时，尾置requires更清晰
template<typename T>
    requires std::integral<T> && (sizeof(T) >= 4) && std::is_signed_v<T>
T process(T val);

// 或者分行写
template<typename T>
T process(T val)
    requires std::integral<T>
          && (sizeof(T) >= 4)
          && std::is_signed_v<T>;
```

### 2. 常见陷阱

#### 陷阱一：requires表达式中的副作用

```cpp
// ❌ 错误：requires表达式中的函数调用不会被实际执行
template<typename T>
concept HasInit = requires(T t) {
    t.initialize();  // 这只是检查语法合法性，不会真的调用
};

// ✅ 正确理解：requires只做编译期检查
struct Bad {
    void initialize() { std::cout << "init\n"; }  // 有副作用
};
static_assert(HasInit<Bad>);  // ✅ 通过，但initialize()从未被调用
```

#### 陷阱二：concept定义中的循环依赖

```cpp
// ❌ 错误：A依赖B，B依赖A
template<typename T>
concept A = requires(T t) { t.foo(); } && B<T>;  // B还没定义！

template<typename T>
concept B = requires(T t) { t.bar(); } && A<T>;  // 循环依赖

// ✅ 正确：拆分约束，避免循环
template<typename T>
concept HasFoo = requires(T t) { t.foo(); };

template<typename T>
concept HasBar = requires(T t) { t.bar(); };

template<typename T>
concept A = HasFoo<T> && HasBar<T>;
```

#### 陷阱三：约束auto与模板推导冲突

```cpp
// ⚠️ 注意：约束auto在函数参数中会创建不同的模板
void f(std::integral auto a);  // 模板1
void f(std::floating_point auto a);  // 模板2（不同模板！）

f(42);    // 调用模板1
f(3.14);  // 调用模板2

// 但如果两个参数都使用约束auto，它们是独立的模板参数
void g(std::integral auto a, std::integral auto b);
g(42, 42L);  // ✅ a是int，b是long，都是integral
```

#### 陷阱四：concept不是类型

```cpp
// ❌ 错误：concept不是类型，不能用作变量类型
std::integral x = 42;  // 编译错误！

// ✅ 正确：使用约束auto
std::integral auto x = 42;  // OK

// ❌ 错误：不能对concept取sizeof
sizeof(std::integral);  // 编译错误！

// ✅ 正确：concept是编译期bool谓词
static_assert(std::integral<int>);  // OK
```

#### 陷阱五：过度使用concept

```cpp
// ❌ 不好的做法：对每个函数都加concept约束
template<std::integral T>
void print(T val) { std::cout << val << std::endl; }
// 如果函数对类型没有实际约束要求，不要加concept

// ✅ 好的做法：只在确实需要约束时使用
template<typename T>
void print(const T& val) { std::cout << val << std::endl; }
// print不需要约束，任何可打印类型都能用
```

#### 陷阱六：复合要求中的类型约束过严

```cpp
// ❌ 不好的做法：使用same_as过严
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::same_as<T>;  // 1 + 2 返回int，但如果T是short则失败
};

// ✅ 好的做法：使用convertible_to更宽松
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;  // short + short → int，可转换回short
};
```

***

## 13. 本章小结

| 主题 | 核心内容 |
|------|---------|
| **Concepts的价值** | 更好的错误信息、更清晰的接口、更好的重载决议 |
| **自定义Concept** | `requires`表达式、组合（合取/析取）、多参数concept |
| **requires子句 vs 表达式** | 子句约束模板，表达式检查语法合法性 |
| **约束auto** | 变量、返回值、函数参数中的约束auto |
| **约束模板** | 类模板、成员函数、别名模板的约束 |
| **标准Concept** | 类型/比较/对象/可调用/迭代器/范围 |
| **基于Concept的重载** | subsumption规则、歧义处理 |
| **基于Concept的偏特化** | 类模板偏特化、多参数偏特化 |
| **约束与继承** | CRTP与Concepts、静态多态 |
| **vs SFINAE** | 更可读、错误信息更好、组合更简单 |
| **最佳实践** | 优先标准concept、语义命名、宽松约束 |
| **常见陷阱** | 副作用、循环依赖、concept不是类型、过度约束 |

***

**上一章：** [第21章：C++17新特性](./21-C++17新特性.md)\
**下一章：** [第23章：Ranges](./23-Ranges.md)
