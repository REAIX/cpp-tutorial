/** @file 02_deep_dive_concepts_design.cpp
 *  @brief 设计好的概念、概念粒度、概念命名、概念与错误信息
 *  @description 对应文档: 02-CPP/23-concepts | 举一反三：如何设计高质量的概念
 *  编译命令: g++ -std=c++20 02_deep_dive_concepts_design.cpp -o 02_deep_dive_concepts_design
 */

#include <iostream>
#include <string>
#include <vector>
#include <concepts>
#include <type_traits>
#include <algorithm>
#include <cmath>
#include <sstream>

void demo_designing_good_concepts() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  设计好的概念\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "好概念的标准:\n";
    std::cout << "  1. 语义完整性 —— 概念应表达完整的语义要求\n";
    std::cout << "  2. 不过度约束 —— 不排除合理的类型\n";
    std::cout << "  3. 不过度宽松 —— 不接受不合适的类型\n";
    std::cout << "  4. 可组合 —— 可与其他概念组合\n";
    std::cout << "  5. 有文档价值 —— 名称和约束即文档\n\n";

    std::cout << "反面示例 —— 过度约束:\n";
    std::cout << "  template<typename T>\n";
    std::cout << "  concept SortableContainer = requires(T c) {\n";
    std::cout << "    { c.begin() } -> std::random_access_iterator;  // 太严格!\n";
    std::cout << "    { c.end() } -> std::random_access_iterator;\n";
    std::cout << "    { c.size() } -> std::convertible_to<std::size_t>;  // 非必须!\n";
    std::cout << "    { c.reserve(10); };  // 非必须!\n";
    std::cout << "  };\n";
    std::cout << "  问题: list 可以排序但不满足此概念\n\n";

    std::cout << "正面示例 —— 适度约束:\n";
    std::cout << "  template<typename T>\n";
    std::cout << "  concept SortableRange = requires(T& r) {\n";
    std::cout << "    requires std::permutable<decltype(r.begin())>;\n";
    std::cout << "    { *r.begin() < *r.begin() } -> std::convertible_to<bool>;\n";
    std::cout << "  };\n";
    std::cout << "  优势: 只要求可交换和可比较\n\n";

    std::cout << "设计原则:\n";
    std::cout << "  - 基于语义而非语法\n";
    std::cout << "  - 只约束必要的操作\n";
    std::cout << "  - 参考标准库概念的命名和粒度\n";
    std::cout << "  - 考虑概念的语义集合(如语义上的\"数值\"而非语法上的\"有+\")\n";
}

template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template<typename T>
concept Subtractable = requires(T a, T b) {
    { a - b } -> std::convertible_to<T>;
};

template<typename T>
concept Multipliable = requires(T a, T b) {
    { a * b } -> std::convertible_to<T>;
};

template<typename T>
concept Divisible = requires(T a, T b) {
    { a / b } -> std::convertible_to<T>;
};

template<typename T>
concept Arithmetic = Addable<T> && Subtractable<T> && Multipliable<T> && Divisible<T>;

template<typename T>
concept Ring = Addable<T> && Subtractable<T> && Multipliable<T>;

template<typename T>
concept Group = Addable<T> && Subtractable<T>;

void demo_concept_granularity() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  概念粒度\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "细粒度概念(原子概念):\n";
    std::cout << "  Addable, Subtractable, Multipliable, Divisible\n";
    std::cout << "  每个概念只描述一个操作\n\n";

    std::cout << "组合概念(复合概念):\n";
    std::cout << "  Group = Addable && Subtractable\n";
    std::cout << "  Ring = Group && Multipliable\n";
    std::cout << "  Arithmetic = Ring && Divisible\n\n";

    static_assert(Addable<int>);
    static_assert(Arithmetic<int>);
    static_assert(Arithmetic<double>);
    static_assert(!Arithmetic<std::string>);
    static_assert(Addable<std::string>);
    static_assert(!Subtractable<std::string>);

    std::cout << "验证:\n";
    std::cout << "  Arithmetic<int>: true\n";
    std::cout << "  Arithmetic<double>: true\n";
    std::cout << "  Arithmetic<string>: false (string不可除)\n";
    std::cout << "  Addable<string>: true (string可加)\n\n";

    std::cout << "粒度选择指南:\n";
    std::cout << "  - 细粒度: 灵活，可精确约束，但概念数量多\n";
    std::cout << "  - 粗粒度: 简洁，但可能过度约束\n";
    std::cout << "  - 推荐: 先定义细粒度，再组合为粗粒度\n";
    std::cout << "  - 函数参数使用最粗粒度(只需Addable就不要求Arithmetic)\n";
}

