# GDB 调试器配置与使用指南

> **前置阅读**：如果你还没有安装GDB，请先阅读 [FAQ-138：开发环境配置详解](../../03-问题解答/01-基础概念/33-开发环境配置.md) 完成调试器安装。本文档假设你已经安装好了GDB，需要学习如何配置和深度使用。

> **相关教程**：GDB调试技巧和Sanitizer使用见 [调试技巧](../04-工程实践/07-调试技巧.md)，Core Dump分析见 [什么是Core-Dump核心转储](../03-问题解答/08-调试与性能/04-什么是Core-Dump核心转储.md)。

## 1. GDB 调试配置：在CMake中配置 vs 在VS Code中配置

### 1. GDB调试的前提条件

GDB调试器要正常工作，必须满足两个核心条件：

1. **编译时加入调试信息**：使用 `-g` 编译选项，将源代码信息（文件名、行号、变量名等）嵌入到可执行文件中
2. **存在可执行文件**：GDB需要加载编译后的可执行文件才能进行调试

没有 `-g` 选项编译的程序，GDB无法显示源代码、设置断点或查看变量值。

```bash
# 无调试信息的编译——GDB几乎无法使用
g++ main.cpp -o program

# 带调试信息的编译——GDB可以完整调试
g++ -g main.cpp -o program
```

### 2. 在CMakeLists.txt中配置调试信息

CMake通过构建类型和编译选项来控制调试信息的生成：

```cmake
cmake_minimum_required(VERSION 3.10)
project(MyProject)

# 方式一：设置构建类型为Debug
set(CMAKE_BUILD_TYPE Debug)

# 方式二：手动添加编译选项（更精细的控制）
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG")

# 方式三：强制所有构建类型都带调试信息
add_compile_options(-g -O0)

add_executable(program main.cpp)
```

各编译选项的含义：

| 选项 | 含义 |
|------|------|
| `-g` | 生成调试信息（DWARF格式） |
| `-O0` | 关闭优化，确保变量不会被优化掉 |
| `-DDEBUG` | 定义DEBUG宏，用于条件编译 |
| `-ggdb` | 生成GDB专用的调试信息（比 `-g` 更丰富） |
| `-rdynamic` | 导出符号信息，便于动态链接器使用 |

**推荐配置**：在开发阶段使用 `-g -O0`，确保调试信息完整且变量不被优化：

```cmake
# 完整的Debug模式CMake配置示例
cmake_minimum_required(VERSION 3.10)
project(DebugDemo)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_STANDARD 17)

# Debug模式下的编译选项
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -ggdb -Wall -Wextra")

add_executable(program
    main.cpp
    utils.cpp
)
```

也可以在命令行中指定构建类型：

```bash
# 配置时指定Debug构建
cmake -DCMAKE_BUILD_TYPE=Debug -B build

# 编译
cmake --build build
```

### 3. 在VS Code的launch.json中配置GDB

VS Code通过 `launch.json` 文件配置调试器的启动参数。GDB相关的核心配置项：

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "GDB 调试",
            "type": "cppdbg",
            "request": "launch",
            "program": "${workspaceFolder}/build/program",
            "args": ["arg1", "arg2"],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [
                {
                    "name": "LD_LIBRARY_PATH",
                    "value": "${workspaceFolder}/build/lib"
                }
            ],
            "externalConsole": false,
            "MIMode": "gdb",
            "miDebuggerPath": "/usr/bin/gdb",
            "setupCommands": [
                {
                    "description": "为GDB启用整齐打印",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description": "将反汇编风格设置为Intel",
                    "text": "-gdb-set disassembly-flavor intel",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "CMake: build"
        }
    ]
}
```

关键配置项说明：

| 配置项 | 说明 |
|--------|------|
| `program` | 要调试的可执行文件路径（必须带调试信息） |
| `MIMode` | 调试器模式，Linux/MinGW用 `gdb`，macOS用 `lldb` |
| `miDebuggerPath` | GDB可执行文件的路径 |
| `setupCommands` | GDB启动后自动执行的命令列表 |
| `args` | 程序运行参数 |
| `stopAtEntry` | 是否在程序入口处暂停 |
| `cwd` | 程序工作目录 |

**附加到进程的配置**：

```json
{
    "name": "GDB 附加到进程",
    "type": "cppdbg",
    "request": "attach",
    "program": "${workspaceFolder}/build/program",
    "MIMode": "gdb",
    "miDebuggerPath": "/usr/bin/gdb",
    "processId": "${command:pickProcess}"
}
```

### 4. 两者的关系

CMake和launch.json各司其职，协同完成调试：

```
┌─────────────────────────────────────────────────────────────────┐
│                        调试配置流程                              │
│                                                                 │
│  ┌──────────────────┐         ┌──────────────────────────────┐ │
│  │  CMakeLists.txt  │         │       launch.json            │ │
│  │                  │         │                              │ │
│  │  - 构建类型      │         │  - 调试器类型(GDB/LLDB)      │ │
│  │  - 编译选项(-g)  │         │  - 可执行文件路径            │ │
│  │  - 优化等级(-O0) │         │  - 程序参数                  │ │
│  │  - 链接选项      │         │  - 环境变量                  │ │
│  └────────┬─────────┘         │  - 启动前任务                │ │
│           │                   │  - GDB初始化命令              │ │
│           ▼                   └──────────────┬───────────────┘ │
│  ┌──────────────────┐                        │                 │
│  │  编译（CMake）   │                        │                 │
│  │                  │                        │                 │
│  │  生成带调试信息  │                        │                 │
│  │  的可执行文件    │                        │                 │
│  └────────┬─────────┘                        │                 │
│           │                                  │                 │
│           ▼                                  ▼                 │
│  ┌─────────────────────────────────────────────────────────┐   │
│  │              GDB 加载可执行文件并启动调试                │   │
│  └─────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────┘
```

**核心要点**：
- **CMake** 负责"造子弹"——确保编译出的可执行文件包含调试信息
- **launch.json** 负责"开枪"——告诉调试器如何加载和运行程序
- 如果CMake没有加 `-g`，launch.json配置再完美也无法查看变量和源码

### 5. 在CLion中配置GDB

CLion默认使用GDB作为调试器，配置步骤：

1. 打开 `File → Settings → Build, Execution, Deployment → Toolchains`
2. 选择或添加工具链（如MinGW、WSL、Remote）
3. 在Debugger字段中指定GDB路径：
   - MinGW: `C:\msys64\mingw64\bin\gdb.exe`
   - WSL: `/usr/bin/gdb`
   - 自定义: 点击 `...` 浏览选择GDB可执行文件
4. 点击OK保存

CLion中自定义GDB初始化脚本：

1. 打开 `File → Settings → Build, Execution, Deployment → Debugger → GDB`
2. 在 "GDB options" 中添加 `-x /path/to/.gdbinit`
3. 或在 "Custom .gdbinit" 字段中指定初始化脚本路径

### 6. 完整配置流程图

```
  源代码(.cpp/.h)
       │
       ▼
  ┌─────────────────────────────┐
  │    编译系统（CMake/Make）    │
  │                             │
  │  CMAKE_BUILD_TYPE=Debug     │
  │  CXX_FLAGS = -g -O0        │
  │                             │
  └─────────────┬───────────────┘
                │
                ▼
  ┌─────────────────────────────┐
  │  带调试信息的可执行文件      │
  │  (ELF/DWARF格式)            │
  │                             │
  │  包含：源码行号映射、        │
  │        变量类型与位置、      │
  │        函数符号表            │
  └─────────────┬───────────────┘
                │
                ▼
  ┌─────────────────────────────┐
  │    调试器（GDB/LLDB）        │
  │                             │
  │  launch.json / IDE配置      │
  │  指定program路径、参数等     │
  │                             │
  └─────────────────────────────┘
