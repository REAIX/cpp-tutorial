# SFINAE与替换失败

> C++模板元编程核心机制：SFINAE原理、模式与替代方案

---

> **SFINAE is not a bug, it's a feature.**
> （SFINAE不是缺陷，而是特性。）

> **替换失败不是错误，而是选择。**
> （Substitution failure is not an error, but a choice.）

---

> **🎯 SFINAE：让编译器帮你选择正确的重载。**

> 💡 **通俗理解 - 什么是SFINAE？**

想象你在餐厅点菜：
- **没有SFINAE**：菜单上有一道菜缺食材，厨师直接报错，整个餐厅关门
- **有SFINAE**：缺食材的菜自动从菜单上划掉，你从剩余菜品中选择

**SFINAE就是"这道菜做不了就换一道"的机制！**

```cpp
// 没有SFINAE的世界：只能为每种类型写一个函数
int process(int x) { return x * 2; }
double process(double x) { return x + 0.5; }
std::string process(std::string s) { return s + "!"; }

// 有SFINAE的世界：一个模板搞定，编译器自动选择
template<typename T>
std::enable_if_t<std::is_integral_v<T>, T>  // 整数走这里
process(T x) { return x * 2; }

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, T>  // 浮点走这里
process(T x) { return x + 0.5; }
```

> 🔬 **抽象理解 - SFINAE的本质**：
> - **替换（Substitution）**：编译器将模板参数替换为具体类型的过程
> - **失败（Failure）**：替换导致类型不匹配或表达式无效
> - **不是错误（Not An Error）**：替换失败不会导致编译错误，而是淘汰该候选
> - **重载决议**：编译器从剩余候选中选择最佳匹配
> - **SFINAE的用途**：根据类型特征选择不同的模板实现

---

## 📚 难度分级与推荐阅读

> **本文档采用三级难度标注：**
> - 🟢 **入门级**：基础概念，适合初学者
> - 🟡 **进阶级**：需要一定基础，适合有经验的开发者
> - 🔴 **专家级**：深入底层原理，适合高级开发者

### 推荐阅读范围

| 读者类型 | 建议阅读范围 | 跳过内容 |
|---------|------------|---------|
| **初学者** | 🟢 1.1–1.4、🟢 2.1–2.3、🟡 3.1–3.2、🟢 4.1–4.2、🟡 5.1–5.3 | 🔴 3.3–3.4 标签分发与表达式SFINAE、🔴 4.4–4.5 组合检测与局限、🔴 5.5–5.8 深度分析与实战 |
| **中级开发者** | 全部 🟢 和 🟡 章节 | 🔴 5.6 常见陷阱深度分析、🔴 5.8 真实项目应用 |
| **高级开发者/专家** | 全文阅读 | — |

---

## 前置知识
- [Type Traits与类型操作](01-Type-Traits与类型操作.md)
- [模板进阶](../../02-CPP/11-模板进阶.md)

## 后续内容
- [模板元编程模式](03-模板元编程模式.md)

## 目录

