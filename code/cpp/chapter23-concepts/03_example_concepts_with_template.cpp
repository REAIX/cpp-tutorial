/** @file 03_example_concepts_with_template.cpp
 *  @brief Concepts vs SFINAE、约束auto、缩写函数模板
 *  @description 对应文档: 02-CPP/23-concepts | 演示Concepts替代SFINAE的优雅方式
 *  编译命令: g++ -std=c++20 03_example_concepts_with_template.cpp -o 03_example_concepts_with_template
 */

#include <iostream>
#include <string>
#include <vector>
#include <concepts>
#include <type_traits>
#include <iterator>
#include <algorithm>
#include <cmath>

void demo_concepts_vs_sfinae() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  Concepts vs SFINAE\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "SFINAE 方式 (C++11/14/17):\n";
    std::cout << "  template<typename T,\n";
    std::cout << "           typename = std::enable_if_t<std::is_integral_v<T>>>\n";
    std::cout << "  T process(T value) { return value * 2; }\n\n";

    std::cout << "Concepts 方式 (C++20):\n";
    std::cout << "  template<std::integral T>\n";
    std::cout << "  T process(T value) { return value * 2; }\n\n";

    auto process_sfinae = []<typename T>(T value)
        -> std::enable_if_t<std::is_integral_v<T>, T> {
        return value * 2;
    };

    auto process_concept = []<std::integral T>(T value) -> T {
        return value * 2;
    };

    std::cout << "结果对比:\n";
    std::cout << "  SFINAE: process(5) = " << process_sfinae(5) << "\n";
    std::cout << "  Concept: process(5) = " << process_concept(5) << "\n\n";

    std::cout << "Concepts 的优势:\n";
    std::cout << "  1. 更简洁 —— 一行约束代替 enable_if 嵌套\n";
    std::cout << "  2. 更好的错误信息 —— 指出哪个约束不满足\n";
    std::cout << "  3. 更直观 —— 接口即文档\n";
    std::cout << "  4. 更快编译 —— 减少模板实例化\n\n";

    std::cout << "错误信息对比:\n";
    std::cout << "  SFINAE: \"no matching function\" + 长篇模板替换信息\n";
    std::cout << "  Concepts: \"constraint not satisfied: std::integral<double>\"\n";
}

void demo_constrained_auto() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  约束 auto (constrained auto)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::integral auto x = 42;
    std::floating_point auto y = 3.14;
    std::same_as<std::string> auto s = std::string("hello");

    std::cout << "约束auto变量:\n";
    std::cout << "  std::integral auto x = 42;       → x = " << x << "\n";
    std::cout << "  std::floating_point auto y = 3.14; → y = " << y << "\n";
    std::cout << "  std::same_as<string> auto s = ...; → s = " << s << "\n\n";

    std::cout << "约束auto返回类型:\n";
    auto get_integral = []() -> std::integral auto {
        return 42;
    };

    auto get_float = []() -> std::floating_point auto {
        return 3.14;
    };

    std::cout << "  get_integral() = " << get_integral() << "\n";
    std::cout << "  get_float() = " << get_float() << "\n\n";

    std::cout << "编译期检查:\n";
    std::cout << "  std::integral auto x = 3.14; → 编译错误!\n";
    std::cout << "  double 不满足 std::integral 约束\n";
}

void demo_abbreviated_templates() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  缩写函数模板 (Abbreviated Function Templates)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "传统模板 vs 缩写模板:\n";
    std::cout << "  template<typename T>  void foo(T x);   // 传统\n";
    std::cout << "  void foo(auto x);                        // 缩写\n\n";
    std::cout << "  template<typename T>  void foo(T x);   // 传统\n";
    std::cout << "  void foo(std::integral auto x);          // 缩写+约束\n\n";

    auto add = [](auto a, auto b) {
        return a + b;
    };

    auto add_integral = [](std::integral auto a, std::integral auto b) {
        return a + b;
    };

    auto add_same = [](std::integral auto a, std::same_as<decltype(a)> auto b) {
        return a + b;
    };

    std::cout << "add(3, 4) = " << add(3, 4) << "\n";
    std::cout << "add(1.5, 2.3) = " << add(1.5, 2.3) << "\n";
    std::cout << "add_integral(3, 4) = " << add_integral(3, 4) << "\n";
    std::cout << "add_integral(3, 4L) = " << add_integral(3, 4L) << "\n";
    std::cout << "add_same(3, 4) = " << add_same(3, 4) << "\n\n";

    std::cout << "缩写模板的约束:\n";
    auto process = [](std::integral auto x) {
        return x * 2;
    };

    std::cout << "  process(42) = " << process(42) << "\n";
    std::cout << "  process(3.14) → 编译错误(double不满足integral)\n\n";

    std::cout << "缩写模板与概念结合:\n";
    auto print_container = [](const std::ranges::range auto& c) {
        for (const auto& x : c) {
            std::cout << x << " ";
        }
        std::cout << "\n";
    };

    std::vector<int> v = {1, 2, 3, 4, 5};
    std::cout << "  vector: ";
    print_container(v);
}

void demo_overload_with_concepts() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  基于Concepts的重载\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto process = [](auto value) {
        if constexpr (std::integral<decltype(value)>) {
            return value * 2;
        } else if constexpr (std::floating_point<decltype(value)>) {
            return value * 2.5;
        } else {
            return value;
        }
    };

    std::cout << "if constexpr + concepts 分派:\n";
    std::cout << "  process(10) = " << process(10) << "\n";
    std::cout << "  process(3.14) = " << process(3.14) << "\n";
    std::cout << "  process(\"hello\") = " << process("hello") << "\n\n";

    auto handle_integral = [](std::integral auto x) {
        std::cout << "  整数处理: " << x << " → " << x * 2 << "\n";
    };

    auto handle_float = [](std::floating_point auto x) {
        std::cout << "  浮点处理: " << x << " → " << std::sqrt(x) << "\n";
    };

    auto handle_other = [](const auto& x) {
        std::cout << "  通用处理: " << x << "\n";
    };

    std::cout << "概念约束的重载(不同函数):\n";
    handle_integral(42);
    handle_float(2.0);
    handle_other(std::string("hello"));
}

int main() {
    demo_concepts_vs_sfinae();
    demo_constrained_auto();
    demo_abbreviated_templates();
    demo_overload_with_concepts();
    return 0;
}
