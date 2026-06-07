# 什么是 NULL 与 nullptr
> 📖 相关章节：[指针](../../01-C语言/06-指针.md)、[核心机制](../../02-CPP/05-核心机制.md)

> **nullptr is a keyword, not a macro. It's type-safe and unambiguous.** — nullptr 是关键字而非宏，类型安全且无歧义。

***

### 1. 精髓速览

**`NULL`** 是 C 语言时代的空指针宏，本质是整数 `0`；**`nullptr`** 是 C++11 引入的空指针关键字，类型为 `nullptr_t`，不会与整数混淆。在 C++ 中，永远用 `nullptr`。

***

### 2. 生活类比

把指针比作**电话号码**：

- **空指针** → 一个"空号"占位符，表示"没有有效号码"
- **`NULL`** → 用数字 `0` 表示空号，但 `0` 也可能是一个真实的号码（整数零）
- **`nullptr`** → 专门设计了一个"空号"标记，不会和任何真实号码混淆

用 `0` 代替空号，接线员可能把"空号"和"分机号 0"搞混；用专门的标记就不会。

### 3. C 语言中的 NULL

```c
#include <stdio.h>
#include <stddef.h>

// NULL 的典型定义
#define NULL 0           // 可能的定义1：整数零
#define NULL ((void*)0)  // 可能的定义2：void* 指针零

int main() {
    int* p = NULL;       // 空指针
    if (p == NULL) {
        printf("p 是空指针\n");
    }
}
```

**问题**：`NULL` 的定义因编译器而异，可能是 `0`，也可能是 `((void*)0)`。

### 4. NULL 在 C++ 中的问题

C++ 不允许 `void*` 隐式转换为其他指针类型，所以 C++ 中 `NULL` 通常定义为 `0`：

```cpp
// C++ 中 NULL 的典型定义
#define NULL 0
```

这导致了**函数重载歧义**：

```cpp
#include <iostream>

void func(int value) {
    std::cout << "整数版本: " << value << std::endl;
}

void func(int* ptr) {
    std::cout << "指针版本" << std::endl;
}

int main() {
    func(0);       // 调用哪个？→ 整数版本
    func(NULL);    // 调用哪个？→ 歧义！NULL 是 0，编译器认为是整数
    func(nullptr); // 调用哪个？→ 指针版本，无歧义
}
```

| 调用 | 结果 | 原因 |
|------|------|------|
| `func(0)` | 整数版本 | `0` 是 `int` |
| `func(NULL)` | **编译错误**（歧义） | `NULL` 是 `int(0)`，匹配整数版本，但意图是指针 |
| `func(nullptr)` | 指针版本 | `nullptr` 是 `nullptr_t`，只能匹配指针 |

### 5. nullptr 的优势

```cpp
#include <iostream>
#include <cstddef>

void process(int value) {
    std::cout << "处理整数: " << value << std::endl;
}

void process(int* ptr) {
    std::cout << "处理指针: " << (ptr ? "非空" : "空") << std::endl;
}

void process(std::nullptr_t null) {
    std::cout << "处理空指针" << std::endl;
}

int main() {
    process(0);        // 整数版本
    process(nullptr);  // nullptr_t 版本（或指针版本）
    int x = 42;
    process(&x);       // 指针版本
}
```

**nullptr 的特性**：

| 特性 | 说明 |
|------|------|
| 类型安全 | `nullptr_t` 类型，不会与整数混淆 |
| 可转换为任意指针类型 | `int* p = nullptr;` 合法 |
| 可转换为 `bool` | `if (nullptr)` 为 `false` |
| 不能转换为整数 | `int n = nullptr;` 错误 |
| 关键字，非宏 | 不受宏定义影响 |

### 6. nullptr_t 类型

```cpp
#include <iostream>
#include <cstddef>

int main() {
    std::nullptr_t np1 = nullptr;   // nullptr_t 类型变量
    std::nullptr_t np2 = nullptr;   // 所有 nullptr 都是同一类型

    int* p1 = np1;       // 可以赋值给任意指针类型
    double* p2 = np2;    // 同上

    // int n = np1;      // 错误！不能转换为整数

    bool b = np1;        // 可以转换为 bool（值为 false）
    std::cout << std::boolalpha << b << std::endl;  // false

    // nullptr_t 的比较
    std::cout << (np1 == np2) << std::endl;  // true
    std::cout << (np1 == nullptr) << std::endl;  // true
}
```

### 7. 空指针的常见陷阱

**陷阱1：解引用空指针**

```cpp
int* p = nullptr;
// int val = *p;   // 未定义行为！崩溃或更糟

// 安全做法：先检查
if (p != nullptr) {
    int val = *p;
}
```

**陷阱2：空指针用于算术运算**

```cpp
int* p = nullptr;
// int offset = p + 1;  // 未定义行为！对空指针做算术运算
```

**陷阱3：delete 后未置空**

```cpp
int* p = new int(42);
delete p;
// p = nullptr;  // 忘记置空 → p 变成悬空指针
// delete p;     // 再次 delete → 未定义行为

// 正确做法
delete p;
p = nullptr;    // 置空，delete nullptr 是安全的（什么都不做）
```

**陷阱4：智能指针与 nullptr**

```cpp
#include <memory>

std::unique_ptr<int> p = nullptr;  // 合法
std::unique_ptr<int> q;            // 默认就是 nullptr

if (p) { /* 非空 */ }              // 检查是否为空
if (p != nullptr) { /* 非空 */ }   // 同上
if (p == nullptr) { /* 空 */ }     // 同上
```

### 8. NULL vs nullptr 完整对比

| 特性 | `NULL` | `nullptr` |
|------|:---:|:---:|
| 语言 | C / C++ | C++11 起 |
| 本质 | 宏（`#define NULL 0`） | 关键字 |
| 类型 | `int` 或 `void*` | `nullptr_t` |
| 类型安全 | ❌ 可能与整数混淆 | ✅ 不会与整数混淆 |
| 函数重载 | 有歧义 | 无歧义 |
| 可移植性 | 定义不一致 | 行为一致 |
| 推荐使用 | C 代码中 | C++ 代码中 |

### 9. 模板中的 nullptr

在模板编程中，`nullptr` 的类型安全尤为重要：

```cpp
#include <iostream>
#include <memory>

// 模板函数：用 nullptr 作为默认参数
template<typename T>
void process(T* ptr = nullptr) {
    if (ptr == nullptr) {
        std::cout << "空指针" << std::endl;
    } else {
        std::cout << "非空指针: " << *ptr << std::endl;
    }
}

// nullptr 作为模板参数推导
template<typename T>
void check(T arg) {
    // 如果传入 NULL（即 0），T 推导为 int
    // 如果传入 nullptr，T 推导为 nullptr_t
}

int main() {
    int x = 42;
    process(&x);       // 非空指针: 42
    process();         // 空指针（默认参数 nullptr）
    process(nullptr);  // 空指针

    check(NULL);      // T = int（危险！）
    check(nullptr);   // T = nullptr_t（正确！）
}
```

### 10. 极简总结

**NULL = C 时代的空指针宏（本质是 0）→ 函数重载歧义 → nullptr = C++11 空指针关键字（类型 nullptr_t）→ 类型安全、无歧义 → C++ 中永远用 nullptr → 空指针不能解引用 → delete 后置 nullptr → 模板中 nullptr 更安全**

***

### 相关阅读

- [什么是指针](../02-内存与底层/01-什么是指针.md)
- [什么是野指针与悬空指针](../02-内存与底层/03-什么是野指针与悬空指针.md)
- [什么是声明和定义](./01-什么是声明和定义.md)