# 什么是 Type Traits
> 📖 相关章节：[模板进阶](../../02-CPP/11-模板进阶.md)、[编译期计算基础](../../07-模板元编程与编译期计算/00-编译期计算基础.md)、[Type Traits](../../07-模板元编程与编译期计算/01-Type-Traits与类型操作.md)

## 要义概览

**Type Traits（类型特征）是 C++ 模板元编程的"类型雷达系统"——它让你在编译期对任意类型进行"体检"：判断是不是整数、有没有拷贝构造、是不是指针、能不能转换，甚至还能按需"改造"类型——去掉 const、加引用、挑出元素类型，是整个 C++ 泛型编程基础设施的地基。**

---

## 1. Type Traits 的设计哲学

### 1.1 为什么需要 Type Traits？

```cpp
// 没有 Type Traits 的世界：你只能靠特化来区分类型
template<typename T>
struct MyContainer {
    // 怎么知道 T 是指针？是数组？是 const 的？
    // 没有统一的方法，只能为每种情况写特化...
};

// 有了 Type Traits：统一的类型查询接口
#include <type_traits>

template<typename T>
struct SmartContainer {
    // 一行代码就知道类型的"一切"
    static constexpr bool is_pointer_type = std::is_pointer_v<T>;
    static constexpr bool is_integral_type = std::is_integral_v<T>;
    using value_type = std::remove_cvref_t<T>;  // 去掉引用和 cv 限定符
};
```

### 1.2 设计原则

```cpp
// 原则 1：一切都是编译期常量
// 所有 trait 的 ::value 都是 constexpr bool 或 constexpr 整数
static_assert(std::is_integral_v<int> == true);
static_assert(std::is_pointer_v<int> == false);
static_assert(std::remove_reference_t<int&> 是 int 类型);

// 原则 2：统一的命名约定
// is_xxx_v     → 类型判断（返回 bool）
// xxx_t        → 类型修改/转换（返回类型）
// xxx_v        → C++14 的 _v 简写（省去 ::value）

// 原则 3：组合性 —— 小 trait 组合成大 trait
template<typename T>
constexpr bool is_non_const_lvalue_reference() {
    return std::is_lvalue_reference_v<T> &&
           !std::is_const_v<std::remove_reference_t<T>>;
}

static_assert(is_non_const_lvalue_reference<int&>());
static_assert(!is_non_const_lvalue_reference<const int&>());
static_assert(!is_non_const_lvalue_reference<int>());
```

### 1.3 Type Traits 的历史演进

```cpp
// C++11：type_traits 诞生，约 100+ 个 trait
// C++14：添加 _t 和 _v 变量模板简化语法
// C++17：添加 more traits（如 is_swappable_with, invocation_type）
// C++20：添加 Concepts（与 traits 互补而非替代）
// C++23：继续扩充（如 remove_cvref, is_scoped_enum 等）
```

---

## 2. 类型判断 Traits（Is-Type Traits）

### 2.1 基础类别判断

