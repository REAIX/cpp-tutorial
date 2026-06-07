/** @file 01_example_type_traits.cpp
 *  @brief 标准类型特征：is_integral、is_pointer、remove_reference、conditional_t等
 *  @description 对应文档: 07-模板元编程与编译期计算 / Type-Traits与类型操作
 */

#include <iostream>
#include <type_traits>
#include <string>
#include <vector>
#include <memory>

// ============================================================
// 1. 类型检查 Traits（一元类型谓词）
// ============================================================

void demo_type_check_traits() {
    std::cout << "=== 类型检查 Traits ===\n";

    // is_integral: 是否为整型
    std::cout << "is_integral:\n";
    std::cout << "  int:     " << std::is_integral_v<int>     << "\n";
    std::cout << "  double:  " << std::is_integral_v<double>  << "\n";
    std::cout << "  bool:    " << std::is_integral_v<bool>    << "\n";
    std::cout << "  char:    " << std::is_integral_v<char>    << "\n";

    // is_floating_point: 是否为浮点型
    std::cout << "\nis_floating_point:\n";
    std::cout << "  float:   " << std::is_floating_point_v<float>   << "\n";
    std::cout << "  double:  " << std::is_floating_point_v<double>  << "\n";
    std::cout << "  int:     " << std::is_floating_point_v<int>     << "\n";

    // is_pointer: 是否为指针
    std::cout << "\nis_pointer:\n";
    std::cout << "  int*:       " << std::is_pointer_v<int*>       << "\n";
    std::cout << "  int:        " << std::is_pointer_v<int>        << "\n";
    std::cout << "  int**:      " << std::is_pointer_v<int**>      << "\n";
    std::cout << "  std::string*: " << std::is_pointer_v<std::string*> << "\n";

    // is_reference: 是否为引用
    std::cout << "\nis_reference:\n";
    std::cout << "  int&:    " << std::is_reference_v<int&>    << "\n";
    std::cout << "  int&&:   " << std::is_lvalue_reference_v<int&>   << " (左值引用)\n";
    std::cout << "  int&&:   " << std::is_rvalue_reference_v<int&&>  << " (右值引用)\n";
    std::cout << "  int:     " << std::is_reference_v<int>     << "\n";

    // is_const: 是否为 const 修饰
    std::cout << "\nis_const:\n";
    std::cout << "  const int:  " << std::is_const_v<const int>  << "\n";
    std::cout << "  int:        " << std::is_const_v<int>        << "\n";
    std::cout << "  const int&: " << std::is_const_v<const int&> << " (引用本身不是const)\n";

    // is_class / is_enum / is_union
    std::cout << "\nis_class:\n";
    std::cout << "  std::string: " << std::is_class_v<std::string> << "\n";
    std::cout << "  int:         " << std::is_class_v<int>         << "\n";

    // is_same: 两个类型是否相同
    std::cout << "\nis_same:\n";
    std::cout << "  int, int:       " << std::is_same_v<int, int>       << "\n";
    std::cout << "  int, long:      " << std::is_same_v<int, long>      << "\n";
    std::cout << "  int&, int:      " << std::is_same_v<int&, int>      << "\n";
    std::cout << "  const int, int: " << std::is_same_v<const int, int> << "\n";

    std::cout << "\n";
}

// ============================================================
// 2. 类型修改 Traits（类型变换）
// ============================================================

void demo_type_modification_traits() {
    std::cout << "=== 类型修改 Traits ===\n";

    // remove_reference: 移除引用
    std::cout << "remove_reference:\n";
    std::cout << "  int&  -> " << typeid(std::remove_reference_t<int&>).name()  << "\n";
    std::cout << "  int&& -> " << typeid(std::remove_reference_t<int&&>).name() << "\n";
    std::cout << "  int   -> " << typeid(std::remove_reference_t<int>).name()   << "\n";

    // add_reference: 添加引用
    std::cout << "\nadd_reference:\n";
    using LRef = std::add_lvalue_reference_t<int>;
    using RRef = std::add_rvalue_reference_t<int>;
    std::cout << "  int + 左值引用: " << std::is_lvalue_reference_v<LRef> << "\n";
    std::cout << "  int + 右值引用: " << std::is_rvalue_reference_v<RRef> << "\n";

    // remove_const / add_const: 移除/添加 const
    std::cout << "\nremove_const / add_const:\n";
    std::cout << "  const int 移除const: " << std::is_const_v<std::remove_const_t<const int>> << "\n";
    std::cout << "  int 添加const: " << std::is_const_v<std::add_const_t<int>> << "\n";

    // remove_pointer / add_pointer: 移除/添加指针
    std::cout << "\nremove_pointer / add_pointer:\n";
    std::cout << "  int* 移除指针: " << std::is_pointer_v<std::remove_pointer_t<int*>> << "\n";
    std::cout << "  int 添加指针: " << std::is_pointer_v<std::add_pointer_t<int>> << "\n";

    // decay: 模拟按值传递时的类型退化
    std::cout << "\ndecay (类型退化):\n";
    std::cout << "  const int& -> " << std::is_same_v<std::decay_t<const int&>, int> << " (退化为int)\n";
    std::cout << "  int[5] -> " << std::is_same_v<std::decay_t<int[5]>, int*> << " (数组退化为指针)\n";
    std::cout << "  void(int) -> " << std::is_same_v<std::decay_t<void(int)>, void(*)(int)> << " (函数退化为函数指针)\n";

    // make_signed / make_unsigned: 有符号/无符号转换
    std::cout << "\nmake_signed / make_unsigned:\n";
    std::cout << "  unsigned int -> signed: " << std::is_signed_v<std::make_signed_t<unsigned int>> << "\n";
    std::cout << "  int -> unsigned: " << std::is_unsigned_v<std::make_unsigned_t<int>> << "\n";

    std::cout << "\n";
}

