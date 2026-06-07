# 什么是PLT和GOT
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)、[动态库与共享库](../../01-C语言/19-动态库与共享库.md)

> "PLT=快递代收点，GOT=你的收货地址簿。第一次取件要查地址簿，之后直接去——这就是延迟绑定。"——理解PLT/GOT，就理解了动态链接的精髓。

***

### 1. 通俗理解

- **PLT（Procedure Linkage Table）** = 过程链接表，一个跳板，帮你跳到动态库函数的真实地址
- **GOT（Global Offset Table）** = 全局偏移表，一个地址簿，记录动态库函数的真实地址
- **延迟绑定（Lazy Binding）** = 第一次调用时才查找真实地址，之后直接跳转

| 概念 | 类比 | 说明 |
|------|------|------|
| PLT | 快递代收点 | 你去代收点取快递，代收点帮你找到快递 |
| GOT | 收货地址簿 | 代收点查地址簿找到快递存放位置 |
| 延迟绑定 | 第一次取件才登记地址 | 第一次调用才解析地址，后续直接跳转 |
| 动态链接器 | 快递公司客服 | 负责查找函数的真实地址 |

***

### 2. 技术说明

#### 1. 为什么需要PLT/GOT

**问题**：程序调用`printf`时，`printf`在`libc.so`中，地址在编译时无法确定（因为ASLR每次加载地址不同）。

**解决**：
1. 编译时：把`printf`的调用改为跳到PLT中的`printf@plt`
2. 第一次调用：PLT跳到GOT，GOT里存的是"回去解析"的地址，于是触发动态链接器查找`printf`真实地址
3. 后续调用：GOT已被更新为`printf`真实地址，直接跳转

#### 2. PLT/GOT的工作流程

**第一次调用printf**：

```
程序代码                PLT                    GOT                   动态链接器
  │                     │                      │                       │
  │  call printf ──────→│                      │                       │
  │                     │  printf@plt:         │                       │
  │                     │  jmp *GOT[printf] ──→│  GOT[printf] =        │
  │                     │                      │  下一条指令地址         │
  │                     │  ←───────────────────│  (还没解析，跳回来)    │
  │                     │  push printf序号      │                       │
  │                     │  jmp 解析桩 ──────────────────────────────→  │
  │                     │                      │                  查找printf真实地址
  │                     │                      │  GOT[printf] =   │
  │                     │                      │  printf真实地址 ←─│
  │                     │  jmp *GOT[printf] ──→│  真实地址           │
  │                     │ ──────────────────────────────────────────→  printf()
```

**后续调用printf**：

```
程序代码                PLT                    GOT
  │                     │                      │
  │  call printf ──────→│                      │
  │                     │  printf@plt:         │
  │                     │  jmp *GOT[printf] ──→│  GOT[printf] =       │
  │                     │                      │  printf真实地址       │
  │                     │ ────────────────────→│  直接跳转到printf()   │
```

#### 3. 延迟绑定的好处

| 维度 | 无延迟绑定 | 有延迟绑定 |
|------|-----------|-----------|
| 启动时间 | 启动时解析所有动态符号，慢 | 只在首次调用时解析，快 |
| 内存使用 | 所有符号都要解析 | 只解析实际用到的符号 |
| 典型场景 | — | 大部分函数只在运行中用到一小部分 |

#### 4. GOT的结构

```
GOT[0]  = .dynamic段的地址
GOT[1]  = link_map结构地址（动态链接器用）
GOT[2]  = _dl_runtime_resolve地址（动态链接器解析函数）
GOT[3]  = printf的真实地址（第一次调用后填入）
GOT[4]  = malloc的真实地址
GOT[5]  = strlen的真实地址
...
```

***

### 3. 安全问题

#### 1. GOT覆写攻击

**原理**：GOT是可写的，攻击者通过缓冲区溢出覆盖GOT中的函数地址，把`printf@GOT`改为恶意代码地址。

```c
#include <stdio.h>
#include <string.h>

void vulnerable(void) {
    char buf[16];
    gets(buf);
}

void safe_function(void) {
    printf("安全函数\n");
}

int main(void) {
    vulnerable();
    return 0;
}
```

攻击者输入超长字符串覆盖GOT中`printf`的地址为恶意代码地址，下次调用`printf`时跳转到恶意代码。

#### 2. RELRO防护

| 级别 | 全称 | 行为 | 安全性 |
|------|------|------|--------|
| 无RELRO | No RELRO | GOT完全可写 | 不安全 |
| Partial RELRO | 部分RELRO | GOT[0-2]只读，其余可写 | 部分安全 |
| Full RELRO | 完全RELRO | 启动时解析所有符号，整个GOT只读 | 安全（但启动慢） |

**编译选项**：

```bash
gcc -z norelro main.c      # 无RELRO
gcc -z lazy main.c          # 部分RELRO（默认）
gcc -z now main.c           # 完全RELRO
```

