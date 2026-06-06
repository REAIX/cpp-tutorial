# 如何编写Clang插件
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)、[编译器原理](../../10-编译器与工具链开发/00-编译器原理概述.md)

> "要义概览：Clang插件通过遍历AST（抽象语法树）来检查代码模式，你只需继承ASTVisitor或MatchCallback，在回调中编写检查逻辑，就能实现自定义警告、代码规范检查、自动重构等能力——本质就是'在编译时对语法树做模式匹配'。"

***

## 1. Clang AST结构

### 1. AST是什么

```
AST（Abstract Syntax Tree）= 抽象语法树

源代码：
  int add(int a, int b) {
      return a + b;
  }

对应的AST（简化）：
  TranslationUnitDecl
  └── FunctionDecl "add"
      ├── ParmVarDecl "a" (int)
      ├── ParmVarDecl "b" (int)
      ├── QualType: int (返回类型)
      └── CompoundStmt
          └── ReturnStmt
              └── BinaryOperator "+"
                  ├── DeclRefExpr "a"
                  └── DeclRefExpr "b"

AST的关键特性：
  ├── 保留了完整的语义信息（类型、作用域、声明等）
  ├── 是编译器理解和处理代码的核心数据结构
  ├── Clang的AST比其他编译器更完整（可以原样还原源代码）
  └── 所有Clang工具（clangd/clang-tidy/LibTooling）都基于AST
```

### 2. AST节点类型层次

```
AST节点的主要类型：

Decl（声明）
├── TranslationUnitDecl  — 翻译单元（一个源文件）
├── FunctionDecl         — 函数声明
├── VarDecl              — 变量声明
├── CXXRecordDecl        — C++类/结构体声明
├── EnumDecl             — 枚举声明
├── TypedefDecl          — 类型别名
├── NamespaceDecl        — 命名空间
├── FieldDecl            — 类成员变量
├── CXXMethodDecl        — 类方法
└── TemplateDecl         — 模板声明

Stmt（语句）
├── CompoundStmt         — 复合语句 { ... }
├── IfStmt               — if语句
├── ForStmt              — for循环
├── WhileStmt            — while循环
├── ReturnStmt           — return语句
├── DeclStmt             — 声明语句
├── ExprStmt             — 表达式语句
└── SwitchStmt           — switch语句

Expr（表达式，继承自Stmt）
├── IntegerLiteral       — 整数字面量
├── FloatingLiteral      — 浮点字面量
├── StringLiteral        — 字符串字面量
├── DeclRefExpr          — 变量/函数引用
├── MemberExpr           — 成员访问（a.b / a->b）
├── BinaryOperator       — 二元运算（+ - * / == &&等）
├── UnaryOperator        — 一元运算（! - ++ --等）
├── CallExpr             — 函数调用
├── CXXMemberCallExpr    — 成员函数调用
├── CXXConstructExpr     — 构造函数调用
├── CastExpr             — 类型转换
└── CXXNewExpr           — new表达式

Type（类型）
├── BuiltinType          — 内置类型（int, float等）
├── PointerType          — 指针类型
├── ReferenceType        — 引用类型
├── ArrayType            — 数组类型
├── RecordType           — 类/结构体类型
├── TemplateTypeParmType — 模板类型参数
└── AutoType             — auto类型
```

### 3. 查看AST

```bash
# 打印完整的AST
clang -Xclang -ast-dump -fsyntax-only hello.cpp

# 只打印声明
clang -Xclang -ast-dump -fsyntax-only -ast-dump-filter="add" hello.cpp

# 以JSON格式输出AST
clang -Xclang -ast-dump=json -fsyntax-only hello.cpp

# 查看特定函数的AST
clang -Xclang -ast-dump -fsyntax-only -ast-dump-filter="main" hello.cpp
```

**AST输出示例**：

```
TranslationUnitDecl
|-TypedefDecl implicit __int128_t
|-TypedefDecl implicit __uint128_t
|-FunctionDecl main 'int ()'
| `-CompoundStmt
|   |-DeclStmt
|   | `-VarDecl x 'int' cinit
|   |   `-IntegerLiteral 'int' 42
|   `-ReturnStmt
|     `-IntegerLiteral 'int' 0
```

***

## 2. AST遍历

### 1. RecursiveASTVisitor

```cpp
// RecursiveASTVisitor是最常用的AST遍历框架
// 它会递归访问AST中的所有节点

