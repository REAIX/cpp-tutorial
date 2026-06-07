/** @file 03_deep_dive_typelist.cpp
 *  @brief Typelist实现：push_front、push_back、transform、filter等类型列表操作
 *  @description 对应文档: 07-模板元编程与编译期计算 / 模板元编程模式(深入)
 */

#include <iostream>
#include <string>
#include <type_traits>
#include <typeinfo>

// ============================================================
// 1. Typelist 基本定义
// ============================================================

// Typelist: 编译期类型容器，类似运行期的 vector<type>
// 经典设计来自《C++模板元编程》(Abrahams & Gurtovoy)
template<typename... Ts>
struct Typelist {};

// 常用别名
using EmptyList = Typelist<>;
using IntList = Typelist<int, long, short>;
using NumericList = Typelist<int, float, double, long>;

// 基本属性：大小
template<typename List>
struct Size;

template<typename... Ts>
struct Size<Typelist<Ts...>> {
    static constexpr std::size_t value = sizeof...(Ts);
};

template<typename List>
inline constexpr std::size_t Size_v = Size<List>::value;

// 基本属性：是否为空
template<typename List>
struct IsEmpty : std::bool_constant<Size_v<List> == 0> {};

template<typename List>
inline constexpr bool IsEmpty_v = IsEmpty<List>::value;

void demo_typelist_basics() {
    std::cout << "=== Typelist 基本定义 ===\n";

    std::cout << "Size:\n";
    std::cout << "  EmptyList:    " << Size_v<EmptyList> << "\n";
    std::cout << "  IntList:      " << Size_v<IntList> << "\n";
    std::cout << "  NumericList:  " << Size_v<NumericList> << "\n";

    std::cout << "\nIsEmpty:\n";
    std::cout << "  EmptyList:    " << IsEmpty_v<EmptyList> << "\n";
    std::cout << "  IntList:      " << IsEmpty_v<IntList> << "\n";

    std::cout << "\n";
}

// ============================================================
// 2. Typelist 访问操作
// ============================================================

// Front: 获取第一个类型
template<typename List>
struct Front;

template<typename Head, typename... Tail>
struct Front<Typelist<Head, Tail...>> {
    using type = Head;
};

template<typename List>
using Front_t = typename Front<List>::type;

// Back: 获取最后一个类型
template<typename List>
struct Back;

template<typename Head>
struct Back<Typelist<Head>> {
    using type = Head;
};

template<typename Head, typename... Tail>
struct Back<Typelist<Head, Tail...>> {
    using type = typename Back<Typelist<Tail...>>::type;
};

template<typename List>
using Back_t = typename Back<List>::type;

// At: 按索引获取类型
template<std::size_t N, typename List>
struct At;

template<typename Head, typename... Tail>
struct At<0, Typelist<Head, Tail...>> {
    using type = Head;
};

template<std::size_t N, typename Head, typename... Tail>
struct At<N, Typelist<Head, Tail...>> {
    using type = typename At<N - 1, Typelist<Tail...>>::type;
};

template<std::size_t N, typename List>
using At_t = typename At<N, List>::type;

// 类型名称辅助（简化版）
template<typename T>
const char* type_name() {
#if defined(_MSC_VER)
    return __FUNCSIG__;
#elif defined(__GNUC__) || defined(__clang__)
    return __PRETTY_FUNCTION__;
#else
    return typeid(T).name();
#endif
}

void demo_typelist_access() {
    std::cout << "=== Typelist 访问操作 ===\n";

    using List = Typelist<int, double, std::string, char>;

    std::cout << "Front: " << type_name<Front_t<List>>() << "\n";
    std::cout << "Back:  " << type_name<Back_t<List>>() << "\n";
    std::cout << "At<0>: " << type_name<At_t<0, List>>() << "\n";
    std::cout << "At<2>: " << type_name<At_t<2, List>>() << "\n";

    // 编译期验证
    static_assert(std::is_same_v<Front_t<List>, int>);
    static_assert(std::is_same_v<Back_t<List>, char>);
    static_assert(std::is_same_v<At_t<2, List>, std::string>);

    std::cout << "\n";
}

