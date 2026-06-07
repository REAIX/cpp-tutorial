# 什么是setjmp与longjmp
> 📖 相关章节：[错误处理与信号](../../01-C语言/13-错误处理与信号.md)

> "setjmp/longjmp=编程世界的'存档/读档'——setjmp是存档点，longjmp是读档回到存档点。"——C语言的非局部跳转机制，让你跳过正常调用流程直接回到之前的位置。

***

### 1. 通俗理解

- **setjmp** = 在程序中设置一个"存档点"，保存当前的执行环境（栈、寄存器等）
- **longjmp** = "读档"，直接跳回之前setjmp保存的存档点，就像游戏里传送回存档点
- 正常函数调用是一层一层返回，longjmp可以跨越多层函数直接跳回
- 就像你在迷宫深处，不用一层层走回去，直接"传送"回入口

| 概念 | 类比 | 说明 |
|------|------|------|
| setjmp | 游戏存档 | 保存当前执行状态到jmp_buf |
| longjmp | 读档传送 | 跳回setjmp的位置继续执行 |
| jmp_buf | 存档文件 | 保存栈环境的数据结构 |
| 非局部跳转 | 传送门 | 跨越函数调用层直接跳转 |
| 返回值 | 存档/读档标记 | 0=存档，非0=读档 |

***

### 2. 技术说明

#### 1. setjmp/longjmp的基本原理

**setjmp**：
- 调用`setjmp(env)`时，将当前的栈环境（栈指针、程序计数器、寄存器等）保存到`jmp_buf`变量中
- 第一次调用返回0（表示"存档"成功）
- 当`longjmp`跳回时，`setjmp`再次"返回"，返回值为`longjmp`传入的`val`

**longjmp**：
- 调用`longjmp(env, val)`时，恢复`setjmp`保存的栈环境
- 程序跳回`setjmp`调用的位置，`setjmp`返回`val`
- 如果`val`为0，实际返回1（确保能区分首次调用和跳回）

#### 2. 基本用法

```c
#include <stdio.h>
#include <setjmp.h>

jmp_buf env;

void do_something(void) {
    printf("正在执行某些操作...\n");
    longjmp(env, 1);
    printf("这行不会执行\n");
}

int main(void) {
    int ret = setjmp(env);
    if (ret == 0) {
        printf("首次调用setjmp，返回值=%d\n", ret);
        do_something();
    } else {
        printf("从longjmp跳回，返回值=%d\n", ret);
    }
    return 0;
}
```

**输出**：
```
首次调用setjmp，返回值=0
正在执行某些操作...
从longjmp跳回，返回值=1
```

#### 3. 用setjmp/longjmp模拟try-catch

```c
#include <stdio.h>
#include <setjmp.h>

jmp_buf exception_env;
#define TRY     if (setjmp(exception_env) == 0)
#define CATCH   else
#define THROW   longjmp(exception_env, 1)

int divide(int a, int b) {
    if (b == 0) {
        THROW;
    }
    return a / b;
}

int main(void) {
    int a = 10, b = 0;

    TRY {
        int result = divide(a, b);
        printf("结果: %d\n", result);
    } CATCH {
        printf("捕获异常: 除零错误!\n");
    }

    printf("程序继续执行\n");
    return 0;
}
```

#### 4. 限制与风险

| 风险 | 说明 | 示例 |
|------|------|------|
| 不调用析构函数 | longjmp跳回时，中间函数的局部C++对象不会析构 | 跳过`std::string`的析构导致内存泄漏 |
| 局部变量可能失效 | longjmp回跳后，setjmp和longjmp之间函数的局部变量值不确定 | 除非声明为`volatile` |
| volatile要求 | setjmp所在函数中，setjmp之后修改的局部变量必须加`volatile` | 否则longjmp回跳后变量值可能是旧值 |
| 不可跨线程 | longjmp只能跳回同一线程中的setjmp | 跨线程行为未定义 |
| 不可跳入已返回的函数 | longjmp跳回的函数栈帧必须仍然存在 | 函数已返回后longjmp是未定义行为 |

**volatile示例**：

