# C/C++ 关键字、保留字与常用词汇速查

> 编程语言的"词汇表"——从关键字到命名惯例的完整参考

---

> **学习一门编程语言，就像学习一门外语——先背单词，再学语法，最后写文章。**
> （Learning a programming language is like learning a foreign language: first vocabulary, then grammar, finally writing.）

---

> 💡 **通俗理解**
>
> 学英语要先背单词，学 C/C++ 也一样。这份文档就是你的"编程词典"——把 C/C++ 中所有关键字、常用标识符、命名惯例一网打尽，方便随时查阅。

---

## 1. C 语言关键字（C89/C90：32 个）

C89 标准定义了 32 个关键字，它们是语言的"骨架"，**不能用作变量名、函数名或其他标识符**。

### 1.1 数据类型关键字（12 个）

| 关键字 | 含义 | 示例 | 说明 |
|--------|------|------|------|
| `char` | 字符型 | `char c = 'A';` | 占 1 字节 |
| `short` | 短整型 | `short s = 32767;` | 通常 2 字节 |
| `int` | 整型 | `int n = 42;` | 通常 4 字节 |
| `long` | 长整型 | `long l = 100000L;` | 4 或 8 字节 |
| `float` | 单精度浮点 | `float f = 3.14f;` | 4 字节 |
| `double` | 双精度浮点 | `double d = 3.14;` | 8 字节 |
| `unsigned` | 无符号 | `unsigned int n = 42U;` | 只能表示非负数 |
| `signed` | 有符号 | `signed int n = -1;` | 可正可负（默认） |
| `void` | 无类型 | `void func(void);` | 函数无返回值/无参数 |
| `enum` | 枚举 | `enum Color { RED, GREEN };` | 整数常量集合 |
| `struct` | 结构体 | `struct Point { int x; int y; };` | 多个成员的组合 |
| `union` | 联合体 | `union Data { int i; float f; };` | 成员共享内存 |

### 1.2 控制语句关键字（12 个）

| 关键字 | 含义 | 示例 | 说明 |
|--------|------|------|------|
| `if` | 条件判断 | `if (x > 0) { ... }` | 满足条件执行 |
| `else` | 否则 | `else { ... }` | if 的对立分支 |
| `switch` | 多分支选择 | `switch (n) { case 1: ... }` | 整数/枚举多分支 |
| `case` | 分支标签 | `case 1: printf("one");` | switch 中的分支 |
| `default` | 默认分支 | `default: break;` | switch 的兜底 |
| `for` | 循环 | `for (int i=0; i<10; i++)` | 计数型循环 |
| `while` | 当循环 | `while (x > 0) { ... }` | 先判断后执行 |
| `do` | 执行循环 | `do { ... } while (x>0);` | 先执行后判断 |
| `break` | 跳出 | `break;` | 跳出循环/switch |
| `continue` | 继续 | `continue;` | 跳过本次迭代 |
| `goto` | 跳转 | `goto cleanup;` | 无条件跳转（慎用） |
| `return` | 返回 | `return 0;` | 函数返回 |

### 1.3 存储类与类型修饰关键字（6 个）

| 关键字 | 含义 | 示例 | 说明 |
|--------|------|------|------|
| `auto` | 自动存储 | `auto int x = 0;` | 局部变量默认（C 中几乎不用） |
| `extern` | 外部声明 | `extern int g_count;` | 声明在其他文件中定义的变量 |
| `register` | 寄存器存储 | `register int i;` | 建议编译器放入寄存器（现代编译器忽略） |
| `static` | 静态存储 | `static int count = 0;` | 改变作用域和生命周期 |
| `const` | 常量 | `const int MAX = 100;` | 不可修改 |
| `volatile` | 易变的 | `volatile int *p;` | 禁止编译器优化 |

### 1.4 其他关键字（2 个）

| 关键字 | 含义 | 示例 | 说明 |
|--------|------|------|------|
| `typedef` | 类型别名 | `typedef int BOOL;` | 给类型起新名字 |
| `sizeof` | 大小运算 | `sizeof(int)` | 编译期求值，返回字节数 |

---

## 2. C99 新增关键字（5 个）

