/** @file 01_deep_dive_deduction_pitfalls.cpp
 *  @brief 类型推导陷阱：auto剥离引用、initializer_list陷阱、花括号初始化、decltype(auto)边界情况
 *  @description 对应文档: 12-类型推导 | 举一反三：避开类型推导的常见陷阱
 */

#include <iostream>
#include <string>
#include <vector>
#include <initializer_list>
#include <type_traits>

void demo_auto_strips_reference() {
    std::cout << "=== auto 剥离引用陷阱 ===\n";

    std::string str = "Hello";
    std::string& ref = str;

    auto a = ref;
    a = "Modified";
    std::cout << "auto a = ref; 修改 a 后, str = \"" << str << "\"\n";
    std::cout << "  a 是副本, 不是引用! 修改 a 不影响 str\n\n";

    auto& b = ref;
    b = "Modified via b";
    std::cout << "auto& b = ref; 修改 b 后, str = \"" << str << "\"\n";
    std::cout << "  b 是引用, 修改 b 影响 str\n\n";

    const std::string& cref = str;
    auto c = cref;
    c = "New value";
    std::cout << "auto c = cref; (const引用) c = \"" << c << "\", str = \"" << str << "\"\n";
    std::cout << "  auto 剥离了 const 和引用, c 是可修改的副本\n\n";

    const auto& d = cref;
    // d = "error";  // 编译错误
    std::cout << "const auto& d = cref; d 是 const 引用\n";

    std::cout << "\n陷阱总结:\n";
    std::cout << "  auto = ref   => 副本, 不是引用!\n";
    std::cout << "  auto = cref  => 可修改副本, 不是const!\n";
    std::cout << "  需要: auto& 或 const auto&\n";

    std::cout << "\n";
}

void demo_initializer_list_trap() {
    std::cout << "=== initializer_list 陷阱 ===\n";

    auto x1 = 42;
    // auto x2 = {42};  // C++17 前: 推导为 initializer_list<int>
    // C++17: 不能直接推导, 需要显式类型
    std::initializer_list<int> x2 = {42};

    std::cout << "auto x1 = 42;           => int\n";
    std::cout << "initializer_list x2 = {42}; => initializer_list<int>\n\n";

    auto x3 = {1, 2, 3};
    std::cout << "auto x3 = {1, 2, 3};    => initializer_list<int>\n";
    std::cout << "  x3.size() = " << x3.size() << "\n\n";

    // auto x4 = {1, 2.0};  // 编译错误! 元素类型不一致
    std::cout << "auto x4 = {1, 2.0};     => 编译错误! 类型不一致\n\n";

    auto x5 = std::vector<int>{1, 2, 3};
    std::cout << "auto x5 = vector<int>{1,2,3}; => vector<int>\n";
    std::cout << "  x5.size() = " << x5.size() << "\n";

    std::cout << "\ninitializer_list 陷阱要点:\n";
    std::cout << "  1. auto + 花括号 => initializer_list\n";
    std::cout << "  2. 花括号内类型必须一致\n";
    std::cout << "  3. 想要 vector, 需要显式指定类型\n";

    std::cout << "\n";
}

void demo_brace_init_with_auto() {
    std::cout << "=== 花括号初始化与 auto ===\n";

    auto a = 42;
    auto b(42);
    auto c{42};
    auto d = {42};

    std::cout << "auto a = 42;    => int, 值=" << a << "\n";
    std::cout << "auto b(42);     => int, 值=" << b << "\n";
    std::cout << "auto c{42};     => int (C++17), 值=" << c << "\n";
    std::cout << "auto d = {42};  => initializer_list<int>\n";
    std::cout << "  d.size() = " << d.size() << "\n\n";

    // auto e = {1, 2, 3};  // initializer_list<int>
    // auto f{1, 2, 3};     // C++17 编译错误! 单元素花括号才是int
    std::cout << "auto f{1, 2, 3}; => C++17 编译错误!\n";
    std::cout << "  C++17: auto{x} 只允许单个元素\n\n";

    std::cout << "花括号初始化的规则:\n";
    std::cout << "  auto x = 42;    => int\n";
    std::cout << "  auto x = {42};  => initializer_list<int>\n";
    std::cout << "  auto x{42};     => int (C++17)\n";
    std::cout << "  auto x{1,2,3};  => 编译错误 (C++17)\n";

    std::cout << "\n";
}

