/** @file 02_deep_dive_object_layout.cpp
 *  @brief 对象内存布局、虚表指针、空基优化、对齐、POD类型
 *  @description 对应文档: 02-CPP/03-class-object
 */

#include <iostream>
#include <string>
#include <cstdint>

// ===== 1. 对象内存布局 =====
class SimpleClass {
public:
    void func() {}  // 成员函数不占对象空间
private:
    int a_;      // 4 字节
    double b_;   // 8 字节
    char c_;     // 1 字节
};

class WithVirtual {
public:
    virtual void foo() {}  // 虚函数引入 vptr
private:
    int a_;
};

void demo_object_layout() {
    std::cout << "===== 对象内存布局 =====" << std::endl;

    std::cout << "SimpleClass 大小: " << sizeof(SimpleClass) << " 字节" << std::endl;
    std::cout << "  int: 4, double: 8, char: 1, 总计 13" << std::endl;
    std::cout << "  但由于对齐, 实际大小可能更大" << std::endl;

    std::cout << "\nWithVirtual 大小: " << sizeof(WithVirtual) << " 字节" << std::endl;
    std::cout << "  vptr: " << sizeof(void*) << " 字节 (64位系统)" << std::endl;
    std::cout << "  int: 4 字节" << std::endl;
    std::cout << "  加上对齐填充" << std::endl;

    std::cout << "\n内存布局要点:" << std::endl;
    std::cout << "  - 成员变量按声明顺序排列" << std::endl;
    std::cout << "  - 编译器可能插入填充字节 (padding)" << std::endl;
    std::cout << "  - 虚函数引入虚表指针 (vptr), 通常在对象开头" << std::endl;
    std::cout << "  - 成员函数不占对象空间 (存在代码段)" << std::endl;
    std::cout << "  - 静态成员不占对象空间 (存在数据段)" << std::endl;
}

// ===== 2. 虚表指针 (vptr) =====
class Base1 {
public:
    virtual void f1() { std::cout << "  Base1::f1()" << std::endl; }
    virtual void f2() { std::cout << "  Base1::f2()" << std::endl; }
    virtual ~Base1() = default;
    int base_data_ = 1;
};

class Derived1 : public Base1 {
public:
    void f1() override { std::cout << "  Derived1::f1()" << std::endl; }
    virtual void f3() { std::cout << "  Derived1::f3()" << std::endl; }
    int derived_data_ = 2;
};

void demo_vtable() {
    std::cout << "\n===== 虚表指针 (vptr) =====" << std::endl;

    std::cout << "Base1 大小: " << sizeof(Base1) << " 字节" << std::endl;
    std::cout << "  vptr: " << sizeof(void*) << " 字节" << std::endl;
    std::cout << "  base_data_: " << sizeof(int) << " 字节" << std::endl;

    std::cout << "\nDerived1 大小: " << sizeof(Derived1) << " 字节" << std::endl;
    std::cout << "  继承 Base1 的 vptr 和 base_data_" << std::endl;
    std::cout << "  加上 derived_data_" << std::endl;

    Base1* ptr = new Derived1;
    ptr->f1();  // 通过 vtable 调用 Derived1::f1()
    ptr->f2();  // 通过 vtable 调用 Base1::f2()
    delete ptr;

    std::cout << "\n虚表机制:" << std::endl;
    std::cout << "  1. 每个多态类有一个虚表 (vtable), 存储虚函数指针" << std::endl;
    std::cout << "  2. 每个对象有一个虚表指针 (vptr), 指向类的虚表" << std::endl;
    std::cout << "  3. 虚函数调用: 对象 -> vptr -> vtable -> 函数指针" << std::endl;
    std::cout << "  4. 开销: 每个对象多一个指针, 每次虚调用一次间接寻址" << std::endl;
}

// ===== 3. 空基优化 (Empty Base Optimization, EBO) =====
class EmptyClass {
public:
    void do_something() {}
};

class NoEBO {
public:
    EmptyClass e;  // 空类成员
    int value;
};

class WithEBO : public EmptyClass {  // 空基类继承
public:
    int value;
};

