# SFINAE 与 Type Traits
> 📖 相关章节：[模板基础](../../02-CPP/10-模板基础.md)、[模板进阶](../../02-CPP/11-模板进阶.md)、[Concepts](../../02-CPP/22-Concepts.md)

### 1. 本质速解

**SFINAE**（Substitution Failure Is Not An Error）= **替换失败不是错误**：模板参数替换失败时，编译器不会报错，而是默默跳过这个重载。配合 `enable_if` 和 `type_traits`，可以在编译期根据类型特征选择不同的实现。

***

### 2. SFINAE 是什么

```cpp
template <typename T>
typename T::value_type foo(T t);  // 如果 T 没有 value_type，替换失败，跳过

template <typename T>
void foo(T t);                     // 走这个备选
```

**关键**：替换失败 ≠ 编译错误，只是这个模板不参与重载决议。

SFINAE 的全称是 **Substitution Failure Is Not An Error**，发生在模板参数推导（deduction）和替换（substitution）阶段。当编译器尝试用具体类型替换模板参数时，如果产生了非法类型或表达式，该重载候选被静默丢弃，而非触发编译错误。

### 3. SFINAE 的触发场景

```cpp
// 场景1：访问不存在的类型成员
template <typename T>
typename T::value_type func(T);  // T=int 时，int::value_type 不存在 → SFINAE

// 场景2：对非类类型使用 sizeof
template <typename T>
auto func(T) -> decltype(sizeof(T::x), void());  // T 不是类类型 → SFINAE

// 场景3：enable_if 条件为 false
template <typename T, typename = std::enable_if_t<std::is_integral_v<T>>>
void func(T);  // T=float 时，条件为 false → SFINAE

// 场景4：decltype 表达式非法
template <typename T>
auto func(T t) -> decltype(t + t);  // T=void* 时，void*+void* 非法 → SFINAE
```

**不是 SFINAE 的场景**（这些会报编译错误）：

```cpp
// 模板定义本身的语法错误 → 编译错误
template <typename T>
void func(T t) { t.nonexistent(); }  // 如果 T 没有 nonexistent()，实例化时报错

// 注意：函数体内的错误不是 SFINAE
// SFINAE 只发生在"直接上下文"（immediate context）中
```

### 4. 经典 SFINAE 用法

#### 1. 方法1：函数重载 + `enable_if`

```cpp
#include <type_traits>
#include <iostream>

// 整数版本
template <typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
process(T val) {
    std::cout << "integer: ";
    return val * 2;
}

// 浮点版本
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
process(T val) {
    std::cout << "float: ";
    return val * 2.5;
}

int main() {
    std::cout << process(10) << std::endl;     // integer: 20
    std::cout << process(3.14) << std::endl;   // float: 7.85
}
```

#### 2. 方法2：模板参数默认值

```cpp
template <typename T, typename = typename std::enable_if<std::is_integral<T>::value>::type>
void process(T val) {
    // 整数版本
}

// 注意：这种方法可能导致歧义
// 如果两个重载只有默认模板参数不同，可能无法区分
```

#### 3. 方法3：非类型模板参数

```cpp
template <typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
void process(T val) {
    // 整数版本
    // 使用 int = 0 避免方法2的歧义问题
}

template <typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
void process(T val) {
    // 浮点版本
}
```

#### 4. 方法4：C++17 `if constexpr`（推荐）

```cpp
template <typename T>
auto process(T val) {
    if constexpr (std::is_integral_v<T>) {
        return val * 2;      // 整数分支
    } else if constexpr (std::is_floating_point_v<T>) {
        return val * 2.5;    // 浮点分支
    } else {
        return val;          // 其他类型
    }
}
```

### 5. `enable_if` 详解

```cpp
// enable_if 的实现原理
template <bool Cond, typename T = void>
struct enable_if {};

template <typename T>
struct enable_if<true, T> {
    using type = T;
};

// Cond 为 true → 有 type 成员 → 正常参与重载
// Cond 为 false → 没有 type 成员 → SFINAE 触发，跳过此重载
```

#### 1. `enable_if` 的三种放置位置

```cpp
// 位置1：返回类型（最常见）
template <typename T>
typename std::enable_if<std::is_integral_v<T>, T>::type
func(T val) { return val; }

// 位置2：模板参数（避免歧义）
template <typename T, typename std::enable_if<std::is_integral_v<T>, int>::type = 0>
void func(T val) {}

// 位置3：函数参数
template <typename T>
void func(T val, typename std::enable_if<std::is_integral_v<T>>::type* = nullptr) {}
```