**检查RELRO状态**：

```bash
readelf -d main | grep RELRO
readelf -l main | grep GNU_RELRO
```

***

### 4. 实战分析

#### 1. 用readelf查看重定位条目

```bash
gcc -c main.c -o main.o
readelf -r main.o
```

```
Relocation section '.rela.plt' at offset 0xxxx contains 3 entries:
  Offset          Info           Type           Symbol's Value  Symbol's Name
  000000401000  000300000007 R_X86_64_JUMP_SLOT  0000000000000000 printf
  000000401008  000400000007 R_X86_64_JUMP_SLOT  0000000000000000 malloc
  000000401010  000500000007 R_X86_64_JUMP_SLOT  0000000000000000 strlen
```

`R_X86_64_JUMP_SLOT`类型表示这是PLT/GOT重定位条目。

#### 2. 用objdump查看PLT条目

```bash
gcc main.c -o main
objdump -d main | grep -A3 "plt>"
```

```
0000000000401020 <printf@plt>:
  401020:   ff 25 02 2f 00 00    jmpq   *0x2f02(%rip)   # 403f28 <printf@GLIBC_2.2.5>
  401026:   68 00 00 00 00       pushq  $0x0
  40102b:   e9 e0 ff ff ff       jmpq   401010 <_init+0x18>

0000000000401030 <malloc@plt>:
  401030:   ff 25 fa 2e 00 00    jmpq   *0x2efa(%rip)   # 403f30 <malloc@GLIBC_2.2.5>
  401036:   68 01 00 00 00       pushq  $0x1
  40103b:   e9 d0 ff ff ff       jmpq   401010 <_init+0x18>
```

**解读**：
- `jmpq *0x2f02(%rip)` = 跳到GOT中对应条目指向的地址
- 第一次调用时，GOT中存的是下一条`pushq`指令的地址，所以跳回来
- `pushq $0x0` = 压入符号索引0（printf）
- `jmpq 401010` = 跳到PLT第0项（解析桩），触发动态链接器

#### 3. 代码示例：观察延迟绑定

```c
#include <stdio.h>
#include <string.h>

int main(void) {
    printf("第一次调用printf之前\n");

    printf("第一次调用printf\n");

    printf("第二次调用printf\n");

    strlen("hello");

    return 0;
}
```

**用GDB观察GOT变化**：

```bash
gcc -g -no-pie main.c -o main
gdb ./main
```

```
(gdb) break main
(gdb) run
(gdb) print/x *(void**)0x403f28
$1 = 0x401026

(gdb) next
(gdb) print/x *(void**)0x403f28
$2 = 0x7fxxxxxx

(gdb) next
(gdb) print/x *(void**)0x403f28
$3 = 0x7fxxxxxx
```

第一次调用前，GOT中存的是PLT中`pushq`指令的地址（0x401026）。第一次调用后，GOT被更新为`printf`的真实地址（0x7fxxxxxx）。后续调用不再变化。

***

### 5. 常见问题

#### 1. 问题1：PLT和GOT分别在哪个段

| 表 | 所在段 | 属性 |
|----|--------|------|
| PLT | `.plt` | 可读可执行（代码） |
| GOT | `.got.plt` | 可读可写（数据，Full RELRO下只读） |

#### 2. 问题2：静态链接的程序有PLT/GOT吗

没有。静态链接时所有函数地址在编译期确定，不需要延迟绑定。

#### 3. 问题3：-fno-plt选项是什么

告诉编译器不通过PLT跳转，直接通过GOT调用。省掉一次间接跳转，但失去了延迟绑定。

```bash
gcc -fno-plt main.c -o main
```

***

### 6. 极简总结

**PLT是跳板代码，GOT是地址簿。第一次调用动态库函数时，PLT跳到GOT发现地址未解析，触发动态链接器查找真实地址并写入GOT。后续调用直接通过GOT跳转，无需再解析。延迟绑定加快启动速度，但GOT可写带来安全风险，Full RELRO通过启动时解析+只读GOT来防护。**

| 要点 | 一句话 |
|------|--------|
| PLT | 过程链接表——跳板代码，跳到动态库函数 |
| GOT | 全局偏移表——地址簿，存函数真实地址 |
| 延迟绑定 | 第一次调用才解析地址——加快启动速度 |
| 第一次调用 | PLT→GOT(未解析)→动态链接器→填入GOT→跳转 |
| 后续调用 | PLT→GOT(已解析)→直接跳转 |
| GOT覆写攻击 | 覆盖GOT中的地址——Full RELRO可防护 |
| RELRO | No/Partial/Full三级——Full最安全但启动慢 |

***

### 相关阅读

- [什么是动态链接与静态链接](./08-什么是动态链接与静态链接.md)
- [什么是重定位Relocation](./12-什么是重定位Relocation.md)