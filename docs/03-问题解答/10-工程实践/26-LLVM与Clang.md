# LLVM与Clang — 编译器基础设施与现代C++编译器
> 📖 相关章节：[编码规范](../../04-工程实践/00-编码规范.md)、[设计模式](../../04-工程实践/03-设计模式.md)、[单元测试](../../04-工程实践/06-单元测试.md)、[代码审查](../../04-工程实践/08-代码审查.md)

> "LLVM不是编译器，Clang不是LLVM的别名。" — 理解它们的关系，才能理解现代编译器架构。

---

## 1. LLVM是什么 — 编译器的"乐高积木"

### 1. 一句话定义

**LLVM** 是一个编译器基础设施项目，提供了一整套可复用的编译器组件（中间表示、优化器、代码生成器等），让开发者可以像搭乐高一样构建自己的编译器或语言工具。

### 2. 通俗比喻

```
传统编译器（如GCC）：
  源代码 → [一整块黑箱] → 机器码
  └── 所有功能耦合在一起，无法单独使用某个部分

LLVM：
  源代码 → [前端] → [中间表示IR] → [优化器] → [后端] → 机器码
  └── 每个部分都是独立的模块，可以单独使用和替换
  └── 像乐高积木——你可以只用优化器、只用代码生成器、或自己写前端

更精确的比喻：
  LLVM = 发动机工厂
    └── 不生产整车，但提供各种发动机零件
    └── 你可以用这些零件组装出自己的发动机

  Clang = 用LLVM零件组装出的一台发动机
    └── 专门处理C/C++/Objective-C
```

### 3. LLVM名字的由来

```
LLVM名称的真相：
  LLVM ≠ Low Level Virtual Machine（虽然最初是这个缩写）
  现在 LLVM 就是 LLVM，不再是一个缩写
  官方说法："LLVM is the name of the project, not an acronym"

历史：
  2000年：Vikram Adve和Chris Lattner在伊利诺伊大学开始研究
  2003年：Chris Lattner发表博士论文
  2005年：Chris Lattner加入Apple，Apple成为LLVM最大支持者
  2010年：Apple将GCC替换为Clang+LLVM作为默认编译器
  至今：LLVM成为最活跃的编译器基础设施项目
```

---

## 2. Clang是什么 — LLVM的C/C++前端

### 1. 一句话定义

**Clang** 是基于LLVM的C/C++/Objective-C编译器前端，负责将源代码翻译成LLVM中间表示（IR），然后交给LLVM后端进行优化和代码生成。

### 2. Clang在LLVM中的位置

```
完整的编译流程（以Clang为例）：

┌──────────────────────────────────────────────────────────────┐
│                      编译流程                                  │
│                                                              │
│  [C/C++源码]                                                │
│      │                                                      │
│      ▼                                                      │
│  ┌──────────┐                                               │
│  │  Clang   │ ← 前端（Frontend）                             │
│  │  前端    │    词法分析→语法分析→语义分析→生成IR              │
│  └────┬─────┘                                               │
│       │                                                     │
│       ▼                                                     │
│  ┌──────────┐                                               │
│  │  LLVM IR │ ← 中间表示（Intermediate Representation）       │
│  │          │    编译器的"通用语言"                            │
│  └────┬─────┘                                               │
│       │                                                     │
│       ▼                                                     │
│  ┌──────────┐                                               │
│  │  LLVM    │ ← 中端优化器（Middle-end Optimizer）            │
│  │  优化器  │    死代码消除/常量传播/循环优化/内联...           │
│  └────┬─────┘                                               │
│       │                                                     │
│       ▼                                                     │
│  ┌──────────┐                                               │
│  │  LLVM    │ ← 后端（Backend）                               │
│  │  代码生成│    指令选择→寄存器分配→指令调度→生成机器码        │
│  └────┬─────┘                                               │
│       │                                                     │
│       ▼                                                     │
│  [x86/ARM/RISC-V/WebAssembly/... 机器码]                    │
└──────────────────────────────────────────────────────────────┘

关键：Clang只负责"前端"部分，其余都由LLVM完成
```

### 3. LLVM与Clang的关系

