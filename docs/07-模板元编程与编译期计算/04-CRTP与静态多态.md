# CRTP与静态多态

> C++编译期多态核心：CRTP原理、应用与局限

---

> **CRTP: static polymorphism without virtual function overhead.**
> （CRTP：没有虚函数开销的静态多态。）

> **将类型信息编码到继承关系中，让编译器帮你分派。**
> （Encode type information into inheritance, let the compiler dispatch for you.）

---

> **🎯 CRTP：用继承实现编译期多态的精妙技巧。**

> 💡 **通俗理解 - 什么是CRTP？**

想象你在公司里：
- **动态多态（虚函数）**：老板发指令，员工根据职位自行理解执行（运行时决定）
- **CRTP（静态多态）**：公司直接把具体岗位的执行手册发给你（编译时确定）

**CRTP就是"自己把自己传给基类"的奇怪但强大的继承方式！**

```cpp
// 普通继承：基类不知道派生类是谁
class Animal {
public:
    virtual void speak() = 0;  // 虚函数，运行时分派
};

class Dog : public Animal {
public:
    void speak() override { std::cout << "汪汪" << std::endl; }
};

// CRTP：基类知道派生类是谁！
template<typename Derived>
class AnimalBase {
public:
    void speak() {  // 非虚函数，编译时分派
        static_cast<Derived*>(this)->speak_impl();
    }
};

class Cat : public AnimalBase<Cat> {  // 把自己传给基类
public:
    void speak_impl() { std::cout << "喵喵" << std::endl; }
};
```

> 🔬 **抽象理解 - CRTP的本质**：
> - **CRTP（Curiously Recurring Template Pattern）**：奇异递归模板模式
> - **派生类将自己作为模板参数传给基类**：`class Derived : public Base<Derived>`
> - **静态分派**：基类通过static_cast调用派生类方法，无需虚函数表
> - **零开销抽象**：编译期确定调用目标，无虚函数调用开销
> - **编译期接口约束**：基类可以要求派生类提供特定方法

---

## 前置知识
- [模板元编程模式](03-模板元编程模式.md)
- [继承与多态](../02-CPP/04-继承与多态.md)

## 后续内容
- [表达式模板](05-表达式模板.md)

## 目录