| 关键字 | 含义 | 示例 | 说明 |
|--------|------|------|------|
| `_Bool` | 布尔类型 | `_Bool flag = 1;` | C99 原生布尔（通常用 `bool` 宏） |
| `_Complex` | 复数类型 | `_Complex double z;` | 复数运算 |
| `_Imaginary` | 虚数类型 | `_Imaginary double im;` | 纯虚数 |
| `inline` | 内联函数 | `inline int add(int a, int b)` | 建议内联展开 |
| `restrict` | 限制指针 | `void func(int *restrict p)` | 告知编译器指针不重叠 |

> 💡 C99 还引入了头文件 `<stdbool.h>`，其中定义了 `bool`、`true`、`false` 宏，使 `_Bool` 更易用。

---

## 3. C11 新增关键字（7 个）

| 关键字 | 含义 | 示例 | 说明 |
|--------|------|------|------|
| `_Alignas` | 对齐指定 | `_Alignas(16) int x;` | 指定变量对齐方式 |
| `_Alignof` | 对齐查询 | `_Alignof(int)` | 查询类型的对齐要求 |
| `_Atomic` | 原子类型 | `_Atomic int counter;` | 原子操作变量 |
| `_Generic` | 泛型选择 | `_Generic(x, int: 1, default: 0)` | 编译期类型选择 |
| `_Noreturn` | 不返回 | `_Noreturn void fatal(void)` | 函数不会返回 |
| `_Static_assert` | 静态断言 | `_Static_assert(sizeof(int)==4, "...")` | 编译期断言 |
| `_Thread_local` | 线程局部 | `_Thread_local int tls_var;` | 线程私有变量 |

> 💡 C11 头文件提供了易用的宏别名：`<stdalign.h>`（`alignas`/`alignof`）、`<stdatomic.h>`（`atomic_*`）、`<stdnoreturn.h>`（`noreturn`）、`<threads.h>`（`thread_local`）。

---

## 4. C23 新增/变更关键字

| 关键字 | 含义 | 说明 |
|--------|------|------|
| `bool` | 布尔类型 | 终于成为真正的关键字（不再是宏） |
| `true` | 布尔真 | 终于成为关键字 |
| `false` | 布尔假 | 终于成为关键字 |
| `nullptr` | 空指针 | 类型安全的空指针常量 |
| `constexpr` | 常量表达式 | 编译期求值 |
| `typeof` | 类型推导 | 获取表达式的类型 |
| `thread_local` | 线程局部 | 从宏升级为关键字 |
| `alignas` / `alignof` | 对齐 | 从宏升级为关键字 |
| `noreturn` | 不返回 | 从宏升级为关键字 |

---

## 5. C++ 关键字（C++20：80 个）

C++ 在 C 语言基础上大幅扩展了关键字。以下按功能分类列出 **C++ 独有**的关键字（不含与 C 重复的）。

### 5.1 面向对象关键字

| 关键字 | 含义 | 示例 | 说明 |
|--------|------|------|------|
| `class` | 类 | `class Foo { ... };` | C++ 核心抽象 |
| `public` | 公有 | `public: int x;` | 任何地方可访问 |
| `private` | 私有 | `private: int y;` | 仅类内可访问 |
| `protected` | 保护 | `protected: int z;` | 类内及派生类可访问 |
| `virtual` | 虚函数 | `virtual void draw();` | 支持多态 |
| `friend` | 友元 | `friend class Bar;` | 授予外部访问权限 |
| `this` | 当前对象 | `return this->x;` | 指向当前对象的指针 |
| `operator` | 运算符重载 | `int operator+(const T& rhs);` | 自定义运算符行为 |
| `template` | 模板 | `template<typename T>` | 泛型编程基础 |
| `typename` | 类型名 | `typename T::value_type` | 模板中指明类型 |
| `namespace` | 命名空间 | `namespace mylib { ... }` | 防止名字冲突 |
| `using` | 使用/别名 | `using namespace std;` | 引入命名空间/类型别名 |
| `explicit` | 显式 | `explicit Foo(int x);` | 禁止隐式转换 |
| `mutable` | 可变 | `mutable int cache;` | const 对象中仍可修改 |
| `override` | 重写（C++11） | `void draw() override;` | 确保正确重写虚函数 |
| `final` | 最终（C++11） | `class Base final { };` | 禁止继承/重写 |

