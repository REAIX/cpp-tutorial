# CRTP 模式与静态多态
> 📖 相关章节：[类与对象](../../02-CPP/03-类与对象.md)、[继承与多态](../../02-CPP/04-继承与多态.md)、[核心机制](../../02-CPP/05-核心机制.md)、[异常处理](../../02-CPP/07-异常处理.md)、[移动语义](../../02-CPP/09-移动语义与完美转发.md)

### 1. 一句话概括

**CRTP**（Curiously Recurring Template Pattern）= **奇异递归模板模式**：派生类把自己作为基类的模板参数，在编译期实现多态，零运行时开销。

***

### 2. CRTP 长什么样

```cpp
template <typename Derived>
class Base {
public:
    void interface() {
        static_cast<Derived*>(this)->implementation();
    }
};

class Concrete : public Base<Concrete> {
public:
    void implementation() {
        // 具体实现
    }
};
```

**关键**：`Concrete` 继承 `Base<Concrete>`，把自身类型传给基类。基类通过 `static_cast<Derived*>(this)` 将自己"向下转型"为派生类，从而调用派生类的实现。

### 3. 为什么叫"奇异递归"

- **递归**：`Concrete` 出现在自己的基类 `Base<Concrete>` 的模板参数里，看起来像自己引用自己
- **奇异**：派生类还没定义完，就已经出现在基类定义中了——这在普通继承中不可能发生
- **本质**：这不是真正的递归，基类模板在派生类之前就已实例化，派生类只是作为模板参数被"提前引用"

```
普通继承：  class Dog : public Animal {}     // Dog 继承 Animal
CRTP继承：  class Dog : public Base<Dog> {}   // Dog 继承 Base<Dog>
                                              // Dog 出现在自己的基类中！
```

### 4. CRTP 的工作原理

CRTP 的核心机制是 **编译期静态分发**：

```cpp
template <typename Derived>
class Base {
public:
    void interface() {
        // 编译期就知道 Derived 的具体类型
        // static_cast 是安全的，因为 this 实际上指向 Derived 对象
        static_cast<Derived*>(this)->implementation();
    }

    void common_logic() {
        // 基类可以提供通用逻辑
        std::cout << "before" << std::endl;
        static_cast<Derived*>(this)->implementation();
        std::cout << "after" << std::endl;
    }
};

class ImplA : public Base<ImplA> {
public:
    void implementation() {
        std::cout << "ImplA" << std::endl;
    }
};

class ImplB : public Base<ImplB> {
public:
    void implementation() {
        std::cout << "ImplB" << std::endl;
    }
};

int main() {
    ImplA a;
    a.interface();    // 输出: ImplA
    a.common_logic(); // 输出: before\nImplA\nafter

    ImplB b;
    b.interface();    // 输出: ImplB
}
```

编译器为 `Base<ImplA>` 和 `Base<ImplB>` 分别生成代码，调用目标在编译期就已确定。

### 5. 静态多态 vs 动态多态

| 特性 | 虚函数（动态多态） | CRTP（静态多态） |
|------|:---:|:---:|
| 绑定时机 | 运行时 | 编译期 |
| 虚函数表 | 需要 | 不需要 |
| 运行时开销 | 有（vtable查找） | 零 |
| 代码膨胀 | 无 | 有（每个类型生成一份） |
| 灵活性 | 高（运行时换类型） | 低（编译期确定） |
| 异构容器 | 支持（基类指针） | 不支持（不同基类实例） |
| 编译速度 | 快 | 慢（模板实例化） |
| 错误信息 | 友好 | 复杂难读 |
| 二进制兼容 | 好 | 差 |

#### 1. 动态多态写法对比

```cpp
// 动态多态：虚函数
class Shape {
public:
    virtual double area() const = 0;
    virtual ~Shape() = default;
};

class Circle : public Shape {
    double r_;
public:
    Circle(double r) : r_(r) {}
    double area() const override { return 3.14159 * r_ * r_; }
};

// 可以用基类指针统一管理
std::vector<std::unique_ptr<Shape>> shapes;
shapes.push_back(std::make_unique<Circle>(1.0));
```

```cpp
// 静态多态：CRTP
template <typename Derived>
class Shape {
public:
    double area() const {
        return static_cast<const Derived*>(this)->area_impl();
    }
};

class Circle : public Shape<Circle> {
    double r_;
public:
    Circle(double r) : r_(r) {}
    double area_impl() const { return 3.14159 * r_ * r_; }
};

// 不能用基类指针统一管理！Shape<Circle> 和 Shape<Rectangle> 是不同类型
```