- [1. SFINAE原理](#1-sfinae原理)
- [2. 替换失败不是错误](#2-替换失败不是错误)
- [3. SFINAE的常见模式](#3-sfinae的常见模式)
- [4. void_t技术](#4-void_t技术)
- [5. SFINAE的局限与替代方案](#5-sfinae的局限与替代方案)

---

## 1. SFINAE原理

### 1.1 概念与定义

**SFINAE（Substitution Failure Is Not An Error）**：C++模板实例化规则。当编译器在模板参数替换阶段遇到失败时，不会产生编译错误，而是将该模板从候选列表中移除，继续尝试其他候选。

**SFINAE的作用时机**：仅在模板参数的**直接替换**阶段生效，不包括模板函数体内部的实例化错误。

### 1.2 模板实例化过程

```cpp
// 模板实例化的步骤：
// 1. 模板名称查找 -> 找到所有候选模板
// 2. 模板参数推导 -> 推导模板参数
// 3. 模板参数替换 -> 将推导结果替换到模板签名中（SFINAE在此阶段生效）
// 4. 重载决议 -> 从成功替换的候选中选择最佳匹配
// 5. 模板实例化 -> 实例化选中的模板（此阶段的错误不是SFINAE）

// 示例：SFINAE在替换阶段生效
template<typename T>
auto foo(T t) -> decltype(t.size()) {  // 替换阶段检查t.size()
    return t.size();
}

template<typename T>
auto foo(T t) -> decltype(t * 2) {  // 替换阶段检查t * 2
    return t * 2;
}

std::vector<int> v;
foo(v);   // 第一个foo：T=vector<int>，t.size()有效 -> 选中
foo(42);  // 第一个foo：T=int，t.size()无效 -> SFINAE淘汰
          // 第二个foo：T=int，t * 2有效 -> 选中
```

### 1.3 SFINAE的触发条件

```cpp
// 以下情况会触发SFINAE（替换失败，不是错误）：

// 1. 类型成员不存在
template<typename T>
void foo(typename T::value_type) {}  // T=int时，int::value_type不存在 -> SFINAE

// 2. 表达式无效
template<typename T>
auto bar(T t) -> decltype(t + t) { return t + t; }  // T=void*时，无法相加 -> SFINAE

// 3. 数组大小为负
template<typename T, std::size_t N>
void baz(T (&arr)[N]) {}  // N推导为负数时 -> SFINAE

// 4. 歧义
template<typename T>
void qux(T, T) {}  // 两个参数类型不同时推导歧义 -> SFINAE

// 5. enable_if条件为假
template<typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
void test(T) {}  // T=double时，enable_if失败 -> SFINAE
```

### 1.4 非SFINAE场景

```cpp
// 以下情况不会触发SFINAE（会产生编译错误）：

// 1. 函数体内的实例化错误
template<typename T>
void bad1(T t) {
    t.nonexistent_method();  // 不是替换阶段的错误，是实例化错误
}

// 2. 模板定义中的语法错误
template<typename T>
void bad2(T t) {
    return "语法错误";  // 语法错误，不是SFINAE
}

// 3. 链接错误
template<typename T>
void bad3(T t);  // 声明但不定义，链接错误

// 重要区别：
// SFINAE只保护"签名中的替换失败"
// 不保护"函数体中的实例化失败"
```

---

## 2. 替换失败不是错误

### 2.1 经典示例

```cpp
#include <iostream>
#include <vector>
#include <type_traits>

// 重载1：容器类型（有size方法）
template<typename T>
auto describe(const T& container)
    -> decltype(container.size(), std::size_t{})
{
    return container.size();
}

// 重载2：非容器类型
template<typename T>
int describe(const T&) {
    return sizeof(T);
}

int main() {
    std::vector<int> v{1, 2, 3};
    std::cout << describe(v) << std::endl;   // 调用重载1，输出3
    std::cout << describe(42) << std::endl;  // 调用重载2，输出4（sizeof(int)）
    return 0;
}
```

### 2.2 SFINAE与decltype

```cpp
#include <type_traits>
#include <string>

// 使用decltype检测表达式有效性
template<typename T>
auto to_string_impl(T val, int)  // 优先级高（int精确匹配）
    -> decltype(std::to_string(val))
{
    return std::to_string(val);
}

template<typename T>
std::string to_string_impl(T val, long)  // 优先级低（long需要转换）
{
    return val.toString();  // 假设T有toString方法
}

// 统一接口
template<typename T>
std::string to_string(T val) {
    return to_string_impl(val, 0);  // 0是int，优先匹配第一个重载
}

// 对于int：std::to_string(int)有效 -> 第一个重载
// 对于自定义类型：std::to_string(Custom)无效 -> SFINAE -> 第二个重载
```

### 2.3 SFINAE与函数重载

```cpp
#include <iostream>
#include <type_traits>

// 重载集：根据类型特征选择不同实现

// 版本1：整数类型
template<typename T>
typename std::enable_if_t<std::is_integral_v<T>, T>
clamp(T value, T lo, T hi) {
    std::cout << "整数clamp" << std::endl;
    return value < lo ? lo : (value > hi ? hi : value);
}

// 版本2：浮点类型
template<typename T>
typename std::enable_if_t<std::is_floating_point_v<T>, T>
clamp(T value, T lo, T hi) {
    std::cout << "浮点clamp" << std::endl;
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

// 版本3：自定义类型（有operator<）
template<typename T>
typename std::enable_if_t<
    !std::is_arithmetic_v<T> && std::is_class_v<T>,
    T
>
clamp(T value, T lo, T hi) {
    std::cout << "自定义类型clamp" << std::endl;
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

int main() {
    clamp(5, 0, 10);      // 整数clamp
    clamp(3.14, 0.0, 1.0); // 浮点clamp
    return 0;
}
```

---

## 3. SFINAE的常见模式

### 3.1 函数重载模式

```cpp
// 模式1：enable_if在返回类型中
template<typename T>
auto serialize(const T& value)
    -> std::enable_if_t<std::is_arithmetic_v<T>, std::string>
{
    return std::to_string(value);
}

template<typename T>
auto serialize(const T& value)
    -> std::enable_if_t<std::is_same_v<T, std::string>, std::string>
{
    return "\"" + value + "\"";
}

// 模式2：enable_if在默认模板参数中
template<typename T,
         typename std::enable_if_t<std::is_integral_v<T>, int> = 0>
std::string format(T value) {
    return std::to_string(value);
}

template<typename T,
         typename std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
std::string format(T value) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << value;
    return oss.str();
}

// 模式3：enable_if在函数参数中
template<typename T>
void print(T value,
           typename std::enable_if_t<std::is_integral_v<T>>* = nullptr)
{
    std::cout << "整数: " << value << std::endl;
}

template<typename T>
void print(T value,
           typename std::enable_if_t<std::is_floating_point_v<T>>* = nullptr)
{
    std::cout << "浮点: " << value << std::endl;
}
```

### 3.2 偏特化模式

```cpp
// SFINAE通过偏特化选择不同的类实现

// 通用版本
template<typename T, typename = void>
struct Serializer {
    static std::string serialize(const T& value) {
        return value.serialize();  // 假设T有serialize方法
    }
};

// 整数特化版本
template<typename T>
struct Serializer<T, std::enable_if_t<std::is_integral_v<T>>> {
    static std::string serialize(T value) {
        return std::to_string(value);
    }
};

// 浮点特化版本
template<typename T>
struct Serializer<T, std::enable_if_t<std::is_floating_point_v<T>>> {
    static std::string serialize(T value) {
        std::ostringstream oss;
        oss << value;
        return oss.str();
    }
};

// 容器特化版本
template<typename T>
struct Serializer<T, std::void_t<decltype(std::declval<T>().begin())>> {
    static std::string serialize(const T& container) {
        std::string result = "[";
        bool first = true;
        for (const auto& elem : container) {
            if (!first) result += ", ";
            first = false;
            result += Serializer<std::decay_t<decltype(elem)>>::serialize(elem);
        }
        result += "]";
        return result;
    }
};
```

### 3.3 标签分发模式

```cpp
#include <type_traits>
#include <iostream>

// 标签分发：不直接使用SFINAE，而是通过标签类型选择重载
// 比SFINAE更简洁，错误信息更友好

// 定义标签类型
struct integral_tag {};
struct floating_tag {};
struct other_tag {};

// 根据类型选择标签
template<typename T>
constexpr auto get_tag() {
    if constexpr (std::is_integral_v<T>) return integral_tag{};
    else if constexpr (std::is_floating_point_v<T>) return floating_tag{};
    else return other_tag{};
}

// 使用标签分发的实现
template<typename T>
T abs_value(T x, integral_tag) {
    return x < 0 ? -x : x;
}

template<typename T>
T abs_value(T x, floating_tag) {
    return std::fabs(x);
}

template<typename T>
T abs_value(T x, other_tag) {
    return x.abs();  // 假设T有abs方法
}

// 统一接口
template<typename T>
T abs_value(T x) {
    return abs_value(x, get_tag<T>());
}

int main() {
    std::cout << abs_value(-5) << std::endl;      // 5
    std::cout << abs_value(-3.14) << std::endl;   // 3.14
    return 0;
}
```

### 3.4 表达式SFINAE（C++11）

```cpp
// 表达式SFINAE：使用decltype检测表达式有效性

// 检测是否支持operator[]
template<typename T, typename Index>
auto safe_at(T& container, Index idx)
    -> decltype(container[idx])
{
    return container[idx];
}

template<typename T, typename Index>
auto safe_at(T& container, Index idx)
    -> decltype(container.at(idx))
{
    return container.at(idx);
}

// 检测是否支持迭代
template<typename T>
auto begin_or_ptr(T& container)
    -> decltype(container.begin())
{
    return container.begin();
}

template<typename T, std::size_t N>
T* begin_or_ptr(T (&arr)[N]) {
    return arr;
}

// 逗号运算符组合多个检测
template<typename T>
auto test_container(T& c)
    -> decltype(c.begin(), c.end(), c.size(), void())
{
    // T同时支持begin(), end(), size()
    std::cout << "完整容器" << std::endl;
}

template<typename T>
auto test_container(T& c)
    -> decltype(c.begin(), c.end(), void())
{
    // T只支持begin()和end()
    std::cout << "部分容器" << std::endl;
}
```

---

## 4. void_t技术

### 4.1 void_t的定义与原理

```cpp
// C++17引入的void_t定义
template<typename...>
using void_t = void;

// 原理：如果...中的任何类型导致替换失败，
// 整个别名模板的替换也会失败（SFINAE）

// void_t的妙用：检测类型特征
template<typename T, typename = void>
struct has_type_member : std::false_type {};

template<typename T>
struct has_type_member<T, std::void_t<typename T::type>> : std::true_type {};

// 检测过程：
// 1. 尝试特化：has_type_member<T, void_t<typename T::type>>
// 2. 如果T有type成员：void_t<typename T::type> = void，特化成功
// 3. 如果T没有type成员：void_t<typename T::type>替换失败，SFINAE淘汰
// 4. 回退到主模板：has_type_member<T, void> : false_type
```

### 4.2 void_t检测成员类型

```cpp
// 检测value_type
template<typename T, typename = void>
struct has_value_type : std::false_type {};

template<typename T>
struct has_value_type<T, std::void_t<typename T::value_type>>
    : std::true_type {};

static_assert(has_value_type<std::vector<int>>::value);
static_assert(!has_value_type<int>::value);

// 检测iterator
template<typename T, typename = void>
struct has_iterator : std::false_type {};

template<typename T>
struct has_iterator<T, std::void_t<typename T::iterator>>
    : std::true_type {};

static_assert(has_iterator<std::vector<int>>::value);

// 检测嵌套模板
template<typename T, typename = void>
struct has_allocator_type : std::false_type {};

template<typename T>
struct has_allocator_type<T, std::void_t<typename T::allocator_type>>
    : std::true_type {};

static_assert(has_allocator_type<std::vector<int>>::value);
```

### 4.3 void_t检测成员函数

```cpp
// 检测无参成员函数
template<typename T, typename = void>
struct has_default_constructor : std::false_type {};

template<typename T>
struct has_default_constructor<T, std::void_t<decltype(T{})>>
    : std::true_type {};

// 检测带参成员函数
template<typename T, typename = void>
struct has_push_back : std::false_type {};

template<typename T>
struct has_push_back<T, std::void_t<
    decltype(std::declval<T&>().push_back(std::declval<typename T::value_type>()))
>> : std::true_type {};

static_assert(has_push_back<std::vector<int>>::value);
static_assert(!has_push_back<std::array<int, 5>>::value);

// 检测成员函数返回类型
template<typename T, typename = void>
struct has_size_method : std::false_type {};

template<typename T>
struct has_size_method<T, std::void_t<
    decltype(std::declval<const T&>().size())
>> : std::true_type {};

static_assert(has_size_method<std::vector<int>>::value);
static_assert(!has_size_method<int>::value);
```

### 4.4 void_t检测运算符

```cpp
// 检测operator==
template<typename T, typename = void>
struct is_equality_comparable : std::false_type {};

template<typename T>
struct is_equality_comparable<T, std::void_t<
    decltype(std::declval<const T&>() == std::declval<const T&>())
>> : std::true_type {};

static_assert(is_equality_comparable<int>::value);
static_assert(is_equality_comparable<std::string>::value);

// 检测operator<
template<typename T, typename = void>
struct is_less_comparable : std::false_type {};

template<typename T>
struct is_less_comparable<T, std::void_t<
    decltype(std::declval<const T&>() < std::declval<const T&>())
>> : std::true_type {};

static_assert(is_less_comparable<int>::value);

// 检测operator<<（可输出）
template<typename T, typename = void>
struct is_ostreamable : std::false_type {};

template<typename T>
struct is_ostreamable<T, std::void_t<
    decltype(std::declval<std::ostream&>() << std::declval<const T&>())
>> : std::true_type {};

static_assert(is_ostreamable<int>::value);
static_assert(is_ostreamable<std::string>::value);
```

### 4.5 void_t的组合检测

```cpp
// 同时检测多个特征
template<typename T, typename = void>
struct is_container : std::false_type {};

template<typename T>
struct is_container<T, std::void_t<
    typename T::value_type,
    typename T::iterator,
    decltype(std::declval<T&>().begin()),
    decltype(std::declval<T&>().end()),
    decltype(std::declval<T&>().size())
>> : std::true_type {};

static_assert(is_container<std::vector<int>>::value);
static_assert(is_container<std::string>::value);
static_assert(!is_container<int>::value);

// 检测可调用对象
template<typename F, typename... Args>
struct is_callable : std::false_type {};

template<typename F, typename... Args>
struct is_callable<F, Args..., std::void_t<
    decltype(std::declval<F>()(std::declval<Args>()...))
>> : std::true_type {};

// 注意：上面的写法有语法问题，正确写法需要偏特化
template<typename F, typename Sig, typename = void>
struct is_callable_with : std::false_type {};

template<typename F, typename R, typename... Args>
struct is_callable_with<F, R(Args...), std::void_t<
    decltype(std::declval<F>()(std::declval<Args>()...))
>> : std::true_type {};
```

---

## 5. SFINAE的局限与替代方案

### 5.1 SFINAE的局限

```cpp
// 局限1：错误信息晦涩
// SFINAE失败时，编译器不会告诉你"为什么失败"
// 只会告诉你"没有匹配的重载"

// 局限2：组合条件复杂
// 多个enable_if嵌套，代码可读性差
template<typename T,
         typename = std::enable_if_t<
             std::is_arithmetic_v<T> &&
             !std::is_same_v<T, bool> &&
             !std::is_same_v<T, char>
         >>
void complex_constraint(T) {}

// 局限3：SFINAE不保护函数体
template<typename T>
auto bad_example(T t)
    -> decltype(t.size())  // SFINAE保护签名
{
    t.nonexistent();  // 不受SFINAE保护，实例化时编译错误
    return t.size();
}

// 局限4：难以调试
// 当SFINAE意外淘汰了所有候选时，错误信息不明确
```

### 5.2 if constexpr替代

```cpp
// C++17 if constexpr：更简洁的条件分支
// 不需要SFINAE，直接在编译期选择代码分支

template<typename T>
auto process(T value) {
    if constexpr (std::is_integral_v<T>) {
        return value * 2;       // 整数：乘法
    } else if constexpr (std::is_floating_point_v<T>) {
        return value + 0.5;     // 浮点：加法
    } else if constexpr (requires { value.size(); }) {
        return value.size();    // 容器：返回大小
    } else {
        return value;           // 其他：原样返回
    }
}

// if constexpr vs SFINAE：
// 优点：代码更清晰，错误信息更友好
// 缺点：不能用于选择不同的函数签名（返回类型必须兼容）
```

### 5.3 C++20 Concepts替代

```cpp
#include <concepts>

// Concepts：SFINAE的现代替代方案
// 更简洁、更清晰的类型约束

// 定义概念
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<typename T>
concept Container = requires(T t) {
    t.begin();
    t.end();
    t.size();
    typename T::value_type;
};

// 使用概念约束模板
template<Numeric T>
T clamp(T value, T lo, T hi) {
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

template<Container C>
std::size_t count(const C& container) {
    return container.size();
}

// 概念的requires子句
template<typename T>
    requires requires(T t) { t.serialize(); }
std::string to_json(const T& obj) {
    return obj.serialize();
}

// 概念的错误信息更友好
// int x = clamp(true, false, true);  // 错误：bool不满足Numeric
```

### 5.4 三种方案对比

```cpp
// 场景：实现一个只接受整数类型的函数

// 方案1：SFINAE（C++11）
template<typename T>
typename std::enable_if_t<std::is_integral_v<T>, T>
square_sfinae(T x) { return x * x; }

// 方案2：if constexpr（C++17）
template<typename T>
auto square_constexpr(T x) {
    static_assert(std::is_integral_v<T>, "T必须是整数类型");
    return x * x;
}

// 方案3：Concepts（C++20）
template<std::integral T>
T square_concept(T x) { return x * x; }

// 对比：
// | 特性           | SFINAE        | if constexpr | Concepts    |
// |---------------|---------------|--------------|-------------|
// | 标准版本       | C++11         | C++17        | C++20       |
// | 代码可读性     | 差            | 好           | 最好        |
// | 错误信息       | 晦涩          | 一般         | 清晰        |
// | 重载选择       | 支持          | 有限         | 支持        |
// | 函数签名影响   | 可以          | 不可以       | 可以        |
// | 学习曲线       | 陡峭          | 平缓         | 平缓        |

// 推荐策略：
// C++20及以上：优先使用Concepts
// C++17：优先使用if constexpr，必要时用SFINAE
// C++11/14：只能用SFINAE
```

### 5.5 SFINAE的最佳实践

```cpp
// 实践1：优先使用标准库type_traits
// 不要重复造轮子
template<typename T>
using is_integer = std::is_integral<T>;  // 使用标准库

// 实践2：封装SFINAE逻辑
// 将复杂的SFINAE封装为命名的trait
template<typename T, typename = void>
struct is_serializable : std::false_type {};

template<typename T>
struct is_serializable<T, std::void_t<
    decltype(std::declval<const T&>().serialize(std::declval<std::ostream&>()))
>> : std::true_type {};

template<typename T>
inline constexpr bool is_serializable_v = is_serializable<T>::value;

// 使用封装好的trait
template<typename T>
std::enable_if_t<is_serializable_v<T>> save(const T& obj) {
    obj.serialize(std::cout);
}

// 实践3：使用变量模板简化
template<typename T>
inline constexpr bool is_numeric_v = std::is_arithmetic_v<T> &&
                                      !std::is_same_v<T, bool>;

// 实践4：文档化SFINAE约束
// 为每个SFINAE约束添加注释，说明为什么需要这个约束
```

### 5.6 SFINAE常见陷阱深度分析

```cpp
// 陷阱1：SFINAE只保护直接上下文
template<typename T>
auto bad_func(T t) -> decltype(t.size()) {
    t.nonexistent_method();  // 不受SFINAE保护！
    return t.size();
}
// 当T=int时，t.size()导致SFINAE淘汰 → OK
// 当T=vector<int>时，t.size()有效，但t.nonexistent_method()导致编译错误
// 这个错误不是SFINAE，因为发生在函数体内

// 陷阱2：enable_if的歧义
template<typename T,
         typename = std::enable_if_t<std::is_integral_v<T>>>
void func(T) {}  // 重载1

template<typename T,
         typename = std::enable_if_t<std::is_floating_point_v<T>>>
void func(T) {}  // 重载2
// 问题：两个重载的默认模板参数不同，但签名相同！
// 调用func(42)时，两个重载都匹配，导致歧义

// 正确做法：使用不同的函数签名
template<typename T,
         std::enable_if_t<std::is_integral_v<T>, int> = 0>
void func_fixed(T) {}  // 重载1

template<typename T,
         std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
void func_fixed(T) {}  // 重载2
// 现在模板参数不同，不会歧义

// 陷阱3：SFINAE与auto返回类型的交互
template<typename T>
auto process(T t) -> decltype(t.foo()) {
    return t.foo();
}

template<typename T>
auto process(T t) -> decltype(t.bar()) {
    return t.bar();
}
// 如果T同时有foo()和bar()，两个重载都有效，导致歧义
// 解决：添加额外的SFINAE条件或使用if constexpr

// 陷阱4：SFINAE与构造函数
template<typename T>
struct Wrapper {
    // 错误：构造函数不能使用返回类型SFINAE
    // template<typename U>
    // std::enable_if_t<std::is_convertible_v<U, T>> Wrapper(U&& u);

    // 正确：使用默认模板参数
    template<typename U,
             std::enable_if_t<std::is_convertible_v<U, T>, int> = 0>
    Wrapper(U&& u) : value(std::forward<U>(u)) {}

    T value;
};

// 陷阱5：SFINAE与重载解析的优先级
template<typename T>
auto to_string(T val, int)  // 优先级高（int精确匹配）
    -> decltype(std::to_string(val)) {
    return std::to_string(val);
}

template<typename T>
std::string to_string(T val, long) {  // 优先级低（long需要转换）
    return val.toString();
}

// 注意：使用int/long标签分派时，确保优先级正确
```

### 5.7 SFINAE完整实战案例：通用容器操作

```cpp
#include <iostream>
#include <vector>
#include <array>
#include <list>
#include <type_traits>
#include <string>

// 完整案例：使用SFINAE为不同容器类型提供最优操作

// 检测容器是否有operator[]
template<typename T, typename = void>
struct has_subscript : std::false_type {};

template<typename T>
struct has_subscript<T, std::void_t<
    decltype(std::declval<T&>()[std::declval<std::size_t>()])
>> : std::true_type {};

template<typename T>
inline constexpr bool has_subscript_v = has_subscript<T>::value;

// 检测容器是否有push_back
template<typename T, typename = void>
struct has_push_back : std::false_type {};

template<typename T>
struct has_push_back<T, std::void_t<
    decltype(std::declval<T&>().push_back(std::declval<typename T::value_type>()))
>> : std::true_type {};

template<typename T>
inline constexpr bool has_push_back_v = has_push_back<T>::value;

// 检测容器是否有reserve
template<typename T, typename = void>
struct has_reserve : std::false_type {};

template<typename T>
struct has_reserve<T, std::void_t<
    decltype(std::declval<T&>().reserve(std::declval<std::size_t>()))
>> : std::true_type {};

template<typename T>
inline constexpr bool has_reserve_v = has_reserve<T>::value;

// 通用填充函数：根据容器能力选择最优策略
template<typename Container>
void fill_container(Container& c, std::size_t n,
                    const typename Container::value_type& val) {
    if constexpr (has_reserve_v<Container> && has_push_back_v<Container>) {
        // vector/string等：先reserve再push_back
        c.reserve(n);
        for (std::size_t i = 0; i < n; ++i) {
            c.push_back(val);
        }
    } else if constexpr (has_push_back_v<Container>) {
        // list/deque等：直接push_back
        for (std::size_t i = 0; i < n; ++i) {
            c.push_back(val);
        }
    } else if constexpr (has_subscript_v<Container>) {
        // array等：通过下标赋值
        for (std::size_t i = 0; i < n && i < c.size(); ++i) {
            c[i] = val;
        }
    } else {
        // 其他：使用迭代器
        for (auto it = c.begin(); it != c.end() && n > 0; ++it, --n) {
            *it = val;
        }
    }
}

// 通用随机访问函数
template<typename Container>
decltype(auto) safe_at(Container& c, std::size_t idx) {
    if constexpr (has_subscript_v<Container>) {
        return c[idx];  // 随机访问容器
    } else {
        auto it = c.begin();
        std::advance(it, idx);  // 顺序访问容器
        return *it;
    }
}

int main() {
    std::vector<int> vec;
    fill_container(vec, 5, 42);
    std::cout << "vector[2] = " << safe_at(vec, 2) << std::endl;  // 42

    std::list<int> lst;
    fill_container(lst, 5, 10);
    std::cout << "list[2] = " << safe_at(lst, 2) << std::endl;  // 10

    std::array<int, 5> arr{};
    fill_container(arr, 5, 7);
    std::cout << "array[2] = " << safe_at(arr, 2) << std::endl;  // 7
    return 0;
}
```

### 5.8 SFINAE在真实项目中的应用

```cpp
// 应用1：STL中的SFINAE
// std::vector的构造函数使用SFINAE区分不同重载
template<typename T, typename Alloc>
class vector {
    // 仅当InputIt满足迭代器要求时才启用
    template<typename InputIt,
             typename = std::enable_if_t<
                 std::is_base_of_v<std::input_iterator_tag,
                     typename std::iterator_traits<InputIt>::iterator_category>
             >>
    vector(InputIt first, InputIt last);

    // 仅当count和value参数不是迭代器时才启用
    vector(size_type count, const T& value);
};

// 应用2：std::function的SFINAE
// std::function使用SFINAE确保可调用对象签名匹配
template<typename>
class function;

template<typename R, typename... Args>
class function<R(Args...)> {
    template<typename F,
             typename = std::enable_if_t<
                 !std::is_same_v<std::decay_t<F>, function> &&
                 std::is_invocable_r_v<R, F&, Args...>
             >>
    function(F&& f);
};

// 应用3：Boost库中的SFINAE
// Boost.Range使用SFINAE区分容器和数组
template<typename T>
auto begin(T& r) -> decltype(r.begin()) {
    return r.begin();  // 容器版本
}

template<typename T, std::size_t N>
T* begin(T (&arr)[N]) {
    return arr;  // 数组版本
}

// 应用4：现代C++库中的SFINAE→Concepts迁移
// fmtlib使用SFINAE检测可格式化类型
template<typename T, typename = void>
struct is_formattable : std::false_type {};

template<typename T>
struct is_formattable<T, std::void_t<
    decltype(std::declval<formatter<T>>().format(
        std::declval<T>(), std::declval<format_context&>()))
>> : std::true_type {};
```

---

## 小结

本章介绍了SFINAE的核心原理和常见模式：

| 模式 | 适用场景 | 复杂度 |
|------|---------|--------|
| enable_if + 函数重载 | 根据类型选择不同实现 | 中 |
| enable_if + 偏特化 | 根据类型选择不同类实现 | 中 |
| 标签分发 | 多种类型的不同实现 | 低 |
| void_t检测 | 检测类型是否具有某特征 | 中 |
| if constexpr | 简单条件分支 | 低 |
| Concepts | C++20类型约束 | 低 |

**关键要点**：

1. **SFINAE只保护替换阶段的失败**：函数体内的错误不受保护
2. **void_t是检测trait的利器**：简洁优雅地检测类型特征
3. **标签分发比SFINAE更简洁**：错误信息更友好
4. **if constexpr替代部分SFINAE**：代码更清晰
5. **Concepts是SFINAE的现代替代**：C++20应优先使用
6. **封装SFINAE逻辑**：不要在业务代码中直接使用复杂的SFINAE

SFINAE是C++模板元编程的核心机制，理解其原理对于阅读和编写模板库至关重要。随着C++20 Concepts的普及，SFINAE的使用将逐渐减少，但在可预见的未来仍需掌握。
