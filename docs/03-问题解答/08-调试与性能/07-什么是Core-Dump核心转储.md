# 什么是Core Dump核心转储
> 📖 相关章节：[调试技巧](../../04-工程实践/06-调试技巧.md)

> "Core Dump=程序崩溃瞬间的'黑匣子'，记录了坠毁前所有内存状态。"——有了它，你就能像事故调查员一样还原崩溃现场。

***

### 1. 通俗理解

- **Core Dump** = 程序崩溃时，操作系统把程序的内存映像写入磁盘文件
- 就像飞机的黑匣子——记录了坠毁前的所有状态，供事后分析
- 有了core文件，就能用GDB回到崩溃现场，查看变量值、调用栈、寄存器状态

| 概念 | 类比 | 说明 |
|------|------|------|
| Core Dump | 飞机黑匣子 | 记录崩溃瞬间的完整状态 |
| core文件 | 黑匣子数据 | 磁盘上的内存映像文件 |
| GDB分析 | 事故调查 | 用工具还原崩溃现场 |
| ulimit -c | 黑匣子开关 | 控制是否生成core文件 |

***

### 2. 技术说明

#### 1. 如何启用Core Dump

**1. 检查当前限制**：

```bash
ulimit -c
# 0 = 不生成core文件
# unlimited = 不限制core文件大小
```

**2. 启用Core Dump**：

```bash
ulimit -c unlimited
```

**3. 持久化设置**（写入shell配置文件）：

```bash
echo "ulimit -c unlimited" >> ~/.bashrc
```

**4. 配置core文件路径**：

```bash
cat /proc/sys/kernel/core_pattern
# 默认可能是 "core" 或 "|/usr/share/apport/apport"
```

**自定义core文件路径**：

```bash
sudo sysctl -w kernel.core_pattern=/tmp/core.%e.%p.%t
```

| 占位符 | 含义 |
|--------|------|
| `%e` | 程序名 |
| `%p` | 进程PID |
| `%t` | 崩溃时间戳 |
| `%s` | 导致崩溃的信号 |
| `%h` | 主机名 |

**永久生效**：

```bash
echo "kernel.core_pattern=/tmp/core.%e.%p.%t" | sudo tee -a /etc/sysctl.conf
sudo sysctl -p
```

#### 2. 用GDB分析Core Dump

**基本流程**：

```bash
gdb ./program core
```

| GDB命令 | 作用 | 说明 |
|---------|------|------|
| `bt` | 查看调用栈 | 崩溃时的函数调用链 |
| `bt full` | 查看完整调用栈 | 包含每层的局部变量 |
| `frame N` | 切换到第N层栈帧 | 查看该层的变量 |
| `print var` | 打印变量值 | 查看崩溃时的变量状态 |
| `info registers` | 查看寄存器 | 查看CPU寄存器值 |
| `info locals` | 查看局部变量 | 当前栈帧的所有局部变量 |
| `list` | 查看源码 | 崩溃位置附近的代码 |
| `disassemble` | 反汇编 | 查看崩溃位置的汇编代码 |

#### 3. 常见崩溃场景分析

| 崩溃类型 | 信号 | 典型原因 | GDB线索 |
|---------|------|---------|---------|
| 段错误 | SIGSEGV | 空指针、野指针、越界 | `bt`显示崩溃位置，`print`查看指针值 |
| 空指针 | SIGSEGV | 解引用NULL | `print ptr`显示0x0 |
| 栈溢出 | SIGSEGV | 无限递归 | `bt`显示极深的递归调用 |
| 除零 | SIGFPE | 整数除以0 | 崩溃在除法指令 |
| 非法指令 | SIGILL | 函数指针损坏 | `disassemble`查看非法指令 |
| 中止 | SIGABRT | assert失败、double free | `bt`显示abort调用 |

#### 4. Windows下的等价物

| Linux | Windows | 说明 |
|-------|---------|------|
| Core Dump | MiniDump | 程序崩溃时的内存快照 |
| GDB | WinDbg/CDB | 调试器 |
| ulimit -c | 注册表/任务管理器 | 启用dump生成 |
| /proc/sys/kernel/core_pattern | 注册表DumpFolder | dump文件路径 |

**Windows启用MiniDump**：

```c
#include <windows.h>
#include <dbghelp.h>

void enable_minidump(void) {
    MINIDUMP_EXCEPTION_INFORMATION mei;
    /* 通常在SetUnhandledExceptionFilter中处理 */
}
```

**WinDbg分析MiniDump**：

```
windbg -z "C:\path\to\crash.dmp"
!analyze -v
k
```

#### 5. Core Dump的大小控制与自动清理

**大小控制**：

```bash
ulimit -c 10485760
```

| 设置 | 含义 |
|------|------|
| `ulimit -c 0` | 不生成core文件 |
| `ulimit -c unlimited` | 不限制大小 |
| `ulimit -c 10485760` | 限制10MB |

**自动清理**：

```bash
find /tmp -name "core.*" -mtime +7 -delete
```

**压缩core文件**：

```bash
kernel.core_pattern=/tmp/core.%e.%p.%t.gz
```

**注意**：大型程序的core文件可能数GB，注意磁盘空间。

***

### 3. 代码示例：故意触发段错误并用GDB分析

