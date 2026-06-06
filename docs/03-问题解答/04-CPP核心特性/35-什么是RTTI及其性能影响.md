# 什么是 RTTI 及其性能影响
> 📖 相关章节：[类与对象](../../02-CPP/03-类与对象.md)、[继承与多态](../../02-CPP/04-继承与多态.md)、[核心机制](../../02-CPP/05-核心机制.md)、[异常处理](../../02-CPP/07-异常处理.md)、[移动语义](../../02-CPP/09-移动语义与完美转发.md)

### 1. 先抓核心

**RTTI（Run-Time Type Information）是运行时类型识别机制，通过 typeid 和 dynamic_cast 实现，依赖 vtable 中的 type_info 指针。它带来额外的内存和运行时开销，可用 -fno-rtti 关闭。**

***

### 2. 核心定义

| | RTTI | 无 RTTI |
|---|---|---|
| 是什么 | 运行时识别对象实际类型的能力 | 编译期确定类型，运行时不查询 |
| 实现方式 | typeid 运算符 + dynamic_cast 运算符 | 静态类型系统 + 虚函数多态 |
| 依赖 | vtable 中的 type_info 指针 | 无额外依赖 |
| 开销 | 有（内存 + 运行时） | 无 |

**本质**：

```cpp
// RTTI 让你在运行时知道对象的"真实类型"
class Base { virtual void foo() {} };
class Derived : public Base {};

Base* ptr = new Derived;

// typeid：获取对象的实际类型信息
std::cout << typeid(*ptr).name();   // 输出 "Derived"（编译器相关）

// dynamic_cast：运行时安全向下转型
Derived* d = dynamic_cast<Derived*>(ptr);  // 成功，d 不为空
Base* b = new Base;
Derived* d2 = dynamic_cast<Derived*>(b);   // 失败，d2 为 nullptr
```

***

### 3. 生活类比

| | RTTI | 没有 RTTI |
|---|---|---|
| 类比 | 身份证系统（运行时查验身份） | 只看工牌不看身份证（编译期确定） |
| 说明 | 你可以要求任何人出示身份证，确认他的真实身份 | 你只知道他的工牌上写的职位，不能进一步验证 |
| 关键区别 | 运行时可以查询真实类型 | 只能依赖编译期的静态类型 |

**具体场景**：

- **RTTI**：公司有身份证系统。你遇到一个人，虽然他穿着管理层的衣服（指针类型是 Base*），但你可以查他的身份证（typeid），确认他其实是技术总监（Derived）。你也可以尝试把他当作技术总监来沟通（dynamic_cast），如果查证他不是，沟通就会失败（返回 nullptr）。
- **没有 RTTI**：公司没有身份证系统。你只能根据工牌（静态类型）判断职位。如果工牌写的是"管理层"，你不知道他具体是什么职位。要实现不同行为，只能靠虚函数（每个人自己知道该怎么做事）。

***

### 4. typeid 与 type_info

```cpp
#include <typeinfo>
#include <iostream>

class Base { virtual void foo() {} };
class Derived : public Base {};

int main() {
    // typeid 用于类型名：编译期确定
    std::cout << typeid(int).name() << "\n";
    std::cout << typeid(Base).name() << "\n";

    // typeid 用于多态对象：运行时确定实际类型
    Base* ptr = new Derived;
    std::cout << typeid(*ptr).name() << "\n";   // "Derived"（运行时查询）

    // typeid 用于非多态对象：编译期确定静态类型
    Base obj = Derived();   // 对象切片！obj 只是 Base
    std::cout << typeid(obj).name() << "\n";    // "Base"（编译期确定）

    // type_info 对象的比较
    if (typeid(*ptr) == typeid(Derived)) {
        std::cout << "ptr points to Derived\n";
    }

    // type_info 的常用方法
    const std::type_info& ti = typeid(*ptr);
    std::cout << "name: " << ti.name() << "\n";
    std::cout << "hash_code: " << ti.hash_code() << "\n";

    // 注意：name() 返回值是编译器相关的修饰名
    // GCC/Clang 可能返回 "7Derived"
    // MSVC 可能返回 "class Derived"
    // 需要用 abi::__cxa_demangle（GCC/Clang）解码

    delete ptr;
    return 0;
}
```

