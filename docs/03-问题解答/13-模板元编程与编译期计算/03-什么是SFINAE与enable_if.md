# 什么是 SFINAE 与 enable_if
> 📖 相关章节：[模板进阶](../../02-CPP/11-模板进阶.md)、[编译期计算基础](../../07-模板元编程与编译期计算/00-编译期计算基础.md)、[Type Traits](../../07-模板元编程与编译期计算/01-Type-Traits与类型操作.md)

## 核心要义

**SFINAE（替换失败并非错误）是 C++ 模板的"沉默过滤机制"——当模板参数替换导致无效代码时，编译器不会报错而是默默丢弃那个候选重载；enable_if 是利用这一机制实现编译期条件加载的利器，配合 void_t 技巧几乎可以实现任意粒度的类型检测与重载分发。**

---

## 1. SFINAE 的原理

### 1.1 名字的由来

```
SFINAE = Substitution Failure Is Not An Error
       = 替换失败并非错误
       = 模板参数推导时产生的"类型不匹配"不算编译错误，
         只是把这个重载候选从候选列表中移除
```

### 1.2 直观理解：从重载决议说起

```cpp
#include <iostream>

// 普通函数重载：编译器根据参数类型选择最匹配的版本
void foo(int) {
    std::cout << "foo(int)\n";
}

void foo(double) {
    std::cout << "foo(double)\n";
}

void foo(const char*) {
    std::cout << "foo(const char*)\n";
}

int main() {
    foo(42);          // 调用 foo(int)
    foo(3.14);        // 调用 foo(double)
    foo("hello");     // 调用 foo(const char*)
}
```

### 1.3 当普通重载不够用时——模板登场

```cpp
// 问题：我们想对所有"数字类型"做一个版本，对其他类型做另一个版本
// 但 C++ 没有内置"数字类型"的概念...

// 尝试 1：用普通重载？做不到，类型无穷多
// void foo(short);
// void foo(long);
// void foo(unsigned int);
// void foo(long long);
// ... 写不完

// 尝试 2：用模板？
template<typename T>
void foo(T arg) {
    std::cout << "通用版本\n";
}

// 但如何为数字类型提供特殊版本？这就需要 SFINAE 了
```

### 1.4 SFINAE 的触发机制详解

```cpp
#include <type_traits>
#include <iostream>
#include <string>

// SFINAE 发生的精确时刻：模板参数推导过程中的"直接上下文"

// ========== 示例：只有当 T 是整型时才参与重载决议 ==========
template<typename T>
typename std::enable_if<std::is_integral_v<T>>::type
foo(T arg) {
    std::cout << "整型版本: " << arg << "\n";
}

// 当 T = int 时：
//   std::enable_if<true>::type 存在 → void → 有效候选 ✓
// 当 T = std::string 时：
//   std::enable_if<false>::type 不存在 → 替换失败 → 静默丢弃 ✗（不报错！）
//   这就是 SFINAE！

template<typename T>
typename std::enable_if<!std::is_integral_v<T>>::type
foo(T arg) {
    std::cout << "非整型版本\n";
}

void test_sfinae() {
    foo(42);              // T=int, is_integral=true → 第一个版本
    foo(3.14);            // T=double, is_integral=false → 第二个版本
    foo(std::string("hi")); // T=string, is_integral=false → 第二个版本
}
```

### 1.5 什么触发 SFINAE，什么导致真正的编译错误？

```cpp
#include <type_traits>

// ========== 会触发 SFINAE 的情况（在直接上下文中）==========

// 1. 访问不存在的嵌套类型
template<typename T>
void sfinae_case_1(typename T::inner_type* = nullptr) {
    // 如果 T 没有 inner_type，此处替换失败 → SFINAE
}

// 2. enable_if 条件不满足
template<typename T>
std::enable_if_t<(sizeof(T) > 4)>
sfinae_case_2(T) {}

// 3. 表达式中类型不匹配（在 decltype/sizeof/typeof 等上下文中）
template<typename T>
auto sfinae_case_3(T t) -> decltype(t.foo()) {
    // 如果 T 没有 foo() 方法，此处替换失败 → SFINAE
}

// 4. 数组边界为负
template<typename T>
void sfinae_case_4(T (&arr)[T::negative_value]) {}
// 如果 T::negative_value 是负数，数组大小无效 → SFINAE


// ========== 会导致真正编译错误的情况（不在直接上下文中）==========

template<typename T>
void hard_error_case(T) {
    // 函数体中的错误不属于"直接上下文"！
    typename T::this_type_does_not_exist x;  // ❌ 硬错误！不管 T 是什么都会报错
                                            // 因为这不是在"推导模板参数"时发生的
}

// 另一个硬错误示例
template<typename T>
void another_hard_error(T) {
    static_assert(sizeof(T) == 0, "永远失败的断言");  // ❌ 硬错误！
}

// 关键区别：
// SFINAE 只在模板参数**推导过程**的直接上下文中发生
// 函数体内的实例化错误永远是硬错误
```

