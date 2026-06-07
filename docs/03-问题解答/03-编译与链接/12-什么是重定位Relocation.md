# 什么是重定位Relocation
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)

> "重定位是链接器的核心使命——将零散的目标文件缝合为可执行的完整程序。" —— John R. Levine

***

### 1. 要点直击

重定位（Relocation）是链接器和动态加载器修正代码和数据中地址引用的过程，将编译时无法确定的符号地址替换为运行时的实际地址，使程序能够正确执行。

***

### 2. 为什么需要重定位

编译器在编译单个源文件时，无法知道外部符号的最终地址。它会在目标文件中留下"占位符"，由链接器在合并所有目标文件后填充真实地址。

```
编译阶段：
  main.o:   call <printf的地址=???>     ← 编译器不知道 printf 在哪
  utils.o:  mov <global_var的地址=???>   ← 编译器不知道 global_var 在哪

链接阶段：
  链接器确定所有符号的最终地址，修正占位符：
  call 0x401050                   ← printf 的真实地址
  mov 0x404020                    ← global_var 的真实地址
```

**需要重定位的根本原因**：

| 原因 | 说明 |
|------|------|
| 分离编译 | 每个源文件独立编译，看不到其他文件的符号地址 |
| 地址不确定 | 编译器不知道目标文件最终被加载到哪个地址 |
| 动态链接 | 共享库在运行时才加载，地址运行时才确定 |
| ASLR | 操作系统随机化加载地址，编译时无法预知 |

```cpp
// a.cpp
extern int shared_var;
extern void helper();

int main() {
    shared_var = 42;
    helper();
    return 0;
}
```

```cpp
// b.cpp
int shared_var = 0;

void helper() {
    shared_var++;
}
```

```bash
# 编译但不链接
g++ -c a.cpp -o a.o
g++ -c b.cpp -o b.o

# 查看重定位条目
readelf -r a.o
```

输出示例：

```
Relocation section '.rela.text' at offset 0x200 contains 2 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
  00000000000a    000a00000001   R_X86_64_64       0000000000000000 shared_var + 0
  000000000010    000b00000004   R_X86_64_PLT32    0000000000000000 helper - 4
```

***

### 3. 重定位条目结构

每个重定位条目描述一个需要修正的地址引用。

**Elf64_Rel 结构**：

```c
typedef struct {
    Elf64_Addr r_offset;    // 需要修正的位置偏移
    Elf64_Xword r_info;     // 符号索引 + 重定位类型
} Elf64_Rel;

typedef struct {
    Elf64_Addr r_offset;    // 需要修正的位置偏移
    Elf64_Xword r_info;     // 符号索引 + 重定位类型
    Elf64_Sxword r_addend;  // 加数
} Elf64_Rela;
```

**r_info 编码**：

```c
#define ELF64_R_SYM(i)  ((i) >> 32)    // 高32位：符号表索引
#define ELF64_R_TYPE(i) ((i) & 0xffffffff) // 低32位：重定位类型
#define ELF64_R_INFO(s, t) (((Elf64_Xword)(s) << 32) + (t))
```

**重定位条目字段解读**：

| 字段 | 含义 |
|------|------|
| `r_offset` | 需要修正的位置（目标文件中是段内偏移，可执行文件中是虚拟地址） |
| `r_info` | 高32位指向符号表中的符号索引，低32位指定重定位类型 |
| `r_addend` | 仅 RELA 格式有，参与地址计算的加数 |

**REL vs RELA**：

| 格式 | 隐式加数 | 显式加数 | 使用场景 |
|------|---------|---------|---------|
| REL | 从被修正位置读取 | 无 | 旧格式，x86 32位常用 |
| RELA | 不读取 | r_addend | 新格式，x86_64 必须使用 |

```bash
# 查看 RELA 重定位
readelf -r a.o

# 查看重定位段信息
readelf -S a.o | grep rela
```

***

### 4. x86_64 重定位类型详解

x86_64 架构定义了多种重定位类型，每种对应不同的寻址方式和修正方法。

**核心重定位类型**：