**typeid 的两种行为**：

| 表达式 | 行为 | 时机 |
|------|---|---|
| typeid(类型名) | 返回该类型的 type_info | 编译期 |
| typeid(多态表达式) | 返回实际类型的 type_info | 运行时 |
| typeid(非多态表达式) | 返回静态类型的 type_info | 编译期 |

***

### 5. dynamic_cast 的开销来源

```cpp
#include <iostream>

class Base { virtual void foo() {} };
class Derived : public Base {};
class Other : public Base {};

int main() {
    Base* ptr = new Derived;

    // dynamic_cast 的运行时开销来自：
    // 1. 遍历继承层次，检查类型是否匹配
    // 2. 对于多重继承，需要调整 this 指针偏移
    // 3. 对于虚继承，需要通过 vbase offset 查找

    // 单继承：开销较小
    Derived* d = dynamic_cast<Derived*>(ptr);   // 一次类型检查

    // 多重继承：开销较大
    class A { virtual void fa() {} };
    class B { virtual void fb() {} };
    class C : public A, public B {};
    A* a = new C;
    B* b = dynamic_cast<B*>(a);   // 需要指针偏移 + 类型检查

    // 虚继承：开销最大
    class V { virtual void fv() {} };
    class X : virtual public V {};
    class Y : virtual public V {};
    class Z : public X, public Y {};
    V* v = new Z;
    X* x = dynamic_cast<X*>(v);   // 需要虚基类查找

    // 对比：static_cast 零运行时开销
    Derived* sd = static_cast<Derived*>(ptr);  // 编译期直接偏移，不检查

    // 引用版本的 dynamic_cast：失败抛异常
    try {
        Derived& dr = dynamic_cast<Derived&>(*ptr);
    } catch (const std::bad_cast& e) {
        std::cout << "Cast failed: " << e.what() << "\n";
    }

    delete ptr;
    return 0;
}
```

**dynamic_cast 开销对比**：

| 转型方式 | 运行时检查 | 开销 |
|------|:---:|---|
| static_cast | 无 | 零（编译期偏移） |
| dynamic_cast（单继承） | 有 | 小（一次类型比较） |
| dynamic_cast（多重继承） | 有 | 中（遍历 + 指针偏移） |
| dynamic_cast（虚继承） | 有 | 大（虚基类查找） |

***

### 6. vtable 中的 type_info 指针

```cpp
// 每个 vtable 的布局（简化）：
// ┌──────────────────────┐
// │ offset_to_top        │  用于多重继承的指针调整
// ├──────────────────────┤
// │ type_info pointer    │  ← RTTI 的关键！指向 type_info 对象
// ├──────────────────────┤
// │ vfunc[0]             │  第一个虚函数指针
// ├──────────────────────┤
// │ vfunc[1]             │  第二个虚函数指针
// ├──────────────────────┤
// │ ...                  │
// └──────────────────────┘

// type_info 对象的布局（简化）：
// ┌──────────────────────┐
// │ vtable_ptr           │  type_info 自己的 vtable
// ├──────────────────────┤
// │ name                 │  类型名字符串（如 "7Derived"）
// ├──────────────────────┤
// │ base_info[]          │  基类信息（用于 dynamic_cast 遍历）
// └──────────────────────┘

// RTTI 的内存开销：
// 1. 每个多态类一个 type_info 对象（.rodata 段）
// 2. 每个 vtable 多一个 type_info 指针（8 字节）
// 3. 每个 type_info 包含类型名和继承关系信息

class Base {
    virtual void foo() {}
    int x;
};
// sizeof(Base) = 8(vptr) + 4(x) + 4(padding) = 16
// vptr 指向的 vtable 包含 type_info 指针
// type_info 对象本身额外占用 .rodata 空间
```

