# extern 关键字详解
> 📖 相关章节：[运算符与表达式](../../01-C语言/02-运算符与表达式.md)、[控制结构](../../01-C语言/03-控制结构.md)、[函数](../../01-C语言/04-函数.md)、[基础特性](../../02-CPP/01-基础特性.md)、[命名空间](../../02-CPP/02-命名空间与编码规范.md)

> **extern tells the compiler: "I promise this exists somewhere else."** — extern 告诉编译器："我保证它存在于别处。"

***

### 1. 要义概览

**`extern`** 是「告诉编译器：这个变量/函数，定义在别的地方」。只做**声明**，不分配内存、不写定义。

***

### 2. 两种基本用法

#### 1. 修饰全局变量

| 写法 | 性质 | 是否分配内存 |
|------|------|-------------|
| `int g_num;` | 定义 | 是，分配内存，初始值为0 |
| `extern int g_num;` | 声明 | 否，仅告知变量在别处 |

多文件共享全局变量的经典场景：

**文件 A.cpp**：

```cpp
int g_num = 100;
```

**文件 B.cpp**：

```cpp
#include <cstdio>

extern int g_num;

void test() {
    printf("%d\n", g_num);
}
```

如果不写 `extern`，在 B.cpp 再写 `int g_num` 会报**重复定义错误**。

#### 2. 修饰函数

```cpp
extern void hello();
```

函数默认自带 extern，加不加都一样：

```cpp
void hello();       // 等价于
extern void hello();
```

### 3. 内部链接与外部链接

链接性（Linkage）决定了名字在多个翻译单元中的可见性。

| 链接性 | 含义 | 示例 |
|--------|------|------|
| 外部链接 | 其他翻译单元可见 | 普通全局变量、非static函数 |
| 内部链接 | 仅当前翻译单元可见 | static全局变量、static函数、const全局变量 |
| 无链接 | 仅当前作用域可见 | 局部变量 |

```cpp
int g_global = 1;           // 外部链接
static int g_file_only = 2; // 内部链接
const int g_const = 3;      // C++中默认内部链接
extern const int g_ext_const = 4; // 加extern变外部链接

void func() {}              // 外部链接
static void helper() {}     // 内部链接
```

**const 全局变量的陷阱**：C++ 中 `const` 全局变量默认内部链接，其他文件 `extern` 也找不到，除非定义时加 `extern`：

```cpp
// file1.cpp
extern const int kMaxSize = 1024;  // 必须加extern才是外部链接

// file2.cpp
extern const int kMaxSize;  // 现在可以找到
```

### 4. extern "C" — C++与C的桥梁

C++编译器会对函数名做名称修饰（Name Mangling），C编译器不会。当C++程序需要调用C编译的库时，必须用 `extern "C"` 禁止名称修饰。

```cpp
#ifdef __cplusplus
extern "C" {
#endif

int c_library_func(int arg);
void c_library_init(void);

#ifdef __cplusplus
}
#endif
```

名称修饰的实际效果：

| 函数签名 | C编译后的符号名 | C++编译后的符号名（GCC） |
|----------|----------------|--------------------------|
| `int add(int, int)` | `add` | `_Z3addii` |
| `float add(float, float)` | `add`（冲突！） | `_Z3addff` |

这就是为什么C不支持函数重载而C++支持——名称修饰让同名不同参数的函数有了不同的符号名。

### 5. extern template（C++11）

模板在使用时隐式实例化，多个翻译单元可能重复实例化同一个模板，导致编译时间暴增。`extern template` 显式阻止隐式实例化：

```cpp
// header.h
template<typename T>
class BigContainer {
    T data_[10000];
public:
    void process();
};

// main.cpp — 显式实例化定义（真正生成代码）
#include "header.h"
template class BigContainer<int>;  // 只在这里实例化

// other.cpp — 显式实例化声明（不生成代码，引用main.cpp中的）
#include "header.h"
extern template class BigContainer<int>;  // 不实例化，链接到main.cpp的

void other_func() {
    BigContainer<int> container;
    container.process();
}
```

