# RVO 与 NRVO 返回值优化
> 📖 相关章节：[移动语义与完美转发](../../02-CPP/09-移动语义与完美转发.md)、[核心机制](../../02-CPP/05-核心机制.md)

> **📖 本文定位**：聚焦 RVO/NRVO 的基本原理和工作机制，包括无 RVO/有 RVO 的对比、C++17 保证、"不要 return std::move"。
>
> **🔗 相关阅读**：
> - [什么是强制拷贝消除与RVO局限](32-什么是强制拷贝消除与RVO局限.md) — 聚焦 C++17 prvalue 语义变革带来的强制拷贝消除保证，以及 RVO 不能覆盖的场景
> - [什么是返回值优化失败的情况](33-什么是返回值优化失败的情况.md) — 聚焦 8 种 RVO 失败的具体场景、编译器差异对比及检查方法

### 1. 要义概览

**RVO**（Return Value Optimization）= 编译器直接在调用者的栈帧上构造返回值，省去拷贝/移动。**不要对返回值用 std::move**，反而会阻止 RVO。C++17 保证 RVO 必定发生。

***

### 2. 没有 RVO 时发生了什么

```cpp
class Widget {
public:
    Widget() { cout << "Default construct\n"; }
    Widget(const Widget&) { cout << "Copy construct\n"; }
    Widget(Widget&&) { cout << "Move construct\n"; }
    ~Widget() { cout << "Destruct\n"; }
};

Widget createWidget() {
    Widget w;          // 1. 默认构造 w
    return w;          // 2. 拷贝构造临时对象（w → 临时对象）
}                       // 3. 析构 w
                        // 4. 拷贝构造目标（临时对象 → 目标）
                        // 5. 析构临时对象

Widget w = createWidget();
// 没有 RVO 时输出：
// Default construct    ← 构造 w
// Copy construct       ← 拷贝到临时对象
// Destruct             ← 析构 w
// Copy construct       ← 拷贝到目标
// Destruct             ← 析构临时对象
// 共 2 次拷贝，5 步操作
```

**没有 RVO 的开销**：

| 步骤 | 操作 | 开销 |
|------|------|------|
| 1 | 构造局部对象 | 1次构造 |
| 2 | 拷贝到返回值 | 1次拷贝 |
| 3 | 析构局部对象 | 1次析构 |
| 4 | 拷贝到目标 | 1次拷贝 |
| 5 | 析构临时对象 | 1次析构 |

### 3. 有 RVO 时发生了什么

```cpp
Widget createWidget() {
    Widget w;          // 直接在目标位置构造！
    return w;          // 不需要拷贝！
}

Widget w = createWidget();
// 有 RVO 时输出：
// Default construct    ← 直接在 w 的位置构造
// 共 1 次构造，0 次拷贝
```

**RVO 的原理**：

```
没有 RVO：
调用者栈帧          被调函数栈帧
+----------+        +----------+
| w        |  ←---- | 临时对象  |  ←---- | 局部 w   |
+----------+        +----------+        +----------+
  拷贝1               拷贝2              构造

有 RVO：
调用者栈帧
+----------+
| w        |  ← 直接在这里构造，没有拷贝
+----------+
被调函数直接在调用者指定的地址上构造对象
```

### 4. RVO vs NRVO

| 类型 | 全称 | 场景 | 示例 |
|------|------|------|------|
| RVO | Return Value Optimization | 返回无名临时对象 | `return Widget(42);` |
| NRVO | Named Return Value Optimization | 返回命名局部对象 | `Widget w; return w;` |

**RVO（返回临时对象）**：

```cpp
Widget createWidget() {
    return Widget(42);  // 返回无名临时对象
}

Widget w = createWidget();
// RVO 输出：Default construct（只有1次构造）
```

**NRVO（返回命名对象）**：

```cpp
Widget createWidget(int x) {
    Widget w;           // 命名局部对象
    w.setValue(x);
    return w;           // 返回命名对象
}

Widget w = createWidget(42);
// NRVO 输出：Default construct（只有1次构造）
```

**RVO vs NRVO 的可靠性**：

