/** @file 02_deep_dive_template_specialization.cpp
 *  @brief 模板特化：全特化、偏特化、指针特化、traits模式
 *  @description 对应文档: 10-模板基础 | 举一反三：掌握模板特化的设计模式
 */

#include <iostream>
#include <string>
#include <vector>
#include <type_traits>
#include <cstdint>

template<typename T>
struct TypeName {
    static std::string get() { return "未知类型"; }
};

template<>
struct TypeName<int> {
    static std::string get() { return "int"; }
};

template<>
struct TypeName<double> {
    static std::string get() { return "double"; }
};

template<>
struct TypeName<std::string> {
    static std::string get() { return "std::string"; }
};

template<>
struct TypeName<bool> {
    static std::string get() { return "bool"; }
};

void demo_full_specialization() {
    std::cout << "=== 全特化 ===\n";

    std::cout << "TypeName<int>::get() = " << TypeName<int>::get() << "\n";
    std::cout << "TypeName<double>::get() = " << TypeName<double>::get() << "\n";
    std::cout << "TypeName<std::string>::get() = " << TypeName<std::string>::get() << "\n";
    std::cout << "TypeName<bool>::get() = " << TypeName<bool>::get() << "\n";
    std::cout << "TypeName<float>::get() = " << TypeName<float>::get() << "\n";

    std::cout << "\n全特化的特点:\n";
    std::cout << "  1. 所有模板参数都被具体类型替代\n";
    std::cout << "  2. template<> 空的模板参数列表\n";
    std::cout << "  3. 完全替代主模板, 不再是模板\n";

    std::cout << "\n";
}

template<typename T>
struct IsPointer {
    static constexpr bool value = false;
    using type = T;
};

template<typename T>
struct IsPointer<T*> {
    static constexpr bool value = true;
    using type = T;
};

void demo_pointer_specialization() {
    std::cout << "=== 指针特化 ===\n";

    std::cout << "IsPointer<int>::value = " << IsPointer<int>::value << "\n";
    std::cout << "IsPointer<int*>::value = " << IsPointer<int*>::value << "\n";
    std::cout << "IsPointer<int**>::value = " << IsPointer<int**>::value << "\n";
    std::cout << "IsPointer<int*>::type = " << TypeName<IsPointer<int*>::type>::get() << "\n";

    std::cout << "\n指针特化的用途:\n";
    std::cout << "  1. 区分指针和非指针类型\n";
    std::cout << "  2. 为指针类型提供不同的实现\n";
    std::cout << "  3. 提取指针指向的类型\n";

    std::cout << "\n";
}

template<typename T, typename U>
struct PairTraits {
    static std::string category() { return "通用Pair"; }
};

template<typename T>
struct PairTraits<T, T> {
    static std::string category() { return "同类型Pair"; }
};

template<typename T>
struct PairTraits<T*, T> {
    static std::string category() { return "指针-值Pair"; }
};

template<typename T, typename U>
struct PairTraits<T*, U*> {
    static std::string category() { return "双指针Pair"; }
};

void demo_partial_specialization() {
    std::cout << "=== 偏特化 ===\n";

    std::cout << "PairTraits<int, double>: " << PairTraits<int, double>::category() << "\n";
    std::cout << "PairTraits<int, int>: " << PairTraits<int, int>::category() << "\n";
    std::cout << "PairTraits<int*, int>: " << PairTraits<int*, int>::category() << "\n";
    std::cout << "PairTraits<int*, double*>: " << PairTraits<int*, double*>::category() << "\n";

    std::cout << "\n偏特化的特点:\n";
    std::cout << "  1. 只特化部分模板参数\n";
    std::cout << "  2. 仍然是模板, 需要剩余参数\n";
    std::cout << "  3. 可以对参数关系进行约束\n";
    std::cout << "  4. 偏特化只在类模板中存在, 函数模板没有偏特化\n";

    std::cout << "\n";
}

template<typename T>
struct Serializer {
    static std::string serialize(const T& value) {
        return std::to_string(value);
    }
};

template<>
struct Serializer<std::string> {
    static std::string serialize(const std::string& value) {
        return "\"" + value + "\"";
    }
};

template<>
struct Serializer<bool> {
    static std::string serialize(bool value) {
        return value ? "true" : "false";
    }
};

