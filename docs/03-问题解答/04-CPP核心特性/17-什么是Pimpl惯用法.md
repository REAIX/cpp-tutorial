# 什么是 Pimpl 惯用法
> 📖 相关章节：[类与对象](../../02-CPP/03-类与对象.md)、[核心机制](../../02-CPP/05-核心机制.md)

> "自动售货机——你只看到按钮和出货口，内部机械结构被外壳包住"——Pimpl 把实现细节藏起来，只暴露接口。

***

### 1. 核心定义

**Pimpl（Pointer to Implementation）** = 将类的私有成员和实现细节移到另一个类中，原类只保留一个指向实现类的指针。外部看到的头文件不再包含任何实现细节。

关键点：**头文件只暴露接口和指针，实现细节全部藏在 `.cpp` 文件里**。

***

### 2. 生活类比

**自动售货机**：

| 概念 | 类比 | 对应 Pimpl |
|------|------|-----------|
| 头文件（接口） | 售货机外壳：按钮、投币口、出货口 | 用户只能看到公开接口 |
| 实现类 | 售货机内部：电机、货架、制冷系统 | 实现细节被隐藏 |
| pimpl 指针 | 外壳和内部之间的连接线 | 通过指针访问实现 |

用户只需要知道"投币→按按钮→取货"，不需要知道内部电机怎么转、货架怎么推。修电机（改实现）不影响外壳（头文件），换电机也不需要用户重新学习使用方法。

***

### 3. 完整代码示例

#### 1. 头文件：widget.h

```cpp
#pragma once
#include <memory>
#include <string>

class Widget {
public:
    Widget();
    ~Widget();

    Widget(Widget&& rhs) noexcept;
    Widget& operator=(Widget&& rhs) noexcept;

    Widget(const Widget& rhs);
    Widget& operator=(const Widget& rhs);

    void do_work();
    void set_name(const std::string& name);
    std::string get_name() const;
    int compute(int x) const;

private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
```

**注意**：头文件中**没有** `#include <vector>`、`#include <mutex>` 等实现细节需要的头文件。

#### 2. 实现文件：widget.cpp

```cpp
#include "widget.h"
#include <vector>
#include <mutex>
#include <algorithm>

struct Widget::Impl {
    std::string name;
    std::vector<int> data;
    std::mutex mtx;
    int cache = 0;

    void do_work_internal() {
        std::lock_guard<std::mutex> lock(mtx);
        data.push_back(cache);
        cache = 0;
    }

    int compute_internal(int x) const {
        return std::accumulate(data.begin(), data.end(), x);
    }
};

Widget::Widget() : pimpl_(std::make_unique<Impl>()) {}

Widget::~Widget() = default;

Widget::Widget(Widget&& rhs) noexcept = default;
Widget& Widget::operator=(Widget&& rhs) noexcept = default;

Widget::Widget(const Widget& rhs)
    : pimpl_(std::make_unique<Impl>(*rhs.pimpl_)) {}

Widget& Widget::operator=(const Widget& rhs) {
    if (this != &rhs) {
        *pimpl_ = *rhs.pimpl_;
    }
    return *this;
}

void Widget::do_work() {
    pimpl_->do_work_internal();
}

void Widget::set_name(const std::string& name) {
    pimpl_->name = name;
}

std::string Widget::get_name() const {
    return pimpl_->name;
}

int Widget::compute(int x) const {
    return pimpl_->compute_internal(x);
}
```

#### 3. 使用方：main.cpp

```cpp
#include "widget.h"
#include <iostream>

int main() {
    Widget w;
    w.set_name("MyWidget");
    w.do_work();
    std::cout << w.get_name() << ": " << w.compute(42) << "\n";

    Widget w2 = w;  // 拷贝构造
    Widget w3 = std::move(w);  // 移动构造

    return 0;
}
```

**关键**：`main.cpp` 只需要 `#include "widget.h"`，不需要知道 `vector`、`mutex` 等任何实现细节。

***

### 4. 编译防火墙效果

#### 1. 不用 Pimpl 时的问题

```cpp
// ========== widget.h（不用 Pimpl）==========
#pragma once
#include <string>
#include <vector>
#include <mutex>

class Widget {
public:
    void do_work();
    void set_name(const std::string& name);
private:
    std::string name_;
    std::vector<int> data_;
    std::mutex mtx_;
    int cache_ = 0;
};
```

**问题**：

| 问题 | 说明 |
|------|------|
| 编译依赖传染 | 所有 `#include "widget.h"` 的文件都间接包含了 `<vector>`、`<mutex>` |
| 修改重编译 | 改 `Widget` 的私有成员（加字段、改类型），所有包含此头文件的 `.cpp` 都要重编译 |
| 头文件膨胀 | 大型项目中，一个头文件改动可能触发数百个文件重编译 |

#### 2. 用 Pimpl 后的效果

```cpp
// ========== widget.h（用 Pimpl）==========
#pragma once
#include <memory>

class Widget {
public:
    Widget();
    ~Widget();
    void do_work();
    void set_name(const std::string& name);
private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
```

| 效果 | 说明 |
|------|------|
| 编译依赖隔离 | 头文件只依赖 `<memory>`，不依赖 `<vector>`、`<mutex>` |
| 修改免重编译 | 改 `Impl` 的成员（加字段、改类型），只需重编译 `widget.cpp` |
| 头文件精简 | 使用方 `#include "widget.h"` 不拉入实现细节的头文件 |

