# override 与 final 关键字
> 📖 相关章节：[继承与多态](../../02-CPP/04-继承与多态.md)

### 1. 要义概览

**override** = 告诉编译器"我要重写基类虚函数，请帮我检查"；**final** = "这个函数/类不能再被重写/继承了"。

两者都是 **C++11** 引入，用于**编译期检查**，防止继承体系中的常见错误。

***

### 2. override 的作用与必要性

#### 1. 没有 override 的世界

```cpp
class Base {
public:
    virtual void foo(int x);
};

class Derived : public Base {
public:
    void foo(double x);  // 签名不匹配！但编译器不报错
    // 这不是重写，而是隐藏（hide）
};

Derived d;
d.foo(42);    // 调用 Derived::foo(double)，不是 Base::foo(int)

Base* ptr = &d;
ptr->foo(42); // 调用 Base::foo(int)，不是 Derived::foo(double)
// 行为不一致！但编译器没有任何警告
```

#### 2. 有 override 的世界

```cpp
class Base {
public:
    virtual void foo(int x);
};

class Derived : public Base {
public:
    void foo(double x) override;  // 编译错误！签名不匹配
    // 编译器明确告诉你：没有可以重写的基类虚函数
};
```

**override 让编译器帮你检查，签名是否真的匹配基类虚函数。**

***

### 3. 常见签名不匹配场景

#### 1. 场景1：参数类型不同

```cpp
class Base {
public:
    virtual void process(int x);
};

class Derived : public Base {
public:
    void process(double x) override;  // 编译错误！int vs double
};
```

#### 2. 场景2：缺少 const 修饰

```cpp
class Base {
public:
    virtual void display() const;
};

class Derived : public Base {
public:
    void display() override;  // 编译错误！少了 const
    // 正确写法：
    void display() const override;
};
```

#### 3. 场景3：函数名拼写错误

```cpp
class Base {
public:
    virtual void calculate();
};

class Derived : public Base {
public:
    void calculat() override;  // 编译错误！拼写错误
    // 正确写法：
    void calculate() override;
};
```

#### 4. 场景4：基类函数不是虚函数

```cpp
class Base {
public:
    void foo();  // 没有 virtual！
};

class Derived : public Base {
public:
    void foo() override;  // 编译错误！基类 foo 不是虚函数
};
```

#### 5. 场景5：返回类型协变不正确

```cpp
class Base {
public:
    virtual Base* clone() const;
};

class Derived : public Base {
public:
    int* clone() const override;  // 编译错误！返回类型不协变
    // 正确的协变返回：
    Derived* clone() const override;  // OK：Derived* 是 Base* 的派生类指针
};
```

#### 6. 签名不匹配总表

| 错误类型 | 示例 | override 是否捕获 |
|:--------:|:----:|:----------------:|
| 参数类型不同 | `foo(int)` vs `foo(double)` | 是 |
| 缺少 const | `foo() const` vs `foo()` | 是 |
| 函数名拼写错误 | `calculate` vs `calculat` | 是 |
| 基类不是虚函数 | `void foo()` vs `virtual void foo()` | 是 |
| 返回类型不协变 | `Base*` vs `int*` | 是 |
| 参数数量不同 | `foo(int)` vs `foo(int, int)` | 是 |
| 引用限定符不同 | `foo() &` vs `foo() &&` | 是 |

***

### 4. override 的语法

```cpp
class Derived : public Base {
public:
    // override 写在函数声明的末尾
    void foo(int x) override;       // 正确
    override void foo(int x);       // 错误！override 不是返回类型
    void override foo(int x);       // 错误！位置不对
    void foo(int x) const override; // 正确：const 在 override 前
};
```

**位置规则**：`override` 放在 `const`/`volatile`/引用限定符之后，纯虚标记 `= 0` 之前。

```cpp
virtual void foo() const override;          // OK
virtual void foo() override const;          // 错误！
virtual void foo() const override = 0;      // 错误！override 函数不能是纯虚
void foo() const & override;                // OK
void foo() const override &;                // 错误！
```

***

### 5. final 的两种用法

#### 1. 用法1：禁止函数被重写

```cpp
class Base {
public:
    virtual void foo() final;  // 派生类不能再重写 foo
};

class Derived : public Base {
public:
    void foo() override;  // 编译错误！foo 是 final
};
```

