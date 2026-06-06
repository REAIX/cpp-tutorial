# 什么是ELF文件格式
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)、[静态库](../../01-C语言/18-静态库.md)、[动态库](../../01-C语言/19-动态库与共享库.md)、[CMake](../../01-C语言/23-CMake构建系统.md)

> "ELF=Linux世界的'集装箱标准'，所有可执行文件、动态库、目标文件都按这个格式打包。"——理解ELF，就是理解程序从源码到运行的完整链路。

***

### 1. 通俗理解

- **ELF** = Executable and Linkable Format（可执行与可链接格式），是Linux/Unix下的标准二进制文件格式
- 就像快递有统一的包装标准一样，Linux下所有二进制文件（可执行程序、.o目标文件、.so动态库）都按ELF格式打包
- 操作系统加载器只认ELF格式，链接器也只处理ELF格式——它是编译产物的"通用语言"

| 概念 | 类比 | 说明 |
|------|------|------|
| ELF Header | 集装箱的标签 | 标明这是什么类型的集装箱、多大多重 |
| Section | 集装箱里的小隔间 | 代码放一个隔间，数据放另一个隔间 |
| Program Header | 装卸指南 | 告诉码头工人怎么把集装箱搬上船（加载到内存） |
| Symbol Table | 货物清单 | 列出所有函数和变量的名字与位置 |

***

### 2. 技术说明

#### 1. ELF文件类型

| 类型 | 扩展名 | 说明 | 举例 |
|------|--------|------|------|
| 可重定位文件 | `.o` | 编译器产出，未经过链接 | `gcc -c main.c` → `main.o` |
| 可执行文件 | 无/`.out` | 链接后的完整程序，可直接运行 | `gcc main.c -o main` → `main` |
| 共享目标文件 | `.so` | 动态链接库，运行时加载 | `libstdc++.so`、`libc.so` |
| 核心转储文件 | `core` | 程序崩溃时的内存快照 | 段错误后生成的`core`文件 |

**判断文件类型**：

```bash
file main.o
# main.o: ELF 64-bit LSB relocatable, x86-64

file main
# main: ELF 64-bit LSB executable, x86-64

file /lib/x86_64-linux-gnu/libc.so.6
# /lib/x86_64-linux-gnu/libc.so.6: ELF 64-bit LSB shared object, x86-64

file core
# core: ELF 64-bit LSB core file, x86-64
```

#### 2. ELF文件整体结构

```
┌──────────────────────┐
│     ELF Header       │  ← 文件头：魔数、类型、架构、入口点等
├──────────────────────┤
│   Program Headers    │  ← 程序头表：告诉OS如何加载到内存（可执行文件才有）
├──────────────────────┤
│     .text            │  ← 代码段：机器指令
│     .rodata          │  ← 只读数据段：常量字符串等
│     .data            │  ← 已初始化数据段：全局/静态变量
│     .bss             │  ← 未初始化数据段：只记录大小，不占文件空间
│     .symtab          │  ← 符号表：函数和变量的名字与地址
│     .strtab          │  ← 字符串表：符号名等字符串
│     .rel.text        │  ← 代码重定位表
│     .rel.data        │  ← 数据重定位表
│     ...              │
├──────────────────────┤
│   Section Headers    │  ← 节头表：描述每个Section的属性（链接器使用）
└──────────────────────┘
```

#### 3. ELF Header详解

```bash
readelf -h main.o
```

**关键字段**：

| 字段 | 说明 | 示例值 |
|------|------|--------|
| Magic | 魔数，标识ELF文件 | `7f 45 4c 46`（\x7fELF） |
| Class | 32位/64位 | ELF64 |
| Data | 字节序 | Little endian |
| Type | 文件类型 | REL(可重定位)/EXEC(可执行)/DYN(共享)/CORE(核心转储) |
| Machine | 目标架构 | Advanced Micro Devices X86-64 |
| Entry point address | 程序入口点 | 0x401000（可执行文件） |
| Section header count | 节头数量 | 13 |
| Program header count | 程序头数量 | 9 |

#### 4. 常见段（Section）详解

