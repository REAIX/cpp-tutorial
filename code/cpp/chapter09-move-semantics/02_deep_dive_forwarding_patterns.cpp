/** @file 02_deep_dive_forwarding_patterns.cpp
 *  @brief 完美转发进阶：变参转发、工厂模式、常见转发错误
 *  @description 对应文档: 09-移动语义与完美转发 | 举一反三：掌握完美转发的实战模式
 */

#include <iostream>
#include <string>
#include <utility>
#include <memory>
#include <vector>
#include <tuple>
#include <type_traits>

class Widget {
public:
    Widget() : id_(0), name_("默认") {
        std::cout << "Widget() 默认构造\n";
    }
    Widget(int id) : id_(id), name_("默认") {
        std::cout << "Widget(" << id_ << ") 构造\n";
    }
    Widget(int id, std::string name) : id_(id), name_(std::move(name)) {
        std::cout << "Widget(" << id_ << ", \"" << name_ << "\") 构造\n";
    }
    Widget(const Widget& other) : id_(other.id_), name_(other.name_) {
        std::cout << "Widget 拷贝构造, id=" << id_ << "\n";
    }
    Widget(Widget&& other) noexcept : id_(other.id_), name_(std::move(other.name_)) {
        other.id_ = 0;
        std::cout << "Widget 移动构造, id=" << id_ << "\n";
    }
    ~Widget() {
        std::cout << "Widget(" << id_ << ") 析构\n";
    }
    int id() const { return id_; }
private:
    int id_;
    std::string name_;
};

