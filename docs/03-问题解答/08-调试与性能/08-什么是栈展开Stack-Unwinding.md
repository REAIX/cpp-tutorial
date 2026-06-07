# 什么是栈展开Stack Unwinding
> 📖 相关章节：[异常处理](../../02-CPP/07-异常处理.md)、[调试技巧](../../04-工程实践/06-调试技巧.md)

> "栈展开是 C++ 异常处理和调试回溯的底层引擎——它让程序在灾难中依然有序撤退。" —— Exceptional C++

***

### 1. 本质洞察

栈展开（Stack Unwinding）是从当前执行点逐帧回溯调用栈、销毁局部对象并恢复调用者上下文的过程，是 C++ 异常处理、调试器回溯和性能分析的核心基础设施。

***

### 2. 栈展开的基本原理

程序运行时，每个函数调用在栈上创建一个栈帧（Stack Frame），包含返回地址、局部变量和上一帧指针。栈展开就是沿着这条链逐帧回溯。

```
高地址
┌────────────────────┐
│   main() 栈帧       │  ← RBP=0x7fff0010
│   返回地址: _start   │
│   局部变量: argc     │
├────────────────────┤
│   foo() 栈帧        │  ← RBP=0x7fff0000
│   返回地址: main+0x20│
│   局部变量: x       │
├────────────────────┤
│   bar() 栈帧        │  ← RBP=0x7fff0008 (当前)
│   返回地址: foo+0x15 │
│   局部变量: y       │
└────────────────────┘  ← RSP
低地址
```

**栈帧链（Frame Chain）**：

```c
struct StackFrame {
    StackFrame *prev_rbp;    // 上一帧的 RBP
    void *return_address;    // 返回地址
};
```

展开过程：

```
1. 从当前 RBP 开始
2. 读取 prev_rbp → 跳到上一帧
3. 读取 return_address → 获取调用位置
4. 重复直到栈底
```

**两种展开方式对比**：

| 方式 | 原理 | 依赖 | 精度 |
|------|------|------|------|
| 帧指针链（FP Chain） | 沿 RBP 链回溯 | -fno-omit-frame-pointer | 低（优化代码可能缺帧） |
| DWARF 展开（.eh_frame） | 查表计算每帧信息 | .eh_frame 段 | 高（精确到每条指令） |

```bash
# 查看帧指针是否被省略
g++ -S -o - main.cpp | grep -c "push.*%rbp"

# -O0 通常保留帧指针
# -O2 通常省略帧指针（-fomit-frame-pointer）
# 强制保留帧指针
g++ -fno-omit-frame-pointer -O2 main.cpp -o main
```

***

### 3. .eh_frame 与 DWARF 展开

`.eh_frame` 是 ELF 文件中存储栈展开信息的段，基于 DWARF 标准的 CFI（Call Frame Information）格式。

**.eh_frame 结构**：

```
.eh_frame
├── CIE (Common Information Entry) - 公共信息
│   ├── 版本号
│   ├── 编码方式
│   ├── 返回地址寄存器
│   └── 初始 CFI 指令
│
└── FDE (Frame Description Entry) - 每个函数的展开信息
    ├── CIE 指针
    ├── 函数起始地址和长度
    └── CFI 指令序列
```

**CFI 指令示例**：

```asm
foo:
    .cfi_startproc
    pushq   %rbp
    .cfi_def_cfa_offset 16       # CFA = RSP + 16
    .cfi_offset %rbp, -16        # RBP 保存在 [CFA-16]
    movq    %rsp, %rbp
    .cfi_def_cfa_register %rbp   # CFA = RBP
    subq    $32, %rsp
    movl    $42, -4(%rbp)
    movq    %rbp, %rsp
    popq    %rbp
    .cfi_def_cfa %rsp, 8
    .cfi_restore %rbp
    ret
    .cfi_endproc
```

**CFA（Canonical Frame Address）**：每条指令处定义的"规范帧地址"，通常是上一帧的栈指针值。

| CFI 指令 | 含义 |
|----------|------|
| `.cfi_def_cfa register, offset` | CFA = register + offset |
| `.cfi_def_cfa_offset offset` | CFA = 当前寄存器 + offset |
| `.cfi_def_cfa_register register` | CFA = register + 旧 offset |
| `.cfi_offset register, offset` | register 保存在 [CFA + offset] |
| `.cfi_restore register` | register 恢复为调用者值 |
| `.cfi_startproc` / `.cfi_endproc` | FDE 边界 |

