# 什么是调用约定 Calling Convention
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)、[静态库](../../01-C语言/18-静态库.md)、[动态库](../../01-C语言/19-动态库与共享库.md)、[CMake](../../01-C语言/22-CMake构建系统.md)

> "函数怎么调用、栈怎么清理、参数怎么传——这些规则就是调用约定。" —— ABI 的基石

***

### 1. 精髓速览

调用约定（Calling Convention）规定了函数调用时参数如何传递、返回值如何回传、栈由谁清理以及寄存器如何使用的底层规则，是连接编译器、操作系统和硬件的 ABI 契约。

***

### 2. 什么是调用约定

当编译器将函数调用翻译为机器码时，必须回答以下问题：

| 问题 | 选项 |
|------|------|
| 参数放在哪里？ | 栈、寄存器、混合 |
| 参数压栈顺序？ | 从左到右、从右到左 |
| 谁清理栈？ | 调用者（caller）、被调用者（callee） |
| 返回值放在哪里？ | 寄存器、栈、内存 |
| 哪些寄存器需要保存？ | caller-saved、callee-saved |
| 函数名如何修饰？ | 下划线前缀、@后缀、C++ name mangling |

调用约定就是这些规则的统称，不同的编译器、操作系统、CPU 架构可能采用不同的约定。

```cpp
#include <iostream>

int __cdecl add_cdecl(int a, int b) {
    return a + b;
}

int __stdcall add_stdcall(int a, int b) {
    return a + b;
}

int __fastcall add_fastcall(int a, int b) {
    return a + b;
}

int main() {
    std::cout << add_cdecl(1, 2) << "\n";
    std::cout << add_stdcall(3, 4) << "\n";
    std::cout << add_fastcall(5, 6) << "\n";
}
```

> 平台说明：`__cdecl`、`__stdcall`、`__fastcall` 是 MSVC 在 x86 上的关键字。GCC/Clang 使用 `__attribute__((cdecl))` 等属性语法。x86-64 和 ARM64 有统一的 ABI，这些关键字基本不再需要。

***

### 3. x86 上的主要调用约定

32 位 x86 是调用约定最复杂的平台，存在多种互不兼容的约定：

**__cdecl（C 声明约定）**

```cpp
int __cdecl func(int a, int b, int c);
```

- 参数从右到左压栈
- 调用者清理栈
- 支持可变参数
- C/C++ 默认约定

**__stdcall（标准约定 / WinAPI 约定）**

```cpp
int __stdcall func(int a, int b);
```

- 参数从右到左压栈
- 被调用者清理栈
- 不支持可变参数
- Windows API 默认约定

**__fastcall（快速约定）**

```cpp
int __fastcall func(int a, int b, int c);
```

- 前两个参数通过 ECX、EDX 传递，其余压栈
- 被调用者清理栈
- 不支持可变参数
- MSVC 特有，GCC 的 fastcall 使用 ECX、EDX

**__thiscall（成员函数约定）**

```cpp
class MyClass {
public:
    void __thiscall method(int x);
};
```

- this 指针通过 ECX 传递
- 参数从右到左压栈
- 被调用者清理栈
- MSVC 中 C++ 成员函数默认约定

对比表：

| 约定 | 参数传递 | 栈清理 | 可变参数 | 名称修饰（MSVC） | this 指针 |
|------|---------|--------|---------|-----------------|----------|
| `__cdecl` | 栈（右→左） | 调用者 | ✅ | `_func` | 栈（首个参数） |
| `__stdcall` | 栈（右→左） | 被调用者 | ❌ | `_func@8` | 栈（首个参数） |
| `__fastcall` | ECX,EDX + 栈 | 被调用者 | ❌ | `@func@8` | ECX（首个参数） |
| `__thiscall` | ECX(this) + 栈 | 被调用者 | ❌ | — | ECX |

```cpp
#include <iostream>

int __cdecl sum_cdecl(int a, int b) {
    return a + b;
}

int __stdcall sum_stdcall(int a, int b) {
    return a + b;
}

class Calculator {
public:
    int value;
    int __thiscall add(int x) {
        return value + x;
    }
};

int main() {
    int r1 = sum_cdecl(1, 2);
    int r2 = sum_stdcall(3, 4);

    Calculator calc{10};
    int r3 = calc.add(5);

    std::cout << r1 << " " << r2 << " " << r3 << "\n";
}
```

***

### 4. x86-64 ABI：System V vs Windows x64

64 位时代大大简化了调用约定，但 Linux/macOS 和 Windows 仍然不同：

**System V AMD64 ABI（Linux、macOS、BSD）**

