/** @file 01_deep_dive_virtual_mechanism.cpp
 *  @brief 虚表内部机制、vptr、虚函数调用开销、RTTI (dynamic_cast, typeid)
 *  @description 对应文档: 02-CPP/04-inheritance-polymorphism
 */

#include <iostream>
#include <string>
#include <typeinfo>

// ===== 1. 虚表内部机制 =====
class Base {
public:
    virtual void func1() { std::cout << "  Base::func1()" << std::endl; }
    virtual void func2() { std::cout << "  Base::func2()" << std::endl; }
    virtual void func3() { std::cout << "  Base::func3()" << std::endl; }
    void non_virtual() { std::cout << "  Base::non_virtual()" << std::endl; }

    virtual ~Base() = default;
};

class Derived : public Base {
public:
    void func1() override { std::cout << "  Derived::func1()" << std::endl; }
    void func3() override { std::cout << "  Derived::func3()" << std::endl; }
    virtual void func4() { std::cout << "  Derived::func4()" << std::endl; }
};

void demo_vtable_internals() {
    std::cout << "===== 虚表内部机制 =====" << std::endl;

    Base b;
    Derived d;

    std::cout << "Base 大小: " << sizeof(Base) << " 字节" << std::endl;
    std::cout << "Derived 大小: " << sizeof(Derived) << " 字节" << std::endl;

    std::cout << "\n虚表 (vtable) 的工作原理:" << std::endl;
    std::cout << "  1. 每个多态类有一个虚表, 存储虚函数指针" << std::endl;
    std::cout << "  2. Base 的 vtable:" << std::endl;
    std::cout << "     [0] Base::func1  [1] Base::func2  [2] Base::func3  [3] Base::~Base()" << std::endl;
    std::cout << "  3. Derived 的 vtable:" << std::endl;
    std::cout << "     [0] Derived::func1  [1] Base::func2  [2] Derived::func3  [3] Derived::~Derived()  [4] Derived::func4" << std::endl;
    std::cout << "  4. 每个对象首部有一个 vptr, 指向类的 vtable" << std::endl;

    Base* ptr = &d;
    ptr->func1();  // vtable[0] -> Derived::func1
    ptr->func2();  // vtable[1] -> Base::func2
    ptr->func3();  // vtable[2] -> Derived::func3
    // ptr->func4(); // 错误: Base 中没有 func4

    std::cout << "\n虚函数调用过程:" << std::endl;
    std::cout << "  ptr->func1() 等价于:" << std::endl;
    std::cout << "  1. 从 ptr 指向的对象中获取 vptr" << std::endl;
    std::cout << "  2. 通过 vptr 找到 vtable" << std::endl;
    std::cout << "  3. 从 vtable 中取出 func1 对应的函数指针 (索引0)" << std::endl;
    std::cout << "  4. 通过函数指针调用函数" << std::endl;
}

// ===== 2. 虚函数调用开销 =====
class NonVirtualClass {
public:
    int compute(int x) { return x * x; }
private:
    int data_ = 0;
};

class VirtualClass {
public:
    virtual int compute(int x) { return x * x; }
    virtual ~VirtualClass() = default;
private:
    int data_ = 0;
};

void demo_virtual_overhead() {
    std::cout << "\n===== 虚函数调用开销 =====" << std::endl;

    std::cout << "NonVirtualClass 大小: " << sizeof(NonVirtualClass) << " 字节" << std::endl;
    std::cout << "VirtualClass 大小: " << sizeof(VirtualClass) << " 字节" << std::endl;
    std::cout << "  差异: vptr 占 " << sizeof(void*) << " 字节" << std::endl;

    std::cout << "\n虚函数的开销:" << std::endl;
    std::cout << "  空间开销:" << std::endl;
    std::cout << "    - 每个对象: +1 个 vptr (" << sizeof(void*) << " 字节)" << std::endl;
    std::cout << "    - 每个类: +1 个 vtable (代码段)" << std::endl;

    std::cout << "\n  时间开销:" << std::endl;
    std::cout << "    - 虚函数调用: 1次间接寻址 (vtable 查找)" << std::endl;
    std::cout << "    - 非虚函数调用: 直接调用 (可能被内联)" << std::endl;
    std::cout << "    - 虚函数通常不能被内联 (编译期不知道调用哪个)" << std::endl;

    std::cout << "\n  实际影响:" << std::endl;
    std::cout << "    - 单次虚函数调用: 约 2-5 个额外 CPU 周期" << std::endl;
    std::cout << "    - 对大多数应用: 可忽略不计" << std::endl;
    std::cout << "    - 对高频调用 (如游戏循环): 可能需要优化" << std::endl;

    std::cout << "\n  优化策略:" << std::endl;
    std::cout << "    - 性能关键路径: CRTP (编译期多态)" << std::endl;
    std::cout << "    - 批量处理: 数据导向设计 (DOD)" << std::endl;
    std::cout << "    - final 类/函数: 帮助编译器去虚化 (devirtualization)" << std::endl;
}

// ===== 3. RTTI (运行时类型信息) =====
class Shape {
public:
    virtual ~Shape() = default;
    virtual std::string name() const = 0;
};

class Circle : public Shape {
public:
    std::string name() const override { return "Circle"; }
    double radius() const { return 5.0; }
};

