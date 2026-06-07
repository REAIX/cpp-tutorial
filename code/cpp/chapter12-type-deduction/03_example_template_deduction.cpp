/** @file 03_example_template_deduction.cpp
 *  @brief 模板参数推导：推导规则、转发引用推导、推导指引
 *  @description 对应文档: 12-类型推导
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <type_traits>

template<typename T>
void show_deduction(T param) {
    std::cout << "T = ";
    if constexpr (std::is_const_v<T>) std::cout << "const ";
    if constexpr (std::is_pointer_v<std::remove_reference_t<T>>) std::cout << "pointer ";
    if constexpr (std::is_lvalue_reference_v<T>) std::cout << "lvalue_ref ";
    if constexpr (std::is_rvalue_reference_v<T>) std::cout << "rvalue_ref ";
    if constexpr (std::is_integral_v<std::remove_reference_t<T>>) std::cout << "integral";
    else if constexpr (std::is_floating_point_v<std::remove_reference_t<T>>) std::cout << "floating";
    else std::cout << "other";
    std::cout << "\n";
}

void demo_template_deduction_by_value() {
    std::cout << "=== 按值传递的模板推导 ===\n";

    int x = 10;
    const int cx = 20;
    const int& rx = cx;

    show_deduction(x);
    show_deduction(cx);
    show_deduction(rx);

    std::cout << "\n按值推导规则:\n";
    std::cout << "  T param = expr;\n";
    std::cout << "  1. 忽略 expr 的引用部分\n";
    std::cout << "  2. 忽略顶层 const\n";
    std::cout << "  3. 保留底层 const (如指针指向的const)\n";
    std::cout << "  4. 数组和函数退化为指针\n";

    std::cout << "\n";
}

template<typename T>
void show_ref_deduction(T& param) {
    std::cout << "T = ";
    if constexpr (std::is_const_v<T>) std::cout << "const ";
    if constexpr (std::is_integral_v<T>) std::cout << "integral";
    else std::cout << "other";
    std::cout << ", ParamType = ";
    if constexpr (std::is_const_v<T>) std::cout << "const ";
    std::cout << "T&\n";
}

void demo_template_deduction_by_ref() {
    std::cout << "=== 按引用传递的模板推导 ===\n";

    int x = 10;
    const int cx = 20;

    show_ref_deduction(x);
    show_ref_deduction(cx);
    // show_ref_deduction(42);  // 错误! 右值不能绑定到非const引用

    std::cout << "\n按引用推导规则:\n";
    std::cout << "  T& param = expr;\n";
    std::cout << "  1. 保留 const\n";
    std::cout << "  2. T 推导为不含引用的类型\n";
    std::cout << "  3. 不接受右值\n";

    std::cout << "\n";
}

template<typename T>
void show_universal_deduction(T&& param) {
    std::cout << "T = ";
    if constexpr (std::is_lvalue_reference_v<T>) std::cout << "lvalue_ref ";
    else std::cout << "non-ref ";
    if constexpr (std::is_const_v<std::remove_reference_t<T>>) std::cout << "const ";
    if constexpr (std::is_integral_v<std::remove_reference_t<T>>) std::cout << "integral";
    else std::cout << "other";
    std::cout << "\n";
}

void demo_forwarding_ref_deduction() {
    std::cout << "=== 转发引用的模板推导 ===\n";

    int x = 10;
    const int cx = 20;

    std::cout << "传入左值 x:\n  ";
    show_universal_deduction(x);

    std::cout << "传入 const 左值 cx:\n  ";
    show_universal_deduction(cx);

    std::cout << "传入右值 42:\n  ";
    show_universal_deduction(42);

    std::cout << "\n转发引用推导规则:\n";
    std::cout << "  T&& param = expr;\n";
    std::cout << "  1. 左值 => T 推导为 int&, param 类型为 int& && => int&\n";
    std::cout << "  2. 右值 => T 推导为 int, param 类型为 int&&\n";
    std::cout << "  3. const 左值 => T 推导为 const int&\n";
    std::cout << "  4. 这是引用折叠的体现\n";

    std::cout << "\n";
}

template<typename T, size_t N>
constexpr size_t array_size(T (&)[N]) {
    return N;
}

void demo_array_function_deduction() {
    std::cout << "=== 数组和函数的推导 ===\n";

    int arr[] = {1, 2, 3, 4, 5};
    const char* names[] = {"Alice", "Bob", "Charlie"};

    std::cout << "int arr[] 大小: " << array_size(arr) << "\n";
    std::cout << "names[] 大小: " << array_size(names) << "\n";

    std::cout << "\n按值传递时数组退化为指针:\n";
    std::cout << "  template<typename T>\n";
    std::cout << "  void f(T param);  // f(arr) => T = int*, 不是 int[5]\n\n";

    std::cout << "按引用传递时保留数组大小:\n";
    std::cout << "  template<typename T, size_t N>\n";
    std::cout << "  void f(T (&param)[N]);  // 保留数组大小信息\n";

    std::cout << "\n";
}

template<typename T>
struct TypeDisplay {
    TypeDisplay() {
        if constexpr (std::is_const_v<T>) std::cout << "const ";
        if constexpr (std::is_pointer_v<T>) std::cout << "pointer ";
        if constexpr (std::is_lvalue_reference_v<T>) std::cout << "lvalue_ref ";
        if constexpr (std::is_rvalue_reference_v<T>) std::cout << "rvalue_ref ";
        if constexpr (std::is_integral_v<std::remove_reference_t<T>>) std::cout << "integral";
        else if constexpr (std::is_floating_point_v<std::remove_reference_t<T>>) std::cout << "floating";
        else std::cout << "other";
        std::cout << "\n";
    }
};

void demo_deduction_comparison() {
    std::cout << "=== auto vs 模板推导对比 ===\n";

    const int x = 42;
    const int& rx = x;

    std::cout << "auto a = x;     => ";
    auto a = x; TypeDisplay<decltype(a)>();

    std::cout << "auto b = rx;    => ";
    auto b = rx; TypeDisplay<decltype(b)>();

    std::cout << "decltype(x) c = x;  => ";
    decltype(x) c = x; TypeDisplay<decltype(c)>();

    std::cout << "decltype(rx) d = x; => ";
    decltype(rx) d = x; TypeDisplay<decltype(d)>();

    std::cout << "\nauto 推导 ≈ 模板推导 (规则相同)\n";
    std::cout << "decltype 推导 ≈ 声明类型 (保留完整类型信息)\n";

    std::cout << "\n";
}

int main() {
    demo_template_deduction_by_value();
    demo_template_deduction_by_ref();
    demo_forwarding_ref_deduction();
    demo_array_function_deduction();
    demo_deduction_comparison();

    return 0;
}
