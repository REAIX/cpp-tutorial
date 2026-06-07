# 什么是强制拷贝消除与RVO局限
> 📖 相关章节：[类与对象](../../02-CPP/03-类与对象.md)、[继承与多态](../../02-CPP/04-继承与多态.md)、[核心机制](../../02-CPP/05-核心机制.md)、[异常处理](../../02-CPP/07-异常处理.md)、[移动语义](../../02-CPP/09-移动语义与完美转发.md)

> **📖 本文定位**：聚焦 C++17 prvalue 语义变革带来的强制拷贝消除保证，以及 RVO 不能覆盖的场景。
>
> **🔗 相关阅读**：
> - [RVO与NRVO返回值优化](31-RVO与NRVO返回值优化.md) — 聚焦 RVO/NRVO 的基本原理和工作机制，是理解本文的前置知识
> - [什么是返回值优化失败的情况](33-什么是返回值优化失败的情况.md) — 聚焦 8 种 RVO 失败的具体场景、编译器差异对比及检查方法

> C++17 的强制拷贝消除（Mandatory Copy Elision）让 prvalue 不再产生临时对象——但 NRVO 仍然是可选优化。

***

### 1. 核心要义

C++17 规定：返回 prvalue（纯右值）时必须消除拷贝/移动，这是语言保证；而返回具名局部变量（NRVO）时编译器可以消除但非强制，且存在多种失败场景。

***

### 2. 拷贝消除的两大类别

| 类别 | 全称 | C++ 标准 | 强制性 | 典型场景 |
|------|------|---------|--------|---------|
| URVO | Unnamed Return Value Optimization | C++17 起强制 | **强制** | `return T(args);` |
| NRVO | Named Return Value Optimization | 所有版本可选 | **可选** | `return named_obj;` |

```cpp
struct Widget {
    Widget() { std::cout << "默认构造\n"; }
    Widget(const Widget&) { std::cout << "拷贝构造\n"; }
    Widget(Widget&&) { std::cout << "移动构造\n"; }
};

Widget urvo_example() {
    return Widget(42);
}

Widget nrvo_example() {
    Widget w;
    return w;
}
```

**C++17 之前 vs 之后对比：**

| 场景 | C++14 | C++17 |
|------|-------|-------|
| `return Widget(42);` | 可能消除（可选） | **必须消除**（强制） |
| `return w;`（具名） | 可能消除（可选） | 可能消除（仍可选） |
| `T a = Widget(42);` | 可能消除（可选） | **必须消除**（强制） |

***

### 3. prvalue 语义变革：C++17 的核心变化

C++17 重新定义了 prvalue 的含义：prvalue 不再是对象，而是对象的"配方"（recipe）。

```
C++14 语义：
  Widget(42) → 创建临时对象 → 可能拷贝/移动到目标

C++17 语义：
  Widget(42) → 直接在目标位置构造，无临时对象
```

**关键概念对比：**

| 概念 | C++14 | C++17 |
|------|-------|-------|
| prvalue | 临时对象 | 构造指令（配方） |
| `Widget(42)` | 创建临时对象 | 描述如何构造一个 Widget |
| 拷贝消除 | 编译器优化 | 语言保证（prvalue 场景） |
| 移动构造 | 可能被调 | 根本不存在临时对象 |

```cpp
struct Heavy {
    Heavy(int x) { std::cout << "构造 " << x << "\n"; }
    Heavy(const Heavy&) { std::cout << "拷贝构造\n"; }
    Heavy(Heavy&&) { std::cout << "移动构造\n"; }
};

Heavy create() {
    return Heavy(42);
}

int main() {
    Heavy h = create();
}
```

**C++17 输出**（保证）：

```
构造 42
```

**C++14 可能输出**（依赖优化）：

```
构造 42
移动构造
```

***

### 4. 何时强制消除：prvalue 场景清单

以下场景在 C++17 中**保证**拷贝消除：

```cpp
struct Widget {
    Widget(int) {}
    Widget(const Widget&) = delete;
    Widget(Widget&&) = delete;
};

Widget case1() {
    return Widget(42);
}

Widget case2() {
    return 42;
}

Widget case3(int x) {
    return x;
}

Widget case4() {
    if (true) {
        return Widget(1);
    }
    return Widget(2);
}

int main() {
    Widget w1 = Widget(42);
    Widget w2 = case1();
    Widget w3 = static_cast<Widget>(42);
}
```

