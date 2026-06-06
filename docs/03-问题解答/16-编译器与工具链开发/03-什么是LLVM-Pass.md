# 什么是LLVM Pass
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)、[编译器原理](../../10-编译器与工具链开发/00-编译器原理概述.md)

> "核心要义：LLVM Pass就是对IR（中间表示）进行一次遍历和变换的过程——分析Pass收集信息，变换Pass改写代码，多个Pass按顺序组合就构成了编译器的优化管线，而你可以编写自己的Pass来扩展编译器的能力。"

***

## 1. Pass的管理与调度

### 1. 什么是Pass

```
Pass = 对LLVM IR进行一次遍历和处理的单元

类比：
  IR就像一条流水线上的产品
  Pass就是流水线上的工位
  每个工位对产品做一种处理
  多个工位按顺序组合 = 完整的优化管线

Pass的分类：
  ├── 分析Pass（Analysis Pass）— 只读，收集信息
  │   └── 例如：活跃变量分析、依赖分析、调用图分析
  │   └── 结果可被其他Pass使用
  │
  └── 变换Pass（Transform Pass）— 读写，改写IR
      └── 例如：常量传播、死代码消除、内联
      └── 可能使用分析Pass的结果
      └── 可能使之前的分析结果失效
```

### 2. Pass的粒度

```
Pass可以作用于不同的粒度：

Module Pass    — 遍历整个模块（一个翻译单元）
  └── 用途：跨函数优化（内联、全局常量传播）
  └── 开销最大，但优化机会最多

Function Pass  — 遍历模块中的每个函数
  └── 用途：函数内优化（常量折叠、死代码消除）
  └── 最常用的粒度

Loop Pass      — 遍历函数中的每个循环
  └── 用途：循环优化（循环展开、向量化）
  └── 针对循环结构的专门优化

CGSCC Pass     — 遍历调用图中的强连通分量
  └── 用途：过程间优化（递归函数优化）
  └── 较少使用
```

### 3. Pass的调度

```
Pass Manager负责调度Pass的执行：

┌──────────────────────────────────────────────────┐
│                  Pass Manager                     │
│                                                  │
│  ModulePassManager                               │
│  ├── ModuleAnalysisManager                       │
│  │   ├── 调用图分析                               │
│  │   └── 全局依赖分析                             │
│  ├── FunctionPassManager (for each function)     │
│  │   ├── FunctionAnalysisManager                 │
│  │   │   ├── 支配树分析                           │
│  │   │   ├── 活跃变量分析                         │
│  │   │   └── 循环信息分析                         │
│  │   ├── LoopPassManager (for each loop)         │
│  │   │   └── 循环相关Pass                         │
│  │   └── 函数级变换Pass                           │
│  └── 模块级变换Pass                               │
└──────────────────────────────────────────────────┘

调度策略：
  ├── 分析Pass按需运行（有消费者时才运行）
  ├── 分析结果缓存（除非被变换Pass失效）
  ├── 变换Pass按注册顺序运行
  └── Pass Manager自动管理依赖关系
```

***

## 2. 分析Pass与变换Pass

### 1. 分析Pass

```cpp
// 分析Pass示例：统计函数中的指令数量
// 分析Pass不修改IR，只收集信息

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassPlugin.h"

using namespace llvm;

// 定义分析结果类型
struct InstructionCountInfo {
    int totalInstructions = 0;
    int arithmeticInstructions = 0;
    int memoryInstructions = 0;
    int controlFlowInstructions = 0;
};

// 分析Pass
class InstructionCountAnalysis : public AnalysisInfoMixin<InstructionCountAnalysis> {
public:
    using Result = InstructionCountInfo;

    Result run(Function& F, FunctionAnalysisManager& AM) {
        Result info;
        for (const BasicBlock& BB : F) {
            for (const Instruction& I : BB) {
                info.totalInstructions++;
                if (isa<BinaryOperator>(I)) {
                    info.arithmeticInstructions++;
                } else if (isa<LoadInst>(I) || isa<StoreInst>(I)) {
                    info.memoryInstructions++;
                } else if (isa<BranchInst>(I) || isa<ReturnInst>(I)) {
                    info.controlFlowInstructions++;
                }
            }
        }
        return info;
    }

    static AnalysisKey Key;  // 分析Pass的唯一标识
};

AnalysisKey InstructionCountAnalysis::Key;
```

### 2. 变换Pass