#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::tooling;

// 自定义AST访问器
class MyASTVisitor : public RecursiveASTVisitor<MyASTVisitor> {
private:
    ASTContext* context;

public:
    explicit MyASTVisitor(ASTContext* ctx) : context(ctx) {}

    // 访问所有函数声明
    bool VisitFunctionDecl(FunctionDecl* func) {
        llvm::outs() << "发现函数: " << func->getNameAsString() << "\n";
        llvm::outs() << "  返回类型: " << func->getReturnType().getAsString() << "\n";
        llvm::outs() << "  参数数量: " << func->getNumParams() << "\n";

        // 检查是否是C++方法
        if (auto* method = dyn_cast<CXXMethodDecl>(func)) {
            llvm::outs() << "  是类方法: " << (method->isVirtual() ? "虚函数" : "非虚函数") << "\n";
        }
        return true;  // 继续遍历
    }

    // 访问所有变量声明
    bool VisitVarDecl(VarDecl* var) {
        llvm::outs() << "发现变量: " << var->getNameAsString() << "\n";
        llvm::outs() << "  类型: " << var->getType().getAsString() << "\n";

        // 检查是否是局部变量
        if (var->isLocalVarDecl()) {
            llvm::outs() << "  位置: 局部变量\n";
        }
        return true;
    }

    // 访问所有二元运算表达式
    bool VisitBinaryOperator(BinaryOperator* binop) {
        llvm::outs() << "发现二元运算: " << binop->getOpcodeStr() << "\n";
        return true;
    }

    // 访问所有函数调用
    bool VisitCallExpr(CallExpr* call) {
        FunctionDecl* callee = call->getDirectCallee();
        if (callee) {
            llvm::outs() << "发现函数调用: " << callee->getNameAsString() << "\n";
        }
        return true;
    }
};
```

### 2. ASTFrontendAction框架

```cpp
// ASTConsumer — 消费AST的接口
class MyASTConsumer : public ASTConsumer {
private:
    MyASTVisitor visitor;

public:
    explicit MyASTConsumer(ASTContext* ctx) : visitor(ctx) {}

    // 每个翻译单元解析完成后调用
    void HandleTranslationUnit(ASTContext& ctx) override {
        visitor.TraverseDecl(ctx.getTranslationUnitDecl());
    }
};

// FrontendAction — 前端动作的基类
class MyFrontendAction : public ASTFrontendAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(
        CompilerInstance& CI, StringRef file) override {
        return std::make_unique<MyASTConsumer>(&CI.getASTContext());
    }
};

// 主函数
int main(int argc, const char** argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv,
        llvm::cl::OptionCategory("my-tool"));
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    ClangTool Tool(ExpectedParser->getCompilations(),
                   ExpectedParser->getSourcePathList());
    return Tool.run(newFrontendActionFactory<MyFrontendAction>().get());
}
```

***

## 3. 自定义警告

### 1. 使用DiagEngine报告警告

```cpp
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/Diagnostic.h"

class UnsafeCastVisitor : public RecursiveASTVisitor<UnsafeCastVisitor> {
private:
    ASTContext* context;

public:
    explicit UnsafeCastVisitor(ASTContext* ctx) : context(ctx) {}

    // 检测C风格强制转换（不安全的类型转换）
    bool VisitCStyleCastExpr(CStyleCastExpr* cast) {
        QualType srcType = cast->getSubExpr()->getType();
        QualType destType = cast->getType();

        // 检查是否从指针转为整数
        if (srcType->isPointerType() && destType->isIntegerType()) {
            reportWarning(cast, "将指针转换为整数可能不安全，建议使用reinterpret_cast");
        }

        // 检查是否从const转为非const
        if (srcType.isConstQualified() && !destType.isConstQualified()) {
            reportWarning(cast, "丢弃const限定符可能导致未定义行为");
        }

        return true;
    }