| 类型 | 值 | 计算 | 适用场景 |
|------|---|------|---------|
| R_X86_64_64 | 1 | S + A | 绝对64位地址 |
| R_X86_64_PC32 | 2 | S + A - P | PC相对32位偏移 |
| R_X86_64_PLT32 | 4 | L + A - P | PLT 相对32位偏移 |
| R_X86_64_32 | 10 | S + A | 绝对32位地址 |
| R_X86_64_32S | 11 | S + A | 符号扩展32位地址 |
| R_X86_64_GOTPCRELX | 41 | G + A - P | GOT 间接寻址（优化） |
| R_X86_64_REX_GOTPCRELX | 42 | G + A - P | GOT 间接寻址（REX前缀） |

**计算公式中的符号**：

| 符号 | 含义 |
|------|------|
| S | 符号的实际地址（Symbol value） |
| A | 加数（Addend） |
| P | 被修正位置的地址（Place of relocation） |
| L | PLT 条目的地址 |
| G | GOT 条目的地址 |

**R_X86_64_64：绝对地址重定位**

```cpp
// 生成绝对地址引用的场景
extern int global_var;

int *get_ptr() {
    return &global_var;
}
```

```bash
g++ -c ref.cpp -o ref.o
objdump -d ref.o
```

反汇编示例：

```
get_ptr():
   0:   48 c7 c0 00 00 00 00    mov    $0x0,%rax    # 00 00 00 00 待修正
                        3: R_X86_64_64 global_var
```

链接后修正为：

```
get_ptr():
   0:   48 c7 c0 20 40 40 00    mov    $0x404020,%rax
```

**R_X86_64_PC32：PC相对地址重定位**

```cpp
extern void helper();

void caller() {
    helper();
}
```

反汇编示例：

```
caller():
   0:   e9 00 00 00 00    jmp    5 <caller()+0x5>
                1: R_X86_64_PC32 helper-4
```

修正计算：`S + A - P = helper_addr + (-4) - (0x401001)`

**R_X86_64_PLT32：PLT相对重定位**

```cpp
extern void dynamic_func();

void caller() {
    dynamic_func();
}
```

反汇编示例：

```
caller():
   0:   e9 00 00 00 00    jmp    5
                1: R_X86_64_PLT32 dynamic_func-4
```

PLT32 与 PC32 的区别在于：PLT32 通过过程链接表（PLT）间接跳转，用于动态链接的函数调用。

***

### 5. 静态链接中的重定位

静态链接时，链接器将多个目标文件合并，确定每个段的最终地址，然后修正所有重定位引用。

**静态链接重定位流程**：

```
1. 空间分配：为每个段分配虚拟地址空间
   .text:  0x401000 - 0x401100
   .data:  0x402000 - 0x402100
   .bss:   0x402100 - 0x402200

2. 符号解析：确定每个符号的最终地址
   main:       0x401000
   helper:     0x401050
   global_var: 0x402000

3. 重定位修正：遍历所有重定位条目，计算并填入真实地址
```

**完整示例**：

```cpp
// main.cpp
#include <cstdio>

extern int compute(int x);
int result;

int main() {
    result = compute(42);
    printf("result = %d\n", result);
    return 0;
}
```

```cpp
// compute.cpp
int compute(int x) {
    return x * x + x;
}
```

```bash
# 编译
g++ -c main.cpp -o main.o
g++ -c compute.cpp -o compute.o

# 查看重定位条目
readelf -r main.o

# 静态链接
g++ main.o compute.o -o app

# 验证符号地址
nm app | grep -E "main|compute|result"
```

**readelf -r 输出解读**：

```
Relocation section '.rela.text' at offset 0x230 contains 3 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
  000000000015    000800000004   R_X86_64_PLT32    0000000000000000 _Z7computei - 4
  000000000027    000900000004   R_X86_64_PLT32    0000000000000000 printf - 4
  000000000031    000a00000001   R_X86_64_64       0000000000000000 result + 0
```

| 条目 | 含义 |
|------|------|
| 偏移0x15 | `compute` 函数调用，PLT32 类型 |
| 偏移0x27 | `printf` 函数调用，PLT32 类型 |
| 偏移0x31 | `result` 变量引用，绝对64位地址 |

**链接器内部处理伪代码**：

