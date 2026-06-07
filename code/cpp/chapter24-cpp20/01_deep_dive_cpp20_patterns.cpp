/**
 * @file 01_deep_dive_cpp20_patterns.cpp
 * @brief C++20实践模式与迁移
 * @description 对应文档: 02-CPP/25-cpp20
 */

#include <iostream>
#include <vector>
#include <string>
#include <concepts>
#include <ranges>
#include <compare>
#include <type_traits>
#include <array>
#include <algorithm>
#include <functional>

template<typename T>
concept Hashable = requires(T t) {
    { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
};

template<Hashable T>
std::size_t compute_hash(const T& val) {
    return std::hash<T>{}(val);
}

template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<typename C>
concept SortableContainer = requires(C& c) {
    { c.begin() } -> std::random_access_iterator;
    { c.end() } -> std::random_access_iterator;
    requires std::totally_ordered<typename C::value_type>;
};

void demo_feature_test_macros() {
    std::cout << "\n=== 特性测试宏 ===\n";

    std::cout << "__cpp_concepts: "
#if __cpp_concepts
        << __cpp_concepts
#else
        << "未定义"
#endif
        << "\n";

    std::cout << "__cpp_ranges: "
#if __cpp_ranges
        << __cpp_ranges
#else
        << "未定义"
#endif
        << "\n";

    std::cout << "__cpp_three_way_comparison: "
#if __cpp_three_way_comparison
        << __cpp_three_way_comparison
#else
        << "未定义"
#endif
        << "\n";

    std::cout << "__cpp_coroutines: "
#if __cpp_coroutines
        << __cpp_coroutines
#else
        << "未定义"
#endif
        << "\n";

    std::cout << "__cpp_modules: "
#if __cpp_modules
        << __cpp_modules
#else
        << "未定义"
#endif
        << "\n";

    std::cout << "__cpp_consteval: "
#if __cpp_consteval
        << __cpp_consteval
#else
        << "未定义"
#endif
        << "\n";

    std::cout << "__cpp_constinit: "
#if __cpp_constinit
        << __cpp_constinit
#else
        << "未定义"
#endif
        << "\n";

    std::cout << "__cpp_designated_initializers: "
#if __cpp_designated_initializers
        << __cpp_designated_initializers
#else
        << "未定义"
#endif
        << "\n";

    std::cout << "\n用法: #if __cpp_concepts >= 202002L ... #endif\n";
}

void demo_concepts_patterns() {
    std::cout << "\n=== Concepts实践模式 ===\n";

    std::cout << "hash(42): " << compute_hash(42) << "\n";
    std::cout << "hash(\"hello\"): " << compute_hash(std::string("hello")) << "\n";

    auto print_value = []<typename T>(const T& val) requires requires(std::ostream& os, T v) { os << v; } {
        std::cout << val << "\n";
    };
    print_value(42);
    print_value("hello");

    std::cout << "vector<int>是SortableContainer: "
              << std::boolalpha << SortableContainer<std::vector<int>> << "\n";

    auto average = []<Numeric T>(const std::vector<T>& vals) -> double {
        if (vals.empty()) return 0.0;
        double sum = 0;
        for (const auto& v : vals) sum += v;
        return sum / vals.size();
    };
    std::vector<int> vi = {1, 2, 3, 4, 5};
    std::vector<double> vd = {1.5, 2.5, 3.5};
    std::cout << "整数平均: " << average(vi) << "\n";
    std::cout << "浮点平均: " << average(vd) << "\n";
}

void demo_comparison_patterns() {
    std::cout << "\n=== 三路比较实践模式 ===\n";

    struct Version {
        int major;
        int minor;
        int patch;

        std::strong_ordering operator<=>(const Version&) const = default;
    };

    Version v1{2, 0, 1}, v2{2, 1, 0}, v3{2, 0, 1};
    std::cout << "v1 <=> v2: " << (v1 < v2 ? "小于" : (v1 > v2 ? "大于" : "等于")) << "\n";
    std::cout << "v1 == v3: " << std::boolalpha << (v1 == v3) << "\n";

    struct CaseInsensitiveString {
        std::string value;

        std::weak_ordering operator<=>(const CaseInsensitiveString& other) const {
            auto to_lower = [](unsigned char c) { return std::tolower(c); };
            std::string a = value, b = other.value;
            std::transform(a.begin(), a.end(), a.begin(), to_lower);
            std::transform(b.begin(), b.end(), b.begin(), to_lower);
            return a <=> b;
        }
        bool operator==(const CaseInsensitiveString& other) const {
            return (*this <=> other) == std::weak_ordering::equivalent;
        }
    };

    CaseInsensitiveString s1{"Hello"}, s2{"HELLO"}, s3{"World"};
    std::cout << "\"Hello\" <=> \"HELLO\" (弱序): "
              << (s1 == s2 ? "等价" : "不等价") << "\n";
    std::cout << "\"Hello\" <=> \"World\": "
              << (s1 < s3 ? "小于" : "大于") << "\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  strong_ordering: 精确等价(可替换)\n";
    std::cout << "  weak_ordering: 等价但不可替换(如大小写不敏感)\n";
    std::cout << "  partial_ordering: 可能不可比(如NaN)\n";
}

void demo_migration_from_cpp17() {
    std::cout << "\n=== 从C++17迁移到C++20 ===\n";

    std::cout << "1. SFINAE -> Concepts\n";
    std::cout << "   C++17: std::enable_if_t<std::is_integral_v<T>, T>\n";
    std::cout << "   C++20: requires std::integral<T> 或 template<std::integral T>\n";

    std::cout << "\n2. 手写比较运算符 -> <=>\n";
    std::cout << "   C++17: 手写operator<, >, <=, >=, ==, !=\n";
    std::cout << "   C++20: auto operator<=>(const T&) const = default;\n";

    std::cout << "\n3. 原始循环 -> Ranges\n";
    std::cout << "   C++17: for + if + push_back\n";
    std::cout << "   C++20: views::filter | views::transform\n";

    std::cout << "\n4. 聚合初始化 -> 指定初始化器\n";
    struct OldStyle { int x; int y; int z; };
    OldStyle old{1, 0, 3};
    std::cout << "   C++17: {1, 0, 3} 必须按顺序, y=0容易被误解\n";
    struct NewStyle { int x = 0; int y = 0; int z = 0; };
    NewStyle nw{.x = 1, .z = 3};
    std::cout << "   C++20: {.x = 1, .z = 3} 意图清晰\n";

    std::cout << "\n5. 运行时检查 -> consteval\n";
    std::cout << "   C++17: constexpr函数可能运行时调用\n";
    std::cout << "   C++20: consteval保证编译期执行\n";
}

void demo_compiler_support() {
    std::cout << "\n=== 编译器支持状态 ===\n";

    std::cout << "GCC 10+: Concepts, Ranges, <=>, 指定初始化器\n";
    std::cout << "GCC 11+: 改进的Ranges支持\n";
    std::cout << "GCC 12+: 更完善的Modules和Coroutines\n";
    std::cout << "Clang 10+: 部分Concepts\n";
    std::cout << "Clang 13+: 较完整的C++20\n";
    std::cout << "MSVC 19.28+: 较完整的C++20\n";

    std::cout << "\n当前编译器版本信息:\n";
#ifdef __GNUC__
    std::cout << "GCC: " << __GNUC__ << "." << __GNUC_MINOR__ << "." << __GNUC_PATCHLEVEL__ << "\n";
#endif
#ifdef __clang__
    std::cout << "Clang: " << __clang_major__ << "." << __clang_minor__ << "\n";
#endif
#ifdef _MSC_VER
    std::cout << "MSVC: " << _MSC_VER << "\n";
#endif

    std::cout << "\n编译建议:\n";
    std::cout << "  g++ -std=c++20 file.cpp\n";
    std::cout << "  GCC协程可能需要: -fcoroutines\n";
    std::cout << "  Clang: -std=c++20 -fcoroutines-ts (旧版本)\n";
}

void demo_pitfalls() {
    std::cout << "\n=== C++20常见陷阱 ===\n";

    std::cout << "1. <=>的默认实现不包含==\n";
    std::cout << "   需要: auto operator<=>(const T&) const = default;\n";
    std::cout << "   C++20会自动生成==, 但自定义<=>时需要同时处理==\n";

    std::cout << "\n2. Concepts的歧义\n";
    std::cout << "   requires子句中的&&和||需要仔细理解\n";
    std::cout << "   requires { expr } 和 requires concept<T> 是不同的\n";

    std::cout << "\n3. Ranges的悬垂引用\n";
    std::cout << "   auto view = std::vector<int>{1,2,3} | std::views::take(2);\n";
    std::cout << "   临时vector销毁后, view悬垂!\n";

    std::cout << "\n4. 指定初始化器的顺序\n";
    std::cout << "   必须按声明顺序: {.y=2, .x=1} 如果x在y前声明则错误\n";

    std::cout << "\n5. constinit与动态初始化\n";
    std::cout << "   constinit变量不能有动态初始化\n";
    std::cout << "   constinit int x = func(); // func必须是constexpr\n";
}

int main() {
    std::cout << "========== C++20 实践模式与迁移 ==========\n";

    demo_feature_test_macros();
    demo_concepts_patterns();
    demo_comparison_patterns();
    demo_migration_from_cpp17();
    demo_compiler_support();
    demo_pitfalls();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
