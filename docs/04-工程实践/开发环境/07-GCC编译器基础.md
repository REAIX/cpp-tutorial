# GCC/G++ 编译器深度使用指南

> **前置阅读**：如果你还没有安装GCC/G++，请先阅读 [FAQ-138：开发环境配置详解](../03-问题解答/01-基础概念/28-开发环境配置.md) 完成编译器安装。本文档假设你已经安装好了GCC/G++，需要深入学习编译参数和使用技巧。

> **相关教程**：编译器的基础概念见 [编译与链接](../01-C语言/17-编译与链接.md)，编译器入门用法见 [GCC/G++编译器详解](../01-C语言/21-GCC与G++编译器.md)。

## 1. GCC与G++的区别

### 1. GCC是编译器集合

GCC（GNU Compiler Collection）是GNU编译器集合，并非单一的C编译器。它包含了以下前端：

- **gcc**：C语言编译器驱动
- **g++**：C++编译器驱动
- **gfortran**：Fortran编译器
- **gdc**：D语言编译器
- **gcj**：Java编译器（已弃用）

GCC的核心是共享的后端优化器和代码生成器，不同的前端将源代码转换为统一的中间表示（GIMPLE/RTL），再由后端进行优化和目标代码生成。

### 2. gcc vs g++ 编译C/C++的区别

gcc和g++最关键的区别在于**默认链接的库不同**：

```bash
# 用gcc编译C++程序——会报错，缺少C++标准库链接
gcc main.cpp -o main
# 报错：undefined reference to `std::cout'

# 手动指定链接C++标准库后可以编译
gcc main.cpp -o main -lstdc++

# 用g++编译C++程序——自动链接C++标准库
g++ main.cpp -o main
```

两者的核心差异总结：

| 特性 | gcc | g++ |
|------|-----|-----|
| 编译.c文件 | 当作C语言处理 | 当作C语言处理 |
| 编译.cpp文件 | 当作C++语言处理，但不自动链接C++标准库（需加-lstdc++） | 当作C++语言处理 |
| 默认链接库 | libc | libc + libstdc++ |
| 预定义宏 | `__STDC__` 等 | `__cplusplus`、`__GNUG__` 等 |
| 链接阶段 | 不自动链接C++运行时 | 自动链接C++运行时 |

### 3. 为什么C++程序要用g++而不是gcc

```cpp
// hello.cpp
#include <iostream>

int main() {
    std::cout << "Hello, C++!" << std::endl;
    return 0;
}
```

```bash
# 使用gcc编译——失败
gcc hello.cpp -o hello
# 输出类似：
# /tmp/ccXXXXXX.o: In function `main':
# hello.cpp:(.text+0x10): undefined reference to `std::cout'
# hello.cpp:(.text+0x15): undefined reference to `std::basic_ostream<char, std::char_traits<char> >& std::operator<< <std::char_traits<char> >(std::basic_ostream<char, std::char_traits<char> >&, char const*)'
# ...

# 使用gcc加-lstdc++——成功
gcc hello.cpp -o hello -lstdc++

# 使用g++编译——成功，自动处理一切
g++ hello.cpp -o hello
```

原因分析：
- g++在链接阶段自动添加`-lstdc++`（C++标准库）和C++运行时支持库
- g++会自动处理C++特有的初始化代码（全局对象构造/析构）
- g++预定义`__cplusplus`宏，影响头文件中的条件编译

### 4. 版本查看

```bash
# 查看GCC版本
gcc --version
# 输出示例：
# gcc (Ubuntu 13.2.0-23ubuntu4) 13.2.0
# Copyright (C) 2023 Free Software Foundation, Inc.

# 查看G++版本
g++ --version

# 查看详细版本信息（包含配置、线程模型等）
gcc -v
# 输出示例：
# Using built-in specs.
# COLLECT_GCC=gcc
# COLLECT_LTO_WRAPPER=/usr/lib/gcc/x86_64-linux-gnu/13/lto-wrapper
# OFFLOAD_TARGET_NAMES=nvptx-none:amdgcn-amdhsa
# OFFLOAD_TARGET_DEFAULT=1
# Target: x86_64-linux-gnu
# Configured with: ../src/configure -v ...
# Thread model: posix
# Supported LTO compression algorithms: zlib zstd
# gcc version 13.2.0 (Ubuntu 13.2.0-23ubuntu4)

# 只看版本号
gcc -dumpversion
# 输出：13

