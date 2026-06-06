# inline 关键字的真实含义
> 📖 相关章节：[运算符与表达式](../../01-C语言/02-运算符与表达式.md)、[控制结构](../../01-C语言/03-控制结构.md)、[函数](../../01-C语言/04-函数.md)、[基础特性](../../02-CPP/01-基础特性.md)、[命名空间](../../02-CPP/02-命名空间与编码规范.md)

### 1. 精髓速览

**inline 的真实含义不是"内联展开"，而是"允许重复定义"**。编译器自行决定是否内联，inline 关键字只是建议。

***

### 2. inline 解决什么问题

```cpp
// header.h
inline int add(int a, int b) { return a + b; }

// 多个 .cpp 包含 header.h → 每个翻译单元都有 add 的定义
// 没有 inline → 链接错误（重复定义）
// 有 inline → 链接器选一个，OK
```

#### 1. ODR（单一定义规则）

C++ 的 **ODR**（One Definition Rule）规定：每个函数/变量在整个程序中只能有一个定义。

```cpp
// a.cpp
#include "header.h"  // 包含 add 定义

// b.cpp
#include "header.h"  // 也包含 add 定义

// 链接时：两个 .o 都有 add 的定义
// 没有 inline → 链接器报 "multiple definition" 错误
// 有 inline → 链接器接受多个定义，任选一个使用
```

### 3. inline 不保证内联

```cpp
inline void huge_function() {
    // 1000 行代码
    // 编译器几乎不会内联这个函数
}
```

**编译器根据以下因素自行决定是否内联：**

| 因素 | 影响 |
|:---|:---|
| 函数体大小 | 太大 → 不内联 |
| 调用频率 | 高频小函数 → 倾向内联 |
| 递归函数 | 一般不会内联（可展开有限深度） |
| 虚函数 | 动态调用不会内联，静态调用可能内联 |
| 函数指针调用 | 通过指针调用的不能内联 |
| 编译器优化级别 | `-O0` 基本不内联，`-O2` 积极内联 |

#### 1. 编译器内联决策示例

```cpp
// ✅ 几乎一定会被内联
inline int square(int x) { return x * x; }

// ⚠️ 条件内联（取决于编译器和优化级别）
inline int max(int a, int b) { return a > b ? a : b; }

// ❌ 几乎不会被内联
inline void complex_algorithm() {
    std::vector<int> v(1000);
    // 大量复杂操作...
}
```

### 4. 什么函数自动是 inline 的

| 函数类型 | 自动 inline | 原因 |
|:---|:---:|:---|
| 类内定义的成员函数 | ✅ | 隐式 inline，可在头文件定义 |
| 模板函数 | ✅ | 模板必须可见，允许重复定义 |
| constexpr 函数 | ✅ | 固有时在编译期求值 |
| 类外定义的成员函数 | ❌ 需要加 inline | 否则违反 ODR |

### 5. C++17 inline 变量

C++17 之前，只有函数可以 inline，变量定义在头文件中会导致链接错误。

```cpp
// header.h
// C++17 之前 → 错误：变量重复定义
struct Config {
    static int port;  // 只能在 .cpp 中定义
};

// C++17 之后 → 正确：inline 变量
struct Config {
    inline static int port = 8080;  // 允许头文件中定义
    inline static const char* name = "server";
};

// 也可以定义全局 inline 变量
inline int global_timeout = 30;
```

### 6. inline 与链接器

```
没有 inline:
  a.o ── add 定义 ──┐
                     ├── 链接错误 "multiple definition"
  b.o ── add 定义 ──┘

有 inline:
  a.o ── add 定义(weak) ──┐
                          ├── 链接器选一个，OK
  b.o ── add 定义(weak) ──┘
```

inline 函数在目标文件中通常以 **weak symbol** 形式存在，链接器在遇到多个 weak symbol 时不会报错。

### 7. 强制内联：编译器扩展

```cpp
// GCC/Clang：始终内联（建议，仍然不是绝对的）
__attribute__((always_inline)) inline int fast_add(int a, int b) {
    return a + b;
}

// MSVC
__forceinline int fast_add(int a, int b) {
    return a + b;
}
```

即使使用 `__forceinline`，某些情况下编译器仍可能拒绝内联（递归、虚函数、大量异常处理等）。

### 8. 极简总结

**inline = 允许重复定义（不是强制内联）→ 编译器自行决定是否内联 → 头文件中的函数必须 inline → C++17 起也支持 inline 变量**

***

### 相关阅读

- [const关键字](./11-const关键字.md)
- [头文件守卫与pragmaonce](./16-头文件守卫与pragmaonce.md)
- [什么是开销Overhead](./03-什么是开销Overhead.md)