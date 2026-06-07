# Type Traits与类型操作

> C++类型系统操作核心：type_traits头文件与编译期类型计算

---

> **Type traits are the gateway to compile-time type manipulation.**
> （类型特征是编译期类型操作的入口。）

> **enable_if：让模板只接受"对的类型"。**
> （enable_if: make templates accept only "the right types".）

---

> **🎯 类型特征：编译期的类型检查与变换工具箱。**

> 💡 **通俗理解 - 什么是Type Traits？**

想象你在工厂流水线上：
- **类型判断**：检查产品是"红色"还是"蓝色"（is_integral, is_pointer）
- **类型修改**：给产品"贴标签"或"换包装"（add_pointer, remove_const）
- **条件选择**：根据条件选择"方案A"或"方案B"（conditional, enable_if）

**Type Traits就是编译期的"类型检查员和改装车间"！**

```cpp
#include <type_traits>

// 类型判断：int是整数类型吗？是的！
static_assert(std::is_integral_v<int>);       // true
static_assert(!std::is_integral_v<double>);   // false

// 类型修改：去掉const
using T = std::remove_const_t<const int>;     // T = int
static_assert(std::is_same_v<T, int>);

// 条件选择：根据条件选择类型
using Ptr = std::conditional_t<true, int*, double*>;  // Ptr = int*
static_assert(std::is_same_v<Ptr, int*>);
```

> 🔬 **抽象理解 - Type Traits的本质**：
> - **类型判断（Unary Type Traits）**：编译期查询类型的属性，返回bool值
> - **类型修改（Transformation Traits）**：编译期变换类型，返回新类型
> - **条件类型（Type Relations）**：编译期比较类型间的关系
> - **enable_if**：基于类型条件启用/禁用模板，SFINAE的核心工具
> - **自定义type trait**：通过模板特化扩展类型特征系统

---

## 📚 难度分级与推荐阅读

> **本文档采用三级难度标注：**
> - 🟢 **入门级**：基础概念，适合初学者
> - 🟡 **进阶级**：需要一定基础，适合有经验的开发者
> - 🔴 **高阶级**：深入底层原理，适合高级开发者

### 推荐阅读范围

| 读者类型 | 建议阅读范围 | 跳过内容 |
|---------|------------|---------|
| **初学者** | 🟢 1.1–1.3、🟢 2.1–2.3、🟢 3.1–3.3、🟢 4.1–4.2 | 🔴 2.5 底层实现原理、🔴 4.3–4.4 void_t与组合检测、🔴 5.3–5.7 高级应用与性能分析 |
| **中级开发者** | 全部 🟢 和 🟡 章节 | 🔴 5.5 性能影响分析、🔴 5.7 真实项目应用 |
| **高级开发者/专家** | 全文阅读 | — |

---

## 前置知识
- [编译期计算基础](00-编译期计算基础.md)
- [模板基础](../../02-CPP/10-模板基础.md)

## 后续内容
- [SFINAE与替换失败](02-SFINAE与替换失败.md)

## 目录