| 类型 | C++17 前 | C++17 起 |
|------|:---:|:---:|
| RVO | 编译器可选优化 | **强制保证** |
| NRVO | 编译器可选优化 | 仍是可选优化 |

### 5. C++17 保证 RVO

C++17 对返回临时对象的 RVO 做了强制保证（称为"保证的拷贝消除"）。

```cpp
class Widget {
public:
    Widget() { cout << "Construct\n"; }
    Widget(const Widget&) = delete;  // 禁止拷贝！
    Widget(Widget&&) = delete;       // 禁止移动！
    ~Widget() { cout << "Destruct\n"; }
};

// C++17 前：编译错误！需要拷贝/移动构造函数
// C++17 起：编译通过！RVO 保证不调用拷贝/移动
Widget createWidget() {
    return Widget(42);  // C++17 保证直接构造
}

Widget w = createWidget();  // OK in C++17
```

**C++17 保证的条件**：

```cpp
// 保证 RVO：返回纯右值（prvalue）
Widget createWidget() {
    return Widget(42);  // prvalue → 保证 RVO
}

// 不保证 NRVO：返回左值（lvalue）
Widget createWidget() {
    Widget w;
    return w;  // lvalue → NRVO 是可选的
}

// 保证 RVO：类型转换
Widget createWidget() {
    return 42;  // 隐式转换产生 prvalue → 保证 RVO
}
```

### 6. 什么时候 RVO/NRVO 不生效

**情况1：返回类型与函数返回类型不同**

```cpp
class Base { public: virtual ~Base() = default; };
class Derived : public Base {};

Base createBase() {
    Derived d;
    return d;  // NRVO 可能不生效（Derived → Base，类型不同）
}
```

**情况2：多个返回路径返回不同命名对象**

```cpp
Widget createWidget(bool flag) {
    Widget a;
    Widget b;
    if (flag) {
        return a;  // NRVO 不生效：返回 a 还是 b？
    }
    return b;      // 编译器无法确定返回哪个
}

// 修复：统一返回同一个对象
Widget createWidget(bool flag) {
    Widget w;
    if (flag) {
        w.setA();
    } else {
        w.setB();
    }
    return w;  // NRVO 可以生效
}
```

**情况3：返回参数**

```cpp
Widget process(Widget w) {
    w.modify();
    return w;  // RVO 不生效（w 是参数，不是局部对象）
}

// 修复：直接传引用修改
void process(Widget& w) {
    w.modify();
}
```

**情况4：返回全局/成员变量**

```cpp
Widget g_widget;

Widget getWidget() {
    return g_widget;  // RVO 不生效（g_widget 不是局部对象）
}

class Holder {
    Widget widget_;
public:
    Widget getWidget() {
        return widget_;  // RVO 不生效（widget_ 是成员变量）
    }
};
```

**情况5：返回条件表达式**

```cpp
Widget createWidget(bool flag) {
    Widget a, b;
    return flag ? a : b;  // RVO 不生效（条件表达式）
}

// 修复：用 if-else 统一返回
Widget createWidget(bool flag) {
    Widget w;
    if (flag) {
        // 配置 w
    } else {
        // 配置 w
    }
    return w;  // NRVO 可以生效
}
```

**NRVO 失效条件总结**：

| 条件 | NRVO 是否生效 |
|------|:---:|
| 返回同一个命名局部对象 | 生效 |
| 返回不同命名对象 | 不生效 |
| 返回参数 | 不生效 |
| 返回全局/成员变量 | 不生效 |
| 返回条件表达式 | 不生效 |
| 返回类型与函数返回类型不同 | 可能不生效 |

### 7. 不要 return std::move

这是最常见的 RVO 误用：对返回值使用 `std::move` 反而会阻止 NRVO。

```cpp
// 错误：阻止 NRVO
Widget createWidget() {
    Widget w;
    return std::move(w);  // 把 w 变成右值引用 → 编译器必须走移动语义 → NRVO 失效
}

// 输出：
// Default construct    ← 构造 w
// Move construct       ← 移动到返回值（NRVO 被阻止了！）
// Destruct             ← 析构 w

// 正确：让编译器做 NRVO
Widget createWidget() {
    Widget w;
    return w;  // NRVO 生效，零拷贝零移动
}

// 输出：
// Default construct    ← 直接在目标位置构造
```

