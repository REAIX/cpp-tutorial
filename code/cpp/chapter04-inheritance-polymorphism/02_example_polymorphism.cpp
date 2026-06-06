/** @file 02_example_polymorphism.cpp
 *  @brief 虚函数、override、动态分派、抽象类、纯虚函数
 *  @description 对应文档: 02-CPP/04-inheritance-polymorphism
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>

// ===== 1. 虚函数与动态分派 =====
class Shape {
public:
    Shape(const std::string& name) : name_(name) {
        std::cout << "  Shape(\"" << name_ << "\") 构造" << std::endl;
    }

    virtual ~Shape() {
        std::cout << "  ~Shape() \"" << name_ << "\" 析构" << std::endl;
    }

    // 虚函数: 可以在派生类中重写 (override)
    virtual double area() const {
        return 0.0;
    }

    virtual void describe() const {
        std::cout << "  " << name_ << ": 面积 = " << area() << std::endl;
    }

    // 非虚函数: 不支持动态分派
    const std::string& name() const { return name_; }

protected:
    std::string name_;
};

class Circle : public Shape {
public:
    Circle(double radius)
        : Shape("圆形"), radius_(radius) {}

    double area() const override {
        return 3.14159265 * radius_ * radius_;
    }

    void describe() const override {
        std::cout << "  " << name_ << "(r=" << radius_ << "): 面积 = " << area() << std::endl;
    }

private:
    double radius_;
};

class Rectangle : public Shape {
public:
    Rectangle(double width, double height)
        : Shape("矩形"), width_(width), height_(height) {}

    double area() const override {
        return width_ * height_;
    }

    void describe() const override {
        std::cout << "  " << name_ << "(" << width_ << "x" << height_
                  << "): 面积 = " << area() << std::endl;
    }

private:
    double width_;
    double height_;
};

class Triangle : public Shape {
public:
    Triangle(double base, double height)
        : Shape("三角形"), base_(base), height_(height) {}

    double area() const override {
        return 0.5 * base_ * height_;
    }

private:
    double base_;
    double height_;
};

void demo_virtual_functions() {
    std::cout << "===== 虚函数与动态分派 =====" << std::endl;

    // 通过基类指针调用虚函数 -> 动态分派
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(5.0));
    shapes.push_back(std::make_unique<Rectangle>(4.0, 6.0));
    shapes.push_back(std::make_unique<Triangle>(3.0, 8.0));

    for (const auto& shape : shapes) {
        shape->describe();  // 动态分派: 根据实际类型调用正确的函数
    }

    std::cout << "\n动态分派原理:" << std::endl;
    std::cout << "  - 虚函数调用通过虚表 (vtable) 实现" << std::endl;
    std::cout << "  - 运行时根据对象的实际类型选择函数" << std::endl;
    std::cout << "  - 非虚函数在编译时确定 (静态绑定)" << std::endl;
}

// ===== 2. override 关键字 =====
class Base2 {
public:
    virtual void func1() { std::cout << "  Base2::func1()" << std::endl; }
    virtual void func2(int x) { std::cout << "  Base2::func2(" << x << ")" << std::endl; }
    void func3() { std::cout << "  Base2::func3()" << std::endl; }
};

class Derived2 : public Base2 {
public:
    void func1() override { std::cout << "  Derived2::func1()" << std::endl; }

    // void func2(double x) override {}  // 编译错误! 签名不匹配, override 捕获错误

    // void func3() override {}  // 编译错误! func3 不是虚函数, override 捕获错误

    // 如果不加 override, 以下代码会编译通过但不是重写:
    // void func2(double x) { ... }  // 这是重载(隐藏), 不是重写!
};

void demo_override() {
    std::cout << "\n===== override 关键字 =====" << std::endl;

    Derived2 d;
    d.func1();

    Base2& ref = d;
    ref.func1();  // 调用 Derived2::func1() (动态分派)

    std::cout << "\noverride 的好处:" << std::endl;
    std::cout << "  1. 编译器检查是否正确重写了基类虚函数" << std::endl;
    std::cout << "  2. 防止签名不匹配导致的意外隐藏" << std::endl;
    std::cout << "  3. 使代码意图更清晰" << std::endl;
    std::cout << "  建议: 重写虚函数时总是使用 override" << std::endl;
}

// ===== 3. 抽象类与纯虚函数 =====
class Drawable {
public:
    virtual ~Drawable() = default;

    // 纯虚函数: 没有实现, 必须由派生类重写
    virtual void draw() const = 0;
    virtual double area() const = 0;

    // 抽象类可以有普通成员函数
    void show_info() const {
        std::cout << "  面积: " << area() << std::endl;
        draw();
    }
};

class Square : public Drawable {
public:
    Square(double side) : side_(side) {}

    void draw() const override {
        std::cout << "  绘制正方形 (边长=" << side_ << ")" << std::endl;
    }

    double area() const override {
        return side_ * side_;
    }

private:
    double side_;
};

class Ellipse : public Drawable {
public:
    Ellipse(double a, double b) : a_(a), b_(b) {}

    void draw() const override {
        std::cout << "  绘制椭圆 (a=" << a_ << ", b=" << b_ << ")" << std::endl;
    }

    double area() const override {
        return 3.14159265 * a_ * b_;
    }

private:
    double a_, b_;
};

void demo_abstract_class() {
    std::cout << "\n===== 抽象类与纯虚函数 =====" << std::endl;

    // Drawable d;  // 错误: 不能实例化抽象类

    Square sq(5.0);
    sq.show_info();

    Ellipse el(3.0, 4.0);
    el.show_info();

    // 通过基类指针使用
    std::vector<std::unique_ptr<Drawable>> drawables;
    drawables.push_back(std::make_unique<Square>(4.0));
    drawables.push_back(std::make_unique<Ellipse>(2.0, 5.0));

    for (const auto& d : drawables) {
        d->draw();
    }

    std::cout << "\n抽象类要点:" << std::endl;
    std::cout << "  - 含有纯虚函数的类是抽象类" << std::endl;
    std::cout << "  - 不能实例化抽象类" << std::endl;
    std::cout << "  - 派生类必须实现所有纯虚函数才能实例化" << std::endl;
    std::cout << "  - 抽象类定义接口, 派生类提供实现" << std::endl;
    std::cout << "  - 纯虚函数也可以有实现 (但必须通过类名调用)" << std::endl;
}

int main() {
    std::cout << "========== 多态与虚函数 ==========\n" << std::endl;

    demo_virtual_functions();
    demo_override();
    demo_abstract_class();

    return 0;
}