```
LLVM项目包含的组件：

LLVM Project
├── LLVM Core        ← 编译器基础设施（IR/优化器/代码生成器）
├── Clang            ← C/C++/ObjC前端
├── Clangd           ← C/C++语言服务器（IDE支持）
├── lldb             ← 调试器
├── libc++           ← C++标准库实现
├── libc++abi        ← C++ ABI库
├── libunwind        ← 异常处理和栈展开
├── compiler-rt      ← 运行时库（sanitizer/profiling）
├── MLIR             ← 多层中间表示
├── OpenMP           ← OpenMP支持
├── Polly            ← 多面体优化
└── 其他子项目...

关系总结：
  LLVM = 整个项目/基础设施
  Clang = LLVM项目中的一个子项目（C/C++前端）

  Clang依赖LLVM，但LLVM不依赖Clang
  └── 你可以用LLVM但不使用Clang（用其他前端如Rust前端rustc、Swift前端等）
```

### 4. Clang是跨平台的

```
常见误解：Clang是Linux的？还是Windows的？
正确答案：Clang是跨平台的，不属于任何特定操作系统

支持的平台：
├── Linux    ✅ 原生支持，最常用的平台之一
├── Windows  ✅ 原生支持
├── macOS    ✅ 原生支持，Apple默认编译器
└── FreeBSD/其他Unix  ✅ 支持

Windows上的使用方式：
├── MSVC集成 — Visual Studio自带Clang（安装时勾选"C++ Clang Tools"）
├── LLVM官方安装包 — 从 releases.llvm.org 下载
└── MSYS2/MinGW — pacman -S mingw-w64-x86_64-clang

macOS上的特殊性：
└── macOS的clang是Apple定制版本（Apple Clang）
└── xcrun clang --version 会显示"Apple clang"
└── Apple从Xcode 4.2起将GCC替换为Clang+LLVM作为默认编译器

关键区别：
  Clang本身跨平台，但生成的目标代码取决于运行时库：

  配置                  │ 运行时库        │ 适用场景
  ─────────────────────┼────────────────┼──────────────────
  clang + MSVC SDK     │ MSVC运行库     │ Windows原生开发
  clang + MinGW        │ GCC运行库      │ Windows上类Linux开发
  clang + glibc        │ GCC运行库      │ Linux原生开发
  clang + Apple SDK    │ Apple运行库    │ macOS/iOS开发

总结：
  Clang是跨平台编译器，Linux/Windows/macOS都能用
  它不属于任何特定操作系统，而是由LLVM项目维护的开源软件
```

---

## 3. LLVM vs GCC — 两大编译器阵营

### 1. 架构对比

```
GCC架构：
  源代码 → [GCC前端] → [GENERIC/GIMPLE] → [GCC优化器] → [GCC后端] → 机器码
  └── 单体架构，各阶段耦合较紧
  └── 前端和后端在同一进程内

LLVM架构：
  源代码 → [Clang前端] → [LLVM IR] → [LLVM优化器] → [LLVM后端] → 机器码
  └── 模块化架构，各阶段可独立使用
  └── IR可序列化到文件（.ll文本/.bc二进制）
  └── 优化pass可自由组合
```

### 2. 详细对比

| 维度 | GCC | LLVM/Clang |
|------|-----|-----------|
| **架构** | 单体式 | 模块化 |
| **许可证** | GPL v3（传染性） | Apache 2.0 + LLVM Exception（宽松） |
| **编译速度** | 较慢 | 较快（尤其是增量编译） |
| **错误信息** | 冗长难懂 | 清晰友好（精确指向错误位置） |
| **C++标准支持** | 非常完整 | 非常完整 |
| **平台支持** | 非常广泛 | 广泛（快速追赶） |
| **优化能力** | 成熟强大 | 成熟强大（某些场景更优） |
| **可扩展性** | 困难（插件机制有限） | 容易（Pass框架/插件） |
| **工具生态** | 丰富但分散 | 统一且现代 |
| **IDE集成** | 有限 | 优秀（clangd/LibTooling） |
| **Sanitizer** | 支持 | 支持（更全面） |
| **默认编译器** | 大多数Linux发行版 | macOS/Xcode |

### 3. 错误信息对比

```cpp
// 有语法错误的代码
#include <vector>
#include <string>
int main() {
    std::vector<int> v;
    v.push_bakc(42);  // 拼写错误：push_bakc → push_back
}
```

