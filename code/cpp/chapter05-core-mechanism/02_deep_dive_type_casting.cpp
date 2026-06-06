/** @file 02_deep_dive_type_casting.cpp
 *  @brief static_cast, dynamic_cast, const_cast, reinterpret_cast, C风格转换的问题
 *  @description 对应文档: 02-CPP/05-core-mechanism
 */

#include <iostream>
#include <string>
#include <typeinfo>
#include <cstdint>

// ===== 1. static_cast =====
void demo_static_cast() {
    std::cout << "===== static_cast =====" << std::endl;

    // (1) 基本类型转换
    double pi = 3.14159;
    int i = static_cast<int>(pi);  // 截断, 有精度损失但合法
    std::cout << "  double -> int: " << pi << " -> " << i << std::endl;

    // (2) void* 与其他指针互转
    int value = 42;
    void* vp = &value;
    int* ip = static_cast<int*>(vp);  // OK: void* -> int*
    std::cout << "  void* -> int*: " << *ip << std::endl;

    // (3) 向上转型 (派生类 -> 基类, 安全)
    class Base { public: virtual ~Base() = default; int b = 1; };
    class Derived : public Base { public: int d = 2; };

    Derived derived;
    Base* bp = static_cast<Base*>(&derived);  // 向上转型, 安全
    std::cout << "  向上转型: Base::b = " << bp->b << std::endl;

    // (4) 向下转型 (基类 -> 派生类, 不安全! 无运行时检查)
    Base base_obj;
    // Derived* dp = static_cast<Derived*>(&base_obj);  // 编译通过但未定义行为!
    // dp->d;  // 访问不存在的成员

    Derived* dp2 = static_cast<Derived*>(bp);  // bp 实际指向 Derived, OK
    std::cout << "  向下转型 (安全): Derived::d = " << dp2->d << std::endl;

    std::cout << "\nstatic_cast 特点:" << std::endl;
    std::cout << "  - 编译期转换, 无运行时检查" << std::endl;
    std::cout << "  - 用于合理的类型转换" << std::endl;
    std::cout << "  - 不能去掉 const/volatile" << std::endl;
    std::cout << "  - 不能在不相关指针类型间转换" << std::endl;
}

// ===== 2. dynamic_cast =====
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

void demo_dynamic_cast() {
    std::cout << "\n===== dynamic_cast =====" << std::endl;

    Shape* s1 = new Circle;
    Shape* s2 = new Square;

    // 安全的向下转型: 运行时检查
    Circle* c = dynamic_cast<Circle*>(s1);
    if (c) {
        std::cout << "  s1 是 Circle, radius = " << c->radius() << std::endl;
    }

    Square* sq = dynamic_cast<Square*>(s1);
    if (!sq) {
        std::cout << "  s1 不是 Square" << std::endl;
    }

    // 引用版本: 失败抛出 std::bad_cast
    try {
        Circle& cr = dynamic_cast<Circle&>(*s2);  // s2 是 Square, 转换失败
        (void)cr;
    } catch (const std::bad_cast& e) {
        std::cout << "  引用 dynamic_cast 失败: " << e.what() << std::endl;
    }

    // 交叉转型 (cross cast)
    class A { public: virtual ~A() = default; };
    class B { public: virtual ~B() = default; };
    class C : public A, public B {};

    C cobj;
    A* ap = &cobj;
    B* bp = dynamic_cast<B*>(ap);  // 交叉转型
    if (bp) {
        std::cout << "  交叉转型 A* -> B* 成功" << std::endl;
    }

    delete s1;
    delete s2;

    std::cout << "\ndynamic_cast 特点:" << std::endl;
    std::cout << "  - 运行时类型检查 (需要 RTTI)" << std::endl;
    std::cout << "  - 指针失败返回 nullptr" << std::endl;
    std::cout << "  - 引用失败抛出 std::bad_cast" << std::endl;
    std::cout << "  - 只对多态类型有效 (有虚函数)" << std::endl;
    std::cout << "  - 性能开销: 遍历继承层次" << std::endl;
    std::cout << "  - 建议: 优先使用虚函数而非 dynamic_cast" << std::endl;
}

// ===== 3. const_cast =====
void demo_const_cast() {
    std::cout << "\n===== const_cast =====" << std::endl;

    // const_cast 只能添加或移除 const/volatile
    int value = 42;
    const int* cp = &value;

    // 移除 const
    int* p = const_cast<int*>(cp);
    *p = 100;  // OK: value 本身不是 const
    std::cout << "  移除 const 后修改: value = " << value << std::endl;

    // 真正的 const 对象: 修改是未定义行为
    const int const_val = 42;
    const int* cp2 = &const_val;
    int* p2 = const_cast<int*>(cp2);
    // *p2 = 100;  // 未定义行为! const_val 可能存储在只读内存
    (void)p2;
    std::cout << "  警告: 修改真正的 const 变量是未定义行为" << std::endl;

    // 添加 const (合法但通常不需要)
    int x = 10;
    const int* cpx = const_cast<const int*>(&x);  // 添加 const
    (void)cpx;

    std::cout << "\nconst_cast 合理用途:" << std::endl;
    std::cout << "  1. 调用遗留 C API (参数缺少 const)" << std::endl;
    std::cout << "  2. const/非 const 函数重载的代码复用" << std::endl;
    std::cout << "  其他场景应避免使用 const_cast" << std::endl;
}