```cpp
#include <type_traits>
#include <string>
#include <vector>
#include <memory>
#include <iostream>

void fundamental_type_checks() {
    // ====== 基础类型分类 ======
    static_assert(std::is_integral_v<int>);            // 整型
    static_assert(std::is_integral_v<unsigned long>);
    static_assert(std::is_integral_v<char>);
    static_assert(std::is_integral_v<bool>);

    static_assert(std::is_floating_point_v<float>);    // 浮点型
    static_assert(std::is_floating_point_v<double>);
    static_assert(std::is_floating_point_v<long double>);

    static_assert(std::is_arithmetic_v<int>);          // 算术类型（整型+浮点）
    static_assert(std::is_arithmetic_v<double>);
    static_assert(!std::is_arithmetic_v<std::string>);

    static_assert(std::is_void_v<void>);               // void 类型
    static_assert(std::is_null_pointer_v<decltype(nullptr)>);  // nullptr_t

    // ====== 复合类型判断 ======
    static_assert(std::is_lvalue_reference_v<int&>);   // 左值引用
    static_assert(std::is_rvalue_reference_v<int&&>);  // 右值引用
    static_assert(std::is_reference_v<const int&>);    // 引用（统称）

    static_assert(std::is_pointer_v<int*>);            // 原始指针
    static_assert(std::is_pointer_v<const char*>);
    static_assert(!std::is_pointer_v<std::shared_ptr<int>>);  // ⚠️ 智能指针不是 pointer！

    static_assert(std::is_array_v<int[]>);             // 数组
    static_assert(std::is_array_v<int[10]>);
    static_assert(std::is_array_v<char[3][4]>);        // 多维数组

    // ====== 成员指针判断 ======
    struct MyClass { int value; void func(); };
    static_assert(std::is_member_object_pointer_v<int MyClass::*>);     // 数据成员指针
    static_assert(std::is_member_function_pointer_v<void (MyClass::*)()>); // 成员函数指针

    // ====== 函数类型判断 ======
    static_assert(std::is_function_v<void(int)>);       // 函数类型
    static_assert(!std::is_function_v<decltype(&func)>);// 函数指针不是函数类型
    static_assert(std::is_function_v<void(int, double, char)>);
}
```

### 2.2 CV 限定符判断

```cpp
void cv_qualifier_checks() {
    // const / volatile 限定符检测
    static_assert(std::is_const_v<const int>);
    static_assert(std::is_const_v<const int&>);   // 引用的底层类型是 const
    static_assert(!std::is_const_v<int*>);        // 指针本身不是 const
    static_assert(std::is_const_v<const int*>);   // 指向的内容是 const

    static_assert(std::is_volatile_v<volatile int>);
    static_assert(std::is_volatile_v<volatile const int>);

    // 组合判断
    static_assert(std::is_const_v<const volatile int>);
    static_assert(std::is_volatile_v<const volatile int>);
}
```

### 2.3 类型属性判断

```cpp
#include <type_traits>

struct TrivialType {
    int x;
    double y;
    // 无自定义构造/析构/赋值 → 平凡的（trivial）
};

struct NonTrivialType {
    int* ptr;
    NonTrivialType() : ptr(new int(42)) {}  // 自定义构造 → 非平凡的
    ~NonTrivialType() { delete ptr; }
};

struct EmptyType {};  // 空类

struct FinalType final {};  // final 类

enum class ScopedEnum { A, B };  // 作用域枚举

void property_checks() {
    // ====== 构造/析构/赋值属性 ======
    static_assert(std::is_trivially_constructible_v<TrivialType>);
    static_assert(!std::is_trivially_constructible_v<NonTrivialType>);

    static_assert(std::is_trivially_destructible_v<TrivialType>);
    static_assert(!std::is_trivially_destructible_v<NonTrivialType>);

    static_assert(std::is_trivially_copyable_v<TrivialType>);
    static_assert(!std::is_trivially_copyable_v<NonTrivialType>);

    static_assert(std::is_trivially_assignable_v<TrivialType&, const TrivialType&>);

    // ====== 其他重要属性 ======
    static_assert(std::is_empty_v<EmptyType>);           // 空类（无数据成员）
    static_assert(std::is_final_v<FinalType>);           // final 类
    static_assert(std::is_abstract_v<std::ostream>);     // 抽象类
    static_assert(std::is_polymorphic_v<std::exception>); // 有虚函数

    static_assert(std::is_scoped_enum_v<ScopedEnum>);    // C++23: 作用域枚举
    static_assert(!std::is_scoped_enum_v enum Color { Red, Green });  // 非作用域枚举

    // ====== 实用组合 trait ======
    static_assert(std::is_trivial_v<TrivialType>);       // 平凡且标准布局
    static_assert(std::is_standard_layout_v<TrivialType>); // 标准布局类型
}
```

### 2.4 关系与转换判断

