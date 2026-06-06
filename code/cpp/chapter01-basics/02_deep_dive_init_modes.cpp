/** @file 02_deep_dive_init_modes.cpp
 *  @brief C++ 初始化方式详解: 拷贝/直接/列表/值/默认初始化, 最令人烦恼的解析, 统一初始化
 *  @description 对应文档: 02-CPP/01-basics
 */

#include <iostream>
#include <string>
#include <vector>
#include <initializer_list>

// ===== 1. C++ 的多种初始化方式 =====
void demo_init_modes() {
    std::cout << "===== C++ 的多种初始化方式 =====" << std::endl;

    // (1) 拷贝初始化 (Copy Initialization): 使用 =
    int a = 42;
    std::string s1 = "hello";
    std::cout << "拷贝初始化: int a = 42; a = " << a << std::endl;

    // (2) 直接初始化 (Direct Initialization): 使用 ()
    int b(42);
    std::string s2("hello");
    std::cout << "直接初始化: int b(42); b = " << b << std::endl;

    // (3) 列表初始化 (List Initialization): 使用 {} (C++11)
    int c{42};
    std::string s3{"hello"};
    std::vector<int> v{1, 2, 3, 4, 5};
    std::cout << "列表初始化: int c{42}; c = " << c << std::endl;

    // (4) 值初始化 (Value Initialization): 使用 () 或 {} 不带值
    int d{};       // 值为 0
    int e = int(); // 值为 0
    std::cout << "值初始化: int d{}; d = " << d << std::endl;

    // (5) 默认初始化 (Default Initialization): 不带任何初始化器
    int f;  // 局部变量: 值未定义! 全局变量: 值为 0
    std::cout << "默认初始化: int f; f = " << f << " (局部变量, 值未定义!)" << std::endl;
}

// ===== 2. 列表初始化的细节 =====
void demo_list_initialization() {
    std::cout << "\n===== 列表初始化的细节 =====" << std::endl;

    // 列表初始化禁止窄化转换 (narrowing conversion)
    int a{42};      // OK
    // int b{3.14};  // 编译错误: double -> int 窄化转换
    int c = 3.14;   // OK (但会截断): 拷贝初始化允许窄化
    (void)c;
    std::cout << "列表初始化禁止窄化转换: int{3.14} 编译错误" << std::endl;
    std::cout << "拷贝初始化允许窄化: int = 3.14 结果为 " << c << std::endl;

    // 窄化转换的更多例子
    // char ch{999};     // 错误: 999 超出 char 范围
    // unsigned u{-1};   // 错误: 负数转无符号

    // 列表初始化与 std::initializer_list
    // 当构造函数接受 initializer_list 时, {} 优先匹配它
    std::vector<int> v1(5, 10);    // 5个元素, 值为10: (count, value)
    std::vector<int> v2{5, 10};    // 2个元素: 5和10: initializer_list
    std::cout << "vector(5, 10): 大小=" << v1.size() << ", 内容=";
    for (auto x : v1) std::cout << x << " ";
    std::cout << std::endl;
    std::cout << "vector{5, 10}: 大小=" << v2.size() << ", 内容=";
    for (auto x : v2) std::cout << x << " ";
    std::cout << std::endl;
}

// ===== 3. 最令人烦恼的解析 (Most Vexing Parse) =====
void demo_most_vexing_parse() {
    std::cout << "\n===== 最令人烦恼的解析 (Most Vexing Parse) =====" << std::endl;

    // C++ 的声明语法歧义: "任何可以被解析为函数声明的东西, 都会被解析为函数声明"

    // 经典案例1:
    // int x();  // 这不是变量初始化! 这是声明了一个返回 int 的函数!

    // 经典案例2:
    // std::string s();  // 不是默认构造的 string! 是函数声明!

    // 经典案例3: 带参数的版本
    struct Timer {
        Timer() {}
        Timer(int ms) {}
    };

    // Timer t1();   // 函数声明! 不是默认构造的对象!
    Timer t1{};      // 列表初始化: 确实是对象
    Timer t2;        // 也是默认构造的对象(没有括号)
    (void)t1;
    (void)t2;
    std::cout << "Timer t1();  -> 函数声明, 不是对象!" << std::endl;
    std::cout << "Timer t1{};  -> 默认构造的对象" << std::endl;
    std::cout << "Timer t2;    -> 默认构造的对象" << std::endl;

    // 更复杂的案例:
    // 假设有类 Widget, 接受一个函数指针作为参数
    struct Widget {
        Widget(int) {}
    };

    // Widget w(int());  // 声明函数 w, 接受函数指针参数, 返回 Widget
    // 而不是: 用 int() 构造 Widget 对象

    Widget w{int{}};  // 列表初始化: 明确是构造对象
    (void)w;
    std::cout << "\nWidget w(int()); -> 函数声明!" << std::endl;
    std::cout << "Widget w{int{}}; -> 构造对象" << std::endl;

    std::cout << "\n解决方案: 使用列表初始化 {} 代替 ()" << std::endl;
    std::cout << "  列表初始化不会被解析为函数声明" << std::endl;
}

