# GCC/G++编译器详解

> 掌握GCC/G++编译器的使用与编译参数

---

> 💡 **通俗理解 - 什么是编译器？**

想象你要翻译一本英文书：
- **编译器** 就像"翻译官"，把你的代码（英文）翻译成机器能看懂的语言（机器码）
- **GCC** 就是那个"翻译官"，专门翻译C语言
- **G++** 是翻译C++的"翻译官"

---

> 🔬 **抽象理解 - 编译器的本质**：
> - **编译器**：是将高级语言转换为机器语言的"翻译器"
> - **GCC**：是GNU Compiler Collection，支持多种语言的编译器套件
> - **编译过程**：预处理→编译→汇编→链接
> - **编译参数**：是控制编译过程的"选项"，决定如何翻译

---

> **🎯 工欲善其事，必先利其器。掌握编译器，方能驾驭代码之力。**
> 
> （想要做好事情，先要磨快工具；想要写好代码，先要精通编译器。）

> **"A compiler is a program that translates one language into another."** — Alfred Aho
> （编译器是将一种语言翻译成另一种语言的程序。）

## 前置知识
- [动态库加载方式](20-动态库加载方式.md)
## 后续内容
- [Makefile与构建系统](22-Makefile与构建系统.md)
---

## 目录

- [1. GCC简介与版本](#1-gcc简介与版本)
- [2. 编译过程详解](#2-编译过程详解)
- [3. 常用编译参数](#3-新手必背核心参数快速参考)
- [5. 警告与调试参数](#5-警告与调试参数)
- [6. 优化参数](#6-优化参数)
- [7. 链接参数](#7-链接参数)
- [8. 跨平台编译](#8-跨平台编译)

---

## 1. GCC简介与版本

### 1. 什么是GCC？

**GCC（GNU Compiler Collection）**：GNU编译器套件，支持C、C++、Objective-C、Fortran、Java等多种语言。

**G++**：GCC中的C++编译器。

### 2. 查看版本

```bash
# 查看GCC版本
gcc --version

# 查看G++版本
g++ --version

# 查看支持的语言
gcc -v --help
```

### 3. GCC支持的平台

- Linux（所有主流发行版）
- macOS（通过Homebrew或Xcode）
- Windows（通过MinGW、Cygwin、WSL）

---

## 2. 编译过程详解

### 1. 编译的四个阶段

```
源代码 (.c/.cpp)
    ↓
预处理 (Preprocessing)    ← #include, #define 处理
    ↓
编译 (Compilation)       ← 翻译为汇编语言
    ↓
汇编 (Assembly)          ← 翻译为机器码 (.o/.obj)
    ↓
链接 (Linking)           ← 链接库生成可执行文件
    ↓
可执行文件 (.exe/.out)
```

### 2. 各阶段详细说明

**1. 预处理（Preprocessing）**

```bash
# 预处理，生成 .i 文件
gcc -E main.c -o main.i

# 查看预处理后的内容
gcc -E main.c | head -100
```

预处理做的事情：
- 展开 `#include` 文件
- 展开 `#define` 宏
- 处理条件编译 `#if`, `#ifdef`
- 删除注释

**2. 编译（Compilation）**

```bash
# 编译为汇编语言，生成 .s 文件
gcc -S main.i -o main.s

# 一步完成预处理+编译
gcc -S main.c -o main.s
```

**3. 汇编（Assembly）**

```bash
# 汇编为目标文件，生成 .o 文件
gcc -c main.s -o main.o

# 一步完成预处理+编译+汇编
gcc -c main.c -o main.o
```

**4. 链接（Linking）**

```bash
# 链接生成可执行文件
gcc main.o -o main

# 链接多个目标文件
gcc main.o utils.o -o main
```

### 3. 一步编译

```bash
# 最常用的方式，一步完成所有步骤
gcc main.c -o main

g++ main.cpp -o main
```

---

## 3. 新手必背核心参数（快速参考）

### 1. 最常用的10个参数（按编译流程）

| 参数 | 作用 | 用法示例 |
|------|------|----------|
| `-I` | 指定头文件路径 | `gcc main.c -I./include` |
| `-L` | 指定库文件路径 | `gcc main.c -L./lib` |
| `-l` | 链接指定库（小写L） | `gcc main.c -lm` |
| `-o` | 指定输出文件名 | `gcc main.c -o myapp` |
| `-g` | 生成调试信息 | `gcc main.c -g -o main` |
| `-Wall` | 开启所有警告 | `gcc main.c -Wall` |
| `-c` | 只编译不链接 | `gcc -c main.c` |
| `-std=` | 指定C标准 | `gcc main.c -std=c89` |
| `-O0/-O1/-O2` | 优化等级 | `gcc main.c -O2` |
| `-D` | 定义宏 | `gcc main.c -DDEBUG` |

### 2. 最常用组合命令（直接用）

```bash
# 1. 标准编译（带警告+指定输出）
gcc main.c -Wall -o main

# 2. 带头文件路径
gcc main.c -I./include -Wall -o main

# 3. 带头文件 + 链接库
gcc main.c -Iinclude -Llib -lmylib -o main

# 4. 调试版本
gcc main.c -g -Wall -o main
```

### 3. 最容易混淆的三个参数

- **`-I`**：头文件目录（.h）
- **`-L`**：库文件目录（.a/.so）
- **`-l`**：库名字（去掉 lib 和后缀）

---

## 4. 常用编译参数详解

### 1. 基本参数

| 参数 | 说明 | 示例 |
|------|------|------|
| `-o` | 指定输出文件名 | `gcc main.c -o myapp` |
| `-c` | 只编译不链接 | `gcc -c main.c` |
| `-E` | 只预处理 | `gcc -E main.c` |
| `-S` | 编译到汇编 | `gcc -S main.c` |
| `-v` | 显示详细信息 | `gcc -v main.c` |
| `-x` | 指定语言 | `gcc -x c++ main.txt` |

### 2. 指定头文件路径

```bash
# 指定头文件搜索路径
gcc main.c -I./include -I/usr/local/include

# 例子
# gcc main.c -I./headers -o main
```

### 3. 指定库路径

```bash
# 指定库文件搜索路径
gcc main.c -L./lib -L/usr/local/lib

# 指定要链接的库
gcc main.c -lmylib          # -l指定库名（去掉lib和后缀）
gcc main.c -l:mylib.a       # 指定完整库名
gcc main.c -lpthread        # 链接pthread库

# 完整例子
gcc main.c -L./lib -lmylib -o main
```

---

## 5. 警告与调试参数

### 1. 警告参数

```bash
# 开启所有警告
gcc main.c -Wall -Wextra -o main

# 开启额外警告
gcc main.c -Wall -Wextra -Wpedantic -o main

# 将警告视为错误
gcc main.c -Werror -o main

# 关闭特定警告
gcc main.c -Wno-unused-parameter -o main
```

**常用警告参数：**

| 参数 | 说明 |
|------|------|
| `-Wall` | 开启大多数常见警告 |
| `-Wextra` | 开启额外警告 |
| `-Wpedantic` | 严格遵循ISO C/C++标准 |
| `-Werror` | 把警告当作错误 |
| `-w` | 关闭所有警告 |
| `-Wunused` | 未使用变量的警告 |
| `-Wformat` | printf/scanf格式字符串警告 |

### 2. 调试参数

```bash
# 生成调试信息
gcc main.c -g -o main

# 生成更详细的调试信息
gcc main.c -g3 -o main

# 结合Sanitizer检测内存问题
gcc main.c -g -fsanitize=address -o main
```

**常用调试参数：**

| 参数 | 说明 |
|------|------|
| `-g` | 生成调试信息 |
| `-g3` | 包含宏定义的调试信息 |
| `-pg` | 生成gprof性能分析数据 |

---

## 6. 优化参数

### 1. 优化级别

```bash
# 不优化（默认，调试用）
gcc main.c -O0 -o main

# 基本优化
gcc main.c -O1 -o main

# 较高优化（推荐）
gcc main.c -O2 -o main

# 最高优化（可能影响调试）
gcc main.c -O3 -o main

# 优化文件大小
gcc main.c -Os -o main
```

### 2. 优化参数详解

| 参数 | 说明 |
|------|------|
| `-O0` | 不优化，便于调试 |
| `-O1` | 基本优化，编译快 |
| `-O2` | 常用优化，平衡速度和时间 |
| `-O3` | 激进优化，可能增加代码体积 |
| `-Os` | 优化文件大小 |
| `-Ofast` | 忽略严格标准，极限优化 |
| `-Og` | 优化调试体验 |

### 3. 特定优化

```bash
# 内联函数优化
gcc main.c -finline-functions -o main

# 向量化优化（SIMD）
gcc main.c -march=native -O3 -o main

# 链接时优化（LTO）
gcc main.c -flto -o main

# 禁用特定优化
gcc main.c -fno-inline -o main
```

---

## 7. 链接参数

### 1. 链接静态库

```bash
# 链接静态库
gcc main.c -L./lib -static -lmylib -o main
```

### 2. 链接动态库

```bash
# 链接动态库
gcc main.c -L./lib -lmylib -o main

# 指定运行时库路径
gcc main.c -L./lib -lmylib -Wl,-rpath,./lib -o main
```

### 3. 链接参数

| 参数 | 说明 |
|------|------|
| `-static` | 静态链接 |
| `-shared` | 生成动态库 |
| `-Wl,option` | 传递选项给链接器 |
| `-rpath` | 指定运行时库路径 |
| `-rpath-link` | 指定运行时库搜索路径 |

### 4. 完整链接示例

```bash
# 完整编译链接示例
gcc main.c utils.c \
    -I./include \
    -L./lib \
    -lmylib \
    -lpthread \
    -Wl,-rpath,./lib \
    -O2 \
    -Wall \
    -o myapp
```

---

## 8. 跨平台编译

### 1. Windows（MinGW）

```bash
# 在Linux上编译Windows可执行文件
x86_64-w64-mingw32-gcc main.c -o main.exe

# 在Linux上编译Windows动态库
x86_64-w64-mingw32-gcc -shared mylib.c -o mylib.dll
```

### 2. Linux

```bash
# 编译Linux可执行文件
gcc main.c -o main

# 编译动态库
gcc -shared -fPIC mylib.c -o libmylib.so
```

### 3. 指定目标架构

```bash
# 32位编译
gcc -m32 main.c -o main

# 64位编译（默认）
gcc -m64 main.c -o main

# 指定CPU类型
gcc -march=native main.c -o main    # 本地CPU
gcc -march=x86-64 main.c -o main   # x86-64
gcc -march=arm main.c -o main      # ARM
```

---

## 9. 常用编译命令速查

### 1. 开发阶段

```bash
# 快速编译（调试模式）
g++ -g -Wall -o myapp main.cpp

# 带 sanitizer 检测内存问题
g++ -g -fsanitize=address -o myapp main.cpp
```

### 2. 发布阶段

```bash
# 优化编译
g++ -O3 -Wall -o myapp main.cpp

# 带LTO优化
g++ -O3 -flto -o myapp main.cpp
```

### 3. 库开发阶段

```bash
# 编译动态库
g++ -fPIC -shared -o libmylib.so mylib.cpp

# 编译静态库
g++ -c mylib.cpp -o mylib.o
ar rcs libmylib.a mylib.o
```

---

## 10. 常见问题

**Q：编译时找不到头文件？**
A：
```bash
# 使用 -I 指定头文件路径
gcc main.c -I./include -o main
```

**Q：编译时找不到库？**
A：
```bash
# 使用 -L 指定库路径，-l 指定库名
gcc main.c -L./lib -lmylib -o main
```

**Q：运行时找不到动态库？**
A：
```bash
# 方法1：设置LD_LIBRARY_PATH（Linux）
export LD_LIBRARY_PATH=./lib:$LD_LIBRARY_PATH
./main

# 方法2：编译时指定rpath
gcc main.c -L./lib -lmylib -Wl,-rpath,./lib -o main

# 方法3：安装到系统目录
sudo cp libmylib.so /usr/local/lib/
sudo ldconfig
```

**Q：如何查看可执行文件的依赖？**
A：
```bash
# Linux：查看 .so 依赖
ldd myapp

# Windows MSVC：查看 .dll 依赖（需要在VS开发者命令行中运行）
dumpbin /DEPENDENTS myapp.exe

# Windows MinGW（Command Prompt）
objdump -p myapp.exe | findstr "DLL Name"

# Windows MinGW（PowerShell）- PowerShell 没有 grep，用 Select-String 代替
objdump -p myapp.exe | Select-String "DLL Name"

# 专门查看是否依赖某个特定库
objdump -p myapp.exe | Select-String "mylib"
```

**三者效果完全一样**，都是列出程序依赖的动态库。`dumpbin` 必须在 **VS 开发者命令行** 里才能用，普通 cmd/PowerShell 会报错。如果只有 MinGW，用 `objdump -p` 就够了。

---

## 11. 相关链接

**上一章：** [第20章：动态库加载方式](20-动态库加载方式.md)\
**下一章：** [第22章：Makefile与构建系统](22-Makefile与构建系统.md)

***

### 1. 相关章节

- [GCC/G++编译器深度使用指南](../../04-工程实践/开发环境/07-GCC编译器基础.md) — 编译参数完全指南、Sanitizer使用、常用技巧
- [编译器隐藏工具与鲜为人知的能力](../../04-工程实践/开发环境/10-二进制分析工具.md) — addr2line/ar/nm/objdump/readelf/strip等工具
- [为什么代码可以调试](../03-问题解答/08-调试与性能/06-为什么代码可以调试-调试信息深度解析.md) — -g参数生成的DWARF调试信息详解