| 参数位置 | 整数/指针 | 浮点 |
|---------|----------|------|
| 第 1 个 | RDI | XMM0 |
| 第 2 个 | RSI | XMM1 |
| 第 3 个 | RDX | XMM2 |
| 第 4 个 | RCX | XMM3 |
| 第 5 个 | R8 | XMM4 |
| 第 6 个 | R9 | XMM5 |
| 第 7+ | 栈 | 栈 |

**Microsoft x64 ABI（Windows）**

| 参数位置 | 整数/指针 | 浮点 |
|---------|----------|------|
| 第 1 个 | RCX | XMM0 |
| 第 2 个 | RDX | XMM1 |
| 第 3 个 | R8 | XMM2 |
| 第 4 个 | R9 | XMM3 |
| 第 5+ | 栈 | 栈 |

两者关键区别：

| 区别 | System V AMD64 | Microsoft x64 |
|------|---------------|---------------|
| 整数寄存器数量 | 6 个 | 4 个 |
| 浮点寄存器数量 | 8 个 | 4 个 |
| 栈空间预留 | 128 字节 red zone | 32 字节 shadow space |
| 参数传递 | 按类型独立分配寄存器 | 整数和浮点共享槽位 |
| 返回值 | RAX（整数）/ XMM0/XMM1（浮点） | RAX（整数）/ XMM0（浮点） |

```cpp
#include <iostream>

int many_args(int a, int b, int c, int d, int e, int f, int g) {
    return a + b + c + d + e + f + g;
}

double float_args(double a, double b, double c, double d, double e, double f, double g, double h) {
    return a + b + c + d + e + f + g + h;
}

int main() {
    std::cout << many_args(1, 2, 3, 4, 5, 6, 7) << "\n";
    std::cout << float_args(1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0) << "\n";
}
```

> 平台说明：在 Windows x64 上，`many_args` 的第 5 个及之后的参数通过栈传递；在 Linux x64 上，第 7 个及之后通过栈传递。`float_args` 在 Linux 上前 8 个参数全用 XMM 寄存器，Windows 上前 4 个用 XMM，之后用栈。

***

### 5. ARM64（AArch64）调用约定

ARM64 使用 AAPCS64（Procedure Call Standard for the Arm 64-bit Architecture）：

| 参数位置 | 整数/指针 | 浮点/向量 |
|---------|----------|----------|
| 第 1-8 个 | X0-X7 | V0-V7 |
| 第 9+ | 栈 | 栈 |

| 返回值 | 整数 | 浮点/向量 |
|-------|------|----------|
| 单个 | X0 | V0 |
| 大型 | X8 指向内存 | X8 指向内存 |

ARM64 的特点：

| 特性 | 说明 |
|------|------|
| 寄存器数量 | 8 个整数 + 8 个浮点参数寄存器 |
| 栈对齐 | 16 字节对齐 |
| 栈清理 | 调用者分配，被调用者在函数入口分配所需空间 |
| red zone | 无（不像 x86-64 System V） |
| 名称修饰 | 不依赖调用约定区分（统一 ABI） |

```cpp
#include <iostream>

struct LargeStruct {
    int a, b, c, d, e;
};

LargeStruct create_large() {
    return LargeStruct{1, 2, 3, 4, 5};
}

void process_large(LargeStruct s) {
    std::cout << s.a << " " << s.b << " " << s.c << " " << s.d << " " << s.e << "\n";
}

int main() {
    auto s = create_large();
    process_large(s);
}
```

> 平台说明：ARM64 上，大于 16 字节的 `LargeStruct` 通过 X8 指向的隐藏指针返回，调用者分配内存。x86-64 上类似，大型结构体也通过隐藏指针返回。

***

### 6. 返回值处理

返回值的传递方式取决于类型大小和复杂性：

| 返回值类型 | x86-64 System V | x86-64 Windows | ARM64 |
|-----------|----------------|----------------|-------|
| 整数（≤8字节） | RAX | RAX | X0 |
| 浮点（≤8字节） | XMM0 | XMM0 | V0 |
| 双精度浮点 | XMM0 | XMM0 | V0 |
| 小结构体（≤16B） | RAX+RDX | RAX | X0+X1 |
| 中等结构体（17-32B） | RAX+RDX | 隐藏指针 | X0-X3 |
| 大结构体（>32B） | 隐藏指针 | 隐藏指针 | 隐藏指针(X8) |

```cpp
#include <iostream>
#include <cstdint>

struct Small {
    int x;
    int y;
};

struct Medium {
    int a, b, c, d;
};

struct Large {
    int data[10];
};

Small ret_small() { return {1, 2}; }
Medium ret_medium() { return {1, 2, 3, 4}; }
Large ret_large() { return {{1,2,3,4,5,6,7,8,9,10}}; }

int main() {
    auto s = ret_small();
    auto m = ret_medium();
    auto l = ret_large();

    std::cout << "Small: " << s.x << "," << s.y << "\n";
    std::cout << "Medium: " << m.a << "," << m.b << "," << m.c << "," << m.d << "\n";
    std::cout << "Large: " << l.data[0] << "," << l.data[9] << "\n";
}
```