// ===== 4. 统一初始化 (Uniform Initialization) =====
class Container {
public:
    Container() : data_{} {
        std::cout << "  Container() 默认构造" << std::endl;
    }
    Container(int a, int b) : data_{a, b} {
        std::cout << "  Container(" << a << ", " << b << ") 双参数构造" << std::endl;
    }
    Container(std::initializer_list<int> il) : data_(il) {
        std::cout << "  Container{initializer_list} 构造, 大小=" << il.size() << std::endl;
    }
private:
    std::vector<int> data_;
};

void demo_uniform_initialization() {
    std::cout << "\n===== 统一初始化 =====" << std::endl;

    // C++11 引入列表初始化作为"统一初始化"方案
    // 但 initializer_list 构造函数的优先级导致了一些意外

    std::cout << "构造 Container:" << std::endl;
    Container c1;           // 默认构造
    Container c2(1, 2);     // 双参数构造 (直接初始化, 不走 initializer_list)
    Container c3{1, 2};     // initializer_list 构造! 不是双参数构造!
    Container c4 = {1, 2};  // initializer_list 构造

    std::cout << "\n注意: {} 优先匹配 initializer_list 构造函数!" << std::endl;
    std::cout << "  Container{1, 2} 走 initializer_list, 不是 (int, int)" << std::endl;
    std::cout << "  Container(1, 2) 才走 (int, int) 构造" << std::endl;

    std::cout << "\n统一初始化的优缺点:" << std::endl;
    std::cout << "  优点:" << std::endl;
    std::cout << "    - 禁止窄化转换" << std::endl;
    std::cout << "    - 免疫 Most Vexing Parse" << std::endl;
    std::cout << "    - 语法统一" << std::endl;
    std::cout << "  缺点:" << std::endl;
    std::cout << "    - initializer_list 构造函数的优先匹配" << std::endl;
    std::cout << "    - 某些场景下行为出人意料" << std::endl;
}

// ===== 5. 举一反三: 初始化最佳实践 =====
void demo_init_best_practices() {
    std::cout << "\n===== 举一反三: 初始化最佳实践 =====" << std::endl;

    // 陷阱1: 未初始化的变量
    int uninitialized;
    std::cout << "陷阱1: int uninitialized; 值=" << uninitialized << " (未定义!)" << std::endl;
    std::cout << "  建议: 总是初始化变量 int x = 0; 或 int x{};" << std::endl;

    // 陷阱2: 成员初始化顺序
    struct BadOrder {
        int b_;
        int a_;
        BadOrder(int a) : a_(a), b_(a_ * 2) {}  // b_ 先初始化, 此时 a_ 未初始化!
    };
    // 成员按声明顺序初始化, 不是按初始化列表顺序!

    struct GoodOrder {
        int a_;
        int b_;
        GoodOrder(int a) : a_(a), b_(a_ * 2) {}  // 声明顺序与初始化列表一致
    };
    GoodOrder go(5);
    std::cout << "陷阱2: 成员按声明顺序初始化, 不是初始化列表顺序" << std::endl;
    std::cout << "  GoodOrder(5): a_=" << go.a_ << ", b_=" << go.b_ << std::endl;

    // 陷阱3: auto 与初始化列表
    auto v1 = {1, 2, 3};  // std::initializer_list<int>
    // auto v2 = {1, 2.0};  // 错误: 类型不一致
    std::cout << "陷阱3: auto x = {1,2,3} 推导为 initializer_list<int>" << std::endl;

    // 最佳实践总结
    std::cout << "\n初始化最佳实践:" << std::endl;
    std::cout << "  1. 总是初始化变量, 用 {} 或 = 0" << std::endl;
    std::cout << "  2. 成员声明顺序与初始化列表顺序保持一致" << std::endl;
    std::cout << "  3. 用 {} 避免窄化转换和 Most Vexing Parse" << std::endl;
    std::cout << "  4. 注意 initializer_list 构造函数的优先匹配" << std::endl;
    std::cout << "  5. 类内成员初始化器 (C++11) 优于构造函数初始化列表" << std::endl;
}

int main() {
    std::cout << "========== C++ 初始化方式详解 ==========\n" << std::endl;

    demo_init_modes();
    demo_list_initialization();
    demo_most_vexing_parse();
    demo_uniform_initialization();
    demo_init_best_practices();

    return 0;
}
