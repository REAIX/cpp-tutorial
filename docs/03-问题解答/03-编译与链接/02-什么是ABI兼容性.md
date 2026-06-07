# 什么是 ABI 兼容性
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)

> "API 是给人看的说明书，ABI 是给机器看的插头标准"——说明书一样不代表插头能插进去。

***

### 1. 核心定义

**ABI（Application Binary Interface）** = 编译后的二进制接口约定。它规定了编译器生成的机器码之间如何"对话"，包括：

- 函数怎么找到对方（名称改编）
- 数据在内存里怎么排（数据布局）
- 参数怎么传、返回值怎么收（调用约定）
- 异常怎么抛、栈怎么展开（异常处理）

**API 兼容 ≠ ABI 兼容**：源码层面接口没变，但编译后的二进制可能完全对不上。

***

### 2. 生活类比

**ABI = 插头标准，API = 电器说明书**

| 概念 | 类比 | 说明 |
|------|------|------|
| API | 电器说明书 | "这个电器需要 220V 交流电"——人读的 |
| ABI | 插头标准 | 国标/美标/欧标——物理形状必须匹配才能插进去 |

**API 兼容但 ABI 不兼容** = 说明书一样（都是 220V 交流电），但插头形状不同（国标插头插不进美标插座）。

现实例子：你升级了库的 `.so`/`.dll` 文件，头文件（API）没变，但程序一跑就崩——ABI 不兼容。

***

### 3. ABI 包含的四个维度

#### 1. 名称改编（Name Mangling）

C++ 支持函数重载、命名空间、类成员函数，编译器必须把函数签名编码成唯一的符号名。

```cpp
namespace Math {
    int add(int a, int b);
    double add(double a, double b);
}
```

GCC 编译后可能变成：

```
_ZN4Math3addEii      // Math::add(int, int)
_ZN4Math3addEdd      // Math::add(double, double)
```

MSVC 编译后可能变成：

```
?add@Math@@YAHHH@Z   // Math::add(int, int)
?add@Math@@YANNN@Z   // Math::add(double, double)
```

**问题**：不同编译器的改编规则不同，同一个函数编译出来的符号名不一样，链接时找不到对方。

#### 2. 数据布局

结构体在内存中的排列方式：大小、对齐、成员偏移量、vtable 指针位置。

```cpp
struct Config {
    int id;           // 偏移 0，4 字节
    double value;     // 偏移 8（对齐到 8），8 字节
    char name[16];    // 偏移 16，16 字节
};
// sizeof(Config) = 32（64 位系统）
```

**如果改了成员顺序或添加了新成员**：

```cpp
struct Config {
    bool active;      // 新增！偏移 0，1 字节
    int id;           // 偏移从 0 变成 4！
    double value;     // 偏移从 8 变成 8... 看似没变？
    char name[16];
};
```

旧代码按偏移 0 读 `id`，现在偏移 0 是 `active`——**数据全错**。

#### 3. 调用约定

参数怎么传给函数？返回值怎么拿回来？

| 约定 | 参数传递方式 | 谁清理栈 |
|------|------------|---------|
| cdecl | 从右到左压栈 | 调用方 |
| stdcall | 从右到左压栈 | 被调方 |
| fastcall | 前两个参数走寄存器 | 被调方 |
| x64 System V | 前6个整型参数走寄存器(rdi,rsi,rdx,rcx,r8,r9) | 调用方 |
| x64 MSVC | 前4个整型参数走寄存器(rcx,rdx,r8,r9) | 调用方 |

**问题**：Linux 和 Windows 的 x64 调用约定不同，同一个函数编译出来的二进制不能互调。

#### 4. 异常处理

C++ 异常的栈展开机制在不同编译器/平台之间不同：

- **Itanium C++ ABI**（GCC/Clang）：使用 `.gcc_except_table` + `_Unwind_RaiseException`
- **MSVC ABI**：使用 `RUNTIME_FUNCTION` + `__CxxFrameHandler3`

**问题**：一个用 GCC 编译的 `.so` 抛出异常，另一个用 MSVC 编译的 `.dll` 接不住——栈展开机制不兼容。

***

### 4. 常见 ABI 不兼容场景

#### 1. 场景1：添加/删除成员变量

```cpp
// v1.0
class User {
    int id;
    std::string name;
};
// sizeof(User) = 32

// v2.0：中间插了一个字段
class User {
    int id;
    bool active;    // 新增！
    std::string name;
};
// sizeof(User) = 40，所有偏移全变了
```

旧代码 `sizeof(User) = 32`，新代码 `sizeof(User) = 40`，内存分配和访问全错。

#### 2. 场景2：改变虚函数顺序

```cpp
// v1.0
class Shape {
public:
    virtual void draw() = 0;       // vtable[0]
    virtual double area() const = 0; // vtable[1]
};

// v2.0：交换了顺序
class Shape {
public:
    virtual double area() const = 0; // vtable[0]
    virtual void draw() = 0;         // vtable[1]
};
```