```bash
# 查看 .eh_frame 内容
readelf -wf a.out

# 查看 .eh_frame 段信息
readelf -S a.out | grep eh_frame

# 查看 .eh_frame_hdr（加速查找的索引）
readelf -S a.out | grep eh_frame_hdr
```

**.eh_frame_hdr 的作用**：

```
.eh_frame_hdr = 二分查找索引表

条目格式：
  [函数起始地址, FDE偏移]

查找流程：
1. 在 .eh_frame_hdr 中二分查找当前 PC
2. 找到对应的 FDE
3. 从 FDE 读取 CFI 指令
4. 计算展开信息
```

> ⚠️ **平台注意**：`.eh_frame` 是 ELF/Linux 特有的段。macOS 使用 `__unwind_info` 段（紧凑展开格式）。Windows x64 使用 `.pdata` / `.xdata` 段。ARM64 也使用紧凑展开格式。

***

### 4. _Unwind_Backtrace 与 backtrace()

C/C++ 提供了两种获取调用栈的 API：POSIX 的 `backtrace()` 和 Itanium C++ ABI 的 `_Unwind_Backtrace`。

**backtrace() 用法**：

```cpp
#include <execinfo.h>
#include <cstdio>
#include <cstdlib>

void print_backtrace() {
    void *buffer[128];
    int n = backtrace(buffer, 128);
    char **symbols = backtrace_symbols(buffer, n);

    printf("Backtrace (%d frames):\n", n);
    for (int i = 0; i < n; i++) {
        printf("  [%d] %s\n", i, symbols[i]);
    }

    free(symbols);
}

void func_c() { print_backtrace(); }
void func_b() { func_c(); }
void func_a() { func_b(); }

int main() {
    func_a();
    return 0;
}
```

```bash
# 编译（需要 -rdynamic 导出符号，-g 生成调试信息）
g++ -rdynamic -g backtrace_demo.cpp -o backtrace_demo
./backtrace_demo

# 输出示例：
# Backtrace (4 frames):
#   [0] ./backtrace_demo() [0x4011a0]
#   [1] ./backtrace_demo() [0x4011c0]
#   [2] ./backtrace_demo() [0x4011d0]
#   [3] ./backtrace_demo() [0x4011e0]
```

**_Unwind_Backtrace 用法**：

```cpp
#include <unwind.h>
#include <cstdio>

static _Unwind_Reason_Code unwind_callback(struct _Unwind_Context *ctx, void *arg) {
    int *depth = static_cast<int *>(arg);
    uintptr_t pc = _Unwind_GetIP(ctx);
    printf("  [%d] pc=0x%lx\n", *depth, static_cast<unsigned long>(pc));
    (*depth)++;
    return _URC_NO_REASON;
}

void print_unwind_backtrace() {
    int depth = 0;
    printf("Unwind backtrace:\n");
    _Unwind_Backtrace(unwind_callback, &depth);
}

void func_c2() { print_unwind_backtrace(); }
void func_b2() { func_c2(); }
void func_a2() { func_b2(); }
```

**两者对比**：

| 维度 | backtrace() | _Unwind_Backtrace |
|------|-------------|-------------------|
| **标准** | POSIX（GNU 扩展） | Itanium C++ ABI |
| **实现** | 基于 .eh_frame 或帧指针 | 严格基于 .eh_frame |
| **可移植性** | Linux/macOS | GCC/Clang 支持的平台 |
| **信号安全** | 否 | 否 |
| **精度** | 中等 | 高 |
| **C++ 异常感知** | 否 | 是 |
| **性能** | 较快 | 较慢（逐帧查表） |

**C++17 std::stacktrace**：

```cpp
#include <stacktrace>
#include <iostream>

void print_stacktrace() {
    auto trace = std::stacktrace::current();
    std::cout << trace << std::endl;
}
```

> ⚠️ **平台注意**：`backtrace()` 在 Windows 上不可用，可用 `CaptureStackBackTrace()` 替代。`_Unwind_Backtrace` 在 MSVC 上使用 `RtlWalkFrameChain`。`std::stacktrace` 需要 GCC 12+ 或 MSVC 19.34+，且需链接 `-lstdc++_exp`。

***

### 5. 异常处理中的栈展开