```
GCC的错误信息：
  error: 'class std::vector<int>' has no member named 'push_bakc'
     v.push_bakc(42);
       ^~~~~~~~~

Clang的错误信息：
  error: no member named 'push_bakc' in 'std::vector<int>'
    v.push_bakc(42);
      ^~~~~~~~~
  note: corrected to 'push_back'
    v.push_back(42);
      ^~~~~~~~~
      push_back
  note: 'push_back' declared here
    push_back(const value_type& __x);
    ^
  1 error generated.

Clang的优势：
  ├── 自动建议修正（"corrected to 'push_back'"）
  ├── 显示声明位置（"declared here"）
  └── 更精确的错误定位
```

---

## 4. LLVM IR — 编译器的通用语言

### 1. 什么是LLVM IR

**比喻**：IR像世界语——不管源语言是C/C++/Rust/Swift，都先翻译成IR这种"通用语言"，然后再从IR翻译成各种机器码。

```
多语言→IR→多平台的架构：

[C/C++] ──→ Clang ──→ ┐
[Rust]  ──→ rustc ──→ ┤
[Swift] ──→ swiftc ─→ ┤→ [LLVM IR] → [LLVM优化器] → [LLVM后端] → ┬─ x86_64
[Fortran] ─→ Flang ─→ ┤                                           ├─ ARM64
[Kotlin] ─→ Kotlin─→ ┤                                           ├─ RISC-V
[其他]   ─→ ...    ─→ ┘                                           ├─ WebAssembly
                                                                  └─ GPU(PTX/AMDGPU)

关键价值：
  └── 前端只需关心"源语言→IR"
  └── 后端只需关心"IR→机器码"
  └── 新语言只需写前端，自动获得所有后端支持
  └── 新硬件只需写后端，自动支持所有前端语言
```

### 2. IR的两种格式

```
LLVM IR有两种表示形式：

1. 文本格式（.ll文件）— 人类可读
   ; 一个简单的加法函数
   define i32 @add(i32 %a, i32 %b) {
     entry:
       %result = add i32 %a, %b
       ret i32 %result
   }

2. 位码格式（.bc文件）— 机器高效
   └── 二进制编码，体积更小
   └── 加载更快

相互转换：
  .ll → .bc：llvm-as add.ll -o add.bc
  .bc → .ll：llvm-dis add.bc -o add.ll
```

### 3. IR基本语法速查

```
类型系统：
  i1      — 1位整数（布尔）
  i8      — 8位整数（char）
  i32     — 32位整数（int）
  i64     — 64位整数（long）
  float   — 32位浮点
  double  — 64位浮点
  i32*    — 指向i32的指针
  [10 x i32] — 10个i32的数组
  {i32, float} — 结构体

常用指令：
  add/sub/mul/sdiv/udiv  — 算术运算
  and/or/xor             — 位运算
  icmp/fcmp              — 比较
  br/switch/ret          — 控制流
  load/store             — 内存访问
  call                   — 函数调用
  alloca                 — 栈上分配
  getelementptr (GEP)    — 指针运算（最独特的IR指令）
  phi                    — SSA的φ节点

函数定义：
  define 返回类型 @函数名(参数列表) {
    基本块:
      指令
  }
```

### 4. 查看Clang生成的IR

```bash
# 生成文本格式IR
clang -S -emit-llvm main.cpp -o main.ll

# 生成位码格式IR
clang -c -emit-llvm main.cpp -o main.bc

# 查看优化前的IR
clang -O0 -S -emit-llvm main.cpp -o main_O0.ll

# 查看优化后的IR
clang -O2 -S -emit-llvm main.cpp -o main_O2.ll

# 对比优化前后的差异
diff main_O0.ll main_O2.ll
```

---

## 5. Clang的安装与使用

### 1. 安装

```bash
# Ubuntu/Debian
sudo apt install clang llvm

# macOS（Xcode自带）
xcode-select --install

# Windows
# 方法1：随Visual Studio安装（C++ Clang工具）
# 方法2：从 https://releases.llvm.org/ 下载预编译版本
# 方法3：使用winget
winget install LLVM.LLVM

# 验证安装
clang --version
llvm-config --version
```

### 2. 基本使用

