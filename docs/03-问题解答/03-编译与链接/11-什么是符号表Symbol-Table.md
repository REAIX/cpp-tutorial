# 什么是符号表Symbol Table
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)

> "符号表是连接人类可读名称与机器地址的桥梁，是编译链接的基石。" —— Linkers & Loaders

***

### 1. 核心速览

符号表（Symbol Table）是目标文件和可执行文件中存储符号（函数名、变量名等）与其对应地址、大小、类型等元数据的结构，是编译、链接、调试和动态加载的核心基础设施。

***

### 2. 符号的基本概念与类型

符号是程序中命名实体的抽象表示。每个符号在符号表中占据一条记录，包含名称、值（通常是地址）、大小和类型信息。

**符号类型分类**：

| 符号类型 | 说明 | 典型示例 | 绑定属性 |
|---------|------|---------|---------|
| 全局符号（Global） | 在本模块定义，可被其他模块引用 | 非static函数、非static全局变量 | STB_GLOBAL |
| 局部符号（Local） | 仅在本模块可见 | static函数、static全局变量 | STB_LOCAL |
| 弱符号（Weak） | 可被全局符号覆盖的符号 | `__attribute__((weak))` 声明的符号 | STB_WEAK |
| 未定义符号（Undefined） | 在本模块引用但在其他模块定义 | 外部函数调用、外部变量引用 | STB_GLOBAL |
| 段符号（Section） | 代表一个段的符号 | .text、.data 等 | STB_LOCAL |
| 文件符号（File） | 源文件名 | 编译单元的源文件路径 | STB_LOCAL |

```cpp
// symbol_types.cpp
int global_var = 42;
static int local_var = 10;

int global_func() { return global_var + local_var; }
static int local_func() { return local_var; }

extern int extern_var;
extern int extern_func();

__attribute__((weak)) int weak_func() { return 0; }

int caller() {
    return extern_func() + extern_var + weak_func();
}
```

编译后查看符号：

```bash
g++ -c symbol_types.cpp -o symbol_types.o
nm symbol_types.o
```

典型输出：

```
0000000000000000 T _Z11global_funcv
0000000000000000 D global_var
0000000000000000 t _Z10local_funcv
0000000000000004 d local_var
                 U extern_func
                 U extern_var
0000000000000000 W _Z9weak_funcv
000000000000001a T _Z6callerv
```

| nm 标识 | 含义 | 对应类型 |
|---------|------|---------|
| `T` | .text 段全局符号 | 全局函数 |
| `t` | .text 段局部符号 | static 函数 |
| `D` | .data 段全局符号 | 全局已初始化变量 |
| `d` | .data 段局部符号 | static 已初始化变量 |
| `B/b` | .bss 段符号 | 未初始化变量 |
| `U` | 未定义符号 | 外部引用 |
| `W` | 弱符号 | weak 声明 |
| `R/r` | .rodata 段符号 | 只读数据 |

***

### 3. .symtab 与 .dynsym 的区别

ELF 文件中存在两种符号表：`.symtab`（静态符号表）和 `.dynsym`（动态符号表）。

| 维度 | .symtab | .dynsym |
|------|---------|---------|
| **用途** | 静态链接和调试 | 动态链接 |
| **内容** | 所有符号（含局部符号） | 仅动态链接所需符号 |
| **大小** | 较大 | 较小 |
| **是否可strip** | 可以（strip 后不影响运行） | 不可以（运行时必需） |
| **段属性** | SHT_SYMTAB | SHT_DYNSYM |
| **关联字符串表** | .strtab | .dynstr |
| **加载到内存** | 否 | 是 |

```bash
# 查看两种符号表
readelf -S a.out | grep -E "symtab|dynsym"

# 输出示例：
# [Nr] Name      Type     Address          Off    Size
# [ 5] .dynsym   DYNSYM   00000000000002b8 0002b8 000060
# [28] .symtab   SYMTAB   0000000000000000 001c90 000690
```

**符号表条目结构**（Elf64_Sym）：

