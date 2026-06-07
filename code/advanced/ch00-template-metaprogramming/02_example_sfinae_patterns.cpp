/** @file 02_example_sfinae_patterns.cpp
 *  @brief SFINAE模式：函数重载SFINAE、偏特化SFINAE、void_t模式、enable_if惯用法
 *  @description 对应文档: 07-模板元编程与编译期计算 / SFINAE与替换失败
 */

#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <iterator>

// ============================================================
// 1. SFINAE 基本概念
// ============================================================

// SFINAE = Substitution Failure Is Not An Error
// 当模板参数替换失败时，不会产生编译错误，而是将该候选从重载集中移除
// 这是模板元编程最核心的机制之一

void demo_sfinae_concept() {
    std::cout << "=== SFINAE 基本概念 ===\n";
    std::cout << "SFINAE = Substitution Failure Is Not An Error\n";
    std::cout << "  模板参数替换失败时，不产生编译错误\n";
    std::cout << "  而是将该候选从重载集中移除\n";
    std::cout << "  这使得我们可以根据类型特征选择不同实现\n\n";
}

// ============================================================
// 2. 函数重载 SFINAE：enable_if
// ============================================================

// 方式1：enable_if 在默认模板参数中（注意：此方式有歧义风险，仅作演示）
// 以下两个模板签名相同，某些编译器会报重定义错误
// 推荐使用方式2（非类型模板参数）

// 方式2：enable_if 在非类型模板参数中（推荐）
template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
std::string classify(T) {
    return "整型类型";
}

template<typename T, std::enable_if_t<std::is_floating_point_v<T>, int> = 0>
std::string classify(T) {
    return "浮点类型";
}

template<typename T, std::enable_if_t<std::is_same_v<T, std::string>, int> = 0>
std::string classify(T) {
    return "字符串类型";
}

// 方式3：enable_if 在返回类型中
template<typename T>
auto process(T value) -> std::enable_if_t<std::is_integral_v<T>, int> {
    std::cout << "  整型处理: " << value << "\n";
    return value * 2;
}

template<typename T>
auto process(T value) -> std::enable_if_t<std::is_floating_point_v<T>, double> {
    std::cout << "  浮点处理: " << value << "\n";
    return value * 1.5;
}

void demo_enable_if_overload() {
    std::cout << "=== enable_if 函数重载 ===\n";

    std::cout << "classify:\n";
    std::cout << "  42:      " << classify(42) << "\n";
    std::cout << "  3.14:    " << classify(3.14) << "\n";
    std::cout << "  \"hello\": " << classify(std::string("hello")) << "\n";

    std::cout << "\nprocess:\n";
    process(10);
    process(2.5);

    std::cout << "\nenable_if 三种写法:\n";
    std::cout << "  1. 默认模板参数: template<typename T, typename = enable_if_t<cond>>\n";
    std::cout << "  2. 非类型模板参数: template<typename T, enable_if_t<cond, int> = 0>\n";
    std::cout << "  3. 返回类型: auto f() -> enable_if_t<cond, Ret>\n";
    std::cout << "  推荐方式2，避免重载歧义\n";

    std::cout << "\n";
}

// ============================================================
// 3. 偏特化 SFINAE
// ============================================================

// 通用模板：默认实现
template<typename T, typename = void>
struct Serializer {
    static std::string serialize(const T&) {
        return "<non-serializable>";
    }
};

// 偏特化：对有 serialize() 方法的类型
template<typename T>
struct Serializer<T, std::void_t<decltype(std::declval<const T&>().serialize())>> {
    static std::string serialize(const T& obj) {
        return obj.serialize();
    }
};

// 偏特化：对算术类型
template<typename T>
struct Serializer<T, std::enable_if_t<std::is_arithmetic_v<T>>> {
    static std::string serialize(const T& value) {
        return std::to_string(value);
    }
};

// 偏特化：对容器类型
template<typename T>
struct Serializer<T, std::void_t<
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end()),
    typename T::value_type
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