    // 检测未使用的返回值
    bool VisitCallExpr(CallExpr* call) {
        FunctionDecl* callee = call->getDirectCallee();
        if (!callee) return true;

        // 检查是否是[[nodiscard]]函数
        if (callee->hasAttr<WarnUnusedResultAttr>()) {
            // 检查返回值是否被使用
            if (!call->getParent()) {
                reportWarning(call, "忽略了nodiscard函数的返回值");
            }
        }
        return true;
    }

private:
    void reportWarning(SourceLocation loc, const std::string& msg) {
        DiagnosticsEngine& diag = context->getDiagnostics();
        unsigned diagID = diag.getCustomDiagID(
            DiagnosticsEngine::Warning, "%0");
        DiagnosticBuilder builder = diag.Report(loc, diagID);
        builder << msg;
    }
};
```

### 2. 自定义Clang插件

```cpp
// Clang插件——在编译时自动运行
// 文件：UnsafeCastPlugin.cpp

#include "clang/AST/AST.h"
#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "clang/Sema/Sema.h"

using namespace clang;

namespace {

class UnsafeCastPluginVisitor : public RecursiveASTVisitor<UnsafeCastPluginVisitor> {
private:
    ASTContext* context;

public:
    explicit UnsafeCastPluginVisitor(ASTContext* ctx) : context(ctx) {}

    bool VisitCStyleCastExpr(CStyleCastExpr* cast) {
        QualType srcType = cast->getSubExpr()->getType();
        QualType destType = cast->getType();

        if (srcType->isPointerType() && destType->isIntegerType()) {
            DiagnosticsEngine& diag = context->getDiagnostics();
            unsigned id = diag.getCustomDiagID(
                DiagnosticsEngine::Warning,
                "不安全的指针到整数转换，建议使用uintptr_t或reinterpret_cast");
            diag.Report(cast->getExprLoc(), id);
        }
        return true;
    }
};

class UnsafeCastPluginConsumer : public ASTConsumer {
private:
    UnsafeCastPluginVisitor visitor;

public:
    explicit UnsafeCastPluginConsumer(ASTContext* ctx) : visitor(ctx) {}

    void HandleTranslationUnit(ASTContext& ctx) override {
        visitor.TraverseDecl(ctx.getTranslationUnitDecl());
    }
};

class UnsafeCastPluginAction : public PluginASTAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(
        CompilerInstance& CI, StringRef file) override {
        return std::make_unique<UnsafeCastPluginConsumer>(&CI.getASTContext());
    }

    bool ParseArgs(const CompilerInstance& CI,
                   const std::vector<std::string>& args) override {
        // 解析插件参数（如果有）
        return true;
    }
};

} // anonymous namespace

// 注册插件
static FrontendPluginRegistry::Add<UnsafeCastPluginAction>
X("unsafe-cast-plugin", "检测不安全的C风格类型转换");
```

```bash
# 编译插件
clang++ -shared -fPIC UnsafeCastPlugin.cpp -o UnsafeCastPlugin.so \
    $(llvm-config --cxxflags --ldflags --libs core)

# 使用插件
clang -Xclang -load -Xclang ./UnsafeCastPlugin.so \
      -Xclang -add-plugin -Xclang unsafe-cast-plugin \
      hello.cpp
```

***

## 4. LibTooling开发

### 1. LibTooling简介

```
LibTooling = 基于Clang的独立工具开发库

与Clang插件的区别：
  ┌────────────┬──────────────────┬──────────────────┐
  │ 维度        │ Clang插件         │ LibTooling       │
  ├────────────┼──────────────────┼──────────────────┤
  │ 运行方式    │ 随编译器运行       │ 独立程序运行      │
  │ 输出       │ 诊断信息          │ 任意输出          │
  │ 修改代码   │ 不方便            │ 支持（重构）      │
  │ 灵活性    │ 受限              │ 完全自由          │
  │ 适用场景   │ 编译时检查        │ 代码分析/重构工具  │
  └────────────┴──────────────────┴──────────────────┘
```

### 2. LibTooling工具开发示例

```cpp
// 统计代码中的函数调用次数
// 文件：CallCounter.cpp

#include "clang/AST/AST.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <string>

using namespace clang;
using namespace clang::tooling;