```c
typedef struct {
    Elf64_Word    st_name;    // 符号名在字符串表中的偏移
    unsigned char st_info;    // 符号类型和绑定属性
    unsigned char st_other;   // 可见性
    Elf64_Half    st_shndx;   // 所在段索引
    Elf64_Addr    st_value;   // 符号值（通常是地址）
    Elf64_Xword   st_size;    // 符号大小
} Elf64_Sym;
```

**st_info 编码**：

```c
#define ELF64_ST_BIND(i)    ((i) >> 4)
#define ELF64_ST_TYPE(i)    ((i) & 0xf)
#define ELF64_ST_INFO(b, t) (((b) << 4) + ((t) & 0xf))
```

| st_type 值 | 宏名 | 含义 |
|-----------|------|------|
| 0 | STT_NOTYPE | 未指定类型 |
| 1 | STT_OBJECT | 数据对象（变量） |
| 2 | STT_FUNC | 函数 |
| 3 | STT_SECTION | 段 |
| 4 | STT_FILE | 源文件名 |
| 10 | STT_GNU_IFUNC | 间接函数（GNU 扩展） |

| st_bind 值 | 宏名 | 含义 |
|-----------|------|------|
| 0 | STB_LOCAL | 局部符号 |
| 1 | STB_GLOBAL | 全局符号 |
| 2 | STB_WEAK | 弱符号 |

***

### 4. nm 与 readelf -s 实战

**nm 命令详解**：

```bash
# 查看所有符号
nm a.out

# 查看动态符号
nm -D a.out

# 显示符号大小
nm -S a.out

# 按地址排序
nm -n a.out

# 显示段索引
nm -f sysv a.out

# 仅显示未定义符号
nm -u a.out

# C++ 符号反混淆
nm --demangle a.out

# 查看目标文件符号（含段信息）
nm -a symbol_types.o
```

**readelf -s 详解**：

```bash
# 查看完整符号表
readelf -s a.out

# 查看动态符号表
readelf --dyn-syms a.out

# 输出示例：
#    Num:    Value          Size Type    Bind   Vis      Ndx Name
#      1: 0000000000000000     0 FILE    LOCAL  DEFAULT    ABS symbol_types.cpp
#      2: 0000000000000000     0 SECTION LOCAL  DEFAULT      1 .text
#      3: 0000000000000000    24 FUNC    LOCAL  DEFAULT      1 _Z10local_funcv
#      4: 0000000000000004     4 OBJECT  LOCAL  DEFAULT      3 local_var
#      5: 0000000000000000     4 OBJECT  GLOBAL DEFAULT      3 global_var
#      6: 0000000000000018    30 FUNC    GLOBAL DEFAULT      1 _Z11global_funcv
#      7: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT    UND extern_func
#      8: 0000000000000000     0 NOTYPE  GLOBAL DEFAULT    UND extern_var
#      9: 0000000000000000    11 FUNC    WEAK   DEFAULT      1 _Z9weak_funcv
```

**readelf -s 字段解读**：

| 字段 | 含义 |
|------|------|
| Num | 符号表索引 |
| Value | 符号地址（目标文件中是段内偏移） |
| Size | 符号占用的字节数 |
| Type | NOTYPE/OBJECT/FUNC/SECTION/FILE |
| Bind | LOCAL/GLOBAL/WEAK |
| Vis | DEFAULT/HIDDEN/PROTECTED/INTERNAL |
| Ndx | 段索引（UND=未定义，ABS=绝对值） |

**实战：定位链接错误**：

```bash
# 编译时出现 undefined reference
g++ main.o -o main
# main.o: undefined reference to `missing_func()'

# 检查 main.o 需要哪些符号
nm -u main.o
#                  U _Z12missing_funcv

# 检查库文件是否提供该符号
nm -D libmylib.so | grep missing_func

# 如果找不到，检查是否被 C++ name mangling 影响
nm --demangle -D libmylib.so | grep missing
```

***

### 5. strip 与符号剥离

strip 用于移除可执行文件中的符号信息，减小文件体积，但不影响程序运行。

```bash
# 剥离所有符号信息
strip a.out