```cpp
void relationship_checks() {
    // ====== 相同性判断 ======
    static_assert(std::is_same_v<int, int>);
    static_assert(!std::is_same_v<int, const int>);
    static_assert(!std::is_same_v<int, int&>);
    static_assert(std::is_same_v<std::remove_const_t<const int>, int>);

    // ====== 类型转换可能性判断 ======
    static_assert(std::is_convertible_v<int, double>);     // int 能隐式转 double
    static_assert(!std::is_convertible_v<double, int>);    // double 不能隐式转 int（精度损失）
    static_assert(std::is_convertible_v<int*, void*>);     // 指针可以转 void*

    static_assert(std::is_constructible_v<std::string, const char*>); // 能用 const char* 构造 string
    static_assert(std::is assignable_v<int&, int>);         // int& 能接收 int 赋值

    // ====== 继承关系判断 ======
    class Base {};
    class Derived : public Base {};

    static_assert(std::is_base_of_v<Base, Derived>);
    static_assert(!std::is_base_of_v<Derived, Base>);
}
```

---

## 3. 类型修改 Traits（Transformation Traits）

### 3.1 CV 限定符修改

```cpp
#include <type_traits>

void cv_modifications() {
    // 去掉/添加 const
    static_assert(std::is_same_v<std::remove_const_t<const int>, int>);
    static_assert(std::is_same_v<std::add_const_t<int>, const int>);

    // 去掉/添加 volatile
    static_assert(std::is_same_v<std::remove_volatile_t<volatile int>, int>);
    static_assert(std::is_same_v<add_volatile_t<int>, volatile int>);

    // 同时去掉 const 和 volatile
    static_assert(std::is_same_v<std::remove_cv_t<const volatile int>, int>);
    static_assert(std::is_same_v<std::remove_cv_t<const int>, int>);

    // 同时去掉 const、volatile 和引用（C++20）
    static_assert(std::is_same_v<std::remove_cvref_t<const int&>, int>);
    static_assert(std::is_same_v<std::remove_cvref_t<volatile int&&>, int>);
}
```

### 3.2 引用修改

```cpp
void reference_modifications() {
    // 去掉引用
    static_assert(std::is_same_v<std::remove_reference_t<int&>, int>);
    static_assert(std::is_same_v<std::remove_reference_t<int&&>, int>);
    static_assert(std::is_same_v<std::remove_reference_t<int>, int>);  // 不是引用就不变

    // 添加左值引用
    static_assert(std::is_same_v<std::add_lvalue_reference_t<int>, int&>);
    static_assert(std::is_same_v<std::add_lvalue_reference_t<int&>, int&>);  // 已有引用不变

    // 添加右值引用
    static_assert(std::is_same_v<std::add_rvalue_reference_t<int>, int&&>);

    // decay：类型退化（数组→指针，函数→函数指针，去掉 cv 和引用）
    static_assert(std::is_same_v<std::decay_t<int[5]>, int*>);
    static_assert(std::is_same_v<std::decay_t<void(int)>, void(*)(int)>);
    static_assert(std::is_same_v<std::decay_t<const int&>, int>);
}
```

### 3.3 指针修改

```cpp
void pointer_modifications() {
    // 去掉指针
    static_assert(std::is_same_v<std::remove_pointer_t<int*>, int>);
    static_assert(std::is_same_v<std::remove_pointer_t<const int*>, const int>);

    // 添加指针
    static_assert(std::is_same_v<std::add_pointer_t<int>, int*>);
    static_assert(std::is_same_v<std::add_pointer_t<int&>, int*>);  // 先去掉引用再加指针
}
```

### 3.4 符号修饰修改

```cpp
void sign_modifications() {
    // 有符号 ↔ 无符号
    static_assert(std::is_same_v<std::make_signed_t<unsigned int>, int>);
    static_assert(std::is_same_v<std::make_unsigned_t<int>, unsigned int>);

    // 保持宽度不变
    static_assert(sizeof(std::make_signed_t<unsigned long>) ==
                  sizeof(unsigned long));
    static_assert(sizeof(std::make_unsigned_t<long>) ==
                  sizeof(long));
}
```

### 3.5 数组相关修改