- [1. CRTP原理](#1-crtp原理)
- [2. 静态多态vs动态多态](#2-静态多态vs动态多态)
- [3. CRTP的常见应用](#3-crtp的常见应用)
- [4. CRTP的局限与陷阱](#4-crtp的局限与陷阱)

---

## 1. CRTP原理

### 1.1 概念与定义

**CRTP（Curiously Recurring Template Pattern）**：奇异递归模板模式，C++模板编程的经典惯用法。派生类将自身作为模板参数传递给基类，基类通过static_cast将this指针转换为派生类指针，从而调用派生类的方法。

**CRTP的核心思想**：利用模板在编译期确定派生类类型，实现无需虚函数的多态。

### 1.2 CRTP基本结构

```cpp
// CRTP的基本结构
template<typename Derived>
class Base {
public:
    void interface() {
        // 通过static_cast调用派生类实现
        static_cast<Derived*>(this)->implementation();
    }

    // 也可以提供默认实现
    void implementation() {
        // 默认实现
    }
};

class Derived : public Base<Derived> {
public:
    void implementation() {
        // 派生类特定实现
        std::cout << "Derived实现" << std::endl;
    }
};

// 使用
int main() {
    Derived d;
    d.interface();  // 输出"Derived实现"
    return 0;
}
```

### 1.3 CRTP的工作机制

```cpp
// CRTP的调用过程：
// 1. 创建Derived对象
// 2. 调用d.interface()，实际调用Base<Derived>::interface()
// 3. interface()中static_cast<Derived*>(this)将Base指针转为Derived指针
// 4. 通过Derived指针调用Derived::implementation()
// 5. 整个过程在编译期确定，无虚函数开销

// CRTP的this指针转换
template<typename Derived>
class Base {
public:
    void show_type() {
        // this的类型是Base<Derived>*
        // static_cast后变为Derived*
        Derived* derived = static_cast<Derived*>(this);
        std::cout << "派生类类型: " << typeid(*derived).name() << std::endl;
    }

    // const版本
    void show_type() const {
        const Derived* derived = static_cast<const Derived*>(this);
        std::cout << "const派生类类型: " << typeid(*derived).name() << std::endl;
    }
};

class MyDerived : public Base<MyDerived> {
    // Base<MyDerived>知道派生类是MyDerived
};

int main() {
    MyDerived d;
    d.show_type();  // 输出MyDerived的类型信息
    return 0;
}
```

### 1.4 CRTP与虚函数对比

```cpp
// 虚函数方式
class VirtualBase {
public:
    virtual ~VirtualBase() = default;
    virtual void process() = 0;
};

class VirtualDerived : public VirtualBase {
public:
    void process() override {
        std::cout << "VirtualDerived::process" << std::endl;
    }
};

// CRTP方式
template<typename Derived>
class CrtpBase {
public:
    void process() {
        static_cast<Derived*>(this)->process_impl();
    }
};

class CrtpDerived : public CrtpBase<CrtpDerived> {
public:
    void process_impl() {
        std::cout << "CrtpDerived::process" << std::endl;
    }
};

// 对比：
// 虚函数：通过vtable间接调用，有运行时开销
// CRTP：直接调用，无间接开销
// 虚函数：支持运行时多态（基类指针指向派生类对象）
// CRTP：编译期多态，不支持基类指针统一管理
```

---

## 2. 静态多态vs动态多态

### 2.1 动态多态（虚函数）

```cpp
#include <iostream>
#include <vector>
#include <memory>

// 动态多态：通过虚函数实现
class Shape {
public:
    virtual ~Shape() = default;
    virtual double area() const = 0;
    virtual void draw() const = 0;
    virtual std::string name() const = 0;
};

class Circle : public Shape {
    double radius_;
public:
    explicit Circle(double r) : radius_(r) {}
    double area() const override { return 3.14159 * radius_ * radius_; }
    void draw() const override { std::cout << "画圆" << std::endl; }
    std::string name() const override { return "Circle"; }
};

class Rectangle : public Shape {
    double width_, height_;
public:
    Rectangle(double w, double h) : width_(w), height_(h) {}
    double area() const override { return width_ * height_; }
    void draw() const override { std::cout << "画矩形" << std::endl; }
    std::string name() const override { return "Rectangle"; }
};

// 动态多态的优势：可以用基类指针统一管理
void print_areas(const std::vector<std::unique_ptr<Shape>>& shapes) {
    for (const auto& shape : shapes) {
        std::cout << shape->name() << " 面积: " << shape->area() << std::endl;
    }
}

int main() {
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(5.0));
    shapes.push_back(std::make_unique<Rectangle>(3.0, 4.0));
    print_areas(shapes);
    return 0;
}
```

### 2.2 静态多态（CRTP）

```cpp
#include <iostream>

// 静态多态：通过CRTP实现
template<typename Derived>
class ShapeBase {
public:
    double area() const {
        return static_cast<const Derived*>(this)->area_impl();
    }

    void draw() const {
        static_cast<const Derived*>(this)->draw_impl();
    }

    std::string name() const {
        return static_cast<const Derived*>(this)->name_impl();
    }
};

class Circle2 : public ShapeBase<Circle2> {
    double radius_;
public:
    explicit Circle2(double r) : radius_(r) {}
    double area_impl() const { return 3.14159 * radius_ * radius_; }
    void draw_impl() const { std::cout << "画圆" << std::endl; }
    std::string name_impl() const { return "Circle"; }
};

class Rectangle2 : public ShapeBase<Rectangle2> {
    double width_, height_;
public:
    Rectangle2(double w, double h) : width_(w), height_(h) {}
    double area_impl() const { return width_ * height_; }
    void draw_impl() const { std::cout << "画矩形" << std::endl; }
    std::string name_impl() const { return "Rectangle"; }
};

// 静态多态：每个类型单独处理
template<typename ShapeT>
void print_area(const ShapeT& shape) {
    std::cout << shape.name() << " 面积: " << shape.area() << std::endl;
}

int main() {
    Circle2 c(5.0);
    Rectangle2 r(3.0, 4.0);
    print_area(c);  // 编译期确定调用Circle2的方法
    print_area(r);  // 编译期确定调用Rectangle2的方法
    return 0;
}
```

### 2.3 两种多态的性能对比

```cpp
// 性能对比示例
#include <chrono>

// 虚函数版本
class VirtualCounter {
public:
    virtual ~VirtualCounter() = default;
    virtual int increment(int x) = 0;
};

class VirtualCounterImpl : public VirtualCounter {
public:
    int increment(int x) override { return x + 1; }
};

// CRTP版本
template<typename Derived>
class CrtpCounter {
public:
    int increment(int x) {
        return static_cast<Derived*>(this)->increment_impl(x);
    }
};

class CrtpCounterImpl : public CrtpCounter<CrtpCounterImpl> {
public:
    int increment_impl(int x) { return x + 1; }
};

// 性能测试
void benchmark() {
    const int N = 100000000;

    // 虚函数版本
    {
        VirtualCounterImpl impl;
        VirtualCounter* counter = &impl;
        auto start = std::chrono::high_resolution_clock::now();
        int result = 0;
        for (int i = 0; i < N; ++i) {
            result = counter->increment(result);  // 虚函数调用
        }
        auto end = std::chrono::high_resolution_clock::now();
        // 虚函数调用有间接跳转开销
    }

    // CRTP版本
    {
        CrtpCounterImpl counter;
        auto start = std::chrono::high_resolution_clock::now();
        int result = 0;
        for (int i = 0; i < N; ++i) {
            result = counter.increment(result);  // 直接调用
        }
        auto end = std::chrono::high_resolution_clock::now();
        // CRTP调用无间接跳转，可能被内联
    }
}
```

### 2.4 对比总结

| 特性 | 动态多态（虚函数） | 静态多态（CRTP） |
|------|-------------------|-----------------|
| 分派时机 | 运行时 | 编译期 |
| 调用开销 | 虚函数表间接调用 | 直接调用（可内联） |
| 异构容器 | 支持（基类指针） | 不直接支持 |
| 代码膨胀 | 无 | 每种类型生成一份代码 |
| 扩展性 | 运行时添加新类型 | 编译时添加新类型 |
| 类型安全 | 运行时检查 | 编译期检查 |
| 内存开销 | 每个对象有vtable指针 | 无额外开销 |

---

## 3. CRTP的常见应用

### 3.1 接口扩展：为派生类添加功能

```cpp
// CRTP为派生类自动添加功能，无需派生类手动实现

// 示例1：为所有派生类添加计数功能
template<typename Derived>
class ObjectCounter {
    static inline std::size_t count_ = 0;
public:
    ObjectCounter() { ++count_; }
    ObjectCounter(const ObjectCounter&) { ++count_; }
    ~ObjectCounter() { --count_; }

    static std::size_t count() { return count_; }
};

class Widget : public ObjectCounter<Widget> {
    // Widget自动获得计数功能
    // 每个Widget的实例都会被计数
};

class Gadget : public ObjectCounter<Gadget> {
    // Gadget也有自己独立的计数器
    // Widget::count() 和 Gadget::count() 互不影响
};

int main() {
    Widget w1, w2;
    std::cout << Widget::count() << std::endl;  // 2
    {
        Widget w3;
        std::cout << Widget::count() << std::endl;  // 3
    }
    std::cout << Widget::count() << std::endl;  // 2
    return 0;
}
```

### 3.2 代码复用：避免重复实现

```cpp
// CRTP实现运算符复用
template<typename Derived>
class Comparable {
public:
    bool operator!=(const Derived& other) const {
        return !(static_cast<const Derived*>(this)->operator==(other));
    }

    bool operator>(const Derived& other) const {
        return other < static_cast<const Derived&>(*this);
    }

    bool operator<=(const Derived& other) const {
        return !(other < static_cast<const Derived&>(*this));
    }

    bool operator>=(const Derived& other) const {
        return !(static_cast<const Derived*>(this) < other);
    }
};

// 只需实现operator==和operator<，其他运算符自动生成
class Person : public Comparable<Person> {
    std::string name_;
    int age_;
public:
    Person(std::string name, int age) : name_(std::move(name)), age_(age) {}

    bool operator==(const Person& other) const {
        return name_ == other.name_ && age_ == other.age_;
    }

    bool operator<(const Person& other) const {
        return age_ < other.age_;
    }
};

int main() {
    Person alice("Alice", 30);
    Person bob("Bob", 25);
    // 以下运算符由CRTP自动提供
    bool ne = alice != bob;   // OK
    bool gt = alice > bob;    // OK
    bool le = alice <= bob;   // OK
    bool ge = alice >= bob;   // OK
    return 0;
}
```

### 3.3 编译期多态：策略模式

```cpp
// CRTP实现编译期策略模式
template<typename Derived>
class SortStrategy {
public:
    template<typename Iterator>
    void sort(Iterator begin, Iterator end) {
        static_cast<Derived*>(this)->sort_impl(begin, end);
    }
};

class QuickSort : public SortStrategy<QuickSort> {
public:
    template<typename Iterator>
    void sort_impl(Iterator begin, Iterator end) {
        // 快速排序实现
        std::cout << "快速排序" << std::endl;
    }
};

class MergeSort : public SortStrategy<MergeSort> {
public:
    template<typename Iterator>
    void sort_impl(Iterator begin, Iterator end) {
        // 归并排序实现
        std::cout << "归并排序" << std::endl;
    }
};

// 编译期选择排序策略
template<typename Strategy>
class Sorter {
    Strategy strategy_;
public:
    template<typename Iterator>
    void sort(Iterator begin, Iterator end) {
        strategy_.sort(begin, end);
    }
};

int main() {
    std::vector<int> v{5, 3, 1, 4, 2};

    Sorter<QuickSort> quick_sorter;
    quick_sorter.sort(v.begin(), v.end());  // 编译期绑定快速排序

    Sorter<MergeSort> merge_sorter;
    merge_sorter.sort(v.begin(), v.end());  // 编译期绑定归并排序
    return 0;
}
```

### 3.4 链式调用：Builder模式

```cpp
// CRTP实现链式Builder模式
template<typename Derived>
class BuilderBase {
public:
    Derived& set_name(const std::string& name) {
        static_cast<Derived*>(this)->name_ = name;
        return static_cast<Derived&>(*this);
    }

    Derived& set_verbose(bool v) {
        static_cast<Derived*>(this)->verbose_ = v;
        return static_cast<Derived&>(*this);
    }
};

class ConfigBuilder : public BuilderBase<ConfigBuilder> {
    friend class BuilderBase<ConfigBuilder>;
    std::string name_;
    bool verbose_ = false;
    int timeout_ = 30;
public:
    ConfigBuilder& set_timeout(int t) {
        timeout_ = t;
        return *this;
    }

    struct Config {
        std::string name;
        bool verbose;
        int timeout;
    };

    Config build() {
        return {name_, verbose_, timeout_};
    }
};

int main() {
    auto config = ConfigBuilder()
        .set_name("myapp")
        .set_verbose(true)
        .set_timeout(60)
        .build();
    return 0;
}
```

### 3.5 不可复制但可移动的混入

```cpp
// CRTP混入：为类添加特定属性
template<typename Derived>
class NonCopyable {
public:
    NonCopyable() = default;
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};

class UniqueResource : public NonCopyable<UniqueResource> {
    // 自动不可复制，但可移动
    int* data_;
public:
    UniqueResource() : data_(new int(42)) {}
    ~UniqueResource() { delete data_; }
};

// CRTP混入：可序列化
template<typename Derived>
class Serializable {
public:
    std::string to_json() const {
        return static_cast<const Derived*>(this)->to_json_impl();
    }

    static Derived from_json(const std::string& json) {
        return Derived::from_json_impl(json);
    }
};

class User : public Serializable<User> {
public:
    std::string name;
    int age;

    std::string to_json_impl() const {
        return "{\"name\":\"" + name + "\",\"age\":" + std::to_string(age) + "}";
    }

    static User from_json_impl(const std::string& json) {
        User u;
        // 解析json...
        return u;
    }
};
```

### 3.6 CRTP与访问者模式

```cpp
// CRTP实现编译期访问者模式
template<typename... Visitors>
class VisitorGroup;

template<typename Visitor>
class VisitorGroup<Visitor> : public Visitor {
public:
    using Visitor::visit;
};

template<typename First, typename... Rest>
class VisitorGroup<First, Rest...> : public First, public VisitorGroup<Rest...> {
public:
    using First::visit;
    using VisitorGroup<Rest...>::visit;
};

// 定义访问者
struct DrawVisitor {
    template<typename T>
    void visit(const T& shape) {
        shape.draw();
    }
};

struct AreaVisitor {
    template<typename T>
    double visit(const T& shape) {
        return shape.area();
    }
};

using MyVisitors = VisitorGroup<DrawVisitor, AreaVisitor>;
```

---

## 4. CRTP的局限与陷阱

### 4.1 不能用基类指针统一管理

```cpp
// CRTP的派生类没有共同的基类
class Circle : public ShapeBase<Circle> { /* ... */ };
class Rectangle : public ShapeBase<Rectangle> { /* ... */ };

// ShapeBase<Circle> 和 ShapeBase<Rectangle> 是不同的类型
// 不能用 ShapeBase* 指针统一管理

// 解决方案1：添加公共虚基类
class ShapeInterface {
public:
    virtual ~ShapeInterface() = default;
    virtual double area() const = 0;
};

template<typename Derived>
class ShapeBase : public ShapeInterface {
public:
    double area() const override {
        return static_cast<const Derived*>(this)->area_impl();
    }
};

// 解决方案2：使用std::variant
using ShapeVariant = std::variant<Circle, Rectangle>;

void print_area(const ShapeVariant& shape) {
    std::visit([](const auto& s) { std::cout << s.area() << std::endl; }, shape);
}

// 解决方案3：使用类型擦除
```

### 4.2 对象切片问题

```cpp
// CRTP的切片问题
template<typename Derived>
class Base {
public:
    void process() {
        static_cast<Derived*>(this)->process_impl();
    }
};

class Derived1 : public Base<Derived1> {
public:
    void process_impl() { std::cout << "Derived1" << std::endl; }
};

// 危险！切片会导致未定义行为
void dangerous_slice() {
    Derived1 d1;
    Base<Derived1>& base_ref = d1;  // OK：引用不会切片
    base_ref.process();              // OK：调用Derived1::process_impl

    // Base<Derived1> base_copy = d1;  // 切片！
    // base_copy.process();  // 未定义行为！static_cast到不完整的Derived1
}
```

### 4.3 构造和析构中的CRTP调用

```cpp
// 陷阱：在构造/析构中调用CRTP方法
template<typename Derived>
class Base {
public:
    Base() {
        // 危险！此时Derived尚未构造完成
        // static_cast<Derived*>(this)->init();  // 未定义行为！
    }

    ~Base() {
        // 危险！此时Derived已经析构
        // static_cast<Derived*>(this)->cleanup();  // 未定义行为！
    }

    void safe_init() {
        // 安全：在对象完全构造后调用
        static_cast<Derived*>(this)->init();
    }
};

class Derived : public Base<Derived> {
public:
    void init() { /* 安全的初始化 */ }
};

int main() {
    Derived d;
    d.safe_init();  // OK：对象已完全构造
    return 0;
}
```

### 4.4 代码膨胀

```cpp
// CRTP会导致代码膨胀：每种派生类型生成一份基类代码
class TypeA : public Base<TypeA> { /* ... */ };
class TypeB : public Base<TypeB> { /* ... */ };
class TypeC : public Base<TypeC> { /* ... */ };
// Base<TypeA>, Base<TypeB>, Base<TypeC> 是三份不同的代码

// 如果基类有很多代码，这会导致二进制体积增大

// 解决方案：将不依赖Derived的代码提取到非模板基类中
class BaseCommon {
public:
    void common_operation() { /* 不依赖Derived的代码 */ }
protected:
    int common_data_;
};

template<typename Derived>
class Base : public BaseCommon {
public:
    void specific_operation() {
        // 依赖Derived的代码
        static_cast<Derived*>(this)->impl();
    }
};
```

### 4.5 CRTP与智能指针

```cpp
// CRTP与shared_ptr结合
template<typename Derived>
class EnableSharedFromThis {
public:
    std::shared_ptr<Derived> shared_from_this() {
        return std::static_pointer_cast<Derived>(
            weak_ptr_.lock()
        );
    }

    std::shared_ptr<const Derived> shared_from_this() const {
        return std::static_pointer_cast<const Derived>(
            weak_ptr_.lock()
        );
    }

    void set_weak_ptr(const std::shared_ptr<Derived>& sp) {
        weak_ptr_ = sp;
    }

private:
    std::weak_ptr<Derived> weak_ptr_;
};

// 使用
class MyShared : public EnableSharedFromThis<MyShared>,
                 public std::enable_shared_from_this<MyShared> {
public:
    void do_something() {
        auto self = shared_from_this();  // 安全获取shared_ptr
    }
};

// 创建
auto ptr = std::make_shared<MyShared>();
ptr->set_weak_ptr(ptr);
```

### 4.6 CRTP的替代方案

```cpp
// 替代方案1：C++20 Concepts
// 用概念约束替代CRTP的接口约束
template<typename T>
concept Drawable = requires(const T& t) {
    { t.draw() } -> std::same_as<void>;
    { t.area() } -> std::convertible_to<double>;
};

template<Drawable T>
void render(const T& shape) {
    shape.draw();
    std::cout << "面积: " << shape.area() << std::endl;
}

// 替代方案2：if constexpr
template<typename T>
void process(T& obj) {
    if constexpr (requires { obj.process_impl(); }) {
        obj.process_impl();
    } else {
        obj.default_process();
    }
}

// 替代方案3：类型擦除
// 当需要异构容器时，类型擦除比CRTP更合适
class AnyShape {
    struct Concept {
        virtual ~Concept() = default;
        virtual double area() const = 0;
    };

    template<typename T>
    struct Model : Concept {
        T obj;
        double area() const override { return obj.area(); }
    };

    std::unique_ptr<Concept> impl_;
public:
    template<typename T>
    AnyShape(T obj) : impl_(std::make_unique<Model<T>>(std::move(obj))) {}

    double area() const { return impl_->area(); }
};
```

### 4.7 CRTP常见陷阱深度分析

```cpp
// 陷阱1：static_cast的安全性
template<typename Derived>
class Base {
public:
    void process() {
        // 如果Derived不是Base<Derived>的派生类，这是未定义行为！
        static_cast<Derived*>(this)->process_impl();
    }
};

// 错误使用：Derived2不是Base<Derived2>的派生类
class WrongClass : public Base<SomeOtherClass> {  // 错误！
    void process_impl() { /* ... */ }
};
// static_cast<SomeOtherClass*>(this)指向的是WrongClass对象
// 但转换为SomeOtherClass*，这是未定义行为！

// 解决：使用static_assert验证
template<typename Derived>
class SafeBase {
    static_assert(std::is_base_of_v<SafeBase<Derived>, Derived>,
        "Derived必须继承自SafeBase<Derived>");
public:
    void process() {
        static_cast<Derived*>(this)->process_impl();
    }
};

// 陷阱2：CRTP与多重继承的交互
template<typename Derived>
class Counter { /* ... */ };

template<typename Derived>
class Serializable { /* ... */ };

class MyClass : public Counter<MyClass>, public Serializable<MyClass> {
    // OK：两个CRTP基类独立工作
};

// 但注意：如果两个CRTP基类有同名方法
template<typename Derived>
class A {
public:
    void foo() { static_cast<Derived*>(this)->foo_impl(); }
};

template<typename Derived>
class B {
public:
    void foo() { static_cast<Derived*>(this)->bar_impl(); }
};

class C : public A<C>, public B<C> {
public:
    void foo_impl() { /* ... */ }
    void bar_impl() { /* ... */ }
};

C obj;
// obj.foo();  // 歧义！A<C>::foo() vs B<C>::foo()
obj.A<C>::foo();  // 明确调用
obj.B<C>::foo();  // 明确调用

// 陷阱3：CRTP与虚继承
// CRTP基类不能使用虚继承
// template<typename Derived>
// class virtual Base { };  // 错误！模板类不能是虚基类

// 陷阱4：CRTP的构造顺序
template<typename Derived>
class Base {
public:
    Base() {
        // 此时Derived尚未构造
        // static_cast<Derived*>(this)是未定义行为
        // 即使Derived的基类（即Base）已经构造
    }
};
```

### 4.8 CRTP完整实战案例：编译期观察者模式

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <functional>

// 完整案例：使用CRTP实现编译期观察者模式

// 观察者基类（CRTP）
template<typename Derived>
class Observer {
public:
    void notify(const std::string& event) {
        static_cast<Derived*>(this)->on_notify(event);
    }
};

// 被观察者基类（CRTP）
template<typename Derived>
class Observable {
    std::vector<std::function<void(const std::string&)>> observers_;
public:
    template<typename Obs>
    void add_observer(Obs& observer) {
        observers_.push_back([&observer](const std::string& event) {
            observer.notify(event);
        });
    }

    void notify_all(const std::string& event) {
        for (auto& obs : observers_) {
            obs(event);
        }
    }

    // CRTP：派生类可以自定义通知逻辑
    void emit(const std::string& event) {
        static_cast<Derived*>(this)->before_notify(event);
        notify_all(event);
        static_cast<Derived*>(this)->after_notify(event);
    }

    void before_notify(const std::string&) {}  // 默认空实现
    void after_notify(const std::string&) {}   // 默认空实现
};

// 具体观察者
class LogObserver : public Observer<LogObserver> {
public:
    void on_notify(const std::string& event) {
        std::cout << "[LOG] 收到事件: " << event << std::endl;
    }
};

class AlertObserver : public Observer<AlertObserver> {
    int threshold_;
public:
    explicit AlertObserver(int threshold) : threshold_(threshold) {}

    void on_notify(const std::string& event) {
        if (event.find("error") != std::string::npos) {
            std::cout << "[ALERT] 错误事件: " << event << std::endl;
        }
    }
};

// 具体被观察者
class EventBus : public Observable<EventBus> {
public:
    void before_notify(const std::string& event) override {
        std::cout << "[事件开始] " << event << std::endl;
    }

    void after_notify(const std::string& event) override {
        std::cout << "[事件结束] " << event << std::endl;
    }

    void send(const std::string& event) {
        emit(event);
    }
};

int main() {
    EventBus bus;
    LogObserver log_obs;
    AlertObserver alert_obs(0);

    bus.add_observer(log_obs);
    bus.add_observer(alert_obs);

    bus.send("user_login");
    bus.send("error_disk_full");
    return 0;
}
```

### 4.9 CRTP在真实项目中的应用

```cpp
// 应用1：STL中的CRTP
// std::enable_shared_from_this是最著名的CRTP应用
template<typename T>
class enable_shared_from_this {
public:
    std::shared_ptr<T> shared_from_this() {
        return std::shared_ptr<T>(weak_this_);
    }
    std::shared_ptr<const T> shared_from_this() const {
        return std::shared_ptr<const T>(weak_this_);
    }
private:
    mutable std::weak_ptr<T> weak_this_;
};

class Node : public std::enable_shared_from_this<Node> {
public:
    std::shared_ptr<Node> get_self() {
        return shared_from_this();  // 安全获取shared_ptr
    }
};

// 应用2：Eigen库中的CRTP
// Eigen使用CRTP实现表达式模板
template<typename Derived>
class MatrixBase {
public:
    const Derived& derived() const {
        return static_cast<const Derived&>(*this);
    }
    // 通用接口...
};

template<typename Scalar, int Rows, int Cols>
class Matrix : public MatrixBase<Matrix<Scalar, Rows, Cols>> {
    // 具体实现...
};

// 应用3：Boost迭代器库
// Boost.Iterator使用CRTP简化迭代器的实现
template<typename Derived, typename Value>
class iterator_facade {
public:
    Value operator*() const {
        return static_cast<const Derived*>(this)->dereference();
    }
    Derived& operator++() {
        static_cast<Derived*>(this)->increment();
        return *static_cast<Derived*>(this);
    }
    // ...
};

// 应用4：游戏引擎中的组件系统
// 使用CRTP实现编译期组件类型安全
template<typename Derived>
class Component {
public:
    static constexpr std::size_t component_id() {
        return typeid(Derived).hash_code();
    }

    void update(float dt) {
        static_cast<Derived*>(this)->update_impl(dt);
    }
};

class Transform : public Component<Transform> {
public:
    void update_impl(float dt) { /* 更新位置 */ }
};

class Physics : public Component<Physics> {
public:
    void update_impl(float dt) { /* 物理模拟 */ }
};
```

### 4.10 CRTP最佳实践

```cpp
// 实践1：始终添加static_assert验证派生关系
template<typename Derived>
class SafeCRTPBase {
    static_assert(std::is_base_of_v<SafeCRTPBase<Derived>, Derived>,
        "CRTP派生类必须继承自Base<Derived>");
    // ...
};

// 实践2：将不依赖Derived的代码提取到非模板基类
class CommonBase {
protected:
    int common_data_;
    void common_method() { /* 不依赖Derived */ }
};

template<typename Derived>
class CRTPBase : public CommonBase {
public:
    void specific_method() {
        common_method();  // 使用公共代码
        static_cast<Derived*>(this)->impl();  // 依赖Derived
    }
};

// 实践3：CRTP与Concepts结合（C++20）
template<typename T>
concept HasImpl = requires(T t) {
    t.impl();
};

template<typename Derived> requires HasImpl<Derived>
class ConceptCRTPBase {
public:
    void interface() {
        static_cast<Derived*>(this)->impl();
    }
};

// 实践4：为CRTP基类提供默认实现
template<typename Derived>
class DefaultCRTPBase {
public:
    void interface() {
        // 默认实现
        static_cast<Derived*>(this)->impl();
    }

    // 提供默认impl，派生类可以选择覆盖
    void impl() {
        // 默认空实现
    }
};

// 实践5：避免在构造/析构中调用CRTP方法
template<typename Derived>
class SafeCRTP {
public:
    SafeCRTP() {
        // 不要在这里调用Derived的方法
    }
    ~SafeCRTP() {
        // 不要在这里调用Derived的方法
    }

    // 提供安全的初始化方法
    void init() {
        // 对象完全构造后调用
        static_cast<Derived*>(this)->on_init();
    }
};

// 实践6：考虑使用variant替代CRTP处理异构容器
// 当需要异构容器时，CRTP不适合
// 使用std::variant + std::visit替代
using Shape = std::variant<Circle, Rectangle, Triangle>;

double total_area(const std::vector<Shape>& shapes) {
    double sum = 0;
    for (const auto& s : shapes) {
        sum += std::visit([](const auto& shape) {
            return shape.area();
        }, s);
    }
    return sum;
}
```

---

## 小结

本章介绍了CRTP的核心原理和应用：

| 应用场景 | CRTP的作用 | 优势 |
|---------|-----------|------|
| 接口扩展 | 为派生类自动添加功能 | 避免重复代码 |
| 代码复用 | 运算符、通用方法复用 | 只实现核心方法 |
| 编译期多态 | 策略模式、算法选择 | 零运行时开销 |
| 链式调用 | Builder模式 | 类型安全的链式API |
| 混入 | 添加属性（不可复制等） | 灵活组合 |

**关键要点**：

1. **CRTP通过static_cast实现编译期分派**：无虚函数开销
2. **CRTP不适合需要异构容器的场景**：考虑类型擦除或variant
3. **避免在构造/析构中调用CRTP方法**：对象可能不完整
4. **注意代码膨胀**：提取公共代码到非模板基类
5. **C++20 Concepts可以替代部分CRTP**：更清晰的接口约束
6. **CRTP是零开销抽象的经典实现**：理解其原理对阅读STL源码至关重要

CRTP是C++模板元编程中最重要的设计模式之一，在STL（如std::enable_shared_from_this）和各大框架中广泛使用。掌握CRTP将帮助你编写更高效的C++代码。
