/** @file 02_example_decltype.cpp
 *  @brief decltype规则：decltype vs auto、尾置返回类型、decltype与表达式
 *  @description 对应文档: 12-类型推导
 */

#include <iostream>
#include <string>
#include <vector>
#include <type_traits>

void demo_decltype_basics() {
    std::cout << "=== decltype 基础规则 ===\n";

    int x = 10;
    const int cx = 20;
    int& rx = x;
    int* px = &x;

    std::cout << "decltype(x)  = int\n";
    std::cout << "decltype(cx) = const int\n";
    std::cout << "decltype(rx) = int&\n";
    std::cout << "decltype(px) = int*\n";

    std::cout << "\ndecltype 的规则:\n";
    std::cout << "  1. decltype(变量名) => 变量声明的精确类型\n";
    std::cout << "  2. decltype(表达式) => 表达式的值类别决定结果\n";
    std::cout << "     - 左值表达式 => T&\n";
    std::cout << "     - 右值表达式 => T\n";
    std::cout << "     - 将亡值表达式 => T&&\n";

    std::cout << "\n";
}

int global_x = 42;

void demo_decltype_expression() {
    std::cout << "=== decltype 与表达式 ===\n";

    std::cout << "decltype(变量名) vs decltype((变量名)):\n";
    std::cout << "  decltype(x)   => int  (变量名, 直接得到声明类型)\n";
    std::cout << "  decltype((x)) => int& (带括号, 变成左值表达式)\n\n";

    auto f = []() -> int { return 42; };
    auto g = []() -> int& { return global_x; };

    std::cout << "decltype(f())  => int   (f() 返回右值)\n";
    std::cout << "decltype(g())  => int&  (g() 返回左值引用)\n";

    int arr[5] = {1, 2, 3, 4, 5};
    std::cout << "decltype(arr)    => int[5]\n";
    std::cout << "decltype(arr[0]) => int& (下标返回左值引用)\n";

    std::cout << "\n";
}

void demo_decltype_vs_auto() {
    std::cout << "=== decltype vs auto ===\n";

    const int ci = 42;
    const int& cir = ci;

    auto a = ci;
    decltype(ci) b = ci;
    std::cout << "auto a = ci;       => int (忽略顶层const)\n";
    std::cout << "decltype(ci) b = ci; => const int (保留const)\n\n";

    auto c = cir;
    decltype(cir) d = ci;
    std::cout << "auto c = cir;       => int (忽略const和引用)\n";
    std::cout << "decltype(cir) d = ci; => const int& (保留const和引用)\n\n";

    int x = 10;
    int& ref = x;
    auto e = ref;
    decltype(ref) f = x;
    std::cout << "auto e = ref;      => int (忽略引用)\n";
    std::cout << "decltype(ref) f = x; => int& (保留引用)\n";

    std::cout << "\n关键区别:\n";
    std::cout << "  auto: 使用模板推导规则, 忽略顶层const和引用\n";
    std::cout << "  decltype: 保留完整的类型信息\n";

    std::cout << "\n";
}

template<typename Container>
auto get_element(Container& c, size_t index) -> decltype(c[index]) {
    return c[index];
}

template<typename Container>
auto safe_get(Container& c, size_t index) -> decltype(c.at(index)) {
    return c.at(index);
}

void demo_trailing_return_type() {
    std::cout << "=== 尾置返回类型 ===\n";

    std::vector<int> vec = {10, 20, 30, 40, 50};
    std::cout << "get_element(vec, 2) = " << get_element(vec, 2) << "\n";

    get_element(vec, 2) = 999;
    std::cout << "修改后 vec[2] = " << vec[2] << "\n";

    std::cout << "\n尾置返回类型的语法:\n";
    std::cout << "  auto func(Args...) -> decltype(expr) { ... }\n";
    std::cout << "  先声明参数, 再用 decltype 确定返回类型\n\n";

    std::cout << "C++14 简化:\n";
    std::cout << "  auto func(Args...) { return expr; }  // 自动推导返回类型\n";
    std::cout << "  decltype(auto) func(Args...) { return expr; }  // 保留引用\n";

    std::cout << "\n";
}

template<typename T>
decltype(auto) forward_value(T& x) {
    return (x);
}

template<typename T>
decltype(auto) forward_value_move(T&& x) {
    return std::forward<T>(x);
}

void demo_decltype_auto_function() {
    std::cout << "=== decltype(auto) 作为返回类型 ===\n";

    int value = 42;

    auto get_copy = [&]() -> auto { return value; };
    decltype(auto) get_ref = [&]() -> decltype(auto) { return (value); };

    int copy = get_copy();
    int& ref = get_ref();
    ref = 100;
    std::cout << "get_copy() 返回副本: " << copy << "\n";
    std::cout << "get_ref() 返回引用, 修改后 value = " << value << "\n";

    std::cout << "\ndecltype(auto) 的返回类型推导:\n";
    std::cout << "  return x;   => decltype(x) => T (非引用)\n";
    std::cout << "  return (x); => decltype((x)) => T& (左值引用)\n";
    std::cout << "  注意: return (x) 可能意外返回引用!\n";

    std::cout << "\n";
}

void demo_decltype_practical() {
    std::cout << "=== decltype 实用场景 ===\n";

    std::cout << "1. 声明与表达式类型一致的变量:\n";
    {
        std::vector<int> vec = {1, 2, 3};
        decltype(vec.size()) sz = vec.size();
        std::cout << "   decltype(vec.size()) sz = " << sz << "\n";
    }

    std::cout << "\n2. 模板编程中保持表达式类型:\n";
    {
        auto add = [](auto a, auto b) -> decltype(a + b) {
            return a + b;
        };
        std::cout << "   add(1, 2.5) = " << add(1, 2.5) << "\n";
    }

    std::cout << "\n3. 与 typedef/using 配合:\n";
    {
        std::vector<std::string> names = {"Alice", "Bob"};
        using SizeType = decltype(names)::size_type;
        SizeType s = names.size();
        std::cout << "   using SizeType = decltype(names)::size_type\n";
        std::cout << "   SizeType s = " << s << "\n";
    }

    std::cout << "\n";
}

int main() {
    demo_decltype_basics();
    demo_decltype_expression();
    demo_decltype_vs_auto();
    demo_trailing_return_type();
    demo_decltype_auto_function();
    demo_decltype_practical();

    return 0;
}