C++ 异常处理是最典型的栈展开场景。当 `throw` 执行时，运行时沿调用栈搜索匹配的 `catch`，并在回退过程中销毁局部对象。

**异常处理流程**：

```
1. throw 表达式创建异常对象
2. 运行时开始栈展开
3. 逐帧检查是否有匹配的 catch
4. 对每个退出的栈帧，调用局部对象的析构函数
5. 找到匹配的 catch 后，跳转到 catch 块
6. 如果没有匹配的 catch，调用 std::terminate()
```

```cpp
#include <iostream>
#include <stdexcept>

struct Resource {
    Resource(const char *n) : name_(n) {
        std::cout << "Resource(" << name_ << ") 构造\n";
    }
    ~Resource() {
        std::cout << "Resource(" << name_ << ") 析构\n";
    }
    const char *name_;
};

void func_c() {
    Resource r3("C");
    throw std::runtime_error("error in C");
}

void func_b() {
    Resource r2("B");
    func_c();
}

void func_a() {
    Resource r1("A");
    try {
        func_b();
    } catch (const std::runtime_error &e) {
        std::cout << "捕获异常: " << e.what() << "\n";
    }
}

int main() {
    func_a();
    return 0;
}
```

输出：

```
Resource(A) 构造
Resource(B) 构造
Resource(C) 构造
Resource(C) 析构    ← 栈展开时析构 C 的局部对象
Resource(B) 析构    ← 栈展开时析构 B 的局部对象
捕获异常: error in C
Resource(A) 析构    ← A 的局部对象正常析构
```

**异常展开的内部机制**：

```
.eh_frame 中的 LSDA (Language-Specific Data Area):

func_b 的 LSDA:
  call_site_table:
    [0x0, 0x20, 0, landing_pad=0x15]   # 调用区域 → 清理代码
  action_table: []

func_a 的 LSDA:
  call_site_table:
    [0x0, 0x30, 0, landing_pad=0x20]   # 调用区域 → catch 代码
  action_table:
    [type_index=1, next=0]              # std::runtime_error
```

**两阶段展开（Two-Phase Unwinding）**：

| 阶段 | 名称 | 目的 | 行为 |
|------|------|------|------|
| 阶段1 | 搜索阶段 | 查找匹配的 catch | 只读遍历，不调用析构函数 |
| 阶段2 | 清理阶段 | 执行展开 | 调用析构函数，跳转到 catch |

```bash
# 查看异常处理相关段
readelf -S a.out | grep -E "eh_frame|gcc_except"

# 查看 LSDA 信息
readelf -wf a.out | grep -A5 "LSDA"
```

***

### 6. RAII 与栈展开的协作

RAII（Resource Acquisition Is Initialization）是 C++ 资源管理的核心范式，其正确性依赖于栈展开时析构函数的可靠调用。

**RAII 包装器**：

```cpp
#include <cstdio>
#include <stdexcept>

class FileHandle {
    FILE *fp_;

public:
    explicit FileHandle(const char *path, const char *mode)
        : fp_(std::fopen(path, mode)) {
        if (!fp_) throw std::runtime_error("无法打开文件");
    }

    ~FileHandle() {
        if (fp_) {
            std::fclose(fp_);
            fp_ = nullptr;
        }
    }

    FileHandle(const FileHandle &) = delete;
    FileHandle &operator=(const FileHandle &) = delete;

    FILE *get() const { return fp_; }
};

class LockGuard {
    pthread_mutex_t &mtx_;

public:
    explicit LockGuard(pthread_mutex_t &m) : mtx_(m) {
        pthread_mutex_lock(&mtx_);
    }

    ~LockGuard() {
        pthread_mutex_unlock(&mtx_);
    }

    LockGuard(const LockGuard &) = delete;
    LockGuard &operator=(const LockGuard &) = delete;
};

void process_file(const char *path) {
    FileHandle fh(path, "r");
    LockGuard lock(global_mutex);

    char buf[256];
    if (!std::fgets(buf, sizeof(buf), fh.get())) {
        throw std::runtime_error("读取失败");
    }
}
```

**析构函数中的异常陷阱**：

```cpp
struct Bad {
    ~Bad() {
        throw std::runtime_error("析构函数中抛异常");
    }
};

void dangerous() {
    Bad b;
    throw std::logic_error("原始异常");
}
```