隐藏指针机制详解：

```cpp
struct Big {
    double data[100];
};

Big create_big() {
    Big b{};
    b.data[0] = 42.0;
    return b;
}

void caller() {
    Big result = create_big();
}
```

编译器等价转换：

```cpp
void create_big(Big* hidden_ptr) {
    Big b{};
    b.data[0] = 42.0;
    *hidden_ptr = b;
}

void caller() {
    Big tmp;
    create_big(&tmp);
}
```

***

### 7. 栈帧结构

函数调用时的栈帧布局：

```
高地址
┌─────────────────────┐
│   调用者的栈帧       │
├─────────────────────┤
│   参数 N (最后压栈)   │  ← x86 从右到左压栈
│   参数 N-1           │
│   ...               │
│   参数 1 (最先压栈)   │
├─────────────────────┤
│   返回地址           │  ← call 指令自动压入
├─────────────────────┤
│   保存的 RBP/EBP     │  ← push rbp
├─────────────────────┤
│   局部变量           │
│   ...               │
│   临时空间           │
├─────────────────────┤  ← RSP/ESP 指向
│   对齐填充           │
└─────────────────────┘
低地址
```

x86-64 System V 的 red zone：

```
┌─────────────────────┐
│   返回地址           │
├─────────────────────┤
│   保存的 RBP         │
├─────────────────────┤  ← RBP
│   局部变量           │
├─────────────────────┤  ← RSP
│   Red Zone (128B)    │  ← 叶子函数可用，无需修改 RSP
└─────────────────────┘
```

| 平台 | 特殊区域 | 大小 | 说明 |
|------|---------|------|------|
| x86-64 System V | Red Zone | 128 字节 | 叶子函数可直接使用，信号安全 |
| x86-64 Windows | Shadow Space | 32 字节 | 为寄存器参数预留的栈空间 |
| ARM64 | 无特殊 | — | 函数入口分配全部所需栈空间 |

```cpp
#include <iostream>

int leaf_func(int x) {
    int temp = x * 2;
    return temp + 1;
}

int non_leaf_func(int x) {
    std::cout << x << "\n";
    return x + 1;
}

int main() {
    leaf_func(42);
    non_leaf_func(42);
}
```

***

### 8. Caller-saved 与 Callee-saved 寄存器

寄存器保存规则决定了哪些寄存器在函数调用前后需要保存：

**x86-64 System V：**

| 类别 | 寄存器 | 说明 |
|------|--------|------|
| Caller-saved | RAX, RCX, RDX, RSI, RDI, R8-R11 | 调用前保存（如需要），被调用者可自由修改 |
| Callee-saved | RBX, RBP, R12-R15 | 被调用者必须保存并恢复 |
| 特殊 | RSP | 栈指针，始终有效 |

**x86-64 Windows：**

| 类别 | 寄存器 | 说明 |
|------|--------|------|
| Caller-saved | RAX, RCX, RDX, R8-R11 | 调用前保存 |
| Callee-saved | RBX, RBP, RDI, RSI, R12-R15 | 被调用者保存 |
| 特殊 | RSP | 栈指针 |

**ARM64：**

| 类别 | 寄存器 | 说明 |
|------|--------|------|
| Caller-saved | X0-X18 | 参数/临时（部分有特殊用途） |
| Callee-saved | X19-X28 | 必须保存恢复 |
| 特殊 | X29(FP), X30(LR), SP | 帧指针/链接寄存器/栈指针 |

```cpp
#include <iostream>

int compute(int a, int b) {
    int result = a * b;
    return result;
}

int main() {
    int x = 10;
    int y = compute(x, 20);
    int z = x + y;
    std::cout << z << "\n";
}
```

> 平台说明：在 `compute` 调用后，`x` 的值可能被覆盖（如果存在 caller-saved 寄存器中）。编译器要么将 `x` 保存到 callee-saved 寄存器，要么在调用前将其存入栈。

***

### 9. 可变参数函数的调用约定

可变参数函数（如 `printf`）对调用约定有特殊要求：

```cpp
#include <cstdio>
#include <cstdarg>

int my_sum(int count, ...) {
    va_list args;
    va_start(args, count);
    int total = 0;
    for (int i = 0; i < count; ++i) {
        total += va_arg(args, int);
    }
    va_end(args);
    return total;
}

int main() {
    std::printf("sum=%d\n", my_sum(3, 10, 20, 30));
    std::printf("sum=%d\n", my_sum(5, 1, 2, 3, 4, 5));
}
```

各平台可变参数处理：