```

---

## 2. GDB 安装与验证

### 1. Linux安装

```bash
# Debian/Ubuntu
sudo apt update
sudo apt install gdb

# CentOS/RHEL/Fedora
sudo dnf install gdb

# Arch Linux
sudo pacman -S gdb
```

### 2. Windows安装

**方式一：MinGW（推荐）**

```bash
# 通过MSYS2安装
pacman -S mingw-w64-x86_64-gdb

# 验证安装路径
which gdb
# 输出：/mingw64/bin/gdb
```

**方式二：独立MinGW安装**

从 MinGW-w64 项目下载带GDB的发行版，确保 `bin` 目录下有 `gdb.exe`。

**方式三：WSL（Windows Subsystem for Linux）**

```bash
# 在WSL中安装，调试Linux程序
sudo apt install gdb
```

### 3. macOS

macOS默认使用LLDB替代GDB：

```bash
# 安装Xcode命令行工具（自带LLDB）
xcode-select --install

# 也可以通过Homebrew安装GDB
brew install gdb

# macOS上GDB需要代码签名才能调试
# 创建证书：
# 1. 打开"钥匙串访问" → 证书助理 → 创建证书
# 2. 名称：gdb-cert，类型：代码签名
# 3. 设置为信任
# 4. 对GDB签名：
codesign -s gdb-cert $(which gdb)
```

### 4. 验证安装

```bash
# 查看GDB版本
gdb --version
# 输出示例：
# GNU gdb (Ubuntu 12.1-0ubuntu1~22.04) 12.1
# Copyright (C) 2022 Free Software Foundation, Inc.

# 查看GDB支持的架构
gdb --configuration
# 输出包含：--host=x86_64-linux-gnu --target=x86_64-linux-gnu
```

### 5. 常见安装问题

**问题一：GDB无法调试，提示权限不足**

```bash
# Linux：检查文件权限
ls -la ./program
# 确保有执行权限
chmod +x ./program

# Linux：ptrace权限限制
# 临时解决
sudo sysctl -w kernel.yama.ptrace_scope=0
# 永久解决：编辑 /etc/sysctl.d/10-ptrace.conf
# kernel.yama.ptrace_scope = 0
```

**问题二：macOS上GDB提示代码签名错误**

```
Unable to find Mach task port for process-id 12345: (os/kern) failure (0x5).
```

解决方法：创建代码签名证书并对GDB签名（见2.3节）。

**问题三：Windows上GDB路径包含空格**

```json
// launch.json中路径需要正确转义
"miDebuggerPath": "C:\\Program Files\\mingw64\\bin\\gdb.exe"
```

---

## 3. GDB 基本使用

### 1. 启动调试

```bash
# 基本启动方式
gdb ./program

# 启动时加载core dump文件
gdb ./program core.dump

# 附加到运行中的进程
gdb -p 12345

# 启动时指定参数
gdb --args ./program arg1 arg2

# 启动时不显示欢迎信息
gdb -q ./program
```

启动后GDB的交互界面：

```
(gdb) 
```

所有GDB命令都在 `(gdb)` 提示符后输入。

### 2. 运行程序

```bash
# 运行程序（不带参数）
(gdb) run

# 运行程序（带参数）
(gdb) run arg1 arg2 arg3

# 运行程序（带输入重定向）
(gdb) run < input.txt

# 运行程序（带输出重定向）
(gdb) run > output.txt 2>&1
```

### 3. 设置参数

```bash
# 设置程序运行参数
(gdb) set args arg1 arg2 arg3

# 设置环境变量
(gdb) set environment LD_LIBRARY_PATH=/usr/local/lib

# 查看当前参数
(gdb) show args
# 输出：Argument list to give program being debugged when it is started is "arg1 arg2 arg3".
```

### 4. 退出GDB

```bash
# 退出GDB
(gdb) quit

# 如果程序正在运行，GDB会询问是否终止
# A debugging session is active.
# Inferior 1 [process 12345] will be killed.
# Quit anyway? (y or n) y
```

### 5. 帮助系统

```bash
# 查看帮助总览
(gdb) help

# 查看特定命令的帮助
(gdb) help break
# 输出：Set breakpoint at specified line or function.

# 查看某个分类下的命令
(gdb) help breakpoints

# 搜索命令
(gdb) apropos watch
# 输出包含所有与watch相关的命令
```

### 6. 完整的基本调试流程示例

```bash
# 编译带调试信息的程序
$ g++ -g -O0 main.cpp -o program

# 启动GDB
$ gdb ./program

# 在main函数设置断点
(gdb) break main

# 运行程序
(gdb) run

# 单步执行
(gdb) next

# 查看变量
(gdb) print x

# 继续运行
(gdb) continue

# 退出
(gdb) quit
```

---

## 4. 断点操作

### 1. 设置断点

```bash
# 在函数入口设置断点
(gdb) break main
# 输出：Breakpoint 1 at 0x401156: file main.cpp, line 10.

# 在指定文件的指定行设置断点
(gdb) break main.cpp:42
# 输出：Breakpoint 2 at 0x4011a0: file main.cpp, line 42.

# 在指定文件的函数设置断点
(gdb) break utils.cpp:processData
# 输出：Breakpoint 3 at 0x4012e0: file utils.cpp, line 15.

# 在条件满足时触发断点
(gdb) break main.cpp:42 if x > 10
# 输出：Breakpoint 4 at 0x4011a0: file main.cpp, line 42.

# 在C++成员函数设置断点
(gdb) break MyClass::myMethod
# 或使用完整签名
(gdb) break MyClass::myMethod(int)
```

### 2. 查看断点

```bash
# 查看所有断点
(gdb) info breakpoints
# 输出：
# Num     Type           Disp Enb Address    What
# 1       breakpoint     keep y   0x00401156 in main at main.cpp:10
# 2       breakpoint     keep y   0x004011a0 in main at main.cpp:42
#     stop only if x > 10
# 3       breakpoint     keep y   0x004012e0 in processData at utils.cpp:15