```c
#include <stdio.h>
#include <setjmp.h>

jmp_buf env;

int main(void) {
    volatile int x = 10;
    int y = 20;

    if (setjmp(env) == 0) {
        x = 100;
        y = 200;
        longjmp(env, 1);
    } else {
        printf("x = %d（volatile，值可靠）\n", x);
        printf("y = %d（非volatile，值可能不可靠）\n", y);
    }

    return 0;
}
```

#### 5. setjmp/longjmp与C++异常的对比

| 维度 | setjmp/longjmp | C++异常（try/catch/throw） |
|------|----------------|---------------------------|
| 语言 | C语言 | C++ |
| 栈展开 | 不展开，直接跳转 | 正确展开，调用析构函数 |
| 类型安全 | 无类型信息，只传int | 可抛出任意类型，按类型捕获 |
| 局部变量 | 需要volatile保证 | 自动正确处理 |
| 性能 | setjmp有保存开销，longjmp极快 | throw开销较大，正常路径零开销 |
| 可组合性 | 全局jmp_buf，难以嵌套 | 支持嵌套try/catch |
| 适用场景 | C语言错误恢复、信号处理 | C++程序的标准错误处理 |

#### 6. 何时使用setjmp/longjmp

| 场景 | 适合 | 原因 |
|------|------|------|
| C语言错误恢复 | ✅ | C没有异常机制，这是最接近的替代 |
| 嵌入式系统 | ✅ | 资源受限，不用C++异常 |
| 信号处理中跳转 | ✅ | 信号处理函数中不能做太多事，longjmp跳出后处理 |
| 解释器/虚拟机 | ✅ | 实现错误恢复机制 |
| C++程序 | ❌ | 应使用C++异常，确保析构函数调用 |
| 替代正常控制流 | ❌ | 类似goto，使代码难以理解 |

**信号处理中跳转示例**：

```c
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>

jmp_buf jump_buffer;

void signal_handler(int sig) {
    printf("收到信号 %d，跳回主循环\n", sig);
    longjmp(jump_buffer, sig);
}

int main(void) {
    signal(SIGINT, signal_handler);

    while (1) {
        if (setjmp(jump_buffer) == 0) {
            printf("等待输入（Ctrl+C退出）...\n");
            getchar();
        } else {
            printf("已从信号处理中恢复\n");
            break;
        }
    }

    printf("程序正常退出\n");
    return 0;
}
```

***

### 3. 代码示例：多层函数调用的错误恢复

```c
#include <stdio.h>
#include <stdlib.h>
#include <setjmp.h>

jmp_buf error_handler;

#define ERROR_INVALID_INPUT  1
#define ERROR_OUT_OF_MEMORY  2
#define ERROR_IO_FAILURE     3

void* level3_process(int value) {
    if (value < 0) {
        printf("  层3: 输入无效，抛出错误\n");
        longjmp(error_handler, ERROR_INVALID_INPUT);
    }

    void* buffer = malloc(1024);
    if (!buffer) {
        printf("  层3: 内存不足，抛出错误\n");
        longjmp(error_handler, ERROR_OUT_OF_MEMORY);
    }

    printf("  层3: 处理成功\n");
    return buffer;
}

void level2_process(int value) {
    printf(" 层2: 开始处理\n");
    void* buf = level3_process(value);
    printf(" 层2: 处理完成\n");
    free(buf);
}

void level1_process(int value) {
    printf("层1: 开始处理\n");
    level2_process(value);
    printf("层1: 处理完成\n");
}

const char* error_name(int code) {
    switch (code) {
        case ERROR_INVALID_INPUT: return "输入无效";
        case ERROR_OUT_OF_MEMORY: return "内存不足";
        case ERROR_IO_FAILURE:    return "IO失败";
        default:                  return "未知错误";
    }
}

int main(void) {
    int test_values[] = {42, -1};
    int count = sizeof(test_values) / sizeof(test_values[0]);

    for (int i = 0; i < count; i++) {
        printf("\n--- 测试 %d: value=%d ---\n", i + 1, test_values[i]);

        int err = setjmp(error_handler);
        if (err == 0) {
            level1_process(test_values[i]);
            printf("结果: 成功\n");
        } else {
            printf("结果: 失败，错误码=%d (%s)\n", err, error_name(err));
        }
    }

    return 0;
}
```

