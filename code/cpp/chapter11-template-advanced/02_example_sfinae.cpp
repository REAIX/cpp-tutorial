/** @file 02_example_sfinae.cpp
 *  @brief SFINAE模式：enable_if、void_t、检测惯用法
 *  @description 对应文档: 11-模板进阶
 */

#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <iterator>

template<typename T, typename = void>
struct HasSize : std::false_type {};

template<typename T>
struct HasSize<T, std::void_t<decltype(std::declval<T>().size())>> : std::true_type {};

template<typename T, typename = void>
struct HasBeginEnd : std::false_type {};

template<typename T>
struct HasBeginEnd<T, std::void_t<
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end())
>> : std::true_type {};

template<typename T, typename = void>
struct HasOperatorPlus : std::false_type {};

template<typename T>
struct HasOperatorPlus<T, std::void_t<decltype(std::declval<T>() + std::declval<T>())>> : std::true_type {};

void demo_void_t() {
    std::cout << "=== void_t 检测惯用法 ===\n";

    std::cout << "std::vector<int> 有 size(): " << (HasSize<std::vector<int>>::value ? "是" : "否") << "\n";
    std::cout << "int 有 size(): " << (HasSize<int>::value ? "是" : "否") << "\n";
    std::cout << "std::string 有 size(): " << (HasSize<std::string>::value ? "是" : "否") << "\n";

    std::cout << "\nstd::vector<int> 有 begin/end: " << (HasBeginEnd<std::vector<int>>::value ? "是" : "否") << "\n";
    std::cout << "int 有 begin/end: " << (HasBeginEnd<int>::value ? "是" : "否") << "\n";

    std::cout << "\nint 有 operator+: " << (HasOperatorPlus<int>::value ? "是" : "否") << "\n";
    std::cout << "std::string 有 operator+: " << (HasOperatorPlus<std::string>::value ? "是" : "否") << "\n";

    std::cout << "\nvoid_t 的原理:\n";
    std::cout << "  template<typename...> using void_t = void;\n";
    std::cout << "  如果 decltype(expr) 有效, void_t 成功, 匹配特化版本 (true_type)\n";
    std::cout << "  如果 decltype(expr) 无效, 替换失败, SFINAE 退回主模板 (false_type)\n";

    std::cout << "\n";
}

template<typename T>
std::enable_if_t<std::is_integral_v<T>, T>
clamp(T value, T lo, T hi) {
    std::cout << "整型 clamp: ";
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

template<typename T>
std::enable_if_t<std::is_floating_point_v<T>, T>
clamp(T value, T lo, T hi) {
    std::cout << "浮点 clamp: ";
    if (value < lo) return lo;
    if (value > hi) return hi;
    return value;
}

void demo_enable_if_sfinae() {
    std::cout << "=== enable_if 与 SFINAE ===\n";

    std::cout << "clamp(15, 0, 10) = " << clamp(15, 0, 10) << "\n";
    std::cout << "clamp(3.14, 0.0, 5.0) = " << clamp(3.14, 0.0, 5.0) << "\n";

    std::cout << "\nenable_if 的 SFINAE 应用:\n";
    std::cout << "  条件为 true: enable_if_t 有类型定义, 函数参与重载\n";
    std::cout << "  条件为 false: enable_if_t 无类型定义, SFINAE 排除该重载\n";

    std::cout << "\n";
}

template<typename Container>
auto get_size_sfinae(const Container& c, int)
    -> decltype(c.size(), size_t{}) {
    std::cout << "使用 .size() 方法: ";
    return c.size();
}

template<typename Container>
auto get_size_sfinae(const Container& c, long) {
    std::cout << "使用 sizeof 计算: ";
    return sizeof(c) / sizeof(c[0]);
}

void demo_sfinae_overload_selection() {
    std::cout << "=== SFINAE 选择重载 ===\n";

    std::vector<int> vec = {1, 2, 3, 4, 5};
    int arr[] = {10, 20, 30};

    std::cout << "vector: " << get_size_sfinae(vec, 0) << "\n";
    std::cout << "数组: " << get_size_sfinae(arr, 0) << "\n";

    std::cout << "\n重载选择技巧:\n";
    std::cout << "  使用 int/long 参数进行优先级选择\n";
    std::cout << "  int 匹配优先于 long (更精确的匹配)\n";
    std::cout << "  如果 int 版本 SFINAE 失败, 回退到 long 版本\n";

    std::cout << "\n";
}

template<typename T, typename = void>
struct ElementType {
    using type = void;
};

template<typename T>
struct ElementType<T, std::void_t<typename T::value_type>> {
    using type = typename T::value_type;
};

void demo_detection_idiom() {
    std::cout << "=== 检测惯用法 (Detection Idiom) ===\n";

    std::cout << "vector<int> 的元素类型: "
              << typeid(ElementType<std::vector<int>>::type).name() << "\n";
    std::cout << "string 的元素类型: "
              << typeid(ElementType<std::string>::type).name() << "\n";

    std::cout << "\n检测惯用法的标准模式:\n";
    std::cout << "  template<typename T, typename = void>\n";
    std::cout << "  struct HasX : false_type {};\n\n";
    std::cout << "  template<typename T>\n";
    std::cout << "  struct HasX<T, void_t<decltype(T::x)>> : true_type {};\n\n";
    std::cout << "  检测 T 是否有成员 x\n";

    std::cout << "\n";
}

template<typename T, typename = std::enable_if_t<HasSize<T>::value>>
size_t get_container_size(const T& c) {
    return c.size();
}

template<typename T, typename = std::enable_if_t<!HasSize<T>::value>, typename = void>
size_t get_container_size(const T&) {
    return 0;
}

void demo_sfinae_practical() {
    std::cout << "=== SFINAE 实战应用 ===\n";

    std::vector<int> vec = {1, 2, 3};
    std::string str = "hello";
    int num = 42;

    std::cout << "vector 大小: " << get_container_size(vec) << "\n";
    std::cout << "string 大小: " << get_container_size(str) << "\n";
    std::cout << "int 大小: " << get_container_size(num) << "\n";

    std::cout << "\nSFINAE 的典型应用:\n";
    std::cout << "  1. 根据类型特征选择不同实现\n";
    std::cout << "  2. 检测类型是否支持某个操作\n";
    std::cout << "  3. 为不同类型的容器提供统一接口\n";
    std::cout << "  4. C++20 concepts 简化了这些模式\n";

    std::cout << "\n";
}

int main() {
    demo_void_t();
    demo_enable_if_sfinae();
    demo_sfinae_overload_selection();
    demo_detection_idiom();
    demo_sfinae_practical();

    return 0;
}