---

## 2. enable_if 的多种写法

### 2.1 传统写法：返回类型

```cpp
#include <type_traits>
#include <iostream>

// 写法 1：返回类型位置（最经典）
template<typename T>
typename std::enable_if<std::is_integral_v<T>, void>::type
only_for_integers_1(T value) {
    std::cout << "整型值: " << value << "\n";
}

// C++14 简化：enable_if_t
template<typename T>
std::enable_if_t<std::is_integral_v<T>>
only_for_integers_2(T value) {
    std::cout << "整型值(C++14): " << value << "\n";
}

void demo_return_type() {
    only_for_integers_1(42);        // ✅ int 是整型
    only_for_integers_2(100L);      // ✅ long 也是整型
    // only_for_integers_1(3.14);   // ❌ double 不是整型，没有匹配的重载
                                   // （如果没有其他重载的话）
}
```

### 2.2 参数列表写法：默认参数

```cpp
// 写法 2：额外的默认模板参数（匿名参数）
template<typename T,
         typename = typename std::enable_if<std::is_integral_v<T>>::type>
void only_for_integers_3(T value) {
    std::cout << "整型值(参数法): " << value << "\n";
}

// 写法 3：具名默认参数（更易读）
template<typename T,
         typename std::enable_if<std::is_integral_v<T>, int>::type = 0>
void only_for_integers_4(T value) {
    std::cout << "整型值(具名参数): " <<value << "\n";
}

// 写法 4：C++17 的 inline 默认参数风格
template<typename T,
         std::enable_if_t<std::is_integral_v<T>, bool> = true>
void only_for_integers_5(T value) {
    std::cout << "整型值(C++17风格): " << value << "\n";
}
```

### 2.3 模板参数写法：非类型模板参数

```cpp
// 写法 5：使用 bool 非类型模板参数
template<typename T, std::enable_if_t<std::is_integral_v<T>, bool> = true>
void only_for_integers_6(T value) {
    std::cout << "整型值(bool模板参数): " << value << "\n";
}

// 写法 6：更简洁的 bool 模板参数
template<typename T, bool = std::is_integral_v<T>>
void only_for_integers_7(T value) {
    std::cout << "整型值(简洁bool): " << value << "\n";
}
```

### 2.4 C++20 的概念替代（推荐新代码使用）

```cpp
#include <concepts>

// C++20: 用 concept 替代 enable_if，语义清晰得多
template<std::integral T>
void only_for_integers_modern(T value) {
    std::cout << "整型值(concept): " << value << "\n";
}

// requires 子句形式
template<typename T>
    requires std::integral<T>
void only_for_integers_requires(T value) {
    std::cout << "整型值(requires): " << value << "\n";
}
```

### 2.5 各种写法的优缺点对比

```
┌─────────────────────┬──────────┬──────────┬────────────────────────┐
│ 写法                │ 可读性   │ 干扰签名  │ 备注                   │
├─────────────────────┼──────────┼──────────┼────────────────────────┤
│ 返回类型 enable_if  │ ★★☆☆☆   │ ★★★★★   │ 最经典，但污染返回类型   │
│ 默认模板参数(匿名)  │ ★★★☆☆   │ ★★★★☆   │ 隐形参数，容易困惑      │
│ 默认模板参数(具名)  │ ★★★★☆   │ ★★★☆☆   │ 较清晰但仍改变参数列表   │
│ bool 模板参数       │ ★★★★☆   │ ★★★☆☆   │ 简洁但语义不够明确      │
│ C++20 Concept       │ ★★★★★   │ ★★★★★   │ 推荐！语义最清晰        │
└─────────────────────┴──────────┴──────────┴────────────────────────┘
```