**输出**：

```
--- 测试 1: value=42 ---
层1: 开始处理
 层2: 开始处理
  层3: 处理成功
 层2: 处理完成
层1: 处理完成
结果: 成功

--- 测试 2: value=-1 ---
层1: 开始处理
 层2: 开始处理
  层3: 输入无效，抛出错误
结果: 失败，错误码=1 (输入无效)
```

***

### 4. 常见问题

#### 1. 问题1：longjmp跳回后，中间函数的局部变量会怎样

中间函数中未加`volatile`的自动变量值不确定。因为编译器可能将变量优化到寄存器中，longjmp恢复的是setjmp时的寄存器值，而非longjmp时的值。如果需要在longjmp后使用某个变量，必须声明为`volatile`。

#### 2. 问题2：setjmp/longjmp和goto有什么区别

goto只能在同一函数内跳转；longjmp可以跨越函数调用栈跳转。goto是结构化的跳转（编译器可见），longjmp是非结构化的（运行时行为）。goto不会破坏栈，longjmp会"丢弃"中间的栈帧。

#### 3. 问题3：可以在信号处理函数中调用longjmp吗

可以，但要小心。POSIX规定信号处理函数中可以调用longjmp，但必须注意：如果在信号处理期间longjmp跳出了某些不安全的函数（如malloc），可能导致未定义行为。建议使用`sigsetjmp`/`siglongjmp`来正确恢复信号掩码。

#### 4. 问题4：C++程序中能用setjmp/longjmp吗

技术上可以，但强烈不建议。longjmp不会调用析构函数，会破坏RAII机制，导致资源泄漏。C++程序应使用try/catch/throw。

***

### 5. 极简总结

**setjmp保存栈环境（存档），longjmp恢复栈环境（读档），实现C语言的非局部跳转。可用于错误恢复、信号处理跳转、嵌入式系统。限制：不调用C++析构函数、局部变量需volatile、不可跨线程。C++程序应优先使用异常机制。**

| 要点 | 一句话 |
|------|--------|
| setjmp | 保存栈环境——"存档"，首次返回0 |
| longjmp | 恢复栈环境——"读档"，跳回setjmp位置 |
| jmp_buf | 存档数据结构——保存栈指针、寄存器等 |
| volatile | setjmp后修改的局部变量必须加volatile |
| 析构函数 | longjmp不调用C++析构函数——C++程序别用 |
| 信号处理 | 信号处理函数中可用longjmp跳出 |
| C++异常 | C++程序应使用try/catch/throw——安全且类型化 |

***

### 6. longjmp的严重陷阱详解

> longjmp不是"安全传送"，而是"强行撕裂栈帧后瞬移"——每一步都踩在未定义行为的边缘。

#### 1. 局部变量失效：volatile的必要性

longjmp回跳后，setjmp所在函数中在setjmp之后修改的局部变量，其值是**不确定的**——除非声明为`volatile`。这不是"可能出错"，而是C标准明确规定的未定义行为。

**根本原因**：编译器会将频繁访问的局部变量优化到寄存器中。setjmp保存了当时的寄存器快照，longjmp恢复的是setjmp时刻的寄存器值，而非longjmp时刻的值。`volatile`强制编译器每次都从内存读取，绕过寄存器缓存。

```c
#include <stdio.h>
#include <setjmp.h>

jmp_buf env;

int main(void) {
    volatile int safe_var = 0;
    int unsafe_var = 0;

    if (setjmp(env) == 0) {
        safe_var = 42;
        unsafe_var = 42;
        printf("修改后: safe_var=%d, unsafe_var=%d\n", safe_var, unsafe_var);
        longjmp(env, 1);
    } else {
        printf("跳回后: safe_var=%d, unsafe_var=%d\n", safe_var, unsafe_var);
    }

    return 0;
}
```

**可能的输出**（取决于编译器优化级别）：
```
修改后: safe_var=42, unsafe_var=42
跳回后: safe_var=42, unsafe_var=0
```