# 仅剥离调试信息
strip --strip-debug a.out

# 仅剥离局部符号
strip --strip-unneeded a.out

# 保留动态符号表，仅移除 .symtab
strip --strip-all a.out

# 查看剥离前后对比
ls -l a.out          # 剥离前
strip a.out
ls -l a.out          # 剥离后
```

**strip 剥离内容对比**：

| 操作 | 移除的段 | 保留的段 | 效果 |
|------|---------|---------|------|
| `strip` (默认) | .symtab, .strtab, 调试段 | .dynsym, .dynstr | 可运行，不可调试 |
| `strip --strip-debug` | .debug_* 段 | .symtab, .dynsym | 可调试（无源码级） |
| `strip --strip-unneeded` | .symtab 中非必要符号 | .dynsym | 链接不需要的都移除 |

**strip 前后对比实战**：

```bash
# 编译带调试信息
g++ -g -o app_debug main.cpp
ls -l app_debug
# -rwxr-xr-x 1 user user 24568 ...

# 完全剥离
cp app_debug app_stripped
strip app_stripped
ls -l app_stripped
# -rwxr-xr-x 1 user user 6320 ...

# 查看差异
readelf -S app_debug | wc -l    # ~40 段
readelf -S app_stripped | wc -l # ~25 段
```

| 场景 | 建议 |
|------|------|
| 开发阶段 | 保留所有符号和调试信息 |
| 测试阶段 | strip --strip-debug |
| 发布阶段 | strip（仅保留 .dynsym） |
| 崩溃分析 | 保留未 strip 版本 + 单独保存符号文件 |

**分离调试信息**（推荐做法）：

```bash
# 将调试信息保存到单独文件
objcopy --only-keep-debug app app.debug
strip --strip-debug app
objcopy --add-gnu-debuglink=app.debug app

# GDB 自动加载调试信息
gdb ./app
```

> ⚠️ **平台注意**：Windows MSVC 使用 PDB 文件存储调试信息，二进制文件本身不包含调试段。`strip` 是 Linux/ELF 工具，Windows 上对应功能由链接器选项 `/DEBUG` / `/PDB` 控制。

***

### 6. 符号版本（Symbol Versioning）

符号版本是 GNU/Linux 动态链接器提供的机制，允许同一个库中同一函数存在多个版本，实现 ABI 兼容性。

**版本脚本示例**：

```
# libfoo.map
LIBFOO_1.0 {
    global:
        foo_init;
        foo_process;
    local:
        *;
};

LIBFOO_1.1 {
    global:
        foo_process;
} LIBFOO_1.0;

LIBFOO_1.2 {
    global:
        foo_process_v2;
        foo_process;
} LIBFOO_1.1;
```

```cpp
// libfoo.cpp
extern "C" {

void foo_init() {}

__asm__(".symver foo_process_v1,foo_process@LIBFOO_1.0");
void foo_process_v1() {}

__asm__(".symver foo_process_v2,foo_process@@LIBFOO_1.2");
void foo_process_v2() {}

}
```

```bash
# 编译带版本脚本
g++ -shared -Wl,--version-script=libfoo.map -o libfoo.so libfoo.cpp

# 查看符号版本
readelf -V libfoo.so
nm -D libfoo.so | grep foo_process
# 0000000000001119 T foo_process@@LIBFOO_1.2
# 0000000000001109 T foo_process@LIBFOO_1.0
```

| 版本语法 | 含义 |
|---------|------|
| `@@VERSION` | 默认版本（链接时使用） |
| `@VERSION` | 非默认版本（兼容旧版） |

**glibc 中的版本示例**：

```bash
# 查看 glibc 中 memcpy 的版本
nm -D /lib/x86_64-linux-gnu/libc.so.6 | grep memcpy
# 000000000008c140 W memcpy@@GLIBC_2.2.5
# 000000000008c140 W memcpy@GLIBC_2.2.5
```

> ⚠️ **平台注意**：符号版本是 GNU/Linux（ELF）特有的机制。macOS 使用 install_name 和 compatibility_version。Windows DLL 使用序号（ordinal）或名称导出，无版本概念。

***

### 7. 符号可见性（Visibility）

符号可见性控制符号在共享库中的导出行为，影响动态链接和符号解析。

**四种可见性级别**：

| 可见性 | 说明 | 效果 |
|--------|------|------|
| STV_DEFAULT | 默认可见性 | 可被其他模块引用，可被抢占 |
| STV_HIDDEN | 隐藏 | 不导出到动态符号表，不可被外部引用 |
| STV_PROTECTED | 受保护 | 导出但不可被外部符号抢占 |
| STV_INTERNAL | 内部 | 最严格，等同于隐藏 |

**使用 `__attribute__((visibility))` 控制**：

```cpp
class __attribute__((visibility("default"))) PublicAPI {
public:
    PublicAPI();
    virtual ~PublicAPI();