```bash
# 编译C程序
clang hello.c -o hello

# 编译C++程序
clang++ hello.cpp -o hello

# 指定C++标准
clang++ -std=c++20 hello.cpp -o hello

# 启用警告
clang++ -Wall -Wextra -Wpedantic hello.cpp -o hello

# 启用优化
clang++ -O2 hello.cpp -o hello

# 生成调试信息
clang++ -g hello.cpp -o hello

# 生成调试信息 + 不优化
clang++ -g -O0 hello.cpp -o hello

# 仅编译不链接
clang++ -c hello.cpp -o hello.o

# 编译+链接
clang++ hello.o utils.o -o myapp

# 使用Sanitizer
clang++ -fsanitize=address hello.cpp -o hello    # 内存错误检测
clang++ -fsanitize=undefined hello.cpp -o hello   # 未定义行为检测
clang++ -fsanitize=memory hello.cpp -o hello      # 未初始化内存检测
clang++ -fsanitize=thread hello.cpp -o hello      # 数据竞争检测
```

### 3. Clang特有的有用选项

```bash
# 生成AST（抽象语法树）—— 理解代码结构
clang -Xclang -ast-dump -fsyntax-only main.cpp

# 生成预处理后的文件
clang -E main.cpp -o main.i

# 生成汇编代码
clang -S main.cpp -o main.s

# 查看所有include路径
clang -E -v main.cpp 2>&1 | grep "^ "

# 查看预定义宏
clang -dM -E - < /dev/null

# 生成依赖关系
clang -M main.cpp

# 静态分析
clang --analyze main.cpp

# 格式化代码（clang-format）
clang-format -i main.cpp

# 查看Clang使用的GCC兼容选项
clang -dumpversion
```

---

## 6. LLVM工具链 — 编译器隐藏的瑞士军刀

### 1. 核心工具

| 工具 | 功能 | 常用命令 |
|------|------|---------|
| **llvm-as** | .ll文本→.bc位码 | `llvm-as hello.ll -o hello.bc` |
| **llvm-dis** | .bc位码→.ll文本 | `llvm-dis hello.bc -o hello.ll` |
| **opt** | IR优化器 | `opt -O2 hello.bc -o hello_opt.bc` |
| **llc** | IR→汇编代码 | `llc hello.bc -o hello.s` |
| **lli** | IR直接执行（JIT） | `lli hello.bc` |
| **llvm-link** | IR链接器 | `llvm-link a.bc b.bc -o combined.bc` |
| **llvm-ar** | IR归档工具 | `llvm-ar rcs lib.a a.o b.o` |
| **llvm-nm** | 符号查看 | `llvm-nm hello.o` |
| **llvm-objdump** | 反汇编 | `llvm-objdump -d hello.o` |
| **llvm-readobj** | 目标文件信息 | `llvm-readobj -h hello.o` |
| **llvm-size** | 段大小查看 | `llvm-size hello.o` |
| **llvm-strip** | 去除符号 | `llvm-strip hello.o` |
| **llvm-dwarfdump** | DWARF调试信息 | `llvm-dwarfdump hello.o` |
| **llvm-profdata** | Profile数据处理 | `llvm-profdata merge default.profraw` |
| **llvm-cov** | 代码覆盖率 | `llvm-cov show ./myapp` |
| **FileCheck** | 测试输出验证 | `FileCheck test.ll < output` |

### 2. 实用工具组合

```bash
# 完整的编译流程分解（理解每一步）

# Step 1: 预处理
clang -E main.cpp -o main.i

# Step 2: 生成IR
clang -emit-llvm -S main.i -o main.ll

# Step 3: 优化IR
opt -O2 main.ll -S -o main_opt.ll

# Step 4: 生成汇编
llc main_opt.ll -o main.s

# Step 5: 汇编为目标文件
clang -c main.s -o main.o

# Step 6: 链接
clang main.o -o myapp

# 一步到位（等价于上面6步）
clang -O2 main.cpp -o myapp
```

```bash
# 使用lli直接执行IR（不需要编译成机器码）
clang -emit-llvm -S main.cpp -o main.ll
lli main.ll

# 查看优化pass的效果
opt -O0 -S main.ll -o main_O0.ll
opt -O2 -S main.ll -o main_O2.ll
diff main_O0.ll main_O2.ll

# 查看特定优化pass
opt -passes=instcombine -S main.ll -o main_ic.ll
opt -passes=gvn -S main.ll -o main_gvn.ll
opt -passes=loop-unroll -S main.ll -o main_lu.ll
```

