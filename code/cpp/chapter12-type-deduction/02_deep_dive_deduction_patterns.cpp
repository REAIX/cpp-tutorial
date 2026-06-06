/** @file 02_deep_dive_deduction_patterns.cpp
 *  @brief 类型推导模式：现代C++推导、返回类型推导、结构化绑定与推导、decltype用于SFINAE
 *  @description 对应文档: 12-类型推导 | 举一反三：掌握类型推导的高级模式
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <tuple>
#include <type_traits>
#include <functional>

template<typename T, typename = void>
struct HasSize : std::false_type {};

template<typename T>
struct HasSize<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};

template<typename T, typename = void>
struct HasValueType : std::false_type {};

template<typename T>
struct HasValueType<T, std::void_t<typename T::value_type>> : std::true_type {};

template<typename T, typename = void>
struct IsIterable : std::false_type {};

template<typename T>
struct IsIterable<T, std::void_t<
    decltype(std::begin(std::declval<T&>())),
    decltype(std::end(std::declval<T&>()))
>> : std::true_type {};

template<typename T>
auto serialize(const T& value) -> std::enable_if_t<std::is_arithmetic_v<T>, std::string> {
    return std::to_string(value);
}

template<typename T>
auto serialize(const T& value) -> std::enable_if_t<std::is_same_v<T, std::string>, std::string> {
    return "\"" + value + "\"";
}

template<typename T>
auto serialize(const T& container) -> std::enable_if_t<
    HasSize<T>::value && !std::is_same_v<T, std::string>,
    std::string
> {
    std::string result = "[";
    bool first = true;
    for (const auto& item : container) {
        if (!first) result += ", ";
        first = false;
        result += serialize(item);
    }
    result += "]";
    return result;
}

void demo_return_type_deduction() {
    std::cout << "=== 返回类型推导 ===\n";

    auto add = [](auto a, auto b) {
        return a + b;
    };

    std::cout << "add(1, 2) = " << add(1, 2) << "\n";
    std::cout << "add(1.5, 2.5) = " << add(1.5, 2.5) << "\n";
    std::cout << "add(1, 2.5) = " << add(1, 2.5) << "\n\n";

    auto make_pair = [](auto a, auto b) -> std::pair<decltype(a), decltype(b)> {
        return {a, b};
    };

    auto p = make_pair(1, std::string("hello"));
    std::cout << "make_pair(1, \"hello\"): (" << p.first << ", " << p.second << ")\n\n";

    std::cout << "返回类型推导的方式:\n";
    std::cout << "  1. auto f() { return expr; }  => C++14\n";
    std::cout << "  2. decltype(auto) f() { return expr; }  => 保留引用\n";
    std::cout << "  3. auto f() -> decltype(expr) { return expr; }  => C++11\n\n";

    std::cout << "注意: 所有 return 语句必须推导为同一类型\n";

    std::cout << "\n";
}

void demo_structured_bindings() {
    std::cout << "=== 结构化绑定与推导 (C++17) ===\n";

    auto [x, y] = std::pair(1, 2.5);
    std::cout << "auto [x, y] = pair(1, 2.5): x=" << x << ", y=" << y << "\n\n";

    auto tuple = std::make_tuple(42, 3.14, std::string("hello"));
    auto [a, b, c] = tuple;
    std::cout << "tuple 解绑: a=" << a << ", b=" << b << ", c=" << c << "\n\n";

    struct Point { double x, y, z; };
    Point p{1.0, 2.0, 3.0};
    auto [px, py, pz] = p;
    std::cout << "结构体解绑: px=" << px << ", py=" << py << ", pz=" << pz << "\n\n";

    int arr[] = {10, 20, 30};
    auto [e1, e2, e3] = arr;
    std::cout << "数组解绑: e1=" << e1 << ", e2=" << e2 << ", e3=" << e3 << "\n\n";

    std::map<std::string, int> scores = {{"Alice", 95}, {"Bob", 87}, {"Charlie", 92}};
    std::cout << "map 遍历:\n";
    for (const auto& [name, score] : scores) {
        std::cout << "  " << name << ": " << score << "\n";
    }

    std::cout << "\n结构化绑定的推导:\n";
    std::cout << "  auto [a, b] = expr;  => 按值绑定 (拷贝)\n";
    std::cout << "  auto& [a, b] = expr;  => 按引用绑定\n";
    std::cout << "  const auto& [a, b] = expr;  => const引用绑定\n";

    std::cout << "\n";
}

void demo_decltype_for_sfinae() {
    std::cout << "=== decltype 用于 SFINAE ===\n";

    std::cout << "vector<int> 有 size(): " << (HasSize<std::vector<int>>::value ? "是" : "否") << "\n";
    std::cout << "int 有 size(): " << (HasSize<int>::value ? "是" : "否") << "\n";
    std::cout << "vector<int> 有 value_type: " << (HasValueType<std::vector<int>>::value ? "是" : "否") << "\n";
    std::cout << "int 有 value_type: " << (HasValueType<int>::value ? "是" : "否") << "\n";
    std::cout << "vector<int> 可迭代: " << (IsIterable<std::vector<int>>::value ? "是" : "否") << "\n";
    std::cout << "int 可迭代: " << (IsIterable<int>::value ? "是" : "否") << "\n";

    std::cout << "\ndecltype 在 SFINAE 中的作用:\n";
    std::cout << "  1. decltype(expr) 检测表达式是否合法\n";
    std::cout << "  2. std::declval<T>() 创建假设的 T 实例\n";
    std::cout << "  3. void_t 检测推导是否成功\n";
    std::cout << "  4. 失败则 SFINAE 退回主模板\n";

    std::cout << "\n";
}

void demo_deduction_in_generic_code() {
    std::cout << "=== 泛型代码中的推导模式 ===\n";

    std::cout << "serialize(42): " << serialize(42) << "\n";
    std::cout << "serialize(3.14): " << serialize(3.14) << "\n";
    std::cout << "serialize(std::string(\"hello\")): " << serialize(std::string("hello")) << "\n";

    std::vector<int> vec = {1, 2, 3};
    std::cout << "serialize(vector<int>): " << serialize(vec) << "\n";

    std::cout << "\n泛型代码的推导模式:\n";
    std::cout << "  1. 使用 auto 推导返回类型\n";
    std::cout << "  2. 使用 enable_if 约束重载\n";
    std::cout << "  3. 使用 type_traits 区分类型\n";
    std::cout << "  4. C++20 concepts 简化了这些模式\n";

    std::cout << "\n";
}

void demo_modern_deduction_summary() {
    std::cout << "=== 现代 C++ 类型推导总结 ===\n";

    std::cout << "C++11:\n";
    std::cout << "  - auto 变量推导\n";
    std::cout << "  - decltype\n";
    std::cout << "  - 尾置返回类型\n\n";

    std::cout << "C++14:\n";
    std::cout << "  - auto 返回类型推导\n";
    std::cout << "  - decltype(auto)\n";
    std::cout << "  - 泛型 lambda\n\n";

    std::cout << "C++17:\n";
    std::cout << "  - CTAD (类模板参数推导)\n";
    std::cout << "  - 结构化绑定\n";
    std::cout << "  - if constexpr\n\n";

    std::cout << "C++20:\n";
    std::cout << "  - Concepts (约束和概念)\n";
    std::cout << "  - 缩写函数模板 (auto 参数)\n";
    std::cout << "  - 范围 for 中的初始化语句\n\n";

    std::cout << "推导的最佳实践:\n";
    std::cout << "  1. 优先使用 auto, 减少冗余类型名\n";
    std::cout << "  2. 注意 auto 剥离引用和 const\n";
    std::cout << "  3. 范围 for 中使用 const auto&\n";
    std::cout << "  4. 需要保留引用时用 decltype(auto)\n";
    std::cout << "  5. 显式类型比错误推导更好\n";

    std::cout << "\n";
}

int main() {
    demo_return_type_deduction();
    demo_structured_bindings();
    demo_decltype_for_sfinae();
    demo_deduction_in_generic_code();
    demo_modern_deduction_summary();

    return 0;
}
