# 什么是对象切片 Object Slicing
> 📖 相关章节：[类与对象](../../02-CPP/03-类与对象.md)、[继承与多态](../../02-CPP/04-继承与多态.md)、[核心机制](../../02-CPP/05-核心机制.md)、[异常处理](../../02-CPP/07-异常处理.md)、[移动语义](../../02-CPP/09-移动语义与完美转发.md)

> "当你把派生类对象赋值给基类变量时，派生部分就像被刀切掉了一样。"

***

### 1. 精髓速览

对象切片是 C++ 值语义下，将派生类对象拷贝到基类对象时，派生类独有的数据成员和虚表指针被截断丢弃的现象，导致多态行为丧失。

***

### 2. 对象切片是怎么发生的

对象切片的本质是 C++ 的值拷贝语义。当派生类对象被拷贝到基类类型的变量时，编译器只拷贝基类部分的数据，派生类新增的成员被"切掉"。

```cpp
#include <iostream>
#include <string>

class Animal {
public:
    Animal(const std::string& name) : name_(name) {}
    virtual void speak() const { std::cout << name_ << ": ..." << std::endl; }
    virtual ~Animal() = default;
    std::string name_;
};

class Dog : public Animal {
public:
    Dog(const std::string& name, const std::string& breed)
        : Animal(name), breed_(breed) {}
    void speak() const override { std::cout << name_ << ": 汪汪!" << std::endl; }
    std::string breed_;
};

int main() {
    Dog dog("旺财", "柴犬");
    Animal animal = dog;

    animal.speak();

    std::cout << "sizeof(Animal): " << sizeof(Animal) << std::endl;
    std::cout << "sizeof(Dog):    " << sizeof(Dog) << std::endl;
    std::cout << "sizeof(animal): " << sizeof(animal) << std::endl;
}
```

输出：

```
旺财: ...
sizeof(Animal): 40
sizeof(Dog):    64
sizeof(animal): 40
```

`animal` 的 `sizeof` 只有 `Animal` 的大小，`breed_` 成员已被切掉，虚表指针也回退到了 `Animal` 的虚表。

### 3. 函数参数中的切片陷阱

函数按值传递基类参数时，切片悄无声息地发生，这是最常见的 bug 来源之一。

```cpp
#include <iostream>
#include <string>
#include <vector>

class Shape {
public:
    virtual double area() const { return 0.0; }
    virtual ~Shape() = default;
};

class Circle : public Shape {
public:
    Circle(double r) : radius_(r) {}
    double area() const override { return 3.14159 * radius_ * radius_; }
private:
    double radius_;
};

class Rectangle : public Shape {
public:
    Rectangle(double w, double h) : width_(w), height_(h) {}
    double area() const override { return width_ * height_; }
private:
    double width_;
    double height_;
};

void printAreaByValue(Shape s) {
    std::cout << "按值传递: area = " << s.area() << std::endl;
}

void printAreaByRef(const Shape& s) {
    std::cout << "按引用传递: area = " << s.area() << std::endl;
}

int main() {
    Circle c(5.0);
    Rectangle r(3.0, 4.0);

    printAreaByValue(c);
    printAreaByRef(c);

    printAreaByValue(r);
    printAreaByRef(r);
}
```

输出：

```
按值传递: area = 0
按引用传递: area = 78.5397
按值传递: area = 0
按引用传递: area = 12
```

按值传递时，所有派生类信息丢失，`area()` 调用的是 `Shape::area()`，返回 0。

| 传递方式 | 是否切片 | 多态行为 | 推荐程度 |
|---------|---------|---------|---------|
| `Shape s` | ✅ 切片 | ❌ 丢失 | ❌ 禁止 |
| `const Shape& s` | ❌ 不切片 | ✅ 保留 | ✅ 推荐 |
| `Shape* s` | ❌ 不切片 | ✅ 保留 | ✅ 推荐 |
| `std::unique_ptr<Shape>` | ❌ 不切片 | ✅ 保留 | ✅ 推荐 |

### 4. 容器中的切片：vector\<Base\> 陷阱

将派生类对象存入 `vector<Base>` 时，每个元素都会被切片。