### 6. CRTP 实战：静态接口

```cpp
#include <iostream>
#include <cmath>

template <typename Derived>
class Shape {
public:
    double area() const {
        return static_cast<const Derived*>(this)->area_impl();
    }
    double perimeter() const {
        return static_cast<const Derived*>(this)->perimeter_impl();
    }
    const char* name() const {
        return static_cast<const Derived*>(this)->name_impl();
    }

    // 基类提供的通用方法
    void print() const {
        std::cout << name() << ": area=" << area()
                  << ", perimeter=" << perimeter() << std::endl;
    }
};

class Circle : public Shape<Circle> {
    double r_;
public:
    explicit Circle(double r) : r_(r) {}
    double area_impl() const { return 3.14159265 * r_ * r_; }
    double perimeter_impl() const { return 2 * 3.14159265 * r_; }
    const char* name_impl() const { return "Circle"; }
};

class Rectangle : public Shape<Rectangle> {
    double w_, h_;
public:
    Rectangle(double w, double h) : w_(w), h_(h) {}
    double area_impl() const { return w_ * h_; }
    double perimeter_impl() const { return 2 * (w_ + h_); }
    const char* name_impl() const { return "Rectangle"; }
};

class Triangle : public Shape<Triangle> {
    double a_, b_, c_;
public:
    Triangle(double a, double b, double c) : a_(a), b_(b), c_(c) {}
    double area_impl() const {
        double s = (a_ + b_ + c_) / 2;
        return std::sqrt(s * (s - a_) * (s - b_) * (s - c_));
    }
    double perimeter_impl() const { return a_ + b_ + c_; }
    const char* name_impl() const { return "Triangle"; }
};

// 使用模板函数实现"静态多态容器"
template <typename T>
void print_shape(const Shape<T>& s) {
    s.print();
}

int main() {
    Circle c(5.0);
    Rectangle r(3.0, 4.0);
    Triangle t(3.0, 4.0, 5.0);

    print_shape(c); // Circle: area=78.5398, perimeter=31.4159
    print_shape(r); // Rectangle: area=12, perimeter=14
    print_shape(t); // Triangle: area=6, perimeter=12
}
```

### 7. CRTP 实战：Mixin 代码复用

CRTP 最实用的场景之一是为多个类注入相同的功能，且每个类有独立的状态：

```cpp
#include <iostream>
#include <string>

// Mixin 1：对象计数器
template <typename Derived>
class Counter {
    static int count_;
public:
    Counter() { ++count_; }
    Counter(const Counter&) { ++count_; }
    Counter(Counter&&) noexcept { ++count_; }
    ~Counter() { --count_; }
    static int get_count() { return count_; }
};

template <typename Derived>
int Counter<Derived>::count_ = 0;

class Widget : public Counter<Widget> {
    std::string name_;
public:
    explicit Widget(std::string name) : name_(std::move(name)) {}
};

class Gadget : public Counter<Gadget> {
    int id_;
public:
    explicit Gadget(int id) : id_(id) {}
};

// Widget 和 Gadget 各有独立的计数器
// Counter<Widget>::count_ 和 Counter<Gadget>::count_ 是不同的静态变量

int main() {
    Widget w1("btn1");
    Widget w2("btn2");
    Gadget g1(1);

    std::cout << Widget::get_count() << std::endl; // 2
    std::cout << Gadget::get_count() << std::endl; // 1
}
```

```cpp
// Mixin 2：唯一 ID 分配
template <typename Derived>
class UniqueId {
    static int next_id_;
    int id_;
public:
    UniqueId() : id_(next_id_++) {}
    int id() const { return id_; }
};

template <typename Derived>
int UniqueId<Derived>::next_id_ = 0;

class Entity : public UniqueId<Entity> {};
class Particle : public UniqueId<Particle> {};

// Entity 和 Particle 的 ID 序列互不干扰
```

```cpp
// Mixin 3：可序列化
template <typename Derived>
class Serializable {
public:
    std::string to_json() const {
        return static_cast<const Derived*>(this)->to_json_impl();
    }

    void from_json(const std::string& json) {
        static_cast<Derived*>(this)->from_json_impl(json);
    }
};

class Config : public Serializable<Config> {
    int port_ = 8080;
    std::string host_ = "localhost";
public:
    std::string to_json_impl() const {
        return "{\"host\":\"" + host_ + "\",\"port\":" + std::to_string(port_) + "}";
    }
    void from_json_impl(const std::string& json) {
        // 解析 json...
    }
};
```