unsafe_var被优化到了寄存器，longjmp恢复的是setjmp时的寄存器值（0），而非修改后的42。safe_var因volatile而每次从内存读取，值可靠。

**规则**：在setjmp所在函数中，任何在setjmp调用之后、longjmp跳回之前被修改的局部变量，如果需要在longjmp跳回后使用，**必须**声明为`volatile`。这条规则适用于所有自动存储期的变量，包括基本类型、指针、结构体。

#### 2. 栈帧已销毁后跳回：最危险的未定义行为

如果longjmp试图跳回一个**已经返回的函数**中的setjmp点，程序的行为是完全未定义的——通常表现为崩溃、数据损坏或看似正常运行实则内存已被破坏。

```c
#include <stdio.h>
#include <setjmp.h>

jmp_buf env;

void dangerous_function(void) {
    int local = 999;
    if (setjmp(env) == 0) {
        printf("设置跳转点，local=%d\n", local);
    }
}

void caller(void) {
    dangerous_function();
}

void trigger_bug(void) {
    printf("尝试跳回已返回的函数...\n");
    longjmp(env, 1);
}

int main(void) {
    caller();
    trigger_bug();
    return 0;
}
```

**这是未定义行为**。`dangerous_function`返回后，其栈帧已被回收。longjmp试图恢复一个不存在的栈帧，可能：
- 覆盖其他函数的局部变量
- 跳转到已被覆盖的返回地址
- 看似正常执行但数据已损坏（最危险的情况）

**规则**：longjmp跳回的setjmp所在函数**必须仍在调用栈上**——即尚未返回。

#### 3. C++析构函数不调用：RAII的致命破坏

这是C++程序中使用longjmp最严重的后果。longjmp直接恢复栈指针，**跳过了所有中间栈帧上局部对象的析构函数调用**，彻底破坏RAII机制。

```cpp
#include <iostream>
#include <setjmp.h>
#include <fstream>

jmp_buf env;

class Resource {
public:
    Resource() { std::cout << "Resource::Resource() — 获取资源\n"; }
    ~Resource() { std::cout << "Resource::~Resource() — 释放资源\n"; }
};

void inner_work() {
    Resource r;
    std::ofstream file("temp.txt");
    file << "一些数据";

    int* heap = new int[1000];

    std::cout << "即将longjmp...\n";
    longjmp(env, 1);

    delete[] heap;
}

int main() {
    if (setjmp(env) == 0) {
        inner_work();
    } else {
        std::cout << "已跳回\n";
    }
    return 0;
}
```

**输出**：
```
Resource::Resource() — 获取资源
即将longjmp...
已跳回
```

注意：`Resource::~Resource()`、`std::ofstream`的析构、`delete[] heap`全部被跳过。后果：
- **内存泄漏**：heap分配的内存永远不会释放
- **文件资源泄漏**：ofstream未关闭，数据可能未刷盘
- **锁未释放**：如果Resource持有互斥锁，将导致死锁
- **数据库事务未回滚**：RAII守护的事务将永远悬挂

C++标准（[support.runtime]/4）明确规定：如果longjmp的跳转会替换掉任何自动对象的析构函数调用，则行为未定义。这意味着即使程序"看起来正常"，也是UB。

***

### 7. 与C++异常机制的完整对比

#### 1. 语义差异

| 维度 | setjmp/longjmp | C++异常 (try/catch/throw) |
|------|----------------|---------------------------|
| 跳转语义 | 直接恢复栈状态，无栈展开 | 逐帧栈展开（stack unwinding） |
| 析构调用 | 不调用任何析构函数 | 保证调用所有自动对象的析构函数 |
| 类型系统 | 只传递int值 | 可抛出任意类型，按类型匹配catch |
| 异常安全 | 无保证，破坏RAII | 支持基本/强/不抛出三级保证 |
| 嵌套处理 | 全局jmp_buf，嵌套困难 | try/catch天然支持嵌套和重新抛出 |
| 错误信息 | 仅一个int，信息贫乏 | 异常对象可携带丰富上下文 |
| 编译器支持 | C标准库，无特殊编译要求 | 需编译器实现异常表和展开逻辑 |