// ===== 4. reinterpret_cast =====
void demo_reinterpret_cast() {
    std::cout << "\n===== reinterpret_cast =====" << std::endl;

    // reinterpret_cast: 重新解释位模式, 最危险的转换

    // (1) 指针与整数互转
    int value = 42;
    std::uintptr_t addr = reinterpret_cast<std::uintptr_t>(&value);
    std::cout << "  指针转整数: " << addr << std::endl;
    int* restored = reinterpret_cast<int*>(addr);
    std::cout << "  整数转指针: " << *restored << std::endl;

    // (2) 不相关指针类型转换 (危险!)
    double d = 3.14;
    // int* ip = reinterpret_cast<int*>(&d);  // 编译通过但极不安全
    // *ip;  // 读取 double 的位模式作为 int

    // (3) 函数指针转换 (不可移植)
    using FuncPtr = void(*)();
    using IntFuncPtr = int(*)(int);
    // IntFuncPtr ifp = reinterpret_cast<IntFuncPtr>(func_ptr);  // 未定义行为

    std::cout << "\nreinterpret_cast 特点:" << std::endl;
    std::cout << "  - 最危险的转换, 几乎不做任何检查" << std::endl;
    std::cout << "  - 重新解释位模式" << std::endl;
    std::cout << "  - 结果通常是平台相关的" << std::endl;
    std::cout << "  - 很多用法是未定义行为" << std::endl;

    std::cout << "\nreinterpret_cast 合理用途:" << std::endl;
    std::cout << "  1. 序列化/反序列化 (网络/文件)" << std::endl;
    std::cout << "  2. 与 C API 交互 (如 dlsym)" << std::endl;
    std::cout << "  3. 哈希函数中处理位模式" << std::endl;
    std::cout << "  其他场景应避免使用" << std::endl;
}

// ===== 5. C 风格转换的问题 =====
void demo_c_style_cast_problems() {
    std::cout << "\n===== C 风格转换的问题 =====" << std::endl;

    double pi = 3.14159;

    // C 风格转换: (int)pi 或 int(pi)
    int i1 = (int)pi;       // C 风格
    int i2 = int(pi);       // 函数风格

    // C++ 风格: static_cast<int>(pi)
    int i3 = static_cast<int>(pi);

    std::cout << "  C 风格: " << i1 << ", C++ 风格: " << i3 << std::endl;

    // C 风格转换的问题:
    const int cx = 42;
    // int* p = (int*)&cx;  // C 风格: 静默去掉 const! 危险!
    // int* p = static_cast<int*>(&cx);  // 编译错误: static_cast 不能去 const

    // C 风格转换可能等价于:
    // const_cast + static_cast, 或
    // const_cast + reinterpret_cast
    // 你不知道它做了什么!

    std::cout << "\nC 风格转换的问题:" << std::endl;
    std::cout << "  1. 可能静默去掉 const" << std::endl;
    std::cout << "  2. 可能执行 reinterpret_cast 级别的危险操作" << std::endl;
    std::cout << "  3. 不清楚具体做了哪种转换" << std::endl;
    std::cout << "  4. 难以搜索和审查 (grep 不方便)" << std::endl;

    std::cout << "\n最佳实践:" << std::endl;
    std::cout << "  - 禁止使用 C 风格转换" << std::endl;
    std::cout << "  - 优先: static_cast (安全)" << std::endl;
    std::cout << "  - 谨慎: dynamic_cast (多态向下转型)" << std::endl;
    std::cout << "  - 极少: const_cast (遗留代码)" << std::endl;
    std::cout << "  - 避免: reinterpret_cast (底层操作)" << std::endl;

    std::cout << "\n选择指南:" << std::endl;
    std::cout << "  需要类型转换? -> static_cast" << std::endl;
    std::cout << "  向下转型?     -> dynamic_cast" << std::endl;
    std::cout << "  去掉 const?   -> const_cast (三思!)" << std::endl;
    std::cout << "  位模式重解释? -> reinterpret_cast (三思!)" << std::endl;
    std::cout << "  都不合适?     -> 重新审视设计, 可能不需要转换" << std::endl;
}

int main() {
    std::cout << "========== C++ 类型转换详解 ==========\n" << std::endl;

    demo_static_cast();
    demo_dynamic_cast();
    demo_const_cast();
    demo_reinterpret_cast();
    demo_c_style_cast_problems();

    return 0;
}