# 查看指定编号的断点
(gdb) info breakpoints 1
```

### 3. 删除断点

```bash
# 删除指定编号的断点
(gdb) delete 1

# 删除所有断点
(gdb) delete
# 确认：Delete all breakpoints? (y or n) y

# 删除指定范围的断点
(gdb) delete 2-5
```

### 4. 启用/禁用断点

```bash
# 禁用断点（保留但不触发）
(gdb) disable 1

# 启用断点
(gdb) enable 1

# 启用后触发一次后自动禁用
(gdb) enable once 1

# 启用后触发一次后自动删除
(gdb) enable delete 1

# 禁用所有断点
(gdb) disable

# 启用所有断点
(gdb) enable
```

### 5. 条件断点

```bash
# 设置条件断点（新建时指定条件）
(gdb) break main.cpp:42 if x > 10

# 为已有断点添加条件
(gdb) condition 1 x > 10
# 断点1现在只在 x > 10 时触发

# 为已有断点添加复杂条件
(gdb) condition 2 strcmp(name, "test") == 0

# 清除断点条件（断点变为无条件）
(gdb) condition 2
```

### 6. 观察断点（数据断点）

观察断点在变量值发生变化时触发，无需指定代码行：

```bash
# 监视变量写入
(gdb) watch x
# 输出：Hardware watchpoint 1: x

# 监视变量读取
(gdb) rwatch x
# 输出：Hardware read watchpoint 2: x

# 监视变量读写
(gdb) awatch x
# 输出：Hardware access watchpoint 3: x

# 监视指针指向的值
(gdb) watch *ptr

# 监视结构体成员
(gdb) watch person.age
```

观察断点触发时的输出：

```
Hardware watchpoint 1: x

Old value = 0
New value = 42
main () at main.cpp:15
15          x = 42;
```

### 7. 临时断点

临时断点触发一次后自动删除：

```bash
# 在指定行设置临时断点
(gdb) tbreak main.cpp:100
# 输出：Temporary breakpoint 1 at 0x401200: file main.cpp, line 100.

# 在函数入口设置临时断点
(gdb) tbreak processData

# 带条件的临时断点
(gdb) tbreak main.cpp:100 if count > 50
```

### 8. 忽略断点N次

```bash
# 忽略断点1的前100次触发
(gdb) ignore 1 100
# 输出：Will ignore next 100 crossings of breakpoint 1.

# 典型场景：循环中只在第101次迭代时停下
(gdb) break main.cpp:20
(gdb) ignore 1 100
(gdb) run
# 程序会在第101次执行到第20行时暂停
```

---

## 5. 执行控制

### 1. 单步执行（进入函数）—— step

```bash
# 执行一行代码，如果遇到函数调用则进入函数内部
(gdb) step

# 执行指定步数
(gdb) step 5
```

示例：

```cpp
// main.cpp
void inner() {
    int a = 1;    // <-- step会进入这里
}

void outer() {
    inner();      // <-- 当前位置
    int b = 2;
}

int main() {
    outer();
    return 0;
}
```

```
(gdb) break outer
(gdb) run
(gdb) step          # 进入inner函数
inner () at main.cpp:2
2           int a = 1;
```

### 2. 单步执行（不进入函数）—— next

```bash
# 执行一行代码，函数调用作为一步直接执行完
(gdb) next

# 执行指定步数
(gdb) next 5
```

```
(gdb) break outer
(gdb) run
(gdb) next          # 不进入inner函数，直接执行完
outer () at main.cpp:6
6           int b = 2;
```

### 3. 继续运行—— continue

```bash
# 继续运行直到遇到下一个断点或程序结束
(gdb) continue

# 继续运行但忽略当前断点N次
(gdb) continue 10
```

### 4. 执行到当前函数返回—— finish

```bash
# 执行到当前函数返回，并显示返回值
(gdb) finish
# 输出示例：
# Run till exit from #0  inner () at main.cpp:3
# 0x00401178 in outer () at main.cpp:5
# 5           inner();
# Value returned is $1 = void
```

### 5. 执行到指定行—— until

```bash
# 执行到当前函数的指定行
(gdb) until 50

# 不带参数：执行到比当前行号更大的行（用于跳出循环）
(gdb) until
```

典型场景——跳出循环：

```cpp
for (int i = 0; i < 1000; i++) {
    sum += i;    // <-- 当前停在这里
}
// 后续代码      // <-- until会跳到这里
```

```
(gdb) until        # 跳出循环，停到循环后面的代码
```

### 6. 跳过当前行—— advance

```bash
# 继续执行到指定位置（比until更灵活，可以跨函数）
(gdb) advance main.cpp:100
# 输出：main () at main.cpp:100

# 继续执行到指定函数
(gdb) advance processData
```

### 7. 反向调试

GDB支持反向调试（记录程序执行过程并回退），需要先开启执行记录：

```bash
# 在开始运行或某个位置开启执行记录
(gdb) target record-full

# 或使用更轻量的方式（仅支持部分架构）
(gdb) record

# 反向单步（进入函数）
(gdb) reverse-step

# 反向单步（不进入函数）
(gdb) reverse-next

# 反向继续运行（回退到上一个断点）
(gdb) reverse-continue

# 反向执行到函数开头
(gdb) reverse-finish

# 停止记录
(gdb) record stop
```

反向调试示例：

```
(gdb) break main
(gdb) run
(gdb) next
(gdb) next
(gdb) record           # 开启记录
(gdb) next             # 前进一步
(gdb) next             # 再前进一步
(gdb) reverse-next     # 回退一步
(gdb) reverse-next     # 再回退一步
(gdb) record stop      # 停止记录
```

---

## 6. 查看数据

### 1. 打印变量

```bash
# 打印基本类型变量
(gdb) print x
# $1 = 42

# 打印指针
(gdb) print ptr
# $2 = (int *) 0x7fffffffe2a0

# 打印指针指向的值
(gdb) print *ptr
# $3 = 42

# 打印布尔值
(gdb) print flag
# $4 = true

# 打印字符串
(gdb) print str
# $5 = 0x402000 "Hello, GDB"

# 打印字符串内容（指定长度）
(gdb) print str@50
```

### 2. 打印数组

```bash
# 打印静态数组
(gdb) print arr
# $1 = {1, 2, 3, 4, 5}

# 打印动态数组（指针+长度）
(gdb) print *array@10
# $2 = {0, 1, 4, 9, 16, 25, 36, 49, 64, 81}

# 打印数组的一部分
(gdb) print array[2]@5
# $3 = {4, 9, 16, 25, 36}

# 打印二维数组
(gdb) print matrix
# $4 = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}}
```

### 3. 打印结构体

```bash
# 打印结构体变量
(gdb) print person
# $1 = {name = "Alice", age = 30, score = 95.5}