当 `dangerous()` 执行时：
1. `throw logic_error` 触发栈展开
2. 展开到 `Bad` 的析构函数
3. 析构函数中再次 `throw`
4. **两个异常同时活跃 → `std::terminate()` 被调用**

```cpp
#include <iostream>
#include <exception>

struct Safe {
    ~Safe() noexcept {
        try {
            cleanup();
        } catch (...) {
            std::cerr << "析构函数中捕获异常，已忽略\n";
        }
    }

    void cleanup() {
        throw std::runtime_error("清理失败");
    }
};
```

| 规则 | 说明 |
|------|------|
| 析构函数默认 noexcept(C++11) | 析构函数中抛异常会调用 terminate |
| 析构函数应吞掉异常 | 在析构函数内 try-catch 所有异常 |
| 使用 noexcept(false) 谨慎 | 仅在明确需要时标记析构函数可抛异常 |
| swap 不应抛异常 | RAII 类的 swap 必须是 noexcept |

***

### 7. 栈展开的性能影响

异常处理和栈展开信息对程序性能有多方面影响。

**代码体积影响**：

```bash
# 不使用异常
g++ -fno-exceptions -O2 app.cpp -o app_noexc
ls -l app_noexc

# 使用异常
g++ -O2 app.cpp -o app_exc
ls -l app_exc

# 对比 .eh_frame 大小
size app_noexc app_exc
```

| 配置 | 代码段 | .eh_frame | 总体积 |
|------|--------|-----------|--------|
| -fno-exceptions | 较小 | 无 | 小 |
| 默认（异常启用） | 较大 | 有 | 中 |
| -fno-rtti + 异常 | 中等 | 有 | 中 |

**运行时开销**：

| 场景 | 开销 | 原因 |
|------|------|------|
| 正常路径（无异常） | 几乎为零 | 仅增加 .eh_frame 段，不影响执行 |
| throw 路径 | 高 | 需要搜索 LSDA、展开栈帧 |
| backtrace() | 中等 | 遍历 .eh_frame |
| -fno-omit-frame-pointer | 微小 | 多一条 push/mov 指令 |

**异常 vs 错误码性能对比**：

```cpp
#include <stdexcept>
#include <chrono>
#include <iostream>

int error_code_path(int val) {
    if (val < 0) return -1;
    if (val > 100) return -2;
    return val * 2;
}

int exception_path(int val) {
    if (val < 0) throw std::invalid_argument("negative");
    if (val > 100) throw std::out_of_range("too large");
    return val * 2;
}

void bench() {
    const int N = 10000000;

    auto start = std::chrono::steady_clock::now();
    for (int i = 0; i < N; i++) {
        error_code_path(i % 100);
    }
    auto end = std::chrono::steady_clock::now();
    std::cout << "error_code: "
              << std::chrono::duration<double, std::milli>(end - start).count()
              << " ms\n";

    start = std::chrono::steady_clock::now();
    for (int i = 0; i < N; i++) {
        try {
            exception_path(i % 100);
        } catch (...) {}
    }
    end = std::chrono::steady_clock::now();
    std::cout << "exception (no throw): "
              << std::chrono::duration<double, std::milli>(end - start).count()
              << " ms\n";

    int error_count = 0;
    start = std::chrono::steady_clock::now();
    for (int i = 0; i < N; i++) {
        int r = error_code_path(-1);
        if (r < 0) error_count++;
    }
    end = std::chrono::steady_clock::now();
    std::cout << "error_code (error path): "
              << std::chrono::duration<double, std::milli>(end - start).count()
              << " ms\n";

    int catch_count = 0;
    start = std::chrono::steady_clock::now();
    for (int i = 0; i < N; i++) {
        try {
            exception_path(-1);
        } catch (...) { catch_count++; }
    }
    end = std::chrono::steady_clock::now();
    std::cout << "exception (throw path): "
              << std::chrono::duration<double, std::milli>(end - start).count()
              << " ms\n";
}
```

**典型结果**：

| 路径 | 相对耗时 |
|------|---------|
| 错误码（正常路径） | 1x（基准） |
| 异常（正常路径，无 throw） | ~1x（几乎无开销） |
| 错误码（错误路径） | ~1x |
| 异常（throw 路径） | 10-100x |

**结论**：异常在正常路径几乎零开销，但 throw 路径开销巨大。异常适用于"异常"情况，不应用于正常控制流。