| 平台 | 可变参数传递 | 说明 |
|------|-----------|------|
| x86 cdecl | 全部压栈 | 从右到左，调用者清理 |
| x86 stdcall | ❌ 不支持 | 必须用 cdecl |
| x86-64 System V | 寄存器+栈 | 寄存器溢出区必须对齐 |
| x86-64 Windows | 寄存器+栈 | 必须在栈上预留溢出空间 |
| ARM64 | 寄存器+栈 | va_list 是结构体 |

x86-64 可变参数的特殊要求：

```cpp
#include <cstdio>

int main() {
    std::printf("%d %f %s %c\n", 42, 3.14, "hello", 'A');
}
```

> 平台说明：x86-64 System V 要求可变参数函数调用时，将 AL 寄存器设为使用的浮点寄存器数量。Windows x64 要求为前 4 个寄存器参数预留 32 字节 shadow space。

***

### 10. FFI 与跨语言调用

调用约定是 FFI（Foreign Function Interface）的基础：

**C 风格声明防止 C++ 名称修饰：**

```cpp
#include <iostream>

extern "C" {
    int c_style_add(int a, int b) {
        return a + b;
    }

    void c_style_print(const char* msg) {
        std::cout << msg << "\n";
    }
}

int main() {
    std::cout << c_style_add(1, 2) << "\n";
    c_style_print("Hello from C-style function");
}
```

**名称修饰对比：**

| 约定/编译器 | `int add(int, int)` 的修饰名 |
|------------|---------------------------|
| C（MSVC x86） | `_add` |
| C（GCC x86） | `add` |
| C++（MSVC x86） | `?add@@YAHHH@Z` |
| C++（GCC x86） | `_Z3addii` |
| C/C++（x86-64） | 64 位下 C 修饰规则同上，C++ 更复杂 |
| `__stdcall`（MSVC） | `_add@8` |
| `__fastcall`（MSVC） | `@add@8` |

**Python 调用 C 函数示例：**

```cpp
extern "C" int multiply(int a, int b) {
    return a * b;
}
```

```python
from ctypes import CDLL, c_int

lib = CDLL("./mylib.dll")
lib.multiply.argtypes = [c_int, c_int]
lib.multiply.restype = c_int
result = lib.multiply(3, 4)
print(result)
```

**__attribute__ 语法（GCC/Clang）：**

```cpp
int __attribute__((cdecl)) add_cdecl(int a, int b);
int __attribute__((stdcall)) add_stdcall(int a, int b);
void __attribute__((fastcall)) add_fastcall(int a, int b);

void __attribute__((sysv_abi)) linux_func(int x);
void __attribute__((ms_abi)) windows_func(int x);
```

> 平台说明：`__attribute__((sysv_abi))` 和 `__attribute__((ms_abi))` 允许在 Windows 和 Linux 之间交叉指定调用约定，用于 FFI 场景。

**Rust FFI 调用 C：**

```cpp
extern "C" int add(int a, int b);
```

```rust
extern "C" {
    fn add(a: i32, b: i32) -> i32;
}

fn main() {
    unsafe {
        let result = add(1, 2);
        println!("{}", result);
    }
}
```

***

### 11. 极简总结

| 约定 | 平台 | 参数传递 | 栈清理 | 可变参数 |
|------|------|---------|--------|---------|
| `__cdecl` | x86 Windows | 栈（右→左） | 调用者 | ✅ |
| `__stdcall` | x86 Windows | 栈（右→左） | 被调用者 | ❌ |
| `__fastcall` | x86 Windows | ECX,EDX+栈 | 被调用者 | ❌ |
| `__thiscall` | x86 MSVC | ECX(this)+栈 | 被调用者 | ❌ |
| System V AMD64 | Linux/macOS x64 | 6整数+8浮点寄存器+栈 | 调用者 | ✅ |
| Microsoft x64 | Windows x64 | 4整数+4浮点寄存器+栈 | 调用者 | ✅ |
| AAPCS64 | ARM64 | 8整数+8浮点寄存器+栈 | 调用者 | ✅ |

核心要点：

- x86（32位）调用约定最复杂，x86-64 和 ARM64 基本统一
- Windows x64 和 Linux x64 的寄存器分配不同，跨平台 FFI 需注意
- `extern "C"` 消除 C++ 名称修饰，是 FFI 的基础
- 大型返回值通过隐藏指针传递，调用者分配内存
- 可变参数函数只能用调用者清理栈的约定（cdecl/x64/ARM64）
- `__attribute__((sysv_abi/ms_abi))` 可在 GCC/Clang 中跨平台指定调用约定
- 现代代码中 64 位平台无需手动指定调用约定，ABI 已统一

***

### 相关阅读

- [什么是ABI兼容性](./02-什么是ABI兼容性.md)
- [什么是Pimpl惯用法](../04-CPP核心特性/17-什么是Pimpl惯用法.md)