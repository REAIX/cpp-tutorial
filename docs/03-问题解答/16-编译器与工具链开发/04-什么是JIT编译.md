# 什么是JIT编译
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)、[编译器原理](../../10-编译器与工具链开发/00-编译器原理概述.md)

> "本质洞察：JIT（Just-In-Time）编译是在程序运行时将代码编译为机器码的技术，它结合了解释器的快速启动和AOT编译器的高性能，还能利用运行时信息进行AOT无法做到的优化——比如根据实际数据类型内联虚函数、根据分支频率调整代码布局。"

***

## 1. JIT的原理与适用场景

### 1. 三种执行方式对比

```
┌──────────────────────────────────────────────────────────┐
│                                                          │
│  解释执行（Interpreter）                                  │
│  ┌─────────┐    ┌──────────┐    ┌──────┐                │
│  │ 源代码   │ → │ 逐行解释  │ → │ 执行  │                │
│  └─────────┘    └──────────┘    └──────┘                │
│  特点：启动快，运行慢                                    │
│                                                          │
│  AOT编译（Ahead-Of-Time）                                │
│  ┌─────────┐    ┌──────────┐         ┌──────┐           │
│  │ 源代码   │ → │ 编译为机器码 │ → 保存 → │ 执行  │           │
│  └─────────┘    └──────────┘         └──────┘           │
│  特点：启动慢（需先编译），运行快                         │
│                                                          │
│  JIT编译（Just-In-Time）                                 │
│  ┌─────────┐    ┌──────────┐    ┌──────┐                │
│  │ 源代码   │ → │ 运行时编译 │ → │ 执行  │                │
│  └─────────┘    └──────────┘    └──────┘                │
│  特点：启动较快，运行越来越快                             │
│                                                          │
└──────────────────────────────────────────────────────────┘
```

### 2. JIT的核心原理

```
JIT编译的执行流程：

  字节码 / IR
      │
      ▼
  ┌──────────────┐
  │ 解释执行      │ ← 第一阶段：快速启动
  │ 收集Profile   │   同时收集运行时信息
  └──────┬───────┘
         │ 发现热点代码（频繁执行的代码路径）
         ▼
  ┌──────────────┐
  │ JIT编译       │ ← 第二阶段：编译热点
  │ 生成机器码    │   只编译频繁执行的代码
  └──────┬───────┘
         │ 后续调用直接执行机器码
         ▼
  ┌──────────────┐
  │ 优化执行      │ ← 第三阶段：持续优化
  │ 基于Profile   │   利用运行时信息做更激进的优化
  └──────────────┘

JIT的关键优势：
  └── 运行时类型信息 — 知道虚函数的实际目标，可以内联
  └── 运行时分支信息 — 知道哪个分支更可能执行
  └── 运行时值信息 — 知道某些值是常量，可以常量折叠
  └── 按需编译 — 只编译实际用到的代码，节省时间和内存
```

### 3. JIT的适用场景

```
JIT特别适合以下场景：

1. 动态类型语言
   └── Python、JavaScript、Ruby等
   └── 运行时才知道变量类型，AOT无法优化
   └── JIT可以根据实际类型生成特化代码

2. 长时间运行的服务
   └── Web服务器、数据库、游戏引擎
   └── 启动时间不是瓶颈，运行时性能更重要
   └── JIT可以持续优化热路径

3. 代码在运行时生成
   └── 正则表达式引擎、SQL查询引擎
   └── 代码在运行时才确定，无法AOT编译
   └── JIT可以将模式匹配编译为机器码

4. 需要跨平台分发
   └── Java字节码、.NET IL、WebAssembly
   └── 分发中间代码，运行时JIT为当前平台生成机器码
   └── 一次编写，到处运行

JIT不适合的场景：
  └── 嵌入式系统 — 内存和计算资源有限
  └── 启动时间敏感 — 命令行工具、脚本
  └── 实时系统 — JIT编译的停顿不可接受
```

***

## 2. LLVM ORC JIT

### 1. LLVM JIT的演进