| 位置 | 优点 | 缺点 |
|------|------|------|
| 返回类型 | 直观 | 混淆函数签名 |
| 模板参数 | 不影响函数签名 | 语法稍复杂 |
| 函数参数 | 灵活 | 增加虚假参数 |

#### 2. C++14 简写

```cpp
// C++11: 需要加 ::type
typename std::enable_if<std::is_integral<T>::value, T>::type func(T);

// C++14: 用 _t 后缀
std::enable_if_t<std::is_integral<T>::value, T> func(T);

// C++17: 用 _v 后缀进一步简化
std::enable_if_t<std::is_integral_v<T>, T> func(T);
```

### 6. 常用 Type Traits

#### 1. 类型判断 traits

| Trait | 作用 | 示例 |
|-------|------|------|
| `is_integral<T>` | T 是否整数类型 | `is_integral_v<int>` → true |
| `is_floating_point<T>` | T 是否浮点类型 | `is_floating_point_v<double>` → true |
| `is_pointer<T>` | T 是否指针 | `is_pointer_v<int*>` → true |
| `is_reference<T>` | T 是否引用 | `is_reference_v<int&>` → true |
| `is_const<T>` | T 是否 const | `is_const_v<const int>` → true |
| `is_array<T>` | T 是否数组 | `is_array_v<int[10]>` → true |
| `is_enum<T>` | T 是否枚举 | `is_enum_v<Color>` → true |
| `is_class<T>` | T 是否 class/struct | `is_class_v<string>` → true |
| `is_function<T>` | T 是否函数类型 | `is_function_v<void(int)>` → true |
| `is_void<T>` | T 是否 void | `is_void_v<void>` → true |

#### 2. 类型关系 traits

| Trait | 作用 | 示例 |
|-------|------|------|
| `is_same<T, U>` | T 和 U 是否同一类型 | `is_same_v<int, int32_t>` → true |
| `is_base_of<Base, Derived>` | Base 是否 Derived 的基类 | `is_base_of_v<Animal, Dog>` → true |
| `is_convertible<From, To>` | From 能否隐式转为 To | `is_convertible_v<int, double>` → true |
| `is_constructible<T, Args...>` | T 是否可用 Args 构造 | `is_constructible_v<string, const char*>` → true |
| `is_assignable<T, U>` | T 能否被 U 赋值 | `is_assignable_v<int&, int>` → true |

#### 3. 类型变换 traits

| Trait | 作用 |
|-------|------|
| `remove_const<T>` | 去掉 const |
| `remove_reference<T>` | 去掉引用 |
| `remove_pointer<T>` | 去掉指针 |
| `add_const<T>` | 添加 const |
| `add_pointer<T>` | 添加指针 |
| `decay<T>` | 模拟函数参数的退化 |
| `conditional<B, T, F>` | B 为 true 则 T，否则 F |
| `common_type<T...>` | 多个类型的公共类型 |

```cpp
// 类型变换示例
std::remove_const_t<const int>       // → int
std::remove_reference_t<int&>        // → int
std::remove_pointer_t<int*>          // → int
std::decay_t<const int&>             // → int（退化：去const、去引用）
std::conditional_t<true, int, double> // → int
std::common_type_t<int, double>      // → double
```

### 7. 函数重载 SFINAE

```cpp
#include <type_traits>
#include <iostream>
#include <vector>
#include <string>

// 版本1：容器类型（有 begin/end）
template <typename T>
auto describe(const T& val) -> decltype(val.begin(), val.end(), std::string()) {
    return "container with " + std::to_string(val.size()) + " elements";
}

// 版本2：数值类型
template <typename T>
std::enable_if_t<std::is_arithmetic_v<T>, std::string>
describe(const T& val) {
    return "numeric value: " + std::to_string(val);
}

// 版本3：字符串
template <typename T>
std::enable_if_t<std::is_same_v<std::decay_t<T>, const char*>, std::string>
describe(T val) {
    return std::string("C-string: ") + val;
}

int main() {
    std::vector<int> v = {1, 2, 3};
    std::cout << describe(v) << std::endl;      // container with 3 elements
    std::cout << describe(42) << std::endl;      // numeric value: 42
    std::cout << describe("hello") << std::endl; // C-string: hello
}
```

### 8. 类模板 SFINAE

