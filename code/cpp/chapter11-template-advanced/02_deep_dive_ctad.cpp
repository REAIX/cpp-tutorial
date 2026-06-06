/** @file 02_deep_dive_ctad.cpp
 *  @brief CTAD：类模板参数推导、推导指引、显式推导指引、聚合体的CTAD
 *  @description 对应文档: 11-模板进阶 | 举一反三：掌握C++17类模板参数推导
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <utility>
#include <tuple>
#include <optional>

template<typename T, typename U>
class Pair {
public:
    Pair(T first, U second) : first_(std::move(first)), second_(std::move(second)) {}

    T& first() { return first_; }
    U& second() { return second_; }
    const T& first() const { return first_; }
    const U& second() const { return second_; }

    void print() const {
        std::cout << "(" << first_ << ", " << second_ << ")\n";
    }

private:
    T first_;
    U second_;
};

void demo_ctad_basics() {
    std::cout << "=== CTAD 基础 ===\n";

    Pair p1(1, 2.5);
    std::cout << "Pair p1(1, 2.5) => ";
    p1.print();

    Pair p2(std::string("Hello"), 42);
    std::cout << "Pair p2(\"Hello\", 42) => ";
    p2.print();

    Pair p3(3.14, std::string("PI"));
    std::cout << "Pair p3(3.14, \"PI\") => ";
    p3.print();

    std::cout << "\nCTAD (Class Template Argument Deduction):\n";
    std::cout << "  C++17 之前: Pair<int, double> p1(1, 2.5);\n";
    std::cout << "  C++17 起:  Pair p1(1, 2.5);  // 自动推导\n";
    std::cout << "  编译器根据构造函数参数推导模板参数\n";

    std::cout << "\n";
}

template<typename T>
class Wrapper {
public:
    Wrapper(T value) : value_(std::move(value)) {}
    const T& get() const { return value_; }
    void print() const {
        std::cout << "Wrapper 值: " << value_ << "\n";
    }
private:
    T value_;
};

Wrapper(const char*) -> Wrapper<std::string>;

template<typename T>
Wrapper(T*) -> Wrapper<T*>;

void demo_explicit_deduction_guides() {
    std::cout << "=== 显式推导指引 ===\n";

    Wrapper w1(42);
    std::cout << "Wrapper w1(42): ";
    w1.print();

    Wrapper w2("hello");
    std::cout << "Wrapper w2(\"hello\"): ";
    w2.print();

    int x = 100;
    Wrapper w3(&x);
    std::cout << "Wrapper w3(&x): ";
    w3.print();

    std::cout << "\n推导指引的语法:\n";
    std::cout << "  Wrapper(const char*) -> Wrapper<std::string>;\n";
    std::cout << "  将 const char* 推导为 std::string 而非 const char*\n\n";

    std::cout << "  template<typename T>\n";
    std::cout << "  Wrapper(T*) -> Wrapper<T*>;\n";
    std::cout << "  将指针类型推导为 Wrapper<指针类型>\n";

    std::cout << "\n";
}

template<typename Iterator>
class Range {
public:
    Range(Iterator begin, Iterator end) : begin_(begin), end_(end) {}

    template<typename C>
    Range(C& c) : begin_(c.begin()), end_(c.end()) {}

    Iterator begin() const { return begin_; }
    Iterator end() const { return end_; }

    size_t size() const { return std::distance(begin_, end_); }

private:
    Iterator begin_;
    Iterator end_;
};

void demo_deduction_guide_for_range() {
    std::cout << "=== Range 的推导指引 ===\n";

    std::vector<int> vec = {1, 2, 3, 4, 5};
    Range r(vec.begin(), vec.end());
    std::cout << "Range 大小: " << r.size() << "\n";

    const std::vector<int> cvec = {10, 20, 30};
    Range cr(cvec.begin(), cvec.end());
    std::cout << "const Range 大小: " << cr.size() << "\n";

    std::cout << "\n推导指引让 Range 可以从容器自动推导迭代器类型\n";

    std::cout << "\n";
}

struct Point {
    double x, y;
    Point(double x_, double y_) : x(x_), y(y_) {}
};

struct Rectangle {
    Point top_left;
    Point bottom_right;
    Rectangle(Point tl, Point br) : top_left(tl), bottom_right(br) {}
};

void demo_aggregate_ctad() {
    std::cout << "=== 聚合体的 CTAD ===\n";

    Point p{1.0, 2.0};
    std::cout << "Point p{1.0, 2.0}: (" << p.x << ", " << p.y << ")\n";

    Point p2(3.0, 4.0);
    std::cout << "Point p2(3.0, 4.0): (" << p2.x << ", " << p2.y << ")\n";

    Rectangle r{Point{0, 0}, Point{10, 10}};
    std::cout << "Rectangle: top_left=(" << r.top_left.x << "," << r.top_left.y
              << "), bottom_right=(" << r.bottom_right.x << "," << r.bottom_right.y << ")\n";

    std::cout << "\n聚合体 CTAD:\n";
    std::cout << "  C++17: 聚合体需要显式推导指引\n";
    std::cout << "  C++20: 聚合体可以自动推导\n";

    std::cout << "\n";
}

void demo_standard_library_ctad() {
    std::cout << "=== 标准库中的 CTAD ===\n";

    std::pair p(1, 2.5);
    std::cout << "std::pair p(1, 2.5): (" << p.first << ", " << p.second << ")\n";

    std::tuple t(1, 2.5, std::string("hello"));
    std::cout << "std::tuple t(1, 2.5, \"hello\")\n";

    std::vector v = {1, 2, 3, 4, 5};
    std::cout << "std::vector v = {1,2,3,4,5}: 大小=" << v.size() << "\n";

    std::vector v2{10, 20, 30};
    std::cout << "std::vector v2{10,20,30}: 大小=" << v2.size() << "\n";

    auto sp = std::make_shared<int>(42);
    std::cout << "std::make_shared<int>(42): *sp=" << *sp << "\n";

    std::optional opt = 42;
    std::cout << "std::optional opt = 42: " << opt.value() << "\n";

    std::cout << "\n标准库 CTAD 的支持:\n";
    std::cout << "  pair, tuple, vector, array\n";
    std::cout << "  shared_ptr, unique_ptr\n";
    std::cout << "  optional, variant, any\n";
    std::cout << "  basic_string, basic_string_view\n";

    std::cout << "\n";
}

template<typename... Args>
class Overloaded : public Args... {
public:
    using Args::operator()...;
};

template<typename... Args>
Overloaded(Args...) -> Overloaded<Args...>;

void demo_ctad_best_practices() {
    std::cout << "=== CTAD 最佳实践 ===\n";

    std::cout << "1. 何时使用 CTAD:\n";
    std::cout << "   - 模板参数可以从构造函数参数明显推导时\n";
    std::cout << "   - 标准库类型 (pair, tuple, vector 等)\n";
    std::cout << "   - 减少冗余的类型指定\n\n";

    std::cout << "2. 何时避免 CTAD:\n";
    std::cout << "   - 推导结果不直观时\n";
    std::cout << "   - 需要特定类型 (如推导为 int 但需要 long)\n";
    std::cout << "   - 公共 API 中, 显式类型更清晰\n\n";

    std::cout << "3. 编写推导指引的原则:\n";
    std::cout << "   - 让推导结果符合直觉\n";
    std::cout << "   - const char* => std::string 是常见转换\n";
    std::cout << "   - 为不同迭代器类型提供指引\n";
    std::cout << "   - 推导指引放在类定义附近\n\n";

    std::cout << "4. CTAD 与 make_ 函数的关系:\n";
    std::cout << "   - make_pair/make_tuple 在 CTAD 之前是必要的\n";
    std::cout << "   - CTAD 后, 直接构造通常更简洁\n";
    std::cout << "   - 但 make_shared 仍有性能优势 (单次分配)\n";

    std::cout << "\n";
}

int main() {
    demo_ctad_basics();
    demo_explicit_deduction_guides();
    demo_deduction_guide_for_range();
    demo_aggregate_ctad();
    demo_standard_library_ctad();
    demo_ctad_best_practices();

    return 0;
}