---

## 3. void_t 技巧：检测任意类型特性

### 3.1 void_t 的原理

```cpp
#include <type_traits>

// void_t 的定义（C++17 已加入标准库）
template<typename... Ts>
struct make_void { using type = void };

template<typename... Ts>
using void_t = typename make_void<Ts...>::type;

// 核心原理：无论传入什么类型，void_t<X> 总是 void
// 但关键是：如果 X 的求值过程中出现替换失败 → SFINAE！
static_assert(std::is_same_v<void_t<int>, void>);
static_assert(std::is_same_v<void_t<int, double, char>, void>);
```

### 3.2 用 void_t 检测嵌套类型是否存在

```cpp
#include <type_traits>

// 检测类型 T 是否有名为 value_type 的嵌套类型
template<typename, typename = void>
struct has_value_type : std::false_type {};

// 特化版本：如果 T::value_type 存在，则匹配此版本
template<typename T>
struct has_value_type<T, void_t<typename T::value_type>> : std::true_type {};

template<typename T>
inline constexpr bool has_value_type_v = has_value_type<T>::value;

// 测试
struct WithValueType {
    using value_type = int;
};

struct WithoutValueType {
    int data;  // 没有 value_type
};

static_assert(has_value_type_v<WithValueType>);        // ✅
static_assert(!has_value_type_v<WithoutValueType>);    // ✅
static_assert(has_value_type_v<std::vector<int>>);     // ✅ vector 有 value_type
```

### 3.3 用 void_t 检测成员函数是否存在

```cpp
#include <type_traits>

// 检测类型 T 是否有 resize 成员函数
template<typename, typename = void>
struct has_resize : std::false_type {};

template<typename T>
struct has_resize<T,
    void_t<decltype(std::declval<T&>().resize(std::size_t{}))>
> : std::true_type {};

template<typename T>
inline constexpr bool has_resize_v = has_resize<T>::value;

// 检测类型 T 是否有 push_back 成员函数
template<typename, typename = void>
struct has_push_back : std::false_type {};

template<typename T>
struct has_push_back<T,
    void_t<decltype(std::declval<T&>().push_back(std::declval<typename T::value_type>()))>
> : std::true_type {};

template<typename T>
inline constexpr bool has_push_back_v = has_push_back<T>::value;

// 测试
#include <vector>
#include <array>
#include <list>

static_assert(has_resize_v<std::vector<int>>);     // ✅ vector 有 resize
static_assert(!has_resize_v<std::array<int, 5>>);  // ✅ array 没有 resize
static_assert(has_push_back_v<std::vector<int>>);  // ✅ vector 有 push_back
static_assert(has_push_back_v<std::list<int>>);    // ✅ list 有 push_back
static_assert(!has_push_back_v<std::array<int, 5>>); // ✅ array 没有 push_back
```

### 3.4 用 void_t 检测运算符是否存在

```cpp
#include <type_traits>

// 检测是否支持 operator<
template<typename, typename = void>
struct has_less_than : std::false_type {};

template<typename T>
struct has_less_than<T,
    void_t<decltype(std::declval<T>() < std::declval<T>())>
> : std::true_type {};

template<typename T>
inline constexpr bool has_less_than_v = has_less_than<T>::value;

// 检测是否支持 operator<< (流输出)
template<typename, typename = void>
struct is_ostreamable : std::false_type {};

template<typename T>
struct is_ostreamable<T,
    void_t<decltype(std::declval<std::ostream&>() << std::declval<T>())>
> : std::true_type {};

template<typename T>
inline constexpr bool is_ostreamable_v = is_ostreamable<T>::value;

// 检测是否支持 operator++
template<typename, typename = void>
struct has_increment : std::false_type {};

template<typename T>
struct has_increment<T,
    void_t<decltype(++std::declval<T&>())>
> : std::true_type {};

template<typename T>
inline constexpr bool has_increment_v = has_increment<T>::value;

// 测试
struct Comparable { int v; };
bool operator<(const Comparable& a, const Comparable& b) { return a.v < b.v; }

struct NotComparable { int v; };  // 没有 operator<

static_assert(has_less_than_v<Comparable>);       // ✅
static_assert(!has_less_than_v<NotComparable>);   // ✅
static_assert(is_ostreamable_v<int>);             // ✅ int 支持 <<
static_assert(has_increment_v<int>);             // ✅ int 支持 ++
```

