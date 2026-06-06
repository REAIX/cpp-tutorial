# explicit 关键字详解
> 📖 相关章节：[类与对象](../../02-CPP/03-类与对象.md)、[继承与多态](../../02-CPP/04-继承与多态.md)、[核心机制](../../02-CPP/05-核心机制.md)、[异常处理](../../02-CPP/07-异常处理.md)、[移动语义](../../02-CPP/09-移动语义与完美转发.md)

### 1. 一句话结论

**`explicit` 只用来禁止隐式转换，只能修饰构造函数和类型转换运算符。**

***

### 2. 隐式转换的问题

#### 1. 什么是隐式转换

编译器在需要某种类型时，自动调用构造函数或转换运算符将另一种类型转换过来，无需程序员显式写出。

```cpp
class String {
public:
    String(int n);        // 分配长度 n 的字符串
    String(const char* s); // 用 C 字符串构造
};

String s = 10;   // 隐式转换：int → String，等价于 String s(10);
```

#### 2. 隐式转换的坑

```cpp
class String {
public:
    String(int n);        // 分配长度 n
    String(const char* s); // 字符串构造
};

void printStr(const String& s);

// 你想打印一个字符串，但不小心传了一个字符
printStr('A');  // char → int(ASCII 65) → String(65)
               // 编译器不报错！但创建了长度为65的空字符串
```

**BUG 藏得极深，编译器不报错！**

#### 3. 更多隐式转换的灾难

```cpp
class Fraction {
    int numerator, denominator;
public:
    Fraction(int n) : numerator(n), denominator(1) {}
};

Fraction f = 5;    // 隐式转换：5 → Fraction(5)
f = 10;            // 隐式转换：10 → Fraction(10)，然后赋值

// 更危险的例子
void process(const Fraction& f);

process(42);       // 合法！但可能不是你想要的
process(3.14);     // double → int(3) → Fraction(3)，精度丢失！
```

***

### 3. explicit 的作用

**`explicit` = 禁止编译器自动使用该构造函数/转换运算符做隐式转换，只允许显式调用。**

#### 1. 加上 explicit 后

```cpp
class String {
public:
    explicit String(int n);        // 禁止隐式转换
    explicit String(const char* s);
};

String s1(10);      // OK：显式调用
String s2 = 10;     // 编译错误！禁止隐式转换
String s3 = String(10);  // OK：显式构造

void printStr(const String& s);
printStr(String(10));   // OK：显式构造
printStr(10);           // 编译错误！禁止隐式转换
```

#### 2. 对比表格

| 写法 | 无 explicit | 有 explicit |
|:----:|:-----------:|:-----------:|
| `String s(10);` | OK | OK |
| `String s = 10;` | OK（隐式转换） | 编译错误 |
| `String s = String(10);` | OK | OK |
| `func(10);`（参数为 String） | OK（隐式转换） | 编译错误 |
| `func(String(10));` | OK | OK |

***

### 4. explicit 修饰构造函数

#### 1. 单参数构造函数（最常见场景）

```cpp
class Widget {
public:
    explicit Widget(int id) : id_(id) {}
private:
    int id_;
};

Widget w1(42);          // OK：直接初始化
Widget w2 = 42;         // 编译错误！隐式转换被禁止
Widget w3 = Widget(42); // OK：显式构造后拷贝初始化
```

#### 2. 多参数构造函数（C++11 起）

C++11 之前，`explicit` 只能用于单参数构造函数。C++11 起可用于多参数构造函数：

```cpp
class Point {
public:
    explicit Point(int x, int y) : x_(x), y_(y) {}
private:
    int x_, y_;
};

Point p1(1, 2);          // OK
Point p2 = {1, 2};       // C++11 编译错误！列表初始化的隐式转换被禁止
Point p3 = Point(1, 2);  // OK
```

#### 3. explicit 与列表初始化的交互

