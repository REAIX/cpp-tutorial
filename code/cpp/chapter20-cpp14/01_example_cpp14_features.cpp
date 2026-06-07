/** @file 01_example_cpp14_features.cpp
 *  @brief C++14新特性：泛型lambda、返回类型推导、decltype(auto)、二进制字面量、数字分隔符、make_unique
 *  @description 对应文档: 02-CPP/21-cpp14 | 演示C++14的所有重要新特性
 *  编译命令: g++ -std=c++20 01_example_cpp14_features.cpp -o 01_example_cpp14_features
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <type_traits>
#include <utility>
#include <iomanip>

constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; i++) {
        result *= i;
    }
    return result;
}

constexpr int fibonacci(int n) {
    if (n <= 1) return n;
    int a = 0, b = 1;
    for (int i = 2; i <= n; i++) {
        int temp = a + b;
        a = b;
        b = temp;
    }
    return b;
}

void demo_generic_lambda() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  泛型 lambda (auto 参数)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto add = [](auto a, auto b) {
        return a + b;
    };

    std::cout << "add(3, 4) = " << add(3, 4) << "\n";
    std::cout << "add(1.5, 2.3) = " << add(1.5, 2.3) << "\n";
    std::cout << "add(std::string(\"Hello\"), std::string(\" World\")) = "
              << add(std::string("Hello"), std::string(" World")) << "\n\n";

    auto print = [](const auto& container) {
        for (const auto& x : container) {
            std::cout << x << " ";
        }
        std::cout << "\n";
    };

    std::vector<int> vi = {1, 2, 3, 4, 5};
    std::vector<std::string> vs = {"hello", "world"};
    std::cout << "int vector: ";
    print(vi);
    std::cout << "string vector: ";
    print(vs);

    auto transform_container = [](const auto& src, auto func) {
        using result_type = decltype(func(*src.begin()));
        std::vector<result_type> result;
        result.reserve(src.size());
        for (const auto& x : src) {
            result.push_back(func(x));
        }
        return result;
    };

    auto doubled = transform_container(vi, [](int x) { return x * 2; });
    std::cout << "\n变换后: ";
    print(doubled);

    std::cout << "\nC++11 lambda vs C++14 lambda:\n";
    std::cout << "  C++11: [](int x, int y) { return x + y; }\n";
    std::cout << "  C++14: [](auto x, auto y) { return x + y; }\n";
}

auto add_ints(int a, int b) { return a + b; }
auto add_doubles(double a, double b) { return a + b; }
auto make_vector() { return std::vector<int>{1, 2, 3, 4, 5}; }

void demo_return_type_deduction() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  返回类型推导\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "add_ints(3, 4) = " << add_ints(3, 4) << "\n";
    std::cout << "add_doubles(1.5, 2.3) = " << add_doubles(1.5, 2.3) << "\n";

    auto v = make_vector();
    std::cout << "make_vector() 大小: " << v.size() << "\n\n";

    std::cout << "decltype(auto) —— 保留精确类型:\n";
    auto x = 42;
    decltype(auto) r1 = x;
    decltype(auto) r2 = (x);

    std::cout << "  decltype(auto) r1 = x;   → int (值)\n";
    std::cout << "  decltype(auto) r2 = (x); → int& (引用!)\n";
    std::cout << "  r2 = 100; → x = " << x << "\n\n";

    std::cout << "注意事项:\n";
    std::cout << "  - 所有 return 语句必须推导为同一类型\n";
    std::cout << "  - 虚函数不能使用返回类型推导\n";
    std::cout << "  - decltype(auto) 保留引用和cv限定符\n";
}

void demo_binary_literals() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  二进制字面量与数字分隔符\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "二进制字面量 (0b 或 0B 前缀):\n";
    int bin1 = 0b1010;
    int bin2 = 0b11110000;
    int bin3 = 0b00001111;
    std::cout << "  0b1010      = " << bin1 << " (十进制)\n";
    std::cout << "  0b11110000  = " << bin2 << " (十进制)\n";
    std::cout << "  0b00001111  = " << bin3 << " (十进制)\n\n";

    std::cout << "位运算更清晰:\n";
    unsigned char flags = 0b00001111;
    unsigned char mask  = 0b10000000;
    std::cout << "  flags = 0b00001111 (" << static_cast<int>(flags) << ")\n";
    std::cout << "  mask  = 0b10000000 (" << static_cast<int>(mask) << ")\n";
    std::cout << "  flags | mask = " << static_cast<int>(flags | mask) << "\n\n";

    std::cout << "数字分隔符 (C++14, 单引号 '):\n";
    long million = 1'000'000;
    long billion = 1'000'000'000;
    long long big = 9'223'372'036'854'775'807LL;
    double pi = 3.141'592'653'589'793;
    int hex = 0xFF'FF'FF'FF;
    int binary = 0b1010'1010'0000'1111;

    std::cout << "  1'000'000       = " << million << "\n";
    std::cout << "  1'000'000'000   = " << billion << "\n";
    std::cout << "  3.141'592'653   = " << pi << "\n";
    std::cout << "  0xFF'FF'FF'FF   = " << hex << "\n";
    std::cout << "  0b1010'1010'... = " << binary << "\n\n";

    std::cout << "分隔符位置任意，仅提高可读性:\n";
    long long phone = 138'0013'8000LL;
    std::cout << "  138'0013'8000 = " << phone << "\n";
}

void demo_make_unique() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::make_unique\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto p1 = std::make_unique<int>(42);
    std::cout << "make_unique<int>(42): " << *p1 << "\n";

    auto p2 = std::make_unique<std::string>(5, 'x');
    std::cout << "make_unique<string>(5, 'x'): " << *p2 << "\n";

    auto p3 = std::make_unique<std::vector<int>>(5, 10);
    std::cout << "make_unique<vector<int>>(5, 10): ";
    for (const auto& x : *p3) std::cout << x << " ";
    std::cout << "\n\n";

    auto arr = std::make_unique<int[]>(5);
    for (int i = 0; i < 5; i++) arr[i] = i * 10;
    std::cout << "make_unique<int[]>(5): ";
    for (int i = 0; i < 5; i++) std::cout << arr[i] << " ";
    std::cout << "\n\n";

    std::cout << "为什么 C++11 没有 make_unique:\n";
    std::cout << "  C++11 有 make_shared 但遗漏了 make_unique\n";
    std::cout << "  C++14 补充了 make_unique\n\n";

    std::cout << "make_unique 的优势:\n";
    std::cout << "  1. 避免裸 new，更安全\n";
    std::cout << "  2. 异常安全: 防止函数参数求值间的内存泄漏\n";
    std::cout << "  3. 代码更简洁\n\n";

    std::cout << "异常安全示例:\n";
    std::cout << "  危险: func(unique_ptr<T>(new T), unique_ptr<U>(new U))\n";
    std::cout << "  安全: func(make_unique<T>(), make_unique<U>())\n";
    std::cout << "  如果第二个new抛异常，第一个可能泄漏\n";
}

void demo_constexpr_improvements() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  C++14 constexpr 改进\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "C++14 constexpr 允许:\n";
    std::cout << "  - 局部变量\n";
    std::cout << "  - if/else 语句\n";
    std::cout << "  - 循环(for/while)\n";
    std::cout << "  - 多条语句\n\n";

    constexpr int f10 = factorial(10);
    constexpr int fib10 = fibonacci(10);
    std::cout << "  factorial(10) = " << f10 << " (编译期计算)\n";
    std::cout << "  fibonacci(10) = " << fib10 << " (编译期计算)\n\n";

    std::cout << "C++11 constexpr 限制:\n";
    std::cout << "  - 只能有一条 return 语句\n";
    std::cout << "  - 不能有局部变量\n";
    std::cout << "  - 不能有循环(需递归替代)\n";
    std::cout << "  - 不能有 if(需三元运算符替代)\n";
}

void demo_other_cpp14_features() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  C++14 其他特性\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. std::exchange —— 替换并返回旧值:\n";
    int val = 42;
    int old = std::exchange(val, 100);
    std::cout << "  旧值: " << old << ", 新值: " << val << "\n\n";

    std::cout << "2. 编译时整数序列:\n";
    std::cout << "  std::integer_sequence / std::make_integer_sequence\n";
    std::cout << "  用于模板元编程中的参数包展开\n\n";

    std::cout << "3. std::quoted —— 带引号的字符串I/O:\n";
    std::string s = "Hello, World";
    std::cout << "  原始: " << s << "\n";
    std::cout << "  带引号: " << std::quoted(s) << "\n\n";

    std::cout << "4. chrono 字面量:\n";
    using namespace std::chrono_literals;
    auto time = 1h + 30min + 45s + 500ms;
    auto total_sec = std::chrono::duration_cast<std::chrono::seconds>(time);
    std::cout << "  1h + 30min + 45s + 500ms = " << total_sec.count() << " 秒\n\n";

    std::cout << "5. 字符串字面量后缀:\n";
    using namespace std::string_literals;
    auto s1 = "hello"s;
    std::cout << "  \"hello\"s → std::string, 长度=" << s1.size() << "\n";
}

int main() {
    demo_generic_lambda();
    demo_return_type_deduction();
    demo_binary_literals();
    demo_make_unique();
    demo_constexpr_improvements();
    demo_other_cpp14_features();
    return 0;
}