# 查看完整的版本字符串
gcc -dumpversion --verbose
```

---

## 2. 编译四阶段详解

GCC/G++将源代码转换为可执行文件需要经过四个阶段：预处理、编译、汇编、链接。

### 1. 预处理（Preprocessing，-E）

预处理阶段处理所有以`#`开头的预处理指令：

- 宏展开（`#define`）
- 头文件包含（`#include`）
- 条件编译（`#ifdef`、`#ifndef`、`#if`）
- 删除注释
- 处理`#pragma`

```cpp
// preproc_demo.cpp
#include <iostream>
#define MAX_SIZE 100
#define SQUARE(x) ((x) * (x))

#ifdef DEBUG
    #define LOG(msg) std::cout << "[DEBUG] " << msg << std::endl
#else
    #define LOG(msg)
#endif

int main() {
    int arr[MAX_SIZE];
    int val = SQUARE(5);
    LOG("程序启动");
    return 0;
}
```

```bash
# 只执行预处理，输出到标准输出
g++ -E preproc_demo.cpp

# 预处理输出到文件
g++ -E preproc_demo.cpp -o preproc_demo.ii

# 保留注释（默认预处理会删除注释）
g++ -E -C preproc_demo.cpp -o preproc_demo_with_comments.ii

# 查看预处理结果（部分）
head -50 preproc_demo.ii
# 输出类似：
# # 1 "preproc_demo.cpp"
# # 1 "<built-in>"
# # 1 "<command-line>"
# # 1 "/usr/include/stdc-predef.h" 1 3 4
# # 1 "<command-line>" 2
# # 1 "preproc_demo.cpp"
# # 1 "/usr/include/c++/13/iostream" 1 3
# # 36 "/usr/include/c++/13/iostream" 3
# ...（大量头文件展开内容）
# int main() {
#     int arr[100];
#     int val = ((5) * (5));
#
# }
```

预处理后的文件中，`MAX_SIZE`被替换为`100`，`SQUARE(5)`被替换为`((5) * (5))`，`LOG("程序启动")`因为`DEBUG`未定义而被替换为空。

### 2. 编译（Compilation，-S）

编译阶段将预处理后的代码转换为汇编语言：

```bash
# 生成汇编代码
g++ -S preproc_demo.cpp

# 指定输出文件名
g++ -S preproc_demo.cpp -o preproc_demo.s

# 生成带注释的汇编代码（推荐，便于阅读）
g++ -S -fverbose-asm preproc_demo.cpp -o preproc_demo_verbose.s

# 查看汇编输出
cat preproc_demo.s
```

汇编输出示例（x86_64）：

```asm
    .file   "preproc_demo.cpp"
    .text
    .globl  main
    .type   main, @function
main:
.LFB0:
    .cfi_startproc
    pushq   %rbp
    .cfi_def_cfa_offset 16
    .cfi_offset 6, -16
    movq    %rsp, %rbp
    .cfi_def_cfa_register 6
    subq    $416, %rsp
    movl    $25, -4(%rbp)        # val = SQUARE(5) = 25
    movl    $0, %eax
    leave
    .cfi_restore 5
    .cfi_def_cfa 4, 4
    ret
    .cfi_endproc
.LFE0:
    .size   main, .-main
```

### 3. 汇编（Assembly，-c）

汇编阶段将汇编代码转换为目标文件（机器码）：

```bash
# 只编译不链接，生成目标文件
g++ -c preproc_demo.cpp

# 指定输出文件名
g++ -c preproc_demo.cpp -o preproc_demo.o

# 也可以从汇编文件生成目标文件
g++ -c preproc_demo.s -o preproc_demo.o

# 查看目标文件中的符号表
nm preproc_demo.o
# 输出类似：
#                  U __libc_csu_init
# 0000000000000000 T main
#                  U _Unwind_Resume

# 查看目标文件的段信息
objdump -h preproc_demo.o
# 输出类似：
# preproc_demo.o:     file format elf64-x86-64
#
# Sections:
# Idx Name          Size      VMA               LMA               File off  Algn
#   0 .text         00000015  0000000000000000  0000000000000000  00000040  2**0
#   1 .data         00000000  0000000000000000  0000000000000000  00000055  2**0
#   2 .bss          00000000  0000000000000000  0000000000000000  00000055  2**0

# 反汇编目标文件
objdump -d preproc_demo.o
```

### 4. 链接（Linking）

链接阶段将目标文件和库文件合并为最终的可执行文件：