```cpp
#include <iostream>
#include <vector>
#include <memory>
#include <string>

class Shape {
public:
    virtual double area() const { return 0.0; }
    virtual std::string name() const { return "Shape"; }
    virtual ~Shape() = default;
};

class Circle : public Shape {
public:
    Circle(double r) : radius_(r) {}
    double area() const override { return 3.14159 * radius_ * radius_; }
    std::string name() const override { return "Circle"; }
private:
    double radius_;
};

class Triangle : public Shape {
public:
    Triangle(double b, double h) : base_(b), height_(h) {}
    double area() const override { return 0.5 * base_ * height_; }
    std::string name() const override { return "Triangle"; }
private:
    double base_;
    double height_;
};

void demoSlicingVector() {
    std::vector<Shape> shapes;
    shapes.push_back(Circle(5.0));
    shapes.push_back(Triangle(3.0, 4.0));

    for (const auto& s : shapes) {
        std::cout << s.name() << ": area = " << s.area() << std::endl;
    }
}

void demoPolymorphicVector() {
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(5.0));
    shapes.push_back(std::make_unique<Triangle>(3.0, 4.0));

    for (const auto& s : shapes) {
        std::cout << s->name() << ": area = " << s->area() << std::endl;
    }
}

int main() {
    std::cout << "=== 切片的 vector ===" << std::endl;
    demoSlicingVector();
    std::cout << "=== 正确的多态 vector ===" << std::endl;
    demoPolymorphicVector();
}
```

输出：

```
=== 切片的 vector ===
Shape: area = 0
Shape: area = 0
=== 正确的多态 vector ===
Circle: area = 78.5397
Triangle: area = 6
```

| 容器类型 | 是否切片 | 多态 | 所有权 |
|---------|---------|-----|-------|
| `vector<Base>` | ✅ 切片 | ❌ | 值语义 |
| `vector<Base*>` | ❌ | ✅ | 需手动 delete |
| `vector<unique_ptr<Base>>` | ❌ | ✅ | 自动管理 |
| `vector<shared_ptr<Base>>` | ❌ | ✅ | 共享所有权 |

### 5. 防止切片的五种策略

```cpp
#include <iostream>
#include <string>
#include <memory>

class Animal {
public:
    Animal(const std::string& name) : name_(name) {}
    virtual void speak() const { std::cout << name_ << ": ..." << std::endl; }
    virtual ~Animal() = default;

    Animal(const Animal&) = default;
    Animal& operator=(const Animal&) = default;
protected:
    std::string name_;
};

class Cat : public Animal {
public:
    Cat(const std::string& name, bool indoor)
        : Animal(name), indoor_(indoor) {}
    void speak() const override { std::cout << name_ << ": 喵~" << std::endl; }
private:
    bool indoor_;
};
```

**策略一：使用引用或指针**

```cpp
void process(const Animal& a) { a.speak(); }
void process(const Animal* a) { a->speak(); }
```

**策略二：删除基类的拷贝操作**

```cpp
class Animal {
public:
    Animal(const Animal&) = delete;
    Animal& operator=(const Animal&) = delete;
protected:
    Animal(Animal&&) = default;
    Animal& operator=(Animal&&) = default;
};
```

**策略三：使用 `final` 阻止继承**

```cpp
class Animal final {
public:
    Animal(const std::string& name) : name_(name) {}
    void speak() const { std::cout << name_ << ": ..." << std::endl; }
private:
    std::string name_;
};
```

**策略四：使用智能指针容器**

```cpp
std::vector<std::unique_ptr<Animal>> zoo;
zoo.push_back(std::make_unique<Cat>("咪咪", true));
```

**策略五：使用 `std::variant`（无继承多态）**

```cpp
#include <variant>

class Dog2 {
public:
    Dog2(const std::string& name) : name_(name) {}
    void speak() const { std::cout << name_ << ": 汪汪!" << std::endl; }
private:
    std::string name_;
};

class Cat2 {
public:
    Cat2(const std::string& name) : name_(name) {}
    void speak() const { std::cout << name_ << ": 喵~" << std::endl; }
private:
    std::string name_;
};

using Pet = std::variant<Dog2, Cat2>;

void speakPet(const Pet& p) {
    std::visit([](const auto& pet) { pet.speak(); }, p);
}
```

| 策略 | 侵入性 | 安全性 | 灵活性 | 适用场景 |
|-----|-------|-------|-------|---------|
| 引用/指针 | 低 | 中 | 高 | 函数参数 |
| 删除拷贝 | 中 | 高 | 中 | 不需要值拷贝 |
| `final` | 高 | 高 | 低 | 无需继承 |
| 智能指针 | 低 | 高 | 高 | 容器存储 |
| `variant` | 中 | 高 | 中 | 有限类型集 |