### 5.2 类型与转换关键字

| 关键字 | 含义 | 示例 | 说明 |
|--------|------|------|------|
| `bool` | 布尔 | `bool flag = true;` | C++ 原生布尔 |
| `true` | 真 | `bool b = true;` | 布尔字面量 |
| `false` | 假 | `bool b = false;` | 布尔字面量 |
| `wchar_t` | 宽字符 | `wchar_t wc = L'中';` | 宽字符类型 |
| `static_cast` | 静态转换 | `static_cast<int>(d)` | 编译期安全转换 |
| `dynamic_cast` | 动态转换 | `dynamic_cast<Derived*>(base)` | 运行时多态转换 |
| `const_cast` | 常量转换 | `const_cast<int&>(cr)` | 去除/添加 const |
| `reinterpret_cast` | 重解释转换 | `reinterpret_cast<int*>(p)` | 底层位重新解释 |
| `decltype` | 类型推导（C++11） | `decltype(x) y = x;` | 获取表达式类型 |
| `auto` | 自动推导（C++11） | `auto x = 42;` | 编译器推导类型 |

### 5.3 异常与资源管理关键字

| 关键字 | 含义 | 示例 | 说明 |
|--------|------|------|------|
| `try` | 尝试 | `try { ... }` | 异常捕获块 |
| `catch` | 捕获 | `catch (const std::exception& e)` | 异常处理 |
| `throw` | 抛出 | `throw std::runtime_error("err");` | 抛出异常 |
| `noexcept` | 不抛异常（C++11） | `void f() noexcept;` | 承诺不抛异常 |

### 5.4 C++11 新增关键字

| 关键字 | 含义 | 示例 |
|--------|------|------|
| `nullptr` | 空指针 | `int* p = nullptr;` |
| `static_assert` | 静态断言 | `static_assert(sizeof(int)==4);` |
| `constexpr` | 常量表达式 | `constexpr int fib(int n);` |
| `alignas` | 对齐指定 | `alignas(16) int x;` |
| `alignof` | 对齐查询 | `alignof(int)` |
| `char16_t` | UTF-16 字符 | `char16_t c = u'中';` |
| `char32_t` | UTF-32 字符 | `char32_t c = U'中';` |
| `enum class` | 限定作用域枚举 | `enum class Color { Red };` |
| `delete` | 删除函数 | `Foo(const Foo&) = delete;` |
| `default` | 默认函数 | `Foo() = default;` |

### 5.5 C++14 新增关键字

| 关键字/标识符 | 含义 | 说明 |
|--------------|------|------|
| （无新增关键字） | — | C++14 主要是特性增强，未引入新关键字 |

> 💡 C++14 增强了 `constexpr`（允许循环和局部变量）和 `auto`（支持返回类型推导），但未新增关键字。

### 5.6 C++17 新增关键字

| 关键字 | 含义 | 示例 |
|--------|------|------|
| `inline` | 内联变量 | `inline const int N = 42;`（变量版，函数版 C++ 就有） |
| `struct` | 结构化绑定 | `auto [x, y] = getPoint();`（非新关键字，新用法） |

### 5.7 C++20 新增关键字

| 关键字 | 含义 | 示例 |
|--------|------|------|
| `concept` | 概念 | `concept Addable = requires(T a, T b) { a + b; };` |
| `requires` | 约束 | `template<typename T> requires Addable<T>` |
| `co_await` | 协程等待 | `co_await promise;` |
| `co_return` | 协程返回 | `co_return 42;` |
| `co_yield` | 协程产出 | `co_yield value;` |
| `consteval` | 立即函数 | `consteval int fib(int n);` |
| `constinit` | 常量初始化 | `constinit static int x = 42;` |
| `char8_t` | UTF-8 字符 | `char8_t c = u8'A';` |

### 5.8 C++23 新增关键字

| 关键字 | 含义 | 示例 |
|--------|------|------|
| `import` | 模块导入 | `import std;` |
| `module` | 模块声明 | `module mylib;` |

---