```cpp
for (auto &rel : relocations) {
    uint64_t P = section_base + rel.r_offset;
    uint64_t S = symbol_table[ELF64_R_SYM(rel.r_info)].value;
    int64_t A = rel.r_addend;
    uint32_t type = ELF64_R_TYPE(rel.r_info);

    switch (type) {
    case R_X86_64_64:
        write64(P, S + A);
        break;
    case R_X86_64_PC32:
    case R_X86_64_PLT32:
        write32(P, S + A - P);
        break;
    case R_X86_64_32:
        write32(P, S + A);
        break;
    case R_X86_64_32S:
        write32(P, S + A);
        break;
    }
}
```

***

### 6. 动态链接中的重定位：PLT/GOT

动态链接的重定位比静态链接复杂得多，因为共享库的地址在运行时才确定。

**PLT/GOT 机制概览**：

```
┌──────────────────┐     ┌──────────────────┐
│  .plt 段          │     │  .got.plt 段      │
│  (代码，跳转桩)    │     │  (数据，地址表)    │
│                   │     │                  │
│  printf@plt:      │     │  [0] 动态链接器地址 │
│    jmp *GOT[1]    │────▶│  [1] printf GOT   │──▶ 初始: 指向下一行
│    push index     │     │  [2] helper GOT   │    解析后: 指向真实地址
│    jmp resolver   │     │  ...              │
└──────────────────┘     └──────────────────┘
```

**延迟绑定（Lazy Binding）**：

1. 首次调用 `printf@plt`：跳转到 GOT 条目，GOT 初始值指向 PLT 下一条指令
2. 压入符号索引，跳转到动态链接器解析函数
3. 动态链接器查找 `printf` 真实地址，写入 GOT
4. 后续调用直接通过 GOT 跳转，无需再解析

```cpp
// 动态链接重定位示例
#include <cstdio>
#include <cstring>

int main() {
    char buf[64];
    strcpy(buf, "hello");
    printf("%s\n", buf);
    return 0;
}
```

```bash
# 编译为动态链接
g++ -o app_dyn main.cpp

# 查看 PLT
objdump -d -j .plt app_dyn

# 查看 GOT
objdump -d -j .got.plt app_dyn

# 查看动态重定位条目
readelf -r app_dyn | grep -E "JUMP_SLOT|GLOB_DAT"
```

**动态重定位类型**：

| 类型 | 值 | 说明 |
|------|---|------|
| R_X86_64_JUMP_SLOT | 7 | PLT/GOT 跳转槽（函数调用） |
| R_X86_64_GLOB_DAT | 6 | 全局数据引用 |
| R_X86_64_RELATIVE | 8 | 基址重定位（加载时修正） |

```bash
# readelf -r app_dyn 输出示例
Relocation section '.rela.dyn' at offset 0x4a0 contains 4 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
  000000403ff0    000000000008   R_X86_64_RELATIVE                    0
  000000403ff8    000000000008   R_X86_64_RELATIVE                    0
  000000404000    000200000006   R_X86_64_GLOB_DAT  0000000000000000 strcpy + 0
  000000404008    000300000006   R_X86_64_GLOB_DAT  0000000000000000 printf + 0

Relocation section '.rela.plt' at offset 0x500 contains 2 entries:
  Offset          Info           Type           Sym. Value    Sym. Name + Addend
  000000404018    000200000007   R_X86_64_JUMP_SLOT  0000000000000000 strcpy + 0
  000000404020    000300000007   R_X86_64_JUMP_SLOT  0000000000000000 printf + 0
```

| 重定位类型 | 修正时机 | 性能影响 |
|-----------|---------|---------|
| R_X86_64_RELATIVE | 加载时 | 低（仅加基址） |
| R_X86_64_GLOB_DAT | 加载时 | 中（查符号表） |
| R_X86_64_JUMP_SLOT | 首次调用时（延迟绑定） | 首次高，后续无 |

***

### 7. 位置无关代码（PIC）与重定位

位置无关代码（Position-Independent Code）通过相对寻址避免绝对地址引用，使共享库可加载到任意地址。

**PIC vs 非PIC**：

| 维度 | PIC | 非PIC |
|------|-----|-------|
| **编译选项** | `-fPIC` | 默认 |
| **寻址方式** | GOT 间接寻址 | 绝对地址 |
| **重定位数量** | 少（仅 GOT 条目） | 多（每个引用都需修正） |
| **代码段共享** | 可（只读） | 不可（需写回修正） |
| **性能** | 略慢（多一次 GOT 查找） | 略快 |
| **适用场景** | 共享库 | 可执行文件 |