---

## 7. Clangd — C/C++语言服务器

### 1. 什么是Clangd

```
Clangd = 基于Clang的C/C++语言服务器

作用：为编辑器/IDE提供智能代码功能
  ├── 代码补全
  ├── 错误诊断（实时红色波浪线）
  ├── 跳转到定义
  ├── 查找引用
  ├── 代码重构
  ├── 悬停提示
  └── 代码格式化

工作原理：
  [VS Code/Neovim/其他编辑器]
      │ LSP协议
      ▼
  [Clangd进程]
      │ 调用
      ▼
  [Clang前端] → 解析代码 → 提供智能功能

为什么Clangd比其他方案好：
  └── 直接复用Clang的解析器（和编译器看到的信息完全一致）
  └── 不需要维护单独的索引数据库
  └── 支持所有编译器选项（通过compile_commands.json）
```

### 2. Clangd配置

```bash
# 安装
# VS Code：安装 clangd 扩展（替代Microsoft C/C++扩展的IntelliSense）
# Neovim：nvim-lspconfig 配置 clangd

# 核心配置文件：compile_commands.json
# 由CMake自动生成：
cmake -DCMAKE_EXPORT_COMPILE_COMMANDS=ON ..

# 或由Bear工具生成（非CMake项目）：
bear -- make

# .clangd配置文件（项目根目录）
# .clangd
CompileFlags:
  Add: [-std=c++20, -Wall]
  Remove: [-Werror]
Diagnostics:
  UnusedIncludes: Strict
  MissingIncludes: Strict
InlayHints:
  Enabled: Yes
  ParameterNames: Yes
  DeducedTypes: Yes
```

---

## 8. 基于LLVM构建自己的工具

### 1. LLVM的扩展点

```
LLVM提供的扩展方式：

1. LLVM Pass — IR层面的分析和变换
   └── 死代码消除、常量传播、循环优化...
   └── 你可以写自己的优化Pass

2. Clang Plugin — 编译时插件
   └── 自定义警告、代码规范检查、静态分析...

3. LibTooling — 独立的代码分析工具
   └── 基于Clang的AST进行代码重构、分析、转换

4. Clangd扩展 — 编辑器增强
   └── 自定义代码补全、诊断、重构

5. LLVM后端 — 新硬件支持
   └── 为新CPU/GPU/FPGA编写代码生成器
```

### 2. LLVM Pass示例

```cpp
// 一个简单的LLVM Pass：统计函数中的基本块数量
// 文件：HelloPass.cpp

#include "llvm/IR/Function.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

struct HelloPass : public PassInfoMixin<HelloPass> {
    PreservedAnalyses run(Function &F, FunctionAnalysisManager &AM) {
        int bbCount = 0;
        for (auto &BB : F) {
            bbCount++;
        }
        errs() << "函数 " << F.getName() << " 有 " << bbCount << " 个基本块\n";
        return PreservedAnalyses::all();
    }

    static bool isRequired() { return true; }
};

// 注册Pass
extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {LLVM_PLUGIN_API_VERSION, "HelloPass", LLVM_VERSION_STRING,
            [](PassBuilder &PB) {
                PB.registerPipelineParsingCallback(
                    [](StringRef Name, FunctionPassManager &FPM,
                       ArrayRef<PassBuilder::PipelineElement>) {
                        if (Name == "hello-pass") {
                            FPM.addPass(HelloPass());
                            return true;
                        }
                        return false;
                    });
            }};
}
```

```bash
# 编译Pass插件
clang++ -shared -fPIC HelloPass.cpp -o HelloPass.so \
    $(llvm-config --cxxflags --ldflags --libs core)

# 使用Pass插件
opt -load-pass-plugin ./HelloPass.so -passes="hello-pass" -S main.ll
```

### 3. LibTooling示例 — 代码重构工具