void demo_concept_naming() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  概念命名规范\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "标准库命名模式:\n\n";

    std::cout << "1. 能力描述(形容词/名词):\n";
    std::cout << "  std::integral        —— 是整数类型\n";
    std::cout << "  std::floating_point  —— 是浮点类型\n";
    std::cout << "  std::signed_integral —— 是有符号整数\n\n";

    std::cout << "2. 操作描述(-able后缀):\n";
    std::cout << "  std::movable     —— 可移动\n";
    std::cout << "  std::copyable    —— 可拷贝\n";
    std::cout << "  std::invocable   —— 可调用\n";
    std::cout << "  std::equality_comparable —— 可等比较\n\n";

    std::cout << "3. 关系描述:\n";
    std::cout << "  std::same_as<T, U>          —— 相同类型\n";
    std::cout << "  std::derived_from<T, Base>  —— 派生关系\n";
    std::cout << "  std::convertible_to<T, U>   —— 可转换\n";
    std::cout << "  std::common_with<T, U>      —— 有公共类型\n\n";

    std::cout << "自定义概念命名建议:\n";
    std::cout << "  - 类型特征: 名词 (Integral, FloatingPoint)\n";
    std::cout << "  - 操作能力: -able后缀 (Addable, Printable)\n";
    std::cout << "  - 关系约束: 介词 (SameAs, DerivedFrom)\n";
    std::cout << "  - 领域概念: 领域术语 (Vector2D, NumericMatrix)\n";
    std::cout << "  - 避免过于通用的名称 (如 Concept, Type)\n";
}

void demo_concepts_error_messages() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  Concepts 与错误信息\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "Concepts 的最大优势之一: 更好的错误信息\n\n";

    std::cout << "SFINAE 错误信息示例:\n";
    std::cout << "  error: no matching function for call to 'sort'\n";
    std::cout << "  note: candidate template ignored: substitution failure\n";
    std::cout << "  [长篇模板替换信息...]\n\n";

    std::cout << "Concepts 错误信息示例:\n";
    std::cout << "  error: no matching function for call to 'sort'\n";
    std::cout << "  note: constraints not satisfied\n";
    std::cout << "  note: 'std::random_access_iterator<int>' was not satisfied\n\n";

    auto requires_numeric = []<Arithmetic T>(T x) -> T {
        return x * 2;
    };

    std::cout << "正确调用:\n";
    std::cout << "  requires_numeric(42) = " << requires_numeric(42) << "\n";
    std::cout << "  requires_numeric(3.14) = " << requires_numeric(3.14) << "\n\n";

    std::cout << "错误调用(编译错误信息):\n";
    std::cout << "  requires_numeric(std::string(\"hello\"))\n";
    std::cout << "  → constraint not satisfied: Arithmetic<string>\n";
    std::cout << "  → Subtractable<string> was not satisfied\n\n";

    std::cout << "改善错误信息的技巧:\n";
    std::cout << "  1. 使用有意义的概念名称\n";
    std::cout << "  2. 细粒度概念比粗粒度概念错误更精确\n";
    std::cout << "  3. 在概念中添加有意义的约束\n";
    std::cout << "  4. 避免过于复杂的 requires 表达式\n";
    std::cout << "  5. 使用嵌套requires分解复杂约束\n\n";

    std::cout << "概念诊断示例:\n";
    std::cout << "  template<typename T>\n";
    std::cout << "  concept Serializable = requires(T t, std::ostream& os) {\n";
    std::cout << "    { t.serialize(os) } -> std::same_as<void>;\n";
    std::cout << "  };\n";
    std::cout << "  Serializable<string>: false\n";
    std::cout << "  错误信息会指出缺少 serialize() 方法\n";
}

void demo_concept_design_summary() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  概念设计总结\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "DO:\n";
    std::cout << "  ✓ 基于语义设计概念，而非语法\n";
    std::cout << "  ✓ 使用标准库概念作为基础\n";
    std::cout << "  ✓ 先定义细粒度概念，再组合\n";
    std::cout << "  ✓ 用有意义的名称\n";
    std::cout << "  ✓ 用 static_assert 测试概念\n";
    std::cout << "  ✓ 概念约束放在接口最显眼的位置\n";
    std::cout << "  ✓ 文档化概念的语义要求\n\n";

    std::cout << "DON'T:\n";
    std::cout << "  ✗ 不要创建只用于一个类型的概念\n";
    std::cout << "  ✗ 不要过度约束(排除合理类型)\n";
    std::cout << "  ✗ 不要过度宽松(接受不合理类型)\n";
    std::cout << "  ✗ 不要用概念替代所有模板参数\n";
    std::cout << "  ✗ 不要创建过于复杂的概念\n";
    std::cout << "  ✗ 不要忽略概念的可组合性\n\n";

    std::cout << "概念设计流程:\n";
    std::cout << "  1. 明确语义需求(这个概念表达什么?)\n";
    std::cout << "  2. 列出必要操作(最少需要哪些操作?)\n";
    std::cout << "  3. 编写 requires 表达式\n";
    std::cout << "  4. 验证正面类型(应该满足的类型)\n";
    std::cout << "  5. 验证反面类型(不应该满足的类型)\n";
    std::cout << "  6. 检查边界情况\n";
    std::cout << "  7. 考虑与其他概念的关系\n";
    std::cout << "  8. 命名并文档化\n";
}

int main() {
    demo_designing_good_concepts();
    demo_concept_granularity();
    demo_concept_naming();
    demo_concepts_error_messages();
    demo_concept_design_summary();
    return 0;
}
