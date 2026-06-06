# sizeof 运算符常见误区
> 📖 相关章节：[运算符与表达式](../../01-C语言/02-运算符与表达式.md)、[控制结构](../../01-C语言/03-控制结构.md)、[函数](../../01-C语言/04-函数.md)、[基础特性](../../02-CPP/01-基础特性.md)、[命名空间](../../02-CPP/02-命名空间与编码规范.md)

### 1. 要义概览

**sizeof** 在编译期求值，不运行代码。常见误区：数组作参数退化为指针、空类不为0、对齐填充、虚函数增加大小、字符串 sizeof 和 strlen 混淆。

***

### 2. sizeof 的基本规则

```cpp
// sizeof 是编译期运算符，不运行代码
int x = 0;
sizeof(x++);  // x 不会自增！编译期就确定了结果
// x 仍然是 0

// sizeof 的返回类型是 size_t（无符号整数）
std::cout << sizeof(int) << std::endl;    // 4
std::cout << sizeof(double) << std::endl; // 8
std::cout << sizeof(char) << std::endl;   // 1（C++ 规定 char 永远是 1）
```

**关键规则**：
1. sizeof 在编译期求值，不运行任何代码
2. sizeof(char) == 1 永远成立（C++ 标准）
3. sizeof 返回 `size_t` 类型
4. sizeof 对引用类型返回被引用类型的大小
5. sizeof 对函数调用返回返回值类型的大小（不调用函数）

```cpp
int& ref = x;
sizeof(ref);     // 等于 sizeof(int) = 4，不是指针大小

int func();
sizeof(func());  // 等于 sizeof(int) = 4，不会调用 func
```

### 3. 误区1：数组参数退化

这是最常见的 sizeof 误区：

```cpp
int arr[10];
sizeof(arr);  // 40（10 × 4）— 在定义处，数组不退化

void foo(int arr[]) {     // 等价于 void foo(int* arr)
    sizeof(arr);           // 8（指针大小！数组退化为指针了）
}

void bar(int (&arr)[10]) {  // 数组引用，不退化
    sizeof(arr);             // 40（10 × 4）
}

// 计算数组元素个数的正确方法
template <typename T, size_t N>
constexpr size_t array_size(T (&)[N]) {
    return N;
}

int main() {
    int data[20];
    std::cout << array_size(data) << std::endl;  // 20

    // C++17 更简洁
    std::cout << std::size(data) << std::endl;   // 20

    // 旧式写法（危险：指针上会得到错误结果）
    #define ARRAY_LEN(arr) (sizeof(arr) / sizeof(arr[0]))
    std::cout << ARRAY_LEN(data) << std::endl;   // 20
}
```

| 场景 | sizeof 结果 | 原因 |
|------|------------|------|
| 定义处 `int arr[10]; sizeof(arr)` | 40 | 数组类型，不退化 |
| 函数参数 `void f(int arr[])` | 8 | 退化为指针 |
| 堆数组 `int* p = new int[10]; sizeof(p)` | 8 | 指针，不是数组 |
| 数组引用 `void f(int (&arr)[10])` | 40 | 引用不退化 |

### 4. 误区2：空类的大小

```cpp
class Empty {};
sizeof(Empty);  // 1（不是0！）

// 为什么不是0？
// C++ 规定：每个对象必须有唯一的地址
// 如果大小为0，两个 Empty 对象会共享同一地址
Empty e1, e2;
// &e1 != &e2 必须成立 → 每个对象至少占1字节

// EBO：空基类优化
class Empty {};
class Derived : public Empty {
    int x;
};
sizeof(Derived);  // 4（不是5！Empty 部分被优化掉了）

// 无 EBO 的情况
class Contains {
    Empty e;  // 成员变量，不是基类
    int x;
};
sizeof(Contains);  // 8（1 + 3 padding + 4）
```

| 类定义 | sizeof | 说明 |
|--------|--------|------|
| `class Empty {};` | 1 | C++ 规定空类至少1字节 |
| `class Empty {}; class D : public Empty { int x; };` | 4 | EBO 优化 |
| `class C { Empty e; int x; };` | 8 | 无 EBO，有 padding |
| `class MultiEmpty : public Empty1, public Empty2 { int x; };` | 4 | 多重 EBO |

### 5. 误区3：对齐填充（Padding）

```cpp
struct S1 {
    char c;   // 1 字节
    int i;    // 4 字节
    short s;  // 2 字节
};
sizeof(S1);  // 12（不是7！）

// 内存布局：
// offset: 0  1  2  3  4  5  6  7  8  9  10 11
//         c  .  .  .  i  i  i  i  s  s  .  .
//         ^^^^^^^^^^^^  ^^^^^^^^^^^^  ^^^^^^^^
//         c + padding   int           s + padding
```

**对齐规则**：
1. 成员的偏移量必须是该成员对齐要求的整数倍
2. 结构体总大小必须是最大对齐要求的整数倍
3. `int` 通常要求4字节对齐，`double` 要求8字节对齐