    void public_method();

private:
    __attribute__((visibility("hidden"))) void internal_method();
    struct Impl;
    Impl *pImpl_;
};

__attribute__((visibility("hidden"))) void internal_helper() {}

__attribute__((visibility("default"))) void exported_function() {}

class __attribute__((visibility("hidden"))) InternalClass {
};
```

**编译器全局控制**：

```bash
# 默认隐藏所有符号
g++ -fvisibility=hidden -shared -o libfoo.so foo.cpp

# 仅导出显式标记的符号
g++ -fvisibility=hidden -fvisibility-inlines-hidden -shared -o libfoo.so foo.cpp
```

**可见性对动态链接的影响**：

```bash
# 编译为共享库
g++ -fvisibility=hidden -shared -o libfoo.so foo.cpp

# 查看导出符号（应该很少）
nm -D libfoo.so | grep T

# 对比默认可见性
g++ -shared -o libfoo_default.so foo.cpp
nm -D libfoo_default.so | grep T | wc -l
```

| 可见性策略 | 优点 | 缺点 |
|-----------|------|------|
| 全部导出（默认） | 简单 | 符号冲突、二进制体积大 |
| `-fvisibility=hidden` | 减小体积、避免冲突 | 需显式标记导出 |
| 版本脚本 | 精确控制 | 维护成本高 |
| 导出映射文件 | 平台无关 | 需要额外文件 |

> ⚠️ **平台注意**：Windows DLL 默认行为类似 `-fvisibility=hidden`，需要 `__declspec(dllexport)` 显式导出。Linux/ELF 默认全部导出。跨平台库建议使用 `-fvisibility=hidden` + 显式标记，保持行为一致。

***

### 8. C++ Name Mangling 与符号反混淆

C++ 支持函数重载、命名空间、模板等特性，编译器通过 Name Mangling 将这些信息编码到符号名中。

**Mangling 规则示例**：

```cpp
namespace myns {
    void func(int) {}
    void func(double) {}
    void func(int, double) {}

    template<typename T>
    void tpl_func(T) {}

    class MyClass {
    public:
        void method() {}
        static void static_method() {}
    };
}
```

```bash
# 编译查看 mangled 符号
g++ -c mangling.cpp -o mangling.o
nm mangling.o

# 输出：
# 0000000000000000 T _ZN4myns4funcEi
# 000000000000000a T _ZN4myns4funcEd
# 0000000000000014 T _ZN4myns4funcEid
# 000000000000001e T _ZN4myns8tpl_funcIiEEvT_
# 0000000000000028 T _ZN4myns7MyClass6methodEv
# 0000000000000032 T _ZN4myns7MyClass13static_methodEv
```

**Mangling 解码**：

| Mangled 名 | 解码 |
|-----------|------|
| `_ZN4myns4funcEi` | `myns::func(int)` |
| `_ZN4myns4funcEd` | `myns::func(double)` |
| `_ZN4myns4funcEid` | `myns::func(int, double)` |
| `_ZN4myns8tpl_funcIiEEvT_` | `myns::tpl_func<int>(int)` |
| `_ZN4myns7MyClass6methodEv` | `myns::MyClass::method()` |

**反混淆工具**：

```bash
# c++filt
echo "_ZN4myns4funcEi" | c++filt
# myns::func(int)