void demo_decltype_auto_edge_cases() {
    std::cout << "=== decltype(auto) 边界情况 ===\n";

    auto f_copy = [](int x) -> decltype(auto) {
        return x;  // decltype(x) = int => 返回 int
    };

    auto f_ref = [](int& x) -> decltype(auto) {
        return (x);  // decltype((x)) = int& => 返回 int& (危险!)
    };

    auto f_ref_safe = [](int& x) -> decltype(auto) {
        return x;  // decltype(x) = int => 返回 int (安全)
    };

    int val = 42;
    auto r1 = f_copy(val);
    std::cout << "f_copy(val): return x; => 返回 int 副本\n";

    int& r2 = f_ref(val);
    r2 = 100;
    std::cout << "f_ref(val): return (x); => 返回 int& 引用\n";
    std::cout << "  val = " << val << " (通过引用修改)\n\n";

    std::cout << "陷阱: return (x) vs return x\n";
    std::cout << "  return x;   => decltype(x) = int => 返回副本\n";
    std::cout << "  return (x); => decltype((x)) = int& => 返回引用!\n";
    std::cout << "  多余的括号可能意外返回悬垂引用!\n\n";

    std::cout << "decltype(auto) 的安全规则:\n";
    std::cout << "  1. 不要在 return 语句中加多余括号\n";
    std::cout << "  2. 理解 decltype 对变量名和表达式的区别\n";
    std::cout << "  3. 局部变量的 return (x) 可能返回悬垂引用\n";

    std::cout << "\n";
}

void demo_auto_with_proxy_types() {
    std::cout << "=== auto 与代理类型 ===\n";

    std::vector<bool> bv = {true, false, true, true, false};

    std::cout << "vector<bool> 的陷阱:\n";
    std::cout << "  auto val = bv[0];  => 代理类型, 不是 bool!\n";
    std::cout << "  bv[0] 返回 std::vector<bool>::reference\n\n";

    auto ref = bv[0];
    bool val = bv[0];
    std::cout << "  auto ref = bv[0];  => 代理类型\n";
    std::cout << "  bool val = bv[0];  => 显式 bool\n\n";

    std::cout << "其他代理类型:\n";
    std::cout << "  vector<bool>::reference\n";
    std::cout << "  bitset::reference\n";
    std::cout << "  map::operator[] 返回的值类型\n\n";

    std::cout << "安全做法:\n";
    std::cout << "  1. 显式指定类型: bool val = bv[0];\n";
    std::cout << "  2. 使用 static_cast: static_cast<bool>(bv[0])\n";
    std::cout << "  3. 注意 auto 可能推导为代理类型\n";

    std::cout << "\n";
}

void demo_auto_ref_in_loops() {
    std::cout << "=== 循环中的 auto 引用陷阱 ===\n";

    std::vector<std::string> names = {"Alice", "Bob", "Charlie"};

    std::cout << "陷阱1: 按值拷贝 (性能问题)\n";
    for (auto name : names) {
        (void)name;
    }
    std::cout << "  for (auto name : names) => 每次拷贝 string!\n\n";

    std::cout << "正确: const 引用\n";
    for (const auto& name : names) {
        (void)name;
    }
    std::cout << "  for (const auto& name : names) => 零拷贝\n\n";

    std::cout << "陷阱2: 修改临时容器的元素\n";
    // for (auto& x : std::vector<int>{1,2,3})  // 编译错误!
    //     std::cout << x;
    std::cout << "  for (auto& x : 临时vector) => 编译错误!\n";
    std::cout << "  临时对象绑定到 const 引用, 不能用非const引用\n\n";

    std::cout << "正确: 使用 auto&& 或 const auto&\n";
    for (auto&& x : std::vector<int>{1, 2, 3}) {
        std::cout << x << " ";
    }
    std::cout << "\n  for (auto&& x : 临时vector) => OK\n";

    std::cout << "\n";
}

int main() {
    demo_auto_strips_reference();
    demo_initializer_list_trap();
    demo_brace_init_with_auto();
    demo_decltype_auto_edge_cases();
    demo_auto_with_proxy_types();
    demo_auto_ref_in_loops();

    return 0;
}