# 打印结构体指针
(gdb) print *person_ptr
# $2 = {name = "Bob", age = 25, score = 88.0}

# 打印结构体成员
(gdb) print person.age
# $3 = 30

# 打印链表节点
(gdb) print *head
# $4 = {data = 1, next = 0x5555555592a0}
```

### 4. 打印STL容器

```bash
# 打印vector
(gdb) print vec
# $1 = std::vector of length 5, capacity 8 = {1, 2, 3, 4, 5}

# 打印vector的大小
(gdb) print vec.size()
# $2 = 5

# 打印vector的指定元素
(gdb) print vec[2]
# $3 = 3

# 打印map
(gdb) print mymap
# $4 = std::map with 3 elements = {[1] = "one", [2] = "two", [3] = "three"}

# 打印string
(gdb) print str
# $5 = "Hello, World"

# 打印set
(gdb) print myset
# $6 = std::set with 4 elements = {10, 20, 30, 40}

# 打印list
(gdb) print mylist
# $7 = std::list = {[0] = 1, [1] = 2, [2] = 3}

# 打印unordered_map
(gdb) print umap
# $8 = std::unordered_map with 2 elements = {["key1"] = 100, ["key2"] = 200}
```

> **注意**：STL容器的美观打印需要GDB的pretty-printer支持，详见第九章。

### 5. 格式化输出

```bash
# 十六进制
(gdb) print/x 255
# $1 = 0xff

# 十进制
(gdb) print/d 0xff
# $2 = 255

# 字符串
(gdb) print/s str_ptr
# $3 = "Hello"

# 字符
(gdb) print/c 65
# $4 = 65 'A'

# 二进制
(gdb) print/t 10
# $5 = 1010

# 八进制
(gdb) print/o 8
# $6 = 010

# 浮点数
(gdb) print/f 3.14159
# $7 = 3.14159
```

格式字符汇总：

| 格式字符 | 说明 | 示例 |
|----------|------|------|
| `/x` | 十六进制 | `print/x 255` → `0xff` |
| `/d` | 十进制 | `print/d 0xff` → `255` |
| `/u` | 无符号十进制 | `print/u -1` → `4294967295` |
| `/o` | 八进制 | `print/o 8` → `010` |
| `/t` | 二进制 | `print/t 10` → `1010` |
| `/c` | 字符 | `print/c 65` → `65 'A'` |
| `/f` | 浮点数 | `print/f 3` → `3.0` |
| `/s` | 字符串 | `print/s ptr` → `"hello"` |
| `/a` | 地址 | `print/a ptr` → `0x7fff...` |

### 6. 查看内存

```bash
# 查看内存：x/NFU address
# N=数量, F=格式, U=单位大小

# 查看10个十六进制字（4字节）
(gdb) x/10xw 0x7fffffffe2a0
# 0x7fffffffe2a0: 0x00000001  0x00000002  0x00000003  0x00000004
# 0x7fffffffe2b0: 0x00000005  0x00000006  0x00000007  0x00000008
# 0x7fffffffe2c0: 0x00000009  0x0000000a

# 查看20个字节
(gdb) x/20xb 0x7fffffffe2a0
# 0x7fffffffe2a0: 0x01  0x00  0x00  0x00  0x02  0x00  0x00  0x00
# 0x7fffffffe2a8: 0x03  0x00  0x00  0x00  0x04  0x00  0x00  0x00
# 0x7fffffffe2b0: 0x05  0x00  0x00  0x00

# 查看字符串
(gdb) x/s 0x402000
# 0x402000:       "Hello, GDB"

# 查看指令
(gdb) x/10i 0x401156
# 0x401156 <main>:       push   rbp
# 0x401157 <main+1>:     mov    rbp, rsp
# 0x40115a <main+4>:     sub    rsp, 0x10
```

单位大小说明：

| 字符 | 大小 |
|------|------|
| `b` | 1字节 |
| `h` | 2字节（半字） |
| `w` | 4字节（字） |
| `g` | 8字节（双字） |

### 7. 查看寄存器

```bash
# 查看所有寄存器
(gdb) info registers
# rax            0x0                 0
# rbx            0x5555555551a0      93824992235680
# rcx            0x5555555592a0      93824992242336
# rdx            0x7fffffffe3a8      140737488282280
# rsi            0x7fffffffe398      140737488282264
# rdi            0x1                 1
# rip            0x5555555551a9      0x5555555551a9 <main+9>
# ...

# 查看指定寄存器
(gdb) print $rip
# $1 = (void (*)()) 0x5555555551a9 <main+9>

# 查看浮点寄存器
(gdb) info float

# 查看向量寄存器（SSE/AVX）
(gdb) info vector
```

### 8. 查看类型

```bash
# 查看变量类型
(gdb) ptype x
# type = int

# 查看结构体定义
(gdb) ptype person
# type = struct Person {
#     std::string name;
#     int age;
#     double score;
# }

# 查看类定义
(gdb) ptype myObject
# type = class MyClass {
#   public:
#     int getValue(void) const;
#     void setValue(int);
#   private:
#     int m_value;
# }

# 查看模板实例化类型
(gdb) ptype vec
# type = std::vector<int, std::allocator<int> >

# 查看typedef
(gdb) ptype MyInt
# type = int
```

### 9. 自动显示

```bash
# 设置自动显示：每次程序暂停时自动打印变量
(gdb) display x
# 1: x = 42

# 设置自动显示（带格式）
(gdb) display/x flag
# 2: flag = 0x1

# 查看所有自动显示
(gdb) info display
# Num  Enb  Expression
# 1:   y    x
# 2:   y    flag

# 禁用自动显示
(gdb) disable display 1

# 启用自动显示
(gdb) enable display 1

# 删除自动显示
(gdb) undisplay 1

# 删除所有自动显示
(gdb) undisplay
```

### 10. 查看调用栈

```bash
# 查看调用栈
(gdb) backtrace
# #0  inner () at main.cpp:3
# #1  0x00401178 in outer () at main.cpp:6
# #2  0x004011a0 in main () at main.cpp:11

# 查看调用栈（带局部变量）
(gdb) backtrace full
# #0  inner () at main.cpp:3
#         a = 1
# #1  0x00401178 in outer () at main.cpp:6
#         b = 2
# #2  0x004011a0 in main () at main.cpp:11
#         result = 0

# 简写形式
(gdb) bt
(gdb) bt full
```

### 11. 切换栈帧

```bash
# 切换到指定栈帧
(gdb) frame 1
# #1  0x00401178 in outer () at main.cpp:6
# 6           inner();

# 向上切换栈帧（调用者方向）
(gdb) up
# #2  0x004011a0 in main () at main.cpp:11

# 向下切换栈帧（被调用者方向）
(gdb) down
# #1  0x00401178 in outer () at main.cpp:6