- [1. type_traits头文件概览](#1-type_traits头文件概览)
- [2. 类型判断traits](#2-类型判断traits)
- [3. 类型修改traits](#3-类型修改traits)
- [4. 条件类型traits](#4-条件类型traits)
- [5. 自定义type trait](#5-自定义type-trait)

---

## 1. type_traits头文件概览

### 1.1 概念与定义

**type_traits**：C++11引入的`<type_traits>`头文件，提供了一套编译期类型操作的模板工具。这些模板在编译期对类型进行查询、比较和变换，是模板元编程的基础设施。

**Type Traits分类**：

| 类别 | 作用 | 示例 |
|------|------|------|
| 类型判断 | 查询类型属性 | `is_integral`, `is_pointer` |
| 类型修改 | 变换类型 | `remove_const`, `add_pointer` |
| 类型关系 | 比较类型 | `is_same`, `is_base_of` |
| 条件类型 | 根据条件选择类型 | `conditional`, `enable_if` |

### 1.2 命名约定

```cpp
// 类型判断traits：is_xxx 形式，返回bool
std::is_integral<int>::value   // C++11风格：通过::value获取结果
std::is_integral_v<int>        // C++17风格：通过_v变量模板获取结果

// 类型修改traits：xxx_t 形式，返回类型
std::remove_const<int>::type   // C++11风格：通过::type获取结果
std::remove_const_t<int>       // C++14风格：通过_t别名模板获取结果

// 类型关系traits：is_xxx 形式，返回bool
std::is_same<int, int>::value  // C++11风格
std::is_same_v<int, int>       // C++17风格
```

### 1.3 基本使用模式

```cpp
#include <type_traits>
#include <iostream>

// 模式1：编译期条件判断
template<typename T>
void check_type() {
    if constexpr (std::is_integral_v<T>) {
        std::cout << "整数类型" << std::endl;
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "浮点类型" << std::endl;
    } else {
        std::cout << "其他类型" << std::endl;
    }
}

// 模式2：编译期类型变换
template<typename T>
using CleanType = std::remove_cv_t<std::remove_reference_t<T>>;
// CleanType<const int&> = int

// 模式3：static_assert约束
template<typename T>
class Numeric {
    static_assert(std::is_arithmetic_v<T>, "T必须是算术类型");
    T value_;
public:
    Numeric(T v) : value_(v) {}
};
```

---

## 2. 类型判断traits

### 2.1 基础类型分类

```cpp
#include <type_traits>

// 基础类型分类traits
static_assert(std::is_void_v<void>);              // void类型
static_assert(std::is_null_pointer_v<std::nullptr_t>);  // 空指针类型
static_assert(std::is_integral_v<int>);           // 整数类型
static_assert(std::is_floating_point_v<double>);  // 浮点类型
static_assert(std::is_array_v<int[10]>);          // 数组类型
static_assert(std::is_pointer_v<int*>);           // 指针类型
static_assert(std::is_reference_v<int&>);         // 引用类型
static_assert(std::is_member_function_pointer_v<void(MyClass::*)()>);  // 成员函数指针

// 复合类型分类
static_assert(std::is_enum_v<Color>);             // 枚举类型
static_assert(std::is_union_v<MyUnion>);          // 联合体类型
static_assert(std::is_class_v<std::string>);      // 类类型（含struct）
static_assert(std::is_function_v<void(int)>);     // 函数类型
```

### 2.2 类型属性判断

```cpp
// CV限定符判断
static_assert(std::is_const_v<const int>);        // const修饰
static_assert(std::is_volatile_v<volatile int>);  // volatile修饰

// 类型特征判断
static_assert(std::is_signed_v<int>);             // 有符号类型
static_assert(std::is_unsigned_v<unsigned int>);  // 无符号类型
static_assert(std::is_trivial_v<int>);            // 平凡类型
static_assert(std::is_standard_layout_v<int>);    // 标准布局类型

// 构造/析构特征
static_assert(std::is_trivially_constructible_v<int>);           // 平凡可构造
static_assert(std::is_trivially_copy_constructible_v<int>);     // 平凡可拷贝构造
static_assert(std::is_trivially_move_constructible_v<int>);     // 平凡可移动构造
static_assert(std::is_trivially_destructible_v<int>);           // 平凡可析构

// 类型关系
static_assert(std::is_same_v<int, int>);                  // 相同类型
static_assert(!std::is_same_v<int, const int>);           // 不同类型
static_assert(std::is_base_of_v<Base, Derived>);          // 基类关系
static_assert(std::is_convertible_v<int, double>);        // 可转换
static_assert(std::is_constructible_v<std::string, const char*>);  // 可构造
```

### 2.3 实际应用：类型分派

```cpp
#include <type_traits>
#include <iostream>
#include <cmath>

// 根据类型选择最优实现
template<typename T>
auto abs_value(T x) {
    if constexpr (std::is_unsigned_v<T>) {
        return x;  // 无符号类型，绝对值就是自身
    } else if constexpr (std::is_floating_point_v<T>) {
        return std::fabs(x);  // 浮点数用fabs
    } else {
        return x < 0 ? -x : x;  // 有符号整数
    }
}

// 类型分派优化序列化
template<typename T>
void serialize(std::ostream& os, const T& value) {
    if constexpr (std::is_integral_v<T>) {
        // 整数：直接写入字节
        os.write(reinterpret_cast<const char*>(&value), sizeof(T));
    } else if constexpr (std::is_floating_point_v<T>) {
        // 浮点数：确保IEEE754格式
        os.write(reinterpret_cast<const char*>(&value), sizeof(T));
    } else if constexpr (std::is_same_v<T, std::string>) {
        // 字符串：先写长度，再写内容
        auto len = value.size();
        os.write(reinterpret_cast<const char*>(&len), sizeof(len));
        os.write(value.data(), len);
    } else {
        // 其他类型：调用其serialize方法
        value.serialize(os);
    }
}
```

### 2.4 C++20概念与类型判断

```cpp
#include <concepts>

// C++20概念提供了更优雅的类型约束方式
// 标准库预定义概念
template<std::integral T>        // 替代 is_integral_v
T gcd(T a, T b) { /* ... */ return a; }

template<std::floating_point T>  // 替代 is_floating_point_v
T normalize(T val) { /* ... */ return val; }

template<std::signed_integral T> // 有符号整数
T safe_divide(T a, T b) { /* ... */ return a; }

// 自定义概念组合多个type traits
template<typename T>
concept Serializable = requires(T t, std::ostream& os) {
    { t.serialize(os) } -> std::same_as<void>;
    { T::class_name() } -> std::convertible_to<std::string_view>;
};

template<Serializable T>
void save(const T& obj) {
    // 确保T满足Serializable概念
    obj.serialize(std::cout);
}
```

### 2.5 Type Traits底层实现原理

```cpp
// type_traits的实现原理：模板特化

// 以is_const为例，看看标准库是如何实现的
// 主模板：默认情况，不是const
template<typename T>
struct is_const_impl : std::false_type {};

// 偏特化：匹配const T
template<typename T>
struct is_const_impl<const T> : std::true_type {};

// 验证
static_assert(is_const_impl<int>::value == false);
static_assert(is_const_impl<const int>::value == true);
static_assert(is_const_impl<volatile int>::value == false);
static_assert(is_const_impl<const volatile int>::value == true);

// is_pointer的实现
template<typename T>
struct is_pointer_impl : std::false_type {};

template<typename T>
struct is_pointer_impl<T*> : std::true_type {};

static_assert(is_pointer_impl<int>::value == false);
static_assert(is_pointer_impl<int*>::value == true);
static_assert(is_pointer_impl<int**>::value == true);

// is_reference的实现
template<typename T>
struct is_lvalue_reference_impl : std::false_type {};

template<typename T>
struct is_lvalue_reference_impl<T&> : std::true_type {};

template<typename T>
struct is_rvalue_reference_impl : std::false_type {};

template<typename T>
struct is_rvalue_reference_impl<T&&> : std::true_type {};

// remove_const的实现
template<typename T>
struct remove_const_impl { using type = T; };

template<typename T>
struct remove_const_impl<const T> { using type = T; };

static_assert(std::is_same_v<remove_const_impl<const int>::type, int>);
static_assert(std::is_same_v<remove_const_impl<int>::type, int>);

// conditional的实现
template<bool Cond, typename IfTrue, typename IfFalse>
struct conditional_impl { using type = IfFalse; };

template<typename IfTrue, typename IfFalse>
struct conditional_impl<true, IfTrue, IfFalse> { using type = IfTrue; };

static_assert(std::is_same_v<conditional_impl<true, int, double>::type, int>);
static_assert(std::is_same_v<conditional_impl<false, int, double>::type, double>);
```

```cpp
// enable_if的实现原理
template<bool Cond, typename T = void>
struct enable_if_impl {};  // Cond为false时，没有type成员

template<typename T>
struct enable_if_impl<true, T> { using type = T; };  // Cond为true时，有type成员

// 使用SFINAE：当条件为false时，enable_if_impl<false>::type不存在
// 导致模板替换失败，触发SFINAE淘汰该候选

// 示例：理解enable_if的SFINAE机制
template<typename T>
typename enable_if_impl<std::is_integral_v<T>, T>::type
only_integers(T x) {
    return x * 2;
}

// 当T=int时：enable_if_impl<true, int>::type = int → 函数签名有效
// 当T=double时：enable_if_impl<false, double>没有::type → SFINAE淘汰

// 变量模板简化（C++17）
template<bool Cond, typename T = void>
using enable_if_t_impl = typename enable_if_impl<Cond, T>::type;
```

### 2.6 类型判断的常见陷阱

```cpp
// 陷阱1：is_const对指针的判断
static_assert(std::is_const_v<int* const>);     // 指针本身是const
static_assert(!std::is_const_v<const int*>);    // 指向const的指针，指针本身不是const
static_assert(!std::is_const_v<int*>);          // 非const指针

// 陷阱2：is_pointer对函数指针的判断
static_assert(std::is_pointer_v<int*>);         // true
static_assert(std::is_pointer_v<void(*)()>);    // true：函数指针也是指针
static_assert(!std::is_pointer_v<void()>);      // false：函数类型不是指针

// 陷阱3：is_class对enum class的判断
enum class Color { Red, Green, Blue };
static_assert(!std::is_class_v<Color>);         // enum class不是class
static_assert(std::is_enum_v<Color>);           // 是enum

// 陷阱4：is_same忽略CV限定符
static_assert(!std::is_same_v<int, const int>);        // 不同类型！
static_assert(!std::is_same_v<int, int&>);             // 不同类型！
static_assert(std::is_same_v<int, std::remove_const_t<const int>>);  // 去掉const后相同

// 陷阱5：is_base_of对自身的关系
struct Base {};
struct Derived : Base {};

static_assert(std::is_base_of_v<Base, Base>);      // true：类是自身的基类
static_assert(std::is_base_of_v<Base, Derived>);   // true
static_assert(!std::is_base_of_v<Derived, Base>);  // false

// 陷阱6：is_convertible不检查隐式转换的合法性
static_assert(std::is_convertible_v<int, double>);     // true：隐式转换
static_assert(std::is_convertible_v<double, int>);     // true：但可能丢失精度！
// is_convertible只检查语法上的可转换性，不检查语义正确性

// 陷阱7：is_trivially_copyable不等于可memcpy
struct WithPadding {
    char a;
    int b;  // 可能有填充字节
};
static_assert(std::is_trivially_copyable_v<WithPadding>);  // true
// 但memcpy可能复制填充字节的垃圾值，跨平台可能不一致
```

### 2.7 完整实战案例：类型安全的序列化框架

```cpp
#include <type_traits>
#include <iostream>
#include <string>
#include <vector>
#include <sstream>

// 类型安全的序列化框架：利用type_traits选择序列化策略

// 序列化策略选择
template<typename T, typename = void>
struct Serializer {
    // 默认：调用T的serialize方法
    static std::string serialize(const T& obj) {
        return obj.serialize();
    }
};

// 算术类型特化
template<typename T>
struct Serializer<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
    static std::string serialize(T val) {
        if constexpr (std::is_same_v<T, bool>) {
            return val ? "true" : "false";
        } else if constexpr (std::is_floating_point_v<T>) {
            std::ostringstream oss;
            oss << val;
            return oss.str();
        } else {
            return std::to_string(val);
        }
    }
};

// 字符串特化
template<>
struct Serializer<std::string> {
    static std::string serialize(const std::string& val) {
        return "\"" + val + "\"";
    }
};

// 容器特化
template<typename T>
struct Serializer<T, std::enable_if_t<
    std::is_same_v<T, std::vector<typename T::value_type>> &&
    !std::is_same_v<T, std::string>
>> {
    static std::string serialize(const T& container) {
        std::string result = "[";
        bool first = true;
        for (const auto& elem : container) {
            if (!first) result += ", ";
            first = false;
            result += Serializer<typename T::value_type>::serialize(elem);
        }
        result += "]";
        return result;
    }
};

// 通用序列化接口
template<typename T>
std::string serialize(const T& obj) {
    return Serializer<T>::serialize(obj);
}

// 使用
int main() {
    std::cout << serialize(42) << std::endl;           // 42
    std::cout << serialize(3.14) << std::endl;         // 3.14
    std::cout << serialize(true) << std::endl;         // true
    std::cout << serialize(std::string("hello")) << std::endl;  // "hello"
    std::cout << serialize(std::vector<int>{1, 2, 3}) << std::endl;  // [1, 2, 3]
    return 0;
}
```

---

## 3. 类型修改traits

### 3.1 CV限定符操作

```cpp
#include <type_traits>

// 移除const和volatile
using T1 = std::remove_const_t<const int>;          // T1 = int
using T2 = std::remove_volatile_t<volatile int>;    // T2 = int
using T3 = std::remove_cv_t<const volatile int>;    // T3 = int

// 添加const和volatile
using T4 = std::add_const_t<int>;                   // T4 = const int
using T5 = std::add_volatile_t<int>;                // T5 = volatile int
using T6 = std::add_cv_t<int>;                      // T6 = const volatile int

// 组合使用：移除所有修饰得到原始类型
template<typename T>
using BareType = std::remove_cv_t<std::remove_reference_t<T>>;

static_assert(std::is_same_v<BareType<const int&>, int>);
static_assert(std::is_same_v<BareType<volatile int&&>, int>);
static_assert(std::is_same_v<BareType<const volatile int>, int>);
```

### 3.2 引用与指针操作

```cpp
// 移除引用
using R1 = std::remove_reference_t<int&>;           // R1 = int
using R2 = std::remove_reference_t<int&&>;          // R2 = int

// 添加引用
using R3 = std::add_lvalue_reference_t<int>;        // R3 = int&
using R4 = std::add_rvalue_reference_t<int>;        // R4 = int&&

// 移除指针
using P1 = std::remove_pointer_t<int*>;             // P1 = int
using P2 = std::remove_pointer_t<int**>;            // P2 = int*

// 添加指针
using P3 = std::add_pointer_t<int>;                 // P3 = int*
using P4 = std::add_pointer_t<int&>;                // P4 = int*

// 递归移除指针（自定义）
template<typename T>
struct remove_all_pointers {
    using type = T;
};

template<typename T>
struct remove_all_pointers<T*> {
    using type = typename remove_all_pointers<T>::type;
};

template<typename T>
using remove_all_pointers_t = typename remove_all_pointers<T>::type;

static_assert(std::is_same_v<remove_all_pointers_t<int***>, int>);
```

### 3.3 数组与函数操作

```cpp
// 移除数组边界
using A1 = std::remove_extent_t<int[10]>;           // A1 = int
using A2 = std::remove_extent_t<int[10][20]>;       // A2 = int[20]
using A3 = std::remove_all_extents_t<int[10][20]>;  // A3 = int

// decay：模拟按值传递的类型退化
using D1 = std::decay_t<int[10]>;                   // D1 = int*
using D2 = std::decay_t<const int&>;                // D2 = int
using D3 = std::decay_t<void(int)>;                 // D3 = void(*)(int)

// decay的三个规则：
// 1. 数组 -> 指针
// 2. 函数 -> 函数指针
// 3. 移除CV和引用

// 实际应用：完美转发的类型处理
template<typename T>
void forward_example(T&& arg) {
    // decay_t去除引用和CV，得到值类型
    using ValueType = std::decay_t<T>;
    ValueType copy = arg;  // 创建副本
}
```

### 3.4 常见类型修改组合

```cpp
// 组合1：获取纯净类型（去除引用、CV、指针）
template<typename T>
struct bare_type {
    using type = std::remove_cv_t<
        std::remove_reference_t<
            std::remove_pointer_t<T>
        >
    >;
};

// 组合2：统一值类型（用于容器元素类型推导）
template<typename T>
using value_type_of = std::remove_cv_t<
    std::remove_reference_t<T>
>;

// 组合3：函数返回类型推导
template<typename F, typename... Args>
using return_type_of = std::invoke_result_t<F, Args...>;

// 组合4：智能指针元素类型
template<typename T>
struct element_type_of {
    using type = T;
};

template<typename T>
struct element_type_of<std::vector<T>> {
    using type = T;
};

template<typename T, std::size_t N>
struct element_type_of<std::array<T, N>> {
    using type = T;
};

template<typename T>
using element_type_of_t = typename element_type_of<std::decay_t<T>>::type;
```

---

## 4. 条件类型traits

### 4.1 conditional：编译期类型选择

```cpp
#include <type_traits>

// conditional<条件, 真类型, 假类型>
using Type1 = std::conditional_t<true, int, double>;    // Type1 = int
using Type2 = std::conditional_t<false, int, double>;   // Type2 = double

// 实际应用：根据条件选择容器类型
template<bool UseVector, typename T>
using Container = std::conditional_t<
    UseVector,
    std::vector<T>,
    std::list<T>
>;

Container<true, int> vec;    // std::vector<int>
Container<false, int> lst;   // std::list<int>

// 实际应用：根据大小选择整数类型
template<std::size_t Size>
using uint_of_size = std::conditional_t<
    Size <= 1, uint8_t,
    std::conditional_t<
        Size <= 2, uint16_t,
        std::conditional_t<
            Size <= 4, uint32_t,
            uint64_t
        >
    >
>;

static_assert(std::is_same_v<uint_of_size<1>, uint8_t>);
static_assert(std::is_same_v<uint_of_size<3>, uint32_t>);
static_assert(std::is_same_v<uint_of_size<8>, uint64_t>);
```

### 4.2 enable_if：条件启用模板

```cpp
#include <type_traits>
#include <iostream>

// enable_if<条件, 类型=void>
// 条件为真：有type成员，类型为指定类型
// 条件为假：没有type成员，SFINAE淘汰

// 方式1：函数返回类型中使用enable_if
template<typename T>
typename std::enable_if_t<std::is_integral_v<T>, T>
max_value(T a, T b) {
    return a > b ? a : b;
}

// 方式2：函数参数中使用enable_if
template<typename T>
std::string to_string(T value,
    typename std::enable_if_t<std::is_arithmetic_v<T>>* = nullptr)
{
    return std::to_string(value);
}

// 方式3：模板参数中使用enable_if
template<typename T,
         typename = std::enable_if_t<std::is_floating_point_v<T>>>
T sqrt_safe(T value) {
    return value >= 0 ? std::sqrt(value) : T{0};
}

// 方式4：C++17简化写法（if constexpr替代部分enable_if）
template<typename T>
auto process(T value) {
    if constexpr (std::is_integral_v<T>) {
        return value * 2;
    } else if constexpr (std::is_floating_point_v<T>) {
        return value + 0.5;
    } else {
        return value;
    }
}
```

### 4.3 enable_if的常见模式

```cpp
// 模式1：重载决议——不同类型不同实现
template<typename T>
std::enable_if_t<std::is_integral_v<T>, std::string>
convert(T value) {
    return "整数: " + std::to_string(value);
}

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, std::string>
convert(T value) {
    return "浮点: " + std::to_string(value);
}

// 模式2：SFINAE友好的类型特征
template<typename T, typename = void>
struct has_size_method : std::false_type {};

template<typename T>
struct has_size_method<T, std::void_t<decltype(std::declval<T>().size())>>
    : std::true_type {};

static_assert(has_size_method<std::vector<int>>::value);
static_assert(!has_size_method<int>::value);

// 模式3：禁用某些重载
class NonCopyable {
public:
    NonCopyable() = default;

    // 禁用左值拷贝，只允许移动
    template<typename T,
             typename = std::enable_if_t<!std::is_lvalue_reference_v<T>>>
    NonCopyable(T&&) { /* 移动构造 */ }
};
```

### 4.4 void_t：SFINAE的万能工具

```cpp
// void_t：C++17引入，将一组类型特征映射到void
// 如果任何类型特征导致替换失败，整个特化被淘汰

// 检测成员类型
template<typename T, typename = void>
struct has_value_type : std::false_type {};

template<typename T>
struct has_value_type<T, std::void_t<typename T::value_type>>
    : std::true_type {};

static_assert(has_value_type<std::vector<int>>::value);
static_assert(!has_value_type<int>::value);

// 检测成员函数
template<typename T, typename = void>
struct has_begin_end : std::false_type {};

template<typename T>
struct has_begin_end<T, std::void_t<
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end())
>> : std::true_type {};

static_assert(has_begin_end<std::vector<int>>::value);
static_assert(!has_begin_end<int>::value);

// 检测运算符
template<typename T, typename = void>
struct is_addable : std::false_type {};

template<typename T>
struct is_addable<T, std::void_t<
    decltype(std::declval<T>() + std::declval<T>())
>> : std::true_type {};

static_assert(is_addable<int>::value);
static_assert(is_addable<std::string>::value);
```

---

## 5. 自定义type trait

### 5.1 基本自定义trait

```cpp
// 自定义类型判断trait
template<typename T>
struct is_pointer_to_const : std::false_type {};

template<typename T>
struct is_pointer_to_const<const T*> : std::true_type {};

static_assert(is_pointer_to_const<const int*>::value);
static_assert(!is_pointer_to_const<int*>::value);

// 自定义类型修改trait
template<typename T>
struct add_const_if_integral {
    using type = T;
};

template<typename T>
    requires std::is_integral_v<T>
struct add_const_if_integral<T> {
    using type = const T;
};

template<typename T>
using add_const_if_integral_t = typename add_const_if_integral<T>::type;

static_assert(std::is_same_v<add_const_if_integral_t<int>, const int>);
static_assert(std::is_same_v<add_const_if_integral_t<double>, double>);
```

### 5.2 检测成员的trait

```cpp
#include <type_traits>

// 检测是否有序列化方法
template<typename T, typename = void>
struct is_serializable : std::false_type {};

template<typename T>
struct is_serializable<T, std::void_t<
    decltype(std::declval<const T&>().serialize(std::declval<std::ostream&>()))
>> : std::true_type {};

// 检测是否有toString方法
template<typename T, typename = void>
struct has_to_string : std::false_type {};

template<typename T>
struct has_to_string<T, std::void_t<
    decltype(std::declval<const T&>().toString())
>> : std::true_type {};

// 检测是否可迭代
template<typename T, typename = void>
struct is_iterable : std::false_type {};

template<typename T>
struct is_iterable<T, std::void_t<
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end()),
    decltype(++std::declval<decltype(std::declval<T>().begin())&>()),
    decltype(*std::declval<T>().begin())
>> : std::true_type {};

static_assert(is_iterable<std::vector<int>>::value);
static_assert(!is_iterable<int>::value);
```

### 5.3 编译期类型计算trait

```cpp
// 计算类型的对齐要求
template<typename... Types>
struct max_alignment {
    static constexpr std::size_t value = std::max({
        alignof(Types)...
    });
};

template<typename... Types>
inline constexpr std::size_t max_alignment_v = max_alignment<Types...>::value;

static_assert(max_alignment_v<int, double, char> == alignof(double));

// 选择最优的整数类型
template<std::size_t Bits>
struct uint_least {
    using type = std::conditional_t<
        Bits <= 8, uint_least8_t,
        std::conditional_t<
            Bits <= 16, uint_least16_t,
            std::conditional_t<
                Bits <= 32, uint_least32_t,
                uint_least64_t
            >
        >
    >;
};

template<std::size_t Bits>
using uint_least_t = typename uint_least<Bits>::type;

// 编译期类型列表操作
template<typename... Ts>
struct type_list {};

template<typename List>
struct type_list_size;

template<typename... Ts>
struct type_list_size<type_list<Ts...>> {
    static constexpr std::size_t value = sizeof...(Ts);
};

template<typename List>
inline constexpr std::size_t type_list_size_v = type_list_size<List>::value;

static_assert(type_list_size_v<type_list<int, double, char>> == 3);
```

### 5.4 trait的组合与复用

```cpp
// 组合多个trait形成复杂约束
template<typename T>
struct is_string_like : std::bool_constant<
    std::is_same_v<std::decay_t<T>, std::string> ||
    std::is_same_v<std::decay_t<T>, std::string_view> ||
    std::is_same_v<std::decay_t<T>, const char*>
> {};

// 通用容器元素类型trait
template<typename T, typename = void>
struct element_type {
    using type = T;
};

template<typename T>
struct element_type<T, std::void_t<typename T::value_type>> {
    using type = typename T::value_type;
};

template<typename T>
using element_type_t = typename element_type<T>::type;

static_assert(std::is_same_v<element_type_t<std::vector<int>>, int>);
static_assert(std::is_same_v<element_type_t<int>, int>);

// 通用迭代器trait
template<typename T, typename = void>
struct iterator_type_of {
    using type = typename T::iterator;
};

template<typename T>
struct iterator_type_of<T, std::void_t<decltype(std::declval<T>().begin())>> {
    using type = decltype(std::declval<T>().begin());
};
```

### 5.5 Type Traits的性能影响

```cpp
// Type Traits的性能分析：
// 1. 编译期开销：type_traits在编译期执行，增加编译时间
// 2. 运行时开销：零！type_traits的结果在编译期确定
// 3. 代码膨胀：不同类型实例化不同版本的模板

// 示例：if constexpr vs 虚函数的性能对比
#include <chrono>

// 方式A：if constexpr + type_traits
template<typename T>
auto compute(T x) {
    if constexpr (std::is_integral_v<T>) {
        return x * 2;
    } else if constexpr (std::is_floating_point_v<T>) {
        return x + 0.5;
    } else {
        return x;
    }
}

// 方式B：虚函数
class Base {
public:
    virtual double compute() = 0;
    virtual ~Base() = default;
};

class IntImpl : public Base {
    int x_;
public:
    IntImpl(int x) : x_(x) {}
    double compute() override { return x_ * 2; }
};

class DoubleImpl : public Base {
    double x_;
public:
    DoubleImpl(double x) : x_(x) {}
    double compute() override { return x_ + 0.5; }
};

// 性能对比
void benchmark() {
    const int N = 100000000;

    // if constexpr版本
    {
        auto start = std::chrono::high_resolution_clock::now();
        volatile long long sum = 0;
        for (int i = 0; i < N; ++i) {
            sum += compute(i);  // 直接调用，可内联
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "if constexpr: "
                  << std::chrono::duration<double>(end - start).count()
                  << "秒" << std::endl;
    }

    // 虚函数版本
    {
        IntImpl impl(0);
        Base* base = &impl;
        auto start = std::chrono::high_resolution_clock::now();
        volatile long long sum = 0;
        for (int i = 0; i < N; ++i) {
            sum += static_cast<long long>(base->compute());  // 虚函数调用
        }
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "虚函数: "
                  << std::chrono::duration<double>(end - start).count()
                  << "秒" << std::endl;
    }
}
// 结果：if constexpr版本通常快2-5倍（因为内联消除了函数调用开销）
```

### 5.6 Type Traits在真实项目中的应用

```cpp
// 应用1：STL容器接口约束
// C++20标准库使用Concepts（基于type_traits）约束容器接口
template<typename T>
concept Container = requires(T t) {
    { t.begin() } -> std::input_or_output_iterator;
    { t.end() } -> std::input_or_output_iterator;
    { t.size() } -> std::convertible_to<std::size_t>;
    typename T::value_type;
};

// 应用2：序列化库的类型分发
// Protobuf、FlatBuffers等库使用type_traits选择序列化方式
template<typename T>
void serialize_field(std::ostream& os, const T& field) {
    if constexpr (std::is_arithmetic_v<T>) {
        // 算术类型：直接写入二进制
        os.write(reinterpret_cast<const char*>(&field), sizeof(T));
    } else if constexpr (std::is_same_v<T, std::string>) {
        // 字符串：先写长度，再写内容
        auto len = field.size();
        os.write(reinterpret_cast<const char*>(&len), sizeof(len));
        os.write(field.data(), len);
    } else if constexpr (requires { field.begin(); field.end(); }) {
        // 容器：先写元素数量，再逐个序列化
        std::size_t count = field.size();
        os.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (const auto& elem : field) {
            serialize_field(os, elem);
        }
    } else {
        // 自定义类型：调用其serialize方法
        field.serialize(os);
    }
}

// 应用3：智能指针的类型特征
template<typename T>
struct is_smart_pointer : std::false_type {};

template<typename T>
struct is_smart_pointer<std::unique_ptr<T>> : std::true_type {};

template<typename T>
struct is_smart_pointer<std::shared_ptr<T>> : std::true_type {};

template<typename T>
struct is_smart_pointer<std::weak_ptr<T>> : std::true_type {};

template<typename T>
inline constexpr bool is_smart_pointer_v = is_smart_pointer<T>::value;

// 使用：自动解引用智能指针
template<typename T>
auto& deref(T& ptr) {
    if constexpr (is_smart_pointer_v<std::decay_t<T>>) {
        return *ptr;
    } else {
        return ptr;
    }
}

// 应用4：Eigen库中的类型特征
// Eigen使用大量type_traits来优化数学运算
template<typename T>
struct scalar_traits {
    using Real = T;
    using Scalar = T;
    static constexpr int NumTraits = 1;
};

template<>
struct scalar_traits<double> {
    using Real = double;
    using Scalar = double;
    static constexpr int NumTraits = 1;
    static constexpr bool IsInteger = false;
    static constexpr bool IsSigned = true;
};
```

### 5.7 Type Traits最佳实践

```cpp
// 实践1：优先使用_v和_t后缀
// C++11风格（冗长）
bool b1 = std::is_integral<int>::value;
using T1 = std::remove_const<const int>::type;

// C++14/17风格（推荐）
bool b2 = std::is_integral_v<int>;
using T2 = std::remove_const_t<const int>;

// 实践2：自定义trait遵循标准库模式
template<typename T, typename = void>
struct has_serialize : std::false_type {};

template<typename T>
struct has_serialize<T, std::void_t<
    decltype(std::declval<const T&>().serialize(std::declval<std::ostream&>()))
>> : std::true_type {};

// 提供变量模板
template<typename T>
inline constexpr bool has_serialize_v = has_serialize<T>::value;

// 实践3：组合trait时使用变量模板
template<typename T>
inline constexpr bool is_numeric_v = std::is_arithmetic_v<T> && !std::is_same_v<T, bool>;

template<typename T>
inline constexpr bool is_stringish_v = std::is_same_v<std::decay_t<T>, std::string> ||
                                        std::is_same_v<std::decay_t<T>, const char*> ||
                                        std::is_same_v<std::decay_t<T>, std::string_view>;

// 实践4：使用Concepts替代复杂的enable_if
// 不推荐
template<typename T,
         typename = std::enable_if_t<std::is_arithmetic_v<T> && !std::is_same_v<T, bool>>>
T compute(T x) { return x * 2; }

// 推荐（C++20）
template<typename T> requires std::is_arithmetic_v<T> && (!std::is_same_v<T, bool>)
T compute(T x) { return x * 2; }

// 更推荐：定义命名概念
template<typename T>
concept Numeric = std::is_arithmetic_v<T> && !std::is_same_v<T, bool>;

template<Numeric T>
T compute(T x) { return x * 2; }

// 实践5：使用decay_t处理模板参数
template<typename T>
void process(T&& arg) {
    // T&&是转发引用，T可能包含引用和CV限定符
    // 使用decay_t获取值类型
    using ValueType = std::decay_t<T>;

    if constexpr (std::is_integral_v<ValueType>) {
        // 处理整数
    } else if constexpr (std::is_floating_point_v<ValueType>) {
        // 处理浮点数
    }
}

// 实践6：避免过度使用type_traits
// 简单场景不需要type_traits
// 不推荐：对已知类型使用type_traits
static_assert(std::is_integral_v<int>);  // 显然是true，没必要
// 推荐：对模板参数使用type_traits
template<typename T>
void process(T x) {
    static_assert(std::is_integral_v<T>, "T必须是整数类型");  // 有意义
}
```

---

## 小结

本章介绍了C++类型特征系统的核心内容：

| 类别 | 代表trait | 作用 |
|------|----------|------|
| 类型判断 | `is_integral`, `is_pointer`, `is_class` | 查询类型属性 |
| 类型修改 | `remove_const`, `add_pointer`, `decay` | 变换类型 |
| 类型关系 | `is_same`, `is_base_of`, `is_convertible` | 比较类型关系 |
| 条件类型 | `conditional`, `enable_if` | 条件选择类型 |
| 检测工具 | `void_t`, 自定义检测trait | 检测类型接口 |

**关键要点**：

1. **优先使用`_v`和`_t`后缀**：C++14/17的简化写法更简洁
2. **enable_if是SFINAE的核心工具**：理解其三种使用模式
3. **void_t是检测trait的万能工具**：一行代码检测任意类型特征
4. **if constexpr可以替代部分enable_if**：代码更清晰
5. **C++20概念是类型约束的未来**：比enable_if更优雅
6. **自定义trait遵循标准库模式**：特化false_type/true_type

Type Traits是模板元编程的基础工具，掌握它们将为SFINAE和更高级的编译期技术打下基础。