class Square : public Shape {
public:
    std::string name() const override { return "Square"; }
    double side() const { return 10.0; }
};

void demo_rtti() {
    std::cout << "\n===== RTTI (运行时类型信息) =====" << std::endl;

    Shape* s1 = new Circle;
    Shape* s2 = new Square;

    // typeid: 获取对象的实际类型信息
    std::cout << "typeid(*s1).name() = " << typeid(*s1).name() << std::endl;
    std::cout << "typeid(*s2).name() = " << typeid(*s2).name() << std::endl;
    std::cout << "typeid(Shape).name() = " << typeid(Shape).name() << std::endl;

    // typeid 比较
    if (typeid(*s1) == typeid(Circle)) {
        std::cout << "s1 指向 Circle 对象" << std::endl;
    }

    // dynamic_cast: 安全的向下转型
    Circle* c = dynamic_cast<Circle*>(s1);
    if (c) {
        std::cout << "dynamic_cast 成功: s1 -> Circle*, radius = " << c->radius() << std::endl;
    }

    Square* sq = dynamic_cast<Square*>(s1);
    if (!sq) {
        std::cout << "dynamic_cast 失败: s1 不是 Square*" << std::endl;
    }

    // 引用的 dynamic_cast: 失败时抛出 std::bad_cast
    try {
        Circle& cr = dynamic_cast<Circle&>(*s2);  // s2 是 Square, 转换失败
        (void)cr;
    } catch (const std::bad_cast& e) {
        std::cout << "引用 dynamic_cast 失败: " << e.what() << std::endl;
    }

    delete s1;
    delete s2;

    std::cout << "\nRTTI 要点:" << std::endl;
    std::cout << "  - typeid: 返回 type_info 对象, 包含类型名称" << std::endl;
    std::cout << "  - dynamic_cast: 安全的向下转型" << std::endl;
    std::cout << "  - 指针转型失败返回 nullptr" << std::endl;
    std::cout << "  - 引用转型失败抛出 std::bad_cast" << std::endl;
    std::cout << "  - RTTI 只对多态类型有效 (有虚函数的类)" << std::endl;

    std::cout << "\nRTTI 的代价:" << std::endl;
    std::cout << "  - 增加二进制大小 (type_info 对象)" << std::endl;
    std::cout << "  - dynamic_cast 可能较慢 (遍历继承层次)" << std::endl;
    std::cout << "  - 嵌入式系统常禁用: -fno-rtti" << std::endl;
    std::cout << "  - 优先使用虚函数而非 dynamic_cast + 条件分支" << std::endl;
}

// ===== 4. 举一反三: 虚函数的常见陷阱 =====
class Parent {
public:
    Parent() {
        std::cout << "  Parent() 构造" << std::endl;
        // 在构造函数中调用虚函数: 不会动态分派!
        // 此时对象的动态类型是 Parent, 不是 Derived
        virtual_call_in_constructor();
    }

    virtual void virtual_call_in_constructor() {
        std::cout << "  Parent::virtual_call_in_constructor()" << std::endl;
    }

    virtual void func() {
        std::cout << "  Parent::func()" << std::endl;
    }
};

class Child : public Parent {
public:
    Child() : Parent() {
        std::cout << "  Child() 构造" << std::endl;
    }

    void virtual_call_in_constructor() override {
        std::cout << "  Child::virtual_call_in_constructor()" << std::endl;
    }

    void func() override {
        std::cout << "  Child::func()" << std::endl;
    }
};

void demo_virtual_pitfalls() {
    std::cout << "\n===== 举一反三: 虚函数陷阱 =====" << std::endl;

    std::cout << "陷阱1: 构造/析构函数中调用虚函数" << std::endl;
    Child c;  // 构造时调用的是 Parent::virtual_call_in_constructor, 不是 Child 的!
    // 因为在 Parent 构造函数执行时, Child 部分尚未构造

    std::cout << "\n陷阱2: 隐藏而非重写" << std::endl;
    std::cout << "  基类: virtual void func(int x)" << std::endl;
    std::cout << "  派生: void func(int x)   // 缺少 override, 可能不是重写" << std::endl;
    std::cout << "  解决: 总是使用 override 关键字" << std::endl;

    std::cout << "\n陷阱3: 默认参数与虚函数" << std::endl;
    std::cout << "  虚函数的默认参数是静态绑定的!" << std::endl;
    std::cout << "  基类: virtual void f(int x = 10)" << std::endl;
    std::cout << "  派生: void f(int x = 20) override" << std::endl;
    std::cout << "  通过基类指针调用: 使用基类的默认值 10!" << std::endl;
    std::cout << "  解决: 虚函数避免使用默认参数" << std::endl;

    std::cout << "\n陷阱4: 遗漏虚析构函数" << std::endl;
    std::cout << "  多态基类必须有虚析构函数!" << std::endl;
    std::cout << "  否则 delete 基类指针时, 派生类析构函数不会被调用" << std::endl;
    std::cout << "  导致资源泄漏" << std::endl;
}

int main() {
    std::cout << "========== 虚函数机制深入 ==========\n" << std::endl;

    demo_vtable_internals();
    demo_virtual_overhead();
    demo_rtti();
    demo_virtual_pitfalls();

    return 0;
}