struct Point {
    int x, y;
    std::string serialize() const {
        return "Point(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
};

struct Opaque {};

void demo_partial_specialization_sfinae() {
    std::cout << "=== 偏特化 SFINAE ===\n";

    Point p{3, 4};
    std::cout << "Point: " << Serializer<Point>::serialize(p) << "\n";
    std::cout << "int:   " << Serializer<int>::serialize(42) << "\n";
    std::cout << "Opaque:" << Serializer<Opaque>::serialize(Opaque{}) << "\n";

    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::cout << "vector:" << Serializer<std::vector<int>>::serialize(vec) << "\n";

    std::cout << "\n";
}

// ============================================================
// 4. void_t 模式（C++17）
// ============================================================

// void_t 是 SFINAE 的利器，定义极其简单：
// template<typename...> using void_t = void;
// 但它使得检测表达式是否合法变得非常优雅

// 检测是否可调用 f(args...)
template<typename T, typename = void>
struct is_callable : std::false_type {};

template<typename T>
struct is_callable<T, std::void_t<decltype(std::declval<T>()())>> : std::true_type {};

template<typename T>
inline constexpr bool is_callable_v = is_callable<T>::value;

// 检测是否有 operator==
template<typename T, typename = void>
struct is_equality_comparable : std::false_type {};

template<typename T>
struct is_equality_comparable<T, std::void_t<
    decltype(std::declval<const T&>() == std::declval<const T&>())
>> : std::true_type {};

template<typename T>
inline constexpr bool is_equality_comparable_v = is_equality_comparable<T>::value;

// 检测是否是迭代器
template<typename T, typename = void>
struct is_iterator : std::false_type {};

template<typename T>
struct is_iterator<T, std::void_t<
    typename std::iterator_traits<T>::iterator_category
>> : std::true_type {};

template<typename T>
inline constexpr bool is_iterator_v = is_iterator<T>::value;

void demo_void_t_pattern() {
    std::cout << "=== void_t 模式 ===\n";

    // is_callable
    auto lambda = []() { return 42; };
    std::cout << "is_callable:\n";
    std::cout << "  lambda: " << is_callable_v<decltype(lambda)> << "\n";
    std::cout << "  int:    " << is_callable_v<int> << "\n";

    // is_equality_comparable
    std::cout << "\nis_equality_comparable:\n";
    std::cout << "  int:    " << is_equality_comparable_v<int> << "\n";
    std::cout << "  string: " << is_equality_comparable_v<std::string> << "\n";

    // is_iterator
    std::cout << "\nis_iterator:\n";
    std::cout << "  int*:         " << is_iterator_v<int*> << "\n";
    std::cout << "  vector::iter: " << is_iterator_v<std::vector<int>::iterator> << "\n";
    std::cout << "  int:          " << is_iterator_v<int> << "\n";

    std::cout << "\nvoid_t 工作原理:\n";
    std::cout << "  1. 主模板: struct Trait<T, void> : false_type {};\n";
    std::cout << "  2. 特化:   struct Trait<T, void_t<expr>> : true_type {};\n";
    std::cout << "  3. 如果 expr 合法, void_t = void, 匹配特化 (true)\n";
    std::cout << "  4. 如果 expr 非法, 替换失败, SFINAE 回到主模板 (false)\n";

    std::cout << "\n";
}

// ============================================================
// 5. SFINAE 友好的 trait 设计模式
// ============================================================

// 好的 trait 应该是 SFINAE 友好的：
// 当条件不满足时，不应该导致硬编译错误，而是返回 false_type

// 反例：不 SFINAE 友好的 trait
template<typename T>
struct bad_has_size {
    // 直接使用 T::size()，如果 T 没有size()则硬错误
    // static constexpr bool value = sizeof(T().size()) > 0;  // 硬错误!
    static constexpr bool value = false;
};

// 正例：SFINAE 友好的 trait
template<typename T, typename = void>
struct good_has_size : std::false_type {};

template<typename T>
struct good_has_size<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};

// SFINAE 友好的类型特征：检测是否支持流输出
template<typename T, typename = void>
struct is_ostreamable : std::false_type {};

template<typename T>
struct is_ostreamable<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<const T&>())>> : std::true_type {};

template<typename T>
inline constexpr bool is_ostreamable_v = is_ostreamable<T>::value;

// SFINAE 友好的类型特征：检测是否支持范围for循环
template<typename T, typename = void>
struct is_range_for : std::false_type {};