## 6. 预处理指令（C/C++ 通用）

预处理指令不是关键字，但它们是 C/C++ 程序的重要组成部分，以 `#` 开头。

### 6.1 文件包含

| 指令 | 含义 | 示例 |
|------|------|------|
| `#include` | 包含文件 | `#include <stdio.h>` / `#include "mylib.h"` |

### 6.2 宏定义

| 指令 | 含义 | 示例 |
|------|------|------|
| `#define` | 定义宏 | `#define MAX 100` / `#define SQUARE(x) ((x)*(x))` |
| `#undef` | 取消宏 | `#undef MAX` |

### 6.3 条件编译

| 指令 | 含义 | 示例 |
|------|------|------|
| `#if` | 如果 | `#if defined(WIN32)` |
| `#ifdef` | 如果定义了 | `#ifdef DEBUG` |
| `#ifndef` | 如果没定义 | `#ifndef MYHEADER_H` |
| `#else` | 否则 | `#else` |
| `#elif` | 否则如果 | `#elif defined(LINUX)` |
| `#endif` | 结束条件 | `#endif` |

### 6.4 其他预处理指令

| 指令 | 含义 | 示例 |
|------|------|------|
| `#pragma` | 编译器指令 | `#pragma once` / `#pragma pack(4)` |
| `#line` | 行号控制 | `#line 100 "fake.c"` |
| `#error` | 编译错误 | `#error "Unsupported platform"` |
| `#warning` | 编译警告 | `#warning "Deprecated feature"` |

### 6.5 预定义宏

| 宏 | 含义 | 示例值 |
|----|------|--------|
| `__FILE__` | 当前文件名 | `"main.cpp"` |
| `__LINE__` | 当前行号 | `42` |
| `__DATE__` | 编译日期 | `"Jun  3 2026"` |
| `__TIME__` | 编译时间 | `"14:30:00"` |
| `__func__` | 当前函数名 | `"main"` |
| `__cplusplus` | C++ 标准版本 | `201703L`（C++17） |
| `__STDC__` | 是否标准 C | `1` |
| `__STDC_VERSION__` | C 标准版本 | `201112L`（C11） |

---

## 7. 运算符速查

### 7.1 C/C++ 运算符优先级（从高到低）

| 优先级 | 运算符 | 含义 | 结合性 |
|:-----:|--------|------|:------:|
| 1 | `::` | 作用域解析（C++） | 左→右 |
| 2 | `++` `--` | 后置自增/自减 | 左→右 |
| | `()` | 函数调用 | |
| | `[]` | 下标 | |
| | `.` `->` | 成员访问 | |
| | `typeid` | 类型识别（C++） | |
| | `dynamic_cast` `static_cast` `const_cast` `reinterpret_cast` | 类型转换（C++） | |
| 3 | `++` `--` | 前置自增/自减 | 右→左 |
| | `+` `-` | 正/负号 | |
| | `!` `~` | 逻辑非/按位取反 | |
| | `*` `&` | 解引用/取地址 | |
| | `sizeof` | 大小 | |
| | `new` `delete` | 动态内存（C++） | |
| | `(type)` | C 风格类型转换 | |
| 4 | `.*` `->*` | 成员指针（C++） | 左→右 |
| 5 | `*` `/` `%` | 乘/除/取模 | 左→右 |
| 6 | `+` `-` | 加/减 | 左→右 |
| 7 | `<<` `>>` | 左移/右移 | 左→右 |
| 8 | `<` `<=` `>` `>=` | 关系比较 | 左→右 |
| 9 | `==` `!=` | 相等比较 | 左→右 |
| 10 | `&` | 按位与 | 左→右 |
| 11 | `^` | 按位异或 | 左→右 |
| 12 | `|` | 按位或 | 左→右 |
| 13 | `&&` | 逻辑与 | 左→右 |
| 14 | `||` | 逻辑或 | 左→右 |
| 15 | `?:` | 三目条件 | 右→左 |
| 16 | `=` `+=` `-=` `*=` `/=` `%=` `<<=` `>>=` `&=` `^=` `|=` | 赋值 | 右→左 |
| 17 | `throw` | 抛出异常（C++） | 右→左 |
| 18 | `,` | 逗号 | 左→右 |

