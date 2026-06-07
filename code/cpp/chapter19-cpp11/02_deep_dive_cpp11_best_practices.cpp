/** @file 02_deep_dive_cpp11_best_practices.cpp
 *  @brief 现代C++风格指南、C++11日常特性、常见C++11错误
 *  @description 对应文档: 02-CPP/20-cpp11 | 举一反三：掌握C++11最佳实践，避免常见陷阱
 *  编译命令: g++ -std=c++20 02_deep_dive_cpp11_best_practices.cpp -o 02_deep_dive_cpp11_best_practices
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <algorithm>
#include <functional>
#include <map>

void demo_daily_features() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  C++11 每日必用特性\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. auto —— 减少冗余类型声明\n";
    std::map<std::string, std::vector<int>> data;
    auto it = data.find("key");
    std::cout << "   auto it = map.find() 代替冗长的迭代器类型\n\n";

    std::cout << "2. range-for —— 简洁遍历\n";
    std::vector<int> v = {1, 2, 3, 4, 5};
    for (const auto& x : v) std::cout << x << " ";
    std::cout << "\n\n";

    std::cout << "3. 初始化列表 —— 统一初始化\n";
    std::vector<int> vi = {1, 2, 3};
    std::map<std::string, int> m = {{"a", 1}, {"b", 2}};
    std::cout << "   vector 和 map 都可以用 {} 初始化\n\n";

    std::cout << "4. nullptr —— 安全的空指针\n";
    int* p = nullptr;
    std::cout << "   用 nullptr 代替 NULL 和 0\n\n";

    std::cout << "5. enum class —— 类型安全枚举\n";
    std::cout << "   用 enum class 代替 匿名enum\n\n";

    std::cout << "6. override —— 防止虚函数签名错误\n";
    std::cout << "   派生类虚函数始终标注 override\n\n";

    std::cout << "7. 智能指针 —— 自动内存管理\n";
    auto sp = std::make_shared<std::string>("shared");
    auto up = std::make_unique<int>(42);
    std::cout << "   *sp = " << *sp << ", *up = " << *up << "\n\n";

    std::cout << "8. lambda —— 内联函数对象\n";
    auto add = [](int a, int b) { return a + b; };
    std::cout << "   add(3,4) = " << add(3, 4) << "\n";
}

void demo_move_semantics_best_practices() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  移动语义最佳实践\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 按值返回(依赖RVO/移动):\n";
    auto make_vector = []() -> std::vector<int> {
        std::vector<int> v(1000, 42);
        return v;
    };
    auto result = make_vector();
    std::cout << "   返回的vector大小: " << result.size() << "\n\n";

    std::cout << "2. 按值传参 + 移动:\n";
    auto set_name = [](std::string name) {
        std::string member = std::move(name);
        return member;
    };
    std::string my_name = "张三";
    std::string copy_name = set_name(my_name);
    std::cout << "   传入左值: my_name=\"" << my_name << "\", copy_name=\"" << copy_name << "\"\n";

    std::string other_name = "李四";
    std::string moved_name = set_name(std::move(other_name));
    std::cout << "   传入右值: other_name=\"" << other_name << "\", moved_name=\"" << moved_name << "\"\n\n";

    std::cout << "3. 移动后对象状态:\n";
    std::vector<int> source = {1, 2, 3, 4, 5};
    std::vector<int> target = std::move(source);
    std::cout << "   移动后 source.size()=" << source.size() << " (有效但未指定)\n";
    std::cout << "   target.size()=" << target.size() << "\n";
    std::cout << "   规则: 移动后的对象只能被析构或重新赋值\n";
}

void demo_common_mistakes() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  C++11 常见错误\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. auto 推导与引用:\n";
    int x = 10;
    auto a = x;
    auto& b = x;
    a = 20;
    std::cout << "   auto a = x; a=20 → x=" << x << " (a是拷贝)\n";
    b = 30;
    std::cout << "   auto& b = x; b=30 → x=" << x << " (b是引用)\n\n";

    std::cout << "2. lambda 捕获陷阱:\n";
    int val = 42;
    auto capture_value = [val]() { return val; };
    auto capture_ref = [&val]() { return val; };
    val = 100;
    std::cout << "   值捕获: " << capture_value() << " (捕获时拷贝)\n";
    std::cout << "   引用捕获: " << capture_ref() << " (引用当前值)\n";
    std::cout << "   警告: 引用捕获可能悬空!\n\n";

    std::cout << "3. unique_ptr 与容器:\n";
    std::cout << "   错误: vector<unique_ptr> 不能拷贝\n";
    std::cout << "   正确: 使用移动语义 vector.push_back(move(ptr))\n\n";

    std::vector<std::unique_ptr<int>> ptrs;
    ptrs.push_back(std::make_unique<int>(1));
    ptrs.push_back(std::make_unique<int>(2));
    std::cout << "   vector<unique_ptr> 大小: " << ptrs.size() << "\n\n";

    std::cout << "4. override 遗漏:\n";
    std::cout << "   不写 override 可能导致签名不匹配但不报错\n";
    std::cout << "   始终在派生类虚函数上写 override\n\n";

    std::cout << "5. 初始化列表与窄化转换:\n";
    std::cout << "   int x{3.14};  // 编译错误(窄化)\n";
    std::cout << "   int x(3.14);  // 编译通过(截断)\n";
    std::cout << "   {} 初始化更安全\n\n";

    std::cout << "6. shared_ptr 循环引用:\n";
    std::cout << "   A持有shared_ptr<B>, B持有shared_ptr<A>\n";
    std::cout << "   解决: 一方使用 weak_ptr\n";
}

void demo_modern_cpp_style() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  现代C++风格指南\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "资源管理:\n";
    std::cout << "  ✓ 使用 RAII 管理所有资源\n";
    std::cout << "  ✓ 使用智能指针代替裸指针\n";
    std::cout << "  ✓ 使用 make_shared/make_unique\n";
    std::cout << "  ✗ 避免手动 new/delete\n";
    std::cout << "  ✗ 避免裸指针拥有所有权\n\n";

    std::cout << "函数设计:\n";
    std::cout << "  ✓ 使用 lambda 代替局部函数对象\n";
    std::cout << "  ✓ 使用 std::function 存储可调用对象\n";
    std::cout << "  ✓ 使用 override/final 标注虚函数\n";
    std::cout << "  ✓ 使用 default/delete 控制特殊成员\n";
    std::cout << "  ✓ 使用 constexpr 定义编译期常量\n\n";

    std::cout << "类型安全:\n";
    std::cout << "  ✓ 使用 enum class 代替 enum\n";
    std::cout << "  ✓ 使用 nullptr 代替 NULL\n";
    std::cout << "  ✓ 使用 static_assert 编译期检查\n";
    std::cout << "  ✓ 使用 {} 初始化防止窄化\n";
    std::cout << "  ✓ 使用 auto 避免类型不匹配\n\n";

    std::cout << "性能:\n";
    std::cout << "  ✓ 使用移动语义减少拷贝\n";
    std::cout << "  ✓ 使用 emplace 代替 insert/push_back\n";
    std::cout << "  ✓ 按值返回依赖RVO\n";
    std::cout << "  ✓ 使用 constexpr 编译期计算\n\n";

    std::vector<std::string> names;
    names.emplace_back("张三");
    names.emplace_back("李四");
    std::cout << "  emplace_back 直接构造，避免临时对象\n";
    std::cout << "  names 大小: " << names.size() << "\n";
}

int main() {
    demo_daily_features();
    demo_move_semantics_best_practices();
    demo_common_mistakes();
    demo_modern_cpp_style();
    return 0;
}