**为什么 std::move 会阻止 NRVO**：

```cpp
// return w; 的处理：
// 编译器看到 w 是局部对象，尝试 NRVO → 直接在目标位置构造
// 如果 NRVO 不生效，w 被当作右值 → 调用移动构造（C++11 自动优化）

// return std::move(w); 的处理：
// std::move(w) 返回 Widget&&
// 编译器看到返回的是引用，不是局部对象 → NRVO 条件不满足
// 只能走移动构造 → 比 NRVO 多一次移动

// 结论：return w; 在最坏情况下也会走移动语义，不需要 std::move
```

**C++11 的自动右值化**：

```cpp
Widget createWidget() {
    Widget w;
    return w;  // NRVO 优先；如果 NRVO 不生效，w 自动被当作右值
}

// C++11 标准：当返回局部对象时，即使 NRVO 不生效，
// 编译器也会把 w 当作右值，优先使用移动构造而非拷贝构造
// 所以 return w; 在最坏情况下也是移动，不是拷贝
```

### 8. 工厂函数模式

RVO/NRVO 在工厂函数中特别重要。

```cpp
// 工厂函数：返回多态对象
std::unique_ptr<Shape> createShape(ShapeType type) {
    switch (type) {
        case ShapeType::Circle:
            return std::make_unique<Circle>(5.0);  // RVO
        case ShapeType::Rectangle:
            return std::make_unique<Rectangle>(3.0, 4.0);  // RVO
    }
    return nullptr;
}

// 工厂函数：返回值类型
Widget createWidget(Config config) {
    Widget w;  // NRVO
    w.setName(config.name);
    w.setValue(config.value);
    return w;
}

// 工厂函数：不同条件返回不同配置
Widget createWidget(bool advanced) {
    Widget w;  // NRVO
    if (advanced) {
        w.setAdvancedMode();
    }
    return w;
}
```

**参数构造 + RVO**：

```cpp
// 好的写法：利用 RVO
Widget createWidget(int x, int y) {
    return Widget(x, y);  // RVO，直接在目标位置构造
}

// 不好的写法：先构造再修改
Widget createWidget(int x, int y) {
    Widget w;      // 构造
    w.setX(x);     // 修改
    w.setY(y);     // 修改
    return w;      // NRVO（可能生效）
}
```

### 9. 完整示例：RVO 与 NRVO

```cpp
#include <iostream>
using namespace std;

class Widget {
    int id_;
    static int counter_;
public:
    Widget() : id_(++counter_) { cout << "  Widget() id=" << id_ << "\n"; }
    Widget(int x) : id_(++counter_) { cout << "  Widget(int) id=" << id_ << "\n"; }
    Widget(const Widget& other) : id_(++counter_) {
        cout << "  Widget(const Widget&) id=" << id_ << " from id=" << other.id_ << "\n";
    }
    Widget(Widget&& other) noexcept : id_(++counter_) {
        cout << "  Widget(Widget&&) id=" << id_ << " from id=" << other.id_ << "\n";
    }
    ~Widget() { cout << "  ~Widget() id=" << id_ << "\n"; }
    Widget& operator=(const Widget&) = default;
    int id() const { return id_; }
};
int Widget::counter_ = 0;

// RVO：返回临时对象
Widget createRVO() {
    return Widget(42);
}

// NRVO：返回命名对象
Widget createNRVO() {
    Widget w;
    return w;
}

// NRVO 失效：多个返回路径
Widget createNoNRVO(bool flag) {
    Widget a, b;
    if (flag) return a;
    return b;
}

// 错误：std::move 阻止 NRVO
Widget createBad() {
    Widget w;
    return std::move(w);  // 阻止 NRVO
}

// 正确：让编译器优化
Widget createGood() {
    Widget w;
    return w;  // NRVO 或移动语义
}

// 工厂模式
Widget createWidget(int type) {
    Widget w;  // NRVO
    switch (type) {
        case 1: /* 配置 w */ break;
        case 2: /* 配置 w */ break;
        default: /* 配置 w */ break;
    }
    return w;
}

int main() {
    cout << "=== RVO ===\n";
    {
        Widget w = createRVO();
        cout << "  result id=" << w.id() << "\n";
    }

    cout << "\n=== NRVO ===\n";
    {
        Widget w = createNRVO();
        cout << "  result id=" << w.id() << "\n";
    }

    cout << "\n=== No NRVO (multiple returns) ===\n";
    {
        Widget w = createNoNRVO(true);
        cout << "  result id=" << w.id() << "\n";
    }

    cout << "\n=== Bad: std::move prevents NRVO ===\n";
    {
        Widget w = createBad();
        cout << "  result id=" << w.id() << "\n";
    }

    cout << "\n=== Good: let compiler optimize ===\n";
    {
        Widget w = createGood();
        cout << "  result id=" << w.id() << "\n";
    }

    cout << "\n=== Factory pattern ===\n";
    {
        Widget w = createWidget(1);
        cout << "  result id=" << w.id() << "\n";
    }

    return 0;
}
```