### 8. CRTP 实现编译期多态

```cpp
#include <iostream>
#include <vector>

// 编译期"多态"：通过模板实现
template <typename T>
void process_all(const std::vector<T>& items) {
    for (const auto& item : items) {
        item.process();  // 编译期确定调用哪个 process
    }
}

// CRTP 版本：带统一接口
template <typename Derived>
class Processor {
public:
    void process() {
        static_cast<Derived*>(this)->process_impl();
    }
};

class FastProcessor : public Processor<FastProcessor> {
public:
    void process_impl() {
        std::cout << "Fast processing" << std::endl;
    }
};

class SlowProcessor : public Processor<SlowProcessor> {
public:
    void process_impl() {
        std::cout << "Slow processing" << std::endl;
    }
};

int main() {
    std::vector<FastProcessor> fasts = {FastProcessor(), FastProcessor()};
    process_all(fasts); // 编译期绑定，无虚函数开销
}
```

### 9. CRTP 的陷阱

#### 1. 陷阱1：不能通过基类指针统一管理

```cpp
// 错误！Shape<Circle>* 和 Shape<Rectangle>* 是不同类型
// std::vector<Shape*> shapes;  // 编译错误！Shape 不是类，是模板

// 如果需要异构容器，只能用动态多态（虚函数）
// 或者用 std::variant + std::visit
```

#### 2. 陷阱2：static_cast 不安全

```cpp
// 如果传错模板参数，编译能过但运行崩溃！
class Wrong : public Base<SomeOtherClass> {  // 故意传错
    // Base 里的 static_cast<SomeOtherClass*>(this) 指向错误类型
    // 未定义行为！
};
```

#### 3. 陷阱3：调试困难

```cpp
// 编译错误信息极其复杂
// 普通虚函数错误：error: no member named 'area' in 'Circle'
// CRTP 错误：error: no member named 'area_impl' in
//   'Base<Circle>' with [Derived = Circle]
//   in instantiation of function template specialization
//   'Base<Circle>::area' requested here
//   ... 几十行模板展开 ...
```

#### 4. 陷阱4：拷贝和切片

```cpp
template <typename Derived>
class Base {
public:
    void interface() {
        static_cast<Derived*>(this)->implementation();
    }
};

// 危险：基类拷贝会切片
Derived d;
Base<Derived> b = d;  // 对象切片！b 不再指向 Derived
b.interface();         // static_cast<Derived*>(&b) 指向不完整的对象 → 未定义行为

// 正确做法：禁止基类单独使用
template <typename Derived>
class Base {
protected:
    Base() = default;               // 防止直接构造
    Base(const Base&) = default;
public:
    void interface() {
        static_cast<Derived*>(this)->implementation();
    }
};
```

### 10. 与虚函数的性能对比

```cpp
#include <chrono>
#include <iostream>

// 虚函数版本
class VirtualBase {
public:
    virtual int compute(int x) = 0;
    virtual ~VirtualBase() = default;
};

class VirtualImpl : public VirtualBase {
public:
    int compute(int x) override { return x * x + x; }
};

// CRTP 版本
template <typename Derived>
class CrtpBase {
public:
    int compute(int x) {
        return static_cast<Derived*>(this)->compute_impl(x);
    }
};

class CrtpImpl : public CrtpBase<CrtpImpl> {
public:
    int compute_impl(int x) { return x * x + x; }
};

// 性能测试
void benchmark() {
    const int N = 100000000;

    // 虚函数
    VirtualImpl vi;
    VirtualBase* vp = &vi;
    auto t1 = std::chrono::high_resolution_clock::now();
    int sum1 = 0;
    for (int i = 0; i < N; ++i) {
        sum1 += vp->compute(i);  // 虚函数调用：vptr → vtable → 函数
    }
    auto t2 = std::chrono::high_resolution_clock::now();

    // CRTP
    CrtpImpl ci;
    auto t3 = std::chrono::high_resolution_clock::now();
    int sum2 = 0;
    for (int i = 0; i < N; ++i) {
        sum2 += ci.compute(i);  // 直接调用，可内联
    }
    auto t4 = std::chrono::high_resolution_clock::now();

    auto virtual_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    auto crtp_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count();

    std::cout << "Virtual: " << virtual_ms << "ms" << std::endl;
    std::cout << "CRTP:    " << crtp_ms << "ms" << std::endl;
    // CRTP 通常更快，因为编译器可以内联 compute_impl
}
```

