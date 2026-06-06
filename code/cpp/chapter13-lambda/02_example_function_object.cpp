/** @file 02_example_function_object.cpp
 *  @brief 函数对象：std::function、可调用对象、operator()、std::bind、泛型lambda
 *  @description 对应文档: 13-Lambda与函数对象
 */

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <algorithm>
#include <map>

void demo_std_function() {
    std::cout << "=== std::function ===\n";

    std::function<int(int, int)> op;

    op = [](int a, int b) { return a + b; };
    std::cout << "加法: op(3, 4) = " << op(3, 4) << "\n";

    op = [](int a, int b) { return a * b; };
    std::cout << "乘法: op(3, 4) = " << op(3, 4) << "\n";

    struct Divider {
        int operator()(int a, int b) const { return a / b; }
    };
    op = Divider();
    std::cout << "除法: op(10, 3) = " << op(10, 3) << "\n";

    int factor = 10;
    op = [factor](int a, int b) { return (a + b) * factor; };
    std::cout << "带捕获: op(3, 4) = " << op(3, 4) << "\n";

    std::cout << "\nstd::function 的特点:\n";
    std::cout << "  1. 可以存储任何可调用对象\n";
    std::cout << "  2. 类型擦除, 统一接口\n";
    std::cout << "  3. 有运行时开销 (比直接调用慢)\n";
    std::cout << "  4. 适合需要存储/传递可调用对象的场景\n";

    std::cout << "\n";
}

class Adder {
public:
    Adder(int base) : base_(base) {}
    int operator()(int x) const { return x + base_; }
    int operator()(int x, int y) const { return x + y + base_; }
private:
    int base_;
};

void demo_callable_objects() {
    std::cout << "=== 可调用对象 ===\n";

    std::cout << "1. 函数指针:\n";
    int (*fp)(int, int) = [](int a, int b) -> int { return a + b; };
    std::cout << "   fp(3, 4) = " << fp(3, 4) << "\n\n";

    std::cout << "2. 函数对象 (重载 operator()):\n";
    Adder adder(100);
    std::cout << "   adder(5) = " << adder(5) << "\n";
    std::cout << "   adder(5, 10) = " << adder(5, 10) << "\n\n";

    std::cout << "3. Lambda:\n";
    auto lambda = [](int a, int b) { return a - b; };
    std::cout << "   lambda(10, 3) = " << lambda(10, 3) << "\n\n";

    std::cout << "4. 类成员函数指针:\n";
    struct Calc {
        int multiply(int a, int b) { return a * b; }
    };
    Calc calc;
    int (Calc::*mfp)(int, int) = &Calc::multiply;
    std::cout << "   (calc.*mfp)(3, 4) = " << (calc.*mfp)(3, 4) << "\n";

    std::cout << "\n所有可调用对象都可以存入 std::function\n";

    std::cout << "\n";
}

void demo_operator_overload() {
    std::cout << "=== operator() 重载 ===\n";

    class Multiplier {
    public:
        Multiplier(int factor) : factor_(factor) {}
        int operator()(int x) const { return x * factor_; }
        double operator()(double x) const { return x * factor_; }
        std::string operator()(const std::string& s) const {
            std::string result;
            for (size_t i = 0; i < static_cast<size_t>(factor_); ++i) {
                result += s;
            }
            return result;
        }
    private:
        int factor_;
    };

    Multiplier m3(3);
    std::cout << "m3(5) = " << m3(5) << "\n";
    std::cout << "m3(2.5) = " << m3(2.5) << "\n";
    std::cout << "m3(\"Hi\") = " << m3("Hi") << "\n";

    std::cout << "\n函数对象的优势:\n";
    std::cout << "  1. 可以有状态 (成员变量)\n";
    std::cout << "  2. 可以重载多个调用版本\n";
    std::cout << "  3. 可以内联优化\n";
    std::cout << "  4. STL 算法中广泛使用\n";

    std::cout << "\n";
}

void demo_std_bind() {
    std::cout << "=== std::bind ===\n";

    auto add = [](int a, int b, int c) { return a + b + c; };

    auto add10 = std::bind(add, 10, std::placeholders::_1, std::placeholders::_2);
    std::cout << "add10(20, 30) = " << add10(20, 30) << "\n";

    auto add_all_10 = std::bind(add, 10, 20, std::placeholders::_1);
    std::cout << "add_all_10(30) = " << add_all_10(30) << "\n";

    auto reverse_add = std::bind(add, std::placeholders::_2, std::placeholders::_1, 0);
    std::cout << "reverse_add(3, 7) = " << reverse_add(3, 7) << "\n";

    std::cout << "\nstd::bind 的局限:\n";
    std::cout << "  1. 可读性差\n";
    std::cout << "  2. 占位符容易混淆\n";
    std::cout << "  3. 某些场景行为不直观\n";
    std::cout << "  4. 现代 C++ 推荐用 lambda 替代\n";

    std::cout << "\n";
}

void demo_generic_lambda() {
    std::cout << "=== 泛型 Lambda (C++14) ===\n";

    auto print = [](const auto& value) {
        std::cout << value << "\n";
    };

    print(42);
    print(3.14);
    print(std::string("Hello"));
    print("C风格字符串");

    auto add = [](auto a, auto b) {
        return a + b;
    };

    std::cout << "\nadd(1, 2) = " << add(1, 2) << "\n";
    std::cout << "add(1.5, 2.5) = " << add(1.5, 2.5) << "\n";
    std::cout << "add(std::string(\"Hello\"), std::string(\" World\")) = "
              << add(std::string("Hello"), std::string(" World")) << "\n";

    std::cout << "\n泛型 lambda 的本质:\n";
    std::cout << "  auto 参数的 lambda 等价于模板 operator()\n";
    std::cout << "  struct Lambda {\n";
    std::cout << "    template<typename T>\n";
    std::cout << "    auto operator()(const T& value) const { ... }\n";
    std::cout << "  };\n";

    std::cout << "\n";
}

void demo_callback_pattern() {
    std::cout << "=== 回调模式 ===\n";

    using Callback = std::function<void(const std::string&)>;

    class EventSystem {
    public:
        void subscribe(const std::string& event, Callback cb) {
            handlers_[event].push_back(std::move(cb));
        }

        void emit(const std::string& event, const std::string& data) {
            auto it = handlers_.find(event);
            if (it != handlers_.end()) {
                for (const auto& handler : it->second) {
                    handler(data);
                }
            }
        }
    private:
        std::map<std::string, std::vector<Callback>> handlers_;
    };

    EventSystem es;
    es.subscribe("click", [](const std::string& data) {
        std::cout << "点击事件: " << data << "\n";
    });
    es.subscribe("click", [](const std::string& data) {
        std::cout << "日志: 用户点击了 " << data << "\n";
    });
    es.subscribe("hover", [](const std::string& data) {
        std::cout << "悬停事件: " << data << "\n";
    });

    es.emit("click", "按钮A");
    es.emit("hover", "菜单项B");

    std::cout << "\n";
}

int main() {
    demo_std_function();
    demo_callable_objects();
    demo_operator_overload();
    demo_std_bind();
    demo_generic_lambda();
    demo_callback_pattern();

    return 0;
}
