/** @file 02_deep_dive_sfinae_vs_concepts.cpp
 *  @brief SFINAE vs Concepts对比：为什么Concepts更好，迁移指南
 *  @description 对应文档: 07-模板元编程与编译期计算 / SFINAE与替换失败(深入)
 */

#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <concepts>
#include <iterator>

// ============================================================
// 1. SFINAE 的痛点
// ============================================================

// 痛点1：错误信息难以理解
// 当 SFINAE 排除所有候选时，编译器给出的错误信息通常非常冗长

// SFINAE 版本：约束整型
template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
T sfinae_add(T a, T b) {
    return a + b;
}

// 痛点2：代码冗长且重复
// 每个需要约束的地方都要写一长串 enable_if

// SFINAE 版本：约束容器
template<typename T, std::enable_if_t<
    std::is_same_v<decltype(std::declval<T>().begin()), typename T::iterator> &&
    std::is_same_v<decltype(std::declval<T>().end()), typename T::iterator>,
    int> = 0>
void sfinae_print_container(const T& c) {
    for (const auto& e : c) std::cout << e << " ";
    std::cout << "\n";
}

// 痛点3：组合约束极其复杂
// 需要"且"和"或"时，SFINAE 写法非常笨拙

// SFINAE 版本：整型或浮点型
template<typename T, std::enable_if_t<
    std::is_integral_v<T> || std::is_floating_point_v<T>, int> = 0>
std::string sfinae_type_name() {
    if constexpr (std::is_integral_v<T>) return "整型";
    else return "浮点型";
}

void demo_sfinae_pain_points() {
    std::cout << "=== SFINAE 的痛点 ===\n";

    std::cout << "1. 错误信息冗长难懂\n";
    std::cout << "   SFINAE 失败时，编译器可能输出数百行模板实例化信息\n\n";

    std::cout << "2. 代码冗长重复\n";
    std::cout << "   enable_if_t<condition, int> = 0 到处都是\n\n";

    std::cout << "3. 组合约束复杂\n";
    std::cout << "   多个条件的与/或组合使代码难以阅读\n\n";

    // SFINAE 版本可以工作
    std::cout << "sfinae_add(1, 2) = " << sfinae_add(1, 2) << "\n";
    std::cout << "sfinae_type_name<int>() = " << sfinae_type_name<int>() << "\n";
    // sfinae_add("a", "b");  // 错误信息非常不友好

    std::cout << "\n";
}

// ============================================================
// 2. Concepts 的优雅解决方案
// ============================================================

// Concepts 是 C++20 引入的命名约束，替代 SFINAE

// 定义 Concept：比 SFINAE 清晰得多
template<typename T>
concept Integral = std::is_integral_v<T>;

template<typename T>
concept FloatingPoint = std::is_floating_point_v<T>;

template<typename T>
concept Numeric = Integral<T> || FloatingPoint<T>;

// Concept 版本：约束整型
template<Integral T>
T concept_add(T a, T b) {
    return a + b;
}

// Concept 版本：约束数值类型
template<Numeric T>
std::string concept_type_name() {
    if constexpr (Integral<T>) return "整型";
    else return "浮点型";
}

// 定义 Concept：容器
template<typename T>
concept Container = requires(T t) {
    { t.begin() } -> std::input_or_output_iterator;
    { t.end() } -> std::input_or_output_iterator;
    typename T::value_type;
};

// Concept 版本：约束容器
template<Container C>
void concept_print_container(const C& c) {
    for (const auto& e : c) std::cout << e << " ";
    std::cout << "\n";
}

void demo_concepts_elegance() {
    std::cout << "=== Concepts 的优雅解决方案 ===\n";

    std::cout << "Concept 版本:\n";
    std::cout << "  concept_add(1, 2) = " << concept_add(1, 2) << "\n";
    std::cout << "  concept_type_name<int>() = " << concept_type_name<int>() << "\n";
    std::cout << "  concept_type_name<double>() = " << concept_type_name<double>() << "\n";

    std::vector<int> vec = {10, 20, 30};
    std::cout << "  concept_print_container: ";
    concept_print_container(vec);

    // concept_add("a", "b");  // 错误信息清晰：constraint not satisfied

    std::cout << "\n";
}