**RTTI 内存开销汇总**：

| 开销项 | 大小 | 说明 |
|------|---|---|
| vtable 中的 type_info 指针 | 8 字节/类 | 每个多态类的 vtable 多一个指针 |
| type_info 对象 | 数十字节/类 | 包含类型名、哈希值、基类信息 |
| 类型名字符串 | 变长 | 存储修饰后的类型名 |

***

### 7. -fno-rtti 选项

```bash
# GCC/Clang：禁用 RTTI
g++ -fno-rtti main.cpp

# MSVC：禁用 RTTI
cl /GR- main.cpp
```

**禁用 RTTI 的影响**：

```cpp
// 禁用 RTTI 后，以下代码编译失败：
// 1. typeid 用于多态表达式
Base* ptr = new Derived;
// std::cout << typeid(*ptr).name();   // 错误！

// 2. dynamic_cast
// Derived* d = dynamic_cast<Derived*>(ptr);  // 错误！

// 3. std::any（部分实现依赖 RTTI）
// std::any a = 42;
// int* p = std::any_cast<int>(&a);    // 可能出错

// 4. std::exception::what() 的某些实现
```

**禁用 RTTI 后的替代方案**：

```cpp
// 替代1：虚函数多态（推荐）
class Base {
public:
    virtual ~Base() = default;
    virtual void process() = 0;        // 每个派生类自己实现
    virtual std::string typeName() const { return "Base"; }  // 手动类型名
};

class Derived : public Base {
public:
    void process() override { /* ... */ }
    std::string typeName() const override { return "Derived"; }
};

// 替代2：访问者模式（替代 dynamic_cast + 类型判断）
class Visitor;
class Base {
public:
    virtual ~Base() = default;
    virtual void accept(Visitor& v) = 0;
};

class DerivedA : public Base {
public:
    void accept(Visitor& v) override;
};
class DerivedB : public Base {
public:
    void accept(Visitor& v) override;
};

class Visitor {
public:
    void visit(DerivedA& a) { /* 处理 A */ }
    void visit(DerivedB& b) { /* 处理 B */ }
};

// 替代3：手动类型标签
enum class Type { Base, DerivedA, DerivedB };
class Base {
public:
    Type type;
    virtual ~Base() = default;
protected:
    Base(Type t) : type(t) {}
};

class DerivedA : public Base {
public:
    DerivedA() : Base(Type::DerivedA) {}
};

// 使用 switch 代替 dynamic_cast
void process(Base* obj) {
    switch (obj->type) {
        case Type::DerivedA: {
            auto* a = static_cast<DerivedA*>(obj);
            // ...
            break;
        }
        case Type::DerivedB: {
            auto* b = static_cast<DerivedB*>(obj);
            // ...
            break;
        }
    }
}
```

***

### 8. 与虚函数表和 dynamic_cast 的关系

**相关 FAQ**：

- **[FAQ 18 虚函数表 vtable 详解](../02-内存与底层/03-虚函数表vtable.md)**：RTTI 的 type_info 指针存储在 vtable 中，vtable 是 RTTI 的基础设施
- **[FAQ 50 C++ 四种类型转换](../04-CPP核心特性/08-C++四种类型转换.md)**：dynamic_cast 是四种类型转换之一，依赖 RTTI 实现

**关系图**：

```
vtable（虚函数表）
├── 虚函数指针[]        ← FAQ 18 详解
├── offset_to_top
└── type_info 指针      ← RTTI 的核心数据
    └── type_info 对象
        ├── 类型名
        ├── 哈希值
        └── 基类信息[]  ← dynamic_cast 遍历此信息

typeid(*ptr)
  → 通过 ptr->vptr 找到 vtable
  → 通过 vtable 中的 type_info 指针找到 type_info
  → 返回 type_info 引用

dynamic_cast<Derived*>(ptr)
  → 通过 ptr->vptr 找到 vtable
  → 通过 type_info 检查继承关系
  → 成功则返回调整后的指针，失败则返回 nullptr
```