| 强制消除场景 | 示例 | 原因 |
|-------------|------|------|
| 返回临时对象 | `return Widget(42);` | prvalue 直接构造到目标 |
| 返回字面量转换 | `return 42;` | 隐式转换产生 prvalue |
| 初始化时 prvalue | `Widget w = Widget(42);` | prvalue 直接构造 w |
| static_cast 产生 prvalue | `static_cast<Widget>(42)` | cast 产生 prvalue |
| 条件分支返回 prvalue | 多个 `return Widget(n);` | 每个分支都是 prvalue |
| 函数参数传递 prvalue | `f(Widget(42))` | prvalue 直接构造到参数位置 |

> **关键测试**：如果拷贝/移动构造函数是 `= delete`，代码仍能编译，说明强制消除生效。

***

### 5. 何时仅可选消除：NRVO 场景

NRVO 是编译器优化，不是语言保证。以下场景编译器**可以**消除但**不保证**：

```cpp
Widget nrvo_simple() {
    Widget w;
    return w;
}

Widget nrvo_with_work() {
    Widget w;
    w.do_something();
    return w;
}

Widget nrvo_conditional(bool flag) {
    Widget a;
    Widget b;
    if (flag) return a;
    return b;
}
```

| NRVO 场景 | 是否可能消除 | 说明 |
|-----------|------------|------|
| 单一具名变量返回 | ✅ 通常消除 | 经典 NRVO |
| 多个返回路径返回同一变量 | ✅ 通常消除 | 编译器可统一处理 |
| 不同变量不同路径返回 | ❌ 通常失败 | 编译器无法确定构造位置 |
| 返回参数 | ❌ 失败 | 参数已在调用者栈上 |
| 返回成员变量 | ❌ 失败 | 成员已在对象内部 |
| 返回全局/静态变量 | ❌ 失败 | 存储位置已固定 |

```cpp
Widget nrvo_same_var(bool flag) {
    Widget w;
    if (flag) {
        w.set(1);
        return w;
    }
    w.set(2);
    return w;
}
```

> 上述 `nrvo_same_var` 返回同一个变量 `w`，NRVO 通常生效。

***

### 6. NRVO 失败的典型场景

**场景1：多个返回路径返回不同变量**

```cpp
Widget fail_multi_var(bool flag) {
    Widget a;
    Widget b;
    if (flag) return a;
    return b;
}
```

编译器无法在调用者的返回值槽同时放置 `a` 和 `b`，NRVO 失败，触发移动构造。

**场景2：返回函数参数**

```cpp
Widget fail_param(Widget w) {
    w.modify();
    return w;
}
```

参数 `w` 在调用者的栈帧上，不在返回值槽中，NRVO 不适用。会触发移动构造。

**场景3：返回成员变量**

```cpp
struct Container {
    Widget member;
    Widget fail_member() {
        return member;
    }
};
```

成员变量已在 `Container` 对象内部，无法直接放置到返回值槽，触发拷贝构造。

**场景4：返回全局/静态变量**

```cpp
Widget g_widget;

Widget fail_global() {
    return g_widget;
}
```

全局变量地址固定，必须拷贝。

**场景5：抛出异常时**

```cpp
Widget fail_exception() {
    Widget w;
    throw w;
}
```

`throw` 表达式总是拷贝异常对象到异常存储区。

***

### 7. std::move 如何破坏 NRVO

这是最常见的性能陷阱：在 return 语句中使用 `std::move` 反而阻止了 NRVO。

```cpp
struct Widget {
    Widget() {}
    Widget(const Widget&) { std::cout << "拷贝\n"; }
    Widget(Widget&&) { std::cout << "移动\n"; }
};

Widget bad() {
    Widget w;
    return std::move(w);
}

Widget good() {
    Widget w;
    return w;
}
```

| 函数 | 行为 | 输出 |
|------|------|------|
| `good()` | NRVO 生效 | （无输出） |
| `bad()` | NRVO 被阻止，强制移动 | `移动` |

**C++11 起的规则**：当返回的是局部具名变量时，编译器会自动将 `return w;` 视为 `return std::move(w);`（如果 NRVO 失败）。因此**永远不要**在 return 语句中显式写 `std::move`。

```cpp
Widget worst_practice() {
    Widget w;
    return std::move(w);
}

Widget best_practice() {
    Widget w;
    return w;
}
```

| 写法 | NRVO | NRVO 失败时 | 结论 |
|------|------|-----------|------|
| `return w;` | ✅ 尝试消除 | 自动退化为移动 | ✅ 最佳 |
| `return std::move(w);` | ❌ 被阻止 | 强制移动 | ❌ 永远更差 |