template<typename T>
struct Serializer<T*> {
    static std::string serialize(T* ptr) {
        if (ptr) {
            return "ptr->" + Serializer<T>::serialize(*ptr);
        }
        return "nullptr";
    }
};

template<typename T>
struct Serializer<std::vector<T>> {
    static std::string serialize(const std::vector<T>& vec) {
        std::string result = "[";
        for (size_t i = 0; i < vec.size(); ++i) {
            if (i > 0) result += ", ";
            result += Serializer<T>::serialize(vec[i]);
        }
        result += "]";
        return result;
    }
};

void demo_traits_pattern() {
    std::cout << "=== Traits 模式 ===\n";

    std::cout << "serialize(42): " << Serializer<int>::serialize(42) << "\n";
    std::cout << "serialize(3.14): " << Serializer<double>::serialize(3.14) << "\n";
    std::cout << "serialize(\"hello\"): " << Serializer<std::string>::serialize("hello") << "\n";
    std::cout << "serialize(true): " << Serializer<bool>::serialize(true) << "\n";

    int x = 42;
    std::cout << "serialize(&x): " << Serializer<int*>::serialize(&x) << "\n";
    std::cout << "serialize(nullptr): " << Serializer<int*>::serialize(nullptr) << "\n";

    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::cout << "serialize(vector): " << Serializer<std::vector<int>>::serialize(vec) << "\n";

    std::vector<std::string> svec = {"a", "b", "c"};
    std::cout << "serialize(string vector): " << Serializer<std::vector<std::string>>::serialize(svec) << "\n";

    std::cout << "\nTraits 模式的要素:\n";
    std::cout << "  1. 主模板定义通用行为\n";
    std::cout << "  2. 特化提供类型特定行为\n";
    std::cout << "  3. 嵌套类型别名 (type) 和常量 (value)\n";
    std::cout << "  4. 编译期计算, 零运行时开销\n";

    std::cout << "\n";
}

template<typename T>
struct RemoveReference {
    using type = T;
};

template<typename T>
struct RemoveReference<T&> {
    using type = T;
};

template<typename T>
struct RemoveReference<T&&> {
    using type = T;
};

template<typename T>
struct RemoveConst {
    using type = T;
};

template<typename T>
struct RemoveConst<const T> {
    using type = T;
};

template<typename T>
struct RemoveConst<const T&> {
    using type = T&;
};

void demo_standard_traits_implementation() {
    std::cout << "=== 模拟标准库 traits 实现 ===\n";

    std::cout << "RemoveReference<int&>::type = " << TypeName<RemoveReference<int&>::type>::get() << "\n";
    std::cout << "RemoveReference<int&&>::type = " << TypeName<RemoveReference<int&&>::type>::get() << "\n";
    std::cout << "RemoveReference<int>::type = " << TypeName<RemoveReference<int>::type>::get() << "\n";

    std::cout << "\n这些就是 std::remove_reference, std::remove_const 的实现原理\n";
    std::cout << "C++14 起, 标准库提供了 _t 后缀的别名模板:\n";
    std::cout << "  std::remove_reference_t<T> 等价于 typename std::remove_reference<T>::type\n";

    std::cout << "\n";
}

template<typename T>
struct AddPointer {
    using type = typename RemoveReference<T>::type*;
};

template<typename T>
struct Decay {
private:
    using U = typename RemoveReference<T>::type;
public:
    using type = std::conditional_t<
        std::is_array_v<U>,
        typename std::remove_extent_t<U>*,
        std::conditional_t<
            std::is_function_v<U>,
            U*,
            typename std::remove_cv_t<U>
        >
    >;
};

void demo_compound_traits() {
    std::cout << "=== 组合 traits ===\n";

    std::cout << "AddPointer<int&>::type = " << TypeName<AddPointer<int&>::type>::get() << "\n";
    std::cout << "AddPointer<int>::type = " << TypeName<AddPointer<int>::type>::get() << "\n";

    std::cout << "\n组合 traits 的设计模式:\n";
    std::cout << "  1. 每个 trait 做一件简单的事\n";
    std::cout << "  2. 通过组合实现复杂功能\n";
    std::cout << "  3. 类似函数式编程的组合子\n";

    std::cout << "\n";
}

int main() {
    demo_full_specialization();
    demo_pointer_specialization();
    demo_partial_specialization();
    demo_traits_pattern();
    demo_standard_traits_implementation();
    demo_compound_traits();

    return 0;
}
