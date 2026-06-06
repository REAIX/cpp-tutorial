/** @file 01_deep_dive_template_deduction.cpp
 *  @brief 模板推导规则：推导上下文、SFINAE简介、enable_if基础
 *  @description 对应文档: 10-模板基础 | 举一反三：理解模板类型推导的深层机制
 */

#include <iostream>
#include <string>
#include <type_traits>
#include <vector>
#include <limits>

template<typename T>
void deduce_value(T param) {
    std::cout << "T = ";
    if constexpr (std::is_const_v<T>) std::cout << "const ";
    if constexpr (std::is_pointer_v<T>) std::cout << "pointer ";
    if constexpr (std::is_reference_v<T>) std::cout << "reference ";
    if constexpr (std::is_integral_v<T>) std::cout << "integral";
    else if constexpr (std::is_floating_point_v<T>) std::cout << "floating";
    else std::cout << "other";
    std::cout << "\n";
}

template<typename T>
void deduce_ref(T& param) {
    std::cout << "T& 推导: T = ";
    if constexpr (std::is_const_v<T>) std::cout << "const ";
    if constexpr (std::is_integral_v<T>) std::cout << "integral";
    else std::cout << "other";
    std::cout << "\n";
}

template<typename T>
void deduce_const_ref(const T& param) {
    std::cout << "const T& 推导: T = ";
    if constexpr (std::is_integral_v<T>) std::cout << "integral";
    else std::cout << "other";
    std::cout << "\n";
}

template<typename T>
void deduce_universal_ref(T&& param) {
    std::cout << "T&& 推导: T = ";
    if constexpr (std::is_lvalue_reference_v<T>) std::cout << "左值引用 ";
    else std::cout << "非引用(右值) ";
    if constexpr (std::is_const_v<std::remove_reference_t<T>>) std::cout << "const ";
    if constexpr (std::is_integral_v<std::remove_reference_t<T>>) std::cout << "integral";
    else std::cout << "other";
    std::cout << "\n";
}

template<typename T>
void show_type() {
    if constexpr (std::is_integral_v<T>) {
        std::cout << "整型\n";
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "浮点型\n";
    } else {
        std::cout << "其他类型\n";
    }
}

template<typename T>
typename std::enable_if<std::is_integral_v<T>, T>::type
safe_divide(T a, T b) {
    if (b == 0) {
        std::cout << "错误: 除以零! 返回0\n";
        return T(0);
    }
    return a / b;
}

template<typename T>
typename std::enable_if<std::is_floating_point_v<T>, T>::type
safe_divide(T a, T b) {
    if (b == 0.0) {
        std::cout << "警告: 浮点除以零!\n";
        return std::numeric_limits<T>::infinity();
    }
    return a / b;
}

template<typename T>
auto get_size_sfinae(const T& c, int)
    -> decltype(c.size(), size_t{}) {
    std::cout << "使用 .size() 方法: ";
    return c.size();
}

template<typename T>
auto get_size_sfinae(const T& c, long) {
    std::cout << "使用 sizeof 计算: ";
    return sizeof(c) / sizeof(c[0]);
}

void demo_deduction_rules() {
    std::cout << "=== 模板推导三大规则 ===\n";

    int x = 10;
    const int cx = 20;
    const int& rx = cx;

    std::cout << "--- 规则1: T param (按值传递) ---\n";
    deduce_value(x);
    deduce_value(cx);
    deduce_value(rx);
    std::cout << "  按值传递: 忽略 const 和引用, T 总是推导为非引用非const类型\n\n";

    std::cout << "--- 规则2: T& param (引用传递) ---\n";
    deduce_ref(x);
    deduce_ref(cx);
    std::cout << "  引用传递: 保留 const, T 推导为类型本身(不含引用)\n\n";

    std::cout << "--- 规则3: const T& param ---\n";
    deduce_const_ref(x);
    deduce_const_ref(cx);
    deduce_const_ref(42);
    std::cout << "  const引用: 可接受左值和右值\n\n";

    std::cout << "--- 规则4: T&& param (万能引用) ---\n";
    deduce_universal_ref(x);
    deduce_universal_ref(cx);
    deduce_universal_ref(42);
    std::cout << "  万能引用: 左值推导为左值引用, 右值推导为非引用\n";

    std::cout << "\n";
}

void demo_deduction_contexts() {
    std::cout << "=== 推导上下文 ===\n";

    std::cout << "可推导的上下文:\n";
    std::cout << "  T, T&, T&&, T*, const T, std::vector<T> 等\n\n";

    std::cout << "不可推导的上下文 (非推导上下文):\n";
    std::cout << "  1. typename T::type  (嵌套类型名)\n";
    std::cout << "  2. T(T::*)()  (成员指针)\n";
    std::cout << "  3. 非类型参数的复杂表达式\n\n";

    show_type<int>();
    show_type<double>();
    show_type<std::string>();

    std::cout << "\n";
}

void demo_enable_if_basics() {
    std::cout << "=== enable_if 基础 ===\n";

    std::cout << "safe_divide(10, 3) = " << safe_divide(10, 3) << "\n";
    std::cout << "safe_divide(10, 0) = ";
    auto result = safe_divide(10, 0);
    std::cout << result << "\n";

    std::cout << "safe_divide(10.0, 3.0) = " << safe_divide(10.0, 3.0) << "\n";

    std::cout << "\nenable_if 的原理:\n";
    std::cout << "  template<bool Cond, typename T = void>\n";
    std::cout << "  struct enable_if {};\n\n";
    std::cout << "  template<typename T>\n";
    std::cout << "  struct enable_if<true, T> { using type = T; };\n\n";
    std::cout << "  Cond 为 true 时, 有 type 成员\n";
    std::cout << "  Cond 为 false 时, 没有 type 成员 => SFINAE\n";

    std::cout << "\n";
}

void demo_enable_if_styles() {
    std::cout << "=== enable_if 的多种写法 ===\n";

    std::cout << "写法1: 返回类型\n";
    std::cout << "  typename enable_if<cond, T>::type func(T a)\n\n";

    std::cout << "写法2: 模板参数默认值\n";
    std::cout << "  template<typename T, typename = enable_if_t<cond>>\n";
    std::cout << "  T func(T a)\n\n";

    std::cout << "写法3: 函数参数 (不推荐, 改变接口)\n";
    std::cout << "  void func(T a, enable_if_t<cond, int>* = nullptr)\n\n";

    std::cout << "C++17 简化: enable_if_t 代替 typename enable_if<...>::type\n";

    std::cout << "\n";
}

void demo_sfinae_introduction() {
    std::cout << "=== SFINAE 简介 ===\n";

    std::cout << "SFINAE: Substitution Failure Is Not An Error\n";
    std::cout << "替换失败不是错误\n\n";

    std::vector<int> vec = {1, 2, 3};
    int arr[] = {10, 20, 30, 40};

    std::cout << "vector size = " << get_size_sfinae(vec, 0) << "\n";
    std::cout << "array size = " << get_size_sfinae(arr, 0) << "\n";

    std::cout << "\nSFINAE 的应用场景:\n";
    std::cout << "  1. 条件性启用/禁用函数重载\n";
    std::cout << "  2. 类型特征 (type traits)\n";
    std::cout << "  3. 编译期接口检测\n";
    std::cout << "  4. C++20 concepts 的前身\n";

    std::cout << "\n";
}

int main() {
    demo_deduction_rules();
    demo_deduction_contexts();
    demo_enable_if_basics();
    demo_enable_if_styles();
    demo_sfinae_introduction();

    return 0;
}
