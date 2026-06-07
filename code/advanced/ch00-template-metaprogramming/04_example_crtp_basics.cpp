/** @file 04_example_crtp_basics.cpp
 *  @brief CRTP基础：基本CRTP模式、静态多态vs虚函数多态
 *  @description 对应文档: 07-模板元编程与编译期计算 / CRTP与静态多态
 */

#include <iostream>
#include <string>
#include <chrono>
#include <vector>
#include <memory>
#include <cmath>

// ============================================================
// 1. CRTP 基本概念
// ============================================================

// CRTP = Curiously Recurring Template Pattern（奇异递归模板模式）
// 类模板通过派生类自身作为模板参数来继承：
//   class Derived : public Base<Derived> { ... };

// 作用：在基类中访问派生类的接口，实现静态多态（编译期绑定）

void demo_crtp_concept() {
    std::cout << "=== CRTP 基本概念 ===\n";
    std::cout << "CRTP = Curiously Recurring Template Pattern\n";
    std::cout << "  class Derived : public Base<Derived> { ... };\n\n";
    std::cout << "核心思想:\n";
    std::cout << "  基类通过模板参数\"知道\"派生类的类型\n";
    std::cout << "  基类可以 static_cast<Derived*>(this) 调用派生类方法\n";
    std::cout << "  这实现了编译期多态，无需虚函数表\n\n";
}

// ============================================================
// 2. 基本 CRTP 模式
// ============================================================

// 基类：定义接口框架
template<typename Derived>
class Shape {
public:
    // 通过 CRTP 调用派生类的实现
    double area() const {
        return static_cast<const Derived*>(this)->do_area();
    }

    double perimeter() const {
        return static_cast<const Derived*>(this)->do_perimeter();
    }

    std::string name() const {
        return static_cast<const Derived*>(this)->do_name();
    }

    // 基类可以提供通用功能
    void describe() const {
        std::cout << name() << ": 面积=" << area()
                  << ", 周长=" << perimeter() << "\n";
    }
};

// 派生类1：圆形
class Circle : public Shape<Circle> {
    double radius_;
public:
    explicit Circle(double r) : radius_(r) {}

    double do_area() const { return 3.14159265 * radius_ * radius_; }
    double do_perimeter() const { return 2 * 3.14159265 * radius_; }
    std::string do_name() const { return "圆形(r=" + std::to_string(radius_) + ")"; }
};

// 派生类2：矩形
class Rectangle : public Shape<Rectangle> {
    double width_, height_;
public:
    Rectangle(double w, double h) : width_(w), height_(h) {}

    double do_area() const { return width_ * height_; }
    double do_perimeter() const { return 2 * (width_ + height_); }
    std::string do_name() const { return "矩形(" + std::to_string(width_) + "x" + std::to_string(height_) + ")"; }
};

// 派生类3：三角形
class Triangle : public Shape<Triangle> {
    double a_, b_, c_;
public:
    Triangle(double a, double b, double c) : a_(a), b_(b), c_(c) {}

    double do_area() const {
        // 海伦公式
        double s = (a_ + b_ + c_) / 2;
        return std::sqrt(s * (s - a_) * (s - b_) * (s - c_));
    }
    double do_perimeter() const { return a_ + b_ + c_; }
    std::string do_name() const { return "三角形"; }
};

void demo_basic_crtp() {
    std::cout << "=== 基本 CRTP 模式 ===\n";

    Circle c(5.0);
    Rectangle r(3.0, 4.0);
    Triangle t(3.0, 4.0, 5.0);

    c.describe();
    r.describe();
    t.describe();

    std::cout << "\nCRTP 关键代码:\n";
    std::cout << "  基类: template<typename Derived> class Shape {\n";
    std::cout << "    double area() const {\n";
    std::cout << "      return static_cast<const Derived*>(this)->do_area();\n";
    std::cout << "    }\n";
    std::cout << "  };\n";
    std::cout << "  派生类: class Circle : public Shape<Circle> { ... };\n";

    std::cout << "\n";
}

// ============================================================
// 3. 静态多态 vs 虚函数多态
// ============================================================

// 虚函数版本（动态多态）
class VirtualShape {
public:
    virtual ~VirtualShape() = default;
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
};

