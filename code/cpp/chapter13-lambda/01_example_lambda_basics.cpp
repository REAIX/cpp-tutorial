/** @file 01_example_lambda_basics.cpp
 *  @brief Lambda基础：语法、捕获模式、mutable lambda
 *  @description 对应文档: 13-Lambda与函数对象
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <memory>

void demo_lambda_syntax() {
    std::cout << "=== Lambda 基本语法 ===\n";

    auto hello = []() {
        std::cout << "Hello from lambda!\n";
    };
    hello();

    auto add = [](int a, int b) -> int {
        return a + b;
    };
    std::cout << "add(3, 4) = " << add(3, 4) << "\n";

    auto multiply = [](auto a, auto b) {
        return a * b;
    };
    std::cout << "multiply(3, 4) = " << multiply(3, 4) << "\n";
    std::cout << "multiply(2.5, 4) = " << multiply(2.5, 4) << "\n";

    std::cout << "\nLambda 语法: [捕获](参数) -> 返回类型 { 函数体 }\n";
    std::cout << "  [捕获]    - 捕获外部变量\n";
    std::cout << "  (参数)    - 函数参数 (可省略)\n";
    std::cout << "  -> 返回类型 - 可省略, 自动推导\n";
    std::cout << "  { 函数体 } - 函数实现\n";

    std::cout << "\n";
}

void demo_capture_by_value() {
    std::cout << "=== 值捕获 [=] 和 [x] ===\n";

    int x = 10;
    int y = 20;

    auto capture_x = [x]() {
        std::cout << "捕获 x = " << x << "\n";
        // x = 100;  // 编译错误! 值捕获默认是 const
    };
    capture_x();

    auto capture_all = [=]() {
        std::cout << "捕获所有: x=" << x << ", y=" << y << "\n";
    };
    capture_all();

    std::cout << "修改 x 后...\n";
    x = 999;
    capture_x();
    std::cout << "值捕获: lambda 保存的是捕获时的副本, 不受外部修改影响\n";

    std::cout << "\n";
}

void demo_capture_by_reference() {
    std::cout << "=== 引用捕获 [&] 和 [&x] ===\n";

    int x = 10;
    int y = 20;

    auto capture_ref_x = [&x]() {
        x += 5;
        std::cout << "引用捕获 x = " << x << "\n";
    };
    capture_ref_x();
    std::cout << "调用后 x = " << x << " (被 lambda 修改)\n";

    auto capture_all_ref = [&]() {
        x += 10;
        y += 10;
        std::cout << "引用捕获所有: x=" << x << ", y=" << y << "\n";
    };
    capture_all_ref();
    std::cout << "调用后 x=" << x << ", y=" << y << "\n";

    std::cout << "\n引用捕获的风险:\n";
    std::cout << "  如果 lambda 比捕获的变量活得更久, 引用会悬垂!\n";
    std::cout << "  不要返回引用捕获局部变量的 lambda\n";

    std::cout << "\n";
}

void demo_mixed_capture() {
    std::cout << "=== 混合捕获 ===\n";

    int a = 10, b = 20, c = 30;

    auto mixed = [a, &b, &c]() {
        std::cout << "a(值)=" << a << ", b(引用)=" << b << ", c(引用)=" << c << "\n";
        b += 100;
        c += 200;
    };
    mixed();
    std::cout << "调用后: b=" << b << ", c=" << c << "\n";

    auto default_value_except = [=, &c]() {
        std::cout << "默认值捕获, 但 c 引用捕获: c=" << c << "\n";
        c += 500;
    };
    default_value_except();
    std::cout << "调用后 c=" << c << "\n";

    auto default_ref_except = [&, a]() {
        std::cout << "默认引用捕获, 但 a 值捕获: a=" << a << "\n";
        b += 1000;
    };
    default_ref_except();
    std::cout << "调用后 b=" << b << "\n";

    std::cout << "\n混合捕获语法:\n";
    std::cout << "  [=, &x, &y]  - 默认值捕获, x/y 引用捕获\n";
    std::cout << "  [&, x, y]    - 默认引用捕获, x/y 值捕获\n";
    std::cout << "  注意: 默认捕获和例外不能冲突\n";

    std::cout << "\n";
}

void demo_this_capture() {
    std::cout << "=== this 捕获 ===\n";

    class Counter {
    public:
        Counter(int init) : count_(init) {}

        auto get_incrementer() {
            return [this]() {
                ++count_;
                return count_;
            };
        }

        auto get_incrementer_safe() {
            return [self = *this]() mutable {
                ++self.count_;
                return self.count_;
            };
        }

        int count() const { return count_; }
    private:
        int count_;
    };

    Counter c(0);
    auto inc = c.get_incrementer();
    std::cout << "incrementer: " << inc() << "\n";
    std::cout << "incrementer: " << inc() << "\n";
    std::cout << "Counter: " << c.count() << " (通过 this 修改)\n\n";

    std::cout << "this 捕获要点:\n";
    std::cout << "  [this]  - 捕获 this 指针 (引用语义)\n";
    std::cout << "  [*this] - C++17, 拷贝捕获整个对象 (值语义)\n";
    std::cout << "  [=] 在成员函数中也只捕获 this (不是所有成员的副本)\n";

    std::cout << "\n";
}

void demo_mutable_lambda() {
    std::cout << "=== mutable Lambda ===\n";

    int x = 10;

    auto counter = [x]() mutable {
        ++x;
        return x;
    };

    std::cout << "counter() = " << counter() << "\n";
    std::cout << "counter() = " << counter() << "\n";
    std::cout << "counter() = " << counter() << "\n";
    std::cout << "外部 x = " << x << " (不受 mutable lambda 影响)\n\n";

    std::cout << "mutable 的含义:\n";
    std::cout << "  默认: lambda 的 operator() 是 const\n";
    std::cout << "  mutable: 去除 const, 可以修改值捕获的变量\n";
    std::cout << "  修改的是 lambda 对象内部的副本, 不影响外部\n";

    std::cout << "\n";
}

void demo_init_capture() {
    std::cout << "=== 初始化捕获 (C++14) ===\n";

    auto ptr = std::make_unique<int>(42);

    auto take_ptr = [p = std::move(ptr)]() {
        std::cout << "移动捕获: *p = " << *p << "\n";
    };
    take_ptr();
    std::cout << "ptr 移动后: " << (ptr ? "非空" : "空") << "\n\n";

    int x = 10;
    auto capture_expr = [sum = x + 20, product = x * 3]() {
        std::cout << "表达式捕获: sum=" << sum << ", product=" << product << "\n";
    };
    capture_expr();

    std::cout << "\n初始化捕获的语法:\n";
    std::cout << "  [name = expr]  - 用 expr 初始化 name\n";
    std::cout << "  可以移动捕获 (move capture)\n";
    std::cout << "  可以捕获表达式的结果\n";

    std::cout << "\n";
}

int main() {
    demo_lambda_syntax();
    demo_capture_by_value();
    demo_capture_by_reference();
    demo_mixed_capture();
    demo_this_capture();
    demo_mutable_lambda();
    demo_init_capture();

    return 0;
}