// ============================================================
// 3. Typelist 修改操作
// ============================================================

// PushFront: 在头部添加类型
template<typename List, typename T>
struct PushFront;

template<typename... Ts, typename T>
struct PushFront<Typelist<Ts...>, T> {
    using type = Typelist<T, Ts...>;
};

template<typename List, typename T>
using PushFront_t = typename PushFront<List, T>::type;

// PushBack: 在尾部添加类型
template<typename List, typename T>
struct PushBack;

template<typename... Ts, typename T>
struct PushBack<Typelist<Ts...>, T> {
    using type = Typelist<Ts..., T>;
};

template<typename List, typename T>
using PushBack_t = typename PushBack<List, T>::type;

// PopFront: 移除第一个类型
template<typename List>
struct PopFront;

template<typename Head, typename... Tail>
struct PopFront<Typelist<Head, Tail...>> {
    using type = Typelist<Tail...>;
};

template<typename List>
using PopFront_t = typename PopFront<List>::type;

// Reverse: 反转类型列表
template<typename List>
struct Reverse;

template<>
struct Reverse<Typelist<>> {
    using type = Typelist<>;
};

template<typename Head, typename... Tail>
struct Reverse<Typelist<Head, Tail...>> {
    using type = PushBack_t<typename Reverse<Typelist<Tail...>>::type, Head>;
};

template<typename List>
using Reverse_t = typename Reverse<List>::type;

// Concat: 连接两个类型列表
template<typename List1, typename List2>
struct Concat;

template<typename... Ts, typename... Us>
struct Concat<Typelist<Ts...>, Typelist<Us...>> {
    using type = Typelist<Ts..., Us...>;
};

template<typename List1, typename List2>
using Concat_t = typename Concat<List1, List2>::type;

void demo_typelist_modification() {
    std::cout << "=== Typelist 修改操作 ===\n";

    using List = Typelist<int, double>;

    // PushFront
    using WithFront = PushFront_t<List, char>;
    std::cout << "PushFront<char>: 大小=" << Size_v<WithFront> << "\n";
    static_assert(std::is_same_v<Front_t<WithFront>, char>);

    // PushBack
    using WithBack = PushBack_t<List, std::string>;
    std::cout << "PushBack<string>: 大小=" << Size_v<WithBack> << "\n";
    static_assert(std::is_same_v<Back_t<WithBack>, std::string>);

    // PopFront
    using Popped = PopFront_t<List>;
    std::cout << "PopFront: 大小=" << Size_v<Popped> << "\n";
    static_assert(std::is_same_v<Front_t<Popped>, double>);

    // Reverse
    using Rev = Reverse_t<Typelist<int, double, char>>;
    static_assert(std::is_same_v<Front_t<Rev>, char>);
    static_assert(std::is_same_v<Back_t<Rev>, int>);
    std::cout << "Reverse: 首类型=char, 尾类型=int (验证通过)\n";

    // Concat
    using Joined = Concat_t<Typelist<int, double>, Typelist<char, std::string>>;
    std::cout << "Concat: 大小=" << Size_v<Joined> << "\n";
    static_assert(Size_v<Joined> == 4);

    std::cout << "\n";
}

// ============================================================
// 4. Typelist 查询操作
// ============================================================

// Contains: 判断类型是否在列表中
template<typename List, typename T>
struct Contains;

template<typename T>
struct Contains<Typelist<>, T> : std::false_type {};

template<typename T, typename... Tail>
struct Contains<Typelist<T, Tail...>, T> : std::true_type {};

template<typename Head, typename... Tail, typename T>
struct Contains<Typelist<Head, Tail...>, T> : Contains<Typelist<Tail...>, T> {};