template<typename T, typename... Args>
std::unique_ptr<T> make_object(Args&&... args) {
    std::cout << "创建对象, 传递 " << sizeof...(args) << " 个参数\n";
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

struct Product {
    std::string category;
    int id;
    double price;
    Product(std::string cat, int i, double p)
        : category(std::move(cat)), id(i), price(p) {
        std::cout << "Product(\"" << category << "\", " << id << ", " << price << ")\n";
    }
};

template<typename T, typename... Args>
std::shared_ptr<T> create_shared(Args&&... args) {
    return std::shared_ptr<T>(new T(std::forward<Args>(args)...));
}

template<typename... Args>
auto forward_as_tuple_demo(Args&&... args) {
    auto tup = std::forward_as_tuple(std::forward<Args>(args)...);
    std::cout << "tuple 大小: " << std::tuple_size_v<decltype(tup)> << "\n";
    return tup;
}

template<typename T>
void process_type(T&& arg) {
    using ArgType = std::remove_reference_t<T>;
    if constexpr (std::is_const_v<ArgType>) {
        std::cout << "  参数是 const 的\n";
    } else {
        std::cout << "  参数不是 const 的\n";
    }
}

template<typename T>
void make_vector_bad(std::initializer_list<T> init) {
    std::vector<T> v(init);
    std::cout << "  通过 initializer_list 参数间接转发, 大小=" << v.size() << "\n";
}

void demo_variadic_forwarding() {
    std::cout << "=== 变参完美转发 ===\n";

    auto w1 = make_object<Widget>();
    auto w2 = make_object<Widget>(1);
    auto w3 = make_object<Widget>(2, std::string("高级"));

    std::string name = "命名对象";
    auto w4 = make_object<Widget>(3, name);
    auto w5 = make_object<Widget>(4, std::move(name));

    std::cout << "\n变参转发的关键:\n";
    std::cout << "  Args&&... args  - 万能引用参数包\n";
    std::cout << "  std::forward<Args>(args)...  - 逐个完美转发\n";
    std::cout << "  每个参数独立推导, 保持各自的值类别\n";

    std::cout << "\n";
}

void demo_factory_with_perfect_forwarding() {
    std::cout << "=== 工厂模式与完美转发 ===\n";

    auto p1 = create_shared<Product>("电子产品", 101, 999.9);

    std::string cat = "食品";
    auto p2 = create_shared<Product>(cat, 102, 29.9);

    std::cout << "\n工厂函数的优势:\n";
    std::cout << "  1. 统一的对象创建接口\n";
    std::cout << "  2. 完美转发保持参数的值类别\n";
    std::cout << "  3. 可以轻松切换创建策略 (shared/unique/pooled)\n";

    std::cout << "\n";
}

void demo_forwarding_with_tuple() {
    std::cout << "=== tuple 与完美转发 ===\n";

    int a = 10;
    std::string s = "hello";
    auto tup = forward_as_tuple_demo(a, std::move(s), 42);

    std::cout << "std::forward_as_tuple: 创建引用的 tuple\n";
    std::cout << "std::make_tuple: 创建值的 tuple (会拷贝)\n";

    std::cout << "\n";
}

void demo_common_forwarding_mistakes() {
    std::cout << "=== 常见转发错误 ===\n";

    std::cout << "错误1: 在万能引用上使用 std::move 而非 std::forward\n";
    std::cout << "  std::move(arg) 会无条件移动, 破坏左值语义\n";
    std::cout << "  std::forward<T>(arg) 保持原始值类别\n\n";

    std::cout << "错误2: 转发时丢失 const\n";
    {
        const std::string cs = "const数据";
        process_type(cs);
        process_type(std::string("临时数据"));
    }

    std::cout << "\n错误3: 多次转发同一参数\n";
    std::cout << "  不要对同一参数多次 forward\n";

    std::cout << "\n错误4: 在非万能引用上使用 std::forward\n";
    std::cout << "  const T& 不是万能引用, 不应使用 std::forward\n";

    std::cout << "\n";
}

void demo_perfect_forwarding_limitations() {
    std::cout << "=== 完美转发的局限性 ===\n";

    std::cout << "1. 花括号初始化列表不能直接转发:\n";
    {
        make_vector_bad({1, 2, 3, 4, 5});
        std::cout << "  直接传递 {1,2,3} 给万能引用无法推导\n";
    }

    std::cout << "\n2. 位域不能完美转发:\n";
    std::cout << "  位域成员不能取地址, 无法按引用传递\n";

    std::cout << "\n3. 零值和 NULL 的歧义:\n";
    std::cout << "  使用 nullptr 代替 NULL\n";

    std::cout << "\n4. 重载决议的复杂性:\n";
    std::cout << "  完美转发构造函数可能比拷贝构造更匹配\n";
    std::cout << "  需要使用 SFINAE 或约束来避免\n";

    std::cout << "\n";
}

void demo_forwarding_best_practices() {
    std::cout << "=== 完美转发最佳实践 ===\n";

    std::cout << "1. 规则: 万能引用参数用 std::forward\n";
    std::cout << "   template<typename T> void f(T&& x) { g(std::forward<T>(x)); }\n\n";

    std::cout << "2. 规则: 右值引用参数用 std::move\n";
    std::cout << "   void f(Widget&& x) { g(std::move(x)); }\n\n";

    std::cout << "3. 规则: 不要在 return 语句中对局部变量用 std::move\n";
    std::cout << "   Widget f() { Widget w; return w; }  // NRVO\n\n";

    std::cout << "4. 规则: 移动操作标记 noexcept\n";
    std::cout << "   Widget(Widget&&) noexcept;\n\n";

    std::cout << "5. 规则: 移动后不要使用源对象的值\n";
    std::cout << "   只能: 析构、赋新值\n\n";

    std::cout << "6. 规则: 默认按值传递小对象\n";
    std::cout << "   void f(int x);  // 小类型按值\n";
    std::cout << "   void f(const std::string& s);  // 大类型按 const 引用\n";
    std::cout << "   template<typename T> void f(T&& x);  // 需要转发时用万能引用\n";

    std::cout << "\n";
}

int main() {
    demo_variadic_forwarding();
    demo_factory_with_perfect_forwarding();
    demo_forwarding_with_tuple();
    demo_common_forwarding_mistakes();
    demo_perfect_forwarding_limitations();
    demo_forwarding_best_practices();

    return 0;
}
