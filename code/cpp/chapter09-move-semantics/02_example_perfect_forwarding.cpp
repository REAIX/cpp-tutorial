/** @file 02_example_perfect_forwarding.cpp
 *  @brief 完美转发：std::forward、万能引用、模板中的转发引用、std::forward用法
 *  @description 对应文档: 09-移动语义与完美转发
 */

#include <iostream>
#include <string>
#include <utility>
#include <memory>

class Widget {
public:
    Widget(int id) : id_(id), name_("默认") {
        std::cout << "Widget(int) 构造, id=" << id_ << "\n";
    }
    Widget(int id, std::string name) : id_(id), name_(std::move(name)) {
        std::cout << "Widget(int, string) 构造, id=" << id_ << ", name=" << name_ << "\n";
    }
    ~Widget() {
        std::cout << "Widget(" << id_ << ") 析构\n";
    }
    Widget(const Widget& other) : id_(other.id_), name_(other.name_) {
        std::cout << "Widget 拷贝构造, id=" << id_ << "\n";
    }
    Widget(Widget&& other) noexcept : id_(other.id_), name_(std::move(other.name_)) {
        other.id_ = 0;
        std::cout << "Widget 移动构造, id=" << id_ << "\n";
    }
private:
    int id_;
    std::string name_;
};

void process_lvalue(const std::string& s) {
    std::cout << "处理左值: " << s << "\n";
}

void process_rvalue(std::string&& s) {
    std::cout << "处理右值: " << s << "\n";
}

void demo_universal_reference() {
    std::cout << "=== 万能引用 (Universal Reference) ===\n";

    auto identify = [](auto&& x) -> std::string {
        using T = decltype(x);
        if constexpr (std::is_lvalue_reference_v<T>) {
            return "左值引用";
        } else {
            return "右值引用";
        }
    };

    int a = 10;
    const int b = 20;

    std::cout << "auto&& 接收左值 a: " << identify(a) << "\n";
    std::cout << "auto&& 接收右值 42: " << identify(42) << "\n";
    std::cout << "auto&& 接收 const 左值 b: " << identify(b) << "\n";
    std::cout << "auto&& 接收 a+b: " << identify(a + b) << "\n";

    std::cout << "\n万能引用的规则:\n";
    std::cout << "  T&& 其中 T 需要推导时, 才是万能引用\n";
    std::cout << "  接收左值时, T 推导为 int&, 参数类型为 int& && => int&\n";
    std::cout << "  接收右值时, T 推导为 int, 参数类型为 int&&\n";

    std::cout << "\n";
}

template<typename T>
void wrapper_without_forward(T&& arg) {
    process_lvalue(arg);
}

template<typename T>
void wrapper_with_forward(T&& arg) {
    process_lvalue(std::forward<T>(arg));
}

void demo_forward_basics() {
    std::cout << "=== std::forward 基础 ===\n";

    std::string lv = "我是左值";

    std::cout << "不使用 forward (始终传为左值):\n";
    wrapper_without_forward(lv);
    wrapper_without_forward(std::string("我是右值"));

    std::cout << "\n使用 forward (保持值类别):\n";
    wrapper_with_forward(lv);
    wrapper_with_forward(std::string("我是右值"));

    std::cout << "\nstd::forward 的原理:\n";
    std::cout << "  T 为左值引用时: forward 返回左值引用\n";
    std::cout << "  T 为非引用时: forward 返回右值引用\n";
    std::cout << "  与 std::move 不同, forward 是条件性转换\n";

    std::cout << "\n";
}

template<typename T>
void perfect_wrapper(T&& arg) {
    if constexpr (std::is_lvalue_reference_v<T>) {
        process_lvalue(std::forward<T>(arg));
    } else {
        process_rvalue(std::forward<T>(arg));
    }
}

void demo_perfect_forwarding() {
    std::cout << "=== 完美转发实战 ===\n";

    std::string lv = "左值字符串";

    std::cout << "传递左值:\n";
    perfect_wrapper(lv);

    std::cout << "\n传递右值:\n";
    perfect_wrapper(std::string("右值字符串"));

    std::cout << "\n完美转发的含义:\n";
    std::cout << "  完美 = 保持参数的值类别 (左值/右值) 和 const 属性\n";
    std::cout << "  转发 = 将参数传递给另一个函数\n";

    std::cout << "\n";
}

template<typename T, typename... Args>
std::unique_ptr<T> make_widget(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

void demo_forwarding_in_factory() {
    std::cout << "=== 完美转发在工厂函数中的应用 ===\n";

    auto w1 = make_widget<Widget>(1);
    auto w2 = make_widget<Widget>(2, std::string("高级Widget"));

    std::string name = "命名Widget";
    auto w3 = make_widget<Widget>(3, name);
    auto w4 = make_widget<Widget>(4, std::move(name));

    std::cout << "\nstd::make_unique 就是使用完美转发实现的:\n";
    std::cout << "  template<typename T, typename... Args>\n";
    std::cout << "  unique_ptr<T> make_unique(Args&&... args) {\n";
    std::cout << "    return unique_ptr<T>(new T(forward<Args>(args)...));\n";
    std::cout << "  }\n";

    std::cout << "\n";
}

void demo_forward_vs_move() {
    std::cout << "=== std::forward vs std::move ===\n";

    std::cout << "std::move:\n";
    std::cout << "  - 无条件将参数转为右值引用\n";
    std::cout << "  - 用于: 明确要移动的场景\n";
    std::cout << "  - 示例: vec.push_back(std::move(item));\n\n";

    std::cout << "std::forward:\n";
    std::cout << "  - 条件性转换, 保持原始值类别\n";
    std::cout << "  - 用于: 万能引用参数的转发\n";
    std::cout << "  - 示例: factory(std::forward<Args>(args)...)\n\n";

    std::cout << "使用规则:\n";
    std::cout << "  - 对万能引用参数使用 std::forward\n";
    std::cout << "  - 对非万能引用(右值引用)参数使用 std::move\n";
    std::cout << "  - 不要在万能引用上使用 std::move (会错误移动左值)\n";

    std::cout << "\n";
}

int main() {
    demo_universal_reference();
    demo_forward_basics();
    demo_perfect_forwarding();
    demo_forwarding_in_factory();
    demo_forward_vs_move();

    return 0;
}