```cpp
#include <type_traits>
#include <iostream>

// 只对整数类型启用的类
template <typename T, typename = void>
class SafeInteger {
    static_assert(std::is_integral_v<T>, "T must be integral");
};

template <typename T>
class SafeInteger<T, std::enable_if_t<std::is_integral_v<T>>> {
    T value_;
public:
    explicit SafeInteger(T v) : value_(v) {}
    T get() const { return value_; }
    SafeInteger operator+(const SafeInteger& other) const {
        return SafeInteger(value_ + other.value_);
    }
};

// 只对浮点类型启用的类
template <typename T>
class FloatingPoint {
    static_assert(std::is_floating_point_v<T>, "T must be floating point");
    T value_;
public:
    explicit FloatingPoint(T v) : value_(v) {}
    T get() const { return value_; }
    FloatingPoint operator+(const FloatingPoint& other) const {
        return FloatingPoint(value_ + other.value_);
    }
};

int main() {
    SafeInteger<int> a(10);
    SafeInteger<int> b(20);
    auto c = a + b;  // c.get() == 30

    // SafeInteger<double> d(3.14);  // 编译错误：double 不是整数
}
```

### 9. C++17 `if constexpr` 详解

`if constexpr` 是 SFINAE 的现代替代方案，在编译期进行分支选择：

```cpp
#include <type_traits>
#include <iostream>
#include <string>
#include <vector>

template <typename T>
std::string to_string_impl(const T& val) {
    if constexpr (std::is_integral_v<T>) {
        return "int: " + std::to_string(val);
    } else if constexpr (std::is_floating_point_v<T>) {
        return "float: " + std::to_string(val);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return "string: " + val;
    } else {
        return "unknown type";
    }
}

// 递归模板 + if constexpr
template <typename T>
void print_if_container(const T& val) {
    if constexpr (std::is_class_v<T>) {
        // 编译期检查是否有 size() 方法
        if constexpr (requires { val.size(); }) {  // C++20
            std::cout << "container, size=" << val.size() << std::endl;
        } else {
            std::cout << "class, not container" << std::endl;
        }
    } else {
        std::cout << "not a class: " << val << std::endl;
    }
}

int main() {
    std::cout << to_string_impl(42) << std::endl;       // int: 42
    std::cout << to_string_impl(3.14) << std::endl;     // float: 3.14
    std::cout << to_string_impl(std::string("hi")) << std::endl; // string: hi
}
```

#### 1. `if constexpr` vs SFINAE 对比

| 特性 | SFINAE + enable_if | if constexpr |
|------|:---:|:---:|
| 可读性 | 差 | 好 |
| 错误信息 | 复杂 | 清晰 |
| 函数重载选择 | 支持 | 不支持（在同一个函数内） |
| 代码组织 | 分散在多个重载 | 集中在一个函数 |
| 编译速度 | 较慢（多个实例化） | 较快 |
| 适用场景 | 需要不同重载时 | 同一函数内分支选择 |

### 10. C++20 Concepts（终极方案）

Concepts 是 SFINAE 的正式替代品，用命名约束替代 `enable_if` 的晦涩技巧，提供更清晰的语法和更好的错误信息。

C++20 Concepts 的详细语法、标准概念库和迁移指南，请参阅 → [什么是C++20-Concepts](./06-什么是C++20-Concepts.md)

#### 1. SFINAE vs Concepts 语法速查

| SFINAE 写法 | Concepts 写法 |
|-------------|--------------|
| `typename enable_if_t<is_integral_v<T>, T> func(T)` | `T func(T) requires integral<T>` |
| `enable_if_t<is_integral_v<T>, int> = 0` | `template<integral T>` |
| `void_t<decltype(expr)>` | `requires { expr; }` |
| `is_integral_v<T>` | `std::integral<T>` |
| `is_same_v<T, U>` | `std::same_as<T, U>` |

| 特性 | SFINAE + enable_if | Concepts |
|------|:---:|:---:|
| 可读性 | 差 | 好 |
| 错误信息 | 复杂 | 清晰 |
| 约束组合 | 嵌套 enable_if | `&&` `\|\|` |
| C++ 版本 | C++11 | C++20 |

### 11. SFINAE 的演进路线

```
C++98/03: 原始 SFINAE（只能用嵌套类型）
  ↓
C++11: enable_if + type_traits（正式支持 SFINAE 编程）
  ↓
C++14: _t 后缀简写（enable_if_t, remove_const_t）
  ↓
C++17: _v 后缀 + if constexpr（编译期分支，推荐替代 SFINAE）
  ↓
C++20: Concepts（最优雅，终极方案）
```

