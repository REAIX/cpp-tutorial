/** @file 03_example_multiple_inheritance.cpp
 *  @brief 多重继承、虚继承、菱形问题
 *  @description 对应文档: 02-CPP/04-inheritance-polymorphism
 */

#include <iostream>
#include <string>

// ===== 1. 多重继承基础 =====
class Printable {
public:
    virtual ~Printable() = default;
    virtual void print() const = 0;
};

class Serializable {
public:
    virtual ~Serializable() = default;
    virtual std::string serialize() const = 0;
};

class Document : public Printable, public Serializable {
public:
    Document(const std::string& title, const std::string& content)
        : title_(title), content_(content) {}

    void print() const override {
        std::cout << "  文档: " << title_ << std::endl;
        std::cout << "  内容: " << content_ << std::endl;
    }

    std::string serialize() const override {
        return "{title: \"" + title_ + "\", content: \"" + content_ + "\"}";
    }

private:
    std::string title_;
    std::string content_;
};

void demo_multiple_inheritance() {
    std::cout << "===== 多重继承基础 =====" << std::endl;

    Document doc("C++ 指南", "多重继承示例");
    doc.print();
    std::cout << "  序列化: " << doc.serialize() << std::endl;

    // 通过不同基类指针访问
    Printable* p = &doc;
    p->print();

    Serializable* s = &doc;
    std::cout << "  通过 Serializable*: " << s->serialize() << std::endl;

    std::cout << "\n多重继承要点:" << std::endl;
    std::cout << "  - 一个类可以继承多个基类" << std::endl;
    std::cout << "  - 适合接口继承(混入/Mixin)" << std::endl;
    std::cout << "  - 注意二义性问题" << std::endl;
}

// ===== 2. 菱形继承问题 =====
//       Animal
//       /    \
//    Mammal  Bird
//       \    /
//       Bat

class Animal {
public:
    Animal() { std::cout << "  Animal() 构造" << std::endl; }
    virtual ~Animal() { std::cout << "  ~Animal() 析构" << std::endl; }

    std::string name = "动物";
};

// 不使用虚继承: 每个 Animal 子对象独立
class Mammal : public Animal {
public:
    Mammal() { std::cout << "  Mammal() 构造" << std::endl; }
};

class Bird : public Animal {
public:
    Bird() { std::cout << "  Bird() 构造" << std::endl; }
};

// 编译错误: Bat 有两个 Animal 子对象, 访问 name 有二义性
// class Bat : public Mammal, public Bird {};

// 使用虚继承: 共享 Animal 子对象
class VirtualMammal : virtual public Animal {
public:
    VirtualMammal() { std::cout << "  VirtualMammal() 构造" << std::endl; }
};

class VirtualBird : virtual public Animal {
public:
    VirtualBird() { std::cout << "  VirtualBird() 构造" << std::endl; }
};

class Bat : public VirtualMammal, public VirtualBird {
public:
    Bat() { std::cout << "  Bat() 构造" << std::endl; }
};

void demo_diamond_problem() {
    std::cout << "\n===== 菱形继承问题 =====" << std::endl;

    std::cout << "不使用虚继承的问题:" << std::endl;
    std::cout << "  Bat 继承 Mammal 和 Bird, 各有独立的 Animal 子对象" << std::endl;
    std::cout << "  访问 name 时有二义性: Mammal::name 还是 Bird::name?" << std::endl;

    std::cout << "\n使用虚继承:" << std::endl;
    Bat bat;
    bat.name = "蝙蝠";  // OK: 只有一个 Animal 子对象
    std::cout << "  bat.name = " << bat.name << std::endl;

    std::cout << "\n虚继承要点:" << std::endl;
    std::cout << "  - virtual 继承保证只有一个虚基类子对象" << std::endl;
    std::cout << "  - 最远派生类负责构造虚基类" << std::endl;
    std::cout << "  - 虚基类的构造由最远派生类初始化" << std::endl;
}

// ===== 3. 多重继承的二义性 =====
class A {
public:
    void func() { std::cout << "  A::func()" << std::endl; }
    int value = 1;
};

class B {
public:
    void func() { std::cout << "  B::func()" << std::endl; }
    int value = 2;
};

class C : public A, public B {
public:
    // 解决二义性1: 使用作用域解析
    void call_a_func() { A::func(); }
    void call_b_func() { B::func(); }

    // 解决二义性2: using 声明
    using A::value;

    // 解决二义性3: 在 C 中重写
    void func() { std::cout << "  C::func()" << std::endl; }
};

void demo_ambiguity() {
    std::cout << "\n===== 多重继承的二义性 =====" << std::endl;

    C c;
    // c.func();    // 二义性: A::func() 还是 B::func()?
    c.A::func();    // 显式指定
    c.B::func();
    c.func();       // C 中重写, 无二义性

    std::cout << "  c.A::value = " << c.A::value << std::endl;
    std::cout << "  c.B::value = " << c.B::value << std::endl;
    std::cout << "  c.value (using A::value) = " << c.value << std::endl;

    std::cout << "\n解决二义性的方法:" << std::endl;
    std::cout << "  1. 作用域解析: obj.A::func()" << std::endl;
    std::cout << "  2. using 声明: using A::value;" << std::endl;
    std::cout << "  3. 在派生类中重写: void func() override" << std::endl;
    std::cout << "  4. 避免设计导致二义性的继承层次" << std::endl;
}

// ===== 4. 多重继承最佳实践 =====
class Interface1 {
public:
    virtual ~Interface1() = default;
    virtual void method1() = 0;
};

class Interface2 {
public:
    virtual ~Interface2() = default;
    virtual void method2() = 0;
};

class Implementation : public Interface1, public Interface2 {
public:
    void method1() override {
        std::cout << "  Implementation::method1()" << std::endl;
    }

    void method2() override {
        std::cout << "  Implementation::method2()" << std::endl;
    }
};

void demo_mi_best_practices() {
    std::cout << "\n===== 多重继承最佳实践 =====" << std::endl;

    Implementation impl;
    impl.method1();
    impl.method2();

    std::cout << "\n多重继承的使用原则:" << std::endl;
    std::cout << "  1. 接口继承可以多重: 继承多个纯接口类" << std::endl;
    std::cout << "  2. 实现继承尽量单一: 只有一个带实现的基类" << std::endl;
    std::cout << "  3. 菱形继承用虚继承解决" << std::endl;
    std::cout << "  4. 优先使用组合而非继承" << std::endl;
    std::cout << "  5. Mixin 模式: 小的功能类通过继承混入" << std::endl;
}

int main() {
    std::cout << "========== 多重继承 ==========\n" << std::endl;

    demo_multiple_inheritance();
    demo_diamond_problem();
    demo_ambiguity();
    demo_mi_best_practices();

    return 0;
}