```cpp
// 变换Pass示例：常量折叠
// 变换Pass修改IR，进行优化

#include "llvm/IR/Function.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

class SimpleConstantFolding : public PassInfoMixin<SimpleConstantFolding> {
public:
    PreservedAnalyses run(Function& F, FunctionAnalysisManager& AM) {
        bool changed = false;

        for (BasicBlock& BB : F) {
            for (Instruction& I : make_early_inc_range(BB)) {
                // 只处理二元运算
                auto* binOp = dyn_cast<BinaryOperator>(&I);
                if (!binOp) continue;

                // 检查两个操作数是否都是常量
                ConstantInt* lhs = dyn_cast<ConstantInt>(binOp->getOperand(0));
                ConstantInt* rhs = dyn_cast<ConstantInt>(binOp->getOperand(1));
                if (!lhs || !rhs) continue;

                // 在编译期计算结果
                APInt result;
                switch (binOp->getOpcode()) {
                    case Instruction::Add:
                        result = lhs->getValue() + rhs->getValue();
                        break;
                    case Instruction::Sub:
                        result = lhs->getValue() - rhs->getValue();
                        break;
                    case Instruction::Mul:
                        result = lhs->getValue() * rhs->getValue();
                        break;
                    default:
                        continue;
                }

                // 用常量替换运算指令
                ConstantInt* resultConst = ConstantInt::get(lhs->getType(), result);
                binOp->replaceAllUsesWith(resultConst);
                binOp->eraseFromParent();
                changed = true;
            }
        }

        // 如果修改了IR，需要告知哪些分析结果仍然有效
        return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
    }

    static bool isRequired() { return true; }
};
```

### 3. 分析Pass与变换Pass的关系

```
变换Pass使用分析Pass的结果：

  ┌─────────────┐     结果     ┌─────────────┐
  │  分析Pass    │ ──────────→ │  变换Pass    │
  │ (只读)      │             │ (读写)       │
  └─────────────┘             └─────────────┘

  例如：
  活跃变量分析 → 死代码消除（删除不活跃的变量）
  支配树分析   → 支配边界计算 → 插入phi节点
  循环信息分析 → 循环展开/向量化

变换Pass可能使分析结果失效：

  变换Pass修改IR后，之前缓存的分析结果可能不再正确
  └── PreservedAnalyses::none()  — 所有分析结果失效
  └── PreservedAnalyses::all()   — 所有分析结果仍然有效
  └── PreservedAnalyses::preserve<XXX>()  — 特定分析仍然有效
```

***

## 3. Pass的依赖关系

### 1. 声明依赖

```cpp
// 变换Pass声明对分析Pass的依赖

class MyOptimizationPass : public PassInfoMixin<MyOptimizationPass> {
public:
    PreservedAnalyses run(Function& F, FunctionAnalysisManager& AM) {
        // 获取支配树分析的结果
        auto& domTree = AM.getResult<DominatorTreeAnalysis>(F);

        // 获取循环信息
        auto& loopInfo = AM.getResult<LoopAnalysis>(F);

        // 使用分析结果进行优化
        for (const Loop* L : loopInfo) {
            // 检查循环是否可以展开
            if (canUnroll(L, domTree)) {
                unrollLoop(L);
            }
        }

        // 保留不需要失效的分析
        PreservedAnalyses PA;
        PA.preserve<DominatorTreeAnalysis>();
        PA.preserve<LoopAnalysis>();
        return PA;
    }
};
```

### 2. 依赖图

```
Pass之间的依赖关系形成有向无环图（DAG）：

  调用图分析 ──────→ 内联Pass
       │
       ▼
  函数分析 ──────→ 常量传播Pass
       │                │
       ▼                ▼
  支配树分析 ──────→ 死代码消除Pass
       │
       ▼
  活跃变量分析 ──→ 寄存器分配

Pass Manager自动处理依赖：
  ├── 自动运行被依赖的分析Pass
  ├── 自动缓存分析结果
  └── 变换Pass失效时自动重新运行分析Pass
```

***

## 4. 如何编写自定义Pass

### 1. 完整的自定义Pass示例