---

## 4. SFINAE 实战：编译期分发

### 4.1 根据类型特性选择不同实现

```cpp
#include <type_traits>
#include <cstring>
#include <iostream>
#include <vector>

// 根据类型是否可 trivially copy 来选择最优的序列化方式
template<typename T>
std::enable_if_t<std::is_trivially_copyable_v<T>>
serialize(const T& data, std::vector<char>& buffer) {
    // 轻量类型：直接 memcpy
    const char* bytes = reinterpret_cast<const char*>(&data);
    buffer.insert(buffer.end(), bytes, bytes + sizeof(T));
    std::cout << "memcpy 序列化 (" << sizeof(T) << " 字节)\n";
}

template<typename T>
std::enable_if_t<!std::is_trivially_copyable_v<T>>
serialize(const T& data, std::vector<char>& buffer) {
    // 复杂类型：逐字段序列化（示例中简化处理）
    data.serialize_to(buffer);  // 假设 T 有此方法
    std::cout << "逐字段序列化\n";
}

// 使用
struct PODData { int a; double b; };  // POD 类型
struct ComplexData {
    std::string name;
    void serialize_to(std::vector<char>& buf) const {
        buf.insert(buf.end(), name.begin(), name.end());
    }
};

void demo_dispatch() {
    std::vector<char> buffer;

    PODData pod{42, 3.14};
    serialize(pod, buffer);  // → memcpy 版本

    ComplexData complex{"hello"};
    serialize(complex, buffer);  // → 逐字段版本
}
```

### 4.2 容器类型的智能处理

```cpp
#include <type_traits>
#include <vector>
#include <array>
#include <iostream>
#include <string>

// 检测是否是容器类型
template<typename, typename = void>
struct is_container : std::false_type {};

template<typename T>
struct is_container<T, void_t<
    typename T::value_type,
    typename T::size_type,
    typename T::iterator,
    typename T::const_iterator,
    decltype(std::declval<T>().size()),
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end())
>> : std::true_type {};

template<typename T>
inline constexpr bool is_container_v = is_container<T>::value;

// 根据是否是容器选择不同处理
template<typename T>
std::enable_if_t<is_container_v<T>>
process_data(const T& container) {
    std::cout << "容器类型，包含 " << container.size() << " 个元素\n";
    for (const auto& item : container) {
        // 处理每个元素...
    }
}

template<typename T>
std::enable_if_t<!is_container_v<T>>
process_data(const T& value) {
    std::cout << "标量值: " << value << "\n";
}

void demo_container_dispatch() {
    process_data(std::vector<int>{1, 2, 3});   // → 容器版本
    process_data(std::array<double, 4>{1,2,3,4}); // → 容器版本
    process_data(42);                             // → 标量版本
    process_data(std::string("hello"));          // → 容器版本（string 也是容器）
}
```

### 4.3 智能指针与裸指针的统一处理

```cpp
#include <type_traits>
#include <memory>
#include <iostream>

// 检测是否是智能指针
template<typename T>
struct is_smart_pointer : std::false_type {};

template<typename T>
struct is_smart_pointer<std::unique_ptr<T>> : std::true_type {};

template<typename T>
struct is_smart_pointer<std::shared_ptr<T>> : std::true_type {};

template<typename T>
inline constexpr bool is_smart_pointer_v = is_smart_pointer<T>::value;

// 统一的指针解引用
template<typename T>
std::enable_if_t<std::is_pointer_v<T>>
dereference_and_print(T ptr) {
    if (ptr) {
        std::cout << "裸指针指向: " << *ptr << "\n";
    } else {
        std::cout << "裸指针为空\n";
    }
}

template<typename T>
std::enable_if_t<is_smart_pointer_v<T>>
dereference_and_print(const T& ptr) {
    if (ptr) {
        std::cout << "智能指针指向: " << *ptr << "\n";
    } else {
        std::cout << "智能指针为空\n";
    }
}

void demo_pointer_dispatch() {
    int value = 42;
    int* raw_ptr = &value;
    auto smart_ptr = std::make_unique<int>(100);

    dereference_and_print(raw_ptr);     // → 裸指针版本
    dereference_and_print(smart_ptr);   // → 智能指针版本
}
```

---

## 5. SFINAE 的局限性与替代方案