// ============================================================
// 3. 条件类型选择 Traits
// ============================================================

// conditional_t: 编译期三元运算符
template<typename T>
using SafeType = std::conditional_t<
    std::is_integral_v<T>,
    long long,       // 整型用 long long 避免溢出
    double           // 浮点用 double 保持精度
>;

// 根据类型大小选择容器
template<typename T>
using ContainerType = std::conditional_t<
    sizeof(T) <= sizeof(void*),
    T,                    // 小对象直接存值
    const T&              // 大对象存引用
>;

void demo_conditional_traits() {
    std::cout << "=== 条件类型选择 Traits ===\n";

    // conditional_t: 编译期条件选择类型
    std::cout << "conditional_t:\n";
    std::cout << "  SafeType<int>    是 long long: " << std::is_same_v<SafeType<int>, long long> << "\n";
    std::cout << "  SafeType<float>  是 double:    " << std::is_same_v<SafeType<float>, double>    << "\n";
    std::cout << "  SafeType<char>   是 long long: " << std::is_same_v<SafeType<char>, long long>  << "\n";

    // enable_if_t: SFINAE 启用/禁用（简单示例）
    std::cout << "\nenable_if_t (SFINAE 基础):\n";
    std::cout << "  enable_if_t<true, int>  = int:  " << std::is_same_v<std::enable_if_t<true, int>, int>  << "\n";
    std::cout << "  enable_if_t<false, int> 会导致替换失败\n";

    // 实际应用：根据类型选择参数传递方式
    std::cout << "\nContainerType 选择:\n";
    std::cout << "  int (4字节) 直接传值: " << std::is_same_v<ContainerType<int>, int> << "\n";
    std::cout << "  std::string (大对象) 传引用: " << std::is_reference_v<ContainerType<std::string>> << "\n";

    std::cout << "\n";
}

// ============================================================
// 4. 类型关系 Traits
// ============================================================

void demo_type_relation_traits() {
    std::cout << "=== 类型关系 Traits ===\n";

    // is_base_of: 继承关系检查
    struct Base {};
    struct Derived : Base {};
    struct Unrelated {};

    std::cout << "is_base_of:\n";
    std::cout << "  Base <- Derived:    " << std::is_base_of_v<Base, Derived>    << "\n";
    std::cout << "  Base <- Unrelated:  " << std::is_base_of_v<Base, Unrelated>  << "\n";
    std::cout << "  Base <- Base:       " << std::is_base_of_v<Base, Base>       << " (自身)\n";

    // is_convertible: 可转换性检查
    std::cout << "\nis_convertible:\n";
    std::cout << "  int -> long:       " << std::is_convertible_v<int, long>       << "\n";
    std::cout << "  int -> std::string: " << std::is_convertible_v<int, std::string> << "\n";
    std::cout << "  Derived* -> Base*: " << std::is_convertible_v<Derived*, Base*> << "\n";
    std::cout << "  Base* -> Derived*: " << std::is_convertible_v<Base*, Derived*> << "\n";

    // is_constructible / is_default_constructible
    std::cout << "\nis_constructible:\n";
    std::cout << "  std::string():         " << std::is_default_constructible_v<std::string> << "\n";
    std::cout << "  std::string(\"hello\"):  " << std::is_constructible_v<std::string, const char*> << "\n";
    std::cout << "  std::unique_ptr<int>(): " << std::is_default_constructible_v<std::unique_ptr<int>> << "\n";
    std::cout << "  std::unique_ptr<int>(const std::unique_ptr<int>&): "
              << std::is_constructible_v<std::unique_ptr<int>, const std::unique_ptr<int>&> << " (不可拷贝)\n";

    std::cout << "\n";
}

// ============================================================
// 5. 实用示例：类型安全的泛型函数
// ============================================================