```cpp
// 文件：HelloPass.cpp
// 功能：统计函数信息并打印

#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Instructions.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// 自定义Pass：函数信息统计
struct FunctionStatsPass : public PassInfoMixin<FunctionStatsPass> {
    PreservedAnalyses run(Function& F, FunctionAnalysisManager& AM) {
        // 跳过声明（没有函数体的函数）
        if (F.isDeclaration()) return PreservedAnalyses::all();

        int bbCount = 0;
        int instCount = 0;
        int callCount = 0;

        for (const BasicBlock& BB : F) {
            bbCount++;
            for (const Instruction& I : BB) {
                instCount++;
                if (isa<CallInst>(I) || isa<InvokeInst>(I)) {
                    callCount++;
                }
            }
        }

        errs() << "=== 函数统计: " << F.getName() << " ===\n";
        errs() << "  基本块数量: " << bbCount << "\n";
        errs() << "  指令数量:   " << instCount << "\n";
        errs() << "  函数调用数: " << callCount << "\n";
        errs() << "  参数数量:   " << F.arg_size() << "\n";

        return PreservedAnalyses::all();  // 不修改IR
    }

    static bool isRequired() { return true; }
};

// 注册Pass插件
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {
        LLVM_PLUGIN_API_VERSION,
        "FunctionStats",       // 插件名称
        LLVM_VERSION_STRING,   // LLVM版本
        [](PassBuilder& PB) {
            // 注册到优化管线的不同位置

            // 方式1：注册到epilogue（优化管线末尾）
            PB.registerPipelineParsingCallback(
                [](StringRef Name, FunctionPassManager& FPM,
                   ArrayRef<PassBuilder::PipelineElement>) {
                    if (Name == "function-stats") {
                        FPM.addPass(FunctionStatsPass());
                        return true;
                    }
                    return false;
                });

            // 方式2：注册到优化管线的特定阶段
            PB.registerVectorizerStartEPCallback(
                [](FunctionPassManager& FPM, OptimizationLevel Level) {
                    FPM.addPass(FunctionStatsPass());
                });
        }
    };
}
```

### 2. 编译和使用Pass插件

```bash
# 编译Pass插件
clang++ -shared -fPIC HelloPass.cpp -o HelloPass.so \
    $(llvm-config --cxxflags --ldflags --libs core)

# 使用Pass插件
opt -load-pass-plugin ./HelloPass.so \
    -passes="function-stats" \
    -S input.ll -o output.ll

# 在clang中使用Pass插件
clang -fpass-plugin=./HelloPass.so -O2 hello.cpp -o hello
```

### 3. 更实用的Pass：死代码消除

```cpp
// 简易死代码消除Pass
// 删除结果未被使用的指令

struct SimpleDCEPass : public PassInfoMixin<SimpleDCEPass> {
    PreservedAnalyses run(Function& F, FunctionAnalysisManager& AM) {
        bool changed = false;
        SmallVector<Instruction*, 16> deadInstructions;

        // 收集死代码
        for (BasicBlock& BB : F) {
            for (Instruction& I : BB) {
                // 跳过有副作用的指令
                if (I.mayHaveSideEffects()) continue;
                // 跳过终止指令
                if (I.isTerminator()) continue;
                // 检查结果是否被使用
                if (I.use_empty()) {
                    deadInstructions.push_back(&I);
                }
            }
        }

        // 删除死代码
        for (Instruction* I : deadInstructions) {
            I->eraseFromParent();
            changed = true;
        }

        return changed ? PreservedAnalyses::none() : PreservedAnalyses::all();
    }

    static bool isRequired() { return true; }
};
```

### 4. CMakeLists.txt配置

```cmake
# 构建LLVM Pass插件的CMake配置

cmake_minimum_required(VERSION 3.16)
project(MyLLVMPass)

# 查找LLVM
find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# 编译Pass插件
add_library(MyPass MODULE
    HelloPass.cpp
    SimpleDCEPass.cpp
)

target_compile_features(MyPass PRIVATE cxx_std_17)
set_target_properties(MyPass PROPERTIES
    PREFIX ""
    SUFFIX ".so"
)

# 链接LLVM库
target_link_libraries(MyPass PRIVATE
    LLVMCore
    LLVMSupport
    LLVMPasses
)
```

***

## 5. 新Pass Manager

### 1. 旧Pass Manager vs 新Pass Manager

```
旧Pass Manager（Legacy PM）：
  ├── 从LLVM 1.0就存在
  ├── 基于继承和虚函数
  ├── Pass之间通过字符串名称依赖
  ├── 手动管理分析结果失效
  └── 已被弃用（LLVM 14+默认使用新PM）

新Pass Manager（New PM）：
  ├── LLVM 13引入，LLVM 14成为默认
  ├── 基于值语义和CRTP
  ├── Pass之间通过类型系统依赖
  ├── 自动管理分析结果缓存和失效
  └── 更好的组合性和可扩展性
```

### 2. 新旧Pass Manager对比

| 维度 | 旧Pass Manager | 新Pass Manager |
|------|---------------|---------------|
| Pass基类 | 继承FunctionPass/ModulePass | CRTP: PassInfoMixin<T> |
| 分析结果 | getAnalysis<T>() | AM.getResult<T>(F) |
| 注册方式 | RegisterPass<T> | PassBuilder回调 |
| 结果缓存 | 手动管理 | 自动管理 |
| 依赖声明 | 声明getAnalysisUsage | 通过AM.getResult自动 |
| PreservedAnalyses | 无 | 显式返回 |
| 线程安全 | 否 | 是 |
| 组合性 | 有限 | 优秀 |

### 3. 旧Pass Manager的Pass（已弃用，仅供参考）