# nm --demangle
nm --demangle mangling.o

# readelf -s 自动 demangle（部分版本）
readelf -sW mangling.o | c++filt
```

**C 链接避免 Mangling**：

```cpp
extern "C" {
    void c_func(int x) {}
}

class Wrapper {
public:
    extern "C" static void callback(void *ctx) {}
};
```

```bash
nm c_linkage.o
# 0000000000000000 T c_func
# 0000000000000000 T callback
```

***

### 9. 弱符号与符号抢占

弱符号允许同一个符号存在多个定义，链接器选择全局定义而忽略弱定义。

```cpp
// lib.cpp - 提供默认实现
#include <cstdio>

__attribute__((weak)) void log_message(const char *msg) {
    printf("[DEFAULT] %s\n", msg);
}

void do_work() {
    log_message("starting work");
}
```

```cpp
// main.cpp - 覆盖弱符号
#include <cstdio>

extern void do_work();

void log_message(const char *msg) {
    printf("[CUSTOM] %s\n", msg);
}

int main() {
    do_work();
    return 0;
}
```

```bash
g++ -shared -fPIC lib.cpp -o libweak.so
g++ main.cpp -L. -lweak -o main
LD_LIBRARY_PATH=. ./main
# [CUSTOM] starting work
```

**弱符号应用场景**：

| 场景 | 说明 |
|------|------|
| 默认实现 | 库提供弱符号默认实现，用户可覆盖 |
| 插件钩子 | 定义弱符号钩子，无插件时使用空实现 |
| ABI 兼容 | 新版本添加弱符号，旧版本不提供则使用默认 |
| 可选依赖 | 检测弱符号是否被解析判断功能可用性 |

**运行时检测弱符号**：

```cpp
extern "C" __attribute__((weak)) void optional_feature();

int main() {
    if (optional_feature) {
        optional_feature();
    } else {
        printf("optional feature not available\n");
    }
    return 0;
}
```

**符号抢占（Preemption）**：

在动态链接中，全局符号可能被另一个共享库的同名符号抢占：

```bash
# 控制符号抢占行为
g++ -Wl,-Bsymbolic -shared -o libfoo.so foo.cpp

# -Bsymbolic: 优先引用本库内的符号，不抢占
# -Bsymbolic-functions: 仅对函数生效
```

***

### 10. 符号表与调试

符号表是源码级调试的基础。没有符号表，调试器只能显示机器码地址。

**GDB 使用符号表**：

```bash
# 加载带符号的可执行文件
gdb ./app_debug

# 查看函数地址
(gdb) info functions
(gdb) info functions myns::
(gdb) info address main

# 查看全局变量
(gdb) info variables
(gdb) info variables global_var

# 查看符号类型
(gdb) ptype MyClass
(gdb) ptype global_var

# 设置断点（依赖符号）
(gdb) break main
(gdb) break file.cpp:42
```

**调试符号（DWARF）**：

符号表（.symtab）提供名称-地址映射，DWARF 调试信息（.debug_* 段）提供源码行号、变量类型、栈帧信息等。

| 调试段 | 内容 |
|--------|------|
| .debug_info | 核心调试信息（类型、变量、函数描述） |
| .debug_abbrev | .debug_info 的缩写表 |
| .debug_line | 源码行号映射 |
| .debug_str | 字符串表 |
| .debug_ranges | 地址范围列表 |
| .debug_frame | 栈帧信息 |

```bash
# 查看调试信息段
readelf -S app_debug | grep debug

# 查看 DWARF 信息
readelf --debug-dump=info app_debug | head -50

# 查看行号信息
readelf --debug-dump=line app_debug | head -30
```

**编译选项与调试信息**：

| 选项 | 说明 | 体积影响 |
|------|------|---------|
| `-g` | 生成标准调试信息 | 基准 |
| `-g3` | 包含宏定义信息 | 更大 |
| `-g1` | 最小调试信息（函数和行号） | 较小 |
| `-gsplit-dwarf` | 分离调试信息（DWO） | 减小目标文件 |
| `-ggnu-pubnames` | GNU pubnames 加速查找 | 略大 |

> ⚠️ **平台注意**：MSVC 使用 PDB（Program Database）格式存储调试信息，而非 DWARF。PDB 是独立文件，不嵌入二进制。GDB 不支持 PDB，WinDbg 不支持 DWARF。跨平台项目需分别生成调试信息。

***

### 11. 实战：排查符号相关链接问题

**场景 1：undefined reference**

```bash
# 错误信息
# undefined reference to `MyClass::process()'