```
LLVM JIT的发展历程：

1. LLVM MCJIT（已弃用）
   └── LLVM 3.0引入
   └── 基于旧的执行引擎
   └── 功能有限，不够灵活

2. LLVM ORC JIT v1（已弃用）
   └── LLVM 3.7引入
   └── 更模块化的设计
   └── 但API不够直观

3. LLVM ORC JIT v2（推荐）
   └── LLVM 7.0引入
   └── 完全重新设计
   └── 支持懒编译、并发编译
   └── 更好的资源管理
   └── 当前推荐使用的版本
```

### 2. ORC JIT的基本使用

```cpp
// 使用LLVM ORC JIT执行IR代码
#include "llvm/ExecutionEngine/Orc/LLJIT.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/TargetSelect.h"

using namespace llvm;
using namespace llvm::orc;

// 退出函数
ExitOnError exitOnErr;

int main() {
    // 初始化目标
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();

    // 创建JIT实例
    auto jit = exitOnErr(LLJITBuilder().create());

    // 创建LLVM上下文和模块
    auto ctx = std::make_unique<LLVMContext>();
    auto mod = std::make_unique<Module>("jit_module", *ctx);
    mod->setDataLayout(jit->getDataLayout());

    // 生成IR：定义一个add函数
    IRBuilder<> builder(*ctx);
    FunctionType* funcType = FunctionType::get(
        Type::getInt32Ty(*ctx),
        {Type::getInt32Ty(*ctx), Type::getInt32Ty(*ctx)},
        false
    );
    Function* addFunc = Function::Create(
        funcType, Function::ExternalLinkage, "add", mod.get()
    );

    // 设置参数名
    auto args = addFunc->arg_begin();
    Value* a = &(*args++);
    a->setName("a");
    Value* b = &(*args);
    b->setName("b");

    // 创建函数体
    BasicBlock* entry = BasicBlock::Create(*ctx, "entry", addFunc);
    builder.SetInsertPoint(entry);
    Value* result = builder.CreateAdd(a, b, "result");
    builder.CreateRet(result);

    // 将模块添加到JIT
    exitOnErr(jit->addIRModule(ThreadSafeModule(std::move(mod), std::move(ctx))));

    // 查找并执行函数
    auto sym = exitOnErr(jit->lookup("add"));
    auto* addPtr = sym.toPtr<int(int, int)>();

    // 调用JIT编译的函数
    int res = addPtr(3, 4);
    printf("add(3, 4) = %d\n", res);  // 输出: add(3, 4) = 7

    return 0;
}
```

### 3. ORC JIT的高级特性

```cpp
// 懒编译——只在函数首次调用时才编译
#include "llvm/ExecutionEngine/Orc/LLJIT.h"

// 启用懒编译
auto jit = exitOnErr(
    LLJITBuilder()
        .setNumCompileThreads(2)           // 编译线程数
        .setLazyCompileSupport(true)       // 启用懒编译
        .create()
);

// 懒编译的好处：
// ├── 启动更快 — 不需要编译所有代码
// ├── 节省内存 — 只编译实际用到的代码
// └── 适合大型程序 — 大部分代码可能不会被执行
```

```cpp
// 动态添加代码——运行时持续添加新函数
void addNewFunction(LLJIT& jit, LLVMContext& ctx, const std::string& name) {
    auto mod = std::make_unique<Module>("dynamic", ctx);

    // 创建新函数
    FunctionType* ft = FunctionType::get(
        Type::getInt32Ty(ctx), {Type::getInt32Ty(ctx)}, false);
    Function* f = Function::Create(ft, Function::ExternalLinkage, name, mod.get());

    BasicBlock* bb = BasicBlock::Create(ctx, "entry", f);
    IRBuilder<> builder(bb);
    Value* arg = &(*f->arg_begin());
    Value* result = builder.CreateMul(arg, ConstantInt::get(Type::getInt32Ty(ctx), 2));
    builder.CreateRet(result);

    // 动态添加到JIT
    exitOnErr(jit.addIRModule(
        ThreadSafeModule(std::move(mod), std::make_unique<LLVMContext>())));
}
```