***

### 8. -fno-exceptions 的影响

禁用异常会彻底改变栈展开行为，影响程序的资源安全性。

```bash
# 禁用异常
g++ -fno-exceptions -O2 app.cpp -o app_noexc

# 同时禁用 RTTI
g++ -fno-exceptions -fno-rtti -O2 app.cpp -o app_minimal
```

**-fno-exceptions 的效果**：

| 方面 | 影响 |
|------|------|
| throw/catch | 编译错误 |
| 析构函数调用 | 栈展开仍可能发生（longjmp 等），但无异常触发的展开 |
| .eh_frame | 可能仍生成（用于 backtrace），取决于编译器 |
| 代码体积 | 减小 |
| RAII 安全性 | 降低（无异常触发的自动析构） |
| 标准库 | 部分 STL 功能不可用（如 std::vector::at 会调用 terminate） |

```cpp
// -fno-exceptions 下的替代错误处理
#include <cstdlib>
#include <cstdio>

[[noreturn]] void fatal_error(const char *msg) {
    std::fprintf(stderr, "FATAL: %s\n", msg);
    std::abort();
}

int safe_divide(int a, int b) {
    if (b == 0) fatal_error("division by zero");
    return a / b;
}
```

**嵌入式/游戏开发中的常见做法**：

```cpp
#include <cassert>
#include <cstdlib>

class ErrorCode {
    int code_;
    const char *msg_;

public:
    constexpr ErrorCode(int c, const char *m) : code_(c), msg_(m) {}
    constexpr bool ok() const { return code_ == 0; }
    constexpr int code() const { return code_; }
    constexpr const char *message() const { return msg_; }
};

constexpr ErrorCode OK(0, "success");
constexpr ErrorCode ERR_INVALID_ARG(1, "invalid argument");
constexpr ErrorCode ERR_OUT_OF_RANGE(2, "out of range");

ErrorCode process(int val) {
    if (val < 0) return ERR_INVALID_ARG;
    if (val > 100) return ERR_OUT_OF_RANGE;
    return OK;
}
```

> ⚠️ **平台注意**：Windows SEH（Structured Exception Handling）与 C++ 异常是两套机制。MSVC 可以用 `/EHs` 或 `/EHa` 控制异常模型。`-fno-exceptions` 在 MSVC 上对应 `/EHs-c-`。即使在 `-fno-exceptions` 下，Windows SEH 仍可能触发栈展开。

***

### 9. 栈损坏与调试

栈损坏（Stack Corruption）会导致栈展开失败，是最难调试的问题之一。

**常见栈损坏原因**：

| 原因 | 示例 |
|------|------|
| 缓冲区溢出 | `char buf[8]; strcpy(buf, "too long string");` |
| 数组越界 | `int arr[4]; arr[10] = 42;` |
| 野指针写入 | `int *p = nullptr; *p = 42;` |
| 栈帧覆盖 | `longjmp` 跳过析构函数 |
| 未初始化指针 | `int *p; *p = 42;` |

**检测栈损坏**：

```cpp
#include <cstdio>
#include <cstring>

void vulnerable_function() {
    char buffer[8];
    const char *input = "This string is way too long for the buffer";
    strcpy(buffer, input);
    printf("buffer: %s\n", buffer);
}
```

```bash
# 使用 AddressSanitizer 检测
g++ -fsanitize=address -g stack_overflow.cpp -o stack_overflow
./stack_overflow

# 使用 Stack Protector（栈保护）
g++ -fstack-protector-all -g stack_overflow.cpp -o stack_protected
./stack_protected
# 检测到栈损坏时输出：*** stack smashing detected ***
```

**栈保护原理**：

```
┌──────────────────┐
│   返回地址        │
├──────────────────┤
│   旧 RBP         │
├──────────────────┤
│   Canary 值      │ ← 函数入口写入随机值，函数出口检查
├──────────────────┤
│   局部变量        │
│   buffer[8]      │ ← 溢出会覆盖 Canary
├──────────────────┤
│   对齐填充        │
└──────────────────┘
```

| 编译选项 | 说明 |
|---------|------|
| `-fstack-protector` | 仅保护含 char 数组的函数 |
| `-fstack-protector-all` | 保护所有函数 |
| `-fstack-protector-strong` | 保护含数组/地址引用的函数（推荐） |
| `-fstack-protector-explicit` | 仅保护 `__attribute__((stack_protect))` 标记的函数 |

