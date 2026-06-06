# 什么是名称修饰 Name Mangling
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)、[静态库](../../01-C语言/18-静态库.md)、[动态库](../../01-C语言/19-动态库与共享库.md)、[CMake](../../01-C语言/23-CMake构建系统.md)

> 名称修饰是 C++ 编译器为支持函数重载、命名空间和类成员而将符号名编码为唯一内部名称的机制，是链接器能正确工作的幕后功臣。

***

### 1. 一句话概括

名称修饰（Name Mangling）是 C++ 编译器将函数签名（名称+参数类型+命名空间+类名等）编码为唯一字符串的过程，使得重载函数、不同类的同名方法在目标文件中拥有不同符号名，链接器能正确解析。

***

### 2. 为什么 C++ 需要名称修饰

C 语言不支持函数重载，符号名就是函数名本身。C++ 引入重载后，多个同名函数必须区分，否则链接器无法分辨。

```cpp
int add(int a, int b) { return a + b; }
double add(double a, double b) { return a + b; }
int add(int a, int b, int c) { return a + b + c; }
```

| 语言 | 符号名 | 问题 |
|------|--------|------|
| C | `add` | 三个函数符号名冲突，链接失败 |
| C++ (修饰后) | `_Z3addii`, `_Z3adddd`, `_Z3addiii` | 每个函数唯一，链接成功 |

C 语言链接时只看函数名，无法区分重载。C++ 必须将完整签名编码进符号名，这就是名称修饰的根本原因。

```cpp
#include <iostream>

void foo(int x) { std::cout << "foo(int): " << x << "\n"; }
void foo(double x) { std::cout << "foo(double): " << x << "\n"; }
void foo(int x, double y) { std::cout << "foo(int,double): " << x << "," << y << "\n"; }

namespace NS {
    void foo(int x) { std::cout << "NS::foo(int): " << x << "\n"; }
}

class MyClass {
public:
    void foo(int x) { std::cout << "MyClass::foo(int): " << x << "\n"; }
    void foo(double x) { std::cout << "MyClass::foo(double): " << x << "\n"; }
    static void bar(int x) { std::cout << "MyClass::bar(int): " << x << "\n"; }
};

int main() {
    foo(1);
    foo(1.0);
    foo(1, 2.0);
    NS::foo(1);
    MyClass obj;
    obj.foo(1);
    obj.foo(1.0);
    MyClass::bar(1);
    return 0;
}
```

> 以上 7 个 `foo`/`bar` 函数，经名称修饰后产生 7 个不同的符号名，链接器能一一区分。

***

### 3. Itanium ABI 修饰规则详解

GCC、Clang 及大多数非 Windows 平台采用 Itanium C++ ABI 的修饰方案。其基本结构为：

```
_Z <长度><名称> <参数编码>
```

| 编码 | 含义 | 编码 | 含义 |
|------|------|------|------|
| `i` | int | `d` | double |
| `f` | float | `c` | char |
| `l` | long | `x` | long long |
| `b` | bool | `v` | void |
| `Pk` | const 指针 | `R` | 引用 |
| `N...E` | 嵌套名称(命名空间/类) | `1A` | 长度1的名称A |

**解码示例**：

| 修饰后符号 | 解码过程 | 原始签名 |
|-----------|---------|---------|
| `_Z3addii` | `_Z` + `3add` + `i` + `i` | `add(int, int)` |
| `_Z3adddd` | `_Z` + `3add` + `d` + `d` | `add(double, double)` |
| `_ZN2NS3fooEi` | `_Z` + `N` + `2NS` + `3foo` + `E` + `i` | `NS::foo(int)` |
| `_ZN7MyClass3fooEi` | `_Z` + `N` + `7MyClass` + `3foo` + `E` + `i` | `MyClass::foo(int)` |
| `_ZN7MyClass3barEi` | `_Z` + `N` + `7MyClass` + `3bar` + `E` + `i` | `MyClass::bar(int)` |