#### 2. 性能对比

```cpp
#include <iostream>
#include <setjmp.h>
#include <chrono>
#include <stdexcept>

jmp_buf env;

void deep_call_longjmp(int depth) {
    if (depth == 0) {
        longjmp(env, 1);
    }
    deep_call_longjmp(depth - 1);
}

void deep_call_exception(int depth) {
    if (depth == 0) {
        throw std::runtime_error("error");
    }
    deep_call_exception(depth - 1);
}

int main() {
    const int N = 100000;
    const int DEPTH = 10;

    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) {
        if (setjmp(env) == 0) {
            deep_call_longjmp(DEPTH);
        }
    }
    auto t2 = std::chrono::high_resolution_clock::now();
    std::cout << "longjmp: "
              << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()
              << " us\n";

    t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; i++) {
        try {
            deep_call_exception(DEPTH);
        } catch (...) {}
    }
    t2 = std::chrono::high_resolution_clock::now();
    std::cout << "exception: "
              << std::chrono::duration_cast<std::chrono::microseconds>(t2 - t1).count()
              << " us\n";

    return 0;
}
```

**典型结果**（因平台而异）：
```
longjmp: 15000 us
exception: 45000 us
```

**性能分析**：

| 阶段 | setjmp/longjmp | C++异常 |
|------|----------------|---------|
| 正常路径（无异常） | setjmp每次调用有保存开销 | 零开销（仅编译期生成异常表） |
| 异常路径（抛出时） | longjmp极快，直接恢复 | 较慢，需逐帧展开并查找匹配handler |
| 代码体积 | 小 | 较大（异常表、展开信息） |

关键洞察：**C++异常的设计哲学是"零开销原则"——正常路径不付出代价，异常路径才付出代价**。如果你的程序90%走正常路径，C++异常的总开销反而可能更低。而setjmp每次进入try区域都有保存环境的开销，即使异常从不发生。

#### 3. 安全性对比

| 安全维度 | setjmp/longjmp | C++异常 |
|----------|----------------|---------|
| 资源泄漏 | 极易泄漏（不调用析构） | RAII保证不泄漏 |
| 类型安全 | 无，仅传int | 强类型，编译期检查 |
| 异常安全保证 | 无法实现 | 支持三级保证 |
| 可调试性 | 难以追踪跳转来源 | 调试器可捕获throw点 |
| 可组合性 | 全局jmp_buf冲突 | 嵌套try/catch自然组合 |
| 多线程 | 不可跨线程 | 每线程独立异常栈 |

**结论**：在C++代码中，**永远不要使用setjmp/longjmp替代异常**。longjmp唯一的合理使用场景是纯C代码或与C代码交互的边界。

***

### 8. 实际工程中的使用场景和替代方案

#### 1. 错误恢复：Lua和PostgreSQL的实践

**Lua虚拟机**使用setjmp/longjmp实现错误处理：

```c
/* Lua源码简化版 — ldo.c */
typedef struct lua_State {
    jmp_buf errorJmp;
    int status;
} lua_State;

int luaD_rawrunprotected(lua_State *L, Pfunc f, void *ud) {
    volatile int status = LUA_OK;
    if (setjmp(L->errorJmp) == 0) {
        f(L, ud);
    } else {
        status = L->status;
    }
    return status;
}

void luaG_errormsg(lua_State *L) {
    if (L->errorJmp) {
        L->status = LUA_ERRRUN;
        longjmp(L->errorJmp, 1);
    }
}
```

Lua选择longjmp的原因：
- Lua核心是纯C代码，没有C++异常可用
- 虚拟机需要一个统一的错误恢复点
- longjmp的开销可预测，适合嵌入式场景
- Lua自行管理所有资源，不依赖析构函数

**PostgreSQL**使用setjmp/longjmp实现事务回滚：

```c
/* PostgreSQL简化版 — postgres.c */
sigjmp_buf Warn_restart;

void PostgresMain(int argc, char *argv[]) {
    if (sigsetjmp(Warn_restart, 1) != 0) {
        AbortCurrentTransaction();
        ResetAllAfterError();
    }

    for (;;) {
        if (sigsetjmp(Warn_restart, 1) == 0) {
            exec_simple_query(query_string);
        } else {
            HandleErrorRecovery();
        }
    }
}

void elog_finish(int elevel) {
    if (elevel >= ERROR) {
        siglongjmp(Warn_restart, 1);
    }
}
```