***

### 10. GDB 调试栈展开问题

GDB 依赖 .eh_frame 进行栈展开，展开失败时无法正确显示调用栈。

**常见 GDB 栈展开问题**：

```bash
# 问题1：栈展开不完整
(gdb) bt
#0  0x00007fff1234 in func_c ()
#1  0x000000000000 in ?? ()     ← 无法展开
#2  0x000000000000 in ?? ()

# 原因：帧指针被省略且 .eh_frame 不完整
# 解决：编译时加 -fno-omit-frame-pointer

# 问题2：优化导致栈信息丢失
(gdb) bt
#0  func_c () at main.cpp:10
#1  0x401050 in func_b () at main.cpp:15
#2  0x401080 in ?? ()            ← 内联函数无法显示

# 原因：函数被内联
# 解决：编译时加 -fno-inline
```

**GDB 栈展开调试命令**：

```bash
# 查看当前帧信息
(gdb) info frame

# 查看所有帧
(gdb) info stack
(gdb) backtrace
(gdb) backtrace full

# 切换帧
(gdb) frame 2
(gdb) up
(gdb) down

# 查看帧的展开信息
(gdb) info frame 0

# 强制使用帧指针链展开
(gdb) set unwindonsignal on

# 调试展开失败
(gdb) maint info dwarf unwind
```

**调试 .eh_frame 问题**：

```bash
# 查看 .eh_frame 内容
readelf -wf a.out | head -100

# 验证 .eh_frame 完整性
readelf --debug-dump=frames a.out

# 检查特定函数的展开信息
objdump --dwarf=frames a.out | grep -A20 "func_name"

# GDB 中手动展开
(gdb) x/2ag $rbp          # 查看当前帧的 prev_rbp 和 return_addr
(gdb) info symbol 0x401050 # 查看地址对应的符号
```

**Core Dump 分析**：

```bash
# 启用 core dump
ulimit -c unlimited

# 分析 core dump
gdb ./app core

# 在 GDB 中
(gdb) bt full
(gdb) info registers
(gdb) x/20ag $rsp
```

**信号处理器中安全获取调用栈**：

```cpp
#include <signal.h>
#include <execinfo.h>
#include <unistd.h>
#include <cstdio>

static void safe_print_bt() {
    void *buf[32];
    int n = backtrace(buf, 32);
    backtrace_symbols_fd(buf, n, STDERR_FILENO);
}

extern "C" void signal_handler(int sig) {
    const char *name = "UNKNOWN";
    switch (sig) {
    case SIGSEGV: name = "SIGSEGV"; break;
    case SIGABRT: name = "SIGABRT"; break;
    case SIGBUS:  name = "SIGBUS";  break;
    case SIGFPE:  name = "SIGFPE";  break;
    }

    char msg[128];
    snprintf(msg, sizeof(msg), "Signal %s received, backtrace:\n", name);
    write(STDERR_FILENO, msg, strlen(msg));

    safe_print_bt();
    _exit(128 + sig);
}

void install_signal_handlers() {
    struct sigaction sa;
    sa.sa_handler = signal_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGSEGV, &sa, nullptr);
    sigaction(SIGABRT, &sa, nullptr);
    sigaction(SIGBUS,  &sa, nullptr);
    sigaction(SIGFPE,  &sa, nullptr);
}
```

> ⚠️ **平台注意**：信号处理器中调用 `backtrace()` 技术上是未定义行为（非异步信号安全函数），但在 Linux/glibc 上通常可工作。Windows 使用 `SetUnhandledExceptionFilter` + `CaptureStackBackTrace`。

***

### 11. 自定义栈展开与性能分析

栈展开是性能分析工具（profiler）的核心能力，用于采样调用栈。

**采样式性能分析器原理**：

```
定时器中断 → 捕获当前 PC → 栈展开获取完整调用栈 → 聚合统计
```

**轻量级栈采样**：

