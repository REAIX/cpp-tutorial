# GCC编译器入门
> 📖 相关章节：[VSCode配置](../../05-开发环境与IDE/00-VSCode核心配置.md)、[CMake基础](../../05-开发环境与IDE/03-CMake基础入门.md)、[GCC编译器](../../05-开发环境与IDE/07-GCC编译器基础.md)

## 一个生活类比

你写了一封中文信（源代码），但收信人只懂机器语言（0和1）。`GCC` 就像一位翻译官，把你写的"人话"翻译成计算机能直接执行的"机器话"。而且这位翻译官不仅会翻译，还会帮你检查语法错误、优化表达方式，甚至能把多封信合并成一封更高效的信。

## 什么是GCC

`GCC`（GNU Compiler Collection，GNU编译器套件）是自由软件基金会推出的编译器集合。它最初只支持 `C` 语言（GNU C Compiler），后来扩展支持了 `C++`、`Objective-C`、`Fortran`、`Ada`、`Go` 等多种语言。

对于 `C++` 开发者来说，我们主要使用的是 `g++` 命令——它是 `GCC` 中专门编译 `C++` 的前端程序。

> **`gcc` vs `g++`：** `gcc` 主要编译 `C`，`g++` 主要编译 `C++`。用 `gcc` 编译 `.cpp` 文件时不会自动链接 `C++` 标准库，而 `g++` 会。所以编译 `C++` 程序请始终使用 `g++`。

## 安装GCC

### Windows

`Windows` 本身没有 `GCC`，需要安装 `MinGW` 或 `MinGW-w64`：

1. 下载 `MSYS2`（推荐）：https://www.msys2.org/
2. 安装后在终端执行：
   ```bash
   pacman -S mingw-w64-x86_64-gcc
   ```
3. 将 `C:\msys64\mingw64\bin` 添加到系统 `PATH`

验证安装：
```bash
g++ --version
```

### Linux

大多数 `Linux` 发行版已预装或可轻松安装：

```bash
# Ubuntu / Debian
sudo apt update && sudo apt install build-essential

# Fedora
sudo dnf install gcc-c++

# Arch
sudo pacman -S gcc
```

### macOS

`macOS` 使用 `Clang`（`Apple` 版），但命令名也是 `gcc`/`g++`：

```bash
xcode-select --install
```

> 注意：`macOS` 上的 `gcc` 实际上是 `Clang` 的别名。如需真正的 `GCC`，可通过 `Homebrew` 安装：`brew install gcc`，然后用 `g++-13` 等命令调用。

## 基本编译命令

### 最简单的编译

```bash
g++ main.cpp
```

这会生成一个名为 `a.out`（`Linux`/`macOS`）或 `a.exe`（`Windows`）的可执行文件。

### 指定输出文件名

```bash
g++ main.cpp -o myprogram
```

`-o` 选项指定输出文件名。这是最常用的选项之一。

### 编译多个源文件

```bash
g++ main.cpp utils.cpp helper.cpp -o myprogram
```

## 常用编译选项

| 选项 | 作用 | 示例 |
|------|------|------|
| `-o <name>` | 指定输出文件名 | `g++ main.cpp -o app` |
| `-std=c++17` | 指定 `C++` 标准 | `g++ -std=c++17 main.cpp` |
| `-Wall` | 开启常见警告 | `g++ -Wall main.cpp` |
| `-Wextra` | 开启额外警告 | `g++ -Wall -Wextra main.cpp` |
| `-Werror` | 将警告视为错误 | `g++ -Werror main.cpp` |
| `-O0` | 不优化（默认，调试用） | `g++ -O0 main.cpp` |
| `-O2` | 中等优化 | `g++ -O2 main.cpp` |
| `-O3` | 最高优化 | `g++ -O3 main.cpp` |
| `-g` | 生成调试信息 | `g++ -g main.cpp` |
| `-I <dir>` | 添加头文件搜索路径 | `g++ -I./include main.cpp` |
| `-L <dir>` | 添加库文件搜索路径 | `g++ -L./lib main.cpp` |
| `-l<name>` | 链接指定库 | `g++ main.cpp -lpthread` |
| `-c` | 只编译不链接 | `g++ -c main.cpp` |
| `-D<macro>` | 定义预处理宏 | `g++ -DDEBUG main.cpp` |

### 推荐的编译命令组合

**开发调试时：**
```bash
g++ -std=c++17 -Wall -Wextra -g -O0 main.cpp -o app
```

**发布构建时：**
```bash
g++ -std=c++17 -Wall -Wextra -O2 main.cpp -o app
```

## 编译过程详解

从源代码到可执行文件，`GCC` 实际上经历了四个阶段：

```
源代码(.cpp) → 预处理 → 编译 → 汇编 → 链接 → 可执行文件
```

### 1. 预处理（Preprocessing）

处理所有以 `#` 开头的指令，如 `#include`、`#define`、`#ifdef` 等。

```bash
g++ -E main.cpp -o main.i
```

这一步会把 `#include <iostream>` 替换成 `iostream` 头文件的全部内容，展开所有宏定义。

### 2. 编译（Compilation）

将预处理后的代码翻译成汇编语言。

```bash
g++ -S main.i -o main.s
```

### 3. 汇编（Assembly）

将汇编代码翻译成机器码（目标文件）。

```bash
g++ -c main.s -o main.o
```

### 4. 链接（Linking）

将多个目标文件和库合并成最终的可执行文件。

```bash
g++ main.o utils.o -o myprogram
```

### 为什么要了解编译过程？

因为报错信息会告诉你错误发生在哪个阶段：
- **预处理错误**：`#include` 找不到头文件 → 检查路径和 `-I` 选项
- **编译错误**：语法错误、类型不匹配 → 修改代码
- **链接错误**：`undefined reference` → 检查是否缺少源文件或 `-l` 选项

## 分步编译的实际应用

当项目有多个源文件时，分步编译可以节省时间——只重新编译修改过的文件：

```bash
# 第一次编译
g++ -c main.cpp -o main.o
g++ -c utils.cpp -o utils.o
g++ main.o utils.o -o myprogram

# 修改了 utils.cpp 后，只需重新编译它
g++ -c utils.cpp -o utils.o
g++ main.o utils.o -o myprogram
```

这正是 `Make` 和 `CMake` 等构建系统自动帮我们做的事情。

## 常见问题与解决

### "g++不是内部或外部命令"

说明 `GCC` 未安装或未添加到 `PATH`。检查安装和系统环境变量。

### "No such file or directory"（头文件找不到）

使用 `-I` 选项指定头文件路径：
```bash
g++ -I./include main.cpp -o app
```

### "undefined reference to xxx"（链接错误）

原因可能是：
- 忘记编译某个 `.cpp` 文件
- 忘记链接某个库（用 `-l` 选项）
- 函数声明了但没实现

### 中文乱码

`Windows` 下 `MinGW` 默认使用 `UTF-8`，但终端可能是 `GBK`：
```bash
g++ -finput-charset=UTF-8 -fexec-charset=GBK main.cpp -o app
```

## 总结

`GCC` 是 `C++` 开发的基础工具。初学者需要掌握的核心命令：

```bash
g++ -std=c++17 -Wall -g main.cpp -o app
```

记住：**`-std=c++17` 指定标准，`-Wall` 开启警告，`-g` 方便调试，`-o` 指定输出。** 这四个选项覆盖了日常开发的绝大部分需求。随着项目变大，你会自然过渡到使用 `CMake` 等构建系统来管理编译，但底层调用的仍然是 `GCC`。