#### 2. 用法2：禁止类被继承

```cpp
class Widget final {
    // ...
};

class Gadget : public Widget { };  // 编译错误！Widget 是 final 类
```

#### 3. final 函数 + 继承的组合

```cpp
class Base {
public:
    virtual void foo();
    virtual void bar() final;  // bar 不能被重写
};

class Middle : public Base {
public:
    void foo() override;   // OK：可以重写
    // void bar() override;  // 编译错误！bar 是 final
};

class FinalClass final : public Middle {
public:
    void foo() override;   // OK：可以重写
    // 但 FinalClass 不能再被继承
};

// class Further : public FinalClass {};  // 编译错误！FinalClass 是 final
```

***

### 6. final 的典型应用场景

#### 1. 场景1：性能优化提示

```cpp
class Parser {
public:
    virtual void tokenize() final;  // 告诉编译器：不需要虚函数派发
    // 编译器可以对 final 函数去虚化（devirtualization）
};
```

**注意**：`final` 不是性能优化的主要手段，编译器的去虚化优化通常不需要 `final` 提示。`final` 的主要价值是**语义约束**。

#### 2. 场景2：防止 API 被覆盖

```cpp
class SecureAuth {
public:
    virtual bool verify(const std::string& token) final;
    // 防止子类绕过验证逻辑
};
```

#### 3. 场景3：防止类被继承

```cpp
class Singleton final {
public:
    static Singleton& getInstance() {
        static Singleton instance;
        return instance;
    }

private:
    Singleton() = default;
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};

// 防止通过继承绕过单例约束
// class Hacker : public Singleton {};  // 编译错误！
```

#### 4. 场景4：密封类层次结构

```cpp
class Shape {
public:
    virtual double area() const = 0;
    virtual ~Shape() = default;
};

class Circle final : public Shape {
    // Circle 不允许进一步派生
    double area() const override;
};

class Rectangle final : public Shape {
    double area() const override;
};
```

***

### 7. override 与 final 的组合

```cpp
class Base {
public:
    virtual void foo();
    virtual void bar();
};

class Middle : public Base {
public:
    void foo() override;            // 重写 Base::foo
    void bar() override final;      // 重写 Base::bar，且禁止进一步重写
};

class Derived : public Middle {
public:
    void foo() override;            // OK：重写 Middle::foo
    // void bar() override;         // 编译错误！bar 是 final
};
```

**`override final` = 我重写了基类函数，且禁止我的派生类再重写。**

***

### 8. 与 Java/C# 的对比

| 特性 | C++ | Java | C# |
|:----:|:---:|:----:|:---:|
| 重写标记 | `override`（可选但推荐） | `@Override`（注解） | `override`（必须） |
| 禁止重写 | `final` | `final` | `sealed` |
| 禁止继承 | `class X final` | `final class X` | `sealed class X` |
| 默认行为 | 不写 override 也能重写 | 不写 @Override 也能重写 | 必须写 override |
| 编译器检查 | 写了 override 才检查 | 写了 @Override 才检查 | 始终检查 |

**C++ 和 Java 的共同问题**：override/Override 是可选的，不写也能编译通过。

**C# 更严格**：重写必须写 `override`，否则是隐藏（new）。

***

### 9. 编译器检查详解

#### 1. 没有 override 时编译器的行为

```cpp
class Base {
public:
    virtual void foo(int x);
};

class Derived : public Base {
public:
    void foo(double x);  // 编译器认为：这是新函数，不是重写
    // 不报错，不警告（默认情况下）
};
```

#### 2. 开启编译器警告

```bash
# GCC/Clang
g++ -Wall -Wextra -Woverloaded-virtual ...

# MSVC
/w4 /w44265
```

**但 override 比警告更可靠**：警告可以忽略，override 导致的编译错误无法忽略。

#### 3. override 的检查清单

编译器在看到 `override` 时，会检查：

1. 基类中是否存在同名虚函数
2. 参数类型、数量是否完全匹配
3. const/volatile 限定符是否匹配
4. 引用限定符（`&`/`&&`）是否匹配
5. 返回类型是否相同或协变

***

### 10. 常见陷阱

#### 1. 陷阱1：忘记写 override

