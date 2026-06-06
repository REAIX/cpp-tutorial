/** @file 01_example_auto_deduction.cpp
 *  @brief auto推导规则：auto与引用、auto与const、auto&&、decltype(auto)
 *  @description 对应文档: 12-类型推导
 */

#include <iostream>
#include <string>
#include <vector>
#include <type_traits>

template<typename T>
std::string type_name() {
    std::string result;
    if constexpr (std::is_const_v<std::remove_reference_t<T>>) result += "const ";
    if constexpr (std::is_pointer_v<std::remove_reference_t<T>>) result += "pointer ";
    if constexpr (std::is_lvalue_reference_v<T>) result += "lvalue_ref ";
    if constexpr (std::is_rvalue_reference_v<T>) result += "rvalue_ref ";
    if constexpr (std::is_integral_v<std::remove_reference_t<T>>) result += "integral";
    else if constexpr (std::is_floating_point_v<std::remove_reference_t<T>>) result += "floating";
    else result += "other";
    return result;
}

void demo_auto_basics() {
    std::cout << "=== auto 基础推导 ===\n";

    auto a = 42;
    auto b = 3.14;
    auto c = "hello";
    auto d = std::string("world");
    auto e = true;

    std::cout << "auto a = 42;          => int, 值=" << a << "\n";
    std::cout << "auto b = 3.14;        => double, 值=" << b << "\n";
    std::cout << "auto c = \"hello\";     => const char*, 值=" << c << "\n";
    std::cout << "auto d = string(...)  => string, 值=" << d << "\n";
    std::cout << "auto e = true;        => bool, 值=" << e << "\n";

    std::cout << "\nauto 推导规则 (与模板推导相同):\n";
    std::cout << "  auto x = expr;   => 按值推导, 忽略 const 和引用\n";
    std::cout << "  auto& x = expr;  => 按引用推导, 保留 const\n";
    std::cout << "  auto&& x = expr; => 万能引用\n";

    std::cout << "\n";
}

void demo_auto_with_const() {
    std::cout << "=== auto 与 const ===\n";

    const int ci = 42;
    const int& cir = ci;

    auto a = ci;
    auto b = cir;
    std::cout << "auto a = ci;   => int (const 被忽略)\n";
    std::cout << "auto b = cir;  => int (const 和引用都被忽略)\n";

    const auto c = ci;
    const auto& d = ci;
    std::cout << "const auto c = ci;   => const int\n";
    std::cout << "const auto& d = ci;  => const int&\n";

    int x = 10;
    int* ptr = &x;
    const int* cptr = &x;

    auto p1 = ptr;
    auto p2 = cptr;
    std::cout << "\nauto p1 = ptr;   => int* (指针保留)\n";
    std::cout << "auto p2 = cptr;  => const int* (指针的 const 保留)\n";

    std::cout << "\nconst 规则总结:\n";
    std::cout << "  按值推导: 顶层 const 被忽略\n";
    std::cout << "  按引用推导: 顶层 const 被保留\n";
    std::cout << "  指针的 const (底层) 总是被保留\n";

    std::cout << "\n";
}

void demo_auto_with_reference() {
    std::cout << "=== auto 与引用 ===\n";

    int x = 42;
    int& ref = x;

    auto a = ref;
    a = 100;
    std::cout << "auto a = ref; => a 是副本, 修改 a 不影响 x\n";
    std::cout << "  x = " << x << ", a = " << a << "\n";

    auto& b = ref;
    b = 200;
    std::cout << "auto& b = ref; => b 是引用, 修改 b 影响x\n";
    std::cout << "  x = " << x << ", b = " << b << "\n";

    const auto& c = x;
    // c = 300;  // 编译错误! const 引用不可修改
    std::cout << "const auto& c = x; => const 引用, 不可修改\n";

    std::cout << "\n";
}

void demo_auto_universal_reference() {
    std::cout << "=== auto&& 万能引用 ===\n";

    auto&& r1 = 42;
    auto&& r2 = std::string("临时对象");
    int x = 10;
    auto&& r3 = x;

    std::cout << "auto&& r1 = 42;            => 右值引用 (int&&)\n";
    std::cout << "auto&& r2 = string(...)    => 右值引用 (string&&)\n";
    std::cout << "auto&& r3 = x;             => 左值引用 (int&)\n";

    std::cout << "\nauto&& 的用途:\n";
    std::cout << "  1. 范围 for 循环: for (auto&& x : container)\n";
    std::cout << "  2. 延长临时对象生命周期\n";
    std::cout << "  3. 泛型编程中保持值类别\n";

    std::cout << "\n";
}

void demo_decltype_auto() {
    std::cout << "=== decltype(auto) ===\n";

    auto f1 = []() -> int { return 42; };
    auto f2 = []() -> int& {
        static int x = 100;
        return x;
    };
    auto f3 = []() -> const int& {
        static int x = 200;
        return x;
    };

    auto a = f1();
    decltype(auto) b = f1();
    std::cout << "auto a = f1();        => int\n";
    std::cout << "decltype(auto) b = f1(); => int\n\n";

    auto c = f2();
    decltype(auto) d = f2();
    std::cout << "auto c = f2();        => int (引用被忽略)\n";
    std::cout << "decltype(auto) d = f2(); => int& (保留引用)\n";
    d = 999;
    std::cout << "  修改 d 后: " << f2() << "\n\n";

    auto e = f3();
    decltype(auto) g = f3();
    std::cout << "auto e = f3();          => int (const引用被忽略)\n";
    std::cout << "decltype(auto) g = f3(); => const int& (保留const引用)\n";

    std::cout << "\nauto vs decltype(auto):\n";
    std::cout << "  auto: 使用模板推导规则, 忽略引用和顶层const\n";
    std::cout << "  decltype(auto): 使用 decltype 规则, 保留引用和const\n";
    std::cout << "  用于函数返回类型推导时特别有用\n";

    std::cout << "\n";
}

void demo_auto_in_range_for() {
    std::cout << "=== 范围 for 中的 auto ===\n";

    std::vector<std::string> names = {"Alice", "Bob", "Charlie"};

    std::cout << "for (auto name : names)  => 拷贝每个元素\n";
    for (auto name : names) {
        std::cout << "  " << name << "\n";
    }

    std::cout << "\nfor (auto& name : names)  => 引用, 可修改\n";
    for (auto& name : names) {
        name += "!";
    }
    for (const auto& name : names) {
        std::cout << "  " << name << "\n";
    }

    std::cout << "\nfor (const auto& name : names)  => const引用, 推荐\n";
    std::cout << "for (auto&& item : container)  => 万能引用, 通用\n";

    std::cout << "\n";
}

int main() {
    demo_auto_basics();
    demo_auto_with_const();
    demo_auto_with_reference();
    demo_auto_universal_reference();
    demo_decltype_auto();
    demo_auto_in_range_for();

    return 0;
}
