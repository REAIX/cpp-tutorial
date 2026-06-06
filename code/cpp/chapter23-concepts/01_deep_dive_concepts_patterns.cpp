/** @file 01_deep_dive_concepts_patterns.cpp
 *  @brief 基于概念的重载、概念原型、概念与标签分发对比、概念测试
 *  @description 对应文档: 02-CPP/23-concepts | 举一反三：Concepts的高级使用模式和最佳实践
 *  编译命令: g++ -std=c++20 01_deep_dive_concepts_patterns.cpp -o 01_deep_dive_concepts_patterns
 */

#include <iostream>
#include <string>
#include <vector>
#include <concepts>
#include <type_traits>
#include <iterator>
#include <algorithm>
#include <cmath>
#include <numeric>

template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept Container = requires(T c) {
    typename T::value_type;
    { c.begin() } -> std::input_or_output_iterator;
    { c.end() } -> std::input_or_output_iterator;
};

template<typename T>
concept RandomAccessContainer = Container<T> && requires(T c) {
    { c.begin() } -> std::random_access_iterator;
    { c[0] } -> std::convertible_to<typename T::value_type>;
};

void demo_concept_overloading() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  基于概念的重载\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto compute = []<Numeric T>(T x) -> T {
        return x * 2;
    };

    auto compute_str = [](const std::string& s) -> std::string {
        return s + s;
    };

    std::cout << "1. 不同概念约束的函数:\n";
    std::cout << "  compute(42) = " << compute(42) << "\n";
    std::cout << "  compute(3.14) = " << compute(3.14) << "\n";
    std::cout << "  compute_str(\"hi\") = " << compute_str("hi") << "\n\n";

    std::cout << "2. 更严格概念优先:\n";
    auto sum_all = []<Container C>(const C& c)
        requires (!RandomAccessContainer<C>)
    {
        using T = typename C::value_type;
        T total{};
        for (const auto& x : c) total += x;
        return total;
    };

    auto sum_fast = []<RandomAccessContainer C>(const C& c) {
        using T = typename C::value_type;
        T total{};
        for (std::size_t i = 0; i < c.size(); i++) total += c[i];
        return total;
    };

    std::vector<int> v = {1, 2, 3, 4, 5};
    std::cout << "  vector(随机访问): sum_fast = " << sum_fast(v) << "\n\n";

    std::cout << "3. 概念重载决议规则:\n";
    std::cout << "  更严格的约束优先于更宽松的约束\n";
    std::cout << "  RandomAccessContainer 比 Container 更严格\n";
    std::cout << "  编译器自动选择最匹配的重载\n";
}

void demo_concept_archetypes() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  概念原型 (Archetypes)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "概念原型: 满足概念的最小类型实现\n";
    std::cout << "用于验证概念定义是否恰好描述了所需接口\n\n";

    struct MinimalContainer {
        using value_type = int;
        int data[3] = {1, 2, 3};

        int* begin() { return data; }
        int* end() { return data + 3; }
        std::size_t size() const { return 3; }
    };

    static_assert(Container<MinimalContainer>);
    std::cout << "MinimalContainer 满足 Container 概念:\n";
    MinimalContainer mc;
    for (const auto& x : mc) std::cout << "  " << x << "\n";

    std::cout << "\n概念原型的价值:\n";
    std::cout << "  1. 验证概念不过度约束\n";
    std::cout << "  2. 验证概念不缺少约束\n";
    std::cout << "  3. 作为概念文档的具体示例\n";
    std::cout << "  4. 用于概念测试\n\n";

    std::cout << "常见概念原型:\n";
    std::cout << "  Incrementable → 只支持 ++ 的类型\n";
    std::cout << "  Sortable → 支持 begin/end/swap/< 的类型\n";
    std::cout << "  Numeric → 只支持算术运算的类型\n";
}

void demo_concept_vs_tag_dispatch() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  Concepts vs 标签分发 (Tag Dispatch)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "标签分发(C++11方式):\n";
    std::cout << "  template<typename Iter>\n";
    std::cout << "  void sort_impl(Iter begin, Iter end, std::random_access_iterator_tag) { ... }\n";
    std::cout << "  template<typename Iter>\n";
    std::cout << "  void sort_impl(Iter begin, Iter end, std::input_iterator_tag) { ... }\n";
    std::cout << "  template<typename Iter>\n";
    std::cout << "  void sort(Iter begin, Iter end) {\n";
    std::cout << "    sort_impl(begin, end, typename std::iterator_traits<Iter>::iterator_category{});\n";
    std::cout << "  }\n\n";

    std::cout << "Concepts(C++20方式):\n";
    std::cout << "  template<std::random_access_iterator Iter>\n";
    std::cout << "  void sort(Iter begin, Iter end) { ... }\n";
    std::cout << "  template<std::input_iterator Iter>\n";
    std::cout << "    requires (!std::random_access_iterator<Iter>)\n";
    std::cout << "  void sort(Iter begin, Iter end) { ... }\n\n";

    std::cout << "对比:\n";
    std::cout << "  ┌──────────────┬──────────────┬──────────────┐\n";
    std::cout << "  │ 特性         │ 标签分发     │ Concepts     │\n";
    std::cout << "  ├──────────────┼──────────────┼──────────────┤\n";
    std::cout << "  │ 代码量       │ 多(辅助函数) │ 少(直接约束) │\n";
    std::cout << "  │ 可读性       │ 低           │ 高           │\n";
    std::cout << "  │ 错误信息     │ 差           │ 好           │\n";
    std::cout << "  │ 灵活性       │ 中           │ 高           │\n";
    std::cout << "  │ 编译速度     │ 较慢         │ 较快         │\n";
    std::cout << "  └──────────────┴──────────────┴──────────────┘\n";
}

void demo_concept_testing() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  概念测试\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. static_assert 验证:\n";
    static_assert(Numeric<int>);
    static_assert(Numeric<double>);
    static_assert(!Numeric<std::string>);
    std::cout << "  Numeric<int>: true ✓\n";
    std::cout << "  Numeric<double>: true ✓\n";
    std::cout << "  Numeric<string>: false ✓\n\n";

    std::cout << "2. 反向验证(不应满足):\n";
    static_assert(!Container<int>);
    static_assert(!Numeric<void>);
    std::cout << "  Container<int>: false ✓\n";
    std::cout << "  Numeric<void>: false ✓\n\n";

    std::cout << "3. 边界情况测试:\n";
    static_assert(std::integral<bool>);
    static_assert(std::integral<char>);
    static_assert(!std::integral<float>);
    std::cout << "  integral<bool>: true ✓\n";
    std::cout << "  integral<char>: true ✓\n";
    std::cout << "  integral<float>: false ✓\n\n";

    std::cout << "4. 概念测试最佳实践:\n";
    std::cout << "  - 测试应该满足概念的类型\n";
    std::cout << "  - 测试不应该满足概念的类型\n";
    std::cout << "  - 测试边界类型(bool, char等)\n";
    std::cout << "  - 测试const/volatile变体\n";
    std::cout << "  - 测试引用类型\n";
    std::cout << "  - 将 static_assert 放在头文件中作为文档\n";
}

int main() {
    demo_concept_overloading();
    demo_concept_archetypes();
    demo_concept_vs_tag_dispatch();
    demo_concept_testing();
    return 0;
}