### 7.2 C++ 新增运算符

| 运算符 | 含义 | 版本 | 示例 |
|--------|------|------|------|
| `::` | 作用域解析 | C++98 | `std::cout` |
| `.*` `->*` | 成员指针 | C++98 | `obj.*pmf` |
| `<<` `>>` | 流插入/提取 | C++98 | `cout << x` |
| `delete` `new` | 内存管理 | C++98 | `new int(42)` |
| `typeid` | 类型识别 | C++98 | `typeid(x).name()` |
| `<=>` | 三路比较 | C++20 | `auto cmp = a <=> b;` |

---

## 8. 标准库常用标识符

### 8.1 C 标准库常用头文件与函数

| 头文件 | 常用函数/宏 | 用途 |
|--------|------------|------|
| `<stdio.h>` | `printf` `scanf` `fopen` `fclose` `fgets` `fprintf` `fread` `fwrite` | 输入输出 |
| `<stdlib.h>` | `malloc` `free` `calloc` `realloc` `atoi` `atof` `exit` `abort` `rand` `srand` | 通用工具 |
| `<string.h>` | `strlen` `strcpy` `strncpy` `strcmp` `strncmp` `strcat` `strncat` `memcpy` `memset` `memmove` | 字符串/内存 |
| `<math.h>` | `sin` `cos` `tan` `sqrt` `pow` `abs` `fabs` `ceil` `floor` `log` `exp` | 数学 |
| `<ctype.h>` | `isalpha` `isdigit` `isalnum` `isspace` `isupper` `islower` `toupper` `tolower` | 字符判断 |
| `<assert.h>` | `assert` | 断言 |
| `<errno.h>` | `errno` `EINVAL` `ENOENT` `ENOMEM` | 错误码 |
| `<time.h>` | `time` `clock` `difftime` `strftime` `localtime` `gmtime` `mktime` | 时间 |
| `<signal.h>` | `signal` `raise` `SIGINT` `SIGTERM` `SIGKILL` | 信号 |
| `<setjmp.h>` | `setjmp` `longjmp` | 非局部跳转 |
| `<stddef.h>` | `NULL` `size_t` `ptrdiff_t` `offsetof` | 基本类型 |
| `<stdbool.h>` | `bool` `true` `false` `TRUE` `FALSE` | 布尔（C99） |
| `<stdint.h>` | `int8_t` `int16_t` `int32_t` `int64_t` `uint8_t` `uintptr_t` | 固定宽度整数（C99） |
| `<inttypes.h>` | `PRId64` `SCNd32` | 格式化宏（C99） |

### 8.2 C++ 标准库常用头文件与标识符

| 头文件 | 常用标识符 | 用途 |
|--------|-----------|------|
| `<iostream>` | `std::cout` `std::cin` `std::cerr` `std::endl` `std::flush` | 流式 I/O |
| `<string>` | `std::string` `std::stoi` `std::stod` `std::to_string` `std::getline` | 字符串 |
| `<vector>` | `std::vector` `push_back` `emplace_back` `size` `at` `begin` `end` | 动态数组 |
| `<map>` | `std::map` `std::multimap` `insert` `find` `erase` `count` | 有序映射 |
| `<unordered_map>` | `std::unordered_map` `std::unordered_multimap` | 哈希映射 |
| `<set>` | `std::set` `std::multiset` | 有序集合 |
| `<algorithm>` | `std::sort` `std::find` `std::find_if` `std::transform` `std::accumulate` `std::count` `std::remove_if` `std::unique` `std::reverse` `std::min` `std::max` | 算法 |
| `<memory>` | `std::unique_ptr` `std::shared_ptr` `std::weak_ptr` `std::make_unique` `std::make_shared` | 智能指针 |
| `<functional>` | `std::function` `std::bind` `std::placeholders` | 函数对象 |
| `<thread>` | `std::thread` `std::jthread` `std::this_thread::sleep_for` | 线程 |
| `<mutex>` | `std::mutex` `std::lock_guard` `std::unique_lock` `std::scoped_lock` | 互斥量 |
| `<atomic>` | `std::atomic` `std::atomic_flag` `load` `store` `compare_exchange_weak` | 原子操作 |
| `<optional>` | `std::optional` `std::nullopt` `has_value` `value` | 可选值（C++17） |
| `<variant>` | `std::variant` `std::visit` `std::holds_alternative` | 类型安全联合体（C++17） |
| `<any>` | `std::any` `std::any_cast` | 任意类型容器（C++17） |
| `<filesystem>` | `std::path` `std::directory_entry` `std::directory_iterator` | 文件系统（C++17） |
| `<format>` | `std::format` `std::format_to` | 格式化输出（C++20） |
| `<concepts>` | `std::integral` `std::floating_point` `std::same_as` `std::derived_from` | 概念（C++20） |
| `<ranges>` | `std::views::filter` `std::views::transform` `std::views::take` | 范围（C++20） |
| `<span>` | `std::span` | 非拥有视图（C++20） |
| `<expected>` | `std::expected` `std::unexpected` | 错误处理（C++23） |
| `<print>` | `std::print` `std::println` | 格式化打印（C++23） |