***

## 3. JIT vs AOT的性能对比

### 1. 启动性能

```
启动时间对比（典型场景）：

  ┌─────────────────────────────────────────┐
  │  执行方式    │ 启动时间    │ 首次响应时间 │
  │  ───────────┼───────────┼──────────── │
  │  解释执行    │ 10ms      │ 10ms        │
  │  AOT编译    │ 500ms     │ 500ms       │
  │  JIT(急切)  │ 200ms     │ 200ms       │
  │  JIT(懒编译)│ 50ms      │ 100ms       │
  └─────────────────────────────────────────┘

JIT的启动优势：
  └── 懒编译模式下，只编译入口函数
  └── 其他函数在首次调用时才编译
  └── 对于大型应用，启动时间可以减少90%+
```

### 2. 峰值性能

```
稳态性能对比（长时间运行后）：

  ┌────────────────────────────────────────────────────┐
  │  场景              │ AOT    │ JIT    │ JIT优势      │
  │  ─────────────────┼───────┼───────┼────────────  │
  │  纯计算循环        │ 100%  │ 95%   │ 略慢          │
  │  虚函数密集调用    │ 100%  │ 150%  │ 内联优化      │
  │  分支密集代码      │ 100%  │ 120%  │ 分支预测优化  │
  │  动态类型代码      │ N/A   │ 200%+ │ 类型特化      │
  │  多态调用          │ 100%  │ 180%  │ 去虚化        │
  └────────────────────────────────────────────────────┘

JIT可以超越AOT的场景：
  └── 虚函数内联 — JIT知道实际类型，可以消除虚调用
  └── 分支布局优化 — JIT知道分支概率，可以优化代码布局
  └── 类型特化 — JIT可以为具体类型生成特化代码
  └── 逃逸分析 — JIT可以在栈上分配原本在堆上的对象
```

### 3. 实际基准测试示例

```cpp
// 基准测试：虚函数调用性能
#include <cstdio>
#include <chrono>

struct Base {
    virtual int compute(int x) = 0;
    virtual ~Base() = default;
};

struct ImplA : Base {
    int compute(int x) override { return x * 2; }
};

struct ImplB : Base {
    int compute(int x) override { return x * 3; }
};

int main() {
    // 运行时决定使用哪个实现
    Base* obj = new ImplA();  // JIT知道obj实际指向ImplA

    auto start = std::chrono::high_resolution_clock::now();
    int sum = 0;
    for (int i = 0; i < 100000000; i++) {
        sum += obj->compute(i);  // 虚函数调用
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    printf("sum = %d, 耗时: %lld ms\n", sum, ms.count());

    delete obj;
    return 0;
}

// AOT编译：每次调用都要查虚函数表 → 间接跳转
// JIT编译：发现obj总是ImplA，内联compute → 直接乘法
// JIT可能快2-5倍
```

***

## 4. JIT在PyTorch/JVM/V8中的应用

### 1. PyTorch JIT（TorchScript）

```
PyTorch的JIT编译流程：

  Python代码
      │
      ▼
  ┌──────────────┐
  │ TorchScript  │ ← 将Python代码转换为静态图
  │ 追踪/脚本化  │   tracing: 记录实际执行路径
  └──────┬───────┘   scripting: 分析代码结构
         │
         ▼
  ┌──────────────┐
  │ JIT编译器     │ ← 优化计算图
  │ 图优化       │   算子融合、常量折叠、死代码消除
  └──────┬───────┘
         │
         ▼
  ┌──────────────┐
  │ 代码生成     │ ← 生成高效的机器码
  │ CPU/GPU      │   针对特定硬件优化
  └──────────────┘

PyTorch JIT的优势：
  └── 消除Python解释器开销
  └── 算子融合（将多个小操作合并为一个大操作）
  └── 自动微分优化
  └── 模型导出（不依赖Python运行时）
```