**PIC 下的函数调用**：

```cpp
// -fPIC 编译
extern void external_func();

void my_func() {
    external_func();
}
```

非PIC 生成代码：

```
my_func():
    call external_func          # R_X86_64_PLT32，需重定位
```

PIC 生成代码：

```
my_func():
    call external_func@PLT      # 通过 PLT/GOT 间接调用
                               # 仅 GOT 条目需重定位
```

**PIC 下的全局变量访问**：

```cpp
// -fPIC 编译
extern int global_var;

int read_global() {
    return global_var;
}
```

非PIC 生成代码：

```
read_global():
    mov global_var(%rip), %eax    # R_X86_64_PC32，需重定位 .text
```

PIC 生成代码：

```
read_global():
    mov global_var@GOTPCREL(%rip), %rax  # 获取 GOT 条目地址
    mov (%rax), %eax                      # 通过 GOT 间接读取
```

```bash
# 编译 PIC 共享库
g++ -fPIC -shared lib.cpp -o libfoo.so

# 查看 PIC 重定位（应该只有 GOT/PLT 相关）
readelf -r libfoo.so

# 对比非 PIC
g++ -shared lib.cpp -o libfoo_nopic.so
readelf -r libfoo_nopic.so | wc -l
```

**GOT 查找过程**：

```
1. 通过 RIP 相对寻址找到 GOT 条目地址
2. 从 GOT 条目读取全局变量的真实地址
3. 通过该地址访问全局变量

   代码段:                     GOT:
   mov foo@GOTPCREL(%rip),%rax ──▶ [foo的GOT条目] ──▶ foo的真实地址
   mov (%rax),%eax                                         │
                                                           ▼
                                                      foo 的内存
```

> ⚠️ **平台注意**：x86_64 的 RIP 相对寻址使 PIC 开销很小。32 位 x86 的 PIC 需通过 EBX 寄存器访问 GOT，开销较大。ARM64 使用 ADRP+LDR 组合实现 PIC。Windows DLL 默认不使用 PIC，依赖重定位表修正。

***

### 8. readelf -r 实战分析

**完整分析流程**：

```bash
# 1. 编译目标文件
g++ -c main.cpp -o main.o

# 2. 查看目标文件重定位
readelf -r main.o

# 3. 链接为可执行文件
g++ main.o -o main

# 4. 查看可执行文件重定位（仅动态重定位）
readelf -r main

# 5. 查看重定位段
readelf -S main.o | grep rela
```

**目标文件重定位 vs 可执行文件重定位**：

| 维度 | 目标文件（.o） | 可执行文件 | 共享库（.so） |
|------|--------------|-----------|-------------|
| 重定位段 | .rela.text, .rela.data | .rela.dyn, .rela.plt | .rela.dyn, .rela.plt |
| r_offset | 段内偏移 | 虚拟地址 | 虚拟地址 |
| 重定位类型 | PC32, 32, 64 等 | JUMP_SLOT, GLOB_DAT, RELATIVE | 同可执行文件 |
| 处理者 | 静态链接器 | 动态链接器 | 动态链接器 |

**分析共享库的重定位开销**：

```bash
# 编译 PIC 共享库
g++ -fPIC -shared lib.cpp -o libfoo.so

# 统计各类重定位数量
readelf -r libfoo.so | grep -c R_X86_64_RELATIVE
readelf -r libfoo.so | grep -c R_X86_64_GLOB_DAT
readelf -r libfoo.so | grep -c R_X86_64_JUMP_SLOT

# 查看重定位总条目数
readelf -r libfoo.so | tail -1
readelf --dyn-syms libfoo.so | wc -l
```

**排查重定位问题**：

```bash
# 问题：共享库加载时重定位文本段（textrel）
# 检查
readelf -d libfoo.so | grep TEXTREL

# 如果存在 TEXTREL，说明没有使用 -fPIC
# 解决：重新编译时加 -fPIC

# 问题：大量 RELATIVE 重定位影响启动速度
readelf -r libfoo.so | grep -c R_X86_64_RELATIVE

# 解决：使用 -fPIC 减少重定位数量
```

***

### 9. 链接器处理重定位的完整流程