| 段名 | 内容 | 属性 | 说明 |
|------|------|------|------|
| `.text` | 机器指令 | 可读可执行 | 编译后的函数代码 |
| `.rodata` | 常量数据 | 只读 | 字符串常量、const全局变量 |
| `.data` | 已初始化全局/静态变量 | 可读可写 | `int g = 42;` |
| `.bss` | 未初始化全局/静态变量 | 可读可写 | `int g;`（不占文件空间，加载时清零） |
| `.symtab` | 符号表 | — | 函数名、变量名及其地址 |
| `.strtab` | 字符串表 | — | 符号名的字符串存储 |
| `.rel.text` | 代码重定位信息 | — | 链接时修改代码中的地址 |
| `.rel.data` | 数据重定位信息 | — | 链接时修改数据中的地址 |
| `.debug` | 调试信息 | — | `-g`编译时生成 |
| `.comment` | 编译器版本信息 | — | GCC版本字符串 |

***

### 3. readelf与objdump实战

#### 1. readelf常用选项

| 选项 | 作用 | 命令 |
|------|------|------|
| `-h` | 查看ELF头 | `readelf -h main.o` |
| `-S` | 查看所有Section头 | `readelf -S main.o` |
| `-s` | 查看符号表 | `readelf -s main.o` |
| `-l` | 查看Program Header | `readelf -l main` |
| `-r` | 查看重定位表 | `readelf -r main.o` |
| `-d` | 查看动态段 | `readelf -d main` |

#### 2. objdump常用选项

| 选项 | 作用 | 命令 |
|------|------|------|
| `-d` | 反汇编代码段 | `objdump -d main.o` |
| `-r` | 显示重定位信息 | `objdump -r main.o` |
| `-t` | 显示符号表 | `objdump -t main.o` |
| `-h` | 显示Section头 | `objdump -h main.o` |
| `-s` | 显示所有Section内容 | `objdump -s main.o` |

#### 3. 实战：分析一个简单的目标文件

**源码**：

```c
#include <stdio.h>

int global_init = 42;
int global_uninit;

int add(int a, int b) {
    return a + b;
}

int main(void) {
    int result = add(1, 2);
    printf("result = %d\n", result);
    return 0;
}
```

**编译并分析**：

```bash
gcc -c main.c -o main.o
```

**查看ELF头**：

```bash
readelf -h main.o
```

```
Class:                             ELF64
Data:                              2's complement, little endian
Type:                              REL (Relocatable file)
Machine:                           Advanced Micro Devices X86-64
```

**查看Section头**：

```bash
readelf -S main.o
```

```
[Nr] Name          Type     Address          Off    Size
[ 0]               NULL     0000000000000000 000000 000000
[ 1] .text         PROGBITS 0000000000000000 000040 000035
[ 2] .data         PROGBITS 0000000000000000 000078 000004
[ 3] .bss          NOBITS   0000000000000000 00007c 000004
[ 4] .rodata       PROGBITS 0000000000000000 00007c 00000c
[ 5] .comment      PROGBITS 0000000000000000 000088 00002d
[ 6] .note.GNU-stack PROGBITS 0000000000000000 0000b5 000000
[ 7] .symtab       SYMTAB   0000000000000000 0000b8 0000a8
[ 8] .strtab       STRTAB   0000000000000000 000160 000034
```

**查看符号表**：

```bash
readelf -s main.o
```

```
Num:    Value          Size Type    Bind   Vis      Ndx Name
     0: 0000000000000000     0 NOTYPE  LOCAL  DEFAULT  UND
     1: 0000000000000000     4 OBJECT  GLOBAL DEFAULT    2 global_init
     2: 0000000000000000     4 OBJECT  GLOBAL DEFAULT    3 global_uninit
     3: 0000000000000000    22 FUNC    GLOBAL DEFAULT    1 add
     4: 0000000000000000    27 FUNC    GLOBAL DEFAULT    1 main
     5: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT  UND printf
```

**反汇编代码段**：

```bash
objdump -d main.o
```

```
Disassembly of section .text:

0000000000000000 <add>:
   0:   55                      push   %rbp
   1:   48 89 e5                mov    %rsp,%rbp
   4:   89 7d fc                mov    %edi,-0x4(%rbp)
   7:   89 75 f8                mov    %esi,-0x8(%rbp)
   a:   8b 55 fc                mov    -0x4(%rbp),%edx
   d:   8b 45 f8                mov    -0x8(%rbp),%eax
  10:   01 d0                   add    %edx,%eax
  12:   5d                      pop    %rbp
  13:   c3                      ret

0000000000000016 <main>:
  16:   55                      push   %rbp
  ...
```

**查看重定位表**：

```bash
readelf -r main.o
```