```cpp
void array_modifications() {
    // 获取数组元素类型
    static_assert(std::is_same_v<std::remove_extent_t<int[10]>, int>);
    static_assert(std::is_same_v<std::remove_extent_t<int[3][4]>, int[4]>);  // 只去掉第一维

    // 去掉所有维度
    static_assert(std::is_same_v<std::remove_all_extents_t<int[3][4][5]>, int>);

    // 获取数组大小
    static_assert(std::extent_v<int[10]> == 10);
    static_assert(std::extent_v<int[3][4], 0> == 3);   // 第 0 维的大小
    static_assert(std::extent_v<int[3][4], 1> == 4);   // 第 1 维的大小
    static_assert(std::extent_v<int[]> == 0);           // 未知大小的数组
}
```

### 3.6 高级类型变换

```cpp
#include <type_traits>
#include <memory>

void advanced_transformations() {
    // 去掉所有包装，获取原始类型
    static_assert(std::is_same_v<std::remove_cvref_t<const int&>, int>);

    // 条件类型选择
    static_assert(std::is_same_v<
        std::conditional_t<true, int, double>,
        int
    >);
    static_assert(std::is_same_v<
        std::conditional_t<false, int, double>,
        double
    >);

    // 公共类型（类似三元运算符 ? : 的类型推断）
    static_assert(std::is_same_v<std::common_type_t<int, double>, double>);
    static_assert(std::is_same_v<std::common_type_t<int, short>, int>);

    // 底层类型（对于 enum 类型）
    enum OldEnum { A, B };
    static_assert(std::is_same_v<std::underlying_type_t<OldEnum>, unsigned int>); // 实现定义
}
```

---

## 4. 如何自定义 Type Trait

### 4.1 基本方法：模板特化

```cpp
// 自定义 trait：判断类型是否是"轻量复制"的
// （即 memcpy 就能完成复制，不需要调用构造函数）

template<typename T>
struct is_trivially_copyable_custom {
    // 默认：假设不是轻量复制的
    static constexpr bool value = false;
};

// 为已知的基本类型特化
template<>
struct is_trivially_copyable_custom<int> {
    static constexpr bool value = true;
};

template<>
struct is_trivially_copyable_custom<double> {
    static constexpr bool value = true;
};

template<>
struct is_trivially_copyable_custom<float> {
    static constexpr bool value = true;
};

// C++14 变量模板简化
template<typename T>
inline constexpr bool is_trivially_copyable_custom_v =
    is_trivially_copyable_custom<T>::value;

// 使用
static_assert(is_trivially_copyable_custom_v<int>);
static_assert(!is_trivially_copyable_custom_v<std::string>);
```

### 4.2 利用已有 trait 组合

```cpp
#include <type_traits>

// 自定义 trait：判断是否为"数值类型"（整型或浮点型，排除 bool 和 char）
template<typename T>
struct is_numeric {
private:
    // 辅助：去除 cv 限定符后的基础判断
    static constexpr bool basic_check() {
        using CleanT = std::remove_cv_t<T>;
        return std::is_integral_v<CleanT> || std::is_floating_point_v<CleanT>;
    }

    // 排除 bool 和字符类型
    static constexpr bool exclusion_check() {
        using CleanT = std::remove_cv_t<T>;
        return !std::is_same_v<CleanT, bool> &&
               !std::is_same_v<CleanT, char> &&
               !std::is_same_v<CleanT, wchar_t> &&
               !std::is_same_v<CleanT, char8_t> &&
               !std::is_same_v<CleanT, char16_t> &&
               !std::is_same_v<CleanT, char32_t>;
    }

public:
    static constexpr bool value = basic_check() && exclusion_check();
};

template<typename T>
inline constexpr bool is_numeric_v = is_numeric<T>::value;

// 测试
static_assert(is_numeric_v<int>);
static_assert(is_numeric_v<double>);
static_assert(is_numeric_v<const long>);
static_assert(!is_numeric_v<bool>);       // 排除了
static_assert(!is_numeric_v<char>);       // 排除了
static_assert(!is_numeric_v<std::string>);
```