PostgreSQL选择longjmp的原因：
- 数据库需要从任意深度的事务处理中快速回滚
- 错误可能发生在解析、规划、执行的任何阶段
- 使用sigsetjmp/siglongjmp确保信号掩码正确恢复

#### 2. 协程实现基础

setjmp/longjmp是实现用户级协程（协程切换）的基础原语之一：

```c
#include <stdio.h>
#include <setjmp.h>

#define MAX_COROUTINES 4

typedef struct {
    jmp_buf env;
    int active;
} Coroutine;

Coroutine coroutines[MAX_COROUTINES];
int current = 0;
jmp_buf scheduler;

void yield(void) {
    if (setjmp(coroutines[current].env) == 0) {
        longjmp(scheduler, 1);
    }
}

void coroutine_entry(int id) {
    for (int i = 0; i < 3; i++) {
        printf("协程%d: 步骤%d\n", id, i);
        yield();
    }
    coroutines[current].active = 0;
    longjmp(scheduler, 1);
}

void schedule(void) {
    while (1) {
        int found = 0;
        for (int i = 0; i < MAX_COROUTINES; i++) {
            if (coroutines[i].active) {
                current = i;
                found = 1;
                if (setjmp(scheduler) == 0) {
                    longjmp(coroutines[i].env, 1);
                }
            }
        }
        if (!found) break;
    }
}

int main(void) {
    for (int i = 0; i < MAX_COROUTINES; i++) {
        coroutines[i].active = 1;
        if (setjmp(coroutines[i].env) == 0) {
            coroutine_entry(i);
        }
    }
    schedule();
    printf("所有协程执行完毕\n");
    return 0;
}
```

**注意**：这种协程实现是**有栈协程**的简化版，存在严重限制——每个协程共享栈空间，无法处理嵌套调用。生产级实现（如libco、boost.context）使用`swapcontext`或手写汇编来切换完整栈，而非仅保存寄存器。

#### 3. 何时该用/不该用：决策指南

```
                    是否使用setjmp/longjmp？
                           │
                    ┌──────┴──────┐
                    │             │
               纯C代码？      C++代码？
                    │             │
              ┌─────┴─────┐      │
              │           │      │
         需要非局部    不需要    → 用C++异常
         跳转恢复？    → 用错误码
              │
        ┌─────┴─────┐
        │           │
   信号处理？    错误恢复？
        │           │
   用sigsetjmp   用setjmp
   /siglongjmp   /longjmp
```

| 场景 | 决策 | 原因 |
|------|------|------|
| C语言解释器/虚拟机 | ✅ 使用 | 需要统一的错误恢复点，无C++异常 |
| C语言信号处理 | ✅ 使用sigsetjmp | 需要从信号中跳出恢复执行 |
| 嵌入式C代码 | ✅ 使用 | 资源受限，异常机制不可用 |
| C++代码 | ❌ 不使用 | 破坏RAII，用C++异常 |
| 替代正常控制流 | ❌ 不使用 | 使代码不可读，用goto或重构 |
| 跨线程通信 | ❌ 不使用 | 行为未定义，用消息队列 |

**替代方案**：

| 替代方案 | 适用场景 | 优势 |
|----------|----------|------|
| 错误码返回 | 简单函数 | 显式、可追踪、无UB风险 |
| C++异常 | C++代码 | 类型安全、RAII友好 |
| 错误处理链（如nginx） | C语言大型项目 | 显式错误传播，可调试 |
| goto集中清理 | C语言单函数内 | 清晰的资源清理路径 |
| 线程取消点 | 多线程场景 | POSIX标准，可取消 |

***

### 9. 信号处理中的longjmp

#### 1. 从信号处理函数中跳出的注意事项

信号处理函数是特殊的执行上下文——它中断正常程序流，在不可预测的时机执行。从中longjmp跳出需要格外小心。