# 查看当前栈帧信息
(gdb) info frame
# Stack level 1, frame at 0x7fffffffe2b0:
#  rip = 0x401178 in outer (main.cpp:6); saved rip = 0x4011a0
#  called by frame at 0x7fffffffe2d0, caller of frame at 0x7fffffffe290
#  source language c++.
#  Arglist at 0x7fffffffe2a0, args:
#  Locals at 0x7fffffffe2a0, Previous frame's sp is 0x7fffffffe2b0
```

---

## 7. 多线程调试

### 1. 查看线程

```bash
# 查看所有线程
(gdb) info threads
#   Id   Target Id          Frame
# * 1    Thread 0x7ffff7a02740 "program"  main () at main.cpp:10
#   2    Thread 0x7ffff7a01700 "program"  worker (arg=0x0) at thread.cpp:25
#   3    Thread 0x7ffff72000700 "program"  __GI___libc_write (fd=1, buf=..., nbytes=5) at write.c:27

# * 标记表示当前线程
```

### 2. 切换线程

```bash
# 切换到指定线程
(gdb) thread 2
# [Switching to thread 2 (Thread 0x7ffff7a01700)]
# #0  worker (arg=0x0) at thread.cpp:25

# 切换后可以查看该线程的变量和调用栈
(gdb) print data
(gdb) backtrace
```

### 3. 查看所有线程调用栈

```bash
# 查看所有线程的调用栈
(gdb) thread apply all bt
# Thread 3 (Thread 0x7ffff72000700):
# #0  __GI___libc_write (fd=1, buf=..., nbytes=5) at write.c:27
# #1  0x00007ffff7a4e5c4 in write () from /lib/libc.so.6
#
# Thread 2 (Thread 0x7ffff7a01700):
# #0  worker (arg=0x0) at thread.cpp:25
# #1  0x00007ffff7bc5ea7 in start_thread () from /lib/libpthread.so.0
#
# Thread 1 (Thread 0x7ffff7a02740):
# #0  main () at main.cpp:10

# 查看所有线程的详细调用栈
(gdb) thread apply all bt full

# 对所有线程执行某个命令
(gdb) thread apply all print x
```

### 4. 线程断点

```bash
# 只在指定线程触发断点
(gdb) break thread.cpp:25 thread 2
# Breakpoint 1 at 0x4012e0: file thread.cpp, line 25.
# 该断点只在线程2执行到此行时触发

# 为所有线程设置断点（默认行为）
(gdb) break thread.cpp:25
```

### 5. 锁定调度

```bash
# 开启调度锁定：只有当前线程执行
(gdb) set scheduler-locking on

# 关闭调度锁定：所有线程都可以执行
(gdb) set scheduler-locking off

# step模式：单步时只有当前线程执行，continue时所有线程执行
(gdb) set scheduler-locking step

# 查看当前调度锁定状态
(gdb) show scheduler-locking
# Scheduler locking is "off".
```

调度锁定模式对比：

| 模式 | 单步执行时 | continue时 | 适用场景 |
|------|-----------|-----------|---------|
| `off` | 所有线程运行 | 所有线程运行 | 默认模式 |
| `on` | 仅当前线程 | 仅当前线程 | 精确调试单线程 |
| `step` | 仅当前线程 | 所有线程运行 | 单步调试时避免干扰 |

### 6. 查看锁状态

```bash
# 查看互斥锁状态（需要libthread_db支持）
(gdb) info locks
#  Mutex 1: Owner 2, Count 1

# 查看线程局部存储
(gdb) info tls
```

### 7. 多线程调试完整示例

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;
int shared_counter = 0;

void worker(int id) {
    for (int i = 0; i < 100; i++) {
        std::lock_guard<std::mutex> lock(mtx);
        shared_counter++;
    }
}

int main() {
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; i++) {
        threads.emplace_back(worker, i);
    }
    for (auto& t : threads) {
        t.join();
    }
    std::cout << "Counter: " << shared_counter << std::endl;
    return 0;
}
```

调试流程：

```bash
$ g++ -g -O0 -pthread thread_demo.cpp -o thread_demo
$ gdb ./thread_demo

(gdb) break worker
(gdb) run

# 查看线程
(gdb) info threads

# 锁定调度，只调试当前线程
(gdb) set scheduler-locking on

# 单步调试当前线程
(gdb) next
(gdb) print shared_counter

# 切换到其他线程查看状态
(gdb) set scheduler-locking off
(gdb) thread 2
(gdb) bt

# 查看所有线程的调用栈
(gdb) thread apply all bt
```

---

## 8. GDB 高级技巧

### 1. .gdbinit配置文件

`.gdbinit` 是GDB启动时自动加载的初始化脚本，可以存放常用配置：

```bash
# ~/.gdbinit —— 全局配置
# 项目根目录/.gdbinit —— 项目级配置

# 设置打印选项
set print pretty on
set print object on
set print static-members on
set print vtbl on
set print demangle on
set print sevenbit-strings off

# 设置分页
set pagination off

# 设置历史记录
set history filename ~/.gdb_history
set history save on
set history size 10000

# 设置默认汇编风格
set disassembly-flavor intel

# 加载STL pretty-printer
python
import sys
sys.path.insert(0, '/usr/share/gcc/python')
from libstdcxx.v6.printers import register_libstdcxx_printers
register_libstdcxx_printers(None)
end

# 自定义快捷命令
define bmain
    break main
end

define bfunc
    break $arg0
    echo 断点已设置在函数: 
    echo $arg0\n
end

document bfunc
在指定函数设置断点
用法：bfunc function_name
end
```

> **注意**：GDB默认不允许加载当前目录的 `.gdbinit`，需要添加信任：

```bash
# 在 ~/.gdbinit 中添加
set auto-load safe-path /
# 或指定特定目录
set auto-load safe-path /path/to/project
```

### 2. 自定义命令

```bash
# 在GDB中定义自定义命令
(gdb) define print_header
> echo ===== 调试信息 =====\n
> info threads
> echo ----- 调用栈 -----\n
> backtrace
> echo ===================\n
> end

# 使用自定义命令
(gdb) print_header

# 为自定义命令添加文档
(gdb) document print_header
打印当前调试状态摘要，包括线程信息和调用栈
用法：print_header
end

# 带参数的自定义命令
(gdb) define watch_var
> watch $arg0
> echo 已设置观察断点: 
> echo $arg0\n
> end

(gdb) watch_var shared_counter
```

### 3. 日志输出

```bash
# 设置日志文件
(gdb) set logging file gdb.log

# 开启日志
(gdb) set logging on
# Copying output to gdb.log.

# 执行一些命令，输出会同时显示在终端和写入日志文件
(gdb) backtrace
(gdb) print x
(gdb) info threads

# 关闭日志
(gdb) set logging off
# Done logging to gdb.log.

# 覆盖模式（默认追加）
(gdb) set logging overwrite on

# 只记录到文件不在终端显示
(gdb) set logging redirect on
```

