# LLVM与Clang开发

> 掌握 LLVM/Clang 架构，开发编译器插件与静态分析工具

---

> **LLVM: A Compilation Framework for Lifelong Program Analysis & Transformation.** — Chris Lattner
> （LLVM：面向终身程序分析与变换的编译框架。）

> **LLVM 不仅是编译器，更是一个可编程的编译器基础设施。**

---

> **🎯 授人以鱼不如授人以渔。**
>
> （学会 LLVM 开发，你就拥有了创造编译器工具的能力。）

---

> 💡 **通俗理解 - LLVM/Clang 开发是什么？**

想象 LLVM 是一个"乐高积木"系统：
1. **每个组件都是积木**：前端、优化器、后端可以独立替换
2. **你可以拼装自己的工具**：用这些积木搭建你需要的编译器工具
3. **Clang 是最精美的积木**：提供了完整的 C/C++ 前端

**LLVM/Clang 开发就是利用这些积木：**
- 编写 Clang 插件：在编译时检查代码规范
- 使用 LibTooling：编写独立的代码分析工具
- 开发 Clang-Tidy 检查器：自动化代码审查
- 构建静态分析器：发现潜在的代码缺陷

---

> 🔬 **抽象理解 - LLVM/Clang 开发的本质**：
> - **LLVM**：提供 IR、优化 Pass、代码生成等编译器基础设施
> - **Clang**：提供 AST、语义分析、诊断等 C/C++ 前端能力
> - **插件机制**：在不修改编译器源码的情况下扩展功能
> - **LibTooling**：将 Clang 的分析能力暴露给独立工具

---

## 前置知识
- [编译器优化技术](03-编译器优化技术.md)
## 后续内容
- [代码生成与目标平台](05-代码生成与目标平台.md)

---

## 目录