```python
# PyTorch JIT示例
import torch

@torch.jit.script
def fused_op(x: torch.Tensor, y: torch.Tensor) -> torch.Tensor:
    # TorchScript会将这些操作融合为一个内核
    z = x + y          # 加法
    z = z * 2.0        # 乘法
    z = torch.relu(z)  # ReLU激活
    return z

# 普通Python版本（3次内核启动）
def normal_op(x, y):
    z = x + y          # 内核1
    z = z * 2.0        # 内核2
    z = torch.relu(z)  # 内核3
    return z

# JIT版本可能快2-3倍（1次内核启动，融合操作）
```

### 2. JVM JIT（HotSpot C1/C2）

```
JVM HotSpot的分层编译：

  ┌──────────────────────────────────────────────────┐
  │                                                  │
  │  Java字节码                                      │
  │      │                                           │
  │      ▼                                           │
  │  解释执行（收集Profile）                           │
  │      │                                           │
  │      ├── 方法调用次数超过阈值 ──→ C1编译（客户端编译器）│
  │      │   快速编译，基本优化                         │
  │      │                                           │
  │      ├── 热点方法进一步升温 ──→ C2编译（服务端编译器） │
  │      │   慢速编译，激进优化                         │
  │      │   基于Profile的优化                          │
  │      │                                           │
  │      └── 优化假设失败 ──→ 去优化（Deoptimization）  │
  │          回退到解释执行，重新收集Profile             │
  │                                                  │
  └──────────────────────────────────────────────────┘

C2编译器的优化技术：
  ├── 逃逸分析 — 栈上分配、锁消除
  ├── 内联 — 消除方法调用开销
  ├── 循环优化 — 展开、向量化
  ├── 分支频率 — 基于Profile的代码布局
  └── 去虚化 — 将虚方法调用转换为直接调用
```

```java
// JVM JIT优化示例
public class JitDemo {
    // 虚方法调用
    static int process(Processor p, int n) {
        int sum = 0;
        for (int i = 0; i < n; i++) {
            sum += p.compute(i);  // 虚方法调用
        }
        return sum;
    }

    public static void main(String[] args) {
        Processor p = new FastProcessor();  // 总是同一个类型

        // 预热：让JIT收集Profile
        for (int i = 0; i < 10000; i++) {
            process(p, 100);
        }

        // JIT发现p总是FastProcessor
        // 将虚方法调用内联为直接调用
        long start = System.nanoTime();
        process(p, 100_000_000);
        long end = System.nanoTime();
        System.out.println("耗时: " + (end - start) / 1_000_000 + "ms");
    }
}
```

### 3. V8 JIT（Sparkplug + TurboFan/Maglev）

```
V8引擎的JIT架构（Chrome/Node.js）：

  JavaScript源码
      │
      ▼
  ┌──────────────┐
  │ 解析器        │ → 生成AST
  └──────┬───────┘
         │
         ▼
  ┌──────────────┐
  │ Ignition     │ ← 解释执行字节码
  │ 解释器        │   收集类型反馈（Type Feedback）
  └──────┬───────┘
         │ 短时间热身
         ▼
  ┌──────────────┐
  │ Sparkplug    │ ← 非优化JIT编译器
  │ (快速JIT)    │   快速生成机器码，不做复杂优化
  └──────┬───────┘   消除解释器开销
         │ 长时间热点
         ▼
  ┌──────────────┐
  │ Maglev       │ ← 中等优化JIT编译器
  │ (中层JIT)    │   基于类型反馈优化
  └──────┬───────┘   平衡编译时间和代码质量
         │ 极热代码
         ▼
  ┌──────────────┐
  │ TurboFan     │ ← 优化JIT编译器
  │ (优化JIT)    │   激进优化（内联、逃逸分析等）
  └──────┬───────┘   基于Sea-of-Nodes IR
         │ 优化假设失败
         ▼
  ┌──────────────┐
  │ 去优化        │ ← 回退到解释器
  └──────────────┘

V8的类型特化优化：
  JavaScript: function add(a, b) { return a + b; }
  └── 如果总是传入整数，JIT生成: 整数加法机器码
  └── 如果总是传入浮点数，JIT生成: 浮点加法机器码
  └── 如果混合传入，JIT生成: 类型检查+分支代码
  └── 类型改变时，去优化并重新编译
```