```cpp
class Base {
public:
    virtual void foo() const;
};

class Derived : public Base {
public:
    void foo();  // 忘了 const，也忘了 override
    // 这不是重写！是隐藏！
    // 如果写了 override，编译器会立即报错
};
```

#### 2. 陷阱2：final 函数在中间类中

```cpp
class A {
public:
    virtual void foo();
};

class B : public A {
public:
    void foo() override final;  // B 中 foo 是 final
};

class C : public B {
public:
    // void foo() override;  // 编译错误！B::foo 是 final
    // C 无法重写 foo，但可以调用
};
```

#### 3. 陷阱3：final 类的成员函数不需要 final

```cpp
class Sealed final {
public:
    virtual void foo() final;  // 多余！类已经是 final，不可能有派生类
    virtual void bar();        // 足够，因为类不能被继承
};
```

#### 4. 陷阱4：override 与重载混淆

```cpp
class Base {
public:
    virtual void process(int x);
};

class Derived : public Base {
public:
    void process(int x) override;     // 重写
    void process(double x);           // 重载（新函数）
    // using Base::process;           // 如果需要两个重载都可见
};
```

***

### 11. 最佳实践

1. **所有重写的虚函数都加 override** -- 防止签名不匹配
2. **不想被重写的虚函数加 final** -- 明确意图
3. **不想被继承的类加 final** -- 防止误用
4. **新代码一律加 override** -- 即使编译器不强制
5. **代码审查时检查 override** -- 确保没有遗漏
6. **final 谨慎使用** -- 过度使用会限制扩展性

#### 1. override 检查清单

```cpp
class Derived : public Base {
public:
    // 每个重写的虚函数都检查：
    // 1. 是否加了 override？
    // 2. 签名是否与基类完全一致？
    // 3. const 是否匹配？
    // 4. 返回类型是否正确？

    void foo() const override;        // OK
    void bar(int x) override;         // 检查 Base 是否有 virtual void bar(int)
    void baz() override;              // 检查 Base 是否有 virtual void baz()
};
```

***

### 12. 完整示例

```cpp
#include <iostream>
#include <string>

// 基类：图形
class Shape {
public:
    virtual double area() const = 0;
    virtual std::string name() const = 0;
    virtual void describe() const {
        std::cout << name() << " with area " << area() << "\n";
    }
    virtual ~Shape() = default;
};

// 中间类：二维图形
class Shape2D : public Shape {
public:
    void describe() const override final {
        // 二维图形的描述方式固定，不允许派生类重写
        std::cout << "2D " << name() << " with area " << area() << "\n";
    }
};

// 具体类：圆形（final 类，不允许继承）
class Circle final : public Shape2D {
    double radius;
public:
    explicit Circle(double r) : radius(r) {}

    double area() const override {
        return 3.14159265 * radius * radius;
    }

    std::string name() const override {
        return "Circle";
    }
};

// 具体类：矩形（final 类）
class Rectangle final : public Shape2D {
    double width, height;
public:
    Rectangle(double w, double h) : width(w), height(h) {}

    double area() const override {
        return width * height;
    }

    std::string name() const override {
        return "Rectangle";
    }
};

int main() {
    Circle c(5.0);
    Rectangle r(3.0, 4.0);

    c.describe();  // 2D Circle with area 78.5398
    r.describe();  // 2D Rectangle with area 12

    // 多态
    Shape* shapes[] = {&c, &r};
    for (auto* s : shapes) {
        s->describe();
    }

    return 0;
}
```

***

### 13. 极简总结

| 要点 | override | final |
|:----:|:--------:|:-----:|
| 作用 | 编译器检查是否正确重写 | 禁止重写/禁止继承 |
| 修饰对象 | 虚函数 | 虚函数、类 |
| C++ 版本 | C++11 | C++11 |
| 是否必须 | 否（但强烈推荐） | 否（按需使用） |
| 不写时后果 | 签名不匹配不报错 | 无影响 |
| 最佳实践 | 所有重写都加 | 需要约束时加 |
| 与 Java 对比 | 类似 @Override | 类似 final |
| 与 C# 对比 | C# 必须写 override | 类似 sealed |

***

### 相关阅读

- [动态绑定与静态绑定](12-动态绑定与静态绑定.md)
- [向上转型与向下转型](11-向上转型与向下转型.md)
- [虚函数表vtable](../02-内存与底层/10-虚函数表vtable.md)