template<typename T>
struct is_range_for<T, std::void_t<
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end())
>> : std::true_type {};

template<typename T>
inline constexpr bool is_range_for_v = is_range_for<T>::value;

void demo_sfinae_friendly_traits() {
    std::cout << "=== SFINAE 友好的 Trait 设计 ===\n";

    std::cout << "good_has_size:\n";
    std::cout << "  std::vector<int>: " << good_has_size<std::vector<int>>::value << "\n";
    std::cout << "  int:              " << good_has_size<int>::value << "\n";

    std::cout << "\nis_ostreamable:\n";
    std::cout << "  int:    " << is_ostreamable_v<int> << "\n";
    std::cout << "  string: " << is_ostreamable_v<std::string> << "\n";
    std::cout << "  Opaque: " << is_ostreamable_v<Opaque> << "\n";

    std::cout << "\nis_range_for:\n";
    std::cout << "  vector<int>: " << is_range_for_v<std::vector<int>> << "\n";
    std::cout << "  string:      " << is_range_for_v<std::string> << "\n";
    std::cout << "  int:         " << is_range_for_v<int> << "\n";

    std::cout << "\nSFINAE 友好设计原则:\n";
    std::cout << "  1. 主模板返回 false_type\n";
    std::cout << "  2. 特化版本用 void_t 检测表达式\n";
    std::cout << "  3. 检测失败时 SFINAE 退回主模板，而非硬错误\n";
    std::cout << "  4. 不要在 trait 内直接使用可能失败的表达式\n";

    std::cout << "\n";
}

// ============================================================
// 6. 综合示例：SFINAE 实现通用打印函数
// ============================================================

// 使用 SFINAE 为不同类型提供不同的打印实现
template<typename T>
std::enable_if_t<std::is_arithmetic_v<T>>
smart_print(const T& value) {
    std::cout << value;
}

template<typename T>
std::enable_if_t<is_range_for_v<T> && !std::is_same_v<std::decay_t<T>, std::string>>
smart_print(const T& container) {
    std::cout << "{";
    bool first = true;
    for (const auto& elem : container) {
        if (!first) std::cout << ", ";
        first = false;
        smart_print(elem);
    }
    std::cout << "}";
}

template<typename T>
std::enable_if_t<std::is_same_v<std::decay_t<T>, std::string>>
smart_print(const T& str) {
    std::cout << "\"" << str << "\"";
}

template<typename T>
std::enable_if_t<is_callable_v<T>>
smart_print(const T&) {
    std::cout << "<callable>";
}

void demo_comprehensive_sfinae() {
    std::cout << "=== 综合示例：SFINAE 通用打印 ===\n";

    std::cout << "整型: "; smart_print(42); std::cout << "\n";
    std::cout << "浮点: "; smart_print(3.14); std::cout << "\n";
    std::cout << "字符串: "; smart_print(std::string("hello")); std::cout << "\n";

    std::vector<int> vec = {1, 2, 3};
    std::cout << "vector: "; smart_print(vec); std::cout << "\n";

    std::vector<std::vector<int>> nested = {{1, 2}, {3, 4}};
    std::cout << "嵌套vector: "; smart_print(nested); std::cout << "\n";

    auto lam = [](){};
    std::cout << "lambda: "; smart_print(lam); std::cout << "\n";

    std::cout << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "  SFINAE 模式 (Substitution Failure Is Not An Error)\n";
    std::cout << "============================================\n\n";

    demo_sfinae_concept();
    demo_enable_if_overload();
    demo_partial_specialization_sfinae();
    demo_void_t_pattern();
    demo_sfinae_friendly_traits();
    demo_comprehensive_sfinae();

    std::cout << "============================================\n";
    std::cout << "  核心要点总结:\n";
    std::cout << "  1. SFINAE: 替换失败不是错误\n";
    std::cout << "  2. enable_if: 条件启用/禁用模板\n";
    std::cout << "  3. void_t: 优雅检测表达式合法性\n";
    std::cout << "  4. 偏特化+SFINAE: 类型分派\n";
    std::cout << "  5. SFINAE友好: 避免硬编译错误\n";
    std::cout << "============================================\n";

    return 0;
}