void demo_ebo() {
    std::cout << "\n===== 空基优化 (EBO) =====" << std::endl;

    std::cout << "EmptyClass 大小: " << sizeof(EmptyClass) << " 字节" << std::endl;
    std::cout << "  (C++ 规定空类大小至少为 1 字节)" << std::endl;

    std::cout << "\nNoEBO (成员变量) 大小: " << sizeof(NoEBO) << " 字节" << std::endl;
    std::cout << "  EmptyClass 成员占 1 字节 + 填充" << std::endl;

    std::cout << "\nWithEBO (继承) 大小: " << sizeof(WithEBO) << " 字节" << std::endl;
    std::cout << "  空基类不占空间 (EBO)" << std::endl;

    std::cout << "\nEBO 的应用:" << std::endl;
    std::cout << "  - STL 分配器: std::allocator 继承空分配器" << std::endl;
    std::cout << "  - 策略类: 通过继承注入策略, 无空间开销" << std::endl;
    std::cout << "  - 类型标签: 用于模板特化, 无运行时开销" << std::endl;
}

// ===== 4. 内存对齐 =====
struct Unaligned {
    char a;     // 1 字节
    double b;   // 8 字节
    char c;     // 1 字节
};

struct Aligned {
    double b;   // 8 字节
    char a;     // 1 字节
    char c;     // 1 字节
};

struct WithAlignas {
    alignas(16) char data[10];
};

void demo_alignment() {
    std::cout << "\n===== 内存对齐 =====" << std::endl;

    std::cout << "Unaligned 大小: " << sizeof(Unaligned) << " 字节" << std::endl;
    std::cout << "  char(1) + 7填充 + double(8) + char(1) + 7填充 = 24" << std::endl;

    std::cout << "\nAligned 大小: " << sizeof(Aligned) << " 字节" << std::endl;
    std::cout << "  double(8) + char(1) + char(1) + 6填充 = 16" << std::endl;

    std::cout << "\nWithAlignas 大小: " << sizeof(WithAlignas) << " 字节" << std::endl;
    std::cout << "  alignas(16) 强制 16 字节对齐" << std::endl;

    std::cout << "\n对齐规则:" << std::endl;
    std::cout << "  - 对象的地址必须是其对齐要求的整数倍" << std::endl;
    std::cout << "  - 基本类型的对齐要求通常等于其大小" << std::endl;
    std::cout << "  - 结构体的对齐要求等于最大成员的对齐要求" << std::endl;
    std::cout << "  - 结构体大小是对齐要求的整数倍" << std::endl;

    std::cout << "\n优化建议:" << std::endl;
    std::cout << "  - 按大小降序排列成员变量" << std::endl;
    std::cout << "  - 使用 alignas 指定特殊对齐要求" << std::endl;
    std::cout << "  - 使用 alignof 查询类型的对齐要求" << std::endl;
    std::cout << "  alignof(double) = " << alignof(double) << std::endl;
}

// ===== 5. POD 类型 =====
struct PodStruct {
    int x;
    double y;
    char z;
};

class NonPodClass {
public:
    NonPodClass() : x_(0) {}
    virtual void func() {}
private:
    int x_;
};

void demo_pod() {
    std::cout << "\n===== POD 类型 =====" << std::endl;

    std::cout << "PodStruct:" << std::endl;
    std::cout << "  is_pod: " << std::is_pod<PodStruct>::value << std::endl;
    std::cout << "  is_trivial: " << std::is_trivial<PodStruct>::value << std::endl;
    std::cout << "  is_standard_layout: " << std::is_standard_layout<PodStruct>::value << std::endl;

    std::cout << "\nNonPodClass:" << std::endl;
    std::cout << "  is_pod: " << std::is_pod<NonPodClass>::value << std::endl;
    std::cout << "  is_trivial: " << std::is_trivial<NonPodClass>::value << std::endl;
    std::cout << "  is_standard_layout: " << std::is_standard_layout<NonPodClass>::value << std::endl;

    std::cout << "\nPOD (Plain Old Data) 的含义:" << std::endl;
    std::cout << "  - 可以用 memset/memcpy 安全操作" << std::endl;
    std::cout << "  - 可以用 C 语言的二进制接口传递" << std::endl;
    std::cout << "  - 可以用 = {0} 初始化" << std::endl;
    std::cout << "  - C++11 拆分为 trivial + standard_layout" << std::endl;

    std::cout << "\n现代 C++ 分类:" << std::endl;
    std::cout << "  is_trivially_copyable: 可安全 memcpy" << std::endl;
    std::cout << "  is_standard_layout: 与 C 兼容的布局" << std::endl;
    std::cout << "  is_pod = is_trivial && is_standard_layout" << std::endl;
    std::cout << "  (C++20 中 is_pod 已被弃用)" << std::endl;
}

int main() {
    std::cout << "========== 对象内存布局 ==========\n" << std::endl;

    demo_object_layout();
    demo_vtable();
    demo_ebo();
    demo_alignment();
    demo_pod();

    return 0;
}