***

### 9. 对比表格

| 特性 | 有 RTTI | 无 RTTI (-fno-rtti) |
|------|:---:|:---:|
| typeid(多态对象) | 可用 | 不可用 |
| dynamic_cast | 可用 | 不可用 |
| vtable 大小 | 多一个 type_info 指针 | 无 type_info 指针 |
| 二进制大小 | 较大（含 type_info 对象） | 较小 |
| 运行时开销 | dynamic_cast 有遍历开销 | 无 |
| 替代方案 | 不需要 | 虚函数多态 / 访问者模式 / 手动标签 |
| 适用场景 | 通用 C++ 开发 | 嵌入式 / 游戏引擎 / 追求极致性能 |

***

### 10. 完整示例

```cpp
#include <iostream>
#include <typeinfo>
#include <vector>
using namespace std;

class Shape {
public:
    virtual ~Shape() = default;
    virtual double area() const = 0;
    virtual string name() const = 0;
};

class Circle : public Shape {
    double r;
public:
    Circle(double r) : r(r) {}
    double area() const override { return 3.14159 * r * r; }
    string name() const override { return "Circle"; }
};

class Rectangle : public Shape {
    double w, h;
public:
    Rectangle(double w, double h) : w(w), h(h) {}
    double area() const override { return w * h; }
    string name() const override { return "Rectangle"; }
};

class Triangle : public Shape {
    double b, h;
public:
    Triangle(double b, double h) : b(b), h(h) {}
    double area() const override { return 0.5 * b * h; }
    string name() const override { return "Triangle"; }
};

void demoRTTI(const vector<Shape*>& shapes) {
    cout << "=== RTTI Demo ===\n";

    for (auto* s : shapes) {
        // typeid：运行时获取类型名
        cout << "  typeid: " << typeid(*s).name() << "\n";

        // dynamic_cast：安全向下转型
        if (auto* c = dynamic_cast<Circle*>(s)) {
            cout << "    -> Circle with radius operations\n";
        } else if (auto* r = dynamic_cast<Rectangle*>(s)) {
            cout << "    -> Rectangle with width/height operations\n";
        } else if (auto* t = dynamic_cast<Triangle*>(s)) {
            cout << "    -> Triangle with base/height operations\n";
        }
    }
}

void demoNoRTTI(const vector<Shape*>& shapes) {
    cout << "\n=== No-RTTI approach (virtual functions) ===\n";

    for (auto* s : shapes) {
        // 不用 typeid/dynamic_cast，用虚函数
        cout << "  " << s->name() << " area=" << s->area() << "\n";
    }
}

int main() {
    vector<Shape*> shapes = {
        new Circle(5.0),
        new Rectangle(3.0, 4.0),
        new Triangle(6.0, 2.0)
    };

    demoRTTI(shapes);
    demoNoRTTI(shapes);

    for (auto* s : shapes) delete s;
    return 0;
}
```

***

### 11. 极简总结

**RTTI = 运行时类型识别（typeid + dynamic_cast）| 依赖 vtable 中的 type_info 指针 | dynamic_cast 有运行时遍历开销 | type_info 增加二进制大小 | -fno-rtti 可禁用 | 禁用后用虚函数多态/访问者模式/手动标签替代 | 优先用虚函数多态而非 RTTI | 嵌入式和游戏引擎常禁用 RTTI**

***

### 相关阅读

- [动态绑定与静态绑定](./10-动态绑定与静态绑定.md)
- [CRTP模式与静态多态](./17-CRTP模式与静态多态.md)
- [C++四种类型转换](./08-C++四种类型转换.md)