/** @file 01_example_variadic_template.cpp
 *  @brief 变参模板：参数包、展开、递归模板、折叠表达式(C++17)
 *  @description 对应文档: 11-模板进阶
 */

#include <iostream>
#include <string>
#include <memory>
#include <vector>

template<typename... Args>
void print_count() {
    std::cout << "参数数量: " << sizeof...(Args) << "\n";
}

template<typename T>
void print(T value) {
    std::cout << value << "\n";
}

template<typename T, typename... Args>
void print(T first, Args... rest) {
    std::cout << first << ", ";
    print(rest...);
}

template<typename... Args>
auto sum(Args... args) {
    return (args + ...);
}

template<typename... Args>
auto product(Args... args) {
    return (... * args);
}

template<typename... Args>
bool all_true(Args... args) {
    return (... && args);
}

template<typename... Args>
bool any_true(Args... args) {
    return (... || args);
}

template<typename... Args>
void print_with_separator(const std::string& sep, Args... args) {
    std::cout << "[";
    bool first = true;
    ((std::cout << (first ? (first = false, "") : sep) << args), ...);
    std::cout << "]\n";
}

template<typename... Args>
void print_lines(Args... args) {
    ((std::cout << "- " << args << "\n"), ...);
}

template<typename T, typename... Args>
std::unique_ptr<T> make_unique_custom(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

struct Product {
    std::string name;
    double price;
    int quantity;
    Product(std::string n, double p, int q)
        : name(std::move(n)), price(p), quantity(q) {
        std::cout << "Product(\"" << name << "\", " << price << ", " << quantity << ")\n";
    }
};

template<typename Container, typename... Args>
void emplace_multiple(Container& c, Args&&... args) {
    (c.emplace_back(std::forward<Args>(args)), ...);
}

void demo_basic_parameter_pack() {
    std::cout << "=== 参数包基础 ===\n";

    print_count<>();
    print_count<int>();
    print_count<int, double>();
    print_count<int, double, std::string>();

    std::cout << "\n参数包的语法:\n";
    std::cout << "  typename... Args  - 声明类型参数包\n";
    std::cout << "  Args... args      - 声明函数参数包\n";
    std::cout << "  sizeof...(Args)   - 获取参数数量 (编译期)\n";

    std::cout << "\n";
}

void demo_recursive_template() {
    std::cout << "=== 递归模板展开 ===\n";

    print(1);
    print(1, 2.5);
    print(1, 2.5, "hello");
    print(1, 2.5, "hello", 'X');

    std::cout << "\n递归展开的原理:\n";
    std::cout << "  1. 定义终止条件 (单参数版本)\n";
    std::cout << "  2. 定义递归版本 (取第一个, 递归处理剩余)\n";
    std::cout << "  3. 编译器在编译期展开所有调用\n";

    std::cout << "\n";
}

void demo_fold_expressions() {
    std::cout << "=== 折叠表达式 (C++17) ===\n";

    std::cout << "sum(1, 2, 3, 4, 5) = " << sum(1, 2, 3, 4, 5) << "\n";
    std::cout << "sum(1.5, 2.5, 3.5) = " << sum(1.5, 2.5, 3.5) << "\n";
    std::cout << "product(1, 2, 3, 4) = " << product(1, 2, 3, 4) << "\n";
    std::cout << "all_true(true, true, true) = " << (all_true(true, true, true) ? "true" : "false") << "\n";
    std::cout << "all_true(true, false, true) = " << (all_true(true, false, true) ? "true" : "false") << "\n";
    std::cout << "any_true(false, false, true) = " << (any_true(false, false, true) ? "true" : "false") << "\n";

    std::cout << "\n折叠表达式的四种形式:\n";
    std::cout << "  一元右折叠: (pack op ...)     => a1 op (a2 op (... op aN))\n";
    std::cout << "  一元左折叠: (... op pack)     => ((a1 op a2) op ...) op aN\n";
    std::cout << "  二元右折叠: (pack op ... op init)\n";
    std::cout << "  二元左折叠: (init op ... op pack)\n";

    std::cout << "\n";
}

void demo_comma_fold() {
    std::cout << "=== 逗号折叠表达式 ===\n";

    print_with_separator(", ", 1, 2, 3, 4, 5);
    print_with_separator(" | ", "Hello", "World", "C++17");
    print_lines("苹果", "香蕉", "橙子");

    std::cout << "\n逗号折叠的用途:\n";
    std::cout << "  1. 对每个参数执行操作 (不关心返回值)\n";
    std::cout << "  2. 逐个打印参数\n";
    std::cout << "  3. 逐个调用函数\n";

    std::cout << "\n";
}

void demo_variadic_forwarding() {
    std::cout << "=== 变参完美转发 ===\n";

    auto p = make_unique_custom<Product>("笔记本电脑", 5999.0, 2);

    std::cout << "\n变参转发的组合:\n";
    std::cout << "  typename... Args     - 变参类型包\n";
    std::cout << "  Args&&... args       - 万能引用参数包\n";
    std::cout << "  std::forward<Args>(args)... - 逐个完美转发\n";
    std::cout << "  这就是 std::make_unique 的实现原理\n";

    std::cout << "\n";
}

void demo_variadic_emplace() {
    std::cout << "=== 变参 emplace ===\n";

    std::vector<int> vec;
    emplace_multiple(vec, 10, 20, 30, 40, 50);
    std::cout << "vector 内容: ";
    for (const auto& v : vec) std::cout << v << " ";
    std::cout << "\n";

    std::vector<std::string> svec;
    emplace_multiple(svec, "Hello", std::string("World"), "C++17");
    std::cout << "string vector 内容: ";
    for (const auto& v : svec) std::cout << v << " ";
    std::cout << "\n";

    std::cout << "\n";
}

int main() {
    demo_basic_parameter_pack();
    demo_recursive_template();
    demo_fold_expressions();
    demo_comma_fold();
    demo_variadic_forwarding();
    demo_variadic_emplace();

    return 0;
}