```cpp
#include <iostream>

namespace Math {
    class Calculator {
    public:
        int compute(int x);
        double compute(double x);
        template<typename T>
        T generic_compute(T x);
    };
}

int main() {
    std::cout << "Math::Calculator::compute(int)     → _ZN4Math9Calculator7computeEi\n";
    std::cout << "Math::Calculator::compute(double)   → _ZN4Math9Calculator7computeEd\n";
    std::cout << "Math::Calculator::generic_compute<int> → _ZN4Math9Calculator15generic_computeIiEET_S1_\n";
    return 0;
}
```

> 模板的修饰更为复杂，包含模板参数编码。Itanium ABI 规范长达数十页，完整规则见 [Itanium C++ ABI](https://itanium-cxx-abi.github.io/cxx-abi/abi.html#mangling)。

***

### 4. MSVC ABI 修饰规则

Microsoft Visual C++ 使用完全不同的修饰方案，以 `?` 开头，编码方式与 Itanium ABI 不兼容。

```
? <函数名> <修饰Y> <类名> <命名空间> @ <参数编码> @ <调用约定> <异常规范>
```

| 编码 | 含义 | 编码 | 含义 |
|------|------|------|------|
| `H` | int | `N` | double |
| `M` | float | `D` | char |
| `J` | long | `_J` | long long |
| `_N` | bool | `X` | void |
| `P` | 指针 | `A` | 引用 |

**MSVC 示例**：

| 修饰后符号 | 原始签名 |
|-----------|---------|
| `?add@@YAHHH@Z` | `int __cdecl add(int, int)` |
| `?add@@YANNN@Z` | `double __cdecl add(double, double)` |
| `?foo@NS@@YAXH@Z` | `void __cdecl NS::foo(int)` |
| `?foo@MyClass@@QAEXH@Z` | `void __thiscall MyClass::foo(int)` |

```cpp
#include <iostream>

int add(int a, int b) { return a + b; }
double add(double a, double b) { return a + b; }

namespace NS {
    void foo(int x) { (void)x; }
}

int main() {
#ifdef _MSC_VER
    std::cout << "MSVC 修饰示例:\n";
    std::cout << "add(int,int)     → ?add@@YAHHH@Z\n";
    std::cout << "add(double,double) → ?add@@YANNN@Z\n";
    std::cout << "NS::foo(int)     → ?foo@NS@@YAXH@Z\n";
#else
    std::cout << "GCC/Clang 修饰示例:\n";
    std::cout << "add(int,int)     → _Z3addii\n";
    std::cout << "add(double,double) → _Z3adddd\n";
    std::cout << "NS::foo(int)     → _ZN2NS3fooEi\n";
#endif
    return 0;
}
```

> **关键差异**：Itanium ABI 和 MSVC ABI 的修饰结果完全不同，这是 C++ DLL/SO 跨编译器不兼容的根本原因之一。

***

### 5. 反修饰工具

将修饰后的符号名还原为可读的函数签名，称为反修饰（Demangling）。

| 工具 | 平台 | 用法 |
|------|------|------|
| `c++filt` | Linux/macOS | `c++filt _Z3addii` |
| `__cxa_demangle` | GCC/Clang (运行时) | C API，程序内调用 |
| `UnDecorateSymbolName` | Windows (运行时) | DbgHelp API |
| `dumpbin /symbols` | MSVC | 查看目标文件符号 |
| `nm --demangle` | Linux/macOS | 查看 .o/.so 符号 |
| `objdump -C` | Linux/macOS | 反汇编时自动 demangle |

```cpp
#include <iostream>
#include <cstdlib>
#include <cstring>

#ifdef __GNUC__
#include <cxxabi.h>
#endif

int sample_func(int x, double y) { return static_cast<int>(x + y); }

int main() {
#ifdef __GNUC__
    const char* mangled = "_Z11sample_funcid";
    int status = 0;
    char* demangled = abi::__cxa_demangle(mangled, nullptr, nullptr, &status);
    if (status == 0) {
        std::cout << "修饰名: " << mangled << "\n";
        std::cout << "还原后: " << demangled << "\n";
    } else {
        std::cout << "反修饰失败, status=" << status << "\n";
    }
    free(demangled);
#elif defined(_MSC_VER)
    std::cout << "MSVC 平台请使用 UnDecorateSymbolName API\n";
    std::cout << "或使用 dumpbin /symbols xxx.obj 查看\n";
#else
    std::cout << "当前平台无运行时反修饰支持\n";
#endif
    return 0;
}
```

**命令行反修饰**：

```bash
# Linux/macOS: 使用 c++filt
echo "_Z3addii" | c++filt
# 输出: add(int, int)

# Linux/macOS: 使用 nm
nm --demangle my_program

# Linux/macOS: 使用 objdump
objdump -C -d my_program

# Windows: 使用 dumpbin
dumpbin /symbols my.obj
```

> `c++filt` 支持批量输入，管道操作非常方便：`nm my_program | c++filt`。

***

### 6. extern "C"——禁用名称修饰

当 C++ 代码需要被 C 代码调用，或需要保持符号名不变（如动态库导出 API）时，使用 `extern "C"` 禁用名称修饰。

```cpp
#ifdef __cplusplus
extern "C" {
#endif

int c_style_add(int a, int b) {
    return a + b;
}

void c_style_process(const char* data) {
    (void)data;
}

#ifdef __cplusplus
}
#endif
```

| 声明方式 | 修饰后符号 | 说明 |
|---------|-----------|------|
| `int add(int, int)` | `_Z3addii` (GCC) | C++ 默认修饰 |
| `extern "C" int add(int, int)` | `add` | 不修饰，C 链接 |
| `extern "C" { int add(int, int); }` | `add` | 块语法 |

**典型使用场景——C/C++ 混合编程**：

```cpp
// math_lib.h - 头文件
#ifdef __cplusplus
extern "C" {
#endif

int math_add(int a, int b);
double math_sqrt(double x);

#ifdef __cplusplus
}
#endif

// math_lib.cpp - 实现文件
#include "math_lib.h"

extern "C" int math_add(int a, int b) {
    return a + b;
}

extern "C" double math_sqrt(double x) {
    return __builtin_sqrt(x);
}
```

> `extern "C"` 仅影响链接规范（名称修饰方式），不影响函数本身的 C++ 语义——函数体仍可使用 C++ 特性。

**限制**：`extern "C"` 函数不能重载，不能是类成员，不能使用 C++ 特有的调用约定。

***

### 7. 跨编译器不兼容问题

由于 Itanium ABI 和 MSVC ABI 的修饰规则完全不同，同一个 C++ 函数在不同编译器下产生不同的修饰名，导致：

1. **无法直接链接**：GCC 编译的 `.o` 无法与 MSVC 编译的 `.obj` 链接
2. **DLL/SO 导出符号不兼容**：C++ 类的导出符号名不同
3. **ABI 不兼容**：不仅是名称修饰，还包括虚表布局、异常处理、内存对齐等

| 方面 | Itanium ABI (GCC/Clang) | MSVC ABI |
|------|------------------------|----------|
| 修饰前缀 | `_Z` | `?` |
| 嵌套名称 | `N...E` | `@` 分隔 |
| 参数编码 | 小写字母 | 大写字母 |
| 虚表布局 | RTTI 在虚表之前 | RTTI 在虚表之后 |
| 异常处理 | DWARF/.eh_frame | SEH |
| 名称还原 | `c++filt` | `dumpbin` |

```cpp
#include <iostream>

class ExportedAPI {
public:
    virtual void process(int data) = 0;
    virtual ~ExportedAPI() = default;
};

extern "C" ExportedAPI* create_api();

int main() {
    std::cout << "跨编译器 C++ DLL 不兼容的原因:\n";
    std::cout << "1. 名称修饰不同 → 符号找不到\n";
    std::cout << "2. 虚表布局不同 → 虚函数调用崩溃\n";
    std::cout << "3. 异常处理不同 → 跨 DLL 抛异常崩溃\n";
    std::cout << "4. 内存分配器不同 → 跨 DLL 释放崩溃\n";
    std::cout << "\n解决方案: 使用 extern \"C\" 导出纯 C 接口\n";
    return 0;
}
```

> 跨编译器共享 C++ 代码的唯一可靠方式是：导出 `extern "C"` 纯 C 接口，内部用 C++ 实现。

***

### 8. nm/objdump/dumpbin 实战

**Linux/macOS 下查看符号**：

```bash
# 编译生成目标文件
g++ -c example.cpp -o example.o

# 查看修饰后的符号
nm example.o

# 查看反修饰后的符号
nm --demangle example.o

# 更详细的信息
nm -C example.o

# objdump 查看符号表
objdump -t example.o

# objdump 反汇编时自动 demangle
objdump -C -d example.o

# 查看动态库导出符号
nm -D libexample.so | c++filt

# readelf 查看 ELF 符号表
readelf -s example.o | c++filt
```

**Windows 下查看符号**：

```bash
# 查看 OBJ 文件符号
dumpbin /symbols example.obj

# 查看 DLL 导出函数
dumpbin /exports example.dll

# 查看 LIB 导出
dumpbin /headers example.lib

# 使用 /EXPORT 链接器选项控制导出名
# link /DLL /EXPORT:??foo@@YAHH@Z example.obj
```

```cpp
#include <iostream>

void overloaded(int x) { (void)x; }
void overloaded(double x) { (void)x; }
void overloaded(int x, double y) { (void)x; (void)y; }

namespace Inner {
    void helper() {}
}

class Widget {
public:
    void draw() {}
    void resize(int w, int h) { (void)w; (void)h; }
};

int main() {
    std::cout << "编译后使用以下命令查看修饰名:\n";
#ifdef _MSC_VER
    std::cout << "  dumpbin /symbols example.obj\n";
#else
    std::cout << "  nm example.o\n";
    std::cout << "  nm --demangle example.o\n";
#endif
    return 0;
}
```

> 链接错误中出现的 `_ZN...` 或 `??...` 就是修饰名，用对应工具反修饰即可定位问题。

***

### 9. 对 DLL/SO API 设计的影响

名称修饰对动态库 API 设计有重大影响，是 C++ 动态库跨平台/跨编译器使用的核心障碍。

| API 设计方式 | 跨编译器 | 跨语言 | 复杂度 | 适用场景 |
|-------------|---------|--------|--------|---------|
| C++ 类直接导出 | ❌ | ❌ | 低 | 单一编译器 |
| `extern "C"` 纯 C 接口 | ✅ | ✅ | 中 | 通用库 |
| `extern "C"` + Opaque Pointer | ✅ | ✅ | 中高 | 稳定 ABI |
| COM 接口 | ✅ | ✅ | 高 | Windows 组件化 |

```cpp
#ifdef MYLIB_EXPORTS
    #ifdef _MSC_VER
        #define MYLIB_API __declspec(dllexport)
    #else
        #define MYLIB_API __attribute__((visibility("default")))
    #endif
#else
    #ifdef _MSC_VER
        #define MYLIB_API __declspec(dllimport)
    #else
        #define MYLIB_API
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

MYLIB_API void* mylib_create();
MYLIB_API void mylib_destroy(void* handle);
MYLIB_API int mylib_process(void* handle, int data);

#ifdef __cplusplus
}
#endif

// 内部实现使用 C++
class MyLibImpl {
public:
    int process(int data) { return data * 2; }
};

extern "C" {

MYLIB_API void* mylib_create() {
    return new MyLibImpl();
}

MYLIB_API void mylib_destroy(void* handle) {
    delete static_cast<MyLibImpl*>(handle);
}

MYLIB_API int mylib_process(void* handle, int data) {
    return static_cast<MyLibImpl*>(handle)->process(data);
}

}
```

> Opaque Pointer（不透明指针）模式是 C++ 库导出稳定 ABI 的标准做法：头文件只暴露 `void*` 和 C 函数，实现细节隐藏在 `.cpp` 内部。

**Windows 平台特殊注意事项**：

```cpp
// Windows DLL 导出 C++ 类时，修饰名会被导出
// 不同 MSVC 版本的修饰名可能不同
// 使用 .def 文件可以强制指定导出名

// mylib.def:
// EXPORTS
//     mylib_create
//     mylib_destroy
//     mylib_process
```

***

### 10. 特殊情况的修饰

| 特殊情况 | 修饰示例 (Itanium) | 说明 |
|---------|-------------------|------|
| 构造函数 | `_ZN1AC1Ev` | `C1` = 完整构造 |
| 析构函数 | `_ZN1AD1Ev` | `D1` = 完整析构 |
| 虚析构函数 | `_ZN1AD0Ev` | `D0` = 删除析构 |
| 运算符重载 | `_ZN1AplERKS_` | `operator+` |
| 类型转换运算符 | `_ZN1AcviEv` | `operator int()` |
| 模板特化 | `_ZN1A7computeIiEET_S1_` | 含模板参数 |
| lambda | `_ZZ4mainENK3$_0clEi` | 匿名类型 |
| 全局运算符 | `_ZlsRSoRK1A` | `operator<<` |

```cpp
#include <iostream>

class Complex {
    double real_, imag_;
public:
    Complex(double r, double i) : real_(r), imag_(i) {}

    Complex operator+(const Complex& other) const {
        return Complex(real_ + other.real_, imag_ + other.imag_);
    }

    operator double() const { return real_; }

    friend std::ostream& operator<<(std::ostream& os, const Complex& c) {
        return os << c.real_ << "+" << c.imag_ << "i";
    }
};

int main() {
    Complex a(1.0, 2.0);
    Complex b(3.0, 4.0);
    Complex c = a + b;
    std::cout << c << "\n";
    std::cout << "隐式转换: " << static_cast<double>(c) << "\n";

    std::cout << "\n修饰名示例:\n";
    std::cout << "Complex::Complex(double,double) → _ZN7ComplexC2Edd\n";
    std::cout << "Complex::operator+             → _ZNK7ComplexplERKS_\n";
    std::cout << "Complex::operator double()     → _ZN7ComplexcvdEv\n";
    return 0;
}
```

> lambda 的修饰名包含 `$` 或 `._0` 等匿名标识符，不同编译器处理方式不同，不可依赖。

***

### 11. 极简总结

| 概念 | 核心要点 |
|------|---------|
| 名称修饰 | 编译器将函数签名编码为唯一符号名，支持重载和命名空间 |
| Itanium ABI | GCC/Clang 使用，`_Z` 前缀，`N...E` 嵌套，小写参数编码 |
| MSVC ABI | MSVC 使用，`?` 前缀，`@` 分隔，大写参数编码 |
| 反修饰 | `c++filt` / `__cxa_demangle` / `dumpbin` 还原可读签名 |
| `extern "C"` | 禁用修饰，保持原始符号名，C/C++ 混合编程必需 |
| 跨编译器不兼容 | 修饰规则不同 + ABI 差异，C++ DLL 无法跨编译器使用 |
| 符号查看 | `nm` / `objdump` (Linux) / `dumpbin` (Windows) |
| DLL API 设计 | 使用 `extern "C"` + Opaque Pointer 导出稳定 ABI |
| 特殊修饰 | 构造/析构/运算符/模板/lambda 各有特殊编码规则 |

**核心原则**：
- 暴露给外部（DLL/SO）的接口必须用 `extern "C"` 禁用修饰
- 链接错误中的修饰名用工具反修饰即可定位
- 永远不要依赖修饰名的具体格式——它是编译器内部实现细节

***

### 相关阅读

- [什么是符号表Symbol-Table](./11-什么是符号表Symbol-Table.md)
- [extern-C与动态库导出](../01-基础概念/09-extern-C与动态库导出.md)