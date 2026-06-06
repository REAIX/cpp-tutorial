/** @file 01_deep_dive_template_metaprogramming.cpp
 *  @brief 模板元编程：编译期计算、类型特征、conditional_t、static_assert、if constexpr
 *  @description 对应文档: 11-模板进阶 | 举一反三：掌握编译期编程的核心技术
 */

#include <iostream>
#include <string>
#include <type_traits>
#include <array>
#include <vector>
#include <cstdint>

template<unsigned int N>
struct Factorial {
    static constexpr unsigned long long value = N * Factorial<N - 1>::value;
};

template<>
struct Factorial<0> {
    static constexpr unsigned long long value = 1;
};

template<unsigned int N>
struct Fibonacci {
    static constexpr unsigned long long value = Fibonacci<N - 1>::value + Fibonacci<N - 2>::value;
};

template<>
struct Fibonacci<0> {
    static constexpr unsigned long long value = 0;
};

template<>
struct Fibonacci<1> {
    static constexpr unsigned long long value = 1;
};

template<unsigned int N>
constexpr unsigned long long factorial() {
    unsigned long long result = 1;
    for (unsigned int i = 2; i <= N; ++i) {
        result *= i;
    }
    return result;
}

void demo_compile_time_computation() {
    std::cout << "=== 编译期计算 ===\n";

    std::cout << "Factorial<5>::value = " << Factorial<5>::value << "\n";
    std::cout << "Factorial<10>::value = " << Factorial<10>::value << "\n";
    std::cout << "Factorial<20>::value = " << Factorial<20>::value << "\n";

    std::cout << "\nFibonacci<10>::value = " << Fibonacci<10>::value << "\n";
    std::cout << "Fibonacci<20>::value = " << Fibonacci<20>::value << "\n";

    std::cout << "\nconstexpr 函数 (C++14起更灵活):\n";
    std::cout << "factorial<5>() = " << factorial<5>() << "\n";
    std::cout << "factorial<20>() = " << factorial<20>() << "\n";
    std::cout << "constexpr 函数可以在编译期和运行期都执行\n";

    std::cout << "\n";
}

template<typename T>
struct TypeInfo {
    static std::string name() {
        if constexpr (std::is_integral_v<T>) {
            if constexpr (std::is_signed_v<T>) return "有符号整型";
            else return "无符号整型";
        } else if constexpr (std::is_floating_point_v<T>) {
            return "浮点型";
        } else if constexpr (std::is_pointer_v<T>) {
            return "指针类型";
        } else if constexpr (std::is_reference_v<T>) {
            return "引用类型";
        } else if constexpr (std::is_array_v<T>) {
            return "数组类型";
        } else {
            return "其他类型";
        }
    }

    static constexpr size_t size = sizeof(T);
    static constexpr bool is_arithmetic = std::is_arithmetic_v<T>;
    static constexpr bool is_pointer = std::is_pointer_v<T>;
};

void demo_type_traits() {
    std::cout << "=== 类型特征 (Type Traits) ===\n";

    std::cout << "int: " << TypeInfo<int>::name() << ", 大小=" << TypeInfo<int>::size << "\n";
    std::cout << "unsigned int: " << TypeInfo<unsigned int>::name() << ", 大小=" << TypeInfo<unsigned int>::size << "\n";
    std::cout << "double: " << TypeInfo<double>::name() << ", 大小=" << TypeInfo<double>::size << "\n";
    std::cout << "int*: " << TypeInfo<int*>::name() << ", 大小=" << TypeInfo<int*>::size << "\n";
    std::cout << "int[5]: " << TypeInfo<int[5]>::name() << ", 大小=" << TypeInfo<int[5]>::size << "\n";

    std::cout << "\n常用类型特征:\n";
    std::cout << "  is_integral, is_floating_point, is_arithmetic\n";
    std::cout << "  is_pointer, is_reference, is_array\n";
    std::cout << "  is_const, is_signed, is_unsigned\n";
    std::cout << "  is_same, is_base_of, is_convertible\n";

    std::cout << "\n";
}

template<bool Cond, typename TrueType, typename FalseType>
struct Conditional {
    using type = TrueType;
};

template<typename TrueType, typename FalseType>
struct Conditional<false, TrueType, FalseType> {
    using type = FalseType;
};

template<typename T>
using SafeType = std::conditional_t<
    std::is_integral_v<T>,
    int64_t,
    std::conditional_t<
        std::is_floating_point_v<T>,
        double,
        std::string
    >
>;