template<typename List, typename T>
inline constexpr bool Contains_v = Contains<List, T>::value;

// CountOf: 统计类型出现次数
template<typename List, typename T>
struct CountOf;

template<typename T>
struct CountOf<Typelist<>, T> : std::integral_constant<std::size_t, 0> {};

template<typename T, typename... Tail>
struct CountOf<Typelist<T, Tail...>, T>
    : std::integral_constant<std::size_t, 1 + CountOf<Typelist<Tail...>, T>::value> {};

template<typename Head, typename... Tail, typename T>
struct CountOf<Typelist<Head, Tail...>, T>
    : CountOf<Typelist<Tail...>, T> {};

template<typename List, typename T>
inline constexpr std::size_t CountOf_v = CountOf<List, T>::value;

// IndexOf: 查找类型的索引
template<typename List, typename T>
struct IndexOf;

template<typename T, typename... Tail>
struct IndexOf<Typelist<T, Tail...>, T>
    : std::integral_constant<std::size_t, 0> {};

template<typename Head, typename... Tail, typename T>
struct IndexOf<Typelist<Head, Tail...>, T>
    : std::integral_constant<std::size_t, 1 + IndexOf<Typelist<Tail...>, T>::value> {};

// 空列表终止: 类型不存在时返回哨兵值
template<typename T>
struct IndexOf<Typelist<>, T>
    : std::integral_constant<std::size_t, static_cast<std::size_t>(-1)> {};

template<typename List, typename T>
inline constexpr std::size_t IndexOf_v = IndexOf<List, T>::value;

void demo_typelist_queries() {
    std::cout << "=== Typelist 查询操作 ===\n";

    using List = Typelist<int, double, int, char>;

    std::cout << "Contains:\n";
    std::cout << "  int in List:    " << Contains_v<List, int> << "\n";
    std::cout << "  float in List:  " << Contains_v<List, float> << "\n";
    std::cout << "  double in List: " << Contains_v<List, double> << "\n";

    std::cout << "\nCountOf:\n";
    std::cout << "  int in List:    " << CountOf_v<List, int> << "\n";
    std::cout << "  double in List: " << CountOf_v<List, double> << "\n";
    std::cout << "  float in List:  " << CountOf_v<List, float> << "\n";

    std::cout << "\nIndexOf:\n";
    std::cout << "  int in List:    " << IndexOf_v<List, int> << "\n";
    std::cout << "  double in List: " << IndexOf_v<List, double> << "\n";
    std::cout << "  char in List:   " << IndexOf_v<List, char> << "\n";

    std::cout << "\n";
}

// ============================================================
// 5. Typelist 高级操作：Transform
// ============================================================

// Transform: 对列表中每个类型应用元函数
template<typename List, template<typename> class F>
struct Transform;

template<typename... Ts, template<typename> class F>
struct Transform<Typelist<Ts...>, F> {
    using type = Typelist<typename F<Ts>::type...>;
};

template<typename List, template<typename> class F>
using Transform_t = typename Transform<List, F>::type;

// 示例元函数
template<typename T>
struct AddPointer { using type = T*; };

template<typename T>
struct AddConst { using type = const T; };

template<typename T>
struct RemoveConst { using type = std::remove_const_t<T>; };

template<typename T>
struct Decay { using type = std::decay_t<T>; };

void demo_typelist_transform() {
    std::cout << "=== Typelist Transform ===\n";

    using List = Typelist<int, double, char>;

    // AddPointer
    using PtrList = Transform_t<List, AddPointer>;
    std::cout << "AddPointer:\n";
    static_assert(std::is_same_v<At_t<0, PtrList>, int*>);
    static_assert(std::is_same_v<At_t<1, PtrList>, double*>);
    std::cout << "  int -> int*, double -> double* (验证通过)\n";

    // AddConst
    using ConstList = Transform_t<List, AddConst>;
    static_assert(std::is_same_v<At_t<0, ConstList>, const int>);
    std::cout << "AddConst: int -> const int (验证通过)\n";

    // 使用标准库的元函数
    using RefList = Typelist<int&, double&, char&>;
    using ValueList = Transform_t<RefList, std::remove_reference>;
    static_assert(std::is_same_v<At_t<0, ValueList>, int>);
    std::cout << "remove_reference: int& -> int (验证通过)\n";

    std::cout << "\n";
}