| 对比项 | 虚函数 | CRTP |
|--------|--------|------|
| 调用方式 | 间接调用（vtable查表） | 直接调用（可内联） |
| 内联可能性 | 低（编译器通常不内联虚函数） | 高（编译期确定目标） |
| 指令缓存 | 间接跳转，可能miss | 直接跳转，更友好 |
| 实际差距 | — | 通常快 10%~50%（取决于场景） |

### 11. CRTP 最佳实践

1. **优先用虚函数**：除非性能分析表明虚函数是瓶颈
2. **构造函数设为 protected**：防止直接构造基类导致切片
3. **用 `static_cast` 不用 `dynamic_cast`**：CRTP 本身就是编译期机制
4. **派生类必须提供基类期望的接口**：没有编译期检查，靠约定
5. **考虑 C++20 Concepts**：可以约束派生类必须提供哪些方法

```cpp
// C++20 用 Concepts 约束 CRTP
template <typename T>
concept ShapeLike = requires(const T& t) {
    { t.area_impl() } -> std::convertible_to<double>;
    { t.name_impl() } -> std::convertible_to<const char*>;
};

template <ShapeLike Derived>
class Shape {
public:
    double area() const {
        return static_cast<const Derived*>(this)->area_impl();
    }
    const char* name() const {
        return static_cast<const Derived*>(this)->name_impl();
    }
};
```

### 12. 完整示例：CRTP 实现编译期策略模式

```cpp
#include <iostream>
#include <vector>
#include <memory>

// 策略基类（CRTP）
template <typename Strategy>
class Sorter {
public:
    void sort(std::vector<int>& data) {
        static_cast<Strategy*>(this)->sort_impl(data);
    }
    const char* name() const {
        return static_cast<const Strategy*>(this)->name_impl();
    }
};

// 策略1：冒泡排序
class BubbleSort : public Sorter<BubbleSort> {
public:
    void sort_impl(std::vector<int>& data) {
        for (size_t i = 0; i < data.size(); ++i)
            for (size_t j = 0; j + 1 < data.size() - i; ++j)
                if (data[j] > data[j + 1])
                    std::swap(data[j], data[j + 1]);
    }
    const char* name_impl() const { return "BubbleSort"; }
};

// 策略2：快速排序
class QuickSort : public Sorter<QuickSort> {
public:
    void sort_impl(std::vector<int>& data) {
        qsort(data, 0, static_cast<int>(data.size()) - 1);
    }
    const char* name_impl() const { return "QuickSort"; }
private:
    void qsort(std::vector<int>& d, int lo, int hi) {
        if (lo >= hi) return;
        int pivot = d[hi], i = lo - 1;
        for (int j = lo; j < hi; ++j)
            if (d[j] <= pivot) std::swap(d[++i], d[j]);
        std::swap(d[i + 1], d[hi]);
        int mid = i + 1;
        qsort(d, lo, mid - 1);
        qsort(d, mid + 1, hi);
    }
};

// 使用
template <typename S>
void test_sort(Sorter<S>& sorter, std::vector<int> data) {
    std::cout << sorter.name() << ": ";
    sorter.sort(data);
    for (int x : data) std::cout << x << " ";
    std::cout << std::endl;
}

int main() {
    BubbleSort bs;
    QuickSort qs;
    std::vector<int> v1 = {5, 3, 1, 4, 2};
    std::vector<int> v2 = {5, 3, 1, 4, 2};

    test_sort(bs, v1);  // BubbleSort: 1 2 3 4 5
    test_sort(qs, v2);  // QuickSort: 1 2 3 4 5
}
```

### 13. 极简总结

**CRTP = 把自己传给基类当模板参数 → 编译期多态 → 零开销但灵活性低**

| 场景 | 推荐 |
|------|------|
| 需要运行时多态 | 虚函数 |
| 性能关键路径 + 编译期类型确定 | CRTP |
| 为多个类注入相同功能（Mixin） | CRTP |
| 需要异构容器 | 虚函数 / std::variant |
| C++20 项目 | Concepts + CRTP |

***

### 相关阅读

- [动态绑定与静态绑定](./10-动态绑定与静态绑定.md)
- [什么是RTTI及其性能影响](./26-什么是RTTI及其性能影响.md)
- [什么是类型擦除](./23-什么是类型擦除.md)