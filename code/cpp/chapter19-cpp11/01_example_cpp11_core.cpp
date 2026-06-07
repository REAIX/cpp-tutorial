/** @file 01_example_cpp11_core.cpp
 *  @brief C++11核心特性：auto, range-for, nullptr, enum class, static_assert, long long, constexpr
 *  @description 对应文档: 02-CPP/20-cpp11 | 演示C++11语言核心改进
 *  编译命令: g++ -std=c++20 01_example_cpp11_core.cpp -o 01_example_cpp11_core
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <type_traits>
#include <cstdint>

constexpr int square(int x) { return x * x; }
constexpr double pi = 3.141592653589793;

void func_overload(int value) { std::cout << "  调用 func(int): " << value << "\n"; }
void func_overload(int* ptr) { std::cout << "  调用 func(int*): " << (ptr ? "非空" : "空") << "\n"; }

enum class Color { Red, Green, Blue };
enum class Size { Small, Medium, Large };
enum class Direction : uint8_t { Up = 0, Down = 1, Left = 2, Right = 3 };

void demo_auto() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  auto —— 自动类型推导\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto i = 42;
    auto d = 3.14;
    auto s = std::string("hello");
    auto v = std::vector<int>{1, 2, 3};

    std::cout << "基本类型推导:\n";
    std::cout << "  auto i = 42;       → int\n";
    std::cout << "  auto d = 3.14;     → double\n";
    std::cout << "  auto s = string(); → std::string\n\n";

    std::cout << "迭代器简化:\n";
    std::map<std::string, int> scores = {{"张三", 95}, {"李四", 87}, {"王五", 92}};

    std::cout << "  C++03: for (std::map<std::string, int>::const_iterator it = scores.begin(); ...)\n";
    std::cout << "  C++11: for (auto it = scores.begin(); ...)\n\n";

    for (auto it = scores.begin(); it != scores.end(); ++it) {
        std::cout << "  " << it->first << ": " << it->second << "\n";
    }

    std::cout << "\nauto 与引用/const:\n";
    int x = 10;
    auto a = x;
    auto& b = x;
    const auto& c = x;
    auto* p = &x;

    b = 20;
    std::cout << "  auto a = x;    → 值拷贝, a=" << a << "\n";
    std::cout << "  auto& b = x;   → 引用, x=" << x << "\n";
    std::cout << "  const auto& c  → 常引用\n";
    std::cout << "  auto* p = &x   → 指针, *p=" << *p << "\n";

    std::cout << "\nauto 注意事项:\n";
    std::cout << "  - auto 推导为值类型时会拷贝\n";
    std::cout << "  - auto&& 可转发引用(万能引用)\n";
    std::cout << "  - auto 不能用于函数参数(C++20前)\n";
    std::cout << "  - auto 不能用于非静态成员变量\n";
}

void demo_range_for() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  range-for —— 范围for循环\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::vector<int> nums = {1, 2, 3, 4, 5};

    std::cout << "只读遍历:\n  ";
    for (auto x : nums) std::cout << x << " ";
    std::cout << "\n\n";

    std::cout << "修改元素:\n  ";
    for (auto& x : nums) x *= 2;
    for (const auto& x : nums) std::cout << x << " ";
    std::cout << "\n\n";

    std::cout << "遍历map:\n";
    std::map<std::string, int> ages = {{"张三", 25}, {"李四", 30}};
    for (const auto& [name, age] : ages) {
        std::cout << "  " << name << ": " << age << "岁\n";
    }

    std::cout << "\n遍历字符串:\n  ";
    std::string text = "Hello";
    for (char c : text) std::cout << c << "-";
    std::cout << "\n\n";

    std::cout << "初始化列表:\n  ";
    for (auto x : {10, 20, 30}) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "\nrange-for 等价于:\n";
    std::cout << "  for (auto it = range.begin(); it != range.end(); ++it)\n";
    std::cout << "  需要类型提供 begin() 和 end()\n";
}

void demo_nullptr() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  nullptr —— 空指针字面量\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "NULL 的问题:\n";
    std::cout << "  NULL 在C++中通常定义为 0 (整数)\n";
    std::cout << "  func(NULL) 可能调用 func(int) 而非 func(int*)\n\n";

    func_overload(0);
    func_overload(nullptr);

    std::cout << "\nnullptr 的优势:\n";
    std::cout << "  - 类型为 std::nullptr_t，不是整数\n";
    std::cout << "  - 可隐式转换为任意指针类型\n";
    std::cout << "  - 不能转换为整数类型\n";
    std::cout << "  - 消除了重载决议的歧义\n";

    int* p1 = nullptr;
    int* p2 = 0;
    std::cout << "\n  p1 == p2: " << (p1 == p2) << "\n";
    std::cout << "  p1 == nullptr: " << (p1 == nullptr) << "\n";
}

void demo_enum_class() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  enum class —— 作用域枚举\n";
    std::cout << "═══════════════════════════════════════\n\n";

    Color c = Color::Red;

    std::cout << "enum class vs enum:\n";
    std::cout << "  enum: 枚举值泄漏到外层作用域\n";
    std::cout << "  enum class: 枚举值限定在枚举作用域内\n\n";

    std::cout << "  Color::Red 访问需要作用域限定\n";
    std::cout << "  不能隐式转换为 int\n";

    int color_val = static_cast<int>(c);
    std::cout << "  static_cast<int>(Color::Red) = " << color_val << "\n";

    std::cout << "\n  指定底层类型: enum class Direction : uint8_t\n";
    std::cout << "  sizeof(Direction) = " << sizeof(Direction) << "\n";

    auto to_string = [](Color color) -> std::string {
        switch (color) {
            case Color::Red:   return "红色";
            case Color::Green: return "绿色";
            case Color::Blue:  return "蓝色";
        }
        return "未知";
    };
    std::cout << "\n  Color::Red → " << to_string(Color::Red) << "\n";
}

void demo_static_assert() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  static_assert —— 编译期断言\n";
    std::cout << "═══════════════════════════════════════\n\n";

    static_assert(sizeof(int) >= 4, "int 必须至少32位");
    static_assert(sizeof(void*) >= 4, "指针必须至少32位");

    std::cout << "static_assert(常量表达式, 错误消息)\n";
    std::cout << "  在编译期检查条件，失败则编译错误\n\n";

    std::cout << "常见用途:\n";
    std::cout << "  1. 类型大小检查:\n";
    std::cout << "     static_assert(sizeof(int) == 4);\n\n";
    std::cout << "  2. 类型特征检查:\n";
    std::cout << "     static_assert(std::is_integral<int>::value);\n\n";
    std::cout << "  3. 模板约束:\n";
    std::cout << "     static_assert(std::is_base_of<Base, Derived>::value);\n\n";

    static_assert(std::is_integral<int>::value, "int 应该是整数类型");
    static_assert(std::is_pointer<int*>::value, "int* 应该是指针类型");
    std::cout << "  所有 static_assert 检查通过!\n";
}

void demo_long_long_and_constexpr() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  long long 与 constexpr\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "long long —— 至少64位整数:\n";
    long long big = 9223372036854775807LL;
    std::cout << "  long long 最大值: " << big << "\n";
    std::cout << "  sizeof(long long) = " << sizeof(long long) << " 字节\n\n";

    std::cout << "constexpr —— 编译期常量:\n";
    constexpr int val = square(5);
    std::cout << "  constexpr square(5) = " << val << "\n";
    std::cout << "  编译期计算，运行时零开销\n\n";

    constexpr double area = pi * 10.0 * 10.0;
    std::cout << "  constexpr pi = " << pi << "\n";
    std::cout << "  constexpr area(pi*10*10) = " << area << "\n\n";

    std::cout << "constexpr vs const:\n";
    std::cout << "  const    —— 运行期不可修改(可能运行期初始化)\n";
    std::cout << "  constexpr —— 编译期常量(保证编译期计算)\n";
    std::cout << "  constexpr 一定是 const，反之不然\n";
}

int main() {
    demo_auto();
    demo_range_for();
    demo_nullptr();
    demo_enum_class();
    demo_static_assert();
    demo_long_long_and_constexpr();
    return 0;
}