// 示例1：根据类型选择序列化方式
template<typename T>
std::string serialize(const T& value) {
    if constexpr (std::is_integral_v<T>) {
        return std::to_string(value);
    } else if constexpr (std::is_floating_point_v<T>) {
        return std::to_string(value);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return "\"" + value + "\"";
    } else if constexpr (std::is_same_v<T, bool>) {
        return value ? "true" : "false";
    } else {
        return "<unknown type>";
    }
}

// 示例2：安全的数值加法，防止溢出
template<typename T>
auto safe_add(T a, T b) -> std::conditional_t<
    std::is_integral_v<T>,
    long long,
    double
> {
    using ResultType = std::conditional_t<
        std::is_integral_v<T>,
        long long,
        double
    >;
    return static_cast<ResultType>(a) + static_cast<ResultType>(b);
}

// 示例3：智能参数传递（完美转发辅助）
template<typename T>
constexpr bool should_pass_by_value() {
    // 小对象 + 可平凡拷贝 → 按值传递
    // 其他 → 按 const 引用传递
    return sizeof(T) <= sizeof(void*) && std::is_trivially_copyable_v<T>;
}

template<typename T>
auto optimal_param_type(int)  // 按值传递
    -> std::enable_if_t<should_pass_by_value<T>(), void> {
    std::cout << "  按值传递 (小对象+可平凡拷贝)\n";
}

template<typename T>
auto optimal_param_type(...)  // 按 const 引用传递
    -> std::enable_if_t<!should_pass_by_value<T>(), void> {
    std::cout << "  按 const 引用传递 (大对象或非平凡拷贝)\n";
}

void demo_practical_examples() {
    std::cout << "=== 实用示例 ===\n";

    // 类型安全序列化
    std::cout << "类型安全序列化:\n";
    std::cout << "  serialize(42):       " << serialize(42) << "\n";
    std::cout << "  serialize(3.14):     " << serialize(3.14) << "\n";
    std::cout << "  serialize(std::string(\"hello\")): " << serialize(std::string("hello")) << "\n";
    std::cout << "  serialize(true):     " << serialize(true) << "\n";

    // 安全数值加法
    std::cout << "\n安全数值加法:\n";
    auto r1 = safe_add(100, 200);        // int → long long
    auto r2 = safe_add(1.5, 2.5);        // double → double
    std::cout << "  safe_add(100, 200) = " << r1 << " (类型: long long)\n";
    std::cout << "  safe_add(1.5, 2.5) = " << r2 << " (类型: double)\n";

    // 智能参数传递
    std::cout << "\n智能参数传递选择:\n";
    std::cout << "  int:";
    optimal_param_type<int>(0);
    std::cout << "  double:";
    optimal_param_type<double>(0);
    std::cout << "  std::string:";
    optimal_param_type<std::string>(0);
    std::cout << "  std::vector<int>:";
    optimal_param_type<std::vector<int>>(0);

    std::cout << "\n";
}

// ============================================================
// 6. 编译期类型信息打印辅助
// ============================================================

// 编译期获取类型名称（简化版，依赖编译器实现）
template<typename T>
constexpr const char* type_name() {
    // __FUNCSIG__ (MSVC) 或 __PRETTY_FUNCTION__ (GCC/Clang)
#if defined(_MSC_VER)
    return __FUNCSIG__;
#elif defined(__GNUC__) || defined(__clang__)
    return __PRETTY_FUNCTION__;
#else
    return "unknown";
#endif
}

void demo_type_name() {
    std::cout << "=== 编译期类型名称 ===\n";
    std::cout << "  int:          " << type_name<int>() << "\n";
    std::cout << "  const int&:   " << type_name<const int&>() << "\n";
    std::cout << "  std::string:  " << type_name<std::string>() << "\n";
    std::cout << "  int(*)(double): " << type_name<int(*)(double)>() << "\n";

    std::cout << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "  标准类型特征 (Type Traits)\n";
    std::cout << "============================================\n\n";

    demo_type_check_traits();
    demo_type_modification_traits();
    demo_conditional_traits();
    demo_type_relation_traits();
    demo_practical_examples();
    demo_type_name();

    std::cout << "============================================\n";
    std::cout << "  核心要点总结:\n";
    std::cout << "  1. 类型检查: is_integral, is_pointer, ...\n";
    std::cout << "  2. 类型修改: remove_reference, add_const, ...\n";
    std::cout << "  3. 条件选择: conditional_t, enable_if_t\n";
    std::cout << "  4. 类型关系: is_base_of, is_convertible\n";
    std::cout << "  5. _v 后缀(C++17): is_xxx_v<T> = is_xxx<T>::value\n";
    std::cout << "  6. _t 后缀(C++14): xxx_t<T> = xxx<T>::type\n";
    std::cout << "============================================\n";

    return 0;
}