### 4. TUI模式

TUI（Text User Interface）提供终端内的图形化调试界面：

```bash
# 启动时进入TUI模式
gdb -tui ./program

# 在GDB中切换TUI模式
(gdb) tui enable
(gdb) tui disable

# 快捷键
# Ctrl+X A    切换TUI模式
# Ctrl+X 2    切换双窗口（源码+汇编/源码+命令）
# Ctrl+X 1    切换单窗口
# Ctrl+X O    切换焦点窗口
# Ctrl+L      刷新屏幕
```

TUI窗口类型：

```bash
# 查看源代码窗口
(gdb) layout src

# 查看汇编窗口
(gdb) layout asm

# 查看源代码和汇编
(gdb) layout split

# 查看寄存器窗口
(gdb) layout regs

# 焦点切换到命令窗口
(gdb) focus cmd

# 焦点切换到源码窗口
(gdb) focus src
```

### 5. 远程调试

远程调试用于在目标设备（嵌入式、服务器等）上运行程序，在开发机上调试：

**目标设备（被调试端）**：

```bash
# 启动gdbserver，监听1234端口
gdbserver :1234 ./program

# 或附加到已运行的进程
gdbserver :1234 --attach 12345

# 输出：
# Process ./program created; pid = 12345
# Listening on port 1234
```

**开发机（调试端）**：

```bash
# 启动GDB
gdb ./program

# 连接到远程目标
(gdb) target remote 192.168.1.100:1234
# Remote debugging using 192.168.1.100:1234

# 连接成功后可以正常使用GDB命令
(gdb) break main
(gdb) continue
```

也可以使用串口调试：

```bash
(gdb) target remote /dev/ttyUSB0
# 或
(gdb) target remote /dev/ttyS0
```

### 6. Core dump分析

Core dump是程序崩溃时的内存快照，可以事后分析崩溃原因：

```bash
# 确保系统允许生成core dump
ulimit -c unlimited

# 编译带调试信息的程序
g++ -g -O0 crash_demo.cpp -o crash_demo

# 运行程序（假设崩溃了）
./crash_demo
# Segmentation fault (core dumped)

# 使用GDB分析core dump
gdb ./crash_demo core
```

分析流程：

```
(gdb) bt
# #0  0x00401178 in processData (ptr=0x0) at crash_demo.cpp:15
# #1  0x004011a0 in main () at crash_demo.cpp:25

# 查看崩溃帧的变量
(gdb) frame 0
(gdb) print ptr
# $1 = (int *) 0x0
# 空指针解引用！

# 查看崩溃时的寄存器
(gdb) info registers
```

Core dump文件位置配置：

```bash
# 查看core dump文件模式
cat /proc/sys/kernel/core_pattern
# 输出可能是：/var/core/core.%e.%p.%t

# 设置core dump文件位置
echo "/var/core/core.%e.%p.%t" | sudo tee /proc/sys/kernel/core_pattern

# 永久设置：编辑 /etc/sysctl.conf
# kernel.core_pattern = /var/core/core.%e.%p.%t
```

### 7. 附加到运行中的进程

```bash
# 方式一：通过PID附加
gdb -p 12345

# 方式二：在GDB中附加
(gdb) attach 12345

# 附加后可以正常调试
(gdb) bt
(gdb) break main.cpp:42
(gdb) continue

# 分离进程（进程继续运行）
(gdb) detach

# 分离并退出
(gdb) quit
```

### 8. 调试子进程

```bash
# 设置fork后跟随子进程
(gdb) set follow-fork-mode child

# 设置fork后跟随父进程（默认）
(gdb) set follow-fork-mode parent

# 设置fork时是否分离另一个进程
(gdb) set detach-on-fork on    # 分离（默认）
(gdb) set detach-on-fork off   # 不分离，两个进程都在GDB控制下

# 查看当前设置
(gdb) show follow-fork-mode
(gdb) show detach-on-fork
```

### 9. 调试时调用函数

```bash
# 调用全局函数
(gdb) call printf("x = %d\n", x)
# x = 42

# 调用成员函数
(gdb) call obj.getValue()
# $1 = 42

# 调用STL函数
(gdb) call vec.size()
# $2 = 5

# 调用自定义函数
(gdb) call calculateSum(arr, 10)
# $3 = 55

# 注意：被调用的函数会实际执行，可能产生副作用
```

### 10. 修改变量值

```bash
# 修改变量值
(gdb) set variable x = 10

# 修改指针指向的值
(gdb) set variable *ptr = 42

# 修改结构体成员
(gdb) set variable person.age = 25

# 修改数组元素
(gdb) set variable arr[0] = 100

# 修改字符串（需要分配内存）
(gdb) call strcpy(str, "new value")
```

### 11. 搜索内存

```bash
# 在内存范围中搜索值
(gdb) find 0x600000, 0x700000, 42
# 0x600abc
# 0x600def
# 2 patterns found

# 搜索字符串
(gdb) find 0x600000, 0x700000, "error"

# 搜索指定类型的值
(gdb) find /w 0x600000, 0x700000, 0xdeadbeef
```

---

## 9. GDB 与各IDE的集成配置

### 1. VS Code + GDB

#### 1. launch.json完整配置

```json
{
    "version": "0.2.0",
    "configurations": [
        {
            "name": "(GDB) 启动调试",
            "type": "cppdbg",
            "request": "launch",
            "program": "${command:cmake.launchTargetPath}",
            "args": [],
            "stopAtEntry": false,
            "cwd": "${workspaceFolder}",
            "environment": [
                {
                    "name": "PATH",
                    "value": "${env:PATH};${workspaceFolder}/build" // Windows用分号分隔
                },
                {
                    "name": "LD_LIBRARY_PATH",
                    "value": "${workspaceFolder}/build/lib"
                }
            ],
            "externalConsole": false,
            "MIMode": "gdb",
            "miDebuggerPath": "/usr/bin/gdb",
            "setupCommands": [
                {
                    "description": "为GDB启用整齐打印",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                },
                {
                    "description": "将反汇编风格设置为Intel",
                    "text": "-gdb-set disassembly-flavor intel",
                    "ignoreFailures": true
                },
                {
                    "description": "设置打印选项",
                    "text": "-gdb-set print pretty on",
                    "ignoreFailures": true
                },
                {
                    "description": "设置分页关闭",
                    "text": "-gdb-set pagination off",
                    "ignoreFailures": true
                }
            ],
            "preLaunchTask": "CMake: build",
            "logging": {
                "moduleLoad": true,
                "trace": false,
                "engineLogging": false,
                "programOutput": true,
                "exceptions": false
            }
        },
        {
            "name": "(GDB) 附加到进程",
            "type": "cppdbg",
            "request": "attach",
            "program": "${command:cmake.launchTargetPath}",
            "MIMode": "gdb",
            "miDebuggerPath": "/usr/bin/gdb",
            "processId": "${command:pickProcess}",
            "setupCommands": [
                {
                    "description": "为GDB启用整齐打印",
                    "text": "-enable-pretty-printing",
                    "ignoreFailures": true
                }
            ]
        }
    ]
}
```