---

## 9. 命名惯例与常用缩写

### 9.1 变量命名风格

| 风格 | 格式 | 示例 | 常见场景 |
|------|------|------|---------|
| **snake_case** | 小写+下划线 | `user_name` `item_count` | C 语言、Linux 内核、Python |
| **camelCase** | 小写驼峰 | `userName` `itemCount` | Java、Qt、C# |
| **PascalCase** | 大写驼峰 | `UserName` `ItemCount` | C++ 类名、C#、TypeScript |
| **UPPER_CASE** | 全大写+下划线 | `MAX_SIZE` `PI` | 宏、常量 |
| **m_ 前缀** | m_+驼峰 | `m_name` `m_count` | C++ 成员变量（旧风格） |
| **_ 后缀** | 驼峰+_ | `name_` `count_` | C++ 成员变量（Google 风格） |

### 9.2 常见缩写词典

| 缩写 | 全称 | 含义 |
|------|------|------|
| `ptr` | pointer | 指针 |
| `ref` | reference | 引用 |
| `idx` | index | 索引 |
| `len` | length | 长度 |
| `cnt` / `num` | count / number | 计数/数量 |
| `buf` | buffer | 缓冲区 |
| `fd` | file descriptor | 文件描述符 |
| `fn` / `func` | function | 函数 |
| `ret` | return / return value | 返回值 |
| `val` | value | 值 |
| `arg` / `param` | argument / parameter | 参数 |
| `err` / `errno` | error / error number | 错误 |
| `tmp` / `temp` | temporary | 临时 |
| `prev` / `curr` / `next` | previous / current / next | 前/当前/后 |
| `src` / `dst` | source / destination | 源/目标 |
| `lhs` / `rhs` | left-hand side / right-hand side | 左操作数/右操作数 |
| `iter` | iterator | 迭代器 |
| `pred` | predicate | 谓词 |
| `alloc` / `dealloc` | allocate / deallocate | 分配/释放 |
| `init` | initialize | 初始化 |
| `dest` / `dtor` | destructor | 析构 |
| `ctor` | constructor | 构造 |
| `impl` | implementation | 实现 |
| `dep` | dependency | 依赖 |
| `sync` / `async` | synchronous / asynchronous | 同步/异步 |
| `cfg` / `config` | configuration | 配置 |
| `env` | environment | 环境 |
| `msg` | message | 消息 |
| `sem` | semaphore | 信号量 |
| `mutex` | mutual exclusion | 互斥量 |
| `cond` | condition | 条件 |
| `addr` | address | 地址 |
| `hdr` | header | 头部 |
| `req` / `resp` | request / response | 请求/响应 |
| `recv` | receive | 接收 |
| `auth` | authentication | 认证 |
| `perm` | permission | 权限 |
| `desc` | description | 描述 |
| `info` | information | 信息 |
| `doc` | document | 文档 |
| `dir` | directory | 目录 |
| `exec` | execute | 执行 |
| `sig` | signal | 信号 |
| `pid` / `tid` | process ID / thread ID | 进程/线程 ID |
| `io` | input/output | 输入输出 |
| `eof` | end of file | 文件结束 |
| `eol` | end of line | 行结束 |
| `sep` | separator | 分隔符 |
| `delim` | delimiter | 定界符 |
| `fmt` | format | 格式 |
| `hex` / `dec` / `oct` | hexadecimal / decimal / octal | 十六/十/八进制 |
| `sync` | synchronize | 同步 |
| `lock` / `unlock` | 锁/解锁 | 加锁/解锁 |
| `min` / `max` | minimum / maximum | 最小/最大 |
| `avg` | average | 平均 |
| `std` | standard / standard deviation | 标准/标准差 |
| `dev` | device / development | 设备/开发 |
| `prod` | production | 生产 |
| `dbg` | debug | 调试 |
| `perf` | performance | 性能 |
| `mem` | memory | 内存 |
| `cpu` | central processing unit | 处理器 |
| `gpu` | graphics processing unit | 图形处理器 |
| `os` | operating system | 操作系统 |
| `hw` / `sw` | hardware / software | 硬件/软件 |
| `sdk` | software development kit | 软件开发工具包 |
| `api` | application programming interface | 应用编程接口 |
| `abi` | application binary interface | 应用二进制接口 |
| `cli` / `gui` | command-line interface / graphical user interface | 命令行/图形界面 |
| `tui` | terminal user interface | 终端用户界面 |