```cpp
// 旧Pass Manager的写法（不推荐新代码使用）

struct LegacyHelloPass : public FunctionPass {
    static char ID;
    LegacyHelloPass() : FunctionPass(ID) {}

    bool runOnFunction(Function& F) override {
        errs() << "函数: " << F.getName() << "\n";
        return false;  // 未修改IR
    }

    // 声明依赖
    void getAnalysisUsage(AnalysisUsage& AU) const override {
        AU.addRequired<DominatorTreeWrapperPass>();
        AU.setPreservesAll();  // 不修改IR
    }
};

char LegacyHelloPass::ID = 0;
static RegisterPass<LegacyHelloPass> X("hello", "Hello Pass", false, false);
```

### 4. 新Pass Manager的Pass（推荐）

```cpp
// 新Pass Manager的写法（推荐）

struct NewHelloPass : public PassInfoMixin<NewHelloPass> {
    PreservedAnalyses run(Function& F, FunctionAnalysisManager& AM) {
        errs() << "函数: " << F.getName() << "\n";

        // 使用分析结果
        auto& domTree = AM.getResult<DominatorTreeAnalysis>(F);
        // ... 使用domTree ...

        return PreservedAnalyses::all();
    }

    static bool isRequired() { return true; }
};

// 注册方式
extern "C" LLVM_ATTRIBUTE_WEAK PassPluginLibraryInfo llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "NewHello", LLVM_VERSION_STRING,
            [](PassBuilder& PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager& FPM,
                       ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "new-hello") {
                            FPM.addPass(NewHelloPass());
                            return true;
                        }
                        return false;
                    });
            }};
}
```

### 5. Pass管线组合

```bash
# 新Pass Manager的管线语法

# 运行单个Pass
opt -passes=instcombine input.ll -S -o output.ll

# 运行多个Pass（按顺序）
opt -passes="instcombine,gvn,dce" input.ll -S -o output.ll

# 函数级Pass + 模块级Pass
opt -passes="inline,instcombine" input.ll -S -o output.ll

# 循环级Pass
opt -passes="loop-unroll" input.ll -S -o output.ll

# 使用优化等级（等价于预定义的Pass管线）
opt -O2 input.ll -S -o output.ll

# 查看O2包含的所有Pass
opt -O2 -debug-pass-manager -S input.ll -o /dev/null 2>&1 | grep "Running pass"
```

***

## 6. 常用内置Pass一览

```
变换Pass（Transform）：
├── instcombine     — 指令合并（最强大的Peephole优化）
├── gvn             — 全局值编号（消除冗余计算）
├── dce             — 死代码消除
├── adce            — 积极死代码消除
├── simplifycfg     — 控制流简化
├── sroa            — 标量替换聚合体（消除alloca）
├── inline          — 函数内联
├── loop-unroll     — 循环展开
├── loop-vectorize  — 循环向量化
├── slp-vectorize   — SLP向量化（超字长级并行）
├── licm            — 循环不变量外提
├── tailcallelim    — 尾调用消除
├── reassociate     — 表达式重关联
├── cfgsimplify     — 控制流图简化
└── mergefunc       — 函数合并

分析Pass（Analysis）：
├── dominatortree      — 支配树
├── loops              — 循环信息
├── callgraph          — 调用图
├── liveness           — 活跃变量
├── scalar-evolution   — 标量演化分析
├── target-transform   — 目标平台信息
├── alias-analysis     — 别名分析
├── memory-ssa         — 内存SSA
└── assumption-cache   — 假设缓存
```

***

## 7. 极简总结

| 概念 | 一句话 |
|------|--------|
| Pass | 对IR的一次遍历处理——编译器优化的基本单元 |
| 分析Pass | 只读遍历——收集信息供其他Pass使用 |
| 变换Pass | 读写遍历——改写IR进行优化 |
| Pass Manager | Pass调度器——管理依赖、缓存、执行顺序 |
| 新Pass Manager | 基于CRTP的现代框架——类型安全、自动管理 |
| PreservedAnalyses | 分析结果有效性——告知哪些分析仍然有效 |
| Pass插件 | 动态加载的Pass——扩展编译器无需重新编译 |

**LLVM Pass = 分析Pass收集信息 + 变换Pass改写IR + PassManager管理调度，新Pass Manager通过类型系统自动管理依赖和缓存。**

***

### 相关阅读

- [编译器是如何工作的](./00-编译器是如何工作的.md)
- [什么是LLVM IR](./01-什么是LLVM-IR.md)
- [什么是SSA静态单赋值](./07-什么是SSA静态单赋值.md)
- [什么是寄存器分配](./06-什么是寄存器分配.md)
- [LLVM与Clang](../10-工程实践/26-LLVM与Clang.md)