旧代码调 `draw()` 走 vtable[0]，新代码 vtable[0] 是 `area()`——**调错函数**。

#### 3. 场景3：换编译器

同一个源码，GCC 和 MSVC 编译出来的 `.so`/`.dll` 不能互调，因为：

- 名称改编规则不同
- 调用约定不同（x64 上也有差异）
- 异常处理机制不同
- vtable 布局可能不同

#### 4. 场景4：修改默认参数

```cpp
// v1.0 头文件
void init(int mode = 0);

// v2.0 头文件
void init(int mode = 1);
```

**默认参数是编译期塞进去的**：调用方编译时用的是旧默认值 0，即使换了新头文件，旧 `.o` 里还是 0。这不是 ABI 问题，但效果类似——行为不一致。

***

### 5. 与 extern "C" 的关系

[extern-C与动态库导出](../01-基础概念/15-extern-C与动态库导出.md) 讲过 `extern "C"` 的核心作用：**禁用名称改编**。

```cpp
// C++ 编译：名称被改编
int add(int a, int b);
// → _Z3addii（GCC）或 ?add@@YAHHH@Z（MSVC）

// extern "C"：禁用改编
extern "C" int add(int a, int b);
// → add（所有编译器一样）
```

**extern "C" 解决了 ABI 的哪个维度？** 只解决了名称改编。调用约定、数据布局等其他维度仍然可能不兼容。

但名称改编是最常见的 ABI 不兼容原因，所以 `extern "C"` 是跨编译器/跨语言互操作的基础手段。

***

### 6. Pimpl 惯用法：用不透明指针实现 ABI 稳定

[什么是不透明指针](../01-基础概念/13-什么是不透明指针.md) 讲过不透明指针的原理。Pimpl（Pointer to Implementation）就是不透明指针在 C++ 中的经典应用。

**核心思想**：头文件只暴露指针，实现类藏在 `.cpp` 里，改实现不用改头文件。

```cpp
// ========== widget.h ==========
#pragma once
#include <memory>

class Widget {
public:
    Widget();
    ~Widget();

    void do_something();
    void set_value(int v);
    int get_value() const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};

// ========== widget.cpp ==========
#include "widget.h"
#include <string>

struct Widget::Impl {
    int value = 0;
    std::string name;
    std::vector<int> data;
};

Widget::Widget() : pimpl_(std::make_unique<Impl>()) {}
Widget::~Widget() = default;

void Widget::do_something() { /* 用 pimpl_->xxx */ }
void Widget::set_value(int v) { pimpl_->value = v; }
int Widget::get_value() const { return pimpl_->value; }
```

**为什么 ABI 稳定？**

| 改动 | 不用 Pimpl | 用 Pimpl |
|------|-----------|----------|
| 给 Impl 加成员 | 头文件类大小变了 → ABI 破坏 | 头文件不变 → ABI 稳定 |
| 改成员顺序 | 偏移变了 → ABI 破坏 | 头文件不变 → ABI 稳定 |
| 换实现逻辑 | 可能影响头文件 | 头文件不变 → ABI 稳定 |

**代价**：多一次指针跳转（`pimpl_->xxx`），轻微性能开销。

***

### 7. 保持 ABI 兼容的清单

| 操作 | ABI 安全？ | 说明 |
|------|:---:|------|
| 添加新的非虚函数 | ✅ | 不影响已有符号 |
| 添加新的静态成员 | ✅ | 不影响对象布局 |
| 修改函数实现体 | ✅ | 只改 `.cpp`，不影响二进制接口 |
| 添加/删除成员变量 | ❌ | 改变对象大小和偏移 |
| 修改成员变量顺序 | ❌ | 改变偏移 |
| 添加/删除虚函数 | ❌ | 改变 vtable 布局 |
| 修改虚函数顺序 | ❌ | vtable 索引错位 |
| 修改函数签名 | ❌ | 名称改编改变 |
| 修改默认参数 | ⚠️ | 旧调用方仍用旧值 |
| 修改对齐方式 | ❌ | 数据布局改变 |

***

### 8. 极简总结

**ABI = 编译后的二进制接口约定，API 兼容不代表 ABI 兼容**

| 要点 | 说明 |
|------|------|
| ABI 四维度 | 名称改编、数据布局、调用约定、异常处理 |
| 最常见破坏 | 加成员变量、改虚函数顺序、换编译器 |
| 跨语言/跨编译器 | 用 `extern "C"` 禁用名称改编 |
| 保持 ABI 稳定 | 用 Pimpl（不透明指针）隐藏实现 |
| 一句话 | API 是给人看的，ABI 是给机器看的 |

***

### 相关阅读

- [什么是调用约定Calling-Convention](./10-什么是调用约定Calling-Convention.md)
- [什么是Pimpl惯用法](../04-CPP核心特性/17-什么是Pimpl惯用法.md)
- [什么是名称修饰Name-Mangling](./09-什么是名称修饰Name-Mangling.md)