```cpp
// 调整成员顺序可以减小结构体大小
struct Bad {
    char c;    // 1 + 3 padding
    int i;     // 4
    char d;    // 1 + 3 padding
};
sizeof(Bad);  // 12

struct Good {
    int i;     // 4
    char c;    // 1
    char d;    // 1 + 2 padding
};
sizeof(Good);  // 8

// 最佳实践：按对齐要求从大到小排列成员
struct Best {
    double d;  // 8
    int i;     // 4
    short s;   // 2
    char c;    // 1 + 1 padding
};
sizeof(Best);  // 16
```

#### 1. 手动控制对齐

```cpp
// #pragma pack 控制对齐
#pragma pack(push, 1)  // 1字节对齐
struct Packed {
    char c;   // 1
    int i;    // 4
    short s;  // 2
};
#pragma pack(pop)
sizeof(Packed);  // 7（无 padding）

// alignas 指定对齐（C++11）
struct Aligned {
    alignas(16) char c;  // 强制16字节对齐
    int i;
};
sizeof(Aligned);  // 16

// alignof 查询对齐要求
std::cout << alignof(int) << std::endl;     // 4
std::cout << alignof(double) << std::endl;  // 8
std::cout << alignof(S1) << std::endl;      // 4
```

### 6. 误区4：虚函数增加大小

```cpp
class NoVirtual {
    int x;
};
sizeof(NoVirtual);  // 4

class WithVirtual {
    virtual void f();
    int x;
};
sizeof(WithVirtual);  // 16（64位系统）
// 8字节 vptr + 4字节 x + 4字节 padding

class MultipleVirtual {
    virtual void f1();
    virtual void f2();
    int x;
};
sizeof(MultipleVirtual);  // 16（虚函数再多也只有一个 vptr）

// 多继承可能有多个 vptr
class Base1 { virtual void f1(); };
class Base2 { virtual void f2(); };
class Derived : public Base1, public Base2 {
    int x;
};
sizeof(Derived);  // 24（vptr1 + vptr2 + x + padding）
```

| 类 | 成员 | sizeof(64位) | 说明 |
|----|------|:---:|------|
| `class A { int x; };` | int | 4 | 无虚函数 |
| `class B { virtual void f(); int x; };` | vptr+int | 16 | 1个vptr |
| `class C { virtual void f1(); virtual void f2(); int x; };` | vptr+int | 16 | 仍然1个vptr |
| `class D : public Base1, public Base2 { int x; };` | vptr1+vptr2+int | 24 | 多继承2个vptr |

### 7. 误区5：字符串的 sizeof

```cpp
const char* s = "hello";
sizeof(s);       // 8（指针大小，64位系统）
sizeof("hello"); // 6（5个字符 + '\0'）

char str[] = "hello";
sizeof(str);   // 6（数组大小，包含 '\0'）
strlen(str);   // 5（字符串长度，不包含 '\0'）

char str2[10] = "hello";
sizeof(str2);  // 10（数组声明大小）
strlen(str2);  // 5（字符串长度）

std::string ss = "hello";
sizeof(ss);    // 32 左右（std::string 对象大小，不是字符串长度）
              // 典型实现：指针 + 大小 + 容量 或 SSO 缓冲区
```

| 表达式 | sizeof | 说明 |
|--------|--------|------|
| `const char* s = "hello"; sizeof(s)` | 8 | 指针大小 |
| `sizeof("hello")` | 6 | 字符串字面量包含 '\0' |
| `char str[] = "hello"; sizeof(str)` | 6 | 数组大小，包含 '\0' |
| `char str[10] = "hello"; sizeof(str)` | 10 | 数组声明大小 |
| `std::string s = "hello"; sizeof(s)` | ~32 | string 对象大小 |

### 8. sizeof 与 strlen 的区别

```cpp
char str[] = "hello";
sizeof(str);   // 6（编译期求值，包含 '\0'）
strlen(str);   // 5（运行时求值，不包含 '\0'）

// 关键区别
```

| 对比项 | sizeof | strlen |
|--------|--------|--------|
| 求值时机 | 编译期 | 运行时 |
| 是否包含 '\0' | 包含 | 不包含 |
| 参数类型 | 任意类型 | C 风格字符串 |
| 对指针 | 返回指针大小 | 返回字符串长度 |
| 对数组 | 返回数组总大小 | 返回字符串长度 |
| 性能 | 零开销 | O(n) 遍历 |

```cpp
// 危险：在指针上用 strlen 前没检查
const char* p = nullptr;
// strlen(p);  // 崩溃！空指针
sizeof(p);     // 安全，返回 8

// 危险：混淆 sizeof 和 strlen 分配内存
const char* src = "hello";
char* dst = new char[strlen(src)];  // 错误！没空间放 '\0'
char* dst2 = new char[strlen(src) + 1];  // 正确
```

### 9. 不同类型的大小

#### 1. 基本类型（64位系统，LP64 模型）