### 4.3 SFINAE 方式的高级自定义 trait

```cpp
#include <type_traits>

// 使用 void_t 技巧检测某个类型是否存在特定的嵌套类型
template<typename, typename = void>
struct has_inner_type : std::false_type {};

template<typename T>
struct has_inner_type<T, std::void_t<typename T::inner_type>> : std::true_type {};

template<typename T>
inline constexpr bool has_inner_type_v = has_inner_type<T>::value;

// 测试
struct WithInner {
    using inner_type = int;
};
struct WithoutInner {};

static_assert(has_inner_type_v<WithInner>);
static_assert(!has_inner_type_v<WithoutInner>);


// 检测类型是否有特定名称的成员函数
template<typename, typename = void>
struct has_reserve_method : std::false_type {};

template<typename T>
struct has_reserve_method<T,
    std::void_t<decltype(std::declval<T>().reserve(std::size_t{}))>
> : std::true_type {};

template<typename T>
inline constexpr bool has_reserve_method_v = has_reserve_method<T>::value;

static_assert(has_reserve_method_v<std::vector<int>>);
static_assert(!has_reserve_method_v<std::array<int, 10>>);


// 检测类型是否能被流输出
template<typename T, typename = void>
struct is_ostreamable : std::false_type {};

template<typename T>
struct is_ostreamable<T,
    std::void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>
> : std::true_type {};

template<typename T>
inline constexpr bool is_ostreamable_v = is_ostreamable<T>::value;

static_assert(is_ostreamable_v<int>);
static_assert(is_ostreamable_v<std::string>);
static_assert(!is_ostreamable_v<struct UnprintableType {}>);
```

### 4.4 C++20 Concepts 作为现代替代

```cpp
#include <concepts>

// C++20: 用 Concept 表达同样的约束（更直观！）

// 替代 is_numeric
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

// 替代 has_reserve_method
template<typename T>
concept Reservable = requires(T t) {
    t.reserve(std::size_t{});
};

// 使用
static_assert(Numeric<int>);        // Concept 可以用于 static_assert
static_assert(Numeric<double>);

// Concept 还可以直接约束模板参数
template<Numeric T>
T add_values(T a, T b) {
    return a + b;
}

template<Reservable Container>
void ensure_capacity(Container& c, std::size_t n) {
    c.reserve(n);
}
```

---

## 5. Type Traits 实战应用

### 5.1 完美转发辅助

```cpp
#include <utility>
#include <type_traits>

// 万能包装器：保持值类别
template<typename F, typename... Args>
decltype(auto) perfect_forward(F&& f, Args&&... args) {
    // std::forward 需要知道原始的引用类型
    // type_traits 帮助我们在泛型代码中正确处理
    return std::forward<F>(f)(std::forward<Args>(args)...);
}

// 移除引用后再处理的常见模式
template<typename T>
void process(T&& arg) {
    // 获取去掉引用后的"纯净"类型
    using CleanType = std::remove_reference_t<T>;

    if constexpr (std::is_const_v<CleanType>) {
        // 处理 const 版本
    } else {
        // 处理非 const 版本
    }
}
```

### 5.2 智能选择最优的实现策略

```cpp
#include <cstring>
#include <type_traits>
#include <vector>
#include <array>

// 根据类型特性选择最快的复制方式
template<typename T>
void optimized_copy(T* dest, const T* src, size_t count) {
    if constexpr (std::is_trivially_copyable_v<T>) {
        // 轻量类型：直接 memcpy，比逐元素拷贝快得多
        std::memcpy(dest, src, count * sizeof(T));
    } else {
        // 复杂类型：必须逐个调用拷贝构造函数
        for (size_t i = 0; i < count; ++i) {
            new (&dest[i]) T(src[i]);  // placement new
        }
    }
}

// 根据容器类型选择不同的遍历策略
template<typename Container>
void efficient_traverse(Container& c) {
    if constexpr (has_reserve_method_v<Container>) {
        // 有 reserve 方法 → 可能是 vector 等连续容器
        // 可以用指针遍历
        auto* data = c.data();
        for (size_t i = 0; i < c.size(); ++i) {
            // 处理 data[i]
        }
    } else if constexpr (requires { c.begin(); c.end(); }) {
        // 有迭代器 → 用迭代器遍历
        for (auto it = c.begin(); it != c.end(); ++it) {
            // 处理 *it
        }
    }
}
```