### 5.1 局限性一：错误信息难以阅读

```cpp
// 当 SFINAE 过滤掉所有候选时，错误信息通常很糟糕
template<typename T>
std::enable_if_t<std::is_integral_v<T>>
very_restricted_func(T) {}

// very_restricted_func(3.14);
// 错误信息大概是这样的：
// error: no matching function for call to 'very_restricted_func'
// note: candidate: [candidate details with pages of template instantiation trace]
//
// 根本看不出是因为"参数必须是整型"！
```

### 5.2 局限性二：重载冲突

```cpp
// 多个 SFINAE 条件可能同时满足或不满足，导致歧义
template<typename T,
         std::enable_if_t<std::is_integral_v<T>, int> = 0>
void ambiguous(T) {}

template<typename T,
         std::enable_if_t<!std::is_floating_point_v<T>, int> = 0>
void ambiguous(T) {}

// ambiguous(42);
// T=int 时两个条件都满足！→ 歧义错误
```

### 5.3 局限性三：代码冗余和可读性差

```cpp
// SFINAE 代码经常看起来很"嘈杂"
template<
    typename T,
    typename std::enable_if<
        std::is_copy_constructible_v<T> &&
        std::is_move_constructible_v<T> &&
        std::is_copy_assignable_v<T> &&
        std::is_move_assignable_v<T> &&
        !std::is_array_v<T> &&
        !std::is_const_v<T>
    , int>::type = 0
>
void noisy_function(T&& value) {
    // 实际逻辑只有几行，但签名占了十几行...
}
```

### 5.4 现代替代方案：if constexpr（C++17）

```cpp
#include <type_traits>
#include <iostream>

// 用 if constexpr 替代 SFINAE 重载（更简洁！）
template<typename T>
void unified_process(T&& value) {
    // 所有逻辑在一个函数体内，编译器会选择性地编译对应分支
    if constexpr (std::is_integral_v<std::decay_t<T>>) {
        std::cout << "整型处理: " << value << "\n";
    } else if constexpr (std::is_floating_point_v<std::decay_t<T>>) {
        std::cout << "浮点处理: " << value << "\n";
    } else if constexpr (std::is_same_v<std::decay_t<T>, std::string>) {
        std::cout << "字符串处理: " << value << " (长度=" << value.size() << ")\n";
    } else {
        // 如果都不匹配，仍会编译此分支（不同于 SFINAE）
        // 可以用 static_assert 提供更好的错误信息
        static_assert(!sizeof(T*), "不支持的类型");  // 总是失败
    }
}

// if constexpr vs SFINAE 的选择指南：
// - 需要不同函数签名（返回类型不同等）→ SFINAE
// - 只是内部实现不同 → if constexpr
// - 新项目优先考虑 if constexpr + Concepts
```

### 5.5 现代替代方案：Concepts（C++20）

```cpp
#include <concepts>
#include <type_traits>
#include <iostream>

// Concepts 提供了语义级别的约束
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept Container = requires(T t) {
    t.begin();
    t.end();
    t.size();
    typename T::value_type;
};

// Concepts 约束的模板——错误信息极其清晰！
template<Numeric T>
T add_numbers(T a, T b) {
    return a + b;
}

template<Container C>
void print_container(const C& c) {
    std::cout << "[";
    for (auto it = c.begin(); it != c.end(); ++it) {
        if (it != c.begin()) std::cout << ", ";
        std::cout << *it;
    }
    std::cout << "]\n";
}

// Concepts + requires 表达式的强大组合
template<typename T>
    requires (sizeof(T) == 4) && std::is_trivially_copyable_v<T>
void process_32bit_pod(T value) {
    // 精确约束：恰好 4 字节且可 trivially copy
}

// 当 Constraints 不满足时：
// add_numbers("hello");
// 错误信息：
// error: cannot call function 'add_numbers<std::__1::basic_string<char>>'
// note: because 'std::__1::basic_string<char>' does not satisfy 'Numeric'
// note: because 'std::is_integral_v<std::__1::basic_string<char>'> evaluated to false
// note: and 'std::is_floating_point_v<std::__1::basic_string<char>'> evaluated to false
//
// 清楚地告诉你为什么不满足！
```

### 5.6 迁移路线图