- [1. LLVM架构与组件](#1-llvm架构与组件)
- [2. Clang架构](#2-clang架构)
- [3. 编写Clang插件](#3-编写clang插件)
- [4. LibTooling开发](#4-libtooling开发)
- [5. Clang-Tidy检查器开发](#5-clang-tidy检查器开发)
- [6. 静态分析器开发](#6-静态分析器开发)
- [7. 本章小结](#7-本章小结)

---

## 1. LLVM架构与组件

### 1.1 LLVM 整体架构

```
┌─────────────────────────────────────────────────────────────┐
│                     LLVM 架构                                │
│                                                             │
│  ┌──────────┐  ┌──────────┐  ┌──────────┐  ┌──────────┐   │
│  │  Clang   │  │  Flang   │  │  Swift   │  │  Rust    │   │
│  │  (C/C++) │  │(Fortran) │  │          │  │          │   │
│  └────┬─────┘  └────┬─────┘  └────┬─────┘  └────┬─────┘   │
│       │              │              │              │         │
│       ▼              ▼              ▼              ▼         │
│  ┌──────────────────────────────────────────────────────┐   │
│  │              LLVM IR (位码 / 文本)                    │   │
│  └──────────────────────┬───────────────────────────────┘   │
│                         │                                   │
│  ┌──────────────────────▼───────────────────────────────┐   │
│  │              LLVM 优化器 (Pass 管理器)                │   │
│  │  ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐ ┌──────┐      │   │
│  │  │内联  │ │GVN   │ │LICM  │ │DCE   │ │向量  │      │   │
│  │  └──────┘ └──────┘ └──────┘ └──────┘ └──────┘      │   │
│  └──────────────────────┬───────────────────────────────┘   │
│                         │                                   │
│       ┌─────────────────┼─────────────────┐                │
│       ▼                 ▼                 ▼                 │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐              │
│  │  X86     │    │  ARM     │    │  RISC-V  │              │
│  │  后端    │    │  后端    │    │  后端    │              │
│  └──────────┘    └──────────┘    └──────────┘              │
└─────────────────────────────────────────────────────────────┘
```

### 1.2 LLVM 核心库

```cpp
// LLVM 核心库的组织

// 1. LLVM Core - IR 和基础设施
#include "llvm/IR/LLVMContext.h"     // LLVM 上下文
#include "llvm/IR/Module.h"          // 模块（编译单元）
#include "llvm/IR/Function.h"        // 函数
#include "llvm/IR/BasicBlock.h"      // 基本块
#include "llvm/IR/IRBuilder.h"       // IR 构建器
#include "llvm/IR/Verifier.h"        // IR 验证器

// 2. LLVM Analysis - 分析 Pass
#include "llvm/Analysis/AliasAnalysis.h"   // 别名分析
#include "llvm/Analysis/LoopInfo.h"        // 循环信息
#include "llvm/Analysis/CallGraph.h"       // 调用图
#include "llvm/Analysis/MemorySSA.h"       // 内存 SSA

// 3. LLVM Transform - 变换 Pass
#include "llvm/Transforms/Utils/Cloning.h"      // 函数克隆
#include "llvm/Transforms/Scalar.h"              // 标量优化
#include "llvm/Transforms/Vectorize.h"           // 向量化
#include "llvm/Transforms/IPO.h"                 // 过程间优化

// 4. LLVM CodeGen - 代码生成
#include "llvm/CodeGen/MachineFunction.h"   // 机器函数
#include "llvm/CodeGen/MachineInstr.h"      // 机器指令
#include "llvm/CodeGen/Passes.h"            // 代码生成 Pass

// 5. LLVM Support - 工具库
#include "llvm/Support/CommandLine.h"    // 命令行解析
#include "llvm/Support/FileSystem.h"     // 文件系统
#include "llvm/Support/raw_ostream.h"    // 输出流
```

### 1.3 LLVM IR 操作实战

```cpp
// 使用 LLVM C++ API 创建 IR
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/Verifier.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

// 创建一个简单的函数并生成 IR
void createSimpleFunction() {
    LLVMContext context;
    Module module("my_module", context);
    IRBuilder<> builder(context);

    // 创建函数类型：int add(int, int)
    FunctionType* funcType = FunctionType::get(
        Type::getInt32Ty(context),
        {Type::getInt32Ty(context), Type::getInt32Ty(context)},
        false
    );

    // 创建函数
    Function* addFunc = Function::Create(
        funcType, Function::ExternalLinkage, "add", module
    );

    // 设置参数名
    Function::arg_iterator args = addFunc->arg_begin();
    Value* a = &*args++; a->setName("a");
    Value* b = &*args++; b->setName("b");

    // 创建基本块
    BasicBlock* entry = BasicBlock::Create(context, "entry", addFunc);
    builder.SetInsertPoint(entry);

    // 生成加法指令
    Value* sum = builder.CreateAdd(a, b, "sum");

    // 生成返回指令
    builder.CreateRet(sum);

    // 验证函数
    verifyFunction(*addFunc);

    // 打印 IR
    module.print(outs(), nullptr);
}

// 创建斐波那契函数
void createFibFunction() {
    LLVMContext context;
    Module module("fib_module", context);
    IRBuilder<> builder(context);

    // 函数类型：int fib(int)
    FunctionType* fibType = FunctionType::get(
        Type::getInt32Ty(context),
        {Type::getInt32Ty(context)},
        false
    );

    Function* fibFunc = Function::Create(
        fibType, Function::ExternalLinkage, "fib", module
    );

    Value* n = &*fibFunc->arg_begin();
    n->setName("n");

    // 入口基本块
    BasicBlock* entry = BasicBlock::Create(context, "entry", fibFunc);
    BasicBlock* thenBB = BasicBlock::Create(context, "then", fibFunc);
    BasicBlock* elseBB = BasicBlock::Create(context, "else", fibFunc);
    BasicBlock* mergeBB = BasicBlock::Create(context, "merge", fibFunc);

    // entry: if (n <= 1) goto then else goto else
    builder.SetInsertPoint(entry);
    Value* cmp = builder.CreateICmpSLE(n, ConstantInt::get(Type::getInt32Ty(context), 1), "cmp");
    builder.CreateCondBr(cmp, thenBB, elseBB);

    // then: return n
    builder.SetInsertPoint(thenBB);
    builder.CreateBr(mergeBB);

    // else: return fib(n-1) + fib(n-2)
    builder.SetInsertPoint(elseBB);
    Value* nMinus1 = builder.CreateSub(n, ConstantInt::get(Type::getInt32Ty(context), 1), "nMinus1");
    Value* nMinus2 = builder.CreateSub(n, ConstantInt::get(Type::getInt32Ty(context), 2), "nMinus2");
    Value* fib1 = builder.CreateCall(fibFunc, {nMinus1}, "fib1");
    Value* fib2 = builder.CreateCall(fibFunc, {nMinus2}, "fib2");
    Value* result = builder.CreateAdd(fib1, fib2, "result");
    builder.CreateBr(mergeBB);

    // merge: phi 节点
    builder.SetInsertPoint(mergeBB);
    PHINode* phi = builder.CreatePHI(Type::getInt32Ty(context), 2, "retval");
    phi->addIncoming(n, thenBB);
    phi->addIncoming(result, elseBB);
    builder.CreateRet(phi);

    verifyFunction(*fibFunc);
    module.print(outs(), nullptr);
}
```

---

## 2. Clang架构

### 2.1 Clang 组件概览

```
┌─────────────────────────────────────────────────────────┐
│                    Clang 架构                            │
│                                                         │
│  ┌─────────────────────────────────────────────────┐   │
│  │              Frontend Actions                    │   │
│  │  - SyntaxOnlyAction (仅语法检查)                 │   │
│  │  - EmitLLVMAction (生成 LLVM IR)                │   │
│  │  - EmitAssemblyAction (生成汇编)                 │   │
│  │  - ASTPrintAction (打印 AST)                    │   │
│  └────────────────────┬────────────────────────────┘   │
│                       │                                 │
│  ┌────────────────────▼────────────────────────────┐   │
│  │              Compiler Instance                   │   │
│  │  - 诊断引擎（错误/警告信息）                     │   │
│  │  - 源码管理器（文件/缓冲区管理）                 │   │
│  │  - 目标信息（平台/ABI 信息）                     │   │
│  │  - 预处理器（宏/条件编译）                       │   │
│  └────────────────────┬────────────────────────────┘   │
│                       │                                 │
│  ┌────────────────────▼────────────────────────────┐   │
│  │              AST（抽象语法树）                    │   │
│  │  - Decl（声明节点）                              │   │
│  │  - Stmt（语句节点）                              │   │
│  │  - Expr（表达式节点）                            │   │
│  │  - Type（类型节点）                              │   │
│  └────────────────────┬────────────────────────────┘   │
│                       │                                 │
│  ┌────────────────────▼────────────────────────────┐   │
│  │              Sema（语义分析）                     │   │
│  │  - 类型检查                                     │   │
│  │  - 名称查找                                     │   │
│  │  - 模板实例化                                   │   │
│  └─────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

### 2.2 Clang AST 节点层次

```cpp
// Clang AST 的核心节点类型

// 声明节点（Decl）
// - TranslationUnitDecl  翻译单元
// - FunctionDecl         函数声明
// - VarDecl              变量声明
// - RecordDecl           结构体/类声明
// - CXXRecordDecl        C++ 类声明
// - ClassTemplateDecl    类模板声明
// - NamespaceDecl        命名空间声明
// - TypedefDecl          类型别名声明

// 语句节点（Stmt）
// - CompoundStmt         复合语句 { ... }
// - IfStmt               if 语句
// - WhileStmt            while 语句
// - ForStmt              for 语句
// - ReturnStmt           return 语句
// - DeclStmt             声明语句

// 表达式节点（Expr，继承自 Stmt）
// - IntegerLiteral       整数字面量
// - FloatingLiteral      浮点字面量
// - StringLiteral        字符串字面量
// - DeclRefExpr          变量引用
// - BinaryOperator       二元运算
// - UnaryOperator        一元运算
// - CallExpr             函数调用
// - MemberExpr           成员访问
// - CXXConstructExpr     C++ 构造函数调用
// - CXXMemberCallExpr    C++ 成员函数调用

// 类型节点（Type）
// - BuiltinType          内建类型（int, float, void）
// - PointerType          指针类型
// - ReferenceType        引用类型
// - ArrayType            数组类型
// - RecordType           结构体/类类型
// - TemplateTypeParmType 模板类型参数
```

### 2.3 使用 Clang 查看 AST

```bash
# 查看 Clang AST
clang -Xclang -ast-dump -fsyntax-only source.cpp

# 示例输出（对于 int add(int a, int b) { return a + b; }）
TranslationUnitDecl
`-FunctionDecl <line:1:1, line:1:31> add 'int (int, int)'
  |-ParmVarDecl <col:9, col:13> a 'int'
  |-ParmVarDecl <col:16, col:20> b 'int'
  `-CompoundStmt <col:23, col:31>
    `-ReturnStmt <col:25, col:30>
      `-BinaryOperator <col:32, col:36> 'int' '+'
        |-DeclRefExpr <col:32> 'int' lvalue ParmVar 'a' 'int'
        `-DeclRefExpr <col:36> 'int' lvalue ParmVar 'b' 'int'

# 只查看声明
clang -Xclang -ast-dump -fsyntax-only -ast-dump-filter="add" source.cpp

# 以 JSON 格式输出 AST
clang -Xclang -ast-dump=json -fsyntax-only source.cpp
```

---

## 3. 编写Clang插件

### 3.1 Clang 插件架构

```
Clang 插件的工作方式：
1. 编译为动态库（.so / .dll）
2. 通过 -fplugin 加载到编译器中
3. 注册 AST 遍历回调
4. 在编译过程中自动执行

┌──────────────┐    加载插件     ┌──────────────┐
│   Clang      │◄──────────────│  Plugin.so   │
│   编译器     │               │              │
│              │───回调────────▶│ ASTConsumer  │
│              │               │ RecursiveVis │
└──────────────┘               └──────────────┘
```

### 3.2 AST 遍历插件

```cpp
// Clang 插件：遍历 AST 并统计代码信息
// 编译：clang++ -shared -fPIC -o Plugin.so Plugin.cpp \
//        $(llvm-config --cxxflags --ldflags)

#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Basic/SourceManager.h"

using namespace clang;

// AST 访问者：遍历 AST 节点
class CodeStatsVisitor : public RecursiveASTVisitor<CodeStatsVisitor> {
private:
    ASTContext& context;
    int functionCount = 0;
    int classCount = 0;
    int loopCount = 0;
    int callCount = 0;

public:
    CodeStatsVisitor(ASTContext& ctx) : context(ctx) {}

    // 访问函数声明
    bool VisitFunctionDecl(FunctionDecl* func) {
        functionCount++;
        // 打印函数签名
        FullSourceLoc loc = context.getFullLoc(func->getBeginLoc());
        if (loc.isValid()) {
            llvm::outs() << "函数: " << func->getQualifiedNameAsString()
                        << " (行 " << loc.getSpellingLineNumber() << ")\n";
        }
        return true;
    }

    // 访问 C++ 类声明
    bool VisitCXXRecordDecl(CXXRecordDecl* record) {
        if (record->isClass() || record->isStruct()) {
            classCount++;
            llvm::outs() << "类/结构体: " << record->getQualifiedNameAsString() << "\n";
        }
        return true;
    }

    // 访问 for 循环
    bool VisitForStmt(ForStmt* forStmt) {
        loopCount++;
        return true;
    }

    // 访问 while 循环
    bool VisitWhileStmt(WhileStmt* whileStmt) {
        loopCount++;
        return true;
    }

    // 访问函数调用
    bool VisitCallExpr(CallExpr* call) {
        callCount++;
        if (auto* callee = call->getDirectCallee()) {
            // 检查是否调用了危险函数
            std::string name = callee->getNameAsString();
            if (name == "strcpy" || name == "gets" || name == "sprintf") {
                FullSourceLoc loc = context.getFullLoc(call->getBeginLoc());
                DiagnosticsEngine& diag = context.getDiagnostics();
                unsigned id = diag.getCustomDiagID(
                    DiagnosticsEngine::Warning,
                    "使用了不安全的函数 '%0'，建议使用更安全的替代方案"
                );
                diag.Report(loc, id) << name;
            }
        }
        return true;
    }

    // 打印统计结果
    void printStats() {
        llvm::outs() << "\n=== 代码统计 ===\n";
        llvm::outs() << "函数数量: " << functionCount << "\n";
        llvm::outs() << "类/结构体数量: " << classCount << "\n";
        llvm::outs() << "循环数量: " << loopCount << "\n";
        llvm::outs() << "函数调用数量: " << callCount << "\n";
    }
};

// AST 消费者：创建访问者并驱动遍历
class CodeStatsConsumer : public ASTConsumer {
private:
    CodeStatsVisitor visitor;

public:
    CodeStatsConsumer(ASTContext& ctx) : visitor(ctx) {}

    void HandleTranslationUnit(ASTContext& ctx) override {
        // 遍历整个翻译单元的 AST
        visitor.TraverseDecl(ctx.getTranslationUnitDecl());
        visitor.printStats();
    }
};

// 插件动作：创建 AST 消费者
class CodeStatsAction : public PluginASTAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(
        CompilerInstance& ci, llvm::StringRef) override {
        return std::make_unique<CodeStatsConsumer>(ci.getASTContext());
    }

    bool ParseArgs(const CompilerInstance& ci,
                  const std::vector<std::string>& args) override {
        // 解析插件参数
        for (const auto& arg : args) {
            if (arg == "verbose") {
                // 开启详细模式
            }
        }
        return true;
    }

    ActionType getActionType() override {
        return AddBeforeMainAction;
    }
};

// 注册插件
static FrontendPluginRegistry::Add<CodeStatsAction>
X("code-stats", "统计代码信息并检测不安全函数");
```

### 3.3 自定义警告插件

```cpp
// 自定义警告：检测未检查的返回值
class UncheckedReturnVisitor : public RecursiveASTVisitor<UncheckedReturnVisitor> {
private:
    ASTContext& context;

public:
    UncheckedReturnVisitor(ASTContext& ctx) : context(ctx) {}

    bool VisitCallExpr(CallExpr* call) {
        // 检查函数是否返回非 void 类型
        QualType returnType = call->getCallReturnType(context);
        if (returnType->isVoidType()) return true;

        // 检查返回值是否被使用
        auto* parent = context.getParents(*call).begin()->get();
        if (!parent) return true;

        // 如果父节点是 CompoundStmt 或 ExprStmt，说明返回值被忽略
        if (isa<CompoundStmt>(parent) ||
            (isa<CXXOperatorCallExpr>(parent) && !isa<DeclRefExpr>(parent))) {
            // 发出警告
            DiagnosticsEngine& diag = context.getDiagnostics();
            unsigned id = diag.getCustomDiagID(
                DiagnosticsEngine::Warning,
                "函数 '%0' 的返回值未被检查"
            );
            diag.Report(context.getFullLoc(call->getBeginLoc()), id)
                << call->getDirectCallee()->getNameAsString();
        }

        return true;
    }
};

// 检测裸 new/delete 的插件（推荐使用智能指针）
class RawNewDeleteVisitor : public RecursiveASTVisitor<RawNewDeleteVisitor> {
private:
    ASTContext& context;

public:
    RawNewDeleteVisitor(ASTContext& ctx) : context(ctx) {}

    bool VisitCXXNewExpr(CXXNewExpr* expr) {
        DiagnosticsEngine& diag = context.getDiagnostics();
        unsigned id = diag.getCustomDiagID(
            DiagnosticsEngine::Warning,
            "使用了裸 new 表达式，建议使用 std::make_unique 或 std::make_shared"
        );
        diag.Report(context.getFullLoc(expr->getBeginLoc()), id);
        return true;
    }

    bool VisitCXXDeleteExpr(CXXDeleteExpr* expr) {
        DiagnosticsEngine& diag = context.getDiagnostics();
        unsigned id = diag.getCustomDiagID(
            DiagnosticsEngine::Warning,
            "使用了裸 delete 表达式，建议使用智能指针管理内存"
        );
        diag.Report(context.getFullLoc(expr->getBeginLoc()), id);
        return true;
    }
};
```

### 3.4 使用 Clang 插件

```bash
# 编译插件
clang++ -shared -fPIC -o MyPlugin.so MyPlugin.cpp \
    $(llvm-config --cxxflags --ldflags) \
    -lclangFrontend -lclangAST -lclangBasic

# 在编译时使用插件
clang -fplugin=./MyPlugin.so -Xclang -plugin-arg-my-plugin -Xclang verbose \
    source.cpp

# 仅运行插件（不编译）
clang -fsyntax-only -fplugin=./MyPlugin.so source.cpp
```

---

## 4. LibTooling开发

### 4.1 LibTooling 概述

LibTooling 是 Clang 提供的独立工具开发库，可以编写独立的代码分析/变换工具：

```
LibTooling vs Clang 插件：

Clang 插件：
- 在编译过程中运行
- 需要修改编译命令
- 适合 CI/CD 集成

LibTooling：
- 独立的可执行程序
- 可以使用 Clang 的完整 AST
- 适合开发重构工具
- 支持 Clang 的编译数据库（compile_commands.json）
```

### 4.2 编写 LibTooling 工具

```cpp
// LibTooling 工具：查找所有 TODO 注释
// 编译：clang++ -o find-todo FindTodo.cpp $(llvm-config --cxxflags --ldflags) \
//        -lclangTooling -lclangFrontend -lclangAST -lclangBasic -lclangLex

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Lex/Preprocessor.h"
#include "clang/Lex/Lexer.h"
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::tooling;

// 命令行选项
static llvm::cl::OptionCategory TodoCategory("find-todo options");
static llvm::cl::extrahelp CommonHelp(CommonOptionsParser::HelpMessage);

// 注释查找器
class TodoCommentVisitor : public RecursiveASTVisitor<TodoCommentVisitor> {
private:
    ASTContext& context;
    SourceManager& sourceMgr;

public:
    TodoCommentVisitor(ASTContext& ctx)
        : context(ctx), sourceMgr(ctx.getSourceManager()) {}

    // 遍历所有注释
    void findTodoComments() {
        // 获取预处理器中的注释
        const auto& commentList = context.getRawCommentList();
        for (const auto* comment : commentList.getComments()) {
            std::string text = comment->getRawText(sourceMgr).str();
            // 检查是否包含 TODO
            if (text.find("TODO") != std::string::npos ||
                text.find("FIXME") != std::string::npos ||
                text.find("HACK") != std::string::npos) {
                FullSourceLoc loc(comment->getBeginLoc(), sourceMgr);
                llvm::outs() << sourceMgr.getFilename(loc)
                            << ":" << loc.getSpellingLineNumber()
                            << ": " << text << "\n";
            }
        }
    }
};

// AST 消费者
class TodoCommentConsumer : public ASTConsumer {
private:
    TodoCommentVisitor visitor;

public:
    TodoCommentConsumer(ASTContext& ctx) : visitor(ctx) {}

    void HandleTranslationUnit(ASTContext& ctx) override {
        visitor.findTodoComments();
    }
};

// Frontend Action
class TodoCommentAction : public ASTFrontendAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(
        CompilerInstance& ci, llvm::StringRef) override {
        return std::make_unique<TodoCommentConsumer>(ci.getASTContext());
    }
};

// 主函数
int main(int argc, const char** argv) {
    auto expectedParser = CommonOptionsParser::create(argc, argv, TodoCategory);
    if (!expectedParser) {
        llvm::errs() << expectedParser.takeError();
        return 1;
    }

    CommonOptionsParser& optionsParser = expectedParser.get();
    ClangTool tool(optionsParser.getCompilations(),
                  optionsParser.getSourcePathList());

    return tool.run(newFrontendActionFactory<TodoCommentAction>().get());
}
```

### 4.3 代码重构工具

```cpp
// LibTooling 代码重构工具：将 C 风格转型替换为 C++ 风格转型
// (int)x → static_cast<int>(x)

#include "clang/Tooling/Refactoring.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Rewrite/Core/Rewriter.h"

class CastReplacementVisitor : public RecursiveASTVisitor<CastReplacementVisitor> {
private:
    ASTContext& context;
    Rewriter& rewriter;

public:
    CastReplacementVisitor(ASTContext& ctx, Rewriter& r)
        : context(ctx), rewriter(r) {}

    bool VisitCStyleCastExpr(CStyleCastExpr* castExpr) {
        // 获取源代码位置
        SourceLocation begin = castExpr->getBeginLoc();
        SourceLocation end = castExpr->getEndLoc();

        // 获取目标类型
        std::string targetType = castExpr->getTypeAsWritten().getAsString();

        // 获取子表达式
        Expr* subExpr = castExpr->getSubExpr();
        std::string subExprStr = getText(subExpr);

        // 构建替换文本
        std::string replacement = "static_cast<" + targetType + ">(" +
                                 subExprStr + ")";

        // 执行替换
        rewriter.ReplaceText(
            SourceRange(begin, end),
            replacement
        );

        return true;
    }

private:
    std::string getText(Expr* expr) {
        SourceManager& sm = context.getSourceManager();
        SourceLocation begin = expr->getBeginLoc();
        SourceLocation end = expr->getEndLoc();
        return std::string(sm.getCharacterData(begin),
                          sm.getCharacterData(end) - sm.getCharacterData(begin));
    }
};
```

---

## 5. Clang-Tidy检查器开发

### 5.1 Clang-Tidy 架构

```
Clang-Tidy 是基于 Clang 的代码检查工具：

┌──────────────────────────────────────────────┐
│              Clang-Tidy 架构                 │
│                                              │
│  ┌──────────────────────────────────────┐   │
│  │         检查器注册表                  │   │
│  │  - bugprone-*  (易错模式)            │   │
│  │  - modernize-* (现代化建议)          │   │
│  │  - readability-* (可读性)            │   │
│  │  - performance-* (性能)              │   │
│  │  - custom-* (自定义)                 │   │
│  └──────────────────────────────────────┘   │
│                                              │
│  ┌──────────────────────────────────────┐   │
│  │         ClangTidyCheck 基类          │   │
│  │  - registerMatchers()                │   │
│  │  - check()                           │   │
│  │  - storeOptions()                    │   │
│  └──────────────────────────────────────┘   │
│                                              │
│  ┌──────────────────────────────────────┐   │
│  │         AST Matcher                  │   │
│  │  - 声明式 AST 模式匹配              │   │
│  │  - 简洁的 DSL                        │   │
│  └──────────────────────────────────────┘   │
└──────────────────────────────────────────────┘
```

### 5.2 编写自定义 Clang-Tidy 检查器

```cpp
// 自定义 Clang-Tidy 检查器：检测未使用的包含文件
// 文件位置：clang-tidy/plugin/UnusedIncludeCheck.cpp

#include "clang/AST/ASTContext.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/ClangTidy/ClangTidyCheck.h"
#include "clang/ClangTidy/ClangTidyModule.h"
#include "clang/ClangTidy/ClangTidyModuleRegistry.h"
#include "clang/Lex/Preprocessor.h"

using namespace clang::ast_matchers;

namespace clang::tidy::custom {

// 检查器类
class UnusedIncludeCheck : public ClangTidyCheck {
public:
    UnusedIncludeCheck(StringRef name, ClangTidyContext* context)
        : ClangTidyCheck(name, context) {}

    // 注册 AST 匹配器
    void registerMatchers(ast_matchers::MatchFinder* finder) override {
        // 匹配所有 #include 指令引入的声明使用情况
        // 这里简化处理，匹配所有声明引用
        finder->addMatcher(
            declRefExpr(to(namedDecl())).bind("ref"),
            this
        );
    }

    // 检查匹配到的节点
    void check(const ast_matchers::MatchFinder::MatchResult& result) override {
        const auto* ref = result.Nodes.getNodeAs<DeclRefExpr>("ref");
        if (!ref) return;

        // 记录被引用的声明所在的头文件
        if (const auto* decl = ref->getDecl()) {
            SourceManager& sm = *result.SourceManager;
            FileID fileID = sm.getFileID(decl->getLocation());
            usedHeaders_.insert(fileID);
        }
    }

    // 编译结束时检查未使用的头文件
    void onEndOfTranslationUnit() override {
        // 对比所有包含的头文件和实际使用的头文件
        // 报告未使用的包含
    }

private:
    std::set<FileID> usedHeaders_;
};

// 检测魔法数字的检查器
class MagicNumberCheck : public ClangTidyCheck {
public:
    MagicNumberCheck(StringRef name, ClangTidyContext* context)
        : ClangTidyCheck(name, context),
          ignoredValues({"0", "1", "2", "-1", "nullptr", "NULL"}) {}

    void registerMatchers(ast_matchers::MatchFinder* finder) override {
        // 匹配整数和浮点字面量（排除 0、1 等常见值）
        finder->addMatcher(
            integerLiteral(unless(hasParent(
                enumConstantDecl()  // 排除枚举常量
            ))).bind("int_lit"),
            this
        );

        finder->addMatcher(
            floatLiteral().bind("float_lit"),
            this
        );
    }

    void check(const ast_matchers::MatchFinder::MatchResult& result) override {
        if (const auto* intLit = result.Nodes.getNodeAs<IntegerLiteral>("int_lit")) {
            checkLiteral(intLit, result);
        }
        if (const auto* floatLit = result.Nodes.getNodeAs<FloatingLiteral>("float_lit")) {
            checkLiteral(floatLit, result);
        }
    }

private:
    std::set<std::string> ignoredValues;

    template<typename LiteralType>
    void checkLiteral(const LiteralType* lit,
                     const ast_matchers::MatchFinder::MatchResult& result) {
        // 获取字面量的文本表示
        SourceManager& sm = *result.SourceManager;
        std::string text = Lexer::getSourceText(
            CharSourceRange::getTokenRange(lit->getSourceRange()),
            sm, result.Context->getLangOpts()
        ).str();

        // 跳过常见值
        if (ignoredValues.count(text)) return;

        // 跳过 const/constexpr 定义
        if (isInConstDeclaration(lit, result)) return;

        // 发出诊断
        diag(lit->getBeginLoc(),
             "魔法数字 '%0'，建议使用命名常量")
            << text;
    }

    template<typename LiteralType>
    bool isInConstDeclaration(const LiteralType* lit,
                             const ast_matchers::MatchFinder::MatchResult& result) {
        // 检查字面量是否在 const/constexpr 声明中
        // ...（简化处理）
        return false;
    }
};

// 注册检查器到模块
class CustomModule : public ClangTidyModule {
public:
    void addCheckFactories(ClangTidyCheckFactories& factories) override {
        factories.registerCheck<UnusedIncludeCheck>("custom-unused-include");
        factories.registerCheck<MagicNumberCheck>("custom-magic-number");
    }
};

// 注册模块
static ClangTidyModuleRegistry::Add<CustomModule>
X("custom-module", "自定义 Clang-Tidy 检查器");
```

### 5.3 使用 AST Matcher

```cpp
// AST Matcher 是 Clang 提供的声明式 AST 模式匹配 DSL

// 常用 Matcher 示例：

// 1. 匹配所有函数声明
functionDecl()

// 2. 匹配名为 "foo" 的函数
functionDecl(hasName("foo"))

// 3. 匹配参数超过 5 个的函数
functionDecl(parameterCountIs(greaterThan(5)))

// 4. 匹配所有 for 循环
forStmt()

// 5. 匹配使用原始指针的变量声明
varDecl(hasType(pointerType()))

// 6. 匹配裸 new 表达式
cxxNewExpr()

// 7. 匹配 C 风格转型
cStyleCastExpr()

// 8. 匹配调用了特定函数的调用表达式
callExpr(callee(functionDecl(hasName("strcpy"))))

// 9. 匹配在类中的公共成员函数
cxxMethodDecl(isPublic())

// 10. 匹配使用了 auto 的变量声明
varDecl(hasType(autoType()))

// 组合 Matcher 示例：
// 匹配返回裸指针的函数
functionDecl(returns(pointerType()))

// 匹配在 for 循环中使用下标的表达式
arraySubscriptExpr(hasAncestor(forStmt()))
```

---

## 6. 静态分析器开发

### 6.1 Clang Static Analyzer 架构

```
Clang Static Analyzer（CSA）是基于符号执行的静态分析框架：

┌──────────────────────────────────────────────────┐
│           Clang Static Analyzer 架构              │
│                                                  │
│  ┌──────────┐    ┌──────────┐    ┌──────────┐  │
│  │ 源代码    │───▶│ AST      │───▶│ CFG      │  │
│  │          │    │          │    │ 控制流图  │  │
│  └──────────┘    └──────────┘    └────┬─────┘  │
│                                       │         │
│  ┌────────────────────────────────────▼──────┐  │
│  │          符号执行引擎                      │  │
│  │  - ExplodedGraph（展开图）                │  │
│  │  - ProgramState（程序状态）               │  │
│  │  - SVal（符号值）                         │  │
│  │  - ConstraintManager（约束管理器）        │  │
│  └────────────────────┬──────────────────────┘  │
│                       │                         │
│  ┌────────────────────▼──────────────────────┐  │
│  │          检查器（Checker）                 │  │
│  │  - checkPreCall                          │  │
│  │  - checkPostCall                         │  │
│  │  - checkBind                             │  │
│  │  - checkDeadSymbols                      │  │
│  │  - checkEndFunction                      │  │
│  └───────────────────────────────────────────┘  │
└──────────────────────────────────────────────────┘
```

### 6.2 编写自定义静态分析检查器

```cpp
// 自定义静态分析检查器：检测未检查的空指针解引用

#include "clang/StaticAnalyzer/Core/Checker.h"
#include "clang/StaticAnalyzer/Core/PathSensitive/CheckerContext.h"
#include "clang/StaticAnalyzer/Core/BugReporter/BugType.h"
#include "clang/StaticAnalyzer/Frontend/CheckerRegistry.h"

using namespace clang;
using namespace ento;

class NullDereferenceChecker : public Checker<check::PreCall, check::Location> {
private:
    std::unique_ptr<BugType> bugType;

public:
    NullDereferenceChecker()
        : bugType(std::make_unique<BugType>(
              this, "空指针解引用", "安全性错误")) {}

    // 检查函数调用前的参数
    void checkPreCall(const CallEvent& call, CheckerContext& ctx) const {
        // 检查参数是否可能为空
        for (unsigned i = 0; i < call.getNumArgs(); i++) {
            SVal argVal = call.getArgSVal(i);
            if (isPossiblyNull(argVal, ctx)) {
                // 检查参数是否标记为 nonnull
                if (isFunctionParamNonNull(call, i)) {
                    reportBug(ctx, call.getArgExpr(i),
                             "传递可能为空的指针给 nonnull 参数");
                }
            }
        }
    }

    // 检查内存访问（解引用）
    void checkLocation(SVal location, bool isLoad, const Stmt* stmt,
                      CheckerContext& ctx) const {
        if (isPossiblyNull(location, ctx)) {
            reportBug(ctx, stmt, "解引用可能为空的指针");
        }
    }

private:
    // 检查值是否可能为空
    bool isPossiblyNull(SVal val, CheckerContext& ctx) const {
        // 获取约束条件
        ProgramStateRef state = ctx.getState();
        ConditionTruthVal nullness = state->isNull(val);

        // 如果确定不为空，返回 false
        if (nullness.isConstrainedTrue() && !nullness.getValue()) {
            return false;
        }

        // 如果确定为空或可能为空，返回 true
        return true;
    }

    // 检查函数参数是否标记为 nonnull
    bool isFunctionParamNonNull(const CallEvent& call, unsigned argIndex) const {
        if (const auto* funcDecl = call.getDecl()) {
            if (funcDecl->hasAttr<NonNullAttr>()) {
                return true;
            }
        }
        return false;
    }

    // 报告 Bug
    void reportBug(CheckerContext& ctx, const Stmt* stmt,
                  const std::string& message) const {
        // 生成错误路径
        ExplodedNode* errorNode = ctx.generateErrorNode();
        if (!errorNode) return;

        // 创建 Bug 报告
        auto report = std::make_unique<PathSensitiveBugReport>(
            *bugType, message, errorNode);
        report->addRange(stmt->getSourceRange());

        // 发出报告
        ctx.emitReport(std::move(report));
    }
};

// 注册检查器
extern "C" void clang_registerCheckers(CheckerRegistry& registry) {
    registry.addChecker<NullDereferenceChecker>(
        "custom.NullDereference",
        "检测未检查的空指针解引用"
    );
}

extern "C" const char clang_analyzerAPIVersionString[] =
    CLANG_ANALYZER_API_VERSION_STRING;
```

### 6.3 运行静态分析

```bash
# 使用 Clang Static Analyzer
# 方法1：使用 scan-build 工具
scan-build make

# 方法2：使用 clang --analyze
clang --analyze -Xanalyzer -analyzer-checker=custom.NullDereference source.cpp

# 方法3：使用 CodeChecker（更友好的前端）
codechecker check --build "make" --output ./reports
codechecker parse ./reports

# 启用/禁用特定检查器
clang --analyze \
    -Xanalyzer -analyzer-checker=core \
    -Xanalyzer -analyzer-checker=unix \
    -Xanalyzer -analyzer-disable-checker=unix.Malloc \
    source.cpp
```

---

## 7. 本章小结

### 核心要点回顾

| 技术 | 关键内容 |
|------|---------|
| LLVM架构 | 三段式设计：前端→IR→后端，核心库包括IR、分析、变换、代码生成 |
| Clang架构 | 前端动作→编译器实例→AST→语义分析，完整的C/C++前端 |
| Clang插件 | ASTConsumer + RecursiveASTVisitor，在编译时运行 |
| LibTooling | 独立工具开发，支持编译数据库，适合重构工具 |
| Clang-Tidy | 基于AST Matcher的声明式检查器开发 |
| 静态分析器 | 基于符号执行，ProgramState + ExplodedGraph |

### 关键理解

1. **LLVM 的核心价值在于可组合性**：IR 是通用枢纽，前端和后端可独立发展
2. **Clang 的 AST 是最丰富的 C/C++ 分析基础**：比正则表达式更精确，比 LLVM IR 更高层
3. **插件 vs LibTooling vs Clang-Tidy**：根据需求选择合适的开发方式
4. **AST Matcher 大幅简化了检查器开发**：声明式匹配比命令式遍历更简洁

### 延伸思考

- 如何将 Clang 插件集成到 CI/CD 流水线中？
- Clang-Tidy 的自动修复功能（-fix）是如何实现的？
- 如何开发一个跨项目的代码风格检查器？

> **下一章**：[代码生成与目标平台](05-代码生成与目标平台.md) — 深入编译器后端，学习代码生成的核心技术