```cpp
class Complex {
public:
    explicit Complex(double r, double i) : real(r), imag(i) {}
private:
    double real, imag;
};

Complex c1(1.0, 2.0);         // OK：直接初始化
Complex c2 = {1.0, 2.0};      // 编译错误！explicit 禁止列表初始化隐式转换
Complex c3{1.0, 2.0};         // OK：列表直接初始化（C++11）
```

***

### 5. explicit 修饰转换运算符

#### 1. 基本语法

```cpp
class Num {
    int val;
public:
    Num(int v) : val(v) {}

    // 隐式转换运算符
    operator int() const { return val; }

    // explicit 转换运算符
    explicit operator bool() const { return val != 0; }
};
```

#### 2. 使用方式

```cpp
Num n(42);

// 隐式转换运算符（无 explicit）
int a = n;       // OK：自动调用 operator int()

// explicit 转换运算符
bool b = n;           // 编译错误！explicit 禁止隐式转换
bool c = static_cast<bool>(n);  // OK：显式转换
bool d = (bool)n;     // OK：C 风格显式转换
if (n) { ... }        // OK：条件上下文中 explicit operator bool 仍可用
```

#### 3. 为什么 operator bool 通常加 explicit

```cpp
class Stream {
public:
    // 如果不加 explicit
    operator bool() const { return !eof; }

    // 隐式转换会导致荒谬的代码合法化
    Stream s;
    int n = s + 1;   // 如果 operator bool 隐式转换，s → bool → int，然后 +1
                     // 这完全是胡闹，但编译通过！
};

// 加上 explicit 后
class Stream {
public:
    explicit operator bool() const { return !eof; }

    int n = s + 1;   // 编译错误！不能隐式转换为 bool
    if (s) { ... }   // OK：条件上下文允许
};
```

**C++11 标准库中的 `operator bool` 几乎都加了 `explicit`**，如 `std::unique_ptr`、`std::shared_ptr`、`std::basic_ios` 等。

***

### 6. C++14 的 explicit constexpr

C++14 允许 `explicit` 与 `constexpr` 同时使用：

```cpp
class FixedPoint {
    int value;
public:
    explicit constexpr FixedPoint(int v) : value(v) {}

    explicit constexpr operator int() const { return value; }
};

constexpr FixedPoint fp(42);                    // OK
// constexpr FixedPoint fp2 = 42;               // 编译错误！explicit
constexpr int v = static_cast<int>(fp);         // OK
```

***

### 7. C++20 的 explicit(bool)

C++20 允许 `explicit` 接受一个编译期 bool 表达式，条件性地禁止隐式转换：

```cpp
template<typename T>
class Wrapper {
public:
    // 只有当 T 不是 int 时才禁止隐式转换
    explicit(!std::is_same_v<T, int>) Wrapper(T v) : value(v) {}
private:
    T value;
};

Wrapper<int> w1 = 42;      // OK：int 的隐式转换允许
Wrapper<double> w2 = 3.14; // 编译错误！double 的隐式转换被禁止
Wrapper<double> w3(3.14);  // OK：显式构造
```

#### 1. 实际应用：std::pair 的构造函数

```cpp
// C++20 std::pair 的声明（简化）
template<typename U1, typename U2>
explicit(!std::is_convertible_v<U1, T1> || !std::is_convertible_v<U2, T2>)
constexpr pair(U1&& x, U2&& y);
```

这意味着：只有当两个元素类型都能隐式转换时，pair 的构造函数才允许隐式转换。

***

### 8. explicit 的使用规则

#### 1. 规则1：只能写在类内声明处

```cpp
class A {
public:
    explicit A(int x);  // 正确：声明处加 explicit
};

// 错误：定义处不能加 explicit
// explicit A::A(int x) {}
A::A(int x) {}  // 正确：定义处不加
```

#### 2. 规则2：不能用于非构造函数/转换运算符

```cpp
class A {
public:
    explicit void fun();  // 编译错误！
    explicit ~A();        // 编译错误！
    explicit int getValue(); // 编译错误！
};
```