| 写法 | 含义 | 效果 |
|------|------|------|
| `template class Foo<int>;` | 显式实例化定义 | 生成代码 |
| `extern template class Foo<int>;` | 显式实例化声明 | 不生成代码，引用别处的 |

### 6. thread_local 与 extern

`thread_local` 变量在每个线程中独立一份，可以与 `extern` 组合使用：

```cpp
// thread_data.h
extern thread_local int thread_counter;

// thread_data.cpp
thread_local int thread_counter = 0;

// worker.cpp
#include <iostream>
#include <thread>

extern thread_local int thread_counter;

void worker(int id) {
    thread_counter = id;
    std::cout << "线程 " << id
              << " counter = " << thread_counter << std::endl;
}

int main() {
    std::thread t1(worker, 1);
    std::thread t2(worker, 2);
    t1.join();
    t2.join();
}
```

每个线程有自己的 `thread_counter` 副本，互不干扰。

### 7. inline 变量（C++17）与 extern 的关系

C++17 引入 `inline` 变量，允许头文件中定义变量而不违反ODR（One Definition Rule）：

```cpp
// C++17之前：头文件中只能声明
extern const int kVersion;  // 声明
// 某个.cpp中：const int kVersion = 2;

// C++17：头文件中直接定义
inline const int kVersion = 2;  // 多个翻译单元包含也OK
```

| 方案 | 头文件写法 | 是否需要.cpp定义 | C++版本 |
|------|-----------|-----------------|---------|
| extern + 定义 | `extern const int k;` | 需要 | C++98 |
| constexpr | `constexpr int k = 2;` | 不需要 | C++11 |
| inline变量 | `inline const int k = 2;` | 不需要 | C++17 |

### 8. extern vs static 对比表

| 维度 | extern | static（全局） |
|------|--------|---------------|
| 链接性 | 外部链接 | 内部链接 |
| 可见性 | 所有翻译单元 | 仅当前翻译单元 |
| 典型用途 | 跨文件共享变量 | 文件私有变量/函数 |
| 与const组合 | `extern const` = 外部链接 | `static const` = 内部链接 |
| 内存 | 不分配（仅声明） | 分配（定义） |
| ODR | 声明可多次，定义只能一次 | 每个翻译单元可有自己的 |

### 9. 常见陷阱

#### 1. 陷阱1：extern声明与定义类型不匹配

```cpp
// file1.cpp
double g_value = 3.14;

// file2.cpp
extern int g_value;  // 类型不匹配！链接器不报错，运行时数据错乱
```

#### 2. 陷阱2：在头文件中写extern定义

```cpp
// bad.h
extern int g_count = 0;  // 这是定义，不是声明！多文件包含会重复定义
```

正确做法：头文件只声明，一个.cpp中定义。

```cpp
// good.h
extern int g_count;  // 声明

// good.cpp
int g_count = 0;  // 定义
```

#### 3. 陷阱3：在函数内用extern

```cpp
void func() {
    extern int g_data;  // 合法但罕见，容易混淆
}
```

虽然语法合法，但函数内的 `extern` 声明作用域仅限函数内，容易让人误以为是局部变量。建议将 `extern` 声明放在文件作用域。

### 10. 与前向声明的区别

| 特性 | 前向声明 | extern |
|------|----------|--------|
| 适用对象 | 类、结构体 | 变量、函数 |
| 目的 | 告诉编译器有这个类型 | 告诉编译器变量在别处定义 |
| 典型场景 | 减少头文件包含 | 跨翻译单元共享 |

都是**先报备、后使用**，只是用途不一样。

### 11. 极简总结

**extern = 声明在别处 → 基本用法：跨文件共享全局变量 → extern "C"：C/C++互调 → extern template：阻止模板重复实例化 → inline变量（C++17）：头文件定义变量 → 注意类型匹配和声明/定义区分**

***

### 相关阅读

- [extern-C与动态库导出](./09-extern-C与动态库导出.md)
- [static关键字](./10-static关键字.md)
- [头文件守卫与pragmaonce](./16-头文件守卫与pragmaonce.md)