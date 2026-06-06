/** @file 01_deep_dive_custom_traits.cpp
 *  @brief 自定义类型特征：实现自定义 type traits，检测成员、SFINAE友好的trait设计
 *  @description 对应文档: 07-模板元编程与编译期计算 / Type-Traits与类型操作(深入)
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <type_traits>
#include <optional>

// ============================================================
// 1. 基础自定义 Trait：判断类型是否为容器
// ============================================================

// 方法1：使用 void_t 检测惯用法（C++17）
// void_t 的原理：如果模板参数中的表达式无效，则替换失败（SFINAE）
template<typename T, typename = void>
struct is_container : std::false_type {};

// 特化版本：当 T 有 begin(), end(), size() 时匹配
template<typename T>
struct is_container<T, std::void_t<
    decltype(std::declval<T>().begin()),
    decltype(std::declval<T>().end()),
    decltype(std::declval<T>().size())
>> : std::true_type {};

// C++17 便捷变量模板
template<typename T>
inline constexpr bool is_container_v = is_container<T>::value;

void demo_custom_container_trait() {
    std::cout << "=== 自定义 is_container Trait ===\n";

    std::cout << "std::vector<int>: " << is_container_v<std::vector<int>> << "\n";
    std::cout << "std::string:      " << is_container_v<std::string>      << "\n";
    std::cout << "std::map<int,int>:" << is_container_v<std::map<int,int>> << "\n";
    std::cout << "int:              " << is_container_v<int>              << "\n";
    std::cout << "double:           " << is_container_v<double>           << "\n";

    std::cout << "\n";
}

// ============================================================
// 2. 检测成员函数
// ============================================================

// 检测是否有 serialize() 方法
template<typename T, typename = void>
struct has_serialize : std::false_type {};

template<typename T>
struct has_serialize<T, std::void_t<
    decltype(std::declval<const T&>().serialize())
>> : std::true_type {};

template<typename T>
inline constexpr bool has_serialize_v = has_serialize<T>::value;

// 检测是否有 resize(size_t) 方法
template<typename T, typename = void>
struct has_resize : std::false_type {};

template<typename T>
struct has_resize<T, std::void_t<
    decltype(std::declval<T&>().resize(std::declval<std::size_t>()))
>> : std::true_type {};

template<typename T>
inline constexpr bool has_resize_v = has_resize<T>::value;

// 检测是否有 operator[] 
template<typename T, typename = void>
struct has_subscript : std::false_type {};

template<typename T>
struct has_subscript<T, std::void_t<
    decltype(std::declval<T&>()[std::declval<std::size_t>()])
>> : std::true_type {};

template<typename T>
inline constexpr bool has_subscript_v = has_subscript<T>::value;

// 测试用类型
struct Serializable {
    std::string serialize() const { return "serialized"; }
};

struct NotSerializable {
    int value;
};

void demo_detect_member_functions() {
    std::cout << "=== 检测成员函数 ===\n";

    std::cout << "has_serialize:\n";
    std::cout << "  Serializable:     " << has_serialize_v<Serializable>     << "\n";
    std::cout << "  NotSerializable:  " << has_serialize_v<NotSerializable>  << "\n";
    std::cout << "  std::string:      " << has_serialize_v<std::string>      << "\n";

    std::cout << "\nhas_resize:\n";
    std::cout << "  std::vector<int>: " << has_resize_v<std::vector<int>> << "\n";
    std::cout << "  std::string:      " << has_resize_v<std::string>      << "\n";
    std::cout << "  int:              " << has_resize_v<int>              << "\n";

    std::cout << "\nhas_subscript:\n";
    std::cout << "  std::vector<int>: " << has_subscript_v<std::vector<int>> << "\n";
    std::cout << "  std::map<int,int>:" << has_subscript_v<std::map<int,int>> << "\n";
    std::cout << "  int:              " << has_subscript_v<int>              << "\n";

    std::cout << "\n";
}

// ============================================================
// 3. 检测成员类型
// ============================================================

// 检测是否有 value_type
template<typename T, typename = void>
struct has_value_type : std::false_type {};

template<typename T>
struct has_value_type<T, std::void_t<typename T::value_type>> : std::true_type {};

template<typename T>
inline constexpr bool has_value_type_v = has_value_type<T>::value;

// 检测是否有 iterator 类型
template<typename T, typename = void>
struct has_iterator : std::false_type {};

template<typename T>
struct has_iterator<T, std::void_t<typename T::iterator>> : std::true_type {};

template<typename T>
inline constexpr bool has_iterator_v = has_iterator<T>::value;

// 提取 value_type（如果存在），否则返回 void
template<typename T, typename = void>
struct get_value_type { using type = void; };

template<typename T>
struct get_value_type<T, std::void_t<typename T::value_type>> {
    using type = typename T::value_type;
};

template<typename T>
using get_value_type_t = typename get_value_type<T>::type;

void demo_detect_member_types() {
    std::cout << "=== 检测成员类型 ===\n";

    std::cout << "has_value_type:\n";
    std::cout << "  std::vector<int>: " << has_value_type_v<std::vector<int>> << "\n";
    std::cout << "  std::string:      " << has_value_type_v<std::string>      << "\n";
    std::cout << "  int:              " << has_value_type_v<int>              << "\n";

    std::cout << "\nhas_iterator:\n";
    std::cout << "  std::vector<int>: " << has_iterator_v<std::vector<int>> << "\n";
    std::cout << "  int:              " << has_iterator_v<int>              << "\n";

    // 使用提取的类型
    std::cout << "\nget_value_type_t:\n";
    std::cout << "  std::vector<int>::value_type 是 int: "
              << std::is_same_v<get_value_type_t<std::vector<int>>, int> << "\n";
    std::cout << "  int 的 value_type 是 void: "
              << std::is_same_v<get_value_type_t<int>, void> << "\n";

    std::cout << "\n";
}

// ============================================================
// 4. 复合 Trait：多条件组合
// ============================================================

// 判断是否为"可迭代的范围"：有 begin/end 且不是 string
template<typename T>
struct is_range : std::bool_constant<
    has_iterator_v<T> &&
    !std::is_same_v<std::decay_t<T>, std::string>
> {};

template<typename T>
inline constexpr bool is_range_v = is_range<T>::value;

// 判断是否为"数值容器"：是容器且 value_type 为算术类型
template<typename T, typename = void>
struct is_numeric_container : std::false_type {};

template<typename T>
struct is_numeric_container<T, std::enable_if_t<
    is_container_v<T> &&
    std::is_arithmetic_v<typename T::value_type>
>> : std::true_type {};

template<typename T>
inline constexpr bool is_numeric_container_v = is_numeric_container<T>::value;

// 判断是否为"字符串类型"
template<typename T>
struct is_string_like : std::bool_constant<
    std::is_same_v<std::decay_t<T>, std::string> ||
    std::is_same_v<std::decay_t<T>, const char*> ||
    std::is_same_v<std::decay_t<T>, char*>
> {};

template<typename T>
inline constexpr bool is_string_like_v = is_string_like<T>::value;

void demo_compound_traits() {
    std::cout << "=== 复合 Trait ===\n";

    std::cout << "is_range (可迭代范围，排除string):\n";
    std::cout << "  std::vector<int>: " << is_range_v<std::vector<int>> << "\n";
    std::cout << "  std::string:      " << is_range_v<std::string>      << "\n";
    std::cout << "  int:              " << is_range_v<int>              << "\n";

    std::cout << "\nis_numeric_container:\n";
    std::cout << "  std::vector<int>:    " << is_numeric_container_v<std::vector<int>>    << "\n";
    std::cout << "  std::vector<double>: " << is_numeric_container_v<std::vector<double>> << "\n";
    std::cout << "  std::vector<std::string>: " << is_numeric_container_v<std::vector<std::string>> << "\n";
    std::cout << "  int:                 " << is_numeric_container_v<int>                 << "\n";

    std::cout << "\nis_string_like:\n";
    std::cout << "  std::string:  " << is_string_like_v<std::string>  << "\n";
    std::cout << "  const char*:  " << is_string_like_v<const char*>  << "\n";
    std::cout << "  int:          " << is_string_like_v<int>          << "\n";

    std::cout << "\n";
}

// ============================================================
// 5. Trait 与 if constexpr 结合的实用模式
// ============================================================

// 通用 to_string：根据类型特征选择最佳实现
template<typename T>
std::string smart_to_string(const T& value) {
    if constexpr (is_string_like_v<T>) {
        return value;
    } else if constexpr (has_serialize_v<T>) {
        return value.serialize();
    } else if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(value);
    } else if constexpr (is_container_v<T>) {
        std::string result = "[";
        bool first = true;
        for (const auto& elem : value) {
            if (!first) result += ", ";
            first = false;
            result += smart_to_string(elem);
        }
        result += "]";
        return result;
    } else {
        return "<non-serializable>";
    }
}

// 通用比较：根据类型选择比较策略
template<typename T>
int smart_compare(const T& a, const T& b) {
    if constexpr (has_subscript_v<T> && is_container_v<T>) {
        // 容器：逐元素比较
        if (a.size() != b.size()) return a.size() < b.size() ? -1 : 1;
        auto it_a = a.begin();
        auto it_b = b.begin();
        while (it_a != a.end()) {
            if (*it_a < *it_b) return -1;
            if (*it_a > *it_b) return 1;
            ++it_a; ++it_b;
        }
        return 0;
    } else if constexpr (std::is_arithmetic_v<T>) {
        if (a < b) return -1;
        if (a > b) return 1;
        return 0;
    } else {
        // 默认使用 operator<
        if (a < b) return -1;
        if (b < a) return 1;
        return 0;
    }
}

void demo_trait_if_constexpr() {
    std::cout << "=== Trait + if constexpr 实用模式 ===\n";

    // 通用 to_string
    std::cout << "smart_to_string:\n";
    std::cout << "  42:          " << smart_to_string(42) << "\n";
    std::cout << "  3.14:        " << smart_to_string(3.14) << "\n";
    std::cout << "  \"hello\":     " << smart_to_string(std::string("hello")) << "\n";
    std::cout << "  Serializable: " << smart_to_string(Serializable{}) << "\n";

    std::vector<int> vec = {1, 2, 3};
    std::cout << "  vector:      " << smart_to_string(vec) << "\n";

    // 通用比较
    std::cout << "\nsmart_compare:\n";
    std::cout << "  (1, 2):   " << smart_compare(1, 2) << "\n";
    std::cout << "  (2, 1):   " << smart_compare(2, 1) << "\n";
    std::cout << "  (1, 1):   " << smart_compare(1, 1) << "\n";

    std::cout << "\n";
}

// ============================================================
// 6. 高级 Trait：编译期类型列表操作
// ============================================================

// 判断类型是否在类型列表中
template<typename T, typename... Types>
struct is_one_of;

template<typename T>
struct is_one_of<T> : std::false_type {};

template<typename T, typename First, typename... Rest>
struct is_one_of<T, First, Rest...>
    : std::conditional_t<std::is_same_v<T, First>, std::true_type, is_one_of<T, Rest...>> {};

template<typename T, typename... Types>
inline constexpr bool is_one_of_v = is_one_of<T, Types...>::value;

// 在类型列表中查找类型的索引
template<typename T, typename... Types>
struct type_index;

template<typename T, typename... Rest>
struct type_index<T, T, Rest...> : std::integral_constant<std::size_t, 0> {};

template<typename T, typename First, typename... Rest>
struct type_index<T, First, Rest...>
    : std::integral_constant<std::size_t, 1 + type_index<T, Rest...>::value> {};

// 获取类型列表的大小
template<typename... Types>
struct type_list_size : std::integral_constant<std::size_t, sizeof...(Types)> {};

void demo_type_list_traits() {
    std::cout << "=== 类型列表操作 Trait ===\n";

    // is_one_of
    std::cout << "is_one_of:\n";
    std::cout << "  int in {int, double, char}: " << is_one_of_v<int, int, double, char> << "\n";
    std::cout << "  float in {int, double, char}: " << is_one_of_v<float, int, double, char> << "\n";
    std::cout << "  int in {long, float}: " << is_one_of_v<int, long, float> << "\n";

    // type_index
    std::cout << "\ntype_index:\n";
    std::cout << "  int in {double, int, char}: " << type_index<int, double, int, char>::value << "\n";
    std::cout << "  char in {int, double, char}: " << type_index<char, int, double, char>::value << "\n";

    // type_list_size
    std::cout << "\ntype_list_size:\n";
    std::cout << "  {int, double, char}: " << type_list_size<int, double, char>::value << "\n";
    std::cout << "  {}: " << type_list_size<>::value << "\n";

    std::cout << "\n";
}

// ============================================================
// 7. C++20 Concepts 风格的 Trait（使用 requires 表达式）
// ============================================================

// C++20 的 requires 表达式可以更简洁地定义 trait
template<typename T>
concept HasSize = requires(T t) {
    { t.size() } -> std::convertible_to<std::size_t>;
};

template<typename T>
concept HasPushBack = requires(T t, typename T::value_type v) {
    { t.push_back(v) };
};

template<typename T>
concept NumericContainer = requires(T t) {
    typename T::value_type;
    requires std::is_arithmetic_v<typename T::value_type>;
    { t.size() } -> std::convertible_to<std::size_t>;
    { t.begin() };
    { t.end() };
};

void demo_concept_style_traits() {
    std::cout << "=== C++20 Concepts 风格 Trait ===\n";

    std::cout << "HasSize:\n";
    std::cout << "  std::vector<int>: " << HasSize<std::vector<int>> << "\n";
    std::cout << "  std::string:      " << HasSize<std::string>      << "\n";
    std::cout << "  int:              " << HasSize<int>              << "\n";

    std::cout << "\nHasPushBack:\n";
    std::cout << "  std::vector<int>: " << HasPushBack<std::vector<int>> << "\n";
    std::cout << "  std::map<int,int>:" << HasPushBack<std::map<int,int>> << "\n";

    std::cout << "\nNumericContainer:\n";
    std::cout << "  std::vector<int>:    " << NumericContainer<std::vector<int>>    << "\n";
    std::cout << "  std::vector<double>: " << NumericContainer<std::vector<double>> << "\n";
    std::cout << "  std::vector<std::string>: " << NumericContainer<std::vector<std::string>> << "\n";

    std::cout << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "  自定义类型特征 (Custom Type Traits)\n";
    std::cout << "============================================\n\n";

    demo_custom_container_trait();
    demo_detect_member_functions();
    demo_detect_member_types();
    demo_compound_traits();
    demo_trait_if_constexpr();
    demo_type_list_traits();
    demo_concept_style_traits();

    std::cout << "============================================\n";
    std::cout << "  核心要点总结:\n";
    std::cout << "  1. void_t 惯用法: 检测成员/方法是否存在\n";
    std::cout << "  2. 主模板=false_type + 特化=true_type 模式\n";
    std::cout << "  3. 复合Trait: 多个条件组合判断\n";
    std::cout << "  4. Trait + if constexpr: 编译期分支选择\n";
    std::cout << "  5. C++20 Concepts: 更简洁的约束表达\n";
    std::cout << "============================================\n";

    return 0;
}