namespace {

class CallCounterVisitor : public RecursiveASTVisitor<CallCounterVisitor> {
private:
    ASTContext* context;
    std::map<std::string, int>& callCounts;  // 函数名 → 调用次数

public:
    CallCounterVisitor(ASTContext* ctx, std::map<std::string, int>& counts)
        : context(ctx), callCounts(counts) {}

    bool VisitCallExpr(CallExpr* call) {
        FunctionDecl* callee = call->getDirectCallee();
        if (callee) {
            std::string name = callee->getQualifiedNameAsString();
            callCounts[name]++;

            // 打印调用位置
            SourceLocation loc = call->getExprLoc();
            SourceManager& sm = context->getSourceManager();
            llvm::outs() << sm.getFilename(loc) << ":"
                         << sm.getSpellingLineNumber(loc) << " "
                         << "调用 " << name << "\n";
        }
        return true;
    }
};

class CallCounterConsumer : public ASTConsumer {
private:
    std::map<std::string, int> callCounts;
    CallCounterVisitor visitor;

public:
    explicit CallCounterConsumer(ASTContext* ctx)
        : visitor(ctx, callCounts) {}

    void HandleTranslationUnit(ASTContext& ctx) override {
        visitor.TraverseDecl(ctx.getTranslationUnitDecl());

        // 打印统计结果
        llvm::outs() << "\n=== 函数调用统计 ===\n";
        for (const auto& [name, count] : callCounts) {
            llvm::outs() << name << ": " << count << " 次\n";
        }
    }
};

class CallCounterAction : public ASTFrontendAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(
        CompilerInstance& CI, StringRef file) override {
        return std::make_unique<CallCounterConsumer>(&CI.getASTContext());
    }
};

} // anonymous namespace

// 命令行选项
static llvm::cl::OptionCategory MyToolCategory("call-counter");

int main(int argc, const char** argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    ClangTool Tool(ExpectedParser->getCompilations(),
                   ExpectedParser->getSourcePathList());
    return Tool.run(newFrontendActionFactory<CallCounterAction>().get());
}
```

```bash
# 编译
clang++ CallCounter.cpp -o call-counter \
    $(llvm-config --cxxflags --ldflags --libs core tooling) \
    -lclangFrontend -lclangTooling -lclangAST -lclangBasic

# 使用
./call-counter hello.cpp -- -std=c++17

# 配合compile_commands.json使用（推荐）
./call-counter -p build/ src/hello.cpp
```

### 3. ASTMatcher——声明式AST匹配

```cpp
// ASTMatcher提供声明式的AST匹配语法
// 比手写RecursiveASTVisitor更简洁

#include "clang/ASTMatchers/ASTMatchers.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"

using namespace clang;
using namespace clang::ast_matchers;

// 1. 定义匹配模式
// 匹配所有使用C风格转换的表达式
StatementMatcher CStyleCastMatcher =
    cStyleCastExpr().bind("cast");

// 匹配所有调用printf的地方
StatementMatcher PrintfCallMatcher =
    callExpr(callee(functionDecl(hasName("printf")))).bind("printf_call");

// 匹配所有标记了[[nodiscard]]但返回值被忽略的调用
StatementMatcher DiscardedNodiscardMatcher =
    callExpr(callee(functionDecl(hasAttr(clang::attr::WarnUnusedResult))))
    .bind("discarded_nodiscard");

// 匹配所有使用原始指针new的表达式
StatementMatcher RawNewMatcher =
    cxxNewExpr(unless(hasParent(cxxConstructExpr(
        hasType(hasDeclaration(classTemplateSpecializationDecl(
            hasName("std::unique_ptr")))))))).bind("raw_new");

// 2. 定义回调处理
class CastCallback : public MatchFinder::MatchCallback {
public:
    void run(const MatchFinder::MatchResult& result) override {
        if (const auto* cast = result.Nodes.getNodeAs<CStyleCastExpr>("cast")) {
            llvm::outs() << "发现C风格转换: "
                         << cast->getExprLoc().printToString(
                                *result.SourceManager)
                         << "\n";
        }
    }
};