Windows MinGW环境下的配置差异：

```json
{
    "name": "(GDB) MinGW启动调试",
    "type": "cppdbg",
    "request": "launch",
    "program": "${workspaceFolder}/build/program.exe",
    "MIMode": "gdb",
    "miDebuggerPath": "C:\\msys64\\mingw64\\bin\\gdb.exe",
    "cwd": "${workspaceFolder}",
    "setupCommands": [
        {
            "description": "为GDB启用整齐打印",
            "text": "-enable-pretty-printing",
            "ignoreFailures": true
        }
    ]
}
```

#### 2. setupCommands的作用

`setupCommands` 是GDB启动后、加载程序前自动执行的命令列表。常用配置：

```json
"setupCommands": [
    {
        "description": "启用pretty-printing，让STL容器可读",
        "text": "-enable-pretty-printing",
        "ignoreFailures": true
    },
    {
        "description": "Intel汇编风格（默认AT&T）",
        "text": "-gdb-set disassembly-flavor intel",
        "ignoreFailures": true
    },
    {
        "description": "结构体美化打印",
        "text": "-gdb-set print pretty on",
        "ignoreFailures": true
    },
    {
        "description": "显示虚函数表",
        "text": "-gdb-set print vtbl on",
        "ignoreFailures": true
    },
    {
        "description": "显示对象动态类型",
        "text": "-gdb-set print object on",
        "ignoreFailures": true
    }
]
```

#### 3. 条件断点的设置方式

在VS Code中设置条件断点：

1. **界面操作**：在代码行号左侧右键 → "添加条件断点" → 输入条件表达式
2. **launch.json中无法预设条件断点**，但可以通过 `.gdbinit` 实现：

```bash
# .gdbinit 中预设条件断点
break main.cpp:42 if x > 10
```

3. **在调试控制台中使用GDB命令**：

```
-exec break main.cpp:42 if x > 10
```

#### 4. 调试时查看STL容器的pretty-printer配置

VS Code默认可能无法正确显示STL容器内容，需要配置pretty-printer：

**步骤一：查找pretty-printer脚本位置**

```bash
# Linux
find /usr -name "printers.py" 2>/dev/null
# 常见路径：/usr/share/gcc/python/libstdcxx/v6/printers.py

# MinGW/MSYS2
find /c/msys64 -name "printers.py" 2>/dev/null
# 常见路径：/mingw64/share/gcc/python/libstdcxx/v6/printers.py
```

**步骤二：在 `.gdbinit` 中加载**

```bash
# ~/.gdbinit 或 项目/.gdbinit
python
import sys
sys.path.insert(0, '/usr/share/gcc/python')
from libstdcxx.v6.printers import register_libstdcxx_printers
register_libstdcxx_printers(None)
end
```

**步骤三：验证**

```
(gdb) print vec
# 配置前：$1 = {_M_impl = {_M_start = 0x5555555592a0, _M_finish = 0x5555555592b4, ...}}
# 配置后：$1 = std::vector of length 5, capacity 8 = {1, 2, 3, 4, 5}
```

### 2. CLion + GDB

#### 1. 工具链中配置GDB路径

1. 打开 `File → Settings → Build, Execution, Deployment → Toolchains`
2. 添加或编辑工具链：
   - **MinGW**：Debugger字段填写 `C:\msys64\mingw64\bin\gdb.exe`
   - **WSL**：Debugger字段填写 `/usr/bin/gdb`
   - **Remote**：填写远程GDB路径
3. 确保工具链被当前CMake配置使用

#### 2. 自定义GDB初始化脚本

1. 打开 `File → Settings → Build, Execution, Deployment → Debugger → GDB`
2. 配置项：
   - **GDB**: 选择Bundled GDB或Custom GDB
   - **Custom GDB executable**: 指定GDB路径
   - **GDB options**: 添加启动参数，如 `-x /path/to/.gdbinit`
3. 点击OK保存

#### 3. 调试配置界面

1. 打开 `Run → Edit Configurations`
2. 添加或编辑CMake Debug配置：
   - **Target**: 选择编译目标
   - **Program arguments**: 程序运行参数
   - **Working directory**: 工作目录
   - **Environment variables**: 环境变量
   - **Before launch**: 启动前执行的任务（如Build）
3. 高级选项：
   - 勾选 "Stop at program entry point" 在程序入口暂停
   - 设置 "Symbol file" 指定符号文件路径

### 3. 命令行直接使用GDB

#### 1. 典型调试流程

```bash
# 1. 编译带调试信息的程序
g++ -g -O0 -std=c++17 main.cpp utils.cpp -o program

# 2. 启动GDB
gdb ./program

# 3. 设置断点
(gdb) break main
(gdb) break utils.cpp:processData

# 4. 运行程序
(gdb) run arg1 arg2

# 5. 程序在断点处暂停，开始调试
(gdb) next                    # 单步执行
(gdb) print x                 # 查看变量
(gdb) step                    # 进入函数
(gdb) finish                  # 执行到函数返回
(gdb) continue                # 继续运行

# 6. 遇到问题，查看调用栈
(gdb) backtrace
(gdb) frame 0
(gdb) print local_var

# 7. 调试完毕，退出
(gdb) quit
```

#### 2. 常用命令速查表

| 类别 | 命令 | 说明 |
|------|------|------|
| **启动** | `gdb ./program` | 启动调试 |
| | `gdb -p PID` | 附加到进程 |
| | `gdb ./program core` | 分析core dump |
| **运行** | `run [args]` | 运行程序 |
| | `set args args` | 设置参数 |
| | `continue [n]` | 继续运行 |
| **断点** | `break loc` | 设置断点 |
| | `tbreak loc` | 临时断点 |
| | `watch var` | 观察断点 |
| | `info breakpoints` | 查看断点 |
| | `delete [n]` | 删除断点 |
| | `enable/disable n` | 启用/禁用 |
| | `condition n expr` | 条件断点 |
| | `ignore n count` | 忽略N次 |
| **执行** | `step [n]` | 单步（进入） |
| | `next [n]` | 单步（不进入） |
| | `finish` | 执行到返回 |
| | `until [line]` | 执行到指定行 |
| **查看** | `print expr` | 打印表达式 |
| | `display expr` | 自动显示 |
| | `ptype var` | 查看类型 |
| | `backtrace` | 调用栈 |
| | `frame n` | 切换栈帧 |
| | `x/NFU addr` | 查看内存 |
| **线程** | `info threads` | 查看线程 |
| | `thread n` | 切换线程 |
| | `thread apply all bt` | 所有线程栈 |
| **修改** | `set var x = val` | 修改变量 |
| | `call func(args)` | 调用函数 |
| **其他** | `quit` | 退出 |
| | `help [cmd]` | 帮助 |
| | `shell cmd` | 执行shell命令 |