**编译防火墙** = 修改实现类不需要重新编译使用方。这是 Pimpl 最重要的价值。

***

### 5. 与不透明指针的关系

[什么是不透明指针](../01-基础概念/13-什么是不透明指针.md) 讲过不透明指针（Opaque Pointer）的原理。Pimpl 本质上就是 C++ 版的不透明指针。

| 概念 | 不透明指针（C 风格） | Pimpl（C++ 风格） |
|------|---------------------|-------------------|
| 定义方式 | `typedef struct Foo Foo;`（前向声明） | `struct Impl;`（前向声明嵌套类） |
| 指针类型 | `Foo*`（裸指针） | `std::unique_ptr<Impl>`（智能指针） |
| 生命周期 | 手动 `create/destroy` | RAII 自动管理 |
| 类型安全 | 弱（void* 也能做） | 强（编译期检查） |
| 典型场景 | C 库的 ABI 隔离 | C++ 类的编译防火墙 + ABI 稳定 |

**Pimpl = 不透明指针 + RAII + C++ 类型安全**。

与 [什么是ABI兼容性](../03-编译与链接/02-什么是ABI兼容性.md) 中提到的 ABI 稳定性结合：Pimpl 不仅隔离编译依赖，还保证修改 `Impl` 不破坏 ABI。

***

### 6. 优缺点对比

| 维度 | 不用 Pimpl | 用 Pimpl |
|------|:---:|:---:|
| 编译时间 | 改私有成员 → 全部重编译 | 改 Impl → 只重编译一个 `.cpp` |
| ABI 稳定性 | 改成员变量 → ABI 破坏 | 头文件不变 → ABI 稳定 |
| 头文件依赖 | 私有成员的 `#include` 暴露给使用方 | 使用方看不到实现细节的 `#include` |
| 代码可读性 | 一目了然，所有成员在头文件 | 需要跳转到 `.cpp` 才能看到实现 |
| 运行性能 | 直接访问成员，无间接开销 | 每次访问多一次指针跳转（`pimpl_->xxx`） |
| 内存开销 | 对象本身的大小 | 对象 + 指针 + 堆上的 Impl |
| 代码量 | 较少 | 需要写转发函数，代码量增加 |
| 五大函数 | 编译器自动生成 | 需要手动处理（析构必须在 `.cpp` 中 =default） |

***

### 7. 常见陷阱

#### 1. 陷阱1：析构函数在头文件中 =default

```cpp
// widget.h
class Widget {
public:
    ~Widget() = default;  // 错误！此时 Impl 是不完整类型
private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
```

**问题**：`std::unique_ptr` 的析构需要 `Impl` 是完整类型，但头文件中 `Impl` 只是前向声明。

**修复**：把析构函数的 `= default` 放到 `.cpp` 中：

```cpp
// widget.h
~Widget();

// widget.cpp
Widget::~Widget() = default;  // 此时 Impl 已完整定义
```

#### 2. 陷阱2：忘记处理拷贝操作

`std::unique_ptr` 不可拷贝，所以 `Widget` 默认也不可拷贝。如果需要拷贝，必须手动实现：

```cpp
// widget.h
Widget(const Widget& rhs);
Widget& operator=(const Widget& rhs);

// widget.cpp
Widget::Widget(const Widget& rhs)
    : pimpl_(std::make_unique<Impl>(*rhs.pimpl_)) {}

Widget& Widget::operator=(const Widget& rhs) {
    if (this != &rhs) {
        *pimpl_ = *rhs.pimpl_;
    }
    return *this;
}
```

#### 3. 陷阱3：const 成员函数的"伪 const"

```cpp
class Widget {
public:
    void modify() const {  // 看起来是 const
        pimpl_->modify_internal();  // 但修改了 Impl 的成员！
    }
private:
    struct Impl;
    std::unique_ptr<Impl> pimpl_;
};
```

`pimpl_` 本身是 `const`（指针不变），但 `*pimpl_` 不是 const。Pimpl 绕过了 const 保护。

**修复**：提供 `const` 和非 `const` 两个重载：

```cpp
void modify() { pimpl_->modify_internal(); }
int read() const { return pimpl_->read_internal(); }
```

或者使用 `const Impl&` 访问器：

```cpp
Impl& impl() { return *pimpl_; }
const Impl& impl() const { return *pimpl_; }

void modify() { impl().modify_internal(); }
int read() const { return impl().read_internal(); }
```

***

### 8. 极简总结

**Pimpl = 实现细节藏到 Impl 类，头文件只暴露指针和接口 = 编译防火墙 + ABI 稳定**

| 要点 | 说明 |
|------|------|
| 核心做法 | 私有成员移到 `struct Impl`，原类持有 `unique_ptr<Impl>` |
| 最大收益 | 修改实现只重编译一个 `.cpp`，ABI 稳定 |
| 代价 | 多一次指针跳转、代码量增加、需手动处理五大函数 |
| 注意 | 析构函数必须在 `.cpp` 中 `= default`；const 语义被绕过 |
| 一句话 | 头文件只放接口和指针，实现细节全藏 `.cpp` |

***

### 相关阅读

- [什么是不透明指针](../01-基础概念/13-什么是不透明指针.md)
- [什么是ABI兼容性](../03-编译与链接/02-什么是ABI兼容性.md)
- [C++编译时间优化](../03-编译与链接/04-C++编译时间优化.md)