// ============================================================
// 3. 逐项对比：SFINAE vs Concepts
// ============================================================

// --- 对比1：函数约束 ---

// SFINAE 写法
template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
void sfinae_process(T value) {
    std::cout << "SFINAE: 处理整型 " << value << "\n";
}

// Concepts 写法（方式1：requires 子句）
template<typename T> requires std::is_integral_v<T>
void concept_process_v1(T value) {
    std::cout << "Concept v1: 处理整型 " << value << "\n";
}

// Concepts 写法（方式2：Concept 名）
template<Integral T>
void concept_process_v2(T value) {
    std::cout << "Concept v2: 处理整型 " << value << "\n";
}

// Concepts 写法（方式3：尾置 requires）
template<typename T>
void concept_process_v3(T value) requires std::is_integral_v<T> {
    std::cout << "Concept v3: 处理整型 " << value << "\n";
}

// --- 对比2：多条件约束 ---

// SFINAE：多条件组合（非常冗长）
template<typename T, std::enable_if_t<
    std::is_integral_v<T> && !std::is_same_v<T, bool>, int> = 0>
void sfinae_non_bool_int(T value) {
    std::cout << "SFINAE: 非布尔整型 " << value << "\n";
}

// Concepts：多条件组合（简洁直观）
template<typename T> requires Integral<T> && (!std::same_as<T, bool>)
void concept_non_bool_int(T value) {
    std::cout << "Concept: 非布尔整型 " << value << "\n";
}

// --- 对比3：检测成员 ---

// SFINAE：检测 to_string() 方法
template<typename T, typename = void>
struct sfinae_has_to_string : std::false_type {};

template<typename T>
struct sfinae_has_to_string<T, std::void_t<decltype(std::declval<T>().to_string())>> : std::true_type {};

// Concepts：检测 to_string() 方法
template<typename T>
concept HasToString = requires(T t) {
    { t.to_string() } -> std::convertible_to<std::string>;
};

// --- 对比4：返回类型约束 ---

// SFINAE：约束返回类型
template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
auto sfinae_double_it(T v) -> T {
    return v * 2;
}

// Concepts：约束返回类型
template<typename T> requires std::is_integral_v<T>
auto concept_double_it(T v) -> T {
    return v * 2;
}

void demo_comparison() {
    std::cout << "=== 逐项对比 ===\n";

    std::cout << "1. 函数约束:\n";
    sfinae_process(42);
    concept_process_v1(42);
    concept_process_v2(42);
    concept_process_v3(42);

    std::cout << "\n2. 多条件约束:\n";
    sfinae_non_bool_int(42);
    concept_non_bool_int(42);

    std::cout << "\n3. 检测成员:\n";
    struct WithToString {
        std::string to_string() const { return "hello"; }
    };
    std::cout << "  SFINAE has_to_string: " << sfinae_has_to_string<WithToString>::value << "\n";
    std::cout << "  Concept HasToString: " << HasToString<WithToString> << "\n";

    std::cout << "\n4. 返回类型约束:\n";
    std::cout << "  sfinae_double_it(21) = " << sfinae_double_it(21) << "\n";
    std::cout << "  concept_double_it(21) = " << concept_double_it(21) << "\n";

    std::cout << "\n";
}

// ============================================================
// 4. 错误信息对比
// ============================================================

// SFINAE 的错误信息通常是这样的：
// "no matching function for call to 'xxx'"
// "template argument deduction/substitution failed"
// "candidate: template<class T, ...> ... [with T = ...]"
// 然后列出所有被排除的候选，非常冗长

// Concepts 的错误信息通常是这样的：
// "constraint not satisfied"
// "requirement std::is_integral_v<T> was not satisfied [with T = const char*]"
// 直接告诉你哪个约束不满足

