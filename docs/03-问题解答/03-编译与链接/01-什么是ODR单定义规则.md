# 什么是 ODR 单定义规则
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)、[静态库](../../01-C语言/18-静态库.md)、[动态库](../../01-C语言/19-动态库与共享库.md)、[CMake](../../01-C语言/22-CMake构建系统.md)

### 1. 本质洞察

**ODR（One Definition Rule）= 每个实体在整个程序中只能有一个定义。违反 ODR = 未定义行为，编译器不报错但程序可能莫名其妙崩溃。**

***

### 2. 核心定义

ODR 是 C++ 链接规则中最重要的一条，规定了"定义"在整个程序中的唯一性约束。

| 术语 | 含义 |
|------|------|
| 声明（Declaration） | 告诉编译器"有这个东西"，不分配内存 |
| 定义（Definition） | 真正创建实体，分配内存/提供实现 |
| 翻译单元（Translation Unit） | 一个 .cpp 文件加上它 include 的所有头文件，预处理后的结果 |

**声明 vs 定义**：

```cpp
// 声明（可以重复）
extern int g_val;           // 变量声明
void foo(int x);            // 函数声明
class Widget;               // 类前向声明

// 定义（受 ODR 约束）
int g_val = 42;             // 变量定义
void foo(int x) { /*...*/ } // 函数定义
class Widget { /*...*/ };   // 类定义
```

***

### 3. 生活类比

| 概念 | 类比 |
|------|------|
| ODR | 法律只能有一部宪法，不能每个省各写一部 |
| 声明 | 公告栏上贴通知"本市有一座图书馆"（可以贴很多次） |
| 定义 | 真正建一座图书馆（只能建一座） |
| 违反 ODR | 两个省各建了一座"国家图书馆"，都声称自己是正宗的 |
| inline | 多个城市可以建连锁分店，但品牌和菜单必须完全一致 |

**为什么 ODR 重要**：链接器需要把多个翻译单元拼成最终程序。如果同一个东西有两个不同的定义，链接器不知道该用哪个，或者选了一个错误的，结果就是未定义行为。

***

### 4. ODR 的三层规则

**第一层：每个翻译单元中，每个实体只能定义一次**

```cpp
// file.cpp
int g_val = 42;       // ✅ 定义一次
// int g_val = 100;   // ❌ 同一翻译单元内重复定义，编译错误

void foo() {}         // ✅ 定义一次
// void foo() {}      // ❌ 重复定义，编译错误

class Widget {        // ✅ 定义一次
    int x;
};
// class Widget {     // ❌ 重复定义，编译错误
//     double y;
// };
```

**第二层：如果实体被多个翻译单元使用，定义必须完全相同**

```cpp
// === 头文件 common.h ===
struct Config {
    int version;
    int port;
};

// === file1.cpp ===
#include "common.h"
// Config 的定义：version + port

// === file2.cpp ===
#include "common.h"
// Config 的定义：version + port
// ✅ 两个翻译单元中 Config 的定义完全相同，ODR 满足

// === 如果有人偷偷改了 ===
// file3.cpp（没有 include common.h，自己手写了一个）
struct Config {
    int version;
    // 少了 port！
    double timeout;  // 多了别的字段！
};
// ❌ ODR 违反！file3 中的 Config 和其他文件不同
// 未定义行为：程序可能崩溃、数据错乱、或者看起来正常
```

**第三层：某些实体允许多个定义（有条件）**

| 实体类型 | 可以多定义吗 | 条件 |
|----------|:---:|------|
| 类定义 | ✅ | 每个翻译单元中的定义必须完全相同 |
| inline 函数 | ✅ | 每个翻译单元中的定义必须完全相同 |
| constexpr 函数 | ✅ | 每个翻译单元中的定义必须完全相同 |
| 模板 | ✅ | 每个翻译单元中的定义必须完全相同 |
| inline 变量（C++17） | ✅ | 每个翻译单元中的定义必须完全相同 |
| 普通函数 | ❌ | 只能在一个翻译单元中定义 |
| 普通全局变量 | ❌ | 只能在一个翻译单元中定义 |
| static 全局变量 | ⚠️ | 每个翻译单元各有一份独立副本（不是同一个实体） |

***

### 5. 违反 ODR 的各种情况和后果

**情况1：多个 .cpp 中定义同一个普通函数**

```cpp
// === utils.h ===
int add(int a, int b);  // 声明 ✅

// === utils.cpp ===
int add(int a, int b) { return a + b; }  // 定义 ✅

// === other.cpp ===
int add(int a, int b) { return a + b + 1; }  // ❌ 重复定义！
// 链接错误：multiple definition of `add(int, int)'
```

**情况2：头文件中定义普通变量**

```cpp
// === config.h ===
int g_port = 8080;  // ❌ 每个 include 这个头文件的 .cpp 都会有一份定义

