/** @file 02_example_auto_type.cpp
 *  @brief auto, decltype, 尾置返回类型与范围for
 *  @description 对应文档: 02-CPP/01-basics
 */

#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <typeinfo>

// ===== 1. auto 关键字 =====
void demo_auto() {
    std::cout << "===== auto 关键字 =====" << std::endl;

    // auto 让编译器自动推导变量类型
    auto i = 42;              // int
    auto d = 3.14;            // double
    auto f = 3.14f;           // float
    auto s = std::string("hello");  // std::string
    auto p = &i;              // int*

    std::cout << "auto i = 42;        类型: " << typeid(i).name() << std::endl;
    std::cout << "auto d = 3.14;      类型: " << typeid(d).name() << std::endl;
    std::cout << "auto f = 3.14f;     类型: " << typeid(f).name() << std::endl;
    std::cout << "auto s = string;    类型: " << typeid(s).name() << std::endl;
    std::cout << "auto p = &i;        类型: " << typeid(p).name() << std::endl;

    // auto 与引用和const
    int value = 42;
    int& ref = value;
    auto auto_from_ref = ref;       // auto 推导为 int, 不是 int& (丢弃引用)
    auto& auto_ref = ref;           // 显式加 &, 推导为 int&
    const int cvalue = 100;
    auto auto_from_const = cvalue;  // auto 推导为 int, 丢弃顶层 const
    const auto auto_const = cvalue; // 显式加 const, 推导为 const int

    auto_from_ref = 999;
    std::cout << "\nauto& 保留引用语义: value = " << value << std::endl;
    std::cout << "auto 默认丢弃引用和顶层const" << std::endl;
}

// ===== 2. decltype =====
void demo_decltype() {
    std::cout << "\n===== decltype =====" << std::endl;

    int x = 42;
    const int& cx = x;

    // decltype 保留完整的类型信息, 包括引用和const
    decltype(x) a = x;       // int
    decltype(cx) b = x;      // const int& (保留引用和const)
    // b = 100;              // 错误: b 是 const 引用

    std::cout << "decltype(x)  -> int" << std::endl;
    std::cout << "decltype(cx) -> const int&" << std::endl;

    // decltype 与表达式的区别
    int arr[5] = {1, 2, 3, 4, 5};
    decltype(arr) arr2;  // int[5] - 数组类型
    // auto arr3 = arr;   // int* - 退化为指针

    // decltype(auto) - C++14, 结合 auto 的方便和 decltype 的精确
    int val = 42;
    int& ref = val;
    decltype(auto) da = ref;  // int& (decltype 语义, 保留引用)
    da = 100;
    std::cout << "decltype(auto) 保留引用: val = " << val << std::endl;
}

// ===== 3. 尾置返回类型 =====
// 传统方式: 返回类型在前面, 看不到参数
template<typename T, typename U>
auto add_trailing(T a, U b) -> decltype(a + b) {
    return a + b;
}

// C++14 可以直接用 auto 推导返回类型
template<typename T, typename U>
auto add_auto(T a, U b) {
    return a + b;
}

void demo_trailing_return_type() {
    std::cout << "\n===== 尾置返回类型 =====" << std::endl;

    auto r1 = add_trailing(1, 2);       // int
    auto r2 = add_trailing(1, 2.5);     // double
    auto r3 = add_auto(1, 2.5);         // double (C++14)

    std::cout << "add_trailing(1, 2) = " << r1 << std::endl;
    std::cout << "add_trailing(1, 2.5) = " << r2 << std::endl;
    std::cout << "add_auto(1, 2.5) = " << r3 << std::endl;

    std::cout << "\n尾置返回类型的使用场景:" << std::endl;
    std::cout << "  1. 返回类型依赖参数时 (decltype(a+b))" << std::endl;
    std::cout << "  2. 函数指针/函数引用声明更清晰" << std::endl;
    std::cout << "  3. C++14 的 auto 返回类型推导更简洁" << std::endl;
}

// ===== 4. 范围 for 循环 =====
void demo_range_for() {
    std::cout << "\n===== 范围 for 循环 =====" << std::endl;

    std::vector<int> vec = {1, 2, 3, 4, 5};

    // 只读遍历
    std::cout << "只读遍历: ";
    for (const auto& elem : vec) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    // 修改元素
    for (auto& elem : vec) {
        elem *= 2;
    }
    std::cout << "修改后: ";
    for (const auto& elem : vec) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    // 遍历 map
    std::map<std::string, int> scores = {
        {"Alice", 95}, {"Bob", 87}, {"Charlie", 92}
    };
    std::cout << "遍历 map:" << std::endl;
    for (const auto& [name, score] : scores) {  // C++17 结构化绑定
        std::cout << "  " << name << ": " << score << std::endl;
    }

    // 遍历数组
    int arr[] = {10, 20, 30};
    std::cout << "遍历数组: ";
    for (auto elem : arr) {
        std::cout << elem << " ";
    }
    std::cout << std::endl;

    // 注意: range-for 不适用于需要索引的场景
    std::cout << "\nrange-for 注意事项:" << std::endl;
    std::cout << "  - 不要在 range-for 中修改容器大小(增删元素)" << std::endl;
    std::cout << "  - 遍历大对象用 const auto& 避免拷贝" << std::endl;
    std::cout << "  - 需要索引时用传统 for 循环" << std::endl;
}

int main() {
    std::cout << "========== auto, decltype 与范围 for ==========\n" << std::endl;

    demo_auto();
    demo_decltype();
    demo_trailing_return_type();
    demo_range_for();

    return 0;
}