---

## 10. GDB 常见问题

### 1. 调试时变量显示 `<optimized out>`

**问题描述**：

```
(gdb) print x
# <optimized out>
```

**原因**：编译器优化导致变量被消除或放入寄存器，调试信息不完整。

**解决方案**：

```bash
# 方案一：使用Debug模式编译（-O0）
g++ -g -O0 main.cpp -o program

# 方案二：在CMake中确保Debug构建类型
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# 方案三：对特定变量禁用优化
# 在源码中使用volatile关键字
volatile int x = 42;

# 方案四：如果必须用优化，尝试查看寄存器
(gdb) info registers
(gdb) print $rax    # 变量可能在寄存器中
```

### 2. 断点不命中

**问题描述**：

```
(gdb) break main.cpp:42
# Breakpoint 1 at 0x4011a0: file main.cpp, line 42.
(gdb) continue
# 程序直接运行结束，没有停在断点处
```

**原因与解决方案**：

```bash
# 原因一：编译时没有加 -g 选项
# 解决：重新编译
g++ -g -O0 main.cpp -o program

# 原因二：代码被优化掉（如内联、死代码消除）
# 解决：关闭优化
g++ -g -O0 main.cpp -o program

# 原因三：多文件编译时部分文件缺少调试信息
# 解决：确保所有源文件都加 -g
g++ -g -O0 main.cpp utils.cpp -o program

# 原因四：断点设置在不会执行的代码路径上
# 检查：确认代码路径确实会执行
(gdb) break main.cpp:42
(gdb) run
# 如果程序没有走到第42行，断点自然不会命中

# 原因五：模板/内联函数的断点问题
# 解决：使用函数签名设置断点
(gdb) break void MyClass<int>::process()
```

### 3. 无法设置断点（No source file）

**问题描述**：

```
(gdb) break utils.cpp:10
# No source file named utils.cpp.
```

**解决方案**：

```bash
# 检查调试信息中包含的源文件列表
(gdb) info sources

# 检查可执行文件是否包含调试信息
(gdb) file ./program
(gdb) info files

# 确保源文件路径正确（可能是相对路径问题）
# 设置源文件搜索路径
(gdb) directory /path/to/source
(gdb) directory src/:include/

# 使用函数名设置断点（不依赖源文件路径）
(gdb) break processData

# 检查编译时是否加了 -g
$ readelf --debug-dump=info ./program | grep DW_AT_name
```

### 4. GDB无法识别STL容器内容

**问题描述**：

```
(gdb) print vec
# $1 = {_M_impl = {_M_start = 0x5555555592a0, _M_finish = 0x5555555592b4,
#   _M_end_of_storage = 0x5555555592b4}}
```

**解决方案**：

```bash
# 方案一：配置pretty-printer（详见9.1.4节）
# 在 .gdbinit 中添加：
python
import sys
sys.path.insert(0, '/usr/share/gcc/python')
from libstdcxx.v6.printers import register_libstdcxx_printers
register_libstdcxx_printers(None)
end

# 方案二：手动遍历vector
(gdb) print vec._M_impl._M_start[0]
(gdb) print vec._M_impl._M_start[1]
# 或打印所有元素
(gdb) print *vec._M_impl._M_start@5

# 方案三：使用GDB的STL查看脚本
# 下载 gcc/gdb-libstdcxx-utils
# 在 .gdbinit 中 source 该脚本

# 方案四：在VS Code中安装C++扩展并配置pretty-printer
# 确保launch.json中有：
# "setupCommands": [{"text": "-enable-pretty-printing"}]
```

### 5. 多线程调试时程序行为改变

**问题描述**：调试多线程程序时，断点和单步执行导致线程调度改变，bug无法复现。

**解决方案**：

```bash
# 方案一：使用调度锁定
(gdb) set scheduler-locking on
# 只有当前线程执行，其他线程暂停

# 方案二：使用条件断点代替单步
(gdb) break main.cpp:42 if shared_counter == 500
# 让程序自由运行，只在特定条件时暂停

# 方案三：使用观察断点
(gdb) watch shared_counter
# 变量变化时自动暂停

# 方案四：记录日志后分析
(gdb) set logging file debug.log
(gdb) set logging on
(gdb) continue
# 程序运行完毕后分析日志

# 方案五：使用非侵入式调试
(gdb) set pagination off
(gdb) break main.cpp:100
(gdb) commands 1
> print shared_counter
> continue
> end
(gdb) run
# 断点触发时自动打印变量并继续运行，不影响线程调度
```

### 6. Core dump文件无法分析

**问题描述**：

```
(gdb) ./program core.12345
# core.12345: not in executable format: File format not recognized
# 或
# warning: Could not load shared library symbols for ...
```

**原因与解决方案**：

```bash
# 原因一：可执行文件与core dump不匹配
# 确认可执行文件路径正确
$ file ./program
# ./program: ELF 64-bit LSB executable, x86-64 ...

$ file core.12345
# core.12345: ELF 64-bit LSB core file, x86-64

# 原因二：可执行文件被重新编译
# core dump对应的是旧版本程序，需要找到旧版本
# 解决：使用版本控制找回旧版本

# 原因三：缺少共享库符号
# 设置共享库搜索路径
(gdb) set solib-search-path /path/to/libs

# 原因四：core dump文件被截断
# 检查core文件大小
$ ls -la core.12345
# 确保ulimit -c设置足够大
$ ulimit -c unlimited

# 原因五：系统禁止生成core dump
# 检查
$ ulimit -c
# 0 表示禁止

# 开启
$ ulimit -c unlimited

# 检查core_pattern
$ cat /proc/sys/kernel/core_pattern
# 如果是 | 开头，表示被管道到其他程序
# 临时修改
$ echo "core.%e.%p" | sudo tee /proc/sys/kernel/core_pattern

# 原因六：分析时缺少调试信息
# 确保程序编译时加了 -g
(gdb) info files
# 查看是否有 .debug 段
```

**Core dump分析完整流程**：

```bash
# 1. 确保core dump可用
ulimit -c unlimited

# 2. 运行程序获取core dump
./program
# Segmentation fault (core dumped)

# 3. 查找core dump文件
ls -la core*
# 或查看系统配置
cat /proc/sys/kernel/core_pattern

# 4. 使用GDB分析
gdb ./program core

# 5. 查看崩溃位置
(gdb) bt
(gdb) bt full

# 6. 查看崩溃帧的详细信息
(gdb) frame 0
(gdb) info locals
(gdb) info args

# 7. 检查是否为空指针
(gdb) print ptr

# 8. 检查是否为非法内存访问
(gdb) x/10xw ptr
```