```
C++11/14 项目：使用 enable_if + void_t（成熟稳定）
     ↓ 升级
C++17 项目：优先使用 if constexpr，复杂场景保留 SFINAE
     ↓ 升级
C++20+ 项目：优先使用 Concepts，SFINAE 仅用于库内部兼容性
     ↓
未来：Concepts 可能完全取代大多数 SFINAE 用法
      但 SFINAE 作为底层机制仍然重要（Concepts 本身就可能基于 SFINAE 实现）
```

---

## 6. SFINAE 的进阶技巧

### 6.1 标签分发（Tag Dispatching）

```cpp
#include <type_traits>
#include <iterator>
#include <iostream>

// 使用标签类型结合 SFINAE 实现高效的算法选择
struct random_access_tag {};
struct bidirectional_tag {};
struct forward_tag {};
struct input_tag {};

// 根据迭代器类别选择标签
template<typename Iterator>
constexpr auto iterator_category_tag() {
    using category = typename std::iterator_traits<Iterator>::iterator_category;
    if constexpr (std::is_base_of_v<std::random_access_iterator_tag, category>) {
        return random_access_tag{};
    } else if constexpr (std::is_base_of_v<std::bidirectional_iterator_tag, category>) {
        return bidirectional_tag{};
    } else {
        return forward_tag{};
    }
}

// 不同标签的不同实现
template<typename Iterator>
void advance_impl(Iterator& it, int n, random_access_tag) {
    // 随机访问迭代器：O(1) 直接跳转
    it += n;
    std::cout << "随机访问: += " << n << "\n";
}

template<typename Iterator>
void advance_impl(Iterator& it, int n, bidirectional_tag) {
    // 双向迭代器：根据正负决定方向
    if (n >= 0) {
        for (int i = 0; i < n; ++i) ++it;
    } else {
        for (int i = 0; i > n; --i) --it;
    }
    std::cout << "双向: 逐步移动 " << n << "\n";
}

template<typename Iterator>
void advance_impl(Iterator& it, int n, forward_tag) {
    // 前向迭代器：只能正向
    for (int i = 0; i < n; ++i) ++it;
    std::cout << "前向: 逐步前进 " << n << "\n";
}

// 统一入口
template<typename Iterator>
void my_advance(Iterator& it, int n) {
    my_advance_impl(it, n, iterator_category_tag<Iterator>());
}
```

### 6.2 逗号运算符技巧（遗留技巧，了解即可）

```cpp
// 旧时代的一种 SFINAE 技巧（在现代 C++ 中不推荐）
template<typename T>
auto check_callable(int) -> decltype(
    std::declval<T>()(),  // 尝试调用 T()
    std::true_type{}
);

template<typename T>
auto check_callable(...) -> std::false_type;

// 这种写法利用逗号运算符返回最后一个值
// 以及 C 变参函数的最低优先级来实现 fallback
// 现代替代：void_t 或 Concepts
```

### 6.3 检测表达式合法性

```cpp
#include <type_traits>

// 通用表达式检测器
#define IS_VALID_EXPRESSION(expr) \
    [](auto&& obj) -> decltype(obj.expr, std::true_type{}) { \
        return {}; \
    }([](auto&&) -> std::false_type { return {}; })(0)

// 使用（C++17 lambda）
template<typename T>
constexpr bool has_begin_end_v = decltype([](auto* p) ->
    decltype(std::declval<T>().begin(), std::declval<T>().end(),
             std::true_type{}) { return {}; }
    (nullptr))::value;
```

---

## 7. 总结

SFINAE 是 C++ 模板编程的核心机制之一，理解它能帮你：

1. **读懂 STL 源码**：STL 中大量使用 SFINAE 进行条件编译
2. **编写灵活的泛型库**：根据类型特性自动选择最优实现
3. **理解 C++ 的编译模型**：模板实例化和重载决议的工作方式

**技术演进路线**：
- **C++11/14**: `enable_if` + 手动 `void_t` → 功能强大但语法繁琐
- **C++17**: `if constexpr` + 标准库 `void_t` → 大幅简化常见场景
- **C++20**: **Concepts** → 语义清晰、错误友好，推荐新代码首选

**黄金法则**：如果你在读别人的代码遇到 SFINAE，耐心分析 `enable_if` 的条件和 `void_t` 的检测目标；如果你在写新代码，优先考虑 `if constexpr` 和 `Concepts`，把 SFINAE 留给真正需要的场景。