void demo_error_messages() {
    std::cout << "=== 错误信息对比 ===\n";

    std::cout << "SFINAE 错误信息特点:\n";
    std::cout << "  - 'no matching function for call to ...'\n";
    std::cout << "  - 列出所有被排除的候选函数\n";
    std::cout << "  - 可能数百行模板实例化堆栈\n";
    std::cout << "  - 很难定位真正的原因\n\n";

    std::cout << "Concepts 错误信息特点:\n";
    std::cout << "  - 'constraint not satisfied'\n";
    std::cout << "  - 明确指出哪个约束失败\n";
    std::cout << "  - 错误信息简洁明了\n";
    std::cout << "  - 容易定位问题\n\n";

    std::cout << "示例：调用 concept_add(\"a\", \"b\") 时\n";
    std::cout << "  Concepts 错误: constraint 'Integral<const char*>' not satisfied\n";
    std::cout << "  SFINAE 错误: 需要翻阅大量模板实例化信息\n";

    std::cout << "\n";
}

// ============================================================
// 5. 迁移指南：SFINAE → Concepts
// ============================================================

// 迁移模式1：enable_if → requires 子句
// SFINAE:
//   template<typename T, enable_if_t<condition, int> = 0>
//   void f(T);
// Concepts:
//   template<typename T> requires condition
//   void f(T);

// 迁移模式2：void_t 检测 → requires 表达式
// SFINAE:
//   template<typename T, typename = void>
//   struct has_x : false_type {};
//   template<typename T>
//   struct has_x<T, void_t<decltype(T::x)>> : true_type {};
// Concepts:
//   template<typename T>
//   concept HasX = requires { T::x; };

// 迁移模式3：返回类型 enable_if → requires
// SFINAE:
//   template<typename T>
//   auto f(T) -> enable_if_t<cond, Ret>;
// Concepts:
//   template<typename T> requires cond
//   auto f(T) -> Ret;

// 实际迁移示例
// SFINAE 版本
template<typename T, std::enable_if_t<std::is_integral_v<T>, int> = 0>
constexpr T sfinae_abs(T n) {
    return n < 0 ? -n : n;
}

// Concepts 版本
template<std::integral T>
constexpr T concept_abs(T n) {
    return n < 0 ? -n : n;
}

// SFINAE 版本：通用 advance
template<typename Iter, typename Distance>
std::enable_if_t<std::is_base_of_v<std::random_access_iterator_tag,
    typename std::iterator_traits<Iter>::iterator_category>>
sfinae_advance(Iter& it, Distance n) {
    it += n;
}

template<typename Iter, typename Distance>
std::enable_if_t<!std::is_base_of_v<std::random_access_iterator_tag,
    typename std::iterator_traits<Iter>::iterator_category>>
sfinae_advance(Iter& it, Distance n) {
    for (Distance i = 0; i < n; ++i) ++it;
}

// Concepts 版本：通用 advance
template<typename Iter, typename Distance>
    requires std::random_access_iterator<Iter>
void concept_advance(Iter& it, Distance n) {
    it += n;
}

template<typename Iter, typename Distance>
    requires std::input_iterator<Iter> && (!std::random_access_iterator<Iter>)
void concept_advance(Iter& it, Distance n) {
    for (Distance i = 0; i < n; ++i) ++it;
}

void demo_migration() {
    std::cout << "=== 迁移指南 ===\n";

    std::cout << "迁移模式:\n";
    std::cout << "  1. enable_if → requires 子句\n";
    std::cout << "  2. void_t 检测 → requires 表达式\n";
    std::cout << "  3. 返回类型 enable_if → requires\n\n";

    std::cout << "实际效果对比:\n";
    std::cout << "  sfinae_abs(-5) = " << sfinae_abs(-5) << "\n";
    std::cout << "  concept_abs(-5) = " << concept_abs(-5) << "\n";

    std::vector<int> v = {1, 2, 3, 4, 5};
    auto it = v.begin();
    concept_advance(it, 3);
    std::cout << "  concept_advance 后 *it = " << *it << "\n";

    std::cout << "\n";
}

// ============================================================
// 6. Concepts 独有的强大特性
// ============================================================