class VirtualCircle : public VirtualShape {
    double radius_;
public:
    explicit VirtualCircle(double r) : radius_(r) {}
    double area() const override { return 3.14159265 * radius_ * radius_; }
    double perimeter() const override { return 2 * 3.14159265 * radius_; }
};

class VirtualRectangle : public VirtualShape {
    double width_, height_;
public:
    VirtualRectangle(double w, double h) : width_(w), height_(h) {}
    double area() const override { return width_ * height_; }
    double perimeter() const override { return 2 * (width_ + height_); }
};

// CRTP 版本的通用计算函数
template<typename T>
double total_area_crtp(const std::vector<T>& shapes) {
    double total = 0.0;
    for (const auto& s : shapes) {
        total += s.area();
    }
    return total;
}

// 虚函数版本的通用计算函数
double total_area_virtual(const std::vector<std::unique_ptr<VirtualShape>>& shapes) {
    double total = 0.0;
    for (const auto& s : shapes) {
        total += s->area();
    }
    return total;
}

void demo_static_vs_dynamic() {
    std::cout << "=== 静态多态 vs 虚函数多态 ===\n";

    // CRTP 版本
    std::vector<Circle> circles;
    for (int i = 0; i < 5; ++i) {
        circles.emplace_back(i + 1.0);
    }
    double crtp_total = total_area_crtp(circles);
    std::cout << "CRTP 总面积: " << crtp_total << "\n";

    // 虚函数版本
    std::vector<std::unique_ptr<VirtualShape>> vshapes;
    for (int i = 0; i < 5; ++i) {
        vshapes.push_back(std::make_unique<VirtualCircle>(i + 1.0));
    }
    double virtual_total = total_area_virtual(vshapes);
    std::cout << "虚函数总面积: " << virtual_total << "\n";

    std::cout << "\n对比:\n";
    std::cout << "  ┌─────────────┬──────────────┬──────────────┐\n";
    std::cout << "  │   特性       │  CRTP静态    │  虚函数动态  │\n";
    std::cout << "  ├─────────────┼──────────────┼──────────────┤\n";
    std::cout << "  │ 绑定时间     │  编译期      │  运行期      │\n";
    std::cout << "  │ 虚函数表     │  无          │  有          │\n";
    std::cout << "  │ 内存开销     │  小          │  大(vptr)    │\n";
    std::cout << "  │ 调用性能     │  高(内联)    │  较低(间接)  │\n";
    std::cout << "  │ 异构容器     │  困难        │  容易        │\n";
    std::cout << "  │ 扩展性       │  需改模板    │  直接继承    │\n";
    std::cout << "  └─────────────┴──────────────┴──────────────┘\n";

    std::cout << "\n";
}

// ============================================================
// 4. CRTP 的注意事项
// ============================================================

// 注意1：确保 static_cast 安全
// CRTP 依赖 static_cast，如果类型不匹配会导致未定义行为
// 例如：class Wrong : public Shape<OtherClass> 会导致错误

// 安全的 CRTP 基类：检查派生类类型
template<typename Derived>
class SafeShape {
public:
    double area() const {
        static_assert(std::is_base_of_v<SafeShape<Derived>, Derived>,
            "Derived 必须继承自 SafeShape<Derived>");
        return static_cast<const Derived*>(this)->do_area();
    }
};

// 注意2：避免在构造/析构中调用 CRTP 方法
// 构造时派生类尚未构造完成，static_cast 不安全
template<typename Derived>
class Counter {
public:
    static std::size_t count() { return count_; }

protected:
    Counter() { ++count_; }
    ~Counter() { --count_; }
    // 注意: 不要在构造/析构中调用 CRTP 方法（如 derived()），
    // 因为此时派生类尚未构造完成或已经析构

private:
    static inline std::size_t count_ = 0;
};

class MyObject : public Counter<MyObject> {
public:
    MyObject() = default;
};

void demo_crtp_pitfalls() {
    std::cout << "=== CRTP 注意事项 ===\n";

    std::cout << "1. 确保 static_cast 安全:\n";
    std::cout << "   派生类必须正确继承 Base<Derived>\n";
    std::cout << "   可用 static_assert + is_base_of 检查\n\n";

    std::cout << "2. 构造/析构中不要调用 CRTP 方法:\n";
    std::cout << "   构造时派生类尚未构造\n";
    std::cout << "   析构时派生类已经析构\n\n";

    std::cout << "3. 每个派生类形成独立的继承链:\n";
    std::cout << "   Circle : Shape<Circle>\n";
    std::cout << "   Rectangle : Shape<Rectangle>\n";
    std::cout << "   两者没有共同基类，不能放入同一容器\n";

    std::cout << "\n";
}