### 6. 对象切片 vs 多态：内存布局视角

```cpp
#include <iostream>
#include <cstdint>

class Base {
public:
    virtual void foo() {}
    int base_data_ = 0;
};

class Derived : public Base {
public:
    void foo() override {}
    int derived_data_ = 1;
    double extra_ = 3.14;
};

int main() {
    Derived d;
    Base b = d;

    std::cout << "sizeof(Base):    " << sizeof(Base) << std::endl;
    std::cout << "sizeof(Derived): " << sizeof(Derived) << std::endl;
    std::cout << "sizeof(b):       " << sizeof(b) << std::endl;

    std::cout << "\n--- 内存布局对比 ---" << std::endl;
    std::cout << "Base 对象:    [vptr][base_data_]" << std::endl;
    std::cout << "Derived 对象: [vptr][base_data_][derived_data_][extra_]" << std::endl;
    std::cout << "切片后:       [vptr(Base)][base_data_] <- 派生部分被截断" << std::endl;
}
```

内存布局图示：

```
Derived 对象内存布局:
+------------------+
| vptr (Derived)   |  <-- 指向 Derived 的虚表
+------------------+
| base_data_       |
+------------------+
| derived_data_    |  <-- 切片时被丢弃
+------------------+
| extra_           |  <-- 切片时被丢弃
+------------------+

切片后的 Base 对象:
+------------------+
| vptr (Base)      |  <-- 回退到 Base 的虚表
+------------------+
| base_data_       |
+------------------+
```

| 属性 | 切片前 (Derived) | 切片后 (Base) |
|-----|-----------------|--------------|
| vptr | 指向 Derived 虚表 | 指向 Base 虚表 |
| base_data_ | ✅ 保留 | ✅ 保留 |
| derived_data_ | ✅ 存在 | ❌ 丢失 |
| extra_ | ✅ 存在 | ❌ 丢失 |
| sizeof | 较大 | 较小 |
| foo() 调用 | Derived::foo() | Base::foo() |

### 7. 克隆模式（Clone Pattern）解决切片

当需要拷贝多态对象时，克隆模式通过虚函数返回派生类的拷贝，避免切片。

```cpp
#include <iostream>
#include <string>
#include <vector>
#include <memory>

class Shape {
public:
    virtual ~Shape() = default;
    virtual double area() const = 0;
    virtual std::string name() const = 0;
    virtual std::unique_ptr<Shape> clone() const = 0;
};

class Circle : public Shape {
public:
    Circle(double r) : radius_(r) {}
    double area() const override { return 3.14159 * radius_ * radius_; }
    std::string name() const override { return "Circle"; }
    std::unique_ptr<Shape> clone() const override {
        return std::make_unique<Circle>(radius_);
    }
private:
    double radius_;
};

class Rectangle : public Shape {
public:
    Rectangle(double w, double h) : width_(w), height_(h) {}
    double area() const override { return width_ * height_; }
    std::string name() const override { return "Rectangle"; }
    std::unique_ptr<Shape> clone() const override {
        return std::make_unique<Rectangle>(width_, height_);
    }
private:
    double width_;
    double height_;
};

class ShapeCollection {
public:
    void add(const Shape& shape) {
        shapes_.push_back(shape.clone());
    }

    ShapeCollection(const ShapeCollection& other) {
        for (const auto& s : other.shapes_) {
            shapes_.push_back(s->clone());
        }
    }

    ShapeCollection& operator=(const ShapeCollection& other) {
        if (this != &other) {
            shapes_.clear();
            for (const auto& s : other.shapes_) {
                shapes_.push_back(s->clone());
            }
        }
        return *this;
    }

    ShapeCollection(ShapeCollection&&) = default;
    ShapeCollection& operator=(ShapeCollection&&) = default;

    void report() const {
        for (const auto& s : shapes_) {
            std::cout << s->name() << ": area = " << s->area() << std::endl;
        }
    }

private:
    std::vector<std::unique_ptr<Shape>> shapes_;
};

int main() {
    ShapeCollection original;
    original.add(Circle(5.0));
    original.add(Rectangle(3.0, 4.0));

    ShapeCollection copy = original;

    std::cout << "原始集合:" << std::endl;
    original.report();
    std::cout << "拷贝集合:" << std::endl;
    copy.report();
}
```