### 5.3 类型安全的 any/variant 辅助

```cpp
#include <any>
#include <variant>
#include <type_traits>
#include <stdexcept>

// 类型安全的 any_cast 包装
template<typename T>
T safe_any_cast(const std::any& operand) {
    // 先检查类型是否匹配
    if (!operand.has_value()) {
        throw std::runtime_error("any 对象为空");
    }
    if constexpr (std::is_reference_v<T>) {
        // 引用类型：使用 any_cast 引用版本
        try {
            return std::any_cast<std::remove_reference_t<T>>(operand);
        } catch (const std::bad_any_cast& e) {
            throw std::runtime_error(std::string("类型不匹配: ") + e.what());
        }
    } else {
        // 值类型
        try {
            return std::any_cast<T>(operand);
        } catch (const std::bad_any_cast& e) {
            throw std::runtime_error(std::string("类型不匹配: ") + e.what());
        }
    }
}

// variant 的类型安全访问
template<typename Variant, typename Visitor>
auto safe_visit(Variant&& v, Visitor&& vis) -> decltype(auto) {
    if (!v.valueless_by_exception()) {
        return std::visit(std::forward<Visitor>(vis),
                          std::forward<Variant>(v));
    }
    throw std::runtime_error("variant 处于异常值状态");
}
```

### 5.4 序列化框架中的类型分发

```cpp
#include <type_traits>
#include <string>
#include <vector>
#include <map>

// 简化的序列化框架
class Serializer {
public:
    // 根据类型自动选择序列化策略
    template<typename T>
    void serialize(const T& value) {
        serialize_impl(value, std::bool_constant<
            is_serializable_v<T>
        >{});
    }

private:
    // 基本类型的快速路径
    template<typename T>
    void serialize_impl(const T& value, std::true_type) {
        if constexpr (std::is_arithmetic_v<T>) {
            write_arithmetic(value);
        } else if constexpr (std::is_same_v<T, std::string>) {
            write_string(value);
        } else {
            // 自定义可序列化类型
            value.serialize(*this);  // 假设有 serialize 方法
        }
    }

    template<typename T>
    void serialize_impl(const T&, std::false_type) {
        static_assert(always_false<T>,
            "类型不支持序列化");
    }

    template<typename T>
    static constexpr bool always_false = false;

    void write_arithmetic(auto value) { /* 实现 */ }
    void write_string(const std::string& s) { /* 实现 */ }
};
```

---

## 6. Type Traits 与 Concepts 的对比

### 6.1 各自的定位

```cpp
#include <type_traits>
#include <concepts>

// ========== Type Traits：编译期"问询"工具 ==========
// 回答"是什么"的问题
template<typename T>
void trait_based_api(T&& arg) {
    if constexpr (std::is_integral_v<std::remove_reference_t<T>>) {
        // 整型处理
    } else if constexpr (std::is_floating_point_v<std::remove_reference_t<T>>) {
        // 浮点处理
    }
    // 运行时/if-else 式的逻辑分支
}

// ========== Concepts：编译期"约束"工具 ==========
// 定义"必须是什么"的要求
template<std::integral T>
void concept_based_api(T arg) {
    // 这里 T 一定是整型，不需要再判断
    // 如果传入非整型，编译直接失败
}

// ========== 两者的关系 ==========
// Concepts 底层很多都是基于 type_traits 实现的
// std::integral concept 的本质大约是：
/*
template<typename T>
concept integral = is_integral_v<T> && (!is_bool_v<T>) && ...;
*/
```

### 6.2 适用场景对比