// ============================================================
// 6. Typelist 高级操作：Filter
// ============================================================

// Filter: 根据谓词过滤类型列表
template<typename List, template<typename> class Pred>
struct Filter;

template<template<typename> class Pred>
struct Filter<Typelist<>, Pred> {
    using type = Typelist<>;
};

template<typename Head, typename... Tail, template<typename> class Pred>
struct Filter<Typelist<Head, Tail...>, Pred> {
    using type = std::conditional_t<
        Pred<Head>::value,
        PushFront_t<typename Filter<Typelist<Tail...>, Pred>::type, Head>,
        typename Filter<Typelist<Tail...>, Pred>::type
    >;
};

template<typename List, template<typename> class Pred>
using Filter_t = typename Filter<List, Pred>::type;

// 示例谓词
template<typename T>
struct IsIntegral : std::is_integral<T> {};

template<typename T>
struct IsFloatingPoint : std::is_floating_point<T> {};

template<typename T>
struct IsPointer : std::is_pointer<T> {};

// 自定义谓词：大小大于4字节的类型
template<typename T>
struct IsLarge : std::bool_constant<(sizeof(T) > 4)> {};

void demo_typelist_filter() {
    std::cout << "=== Typelist Filter ===\n";

    using MixedList = Typelist<char, int, double, short, long long, float>;

    // 过滤整型
    using Ints = Filter_t<MixedList, IsIntegral>;
    std::cout << "Filter IsIntegral: 大小=" << Size_v<Ints> << "\n";
    static_assert(Size_v<Ints> == 4);  // char, int, short, long long

    // 过滤浮点型
    using Floats = Filter_t<MixedList, IsFloatingPoint>;
    std::cout << "Filter IsFloatingPoint: 大小=" << Size_v<Floats> << "\n";
    static_assert(Size_v<Floats> == 2);  // double, float

    // 过滤大类型
    using Large = Filter_t<MixedList, IsLarge>;
    std::cout << "Filter IsLarge(>4bytes): 大小=" << Size_v<Large> << "\n";
    // double(8), long long(8)

    std::cout << "\n";
}

// ============================================================
// 7. Typelist 实用操作：去重、排序
// ============================================================

// Unique: 去除重复类型
template<typename List>
struct Unique;

template<>
struct Unique<Typelist<>> {
    using type = Typelist<>;
};

template<typename Head, typename... Tail>
struct Unique<Typelist<Head, Tail...>> {
    using Rest = typename Unique<Typelist<Tail...>>::type;
    using type = std::conditional_t<
        Contains_v<Rest, Head>,
        Rest,
        PushFront_t<Rest, Head>
    >;
};

template<typename List>
using Unique_t = typename Unique<List>::type;

// 统计所有类型的大小之和
template<typename List>
struct TotalSize;

template<>
struct TotalSize<Typelist<>> : std::integral_constant<std::size_t, 0> {};

template<typename Head, typename... Tail>
struct TotalSize<Typelist<Head, Tail...>>
    : std::integral_constant<std::size_t, sizeof(Head) + TotalSize<Typelist<Tail...>>::value> {};

template<typename List>
inline constexpr std::size_t TotalSize_v = TotalSize<List>::value;

// 找出最大类型
template<typename List>
struct LargestType;

template<typename Head>
struct LargestType<Typelist<Head>> {
    using type = Head;
};