class PrintfCallback : public MatchFinder::MatchCallback {
public:
    void run(const MatchFinder::MatchResult& result) override {
        if (const auto* call = result.Nodes.getNodeAs<CallExpr>("printf_call")) {
            llvm::outs() << "发现printf调用: "
                         << call->getExprLoc().printToString(
                                *result.SourceManager)
                         << "\n";
            llvm::outs() << "  建议使用std::cout或std::format代替\n";
        }
    }
};

// 3. 组合使用
int main(int argc, const char** argv) {
    // ... CommonOptionsParser初始化 ...

    MatchFinder finder;
    CastCallback castCallback;
    PrintfCallback printfCallback;

    finder.addMatcher(CStyleCastMatcher, &castCallback);
    finder.addMatcher(PrintfCallMatcher, &printfCallback);

    // ClangTool Tool(...);
    // Tool.run(newFrontendActionFactory(&finder).get());
}
```

### 4. 常用ASTMatcher模式

```cpp
// 匹配所有类/结构体
declarationMatcher = cxxRecordDecl().bind("class");

// 匹配特定名称的类
declarationMatcher = cxxRecordDecl(hasName("MyClass")).bind("myclass");

// 匹配所有虚函数
declarationMatcher = cxxMethodDecl(isVirtual()).bind("virtual_method");

// 匹配所有public成员变量
declarationMatcher = fieldDecl(isPublic()).bind("public_field");

// 匹配所有使用auto的地方
statementMatcher = declStmt(has(varDecl(hasType(autoType())))).bind("auto_var");

// 匹配所有catch(...)异常处理
statementMatcher = cxxCatchStmt(has(varDecl())).bind("typed_catch");

// 匹配所有使用裸指针new的地方（非智能指针）
statementMatcher = cxxNewExpr(
    unless(hasParent(cxxConstructExpr()))
).bind("raw_new");

// 匹配所有全局变量
declarationMatcher = varDecl(hasGlobalStorage(), unless(hasStaticStorageDuration())).bind("global_var");

// 匹配所有使用memcpy的地方
statementMatcher = callExpr(callee(functionDecl(hasName("memcpy")))).bind("memcpy_call");
```

***

## 5. Clang-Tidy检查器开发流程

### 1. Clang-Tidy架构

```
clang-tidy的架构：
  clang-tidy
  ├── CheckManager — 管理所有检查器
  ├── ClangTidyCheck — 检查器基类
  │   ├── registerMatchers() — 注册AST匹配模式
  │   └── check() — 处理匹配结果
  ├── ClangTidyDiagnosticConsumer — 诊断输出
  └── FixItHint — 自动修复建议

检查器分类：
  ├── bugprone-*      — 常见Bug模式
  ├── modernize-*     — 现代化建议
  ├── readability-*   — 可读性改进
  ├── performance-*   — 性能优化
  ├── cppcoreguidelines-* — C++核心准则
  ├── cert-*          — CERT安全准则
  ├── google-*        — Google编码规范
  ├── llvm-*          — LLVM编码规范
  └── custom-*        — 自定义检查器
```

### 2. 开发自定义Clang-Tidy检查器

```cpp
// 文件：UnsafeStrcpyCheck.h
// 检测不安全的strcpy使用

#ifndef LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_UNSAFE_STRCPY_CHECK_H
#define LLVM_CLANG_TOOLS_EXTRA_CLANG_TIDY_UNSAFE_STRCPY_CHECK_H

#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "ClangTidyCheck.h"

namespace clang::tidy::custom {

class UnsafeStrcpyCheck : public ClangTidyCheck {
public:
    UnsafeStrcpyCheck(StringRef Name, ClangTidyContext* Context)
        : ClangTidyCheck(Name, Context) {}

    // 注册AST匹配模式
    void registerMatchers(ast_matchers::MatchFinder* Finder) override {
        using namespace ast_matchers;
        // 匹配所有strcpy调用
        Finder->addMatcher(
            callExpr(callee(functionDecl(hasName("strcpy"))))
                .bind("strcpy_call"),
            this);
    }

