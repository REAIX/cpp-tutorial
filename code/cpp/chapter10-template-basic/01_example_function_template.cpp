/** @file 01_example_function_template.cpp
 *  @brief 函数模板基础：定义、类型推导、显式特化、重载
 *  @description 对应文档: 10-模板基础
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

template<typename T>
T my_max(T a, T b) {
    return (a > b) ? a : b;
}

template<typename T>
void swap_values(T& a, T& b) {
    T temp = std::move(a);
    a = std::move(b);
    b = std::move(temp);
}

template<typename T>
T sum(const std::vector<T>& vec) {
    T total = T();
    for (const auto& v : vec) {
        total += v;
    }
    return total;
}

void demo_function_template_basics() {
    std::cout << "=== 函数模板基础 ===\n";

    std::cout << "my_max(3, 7) = " << my_max(3, 7) << "\n";
    std::cout << "my_max(3.14, 2.71) = " << my_max(3.14, 2.71) << "\n";
    std::cout << "my_max(std::string(\"abc\"), std::string(\"xyz\")) = "
              << my_max(std::string("abc"), std::string("xyz")) << "\n";

    std::cout << "\n函数模板的定义:\n";
    std::cout << "  template<typename T>  // 声明模板参数\n";
    std::cout << "  T my_max(T a, T b) { ... }  // 使用 T 作为类型\n";

    std::cout << "\n";
}

void demo_type_deduction() {
    std::cout << "=== 模板类型推导 ===\n";

    std::cout << "自动推导:\n";
    std::cout << "  my_max(3, 7) => T 推导为 int\n";
    std::cout << "  my_max(3.14, 2.71) => T 推导为 double\n\n";

    std::cout << "显式指定模板参数:\n";
    std::cout << "  my_max<int>(3, 7) => 显式指定 T = int\n";
    std::cout << "  my_max<double>(3, 7) => 显式指定 T = double, 结果: "
              << my_max<double>(3, 7) << "\n\n";

    std::cout << "类型推导冲突:\n";
    // my_max(3, 3.14);  // 编译错误! T 不能同时是 int 和 double
    std::cout << "  my_max(3, 3.14) => 编译错误, T 推导冲突\n";
    std::cout << "  解决: my_max<double>(3, 3.14) = " << my_max<double>(3, 3.14) << "\n";

    std::cout << "\n";
}

template<typename T>
std::string to_string(T value) {
    return std::to_string(value);
}

template<>
std::string to_string<bool>(bool value) {
    return value ? "true" : "false";
}

template<>
std::string to_string<const char*>(const char* value) {
    return std::string(value);
}

void demo_explicit_specialization() {
    std::cout << "=== 显式特化 ===\n";

    std::cout << "to_string(42) = " << to_string(42) << "\n";
    std::cout << "to_string(3.14) = " << to_string(3.14) << "\n";
    std::cout << "to_string(true) = " << to_string(true) << "\n";
    std::cout << "to_string<const char*>(\"hello\") = " << to_string<const char*>("hello") << "\n";

    std::cout << "\n显式特化的语法:\n";
    std::cout << "  template<>  // 空的 template 声明\n";
    std::cout << "  string to_string<bool>(bool value) { ... }\n";
    std::cout << "  // 为 bool 类型提供专门的实现\n";

    std::cout << "\n";
}

template<typename T>
void print(const T& value) {
    std::cout << "通用版本: " << value << "\n";
}

template<typename T>
void print(T* ptr) {
    if (ptr) {
        std::cout << "指针版本: *ptr = " << *ptr << "\n";
    } else {
        std::cout << "指针版本: nullptr\n";
    }
}

void print(int value) {
    std::cout << "int 重载版本: " << value << "\n";
}

void demo_template_overloading() {
    std::cout << "=== 模板与重载 ===\n";

    double d = 3.14;
    print(42);
    print(3.14);
    print(&d);

    std::cout << "\n重载决议规则:\n";
    std::cout << "  1. 非模板函数优先于模板\n";
    std::cout << "  2. 更特化的模板优先于更通用的模板\n";
    std::cout << "  3. 指针版本比通用版本更特化\n";

    std::cout << "\n";
}

template<typename T, int N>
T array_sum(const T (&arr)[N]) {
    T total = T();
    for (int i = 0; i < N; ++i) {
        total += arr[i];
    }
    return total;
}

void demo_non_type_template_param() {
    std::cout << "=== 非类型模板参数 ===\n";

    int arr1[] = {1, 2, 3, 4, 5};
    double arr2[] = {1.1, 2.2, 3.3};

    std::cout << "array_sum(arr1) = " << array_sum(arr1) << "\n";
    std::cout << "array_sum(arr2) = " << array_sum(arr2) << "\n";

    std::cout << "\n非类型模板参数:\n";
    std::cout << "  template<typename T, int N>  // N 是非类型参数\n";
    std::cout << "  T array_sum(const T (&arr)[N])  // N 自动推导数组大小\n";

    std::cout << "\n";
}

template<typename T1, typename T2>
auto add(T1 a, T2 b) -> decltype(a + b) {
    return a + b;
}

void demo_multiple_template_params() {
    std::cout << "=== 多模板参数与 auto 返回类型 ===\n";

    std::cout << "add(3, 4) = " << add(3, 4) << "\n";
    std::cout << "add(3, 4.5) = " << add(3, 4.5) << "\n";
    std::cout << "add(3.5, 4) = " << add(3.5, 4) << "\n";

    std::cout << "\n多模板参数:\n";
    std::cout << "  template<typename T1, typename T2>\n";
    std::cout << "  auto add(T1 a, T2 b) -> decltype(a + b)\n";
    std::cout << "  返回类型由 a + b 的结果类型决定\n";

    std::cout << "\n";
}

int main() {
    demo_function_template_basics();
    demo_type_deduction();
    demo_explicit_specialization();
    demo_template_overloading();
    demo_non_type_template_param();
    demo_multiple_template_params();

    return 0;
}