```cpp
#include <execinfo.h>
#include <cstdint>
#include <cstring>
#include <unordered_map>
#include <vector>
#include <algorithm>

struct StackSample {
    static constexpr int MAX_DEPTH = 16;
    void *frames[MAX_DEPTH];
    int depth;
};

class StackProfiler {
    std::unordered_map<uint64_t, int> hotspots_;
    int total_samples_ = 0;

    static uint64_t hash_frames(const StackSample &s) {
        uint64_t h = 0;
        for (int i = 0; i < s.depth; i++) {
            h = h * 131 + reinterpret_cast<uintptr_t>(s.frames[i]);
        }
        return h;
    }

public:
    void sample() {
        StackSample s;
        s.depth = backtrace(s.frames, StackSample::MAX_DEPTH);
        uint64_t h = hash_frames(s);
        hotspots_[h]++;
        total_samples_++;
    }

    void report() const {
        std::vector<std::pair<uint64_t, int>> sorted(
            hotspots_.begin(), hotspots_.end());
        std::sort(sorted.begin(), sorted.end(),
                  [](auto &a, auto &b) { return a.second > b.second; });

        printf("Total samples: %d\n", total_samples_);
        printf("Top hotspots:\n");
        int shown = 0;
        for (auto &[hash, count] : sorted) {
            if (shown++ >= 10) break;
            printf("  %d samples (%.1f%%)\n",
                   count, 100.0 * count / total_samples_);
        }
    }
};
```

**perf / gperftools 集成**：

```bash
# Linux perf 采样（依赖 .eh_frame）
perf record -g ./app
perf report

# gperftools CPU profiler
g++ -lprofiler app.cpp -o app
CPUPROFILE=app.prof ./app
pprof --text ./app app.prof

# Intel VTune
amplxe-cl -collect hotspots ./app
```

**减少 .eh_frame 体积**：

```bash
# 使用 .eh_frame_hdr 加速查找（默认启用）
g++ -Wl,--eh-frame-hdr app.cpp -o app

# 使用紧凑展开信息（ARM64）
g++ -fasynchronous-unwind-tables app.cpp -o app

# 完全移除展开表（不推荐，破坏异常和调试）
g++ -fno-asynchronous-unwind-tables -fno-unwind-tables app.cpp -o app
```

| 编译选项 | .eh_frame | 异常 | backtrace | 调试 |
|---------|-----------|------|-----------|------|
| 默认 | 生成 | 可用 | 可用 | 可用 |
| `-fno-asynchronous-unwind-tables` | 不生成 | 受限 | 受限 | 受限 |
| `-fno-exceptions` | 可能仍生成 | 不可用 | 可用 | 可用 |
| `-fno-exceptions -fno-unwind-tables` | 不生成 | 不可用 | 不可用 | 受限 |

***

### 12. 极简总结

| 概念 | 要点 |
|------|------|
| **栈展开本质** | 沿调用栈逐帧回溯，销毁局部对象，恢复调用者上下文 |
| **.eh_frame** | ELF 中存储 DWARF CFI 展开信息的段 |
| **DWARF CFI** | 每条指令处定义 CFA 和寄存器恢复规则 |
| **_Unwind_Backtrace** | Itanium ABI 标准展开 API，基于 .eh_frame |
| **backtrace()** | POSIX/GNU 扩展，快速获取调用栈 |
| **异常展开** | 两阶段：搜索 catch → 清理析构，RAII 依赖此机制 |
| **RAII** | 资源管理范式，析构函数在栈展开时自动调用 |
| **性能** | 正常路径零开销，throw 路径开销大（10-100x） |
| **-fno-exceptions** | 禁用异常，减小体积，但降低 RAII 安全性 |
| **栈损坏** | 缓冲区溢出等破坏帧链，ASan/栈保护检测 |
| **GDB 调试** | 依赖 .eh_frame，优化代码可能导致展开失败 |

**关键记忆**：
- 栈展开 = 沿帧链回退 + 销毁局部对象 + 恢复上下文
- `.eh_frame` 是展开信息的权威来源，帧指针链只是降级方案
- C++ 异常处理的两阶段展开保证 RAII 析构函数被调用
- 析构函数绝不能抛异常，否则 `std::terminate()`
- 异常在正常路径几乎零开销，但 throw 路径很慢
- 编译时加 `-fno-omit-frame-pointer` 可改善调试体验
- 栈损坏是最难调试的问题之一，ASan 是最佳防御工具

***

### 相关阅读

- [段错误排查](01-段错误排查.md)
- [什么是Core-Dump核心转储](07-什么是Core-Dump核心转储.md)
- [为什么代码可以调试-调试信息深度解析](./06-为什么代码可以调试-调试信息深度解析.md)