| 场景 | 推荐 | 理由 |
|------|------|------|
| 模板参数约束 | **Concepts** | 语义清晰，错误信息友好 |
| if constexpr 分支 | **Type Traits** | 需要具体的布尔值做条件判断 |
| SFINAE 重载决议 | **Type Traits** | enable_if 需要 bool 值 |
| 类型变换/提取 | **Type Traits** | Concepts 不做类型变换 |
| 定义新的抽象需求 | **Concepts** | 可以组合多个约束 |
| 库内部实现细节 | **Type Traits** | 更底层、更灵活 |

### 6.3 协同使用

```cpp
#include <concepts>
#include <type_traits>

// 最佳实践：Concepts 做门卫，Traits 做内勤

// 门卫：Concepts 约束模板参数
template<typename T>
    requires std::copyable<T> && std::movable<T>
class SmartPointer {
public:
    // 内勤：Type Traits 在实现中做精细的类型处理
    template<typename U>
        requires std::convertible_to<U*, T*>
    SmartPointer(SmartPtr<U>&& other) {
        // 使用 type_traits 进行类型转换
        ptr_ = static_cast<T*>(other.release());
    }

private:
    T* ptr_;
    using element_type = std::remove_pointer_t<decltype(ptr_)>;
};
```

---

## 7. 常用 Type Traits 速查

### 判断类（返回 bool）

| Trait | 作用 | 示例 |
|-------|------|------|
| `is_integral_v<T>` | 是否整型 | `int`, `long`, `bool` |
| `is_floating_point_v<T>` | 是否浮点 | `float`, `double` |
| `is_array_v<T>` | 是否数组 | `int[5]`, `int[]` |
| `is_pointer_v<T>` | 是否原始指针 | `int*`, `const char*` |
| `is_reference_v<T>` | 是否引用 | `int&`, `int&&` |
| `is_const_v<T>` | 是否 const | `const int` |
| `is_empty_v<T>` | 是否空类 | `struct {}` |
| `is_abstract_v<T>` | 是否抽象类 | 含纯虚函数的类 |
| `is_polymorphic_v<T>` | 是否有多态 | 含虚函数的类 |
| `is_trivially_copyable_v<T>` | 是否可 trivial 复制 | 基本类型、简单 POD |
| `is_same_v<T, U>` | 类型是否相同 | `is_same_v<int, int>` |
| `is_base_of_v<Base, Derived>` | 是否有继承关系 | 类层次检查 |
| `is_convertible_v<From, To>` | 能否隐式转换 | 类型转换检查 |

### 转换类（返回类型）

| Trait | 作用 | 示例 |
|-------|------|------|
| `remove_cv_t<T>` | 去掉 const/volatile | `const int` → `int` |
| `remove_reference_t<T>` | 去掉引用 | `int&` → `int` |
| `remove_cvref_t<T>` | 去掉 cv 和引用 | `const int&` → `int` |
| `remove_pointer_t<T>` | 去掉指针 | `int*` → `int` |
| `decay_t<T>` | 类型退化 | `int[5]` → `int*` |
| `conditional_t<B, T, F>` | 条件选择 | `true ? int : double` |
| `common_type_t<Ts...>` | 公共类型 | `int`, `double` → `double` |
| `underlying_type_t<Enum>` | 枚举底层类型 | `enum class E : int` → `int` |
| `invoke_result_t<F, Args...>` | 调用结果类型 | 推断返回类型 |

---

## 8. 总结

Type Traits 是 C++ 模板元编程的基石，它的核心价值在于：

1. **统一的类型 introspection 接口**：不再需要为每种类型手动写特化
2. **编译期类型变换流水线**：像操作值一样操作类型
3. **与 Concepts 形成互补**：Traits 负责"查询和变换"，Concepts 负责"约束和表达"
4. **组合性极强**：小 trait 可以自由组合成复杂的类型判断逻辑

掌握 Type Traits 是理解 STL 源码、编写高质量泛型代码的必经之路。每一个现代 C++ 程序员都应该熟悉至少最常用的 20-30 个 trait。