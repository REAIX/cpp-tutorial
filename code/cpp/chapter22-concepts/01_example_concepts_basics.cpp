/** @file 01_example_concepts_basics.cpp
 *  @brief Concepts基础：概念定义、requires子句、requires表达式、标准概念
 *  @description 对应文档: 02-CPP/23-concepts | 演示C++20 Concepts的核心语法
 *  编译命令: g++ -std=c++20 01_example_concepts_basics.cpp -o 01_example_concepts_basics
 */

#include <iostream>
#include <string>
#include <vector>
#include <concepts>
#include <type_traits>
#include <cmath>

void demo_concept_definition() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  Concept 定义方式\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 简单概念(基于标准概念组合):\n";
    std::cout << "  template<typename T>\n";
    std::cout << "  concept Integral = std::is_integral_v<T>;\n\n";

    std::cout << "2. requires 表达式定义概念:\n";
    std::cout << "  template<typename T>\n";
    std::cout << "  concept Addable = requires(T a, T b) {\n";
    std::cout << "    { a + b } -> std::convertible_to<T>;\n";
    std::cout << "  };\n\n";

    std::cout << "3. 多约束组合:\n";
    std::cout << "  template<typename T>\n";
    std::cout << "  concept Numeric = std::integral<T> || std::floating_point<T>;\n\n";

    std::cout << "4. 约束嵌套:\n";
    std::cout << "  template<typename T>\n";
    std::cout << "  concept Sortable = requires(T& container) {\n";
    std::cout << "    { container.begin() } -> std::random_access_iterator;\n";
    std::cout << "    { container.end() } -> std::random_access_iterator;\n";
    std::cout << "    { *container.begin() < *container.begin() } -> std::convertible_to<bool>;\n";
    std::cout << "  };\n";
}

template<typename T>
concept Integral = std::is_integral_v<T>;

template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept Printable = requires(T t, std::ostream& os) {
    { os << t } -> std::same_as<std::ostream&>;
};

void demo_requires_clause() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  requires 子句\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto add_integral = []<Integral T>(T a, T b) -> T {
        return a + b;
    };

    std::cout << "Integral 约束:\n";
    std::cout << "  add_integral(3, 4) = " << add_integral(3, 4) << "\n";
    std::cout << "  add_integral(3L, 4L) = " << add_integral(3L, 4L) << "\n";
    std::cout << "  add_integral(3.14, 2.0) → 编译错误(double不是Integral)\n\n";

    auto add_numeric = []<Numeric T>(T a, T b) -> T {
        return a + b;
    };

    std::cout << "Numeric 约束:\n";
    std::cout << "  add_numeric(3, 4) = " << add_numeric(3, 4) << "\n";
    std::cout << "  add_numeric(3.14, 2.0) = " << add_numeric(3.14, 2.0) << "\n\n";

    auto print_value = []<Printable T>(const T& val) {
        std::cout << "  值: " << val << "\n";
    };

    std::cout << "Printable 约束:\n";
    print_value(42);
    print_value(3.14);
    print_value(std::string("hello"));
}

void demo_requires_expression() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  requires 表达式\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "requires 表达式的四种形式:\n\n";

    std::cout << "1. 简单要求: 表达式必须合法\n";
    std::cout << "  requires { expression; }\n\n";

    std::cout << "2. 类型要求: 类型必须合法\n";
    std::cout << "  requires { typename T::value_type; }\n\n";

    std::cout << "3. 复合要求: 表达式 + 返回类型约束\n";
    std::cout << "  requires { { expr } -> concept; }\n\n";

    std::cout << "4. 嵌套要求: 引入另一个概念\n";
    std::cout << "  requires concept_name<T>;\n\n";
}

template<typename T>
concept Container = requires(T c) {
    typename T::value_type;
    { c.begin() } -> std::input_or_output_iterator;
    { c.end() } -> std::input_or_output_iterator;
    { c.size() } -> std::convertible_to<std::size_t>;
};

void demo_container_concept() {
    static_assert(Container<std::vector<int>>);
    static_assert(!Container<int>);
    std::cout << "Container<std::vector<int>>: true\n";
    std::cout << "Container<int>: false\n";
}

void demo_standard_concepts() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  标准库概念\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "核心语言概念:\n";
    std::cout << "  std::same_as<T, U>          —— T和U是同一类型\n";
    std::cout << "  std::derived_from<T, Base>  —— T派生自Base\n";
    std::cout << "  std::convertible_to<T, U>   —— T可转换为U\n";
    std::cout << "  std::common_reference_with  —— 有公共引用类型\n\n";

    std::cout << "类型分类概念:\n";
    std::cout << "  std::integral<T>       —— 整数类型\n";
    std::cout << "  std::floating_point<T> —— 浮点类型\n";
    std::cout << "  std::signed_integral<T>  —— 有符号整数\n";
    std::cout << "  std::unsigned_integral<T> —— 无符号整数\n\n";

    static_assert(std::integral<int>);
    static_assert(!std::integral<double>);
    static_assert(std::floating_point<double>);
    static_assert(std::signed_integral<int>);
    static_assert(std::unsigned_integral<unsigned int>);
    std::cout << "  static_assert 检查全部通过\n\n";

    std::cout << "比较概念:\n";
    std::cout << "  std::equality_comparable<T>   —— 可用==比较\n";
    std::cout << "  std::totally_ordered<T>       —— 完全有序\n";
    static_assert(std::equality_comparable<int>);
    static_assert(std::totally_ordered<double>);

    std::cout << "\n对象概念:\n";
    std::cout << "  std::movable<T>     —— 可移动\n";
    std::cout << "  std::copyable<T>    —— 可拷贝\n";
    std::cout << "  std::semiregular<T> —— 半正则(可拷贝+默认构造)\n";
    std::cout << "  std::regular<T>     —— 正则(半正则+可比较)\n";
    static_assert(std::movable<int>);
    static_assert(std::copyable<std::string>);
    static_assert(std::regular<int>);

    std::cout << "\n可调用概念:\n";
    std::cout << "  std::invocable<F, Args...>       —— 可调用\n";
    std::cout << "  std::predicate<F, Args...>       —— 返回bool的可调用\n";
    std::cout << "  std::regular_invocable<F, Args...> —— 正则可调用\n";
}

void demo_concept_check() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  编译期概念检查\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "static_assert 检查:\n";
    static_assert(Integral<int>);
    static_assert(Integral<long>);
    static_assert(!Integral<double>);
    static_assert(!Integral<std::string>);
    std::cout << "  Integral<int>: true\n";
    std::cout << "  Integral<long>: true\n";
    std::cout << "  Integral<double>: false\n";
    std::cout << "  Integral<string>: false\n\n";

    std::cout << "requires 表达式检查:\n";
    constexpr bool int_addable = Addable<int>;
    constexpr bool string_addable = Addable<std::string>;
    std::cout << "  Addable<int>: " << int_addable << "\n";
    std::cout << "  Addable<string>: " << string_addable << "\n\n";

    std::cout << "if constexpr 运行时分支:\n";
    auto describe = []<typename T>(T) {
        if constexpr (std::integral<T>) {
            std::cout << "  整数类型\n";
        } else if constexpr (std::floating_point<T>) {
            std::cout << "  浮点类型\n";
        } else {
            std::cout << "  其他类型\n";
        }
    };
    describe(42);
    describe(3.14);
    describe(std::string("hello"));
}

int main() {
    demo_concept_definition();
    demo_requires_clause();
    demo_requires_expression();
    demo_standard_concepts();
    demo_concept_check();
    return 0;
}