---

## 10. 转义字符

| 转义字符 | 含义 | ASCII 码 |
|---------|------|:--------:|
| `\n` | 换行（LF） | 10 |
| `\r` | 回车（CR） | 13 |
| `\t` | 水平制表符 | 9 |
| `\v` | 垂直制表符 | 11 |
| `\b` | 退格 | 8 |
| `\f` | 换页 | 12 |
| `\a` | 响铃 | 7 |
| `\\` | 反斜杠 | 92 |
| `\'` | 单引号 | 39 |
| `\"` | 双引号 | 34 |
| `\?` | 问号 | 63 |
| `\0` | 空字符（NUL） | 0 |
| `\ooo` | 八进制表示 | — |
| `\xhh` | 十六进制表示 | — |
| `\uhhhh` | Unicode（C++11） | — |
| `\Uhhhhhhhh` | Unicode（C++11） | — |

---

## 11. 字面量后缀

| 后缀 | 含义 | 示例 | 类型 |
|------|------|------|------|
| `u` / `U` | 无符号 | `42U` | `unsigned int` |
| `l` / `L` | 长整型 | `42L` | `long` |
| `ll` / `LL` | 长长整型 | `42LL` | `long long` |
| `f` / `F` | 单精度 | `3.14f` | `float` |
| （无） | 双精度 | `3.14` | `double` |
| `i` / `I` | 虚数（C99） | `3.14i` | `_Complex` |
| `u8` | UTF-8 字符串（C++17） | `u8"hello"` | `const char8_t[]` |
| `u` | UTF-16 字符串（C++11） | `u"hello"` | `const char16_t[]` |
| `U` | UTF-32 字符串（C++11） | `U"hello"` | `const char32_t[]` |
| `R` | 原始字符串（C++11） | `R"(raw\nstring)"` | `const char[]` |

**组合后缀**：`42ULL` = `unsigned long long`，`3.14fL` 无效

---

## 12. C++ 常用惯用标识符

### 12.1 特殊成员函数

| 函数签名 | 名称 | 用途 |
|---------|------|------|
| `T()` | 默认构造函数 | 无参构造 |
| `T(const T&)` | 拷贝构造 | 用另一个对象初始化 |
| `T(T&&)` | 移动构造（C++11） | 窃取资源 |
| `T& operator=(const T&)` | 拷贝赋值 | 复制赋值 |
| `T& operator=(T&&)` | 移动赋值（C++11） | 窃取赋值 |
| `~T()` | 析构函数 | 销毁时清理 |

### 12.2 运算符重载符号