**静态链接器（ld）处理流程**：

```
1. 输入：所有目标文件 + 库文件

2. 段合并：
   a.o .text + b.o .text → 合并 .text
   a.o .data + b.o .data → 合并 .data

3. 空间分配：
   为每个合并后的段分配虚拟地址
   .text:  0x401000
   .data:  0x402000

4. 符号解析：
   遍历所有符号表，解析未定义符号
   未解析 → 报错 "undefined reference"

5. 重定位修正：
   遍历所有重定位条目
   计算真实地址并写入

6. 输出：可执行文件或共享库
```

**动态链接器（ld.so）处理流程**：

```
1. 加载可执行文件到内存

2. 遍历 DT_NEEDED 条目，加载所有依赖共享库

3. 符号解析：
   遍历 .rela.dyn 和 .rela.plt
   解析每个 GLOB_DAT 和 JUMP_SLOT

4. 基址重定位：
   遍历 R_X86_64_RELATIVE
   加上加载基址偏移

5. 延迟绑定：
   JUMP_SLOT 默认延迟到首次调用时解析

6. 跳转程序入口点
```

**重定位冲突解决**：

```bash
# 查看符号冲突
LD_DEBUG=symbols,bindings ./app

# 禁用延迟绑定（立即解析所有符号）
LD_BIND_NOW=1 ./app

# 查看重定位过程
LD_DEBUG=reloc ./app
```

***

### 10. 特殊重定位场景

**场景1：IFUNC（间接函数）**

IFUNC 允许在运行时根据 CPU 特性选择最优实现。

```cpp
#include <cstring>
#include <cpuid.h>

static size_t memcpy_generic(void *dst, const void *src, size_t n) {
    char *d = static_cast<char *>(dst);
    const char *s = static_cast<const char *>(src);
    for (size_t i = 0; i < n; i++) d[i] = s[i];
    return n;
}

static size_t memcpy_sse(void *dst, const void *src, size_t n) {
    char *d = static_cast<char *>(dst);
    const char *s = static_cast<const char *>(src);
    for (size_t i = 0; i + 15 < n; i += 16) {
        __builtin_ia32_movdqu(reinterpret_cast<void *>(d + i),
                              reinterpret_cast<const void *>(s + i));
    }
    for (size_t i = (n / 16) * 16; i < n; i++) d[i] = s[i];
    return n;
}

extern "C" __attribute__((ifunc("resolve_memcpy")))
void *memcpy_resolver(void *dst, const void *src, size_t n);

static void *resolve_memcpy() {
    unsigned int eax, ebx, ecx, edx;
    __get_cpuid(1, &eax, &ebx, &ecx, &edx);
    if (ecx & bit_SSE4_1) {
        return reinterpret_cast<void *>(memcpy_sse);
    }
    return reinterpret_cast<void *>(memcpy_generic);
}
```

**场景2：线程局部存储（TLS）重定位**

```cpp
// TLS 变量的重定位
thread_local int tls_var = 0;

void set_tls(int val) {
    tls_var = val;
}
```

```bash
g++ -c tls.cpp -o tls.o
readelf -r tls.o
# 包含 R_X86_64_TPOFF32 等TLS重定位类型
```

| TLS 重定位类型 | 说明 |
|---------------|------|
| R_X86_64_TPOFF32 | 线程指针偏移 |
| R_X86_64_TLSGD | General Dynamic 模型 |
| R_X86_64_TLSLD | Local Dynamic 模型 |
| R_X86_64_GOTTPOFF | Initial Exec 模型 |
| R_X86_64_TLSDESC | TLS Descriptor 模式 |

**场景3：链接时优化（LTO）中的重定位**

```bash
# LTO 会产生特殊的重定位
g++ -flto -c a.cpp -o a.o
readelf -r a.o
# 可能包含 .symtab 重定位和 LTO 特殊段
```

> ⚠️ **平台注意**：IFUNC 是 GNU 扩展，MSVC 不支持。Windows 使用 `InitializeCriticalSection` 等运行时检测方式。TLS 重定位在不同平台差异大：Linux 使用 `__thread` / `thread_local`，Windows 使用 `__declspec(thread)` / `TlsAlloc`，macOS 使用 `pthread_key_create`。

***

### 11. 实战：排查重定位相关问题