```bash
# 从目标文件链接生成可执行文件
g++ preproc_demo.o -o preproc_demo

# 直接从源文件一步到位（隐含所有四个阶段）
g++ preproc_demo.cpp -o preproc_demo

# 查看链接了哪些动态库
ldd preproc_demo
# 输出示例：
#     linux-vdso.so.1 (0x00007ffd8f3fe000)
#     libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007f8c2a000000)
#     libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f8c29c00000)
#     libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f8c29b1f000)
#     /lib64/ld-linux-x86-64.so.2 (0x00007f8c2a2a0000)

# 查看详细的链接过程
g++ -v preproc_demo.o -o preproc_demo 2>&1 | tail -20
```

### 5. 四阶段完整流程示意

```bash
# 阶段1：预处理
g++ -E main.cpp -o main.ii

# 阶段2：编译
g++ -S main.ii -o main.s

# 阶段3：汇编
g++ -c main.s -o main.o

# 阶段4：链接
g++ main.o -o main

# 等价于一步完成：
g++ main.cpp -o main
```

各阶段文件类型对照：

| 阶段 | 输入 | 输出 | 扩展名 |
|------|------|------|--------|
| 预处理 | .cpp/.c | .ii/.i | 预处理后的C++/C源文件 |
| 编译 | .ii/.i | .s | 汇编文件 |
| 汇编 | .s | .o | 目标文件（机器码） |
| 链接 | .o + 库 | 可执行文件 | ELF格式 |

---

## 3. 常用编译参数完全指南

### 1. 基本参数

```bash
# -o：指定输出文件名
g++ main.cpp -o myapp

# -c：只编译和汇编，不链接（生成.o文件）
g++ -c main.cpp
g++ -c main.cpp -o main.o

# -S：只编译到汇编阶段（生成.s文件）
g++ -S main.cpp
g++ -S main.cpp -o main.s

# -E：只执行预处理
g++ -E main.cpp

# -v：显示编译的详细过程（查看编译器内部做了什么）
g++ -v main.cpp -o main
# 输出包含：
# - 编译器搜索路径
# - 各阶段调用的实际命令
# - 链接器调用的详细参数

# --save-temps：保存所有中间文件
g++ --save-temps main.cpp -o main
# 会生成：main.ii（预处理）、main.s（汇编）、main.o（目标文件）、main（可执行文件）

# -pipe：使用管道代替临时文件（加速编译）
g++ -pipe main.cpp -o main

# -x：指定输入语言（覆盖文件扩展名推断）
g++ -x c++ source.xyz -o source
# 支持的语言标识：c, c++, assembler, none（恢复自动检测）
```

### 2. 警告参数

警告是编译器发现代码中潜在问题时给出的提示。合理使用警告参数可以极大提高代码质量。

```bash
# -Wall：启用大部分常用警告
g++ -Wall main.cpp -o main
# 包含的警告类型：-Wformat, -Wimplicit, -Wunused, -Wswitch 等

# -Wextra：启用-Wall未包含的额外警告
g++ -Wextra main.cpp -o main
# 包含：-Wtype-limits, -Wempty-body, -Wuninitialized 等

# -Werror：将所有警告视为错误（编译不通过）
g++ -Werror main.cpp -o main
# 任何警告都会导致编译失败

# -Wpedantic：严格遵循ISO标准，不允许任何GNU扩展
g++ -Wpedantic main.cpp -o main
# 例如：禁止使用变长数组（VLA，GNU扩展）

# -Wshadow：检测变量遮蔽（内层作用域变量名与外层相同）
g++ -Wshadow main.cpp -o main
```

```cpp
// shadow_demo.cpp——变量遮蔽示例
int value = 10;

void func() {
    int value = 20;  // -Wshadow 会警告：declaration of 'value' shadows a global declaration
    for (int i = 0; i < 5; i++) {
        int value = i * 2;  // -Wshadow 会警告：declaration of 'value' shadows a previous local
    }
}
```

```bash
# -Wconversion：检测可能导致值改变的隐式类型转换
g++ -Wconversion main.cpp -o main
```

```cpp
// conversion_demo.cpp——隐式转换示例
void convert_demo() {
    double d = 3.14;
    int i = d;           // -Wconversion 警告：double -> int 可能丢失精度
    unsigned int u = -1; // -Wconversion 警告：负数转为无符号
    short s = 65535;     // -Wconversion 警告：溢出
}
```

```bash
# -Wold-style-cast：检测C风格强制转换
g++ -Wold-style-cast main.cpp -o main
```

```cpp
// old_style_cast_demo.cpp
void cast_demo() {
    double d = 3.14;
    int i = (int)d;              // -Wold-style-cast 警告
    int j = static_cast<int>(d); // 正确的C++风格
}
```