```cpp
// 一个简单的LibTooling工具：重命名函数
// 文件：RenameTool.cpp

#include "clang/Tooling/CommonOptionsParser.h"
#include "clang/Tooling/Refactoring.h"
#include "clang/Tooling/Refactoring/Rename/RenamingAction.h"
#include "clang/Frontend/FrontendAction.h"
#include "llvm/Support/CommandLine.h"

using namespace clang;
using namespace clang::tooling;

static llvm::cl::OptionCategory Category("rename-tool");
static llvm::cl::opt<std::string> OldName("old", llvm::cl::desc("旧函数名"),
                                           llvm::cl::cat(Category));
static llvm::cl::opt<std::string> NewName("new", llvm::cl::desc("新函数名"),
                                           llvm::cl::cat(Category));

int main(int argc, const char **argv) {
    auto ExpectedParser = CommonOptionsParser::create(argc, argv, Category);
    if (!ExpectedParser) {
        llvm::errs() << ExpectedParser.takeError();
        return 1;
    }
    ClangTool Tool(ExpectedParser->getCompilations(),
                   ExpectedParser->getSourcePathList());
    // 执行重命名...
    return Tool.run(newFrontendActionFactory().get());
}
```

---

## 9. LLVM生态全景

```
基于LLVM的项目和语言：

编程语言前端：
├── Clang — C/C++/Objective-C
├── rustc — Rust
├── swiftc — Swift
├── Flang — Fortran
├── MLIR — 机器学习IR
├── Julia — 科学计算
├── Zig — 系统编程
├── Crystal — Ruby风格系统语言
└── ISPC — SPMD编程

工具链：
├── Clangd — 语言服务器
├── clang-tidy — 代码规范检查
├── clang-format — 代码格式化
├── clang-query — AST交互查询
├── clang-check — 语法检查
├── scan-build — 静态分析
├── not-yet-common-ssl — SSL分析
└── include-what-you-use — 头文件清理

GPU支持：
├── AMDGPU后端 — AMD显卡
├── NVPTX后端 — NVIDIA GPU
├── SPIR-V后端 — Vulkan/OpenCL
└── Intel GPU后端 — Intel GPU

安全工具：
├── AddressSanitizer — 内存错误检测
├── MemorySanitizer — 未初始化内存检测
├── ThreadSanitizer — 数据竞争检测
├── UndefinedBehaviorSanitizer — UB检测
├── DataFlowSanitizer — 数据流追踪
└── LeakSanitizer — 内存泄漏检测
```

---

## 10. GCC vs Clang选择指南

```
什么时候用GCC：
├── Linux内核开发（官方支持GCC）
├── 需要GCC特有的扩展或插件
├── 目标平台Clang不支持
├── 已有大量GCC构建脚本
└── 需要GCC的某些特定优化

什么时候用Clang：
├── macOS/iOS开发（Apple官方编译器）
├── 需要更好的错误信息
├── 需要模块化编译器组件
├── 开发编译器工具（Pass/LibTooling）
├── 需要clangd的IDE支持
├── 需要更全面的Sanitizer
├── 需要跨平台一致性
└── 需要Apache 2.0许可证（商业友好）

大多数情况下两者可以互换：
  └── 代码兼容性高
  └── 性能差异小
  └── 建议在CI中同时测试GCC和Clang
```

***

### 1. 相关章节

- [GCC-G++编译器深度使用指南](../05-开发环境与IDE/07-GCC编译器基础.md) — GCC编译参数、Sanitizer、分析工具
- [编译器隐藏工具与鲜为人知的能力](../05-开发环境与IDE/10-二进制分析工具.md) — GCC/LLVM工具链全览
- [VS-Code开发环境完全配置指南](../05-开发环境与IDE/00-VSCode核心配置.md) — clangd配置与使用
- [为什么代码可以调试-调试信息深度解析](../08-调试与性能/06-为什么代码可以调试-调试信息深度解析.md) — DWARF/PDB/dSYM格式
- [C++标准库与第三方库学习指南](../07-现代CPP标准库/18-C++标准库与第三方库.md) — libc++ vs libstdc++
- [程序漏洞与安全攻防基础](../09-系统与安全/00-程序漏洞与安全攻防基础.md) — Sanitizer安全检测

***

### 相关阅读

- [GNU与GCC](./27-GNU与GCC.md)
- [什么是交叉编译Cross-Compilation](./29-什么是交叉编译Cross-Compilation.md)
- [CPP工具链](../08-调试与性能/03-CPP工具链.md)