**问题1：-fPIC 缺失导致加载失败**

```bash
# 错误信息
# /usr/bin/ld: libfoo.so: relocation R_X86_64_PC32 against symbol `foo' can not be used when making a shared object; recompile with -fPIC

# 原因：编译共享库时未使用 -fPIC
# 解决：
g++ -fPIC -shared lib.cpp -o libfoo.so
```

**问题2：符号重定位溢出**

```bash
# 错误信息
# relocation truncated to fit: R_X86_64_PC32 against symbol defined in another section

# 原因：32位偏移不够大（超过 ±2GB）
# 解决方案1：使用 -mcmodel=medium 或 -mcmodel=large
g++ -mcmodel=medium large_app.cpp -o large_app

# 解决方案2：使用 -fPIC 让链接器使用 GOT
```

**问题3：运行时重定位错误**

```bash
# 错误信息
# symbol lookup error: ./app: undefined symbol: _ZN3foo3barEv

# 排查步骤：
# 1. 确认符号是否存在于共享库
nm -D libfoo.so | c++filt | grep bar

# 2. 检查库搜索路径
ldd app

# 3. 检查运行时绑定
LD_DEBUG=symbols,bindings ./app 2>&1 | grep bar

# 4. 检查符号版本
readelf -V libfoo.so
objdump -T libfoo.so | grep bar
```

**问题4：TEXTREL 警告**

```bash
# 警告信息
# warning: creating a DT_TEXTREL in a shared library

# 原因：共享库的代码段包含需要运行时修正的重定位
# 检查：
readelf -d libfoo.so | grep TEXTREL

# 解决：确保所有源文件使用 -fPIC 编译
# 查找未使用 -fPIC 的目标文件
for obj in $(ar t libfoo.a); do
    readelf -r libfoo.a 2>/dev/null | grep -v RELATIVE | grep -v JUMP_SLOT | grep -v GLOB_DAT
done
```

**问题5：性能优化——减少重定位**

```bash
# 统计重定位数量
readelf -r libfoo.so | wc -l

# 减少重定位的方法：
# 1. 使用 -fPIC（必须）
# 2. 使用 -fvisibility=hidden 减少导出符号
# 3. 使用版本脚本控制导出
# 4. 使用 -Bsymbolic 减少动态解析
# 5. 使用 -Wl,-z,now 禁用延迟绑定（启动慢但运行时稳定）

g++ -fPIC -fvisibility=hidden -Wl,-Bsymbolic -shared lib.cpp -o libfoo.so
```

***

### 12. 极简总结

| 概念 | 要点 |
|------|------|
| **重定位本质** | 修正编译时无法确定的地址引用 |
| **重定位条目** | r_offset（位置）+ r_info（符号+类型）+ r_addend（加数） |
| **R_X86_64_64** | 绝对64位地址，用于数据引用 |
| **R_X86_64_PC32** | PC相对32位偏移，用于函数调用 |
| **R_X86_64_PLT32** | PLT相对偏移，用于动态函数调用 |
| **静态链接重定位** | 链接时一次性修正所有地址 |
| **动态链接重定位** | 加载时/运行时修正，通过 PLT/GOT 实现 |
| **PLT/GOT** | 延迟绑定机制，首次调用时解析符号地址 |
| **PIC** | 位置无关代码，通过 GOT 间接寻址，减少重定位 |
| **readelf -r** | 查看重定位条目的核心工具 |
| **常见问题** | 缺 -fPIC、偏移溢出、符号未定义、TEXTREL |

**关键记忆**：
- 重定位 = 编译器留占位符 → 链接器填真实地址
- x86_64 三大重定位类型：`R_X86_64_64`（绝对）、`R_X86_64_PC32`（相对）、`R_X86_64_PLT32`（PLT）
- 共享库必须用 `-fPIC` 编译，否则产生 TEXTREL
- PLT/GOT 实现延迟绑定：首次调用解析，后续直接跳转
- PIC 通过 GOT 间接寻址，使代码段可共享，减少重定位数量

***

### 相关阅读

- [什么是PLT和GOT](./07-什么是PLT和GOT.md)
- [什么是动态链接与静态链接](./08-什么是动态链接与静态链接.md)
- [什么是ELF文件格式](./06-什么是ELF文件格式.md)