```bash
# -Wunused：检测未使用的变量、函数、标签等
g++ -Wunused main.cpp -o main

# -Wunused-parameter：检测未使用的函数参数
g++ -Wunused-parameter main.cpp -o main
```

```cpp
// unused_demo.cpp
int unused_demo(int x, int y) {  // y未使用，-Wunused-parameter 会警告
    int temp = x * 2;            // temp未使用，-Wunused-variable 会警告
    return x;
}
```

**推荐的警告组合：**

```bash
# 开发阶段推荐（严格但不过度）
g++ -Wall -Wextra -Wshadow -Wconversion -Wpedantic main.cpp -o main

# 生产环境推荐（最严格）
g++ -Wall -Wextra -Wshadow -Wconversion -Wold-style-cast -Werror main.cpp -o main

# 针对特定警告关闭（当你确认某个警告不需要时）
g++ -Wall -Wextra -Wno-unused-parameter main.cpp -o main

# 将特定警告视为错误
g++ -Wall -Werror=return-type main.cpp -o main
```

### 3. 调试参数

```bash
# -g：生成调试信息（DWARF格式）
g++ -g main.cpp -o main
# 可以在GDB中设置断点、查看变量值等

# -g0：不生成调试信息（等同于无-g）
g++ -g0 main.cpp -o main

# -g1：最小调试信息（函数和外部变量，无局部变量）
g++ -g1 main.cpp -o main

# -g2：默认调试级别（等同于-g）
g++ -g2 main.cpp -o main

# -g3：最详细的调试信息（包含宏定义）
g++ -g3 main.cpp -o main
# 在GDB中可以查看宏的值：print MACRO_NAME

# -ggdb：生成GDB专用的调试信息
g++ -ggdb main.cpp -o main
# 包含GDB扩展的调试信息，可能比-g更有用

# -fno-omit-frame-pointer：保留帧指针（默认在-O及以上会省略）
g++ -O2 -g -fno-omit-frame-pointer main.cpp -o main
# 保留帧指针使栈回溯更准确，便于调试

# -pg：插入性能分析代码（供gprof使用）
g++ -pg main.cpp -o main
# 编译后运行程序会生成gmon.out
# 然后用gprof分析：gprof main gmon.out > analysis.txt
```

调试与优化结合使用：

```bash
# 推荐的调试编译组合
g++ -g -Og -Wall -Wextra main.cpp -o main_debug

# 推荐的发布编译组合（带调试信息，便于事后分析core dump）
g++ -g -O2 -Wall -Wextra main.cpp -o main_release

# 注意：-g和-O可以同时使用，但优化可能使调试体验变差
# 因为变量可能被优化掉、代码执行顺序可能改变
```

### 4. C/C++标准参数

```bash
# C语言标准
gcc -std=c11 main.c -o main       # C11标准
gcc -std=c17 main.c -o main       # C17标准（C11的小修订）
gcc -std=c23 main.c -o main       # C23标准（GCC 14+）

# C++语言标准
g++ -std=c++14 main.cpp -o main   # C++14
g++ -std=c++17 main.cpp -o main   # C++17
g++ -std=c++20 main.cpp -o main   # C++20
g++ -std=c++23 main.cpp -o main   # C++23（GCC 13+部分支持）

# GNU扩展标准（默认）
g++ -std=gnu++20 main.cpp -o main
# gnu++20 = c++20 + GNU扩展
# GNU扩展包括：typeof, 语句表达式, 零长度数组等

# 严格标准模式（禁用GNU扩展）
g++ -std=c++20 main.cpp -o main
# 不允许使用GNU扩展特性

# 查看编译器支持的C++标准
g++ -std=c++2a main.cpp -o main   # C++2a是C++20的实验性名称
```

`-std=gnu++20`与`-std=c++20`的区别：

```cpp
// gnu++20 允许但 c++20 不允许的代码
void gnu_extensions() {
    // GNU扩展：typeof运算符
    int x = 10;
    typeof(x) y = 20;  // c++20会报错，gnu++20允许

    // GNU扩展：语句表达式
    int result = ({ int tmp = 5; tmp * 2; });  // c++20报错

    // GNU扩展：零长度数组
    int arr[0];  // c++20报错，gnu++20允许
}
```

### 5. 预处理器参数