# 步骤1：确认目标文件中确实引用了该符号
nm -u main.o | grep process

# 步骤2：检查库文件是否提供该符号
nm -D libmylib.so | grep process
# 如果找不到，检查 C++ mangling
nm -D libmylib.so | c++filt | grep process

# 步骤3：检查符号可见性
readelf -s libmylib.so | grep process
# 如果 BIND 是 LOCAL，说明符号被隐藏了

# 步骤4：检查链接顺序
g++ main.o -lmylib -o main   # 正确：库在目标文件之后
g++ -lmylib main.o -o main   # 错误：库在目标文件之前
```

**场景 2：multiple definition**

```bash
# 错误信息
# multiple definition of `global_var'

# 原因：头文件中定义了非 inline 非 static 的全局变量
# 解决方案1：使用 extern 声明 + 单独定义
# 解决方案2：使用 inline 变量（C++17）
# 解决方案3：使用 static（但每个编译单元有独立副本）
```

```cpp
// header.h - 错误写法
int global_var = 42;

// header.h - 正确写法1
extern int global_var;

// header.h - 正确写法2 (C++17)
inline int global_var = 42;

// header.h - 正确写法3（每个TU独立副本）
static int global_var = 42;
```

**场景 3：符号版本不匹配**

```bash
# 错误信息
# version `GLIBC_2.28' not found

# 检查程序需要的符号版本
readelf -V app | grep GLIBC
objdump -T app | grep GLIBC_2.28

# 检查系统 glibc 支持的版本
strings /lib/x86_64-linux-gnu/libc.so.6 | grep GLIBC
```

**场景 4：运行时符号查找失败**

```bash
# 错误信息
# error while loading shared libraries: libfoo.so: cannot open shared object file

# 检查动态库依赖
ldd app

# 检查运行时搜索路径
LD_DEBUG=libs ./app

# 查看运行时符号解析过程
LD_DEBUG=symbols ./app
```

***

### 12. 极简总结

| 概念 | 要点 |
|------|------|
| **符号类型** | 全局/局部/弱/未定义/段/文件，各有不同绑定属性 |
| **.symtab** | 静态符号表，含所有符号，可 strip |
| **.dynsym** | 动态符号表，仅含动态链接所需符号，不可 strip |
| **nm** | 快速查看符号，支持 demangle、排序、过滤 |
| **readelf -s** | 查看完整符号信息，含类型/绑定/可见性/段索引 |
| **strip** | 移除符号信息减小体积，推荐分离调试信息 |
| **符号版本** | GNU 扩展，支持同函数多版本，保证 ABI 兼容 |
| **可见性** | hidden/protected/default，控制符号导出和抢占 |
| **Name Mangling** | C++ 符号编码，c++filt 反混淆 |
| **弱符号** | 可被全局符号覆盖，用于默认实现和可选功能 |
| **调试** | .symtab 提供名称映射，DWARF 提供源码级信息 |

**关键记忆**：
- 符号表是名称到地址的映射，是链接器的"通讯录"
- `.symtab` 可删，`.dynsym` 不可删
- 发布时 strip，但务必保存调试符号文件
- `-fvisibility=hidden` 是共享库的最佳实践
- C++ Name Mangling 是链接错误的常见根源，善用 `c++filt`
- 弱符号是实现可选功能和默认实现的优雅方式

***

### 相关阅读

- [什么是名称修饰Name-Mangling](./09-什么是名称修饰Name-Mangling.md)
- [什么是ELF文件格式](./06-什么是ELF文件格式.md)
- [什么是重定位Relocation](./12-什么是重定位Relocation.md)