**POSIX规定的安全要求**：

| 要求 | 说明 |
|------|------|
| 使用sigsetjmp/siglongjmp | 普通setjmp/longjmp不保存信号掩码，可能导致信号被永久阻塞 |
| 跳出时不处于不安全函数中 | 如果信号打断了malloc/printf等不安全函数，longjmp可能导致堆损坏 |
| volatile sig_atomic_t标志 | 复杂逻辑中应先设置标志，在主循环中检查，而非直接longjmp |
| 避免嵌套信号 | longjmp前应屏蔽该信号，防止跳回后立即再次触发 |

#### 2. sigsetjmp/siglongjmp：信号安全的版本

```c
#include <stdio.h>
#include <signal.h>
#include <setjmp.h>

sigjmp_buf sig_env;
volatile sig_atomic_t signal_received = 0;

void sigint_handler(int sig) {
    signal_received = sig;
    siglongjmp(sig_env, sig);
}

int main(void) {
    struct sigaction sa;
    sa.sa_handler = sigint_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    sigaction(SIGINT, &sa, NULL);

    printf("按Ctrl+C触发信号...\n");

    if (sigsetjmp(sig_env, 1) == 0) {
        while (1) {
            pause();
        }
    } else {
        printf("从信号%d恢复，信号掩码已正确还原\n", signal_received);
    }

    return 0;
}
```

**sigsetjmp与setjmp的关键区别**：

| 维度 | setjmp/longjmp | sigsetjmp/siglongjmp |
|------|----------------|---------------------|
| 信号掩码 | 不保存/恢复 | 保存并恢复（当savemask!=0时） |
| 信号安全 | 不保证 | POSIX明确支持从信号处理函数中调用 |
| 可移植性 | C标准 | POSIX标准（非C标准） |
| 用途 | 普通错误恢复 | 信号处理跳转 |

#### 3. 可移植性问题

| 平台/标准 | setjmp/longjmp | sigsetjmp/siglongjmp |
|-----------|----------------|---------------------|
| C89/C99/C11 | ✅ 标准支持 | ❌ 非C标准 |
| POSIX.1-2001 | ✅ 支持 | ✅ 标准支持 |
| Windows (MSVC) | ✅ 支持 | ❌ 无sigsetjmp（用setjmp替代） |
| 嵌入式 (freestanding) | ✅ 通常支持 | ❌ 通常不可用 |

**Windows平台的变通方案**：

```c
#ifdef _WIN32
    #define sigjmp_buf jmp_buf
    #define sigsetjmp(env, savemask) setjmp(env)
    #define siglongjmp(env, val) longjmp(env, val)
#else
    #include <setjmp.h>
#endif
```

#### 4. 信号处理中longjmp的危险场景

```c
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <setjmp.h>

jmp_buf env;

void handler(int sig) {
    longjmp(env, 1);
}

int main(void) {
    signal(SIGINT, handler);

    if (setjmp(env) == 0) {
        void *p = malloc(1024);
        /* 如果SIGINT在malloc内部触发，
         * malloc的内部堆结构可能处于不一致状态，
         * longjmp跳过后堆已损坏——后续任何malloc/free都可能崩溃 */
        free(p);
    } else {
        printf("已恢复，但堆可能已损坏！\n");
    }

    return 0;
}
```

**安全做法**：在信号处理函数中只设置`volatile sig_atomic_t`标志，在主循环的安全点检查标志：

```c
#include <stdio.h>
#include <signal.h>

volatile sig_atomic_t got_signal = 0;

void handler(int sig) {
    got_signal = 1;
}

int main(void) {
    signal(SIGINT, handler);

    while (!got_signal) {
        /* 安全的工作循环 */
    }
    printf("安全退出\n");
    return 0;
}
```

这种方式虽然不如longjmp直接，但避免了所有信号安全性和可移植性问题，是**推荐的信号处理模式**。

***

### 相关阅读

- [什么是信号处理signal](./08-什么是信号处理signal.md)
- [内核态与用户态](./01-内核态与用户态.md)
- [什么是栈展开Stack-Unwinding](../08-调试与性能/08-什么是栈展开Stack-Unwinding.md)