```bash
# -D：定义宏（等价于在代码中写 #define）
g++ -DDEBUG main.cpp -o main
# 等价于在代码开头写：#define DEBUG

# -D带值
g++ -DVERSION=\"2.0\" main.cpp -o main
# 等价于：#define VERSION "2.0"

g++ -DMAX_CONN=100 main.cpp -o main
# 等价于：#define MAX_CONN 100

# -U：取消宏定义
g++ -UDEBUG main.cpp -o main
# 确保DEBUG宏未定义

# -include：强制包含头文件（在源文件之前包含）
g++ -include config.h main.cpp -o main
# 等价于在main.cpp第一行写：#include "config.h"

# 实际应用示例：条件编译
```

```cpp
// config_aware.cpp
#include <iostream>

int main() {
#ifdef DEBUG
    std::cout << "调试模式" << std::endl;
#else
    std::cout << "发布模式" << std::endl;
#endif

#ifdef VERSION
    std::cout << "版本: " << VERSION << std::endl;
#endif

    return 0;
}
```

```bash
# 调试版本
g++ -DDEBUG -DVERSION=\"1.0\" config_aware.cpp -o config_aware
./config_aware
# 输出：
# 调试模式
# 版本: 1.0

# 发布版本
g++ -DVERSION=\"2.0\" config_aware.cpp -o config_aware
./config_aware
# 输出：
# 发布模式
# 版本: 2.0
```

---

## 4. 多文件编译方法

### 1. 方法1：一次性编译所有文件

```cpp
// math_utils.h
#pragma once
int add(int a, int b);
int multiply(int a, int b);
```

```cpp
// math_utils.cpp
#include "math_utils.h"

int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}
```

```cpp
// main.cpp
#include <iostream>
#include "math_utils.h"

int main() {
    std::cout << "3 + 4 = " << add(3, 4) << std::endl;
    std::cout << "3 * 4 = " << multiply(3, 4) << std::endl;
    return 0;
}
```

```bash
# 一次性编译所有源文件
g++ main.cpp math_utils.cpp -o main
./main
# 输出：
# 3 + 4 = 7
# 3 * 4 = 12
```

### 2. 方法2：分别编译再链接

```bash
# 步骤1：分别编译各源文件为目标文件
g++ -c main.cpp -o main.o
g++ -c math_utils.cpp -o math_utils.o

# 步骤2：链接所有目标文件
g++ main.o math_utils.o -o main

./main
# 输出：
# 3 + 4 = 7
# 3 * 4 = 12
```

### 3. 方法3：使用Makefile

```makefile
# Makefile
CXX = g++
CXXFLAGS = -Wall -Wextra -std=c++17 -O2
TARGET = main
SRCS = main.cpp math_utils.cpp
OBJS = $(SRCS:.cpp=.o)

# 默认目标
all: $(TARGET)

# 链接目标文件
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(OBJS) -o $(TARGET)

# 编译源文件为目标文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理
clean:
	rm -f $(OBJS) $(TARGET)

# 声明依赖关系（头文件改变时重新编译）
main.o: math_utils.h
math_utils.o: math_utils.h

.PHONY: all clean
```

```bash
# 使用Makefile编译
make
# g++ -Wall -Wextra -std=c++17 -O2 -c main.cpp -o main.o
# g++ -Wall -Wextra -std=c++17 -O2 -c math_utils.cpp -o math_utils.o
# g++ -Wall -Wextra -std=c++17 -O2 main.o math_utils.o -o main

# 清理
make clean
# rm -f main.o math_utils.o main

# 只重新编译修改过的文件
# 修改math_utils.cpp后
make
# g++ -Wall -Wextra -std=c++17 -O2 -c math_utils.cpp -o math_utils.o
# g++ -Wall -Wextra -std=c++17 -O2 main.o math_utils.o -o main
```

### 4. 方法4：使用CMake

```cmake
# CMakeLists.txt
cmake_minimum_required(VERSION 3.16)
project(MyProject LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(main main.cpp math_utils.cpp)

target_compile_options(main PRIVATE -Wall -Wextra)
```

```bash
# 使用CMake编译
mkdir build && cd build
cmake ..
make
```

### 5. 各方法优缺点对比

| 方法 | 优点 | 缺点 | 适用场景 |
|------|------|------|----------|
| 一次性编译 | 简单直接 | 改一个文件全部重编译 | 2-3个文件的小项目 |
| 分别编译再链接 | 只重编译修改的文件 | 手动管理依赖关系 | 中等项目 |
| Makefile | 自动依赖追踪，增量编译 | 语法复杂，跨平台差 | 中大型项目 |
| CMake | 跨平台，自动依赖，IDE支持 | 学习曲线较陡 | 大型项目、跨平台项目 |