**预期输出（有优化时）**：

```
=== RVO ===
  Widget(int) id=1       ← 只有一次构造
  result id=1
  ~Widget() id=1

=== NRVO ===
  Widget() id=2          ← 只有一次构造
  result id=2
  ~Widget() id=2

=== Bad: std::move prevents NRVO ===
  Widget() id=3          ← 构造局部对象
  Widget(Widget&&) id=4  ← 移动构造（NRVO 被阻止！）
  ~Widget() id=3         ← 析构局部对象
  result id=4
  ~Widget() id=4

=== Good: let compiler optimize ===
  Widget() id=5          ← 只有一次构造（NRVO 生效）
  result id=5
  ~Widget() id=5
```

### 10. 常见陷阱

**陷阱1：对返回值用 std::move**

```cpp
// 错误
return std::move(w);  // 阻止 NRVO，强制移动

// 正确
return w;  // NRVO 优先，最坏也是移动
```

**陷阱2：在 return 语句中做额外操作**

```cpp
// 可能阻止 NRVO
Widget create() {
    Widget w;
    return (w);  // OK，括号不影响
    // return w, doSomething();  // 逗号表达式，可能阻止 NRVO
}

// 正确
Widget create() {
    Widget w;
    doSomething();  // 操作在 return 之前
    return w;
}
```

**陷阱3：不同分支返回不同对象**

```cpp
// NRVO 不生效
Widget create(bool flag) {
    Widget a, b;
    return flag ? a : b;  // 两个不同对象
}

// NRVO 可以生效
Widget create(bool flag) {
    Widget w;
    if (flag) w.setA();
    else w.setB();
    return w;  // 同一个对象
}
```

**陷阱4：C++17 保证只适用于 prvalue**

```cpp
// C++17 保证 RVO
Widget create() {
    return Widget(42);  // prvalue → 保证
}

// C++17 不保证 NRVO
Widget create() {
    Widget w;
    return w;  // lvalue → 不保证（但通常优化器会做）
}
```

### 11. 最佳实践

1. **永远不要 `return std::move(局部对象)`**，让编译器做 NRVO
2. **优先返回临时对象**（`return Widget(42);`），C++17 保证 RVO
3. **统一返回同一个命名对象**，让 NRVO 有机会生效
4. **不要为了 RVO 而过度设计**，编译器很聪明
5. **如果 NRVO 不生效**，C++11 自动退化为移动语义，不是拷贝
6. **工厂函数返回 unique_ptr**，利用 RVO 和移动语义
7. **用 `-O2` 或 `/O2` 编译**，确保优化开启

### 12. 极简总结

**RVO = 编译器省去返回值拷贝 | 不要 return std::move（阻止 NRVO）| C++17 保证 RVO 必定发生 | NRVO 仍是可选优化 | return w 最坏也是移动不是拷贝 | 统一返回同一个对象让 NRVO 生效**

***

### 相关阅读

- [什么是强制拷贝消除与RVO局限](32-什么是强制拷贝消除与RVO局限.md)
- [什么是返回值优化失败的情况](33-什么是返回值优化失败的情况.md)
- [左值右值与将亡值](18-左值右值与将亡值.md)