// 特性1：subsumption（包含关系）
// 更严格的 concept 优先匹配
template<typename T>
concept Addable = requires(T a, T b) { a + b; };

template<typename T>
concept NumericAddable = Numeric<T> && Addable<T>;

// 当 T 同时满足 Addable 和 NumericAddable 时，
// NumericAddable 更严格，优先匹配
template<Addable T>
std::string describe(T) { return "可加的"; }

template<NumericAddable T>
std::string describe(T) { return "数值可加的"; }

// 特性2：requires 表达式可以检测多个约束
template<typename T>
concept SortableContainer = requires(T t) {
    { t.begin() } -> std::random_access_iterator;
    { t.end() } -> std::random_access_iterator;
    { t.size() } -> std::convertible_to<std::size_t>;
    requires std::totally_ordered<typename T::value_type>;
};

// 特性3：嵌套 requires
template<typename T>
concept Nestable = requires {
    typename T::value_type;
    requires SortableContainer<T>;
};

void demo_concepts_unique_features() {
    std::cout << "=== Concepts 独有特性 ===\n";

    // subsumption: NumericAddable 更严格，优先匹配
    std::cout << "subsumption (包含关系):\n";
    std::cout << "  describe(42): " << describe(42) << "\n";     // "数值可加的"
    std::cout << "  describe(std::string(\"a\")): " << describe(std::string("a")) << "\n";  // "可加的"

    // requires 表达式
    std::cout << "\nSortableContainer:\n";
    std::cout << "  std::vector<int>: " << SortableContainer<std::vector<int>> << "\n";
    // std::list 不满足 random_access_iterator

    std::cout << "\nConcepts 独有优势:\n";
    std::cout << "  1. subsumption: 更严格的约束优先匹配\n";
    std::cout << "  2. requires 表达式: 简洁地检测多个约束\n";
    std::cout << "  3. 嵌套 requires: 约束可以组合\n";
    std::cout << "  4. 标准库 concept: integral, floating_point, same_as 等\n";

    std::cout << "\n";
}

// ============================================================
// 7. 何时仍需 SFINAE
// ============================================================

void demo_when_sfinae_still_needed() {
    std::cout << "=== 何时仍需 SFINAE ===\n";

    std::cout << "Concepts 不能完全替代 SFINAE 的场景:\n\n";

    std::cout << "1. 需要兼容 C++17 及更早版本\n";
    std::cout << "   - 旧代码库无法升级\n";
    std::cout << "   - 需要跨版本兼容\n\n";

    std::cout << "2. 类模板偏特化中的条件选择\n";
    std::cout << "   - Concepts 不能直接用于偏特化\n";
    std::cout << "   - 仍需 void_t + 偏特化模式\n\n";

    std::cout << "3. 复杂的类型计算\n";
    std::cout << "   - 某些类型变换仍需 enable_if 辅助\n\n";

    std::cout << "4. 第三方库接口\n";
    std::cout << "   - 库可能只提供 SFINAE 风格的接口\n\n";

    std::cout << "最佳实践:\n";
    std::cout << "  - 新代码优先使用 Concepts\n";
    std::cout << "  - 旧代码逐步迁移\n";
    std::cout << "  - 偏特化场景保留 SFINAE\n";

    std::cout << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "  SFINAE vs Concepts 对比\n";
    std::cout << "============================================\n\n";

    demo_sfinae_pain_points();
    demo_concepts_elegance();
    demo_comparison();
    demo_error_messages();
    demo_migration();
    demo_concepts_unique_features();
    demo_when_sfinae_still_needed();

    std::cout << "============================================\n";
    std::cout << "  核心要点总结:\n";
    std::cout << "  1. SFINAE: 功能强大但语法冗长\n";
    std::cout << "  2. Concepts: 语法简洁、错误信息友好\n";
    std::cout << "  3. 新代码优先使用 Concepts\n";
    std::cout << "  4. 偏特化场景仍需 SFINAE\n";
    std::cout << "  5. Concepts 支持 subsumption 优先级\n";
    std::cout << "============================================\n";

    return 0;
}