| 类型 | sizeof | 说明 |
|------|:---:|------|
| `bool` | 1 | 最小1字节 |
| `char` | 1 | C++ 规定 |
| `short` | 2 | — |
| `int` | 4 | — |
| `long` | 4(Windows) / 8(Linux) | 平台相关！ |
| `long long` | 8 | — |
| `float` | 4 | — |
| `double` | 8 | — |
| `long double` | 8/12/16 | 平台相关 |
| `void*` | 8 | 64位指针 |
| `int*` | 8 | 64位指针 |

#### 2. 固定宽度类型（推荐）

```cpp
#include <cstdint>

// 大小固定的类型，跨平台一致
sizeof(int8_t);    // 1
sizeof(int16_t);   // 2
sizeof(int32_t);   // 4
sizeof(int64_t);   // 8
sizeof(uint8_t);   // 1
sizeof(uint64_t);  // 8

// 指针大小
sizeof(intptr_t);  // 8（64位）/ 4（32位），与指针同大小
```

#### 3. STL 容器的大小

```cpp
// 64位系统，libstdc++ 典型实现
sizeof(std::string);           // 32（SSO 实现）
sizeof(std::vector<int>);      // 24（指针+大小+容量）
sizeof(std::list<int>);        // 24（两个指针）
sizeof(std::map<int,int>);     // 48（红黑树头节点）
sizeof(std::unordered_map<int,int>); // 56（哈希表）
sizeof(std::unique_ptr<int>);  // 8（一个指针）
sizeof(std::shared_ptr<int>);  // 16（两个指针：对象指针+控制块指针）
```

### 10. 误区6：位域的 sizeof

```cpp
struct Flags {
    unsigned int a : 1;  // 1 bit
    unsigned int b : 3;  // 3 bits
    unsigned int c : 4;  // 4 bits
};
sizeof(Flags);  // 4（一个 unsigned int 的大小）
// 位域不按 bit 计算 sizeof，按底层类型计算

struct MixedFlags {
    unsigned int a : 1;
    unsigned int b : 31;
    unsigned int c : 1;  // 新的 unsigned int
};
sizeof(MixedFlags);  // 8（两个 unsigned int）
```

### 11. 误区7：枚举的 sizeof

```cpp
enum Color { Red, Green, Blue };
sizeof(Color);  // 4（默认底层类型是 int）

enum class Color8 : uint8_t { Red, Green, Blue };
sizeof(Color8);  // 1（指定底层类型为 uint8_t）

enum class Color32 : uint32_t { Red, Green, Blue };
sizeof(Color32);  // 4

// C++11 可以指定底层类型
enum class Small : bool { No, Yes };
sizeof(Small);  // 1
```

### 12. 完整示例：结构体大小分析

```cpp
#include <iostream>
#include <cstddef>

struct Example {
    char a;     // offset 0, size 1
    // padding: 3 bytes
    int b;      // offset 4, size 4
    short c;    // offset 8, size 2
    // padding: 2 bytes
    double d;   // offset 12 → 不行！double 需要8字节对齐
                // 实际 offset 16, size 8
};

// 实际布局：
// offset: 0  1-3    4-7   8-9  10-15  16-23
//         a  pad    b     c    pad    d
// 总大小: 24

int main() {
    std::cout << "sizeof(Example): " << sizeof(Example) << std::endl;  // 24
    std::cout << "alignof(Example): " << alignof(Example) << std::endl;  // 8
    std::cout << "offsetof(Example, a): " << offsetof(Example, a) << std::endl;  // 0
    std::cout << "offsetof(Example, b): " << offsetof(Example, b) << std::endl;  // 4
    std::cout << "offsetof(Example, c): " << offsetof(Example, c) << std::endl;  // 8
    std::cout << "offsetof(Example, d): " << offsetof(Example, d) << std::endl;  // 16

    // 优化后的版本
    struct ExampleOptimized {
        double d;  // offset 0, size 8
        int b;     // offset 8, size 4
        short c;   // offset 12, size 2
        char a;    // offset 14, size 1
                   // padding: 1 byte
    };
    // 总大小: 16

    std::cout << "sizeof(ExampleOptimized): "
              << sizeof(ExampleOptimized) << std::endl;  // 16
}
```

### 13. 极简总结

**sizeof = 编译期求值 → 数组退化成指针 → 空类1字节 → 有padding → 虚函数加vptr**

| 误区 | 正确理解 |
|------|----------|
| 数组参数 sizeof | 退化为指针，不是数组大小 |
| 空类 sizeof | 至少1字节 |
| 结构体 sizeof | 包含对齐填充 |
| 虚函数类 sizeof | 多一个 vptr（8字节） |
| 字符串 sizeof | 指针是8字节，字面量含 '\0' |
| sizeof vs strlen | 编译期 vs 运行时，含 '\0' vs 不含 |
| 成员顺序 | 影响结构体大小，按对齐从大到小排列 |

***

### 相关阅读

- [浮点数精度陷阱](./19-浮点数精度陷阱.md)
- [原码反码与补码](./33-原码反码与补码.md)
- [C与Cpp基础数据类型差异](./00-C与Cpp基础数据类型差异.md)