**源码 crash_demo.c**：

```c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void cause_null_pointer(void) {
    int* ptr = NULL;
    printf("即将解引用空指针...\n");
    *ptr = 42;
}

void cause_buffer_overflow(void) {
    char buf[8];
    const char* long_str = "这是一段超长字符串，会溢出缓冲区";
    strcpy(buf, long_str);
}

void cause_use_after_free(void) {
    int* ptr = (int*)malloc(sizeof(int));
    *ptr = 100;
    free(ptr);
    printf("已释放的内存值: %d\n", *ptr);
}

void recursive_crash(int depth) {
    char big_array[1024 * 64];
    big_array[0] = 'x';
    printf("递归深度: %d\n", depth);
    recursive_crash(depth + 1);
}

int main(void) {
    printf("崩溃演示程序\n");
    printf("1=空指针 2=缓冲区溢出 3=释放后使用 4=栈溢出\n");

    int choice = 0;
    scanf("%d", &choice);

    switch (choice) {
        case 1: cause_null_pointer(); break;
        case 2: cause_buffer_overflow(); break;
        case 3: cause_use_after_free(); break;
        case 4: recursive_crash(1); break;
        default: printf("无效选择\n"); break;
    }

    return 0;
}
```

**编译（带调试信息）**：

```bash
gcc -g -O0 crash_demo.c -o crash_demo
```

**运行并崩溃**：

```bash
ulimit -c unlimited
./crash_demo
1
即将解引用空指针...
Segmentation fault (core dumped)
```

**用GDB分析**：

```bash
gdb ./crash_demo core
```

```
(gdb) bt
#0  0x0000000000401196 in cause_null_pointer () at crash_demo.c:7
#1  0x0000000000401211 in main () at crash_demo.c:35
#2  0x00007f1234567b97 in __libc_start_main () from /lib/x86_64-linux-gnu/libc.so.6
#3  0x000000000040105a in _start ()

(gdb) frame 0
#0  0x0000000000401196 in cause_null_pointer () at crash_demo.c:7
7           *ptr = 42;

(gdb) print ptr
$1 = (int *) 0x0

(gdb) list
2       #include <stdlib.h>
3       #include <string.h>
4
5       void cause_null_pointer(void) {
6           int* ptr = NULL;
7           *ptr = 42;
8       }

(gdb) info registers
rax            0x0      0
rbx            0x0      0
...
rip            0x401196 0x401196 <cause_null_pointer+23>
```

**分析结论**：
- `bt`显示崩溃在`cause_null_pointer`函数第7行
- `print ptr`显示指针值为0x0（空指针）
- `list`显示源码确认是`*ptr = 42`解引用了空指针

**分析栈溢出的core**：

```
(gdb) bt
#0  0x00007f1234567b97 in recursive_crash (depth=12984) at crash_demo.c:21
#1  0x00007f1234567b97 in recursive_crash (depth=12983) at crash_demo.c:21
#2  0x00007f1234567b97 in recursive_crash (depth=12982) at crash_demo.c:21
...（重复上万行）
```

栈溢出的特征：`bt`显示极深的递归调用链。

***

### 4. 常见问题

#### 1. 问题1：core文件没有生成怎么办

**排查步骤**：

```bash
ulimit -c
# 如果是0，执行 ulimit -c unlimited

cat /proc/sys/kernel/core_pattern
# 如果是管道（以|开头），core被其他程序截获了

ls -la /tmp/core*
# 检查core文件是否在其他位置

sudo sysctl kernel.core_pattern=core
# 临时改为当前目录生成
```

#### 2. 问题2：core文件太大怎么办

```bash
ulimit -c 10485760
```

或者只生成最小信息：

```bash
kernel.core_pattern=|/usr/lib/systemd/systemd-coredump %P %u %g %s %t %c %h %e
```

systemd-coredump会压缩存储core文件。

#### 3. 问题3：Docker容器中如何生成core文件

```bash
docker run --ulimit core=-1 --sysctl kernel.core_pattern=core ...
```

容器默认禁用core dump，需要显式启用。

***

### 5. 极简总结

**Core Dump是程序崩溃时的内存快照，用GDB加载后可以查看调用栈、变量值、寄存器等崩溃现场信息。启用方式：`ulimit -c unlimited`。常见崩溃类型：段错误（空指针/野指针）、栈溢出（无限递归）、释放后使用。Windows下等价物是MiniDump+WinDbg。**

| 要点 | 一句话 |
|------|--------|
| Core Dump | 程序崩溃时的内存快照——事后分析的"黑匣子" |
| 启用 | `ulimit -c unlimited`——默认是0（不生成） |
| GDB分析 | `gdb ./program core` → `bt` → `frame` → `print` |
| 段错误 | SIGSEGV——空指针、野指针、越界访问 |
| 栈溢出 | 无限递归——`bt`显示极深调用链 |
| core_pattern | `/proc/sys/kernel/core_pattern`——控制core文件路径和格式 |
| Windows | MiniDump + WinDbg——Windows下的等价方案 |

***

### 相关阅读

- [段错误排查](01-段错误排查.md)
- [程序崩溃常见原因](02-程序崩溃常见原因.md)
- [什么是栈展开Stack-Unwinding](08-什么是栈展开Stack-Unwinding.md)

***