void demo_conditional_t() {
    std::cout << "=== conditional_t 条件类型选择 ===\n";

    std::cout << "SafeType<int> 的大小: " << sizeof(SafeType<int>) << " (选择了 int64_t)\n";
    std::cout << "SafeType<float> 的大小: " << sizeof(SafeType<float>) << " (选择了 double)\n";
    std::cout << "SafeType<std::string> 的大小: " << sizeof(SafeType<std::string>) << " (选择了 string)\n";

    std::cout << "\nconditional_t 的原理:\n";
    std::cout << "  template<bool, typename T, typename F>\n";
    std::cout << "  struct conditional { using type = T; };\n\n";
    std::cout << "  template<typename T, typename F>\n";
    std::cout << "  struct conditional<false, T, F> { using type = F; };\n\n";
    std::cout << "  conditional_t<cond, T, F> 是 conditional<cond, T, F>::type 的别名\n";

    std::cout << "\n";
}

template<typename T>
class NumericWrapper {
    static_assert(std::is_arithmetic_v<T>, "NumericWrapper 只支持算术类型!");
public:
    NumericWrapper(T value) : value_(value) {}
    T get() const { return value_; }
    void set(T value) { value_ = value; }
private:
    T value_;
};

void demo_static_assert() {
    std::cout << "=== static_assert 编译期断言 ===\n";

    NumericWrapper<int> wi(42);
    NumericWrapper<double> wd(3.14);
    std::cout << "NumericWrapper<int>(42): " << wi.get() << "\n";
    std::cout << "NumericWrapper<double>(3.14): " << wd.get() << "\n";

    // NumericWrapper<std::string> ws("hello");  // 编译错误!
    std::cout << "NumericWrapper<string> 会触发 static_assert 错误\n";

    std::cout << "\nstatic_assert 的用法:\n";
    std::cout << "  static_assert(常量表达式, \"错误消息\");\n";
    std::cout << "  在编译期检查条件, 失败则编译错误\n";
    std::cout << "  C++17 起, 错误消息可省略\n";

    std::cout << "\n";
}

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

template<typename T>
auto process(T value) {
    if constexpr (std::is_integral_v<T>) {
        std::cout << "整型处理: " << value * 2 << "\n";
        return value * 2;
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "浮点处理: " << value * 1.5 << "\n";
        return value * 1.5;
    } else {
        std::cout << "其他类型处理: " << value << "\n";
        return value;
    }
}

template<typename Container>
void print_container_info(const Container& c) {
    if constexpr (HasSize<Container>::value) {
        std::cout << "容器大小: " << c.size() << "\n";
    } else {
        std::cout << "不是标准容器\n";
    }

    if constexpr (HasBeginEnd<Container>::value) {
        std::cout << "元素: ";
        for (const auto& item : c) {
            std::cout << item << " ";
        }
        std::cout << "\n";
    }
}

void demo_if_constexpr() {
    std::cout << "=== if constexpr 编译期条件 (C++17) ===\n";

    process(21);
    process(3.14);
    process(std::string("hello"));

    std::cout << "\nif constexpr 的特点:\n";
    std::cout << "  1. 条件必须是编译期常量\n";
    std::cout << "  2. 不满足的分支不会被编译 (不会产生语法错误)\n";
    std::cout << "  3. 替代 SFINAE 的许多场景\n";
    std::cout << "  4. 让模板代码更清晰易读\n";

    std::cout << "\n";
}

template<typename T>
std::string to_string_generic(const T& value) {
    if constexpr (std::is_same_v<T, bool>) {
        return value ? "true" : "false";
    } else if constexpr (std::is_integral_v<T>) {
        return std::to_string(value);
    } else if constexpr (std::is_floating_point_v<T>) {
        return std::to_string(value);
    } else if constexpr (std::is_same_v<T, std::string>) {
        return value;
    } else {
        return typeid(T).name();
    }
}

void demo_if_constexpr_practical() {
    std::cout << "=== if constexpr 实战 ===\n";

    std::cout << "to_string(true) = " << to_string_generic(true) << "\n";
    std::cout << "to_string(42) = " << to_string_generic(42) << "\n";
    std::cout << "to_string(3.14) = " << to_string_generic(3.14) << "\n";
    std::cout << "to_string(\"hello\") = " << to_string_generic(std::string("hello")) << "\n";

    std::vector<int> vec = {1, 2, 3, 4, 5};
    print_container_info(vec);

    std::cout << "\nif constexpr vs SFINAE:\n";
    std::cout << "  if constexpr: 更直观, 代码更易读\n";
    std::cout << "  SFINAE: 更灵活, 可以影响重载决议\n";
    std::cout << "  建议: 优先使用 if constexpr\n";

    std::cout << "\n";
}

int main() {
    demo_compile_time_computation();
    demo_type_traits();
    demo_conditional_t();
    demo_static_assert();
    demo_if_constexpr();
    demo_if_constexpr_practical();

    return 0;
}
