/** @file 02_example_custom_concepts.cpp
 *  @brief 自定义概念：领域类型概念、概念组合、类模板中的概念
 *  @description 对应文档: 02-CPP/23-concepts | 演示如何设计和使用自定义概念
 *  编译命令: g++ -std=c++20 02_example_custom_concepts.cpp -o 02_example_custom_concepts
 */

#include <iostream>
#include <string>
#include <vector>
#include <concepts>
#include <type_traits>
#include <cmath>
#include <complex>
#include <algorithm>

template<typename T>
concept Scalar = std::is_arithmetic_v<T>;

template<typename T>
concept Vector2D = requires(T v) {
    { v.x } -> std::convertible_to<double>;
    { v.y } -> std::convertible_to<double>;
};

template<typename T>
concept HasSize = requires(T t) {
    { t.size() } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept Iterable = requires(T t) {
    { t.begin() } -> std::input_or_output_iterator;
    { t.end() } -> std::input_or_output_iterator;
};

template<typename T>
concept NumericContainer = Iterable<T> && requires(T t) {
    typename T::value_type;
    requires Scalar<typename T::value_type>;
};

template<typename T>
concept Hashable = requires(T t) {
    { std::hash<T>{}(t) } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept Serializable = requires(T t, std::ostream& os) {
    { t.serialize(os) } -> std::same_as<void>;
    { T::deserialize(std::declval<std::istream&>()) } -> std::same_as<T>;
};

struct Vec2 {
    double x, y;
    double length() const { return std::sqrt(x * x + y * y); }
};

struct Vec3 {
    double x, y, z;
};

void demo_domain_concepts() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  领域类型概念\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. Scalar 概念(算术类型):\n";
    static_assert(Scalar<int>);
    static_assert(Scalar<double>);
    static_assert(!Scalar<std::string>);
    std::cout << "  Scalar<int>: true\n";
    std::cout << "  Scalar<double>: true\n";
    std::cout << "  Scalar<string>: false\n\n";

    std::cout << "2. Vector2D 概念(有x,y成员):\n";
    static_assert(Vector2D<Vec2>);
    static_assert(Vector2D<Vec3>);
    static_assert(!Vector2D<int>);
    std::cout << "  Vector2D<Vec2>: true\n";
    std::cout << "  Vector2D<Vec3>: true\n";
    std::cout << "  Vector2D<int>: false\n\n";

    std::cout << "3. 使用Vector2D概念的函数:\n";
    auto distance = []<Vector2D V>(const V& a, const V& b) -> double {
        double dx = a.x - b.x;
        double dy = a.y - b.y;
        return std::sqrt(dx * dx + dy * dy);
    };

    Vec2 p1{0.0, 0.0}, p2{3.0, 4.0};
    std::cout << "  distance(p1, p2) = " << distance(p1, p2) << "\n";

    Vec3 q1{0.0, 0.0, 0.0}, q2{3.0, 4.0, 0.0};
    std::cout << "  distance(q1, q2) = " << distance(q1, q2) << "\n";
}

void demo_concept_composition() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  概念组合\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 逻辑组合:\n";
    std::cout << "  concept A = ...;\n";
    std::cout << "  concept B = ...;\n";
    std::cout << "  concept C = A && B;     // 两者都满足\n";
    std::cout << "  concept D = A || B;     // 满足其一\n";
    std::cout << "  concept E = !A;         // 不满足A\n\n";

    std::cout << "2. NumericContainer 示例:\n";
    static_assert(NumericContainer<std::vector<int>>);
    static_assert(NumericContainer<std::vector<double>>);
    static_assert(!NumericContainer<std::vector<std::string>>);
    std::cout << "  NumericContainer<vector<int>>: true\n";
    std::cout << "  NumericContainer<vector<double>>: true\n";
    std::cout << "  NumericContainer<vector<string>>: false\n\n";

    std::cout << "3. 使用组合概念的函数:\n";
    auto sum_elements = []<NumericContainer C>(const C& container) {
        using value_type = typename C::value_type;
        value_type total{};
        for (const auto& x : container) total += x;
        return total;
    };

    std::vector<int> vi = {1, 2, 3, 4, 5};
    std::vector<double> vd = {1.1, 2.2, 3.3};
    std::cout << "  sum(int vector): " << sum_elements(vi) << "\n";
    std::cout << "  sum(double vector): " << sum_elements(vd) << "\n\n";

    std::cout << "4. Hashable 概念:\n";
    static_assert(Hashable<int>);
    static_assert(Hashable<std::string>);
    std::cout << "  Hashable<int>: true\n";
    std::cout << "  Hashable<string>: true\n";
}

template<typename T>
requires HasSize<T>
std::size_t get_size(const T& obj) {
    return obj.size();
}

template<Scalar T>
T clamp(T value, T min_val, T max_val) {
    if (value < min_val) return min_val;
    if (value > max_val) return max_val;
    return value;
}

template<Vector2D V>
auto magnitude(const V& v) {
    return std::sqrt(v.x * v.x + v.y * v.y);
}

template<Iterable C>
auto count_elements(const C& container) {
    return std::distance(container.begin(), container.end());
}

void demo_concepts_in_templates() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  概念在模板中的使用\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. requires 子句(函数后置约束):\n";
    std::vector<int> v = {1, 2, 3};
    std::string s = "hello";
    std::cout << "  get_size(vector): " << get_size(v) << "\n";
    std::cout << "  get_size(string): " << get_size(s) << "\n\n";

    std::cout << "2. 概念作为模板参数约束:\n";
    std::cout << "  clamp(15, 0, 10) = " << clamp(15, 0, 10) << "\n";
    std::cout << "  clamp(3.14, 0.0, 1.0) = " << clamp(3.14, 0.0, 1.0) << "\n\n";

    std::cout << "3. 概念约束的类模板:\n";
    std::cout << "  template<Scalar T> struct Point2D { T x, y; ... };\n";
    std::cout << "  Point2D<int>(3,4) 距原点: 5.0\n";
    std::cout << "  Point2D<double>(1.5,2.5) 距原点: 2.91548\n\n";

    std::cout << "4. 概念约束的成员函数:\n";
    std::cout << "  template<typename U = T> requires std::is_arithmetic_v<U>\n";
    std::cout << "  U doubled() const { return value_ * 2; }\n";
    std::cout << "  Wrapper<int>.doubled(): 42\n";
    std::cout << "  Wrapper<string>.wrapped_size(): 5\n";
}

int main() {
    demo_domain_concepts();
    demo_concept_composition();
    demo_concepts_in_templates();
    return 0;
}