> **核心原则**：`return w;` 在 NRVO 失败时自动退化为移动，而 `return std::move(w);` 主动阻止了 NRVO。永远不要对返回的局部变量使用 `std::move`。

***

### 8. -fno-elide-constructors 与编译器差异

**禁用拷贝消除的编译选项：**

| 编译器 | 选项 | 说明 |
|--------|------|------|
| GCC/Clang | `-fno-elide-constructors` | 禁用所有拷贝消除（包括 NRVO 和 URVO） |
| MSVC | 无直接等价选项 | MSVC 始终执行消除，无法关闭 |

```bash
g++ -std=c++17 -fno-elide-constructors test.cpp && ./a.out
```

> **注意**：C++17 的强制拷贝消除**不受** `-fno-elide-constructors` 影响。该选项只影响 NRVO 等可选消除。

**验证代码：**

```cpp
#include <iostream>

struct Tracker {
    int val;
    Tracker(int v) : val(v) { std::cout << "构造(" << val << ")\n"; }
    Tracker(const Tracker& o) : val(o.val) { std::cout << "拷贝(" << val << ")\n"; }
    Tracker(Tracker&& o) noexcept : val(o.val) { std::cout << "移动(" << val << ")\n"; }
};

Tracker test_urvo() {
    return Tracker(1);
}

Tracker test_nrvo() {
    Tracker t(2);
    return t;
}

int main() {
    std::cout << "=== URVO ===\n";
    auto a = test_urvo();
    std::cout << "=== NRVO ===\n";
    auto b = test_nrvo();
}
```

**C++17 + 默认选项输出：**

```
=== URVO ===
构造(1)
=== NRVO ===
构造(2)
```

**C++17 + -fno-elide-constructors 输出：**

```
=== URVO ===
构造(1)
=== NRVO ===
构造(2)
移动(2)
```

URVO 不受影响（强制消除），NRVO 被禁用。

***

### 9. 实战指导原则

| 原则 | 说明 | 示例 |
|------|------|------|
| 优先返回 prvalue | 利用强制消除 | `return Widget(42);` |
| 不要对返回值用 std::move | 阻止 NRVO | `return w;` 而非 `return std::move(w);` |
| 保持单一返回路径 | 利于 NRVO | 同一变量，一个出口 |
| 不要过早优化 | NRVO 通常生效 | 信任编译器 |
| 按值返回，不要按引用输出 | 值返回可触发消除 | `Widget f()` 优于 `void f(Widget&)` |
| 确保 move 构造 noexcept | NRVO 失败时走移动 | `Widget(Widget&&) noexcept` |

**按值返回 vs 输出参数：**

```cpp
void output_param(std::vector<int>& out) {
    out.resize(100);
    for (int i = 0; i < 100; ++i) out[i] = i;
}

std::vector<int> return_by_value() {
    std::vector<int> result(100);
    for (int i = 0; i < 100; ++i) result[i] = i;
    return result;
}
```

| 方式 | 拷贝/移动 | 可读性 | 推荐 |
|------|---------|--------|------|
| 输出参数 | 零（直接写入） | 差 | ❌ 除非性能瓶颈 |
| 按值返回 | 零（NRVO） | 好 | ✅ 推荐 |

**工厂函数模式：**

```cpp
Widget create_widget(int type) {
    if (type == 1) {
        return Widget(1);
    }
    if (type == 2) {
        return Widget(2);
    }
    return Widget(0);
}
```

每个分支返回 prvalue，C++17 保证强制消除。

***

### 10. 极简总结

| 概念 | 核心要点 |
|------|---------|
| C++17 强制消除 | prvalue 直接在目标位置构造，无临时对象 |
| NRVO | 编译器可选优化，通常生效但不保证 |
| `return Widget(42);` | 强制消除，零开销 |
| `return w;` | NRVO 可能消除，失败时自动移动 |
| `return std::move(w);` | **永远错误**，阻止 NRVO，不会比 `return w;` 更好 |
| `-fno-elide-constructors` | 只影响可选消除，不影响 C++17 强制消除 |
| 多路径返回不同变量 | NRVO 失败，触发移动 |
| 返回参数/成员/全局 | NRVO 不适用，触发拷贝或移动 |

**记忆口诀**：prvalue 必消除，NRVO 看编译器，return 别加 move，按值返回最优雅。

***

### 相关阅读

- [RVO与NRVO返回值优化](31-RVO与NRVO返回值优化.md)
- [什么是返回值优化失败的情况](33-什么是返回值优化失败的情况.md)
- [左值右值与将亡值](18-左值右值与将亡值.md)