// ============================================================
// 5. CRTP 实现对象计数器
// ============================================================

// 使用上面定义的 Counter
class Widget : public Counter<Widget> {
    std::string name_;
public:
    explicit Widget(std::string n) : name_(std::move(n)) {}
    const std::string& name() const { return name_; }
};

class Gadget : public Counter<Gadget> {
    int id_;
public:
    explicit Gadget(int id) : id_(id) {}
    int id() const { return id_; }
};

void demo_crtp_counter() {
    std::cout << "=== CRTP 对象计数器 ===\n";

    std::cout << "Widget 计数: " << Widget::count() << "\n";
    std::cout << "Gadget 计数: " << Gadget::count() << "\n";

    {
        Widget w1("w1");
        Widget w2("w2");
        Gadget g1(1);

        std::cout << "创建后:\n";
        std::cout << "  Widget 计数: " << Widget::count() << "\n";
        std::cout << "  Gadget 计数: " << Gadget::count() << "\n";

        {
            Widget w3("w3");
            std::cout << "  再创建一个Widget: " << Widget::count() << "\n";
        }

        std::cout << "w3 离开作用域: " << Widget::count() << "\n";
    }

    std::cout << "全部离开作用域:\n";
    std::cout << "  Widget 计数: " << Widget::count() << "\n";
    std::cout << "  Gadget 计数: " << Gadget::count() << "\n";

    std::cout << "\nCRTP 计数器的优势:\n";
    std::cout << "  每个派生类有独立的计数器\n";
    std::cout << "  无虚函数开销\n";
    std::cout << "  编译期类型安全\n";

    std::cout << "\n";
}

// ============================================================
// 6. CRTP 实现可克隆模式
// ============================================================

template<typename Derived>
class Cloneable {
public:
    std::unique_ptr<Derived> clone() const {
        return std::make_unique<Derived>(static_cast<const Derived&>(*this));
    }
};

class Document : public Cloneable<Document> {
    std::string content_;
public:
    explicit Document(std::string content) : content_(std::move(content)) {}
    Document(const Document&) = default;

    const std::string& content() const { return content_; }
    void set_content(std::string c) { content_ = std::move(c); }
};

class Spreadsheet : public Cloneable<Spreadsheet> {
    int rows_, cols_;
public:
    Spreadsheet(int r, int c) : rows_(r), cols_(c) {}
    Spreadsheet(const Spreadsheet&) = default;

    int rows() const { return rows_; }
    int cols() const { return cols_; }
};

void demo_crtp_cloneable() {
    std::cout << "=== CRTP 可克隆模式 ===\n";

    Document doc1("原始文档");
    auto doc2 = doc1.clone();
    doc2->set_content("克隆文档");

    std::cout << "原始: " << doc1.content() << "\n";
    std::cout << "克隆: " << doc2->content() << "\n";

    Spreadsheet sheet1(10, 5);
    auto sheet2 = sheet1.clone();
    std::cout << "电子表格: " << sheet2->rows() << "x" << sheet2->cols() << "\n";

    std::cout << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "  CRTP 基础 (Curiously Recurring Template Pattern)\n";
    std::cout << "============================================\n\n";

    demo_crtp_concept();
    demo_basic_crtp();
    demo_static_vs_dynamic();
    demo_crtp_pitfalls();
    demo_crtp_counter();
    demo_crtp_cloneable();

    std::cout << "============================================\n";
    std::cout << "  核心要点总结:\n";
    std::cout << "  1. CRTP: Derived : Base<Derived>\n";
    std::cout << "  2. 静态多态: 编译期绑定，无虚函数开销\n";
    std::cout << "  3. 性能优势: 可内联，无间接调用\n";
    std::cout << "  4. 注意事项: 构造/析构中不要调用\n";
    std::cout << "  5. 常见用途: 计数器、克隆、迭代器适配\n";
    std::cout << "============================================\n";

    return 0;
}