```javascript
// V8 JIT优化示例
function sum(arr) {
    let s = 0;
    for (let i = 0; i < arr.length; i++) {
        s += arr[i];
    }
    return s;
}

// 预热：让V8收集类型信息
const numbers = [1, 2, 3, 4, 5];
for (let i = 0; i < 10000; i++) {
    sum(numbers);  // V8发现arr总是整数数组
}

// V8为整数数组生成特化代码
// 如果传入字符串数组，会触发去优化
console.time('jit');
const bigArr = new Array(1000000).fill(0).map((_, i) => i);
sum(bigArr);
console.timeEnd('jit');
```

***

## 5. JIT的挑战与权衡

### 1. 内存开销

```
JIT的内存开销来源：
  ├── 编译后的机器码 — 每个编译的方法都需要存储
  ├── Profile数据 — 运行时收集的类型和频率信息
  ├── 编译器本身 — JIT编译器也是代码，需要内存
  └── 去优化信息 — 需要保存原始信息以便回退

典型内存开销：
  ┌──────────────┬──────────────┐
  │ 运行方式      │ 额外内存开销  │
  │ ────────────┼────────────  │
  │ 纯解释       │ 很少         │
  │ AOT编译      │ 少           │
  │ JIT编译      │ 中等~大量    │
  └──────────────┴──────────────┘

应对策略：
  ├── 代码缓存淘汰 — 淘汰长时间未使用的编译代码
  ├── 分层编译 — 冷代码用简单JIT，热代码用优化JIT
  └── 内存预算 — 限制JIT使用的最大内存
```

### 2. 编译停顿

```
JIT编译可能导致的停顿：

  问题：编译热点代码需要时间，可能导致暂停
  └── C2编译器编译一个方法可能需要数十毫秒
  └── 在低延迟场景（游戏、交易）中不可接受

解决方案：
  ├── 异步编译 — 在后台线程编译，不阻塞主线程
  ├── 分层编译 — 先用快速JIT，再用优化JIT
  ├── 预编译（AOT + JIT）— 关键路径AOT，其他JIT
  └── 编译预算 — 限制每次编译的时间
```

### 3. 安全性

```
JIT的安全挑战：
  ├── 代码注入 — JIT生成的可执行内存可能被利用
  ├── W^X违反 — JIT需要同时可写和可执行
  └── 信息泄露 — 编译后的代码可能泄露信息

缓解措施：
  ├── JIT硬ening — 限制可执行内存的范围
  ├── 代码签名 — 验证生成的代码完整性
  └── 随机化 — 随机化代码布局
```

***

## 6. 极简总结

| 概念 | 一句话 |
|------|--------|
| JIT | 运行时编译——结合解释器的快速启动和AOT的高性能 |
| 热点检测 | 发现频繁执行的代码——只优化值得优化的代码 |
| 类型特化 | 根据实际类型生成代码——JIT超越AOT的关键 |
| 去优化 | 优化假设失败时回退——保证正确性 |
| 分层编译 | 冷代码简单JIT，热代码优化JIT——平衡启动和峰值 |
| ORC JIT | LLVM的现代JIT框架——支持懒编译和并发编译 |
| TorchScript | PyTorch的JIT——算子融合消除Python开销 |
| HotSpot C1/C2 | JVM的双层JIT——快速编译+激进优化 |
| V8 TurboFan | Chrome的优化JIT——基于Sea-of-Nodes |

**JIT = 运行时编译 + Profile驱动优化 + 去优化保底，让动态语言也能接近C的性能。**

***

### 相关阅读

- [编译器是如何工作的](./00-编译器是如何工作的.md)
- [什么是LLVM IR](./01-什么是LLVM-IR.md)
- [什么是LLVM Pass](./03-什么是LLVM-Pass.md)
- [什么是DSL领域特定语言](./05-什么是DSL领域特定语言.md)
- [LLVM与Clang](../10-工程实践/26-LLVM与Clang.md)