#### 3. 规则3：拷贝/移动构造函数不应加 explicit

```cpp
class A {
public:
    explicit A(const A& other);  // 合法但极不推荐
    // 会导致：A a1; A a2 = a1; 编译错误
    // 只能：A a2(a1);
};
```

***

### 9. 设计指南

#### 1. 什么时候加 explicit

| 场景 | 是否加 explicit | 原因 |
|:----:|:--------------:|:----:|
| 单参数构造函数 | **必须加** | 防止意外隐式转换 |
| 多参数构造函数 | 建议加 | 防止列表初始化的隐式转换 |
| `operator bool()` | **必须加** | 防止算术运算等荒谬操作 |
| 其他转换运算符 | 视情况 | 如果转换语义不自然，加 explicit |
| 拷贝/移动构造 | 不加 | 拷贝/移动不应被禁止 |
| 默认构造 | 不加 | 无参数，不存在隐式转换问题 |

#### 2. 什么时候不加 explicit

```cpp
// 合理的隐式转换：类型之间有自然的映射关系
class Celsius {
    double temp;
public:
    // 不加 explicit：允许 double → Celsius 的隐式转换
    // 因为温度值本身就是数值，转换很自然
    Celsius(double t) : temp(t) {}
};

void setTemperature(Celsius c);
setTemperature(36.5);  // 自然、合理
```

#### 3. 判断标准

**如果从 A 到 B 的转换在语义上是"自然的、无歧义的"，可以不加 explicit；否则必须加。**

- `int → Fraction`：不自然（42 是整数还是 42/1？）→ 加 explicit
- `string → Name`：不自然（字符串不一定是名字）→ 加 explicit
- `double → Celsius`：自然（温度就是数值）→ 可以不加
- `SmartPtr → bool`：不自然（指针不是布尔值）→ 加 explicit

***

### 10. 常见误区

#### 1. 误区1："explicit 禁止所有转换"

```cpp
class A {
public:
    explicit A(int x) {}
};

A a1(42);           // OK：显式调用
A a2 = A(42);       // OK：显式构造后拷贝
A a3 = (A)42;       // OK：C 风格强转
A a4 = static_cast<A>(42);  // OK：C++ 风格强转

// explicit 只禁止"隐式"转换，不禁止"显式"转换
```

#### 2. 误区2："explicit 只能用于单参数构造函数"

C++11 起可以用于多参数构造函数：

```cpp
class Point {
public:
    explicit Point(int x, int y) {}
};

Point p = {1, 2};  // 编译错误！explicit 禁止列表初始化隐式转换
```

#### 3. 误区3："explicit operator bool 不能用于 if 条件"

```cpp
class SmartPtr {
public:
    explicit operator bool() const { return ptr != nullptr; }
};

SmartPtr p;
if (p) { ... }           // OK！条件上下文允许 explicit operator bool
while (p) { ... }        // OK
bool b = p;              // 编译错误！非条件上下文
bool b2 = p ? true : false;  // OK：三元运算符的条件部分
```

***

### 11. 极简总结

| 要点 | 内容 |
|:----:|:----:|
| 作用 | 禁止隐式转换，只允许显式调用 |
| 适用位置 | 构造函数、类型转换运算符 |
| 不适用 | 普通成员函数、析构函数、拷贝/移动构造 |
| C++11 | 多参数构造函数也可加 explicit |
| C++14 | explicit constexpr 合法 |
| C++20 | explicit(bool) 条件性禁止 |
| 核心规则 | 单参构造必加 explicit |
| 判断标准 | 转换不自然 → 加；自然 → 可不加 |

***

### 相关阅读

- [构造函数成员初始化列表](./01-构造函数成员初始化列表.md)
- [什么是统一初始化Uniform-Initialization](./31-什么是统一初始化Uniform-Initialization.md)
- [什么是initializer-list](./32-什么是initializer-list.md)