// === file1.cpp ===
#include "config.h"  // g_port 定义一次

// === file2.cpp ===
#include "config.h"  // g_port 又定义一次
// 链接错误：multiple definition of `g_port'

// ✅ 正确写法
// config.h
extern int g_port;   // 声明
// config.cpp
int g_port = 8080;  // 定义（只在一个 .cpp 中）

// ✅ C++17 正确写法
// config.h
inline int g_port = 8080;  // inline 变量，允许多定义
```

**情况3：头文件中定义普通函数**

```cpp
// === math.h ===
int square(int x) { return x * x; }  // ❌ 每个 include 都会有一份定义

// ✅ 正确写法1：加 inline
inline int square(int x) { return x * x; }

// ✅ 正确写法2：声明放头文件，定义放 .cpp
// math.h
int square(int x);
// math.cpp
int square(int x) { return x * x; }
```

**情况4：头文件中类定义不同（最隐蔽的 ODR 违反）**

```cpp
// === 假设两个 .cpp include 了不同版本的同一个头文件 ===

// file1.cpp 看到的 Data.h
struct Data {
    int id;
    double value;
};

// file2.cpp 看到的 Data.h（有人改了但没重新编译 file1）
struct Data {
    int id;
    float value;   // double → float！
};

// ❌ ODR 违反！但编译器不会报错！
// 后果：file1 和 file2 对 Data 的内存布局理解不同
// file1 认为 sizeof(Data) = 16（int 4 + padding 4 + double 8）
// file2 认为 sizeof(Data) = 8 （int 4 + float 4）
// 访问同一个 Data 对象时，数据完全错乱
```

**情况5：宏导致 ODR 违反**

```cpp
// === debug.h ===
#ifdef USE_BIG_BUFFER
constexpr int BUF_SIZE = 4096;
#else
constexpr int BUF_SIZE = 1024;
#endif

struct Processor {
    char buf[BUF_SIZE];
};

// file1.cpp
#define USE_BIG_BUFFER
#include "debug.h"  // Processor::buf 是 4096

// file2.cpp
// 没定义 USE_BIG_BUFFER
#include "debug.h"  // Processor::buf 是 1024

// ❌ ODR 违反！两个翻译单元中 Processor 的定义不同
// 这就是为什么宏是危险的（参见 FAQ 29）
```

***

### 6. ODR 与 inline 关键字的关系

> 延伸阅读：[inline关键字的真实含义](../01-基础概念/19-inline关键字的真实含义.md)

inline 的核心作用就是"允许 ODR 例外"：让同一个定义出现在多个翻译单元中。

```cpp
// === 没有 inline：ODR 违反 ===
// header.h
void helper() { /* ... */ }
// 多个 .cpp include → 多个定义 → 链接错误

// === 有 inline：ODR 允许 ===
// header.h
inline void helper() { /* ... */ }
// 多个 .cpp include → 多个定义 → 链接器选一个，OK
// 前提：所有翻译单元中的定义必须完全相同！
```

**自动 inline 的实体（天然允许多定义）**：

| 实体 | 为什么允许多定义 |
|------|------|
| 类体内定义的成员函数 | 自动 inline |
| 模板函数/模板类 | 实例化时需要看到完整定义 |
| constexpr 函数 | 自动 inline（C++11起） |
| inline 变量（C++17） | 显式标记允许多定义 |

```cpp
// 类体内定义的成员函数 → 自动 inline → 放头文件没问题
class Widget {
    int value_;
public:
    int getValue() const { return value_; }  // 自动 inline
};

// 类体外定义的成员函数 → 不自动 inline → 放头文件需要加 inline
class Widget {
    int value_;
public:
    int getValue() const;
};

inline int Widget::getValue() const { return value_; }  // 必须加 inline
```

***

### 7. ODR 与 undefined reference 的关系

> 延伸阅读：[undefined-reference排查](./00-undefined-reference排查.md)

ODR 和 undefined reference 是一个硬币的两面：

| 问题 | 原因 | 错误类型 |
|------|------|---------|
| ODR 违反 | 定义重复或定义不一致 | 可能不报错（UB），或链接错误 |
| undefined reference | 有声明但没定义 | 链接错误 |

**ODR 违反导致 undefined reference 的场景**：

```cpp
// === 内联函数 ODR 违反 ===
// version1.h
inline int getVersion() { return 1; }

// version2.h（不同版本）
inline int getVersion() { return 2; }

// file1.cpp include version1.h
// file2.cpp include version2.h
// 链接器可能选 getVersion() 返回 1 或 2 的版本
// 更糟的是：file1 调用 getVersion() 可能得到 1 或 2，取决于链接器心情

// === 模板 ODR 违反 ===
// file1.cpp 中的 std::hash<MyType> 特化
namespace std {
template<>
struct hash<MyType> {
    size_t operator()(const MyType& t) const { return t.id; }
};
}