| C++ 版本 | 推荐方式 | 代码量 | 可读性 | 错误信息 |
|----------|----------|--------|--------|----------|
| C++11 | `enable_if` + `type_traits` | 多 | 差 | 复杂 |
| C++14 | `enable_if_t` + `_v` | 中 | 中 | 复杂 |
| C++17 | `if constexpr` | 少 | 好 | 清晰 |
| C++20 | Concepts | 最少 | 最好 | 最清晰 |

### 12. SFINAE 常见陷阱

#### 1. 陷阱1：SFINAE 只在直接上下文生效

```cpp
template <typename T>
void func(T val) {
    val.nonexistent();  // 这不是 SFINAE！这是实例化错误
    // SFINAE 只在函数签名/模板参数的"直接上下文"中生效
    // 函数体内的错误是硬错误，会导致编译失败
}
```

#### 2. 陷阱2：enable_if 的歧义

```cpp
// 错误：两个重载只有默认模板参数不同，可能歧义
template <typename T, typename = enable_if_t<is_integral_v<T>>>
void func(T) {}

template <typename T, typename = enable_if_t<is_floating_point_v<T>>>
void func(T) {}

// 正确：用非类型模板参数
template <typename T, enable_if_t<is_integral_v<T>, int> = 0>
void func(T) {}

template <typename T, enable_if_t<is_floating_point_v<T>, int> = 0>
void func(T) {}
```

#### 3. 陷阱3：过度使用 SFINAE

```cpp
// 不推荐：过度复杂的 SFINAE 链
template <typename T,
    typename = enable_if_t<is_integral_v<T>>,
    typename = enable_if_t<!is_same_v<T, bool>>,
    typename = enable_if_t<is_signed_v<T>>>
void func(T) {}

// 推荐：用 if constexpr 或 Concepts
template <typename T>
void func(T val) {
    if constexpr (std::is_integral_v<T> && !std::is_same_v<T, bool> && std::is_signed_v<T>) {
        // ...
    }
}
```

### 13. 完整示例：类型安全的序列化

```cpp
#include <type_traits>
#include <iostream>
#include <string>
#include <vector>

// SFINAE 版本（C++11）
template <typename T>
typename std::enable_if<std::is_arithmetic<T>::value, std::string>::type
serialize(const T& val) {
    return std::to_string(val);
}

template <typename T>
typename std::enable_if<std::is_same<T, std::string>::value, std::string>::type
serialize(const T& val) {
    return "\"" + val + "\"";
}

template <typename T>
typename std::enable_if<
    std::is_class<T>::value && !std::is_same<T, std::string>::value,
    std::string
>::type
serialize(const T& val) {
    return "{object}";
}

// if constexpr 版本（C++17）
template <typename T>
std::string serialize_modern(const T& val) {
    if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(val);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return "\"" + val + "\"";
    } else {
        return "{object}";
    }
}

// Concepts 版本（C++20）
// template <std::arithmetic T>
// std::string serialize_concept(const T& val) { return std::to_string(val); }

int main() {
    std::cout << serialize(42) << std::endl;               // 42
    std::cout << serialize(3.14) << std::endl;             // 3.14
    std::cout << serialize(std::string("hello")) << std::endl; // "hello"

    std::cout << serialize_modern(42) << std::endl;        // 42
    std::cout << serialize_modern(std::string("hi")) << std::endl; // "hi"
}
```

### 14. 极简总结

**SFINAE = 模板替换失败不报错 → enable_if 控制重载 → C++17 用 if constexpr → C++20 用 Concepts**

| 你的 C++ 版本 | 推荐方案 |
|---------------|----------|
| C++11 | `enable_if` + `type_traits` |
| C++14 | `enable_if_t` + `_v` 后缀 |
| C++17 | `if constexpr`（优先）+ SFINAE（需要重载时） |
| C++20 | Concepts（优先）+ `if constexpr`（函数内分支） |

***

### 相关阅读

- [什么是C++20-Concepts](./06-什么是C++20-Concepts.md)
- [decltype与auto](../04-CPP核心特性/24-decltype与auto.md)
- [什么是完美转发Perfect-Forwarding](../04-CPP核心特性/20-什么是完美转发Perfect-Forwarding.md)

***