template<typename Head, typename... Tail>
struct LargestType<Typelist<Head, Tail...>> {
    using Rest = typename LargestType<Typelist<Tail...>>::type;
    using type = std::conditional_t<(sizeof(Head) >= sizeof(Rest)), Head, Rest>;
};

template<typename List>
using LargestType_t = typename LargestType<List>::type;

void demo_typelist_utilities() {
    std::cout << "=== Typelist 实用操作 ===\n";

    // Unique
    using DupList = Typelist<int, double, int, char, double, int>;
    using UniqList = Unique_t<DupList>;
    std::cout << "Unique: 大小 " << Size_v<DupList> << " -> " << Size_v<UniqList> << "\n";
    static_assert(Size_v<UniqList> == 3);  // int, double, char

    // TotalSize
    using List = Typelist<int, double, char>;
    std::cout << "TotalSize: " << TotalSize_v<List> << " bytes\n";

    // LargestType
    using Mixed = Typelist<char, double, int>;
    std::cout << "LargestType: " << type_name<LargestType_t<Mixed>>() << "\n";
    static_assert(std::is_same_v<LargestType_t<Mixed>, double>);

    std::cout << "\n";
}

// ============================================================
// 8. Typelist 运行期应用：类型遍历
// ============================================================

// 对 Typelist 中每个类型执行操作
template<typename List, typename Func>
void for_each_type(Func&& func);

template<typename Head, typename... Tail, typename Func>
void for_each_type_impl(Func&& func) {
    func.template operator()<Head>();
    if constexpr (sizeof...(Tail) > 0) {
        for_each_type_impl<Tail...>(std::forward<Func>(func));
    }
}

template<typename... Ts, typename Func>
void for_each_type(Typelist<Ts...>, Func&& func) {
    for_each_type_impl<Ts...>(std::forward<Func>(func));
}

// 根据 Typelist 创建变体访问器
template<typename List>
struct TypeVisitor;

template<typename... Ts>
struct TypeVisitor<Typelist<Ts...>> {
    template<typename Func>
    static void visit_all(Func&& func) {
        (func.template operator()<Ts>(), ...);
    }
};

void demo_typelist_runtime() {
    std::cout << "=== Typelist 运行期应用 ===\n";

    using MyList = Typelist<int, double, std::string>;

    // 遍历打印类型信息
    std::cout << "for_each_type:\n";
    for_each_type(MyList{}, []<typename T>() {
        std::cout << "  类型: " << type_name<T>()
                  << ", 大小: " << sizeof(T) << " bytes\n";
    });

    // 使用 TypeVisitor
    std::cout << "\nTypeVisitor:\n";
    TypeVisitor<MyList>::visit_all([]<typename T>() {
        if constexpr (std::is_integral_v<T>) {
            std::cout << "  " << type_name<T>() << " 是整型\n";
        } else if constexpr (std::is_floating_point_v<T>) {
            std::cout << "  " << type_name<T>() << " 是浮点型\n";
        } else {
            std::cout << "  " << type_name<T>() << " 是其他类型\n";
        }
    });

    std::cout << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "  Typelist 类型列表操作\n";
    std::cout << "============================================\n\n";

    demo_typelist_basics();
    demo_typelist_access();
    demo_typelist_modification();
    demo_typelist_queries();
    demo_typelist_transform();
    demo_typelist_filter();
    demo_typelist_utilities();
    demo_typelist_runtime();

    std::cout << "============================================\n";
    std::cout << "  核心要点总结:\n";
    std::cout << "  1. Typelist: 编译期类型容器\n";
    std::cout << "  2. 访问: Front, Back, At\n";
    std::cout << "  3. 修改: PushFront, PushBack, Reverse, Concat\n";
    std::cout << "  4. 查询: Contains, CountOf, IndexOf\n";
    std::cout << "  5. 高级: Transform, Filter, Unique\n";
    std::cout << "  6. 运行期: for_each_type 类型遍历\n";
    std::cout << "============================================\n";

    return 0;
}