```
Relocation section '.rel.text' at offset 0x1a0 contains 2 entries:
  Offset          Info           Type           Symbol
  00000000001e   000500000004 R_X86_64_PLT32  printf
  000000000009   000a00000002 R_X86_64_PC32   .rodata
```

重定位表告诉我们：代码中引用了`printf`和`.rodata`，链接时需要填入实际地址。

***

### 4. ELF vs PE vs Mach-O对比

| 维度 | ELF | PE | Mach-O |
|------|-----|-----|--------|
| 全称 | Executable and Linkable Format | Portable Executable | Mach Object |
| 平台 | Linux/Unix/Android | Windows | macOS/iOS |
| 魔数 | `\x7fELF` | `MZ` | `\xfe\xed\xfa\xce`（32位）/ `\xfe\xed\xfa\xcf`（64位） |
| 文件扩展名 | 无/`.o`/`.so` | `.exe`/`.dll`/`.obj` | 无/`.dylib`/`.o` |
| 结构组织 | Section + Segment | Section + Section | Segment + Section |
| 动态链接 | `.so` + `dlopen` | `.dll` + `LoadLibrary` | `.dylib` + `dlopen` |
| 符号修饰 | 默认修饰 | MSVC修饰不同 | 默认修饰 |
| 调试格式 | DWARF | PDB | DWARF/DSYM |
| 分析工具 | readelf/objdump | dumpbin/PE Explorer | otool/pagestuff |

**共同点**：都有文件头、代码段、数据段、符号表、重定位表——只是格式和命名不同。

***

### 5. 常见问题

#### 1. 问题1：.data和.bss的区别

| 维度 | .data | .bss |
|------|-------|------|
| 内容 | 已初始化的全局/静态变量 | 未初始化的全局/静态变量 |
| 文件大小 | 占用文件空间 | 不占文件空间（只记录大小） |
| 加载时 | 从文件拷贝到内存 | 内存中清零 |
| 举例 | `int g = 42;` | `int g;` |

```c
int g_init = 42;     /* 放在.data段，占4字节文件空间 */
int g_uninit;        /* 放在.bss段，不占文件空间 */
static int s_init = 10;  /* 放在.data段 */
static int s_uninit;     /* 放在.bss段 */
```

#### 2. 问题2：Section和Segment的区别

| 维度 | Section | Segment |
|------|---------|---------|
| 视角 | 链接器视角 | 加载器（OS）视角 |
| 作用 | 链接时使用 | 运行时加载到内存使用 |
| 对应头 | Section Header | Program Header |
| 关系 | 多个Section映射到一个Segment | 一个Segment包含多个Section |

```
链接器视角（Section）：          加载器视角（Segment）：
┌──────────┐                   ┌──────────────────┐
│  .text   │ ──────────────→   │                  │
├──────────┤                   │  LOAD Segment    │
│  .rodata │ ──────────────→   │  (可读可执行)     │
├──────────┤                   │                  │
│  .data   │ ──────────────→   ├──────────────────┤
├──────────┤                   │  LOAD Segment    │
│  .bss    │ ──────────────→   │  (可读可写)       │
└──────────┘                   └──────────────────┘
```

#### 3. 问题3：为什么可执行文件有Program Header而.o文件没有

`.o`文件是可重定位文件，还没有被链接，地址全是0，不需要加载到内存——所以没有Program Header。链接后生成可执行文件时，链接器会创建Program Header告诉OS怎么加载。

***

### 6. 极简总结

**ELF是Linux下的标准二进制文件格式，包含四种文件类型（.o可重定位、可执行、.so共享、core转储），核心结构是ELF Header + Section Headers + Program Headers。.text存代码、.data存已初始化数据、.bss存未初始化数据。readelf看结构，objdump看反汇编。**

| 要点 | 一句话 |
|------|--------|
| ELF | Linux标准二进制格式——可执行文件、动态库、目标文件都按此格式 |
| 四种类型 | 可重定位(.o)、可执行、共享对象(.so)、核心转储(core) |
| .text | 代码段——存放编译后的机器指令 |
| .data vs .bss | .data占文件空间（已初始化），.bss不占文件空间（未初始化） |
| Section vs Segment | Section是链接器视角，Segment是加载器视角 |
| readelf | 查看ELF结构——-h头、-S段、-s符号、-l程序头 |
| objdump | 反汇编与重定位——-d反汇编、-r重定位 |

***

### 相关阅读

- [什么是符号表Symbol-Table](./11-什么是符号表Symbol-Table.md)
- [什么是重定位Relocation](./12-什么是重定位Relocation.md)