| 运算符 | 函数名 | 示例 |
|--------|--------|------|
| `+` | `operator+` | `a + b` |
| `-` | `operator-` | `a - b` |
| `*` | `operator*` | `a * b` |
| `/` | `operator/` | `a / b` |
| `%` | `operator%` | `a % b` |
| `++` | `operator++` | `++a` / `a++` |
| `--` | `operator--` | `--a` / `a--` |
| `==` | `operator==` | `a == b` |
| `!=` | `operator!=` | `a != b` |
| `<` | `operator<` | `a < b` |
| `>` | `operator>` | `a > b` |
| `<=` | `operator<=` | `a <= b` |
| `>=` | `operator>=` | `a >= b` |
| `<=>` | `operator<=>`（C++20） | `a <=> b` |
| `<<` | `operator<<` | `cout << x` |
| `>>` | `operator>>` | `cin >> x` |
| `()` | `operator()` | `obj(args)` — 仿函数 |
| `[]` | `operator[]` | `obj[idx]` |
| `->` | `operator->` | `ptr->member` |
| `*` | `operator*`（解引用） | `*ptr` |
| `type` | `operator type()` | 隐式转换 |
| `""_x` | `operator""_x`（C++11） | 自定义字面量 |

---

## 13. 编译器相关关键字/扩展

| 关键字/属性 | 编译器 | 含义 |
|------------|--------|------|
| `__attribute__((...))` | GCC/Clang | 函数/变量属性 |
| `__declspec(...)` | MSVC | 声明属性 |
| `__builtin_expect` | GCC | 分支预测提示 |
| `__asm__` / `asm` | GCC/Clang | 内联汇编 |
| `__restrict__` | GCC | restrict 的 GCC 版本 |
| `__thread` | GCC | 线程局部存储（旧式） |
| `[[nodiscard]]` | C++17 | 忽略返回值时警告 |
| `[[maybe_unused]]` | C++17 | 抑制未使用警告 |
| `[[deprecated]]` | C++14 | 标记弃用 |
| `[[likely]]` / `[[unlikely]]` | C++20 | 分支预测提示 |
| `[[fallthrough]]` | C++17 | switch 穿透提示 |
| `[[noreturn]]` | C++11 | 函数不返回 |
| `[[assume]]` | C++23 | 编译器假设 |

---

## 14. 速查统计

| 类别 | 数量 | 说明 |
|------|:----:|------|
| C89 关键字 | 32 | C 语言基础 |
| C99 新增 | 5 | `_Bool` `_Complex` `_Imaginary` `inline` `restrict` |
| C11 新增 | 7 | `_Alignas` `_Alignof` `_Atomic` `_Generic` `_Noreturn` `_Static_assert` `_Thread_local` |
| C23 新增 | ~9 | `bool` `true` `false` `nullptr` `constexpr` `typeof` 等 |
| C++98 关键字 | ~48 | 含 C 语言关键字 |
| C++11 新增 | ~12 | `nullptr` `constexpr` `decltype` `auto` 等 |
| C++20 新增 | 8 | `concept` `requires` `co_await` `co_return` `co_yield` `consteval` `constinit` `char8_t` |
| C++23 新增 | 2 | `import` `module` |
| 预处理指令 | ~12 | `#include` `#define` `#if` 等 |
| 运算符 | ~50 | 含 C++ 新增 |
| 常见缩写 | ~70 | 编程常用英文缩写 |

---

## 15. 本章小结

> 🎯 **记住这些就够了**：
>
> 1. **C89 的 32 个关键字**是基础中的基础，必须全部认识
> 2. **C++ 的 80 个关键字**不需要死记硬背，用多了自然记住
> 3. **预处理指令**是编译前的"预处理"，不是运行时
> 4. **命名缩写**是程序员之间的"行话"，读懂别人的代码需要
> 5. **不要用关键字做变量名**——编译器会直接报错
> 6. **以下划线开头的名字**（如 `_Reserved`）通常保留给实现，不要自己用

---

> 📌 **相关阅读**
> - [编程入门](./00-编程入门.md) — 编程基础概念
> - [计算机术语比喻理解](../03-问题解答/01-基础概念/00-计算机术语比喻理解.md) — 用比喻理解术语
> - [C与Cpp基础数据类型差异](../03-问题解答/01-基础概念/07-C与Cpp基础数据类型差异.md) — C 和 C++ 类型对比
> - [什么是语法糖](../03-问题解答/01-基础概念/12-什么是语法糖.md) — 让代码更简洁的语法特性