输出：

```
原始集合:
Circle: area = 78.5397
Rectangle: area = 12
拷贝集合:
Circle: area = 78.5397
Rectangle: area = 12
```

### 8. 隐式切片的危险案例

切片不仅发生在赋值和函数传参中，还可能出现在条件表达式、返回值等隐蔽位置。

```cpp
#include <iostream>
#include <string>

class Base {
public:
    Base(int v = 0) : value_(v) {}
    virtual void print() const { std::cout << "Base: " << value_ << std::endl; }
    virtual ~Base() = default;
protected:
    int value_;
};

class Derived : public Base {
public:
    Derived(int v, const std::string& s) : Base(v), label_(s) {}
    void print() const override { std::cout << "Derived: " << value_ << " [" << label_ << "]" << std::endl; }
private:
    std::string label_;
};

Base dangerousReturn(bool flag) {
    if (flag) {
        Derived d(42, "important");
        return d;
    }
    return Base(0);
}

void ternarySlicing() {
    Base b;
    Derived d(10, "test");

    Base result = true ? b : d;
    result.print();
}

int main() {
    std::cout << "=== 返回值切片 ===" << std::endl;
    auto obj = dangerousReturn(true);
    obj.print();

    std::cout << "=== 三目运算符切片 ===" << std::endl;
    ternarySlicing();
}
```

输出：

```
=== 返回值切片 ===
Base: 42
=== 三目运算符切片 ===
Base: 0
```

| 切片场景 | 隐蔽程度 | 典型代码 |
|---------|---------|---------|
| 函数按值传参 | ⚠️ 中 | `void f(Base b)` |
| 赋值操作 | ⚠️ 中 | `base = derived` |
| 返回值 | 🔴 高 | `Base f() { return derived; }` |
| 三目运算符 | 🔴 高 | `cond ? base : derived` |
| 异常捕获 | 🔴 高 | `catch (Base e)` |
| 容器 push_back | ⚠️ 中 | `vector<Base>.push_back(derived)` |

### 9. 异常捕获中的切片

```cpp
#include <iostream>
#include <stdexcept>

class AppError : public std::runtime_error {
public:
    AppError(const std::string& msg, int code)
        : std::runtime_error(msg), code_(code) {}
    int code() const { return code_; }
private:
    int code_;
};

void demoCatchByValue() {
    try {
        throw AppError("数据库连接失败", 5003);
    } catch (std::runtime_error e) {
        std::cout << "按值捕获: " << e.what() << std::endl;
    }
}

void demoCatchByRef() {
    try {
        throw AppError("数据库连接失败", 5003);
    } catch (const std::runtime_error& e) {
        auto* app = dynamic_cast<const AppError*>(&e);
        if (app) {
            std::cout << "按引用捕获: " << e.what()
                      << ", code=" << app->code() << std::endl;
        }
    }
}

int main() {
    demoCatchByValue();
    demoCatchByRef();
}
```

输出：

```
按值捕获: 数据库连接失败
按引用捕获: 数据库连接失败, code=5003
```

按值捕获时，`AppError` 被切片为 `std::runtime_error`，`code_` 信息丢失，无法 `dynamic_cast` 回去获取错误码。

### 10. 极简总结

| 要点 | 说明 |
|-----|------|
| **定义** | 派生类对象拷贝到基类变量时，派生部分被截断 |
| **根因** | C++ 值语义 + 对象大小不同 + 编译器只拷贝基类部分 |
| **后果** | 虚表指针回退、数据丢失、多态失效 |
| **常见场景** | 按值传参、容器存储、返回值、三目运算符、异常捕获 |
| **核心防御** | 用引用/指针代替值传递 |
| **容器方案** | `vector<unique_ptr<Base>>` 或 `vector<shared_ptr<Base>>` |
| **拷贝方案** | 克隆模式（虚 `clone()` 方法） |
| **编译期防御** | 删除基类拷贝构造/赋值 |
| **替代多态** | `std::variant` + `std::visit` |

**口诀**：基类按值传，切片必发生；引用指针保多态，容器要用智能指针。

***

### 相关阅读

- [C++四种类型转换](10-C++四种类型转换.md)
- [向上转型与向下转型](11-向上转型与向下转型.md)
- [虚继承与菱形继承](13-虚继承与菱形继承.md)