    // 处理匹配结果
    void check(const ast_matchers::MatchFinder::MatchResult& Result) override {
        const auto* Call = Result.Nodes.getNodeAs<CallExpr>("strcpy_call");
        if (!Call) return;

        // 报告问题
        diag(Call->getBeginLoc(),
             "使用strcpy不安全，可能导致缓冲区溢出，建议使用strncpy或strlcpy")
            // 提供自动修复建议
            << FixItHint::CreateReplacement(
                Call->getCallee()->getSourceRange(), "strncpy");
    }
};

} // namespace clang::tidy::custom

#endif
```

```cpp
// 文件：UnsafeStrcpyCheck.cpp — 注册检查器

#include "UnsafeStrcpyCheck.h"
#include "clang/AST/ASTContext.h"

namespace clang::tidy::custom {

// 注册到clang-tidy
void registerChecks(ClangTidyCheckFactories& CheckFactories) {
    CheckFactories.registerCheck<UnsafeStrcpyCheck>(
        "custom-unsafe-strcpy");
}

} // namespace clang::tidy::custom
```

### 3. 添加检查器到Clang-Tidy的完整流程

```
步骤1：创建检查器文件
  clang-tools-extra/clang-tidy/custom/
  ├── UnsafeStrcpyCheck.h
  └── UnsafeStrcpyCheck.cpp

步骤2：注册检查器
  在 clang-tidy/custom/CustomTidyModule.cpp 中添加注册代码

步骤3：添加CMake构建
  在 clang-tools-extra/clang-tidy/custom/CMakeLists.txt 中添加源文件

步骤4：编译
  cmake --build build --target clang-tidy

步骤5：测试
  clang-tidy -checks='custom-unsafe-strcpy' hello.cpp

步骤6：添加文档
  docs/clang-tidy/checks/custom/unsafe-strcpy.rst
```

### 4. 使用Clang-Tidy

```bash
# 列出所有可用的检查器
clang-tidy -list-checks

# 列出特定类别的检查器
clang-tidy -list-checks -checks='bugprone-*'
clang-tidy -list-checks -checks='modernize-*'

# 运行特定检查器
clang-tidy -checks='modernize-use-auto,modernize-use-nullptr' hello.cpp

# 运行所有检查器
clang-tidy -checks='*' hello.cpp

# 自动修复
clang-tidy -checks='modernize-use-auto' -fix hello.cpp

# 使用.clang-tidy配置文件
# 在项目根目录创建 .clang-tidy 文件：
# Checks: 'modernize-*,bugprone-*,-modernize-use-trailing-return-type'
# HeaderFilterRegex: '.*'
```

**.clang-tidy配置文件示例**：

```yaml
Checks: >
  -*,
  bugprone-*,
  modernize-*,
  -modernize-use-trailing-return-type,
  performance-*,
  readability-*,
  -readability-magic-numbers
HeaderFilterRegex: 'src/.*'
WarningsAsErrors: ''
FormatStyle: file
CheckOptions:
  - key: readability-identifier-naming.ClassCase
    value: CamelCase
  - key: readability-identifier-naming.FunctionCase
    value: camelBack
  - key: readability-identifier-naming.VariableCase
    value: lower_case
  - key: modernize-use-override.AllowOverrideAndFinal
    value: true
```

***

## 6. 实战：完整的代码规范检查工具

```cpp
// 检查器：禁止使用C风格数组和原始指针new
// 文件：ModernCppCheck.cpp

#include "clang/AST/AST.h"
#include "clang/ASTMatchers/ASTMatchFinder.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendAction.h"
#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Tooling.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace clang::ast_matchers;
using namespace clang::tooling;