// file2.cpp 中的 std::hash<MyType> 不同特化
namespace std {
template<>
struct hash<MyType> {
    size_t operator()(const MyType& t) const { return t.id * 31; }
};
}
// ❌ ODR 违反！同一个模板特化有两个不同定义
// unordered_set 和 unordered_map 在不同翻译单元中行为不同
```

**避免 ODR 违反的最佳实践**：

| 做法 | 说明 |
|------|------|
| 头文件只放声明 | 定义放 .cpp，头文件只放声明 |
| 头文件中的函数加 inline | 类体外定义的成员函数必须加 inline |
| 头文件中的变量用 inline 或 extern | C++17 用 inline 变量，否则用 extern + .cpp 定义 |
| 模板和 inline 函数定义放头文件 | 确保所有翻译单元看到相同定义 |
| 避免在头文件中使用宏控制定义 | 宏在不同翻译单元中可能有不同值 |
| 用 #pragma once 或头文件守卫 | 防止同一翻译单元中重复 include |

***

### 8. ODR 常见陷阱

**陷阱1：匿名命名空间不解决 ODR**

```cpp
// header.h
namespace {
    int g_counter = 0;  // 每个 .cpp 有独立的 g_counter
}

// file1.cpp
#include "header.h"  // g_counter = 0（file1 的副本）

// file2.cpp
#include "header.h"  // g_counter = 0（file2 的副本）

// file1.cpp 中 g_counter++ 不影响 file2.cpp 中的 g_counter
// 这不是 ODR 违反，但通常不是你想要的行为
// 如果想要共享变量，用 extern
```

**陷阱2：static 全局变量在头文件中**

```cpp
// header.h
static int g_count = 0;  // 每个 include 的 .cpp 各有一份独立副本

// file1.cpp
#include "header.h"
g_count++;  // file1 的 g_count 变成 1

// file2.cpp
#include "header.h"
g_count++;  // file2 的 g_count 变成 1（不是 2！）
// 和匿名命名空间效果一样，通常不是你想要的
```

**陷阱3：默认参数在头文件中重复且不一致**

```cpp
// header1.h
void foo(int x = 10);

// header2.h
void foo(int x = 20);

// file1.cpp
#include "header1.h"  // foo 的默认参数是 10

// file2.cpp
#include "header2.h"  // foo 的默认参数是 20

// ❌ ODR 违反！同一个函数的默认参数在不同翻译单元中不同
// 默认参数应该在声明中只写一次
```

**陷阱4：不同编译选项导致 ODR 违反**

```cpp
// 假设部分 .cpp 用 -DDEBUG 编译，部分没有

// header.h
struct Config {
    int size;
#ifdef DEBUG
    int debug_id;  // DEBUG 模式多一个字段
#endif
};

// file1.cpp（用 -DDEBUG 编译）
// Config 有 size + debug_id

// file2.cpp（不用 -DDEBUG 编译）
// Config 只有 size

// ❌ ODR 违反！两个翻译单元中 Config 的定义不同
// 解决：所有 .cpp 必须用相同的编译选项
```

***

### 9. 完整示例：ODR 正确与错误

```cpp
// === 正确做法 ===

// === widget.h ===
#pragma once
#include <string>

class Widget {
    std::string name_;
    int value_;
public:
    Widget(const std::string& name, int value);

    int getValue() const { return value_; }  // 类内定义，自动 inline

    void setValue(int v);
};

inline void Widget::setValue(int v) {  // 类外定义，必须加 inline
    value_ = v;
}

inline int globalId() {  // 头文件中的自由函数，必须加 inline
    static int id = 0;
    return ++id;
}

// === widget.cpp ===
#include "widget.h"

Widget::Widget(const std::string& name, int value)
    : name_(name), value_(value) {}  // 定义只在一个 .cpp 中

// === main.cpp ===
#include "widget.h"
#include <iostream>

int main() {
    Widget w("test", 42);
    std::cout << w.getValue() << "\n";  // 42
    w.setValue(100);
    std::cout << w.getValue() << "\n";  // 100
    std::cout << globalId() << "\n";    // 1
    return 0;
}
```

***

### 10. 极简总结

**ODR = 每个实体只能定义一次 | 同一翻译单元内不能重复定义 | 多翻译单元中定义必须完全相同 | inline/模板/类定义/constexpr 允许多定义但必须一致 | 违反 ODR 是 UB 不一定报错 | 头文件只放声明和 inline 定义 | 避免宏控制定义内容 | 所有 .cpp 用相同编译选项**

***

### 相关阅读

- [undefined-reference排查](./00-undefined-reference排查.md)
- [什么是符号表Symbol-Table](./11-什么是符号表Symbol-Table.md)