namespace {

// 检查1：禁止C风格数组，建议使用std::array或std::vector
class CArrayCallback : public MatchFinder::MatchCallback {
public:
    void run(const MatchFinder::MatchResult& result) override {
        const auto* var = result.Nodes.getNodeAs<VarDecl>("c_array");
        if (!var) return;

        // 排除函数参数中的数组（它们实际上是指针）
        if (isa<ParmVarDecl>(var)) return;

        DiagnosticsEngine& diag = result.Context->getDiagnostics();
        unsigned id = diag.getCustomDiagID(
            DiagnosticsEngine::Warning,
            "禁止使用C风格数组'%0'，建议使用std::array或std::vector");
        DiagnosticBuilder builder = diag.Report(var->getLocation(), id);
        builder << var->getNameAsString();
    }
};

// 检查2：禁止原始指针new，建议使用智能指针
class RawNewCallback : public MatchFinder::MatchCallback {
public:
    void run(const MatchFinder::MatchResult& result) override {
        const auto* newExpr = result.Nodes.getNodeAs<CXXNewExpr>("raw_new");
        if (!newExpr) return;

        DiagnosticsEngine& diag = result.Context->getDiagnostics();
        unsigned id = diag.getCustomDiagID(
            DiagnosticsEngine::Warning,
            "禁止使用原始指针new，建议使用std::make_unique或std::make_shared");
        diag.Report(newExpr->getBeginLoc(), id);
    }
};

// 检查3：禁止C风格转换，建议使用C++风格转换
class CStyleCastCallback : public MatchFinder::MatchCallback {
public:
    void run(const MatchFinder::MatchResult& result) override {
        const auto* cast = result.Nodes.getNodeAs<CStyleCastExpr>("c_cast");
        if (!cast) return;

        QualType destType = cast->getType();
        std::string suggestion;

        if (destType->isPointerType() || destType->isReferenceType()) {
            suggestion = "reinterpret_cast";
        } else {
            suggestion = "static_cast";
        }

        DiagnosticsEngine& diag = result.Context->getDiagnostics();
        unsigned id = diag.getCustomDiagID(
            DiagnosticsEngine::Warning,
            "禁止使用C风格转换，建议使用%0");
        DiagnosticBuilder builder = diag.Report(cast->getLParenLoc(), id);
        builder << suggestion;
    }
};

class ModernCppCheckConsumer : public ASTConsumer {
private:
    MatchFinder finder;
    CArrayCallback cArrayCallback;
    RawNewCallback rawNewCallback;
    CStyleCastCallback cStyleCastCallback;

public:
    ModernCppCheckConsumer() {
        // 匹配C风格数组
        finder.addMatcher(
            varDecl(hasType(arrayType())).bind("c_array"),
            &cArrayCallback);

        // 匹配原始指针new
        finder.addMatcher(
            cxxNewExpr().bind("raw_new"),
            &rawNewCallback);

        // 匹配C风格转换
        finder.addMatcher(
            cStyleCastExpr().bind("c_cast"),
            &cStyleCastCallback);
    }

    void HandleTranslationUnit(ASTContext& ctx) override {
        finder.matchAST(ctx);
    }
};

class ModernCppCheckAction : public ASTFrontendAction {
public:
    std::unique_ptr<ASTConsumer> CreateASTConsumer(
        CompilerInstance& CI, StringRef file) override {
        return std::make_unique<ModernCppCheckConsumer>();
    }
};

} // anonymous namespace

static llvm::cl::OptionCategory MyToolCategory("modern-cpp-check");

int main(int argc, const char** argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, MyToolCategory);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    ClangTool Tool(ExpectedParser->getCompilations(),
                   ExpectedParser->getSourcePathList());
    return Tool.run(newFrontendActionFactory<ModernCppCheckAction>().get());
}
```

***

## 7. 极简总结

| 概念 | 一句话 |
|------|--------|
| Clang AST | 抽象语法树——源代码的结构化表示 |
| RecursiveASTVisitor | 递归遍历AST——访问每个节点 |
| ASTMatcher | 声明式匹配——用DSL描述匹配模式 |
| Clang插件 | 编译时运行——随编译器自动检查 |
| LibTooling | 独立工具——灵活的代码分析和重构 |
| Clang-Tidy | 检查器框架——标准化代码规范检查 |
| DiagnosticsEngine | 诊断引擎——报告警告和错误 |
| FixItHint | 自动修复——提供代码修改建议 |

**编写Clang插件 = 定义AST匹配模式 + 编写回调处理 + 报告诊断信息，核心是理解AST结构和匹配器语法。**

***

### 相关阅读

- [编译器是如何工作的](./00-编译器是如何工作的.md)
- [什么是LLVM IR](./01-什么是LLVM-IR.md)
- [如何开发静态分析工具](./08-如何开发静态分析工具.md)
- [LLVM与Clang](../10-工程实践/26-LLVM与Clang.md)