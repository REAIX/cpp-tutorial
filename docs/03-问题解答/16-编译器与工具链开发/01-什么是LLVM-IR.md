# 什么是LLVM IR
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)、[编译器原理](../../10-编译器与工具链开发/00-编译器原理概述.md)

> "核心提炼：LLVM IR是LLVM编译器基础设施的核心中间表示，它采用SSA（静态单赋值）形式，是连接前端（语言相关）和后端（平台相关）的桥梁——前端把源码翻译成IR，后端把IR翻译成机器码，IR就是编译器的'世界语'。"

***

## 1. LLVM IR的语法与语义

### 1. IR在编译流程中的位置

```
源代码（C/C++/Rust/Swift...）
    │
    ▼
┌──────────┐
│  前端     │  Clang / rustc / swiftc
└────┬─────┘
     │
     ▼
┌──────────┐
│ LLVM IR  │  ← 我们要深入理解的部分
│ 中间表示  │  人类可读的文本格式(.ll) + 二进制格式(.bc)
└────┬─────┘
     │
     ▼
┌──────────┐
│  优化器   │  各种Pass对IR进行优化
└────┬─────┘
     │
     ▼
┌──────────┐
│  后端     │  代码生成 → x86/ARM/RISC-V/WASM...
└──────────┘
```

### 2. IR的两种格式

```
1. 文本格式（.ll文件）— 人类可读
   ┌── 用于调试、学习、手动编写IR
   └── 类似汇编语言，但更高级

2. 位码格式（.bc文件）— 机器高效
   ┌── 二进制编码，体积更小
   └── 加载更快，用于模块间链接

相互转换：
   .ll → .bc：llvm-as hello.ll -o hello.bc
   .bc → .ll：llvm-dis hello.bc -o hello.ll
```

### 3. IR基本语法

```llvm
; 这是一个LLVM IR的注释

; === 模块结构 ===
; 源文件名
source_filename = "hello.cpp"
; 目标三元组
target triple = "x86_64-pc-linux-gnu"

; === 全局变量 ===
@global_var = global i32 42
@const_str = private unnamed_addr constant [13 x i8] c"hello world\0A\00"

; === 函数声明 ===
declare i32 @printf(i8*, ...)

; === 函数定义 ===
define i32 @add(i32 %a, i32 %b) {
entry:
    %result = add i32 %a, %b
    ret i32 %result
}

; === 主函数 ===
define i32 @main() {
entry:
    %1 = call i32 @add(i32 3, i32 4)
    ret i32 %1
}
```

### 4. 类型系统

```
LLVM IR的类型系统是严格且显式的：

整数类型：
  i1      — 1位（布尔值）
  i8      — 8位（char/byte）
  i16     — 16位（short）
  i32     — 32位（int）
  i64     — 64位（long/long long）
  i128    — 128位
  iN      — 任意N位整数

浮点类型：
  half    — 16位浮点（IEEE 754半精度）
  float   — 32位浮点（单精度）
  double  — 64位浮点（双精度）
  fp128   — 128位浮点（四精度）

指针类型（旧式，新式使用ptr）：
  i32*    — 指向i32的指针
  float*  — 指向float的指针
  ptr     — 不透明指针（LLVM 15+推荐）

聚合类型：
  [10 x i32]       — 10个i32的数组
  {i32, float*}    — 结构体
  <4 x float>      — 4个float的向量（SIMD）

函数类型：
  i32 (i32, i32*)  — 接受i32和i32*，返回i32的函数
```

### 5. 常用指令详解

```llvm
; === 算术运算 ===
%a = add i32 %x, %y          ; 整数加法
%b = sub i32 %x, %y          ; 整数减法
%c = mul i32 %x, %y          ; 整数乘法
%d = sdiv i32 %x, %y         ; 有符号除法
%e = udiv i32 %x, %y         ; 无符号除法
%f = srem i32 %x, %y         ; 有符号取余
%g = urem i32 %x, %y         ; 无符号取余

; === 浮点运算 ===
%fa = fadd float %x, %y      ; 浮点加法
%fb = fsub float %x, %y      ; 浮点减法
%fc = fmul float %x, %y      ; 浮点乘法
%fd = fdiv float %x, %y      ; 浮点除法

; === 位运算 ===
%ba = and i32 %x, %y         ; 按位与
%bb = or i32 %x, %y          ; 按位或
%bc = xor i32 %x, %y         ; 按位异或
%bd = shl i32 %x, 3          ; 左移
%be = lshr i32 %x, 3         ; 逻辑右移
%bf = ashr i32 %x, 3         ; 算术右移

; === 比较运算 ===
%cmp1 = icmp eq i32 %x, %y    ; 整数相等
%cmp2 = icmp slt i32 %x, %y   ; 有符号小于
%cmp3 = icmp ugt i32 %x, %y   ; 无符号大于
%cmp4 = fcmp oeq float %x, %y ; 浮点有序相等
%cmp5 = fcmp ult float %x, %y ; 浮点无序小于

; === 类型转换 ===
%z1 = zext i8 %byte to i32    ; 零扩展
%z2 = sext i8 %byte to i32    ; 符号扩展
%z3 = trunc i32 %word to i8   ; 截断
%z4 = fptoui float %f to i32  ; 浮点转无符号整数
%z5 = sitofp i32 %i to float  ; 有符号整数转浮点
%z6 = bitcast i32* %p to float* ; 位模式转换（不改数据）

; === 内存操作 ===
%ptr = alloca i32              ; 在栈上分配i32大小的空间
%val = load i32, i32* %ptr     ; 从内存加载i32
store i32 42, i32* %ptr        ; 将42存入内存

; === 函数调用 ===
%ret = call i32 @add(i32 3, i32 4)  ; 调用函数

; === 控制流 ===
br label %target                      ; 无条件跳转
br i1 %cond, label %true, label %false ; 条件跳转
switch i32 %val, label %default [i32 1, label %case1]
ret i32 %result                       ; 函数返回
```

### 6. GEP指令——最独特的IR指令

```llvm
; GEP = GetElementPointer
; 作用：计算聚合类型（数组/结构体）中元素的地址
; 关键：GEP只做指针运算，不访问内存

; 结构体定义
%struct.Point = type { i32, i32 }

; 获取结构体中第二个字段（y坐标）的地址
%p = alloca %struct.Point
%y_ptr = getelementptr %struct.Point, %struct.Point* %p, i32 0, i32 1
;                                              ↑ 基地址  ↑索引0(第0个结构体) ↑索引1(第2个字段)

; 数组元素地址计算
%arr = alloca [10 x i32]
%elem_ptr = getelementptr [10 x i32], [10 x i32]* %arr, i32 0, i32 5
;                                                  ↑ 索引0(第0个数组) ↑ 索引5(第6个元素)

; 等价C代码：
; struct Point p;
; int* y_ptr = &p.y;       // GEP计算 &p.y
; int arr[10];
; int* elem_ptr = &arr[5]; // GEP计算 &arr[5]
```

***

## 2. SSA形式

### 1. 什么是SSA

```
SSA = Static Single Assignment（静态单赋值）

核心规则：每个变量只被赋值一次

非SSA形式：
  x = 1
  x = 2        ← x被赋值两次，违反SSA
  y = x + 1

SSA形式：
  x1 = 1       ← 每次赋值使用新名称
  x2 = 2
  y1 = x2 + 1

为什么SSA很重要：
  └── 简化数据流分析
  └── 使优化算法更简单高效
  └── 消除假依赖（同名变量的不同定义）
  └── 现代编译器（LLVM/GCC）都使用SSA
```

### 2. SSA中的phi函数

```
当控制流汇合时，同一个变量可能有不同的定义，需要phi函数来选择

C代码：
  int x;
  if (condition) {
      x = 1;      // 定义1
  } else {
      x = 2;      // 定义2
  }
  int y = x + 3;  // 使用x，但x有两个可能的值

SSA形式：
  if.then:
    x1 = 1
    br label %merge
  if.else:
    x2 = 2
    br label %merge
  merge:
    x3 = phi i32 [x1, %if.then], [x2, %if.else]  ← phi函数
    ; phi的含义：如果从if.then来，x3=x1；如果从if.else来，x3=x2
    y1 = add i32 x3, 3
```

**完整的if-else示例**：

```llvm
; C代码：
; int max(int a, int b) {
;     if (a > b) return a;
;     else return b;
; }

define i32 @max(i32 %a, i32 %b) {
entry:
    %cmp = icmp sgt i32 %a, %b    ; a > b ?
    br i1 %cmp, label %if.then, label %if.else

if.then:
    br label %return

if.else:
    br label %return

return:
    %result = phi i32 [%a, %if.then], [%b, %if.else]
    ret i32 %result
}
```

### 3. 循环中的SSA

```llvm
; C代码：
; int sum(int n) {
;     int s = 0;
;     for (int i = 0; i < n; i++)
;         s += i;
;     return s;
; }

define i32 @sum(i32 %n) {
entry:
    %cmp0 = icmp sgt i32 %n, 0
    br i1 %cmp0, label %loop.header, label %exit

loop.header:
    ; phi节点：s和i在循环中不断更新
    %i = phi i32 [0, %entry], [%i.next, %loop.body]
    %s = phi i32 [0, %entry], [%s.next, %loop.body]
    %cmp = icmp slt i32 %i, %n
    br i1 %cmp, label %loop.body, label %exit

loop.body:
    %s.next = add i32 %s, %i       ; s += i
    %i.next = add i32 %i, 1        ; i++
    br label %loop.header

exit:
    %s.final = phi i32 [0, %entry], [%s, %loop.header]
    ret i32 %s.final
}
```

***

## 3. 基本块与控制流图

### 1. 基本块（Basic Block）

```
基本块 = 一段连续的指令序列，满足：
  ├── 只有第一条指令可以作为入口（没有跳转进入中间）
  └── 只有最后一条指令可以离开（跳转/返回）

基本块的结构：
  [标签]:          ← 基本块入口
    指令1
    指令2
    ...
    终止指令       ← 基本块出口（br/ret/switch等）

终止指令类型：
  ├── ret         — 函数返回
  ├── br          — 条件/无条件跳转
  ├── switch      — 多路分支
  ├── invoke      — 带异常处理的调用
  ├── resume      — 异常恢复
  ├── unreachable — 不可达代码标记
  └── indirectbr  — 间接跳转
```

### 2. 控制流图（CFG）

```
控制流图 = 基本块 + 基本块之间的边

示例：if-else语句的CFG

        [entry]
           │
           ▼
      ┌─[cond]──┐
      │         │
      ▼         ▼
  [if.then]  [if.else]
      │         │
      ▼         ▼
      └─[merge]─┘
           │
           ▼
        [return]

CFG的用途：
  ├── 优化 — 识别循环、不可达代码
  ├── 分析 — 数据流分析、活跃变量分析
  └── 验证 — 确保所有路径都有正确的返回值
```

### 3. 用LLVM工具查看CFG

```bash
# 生成IR
clang -O0 -S -emit-llvm hello.cpp -o hello.ll

# 使用opt生成CFG的dot文件
opt -passes=dot-cfg hello.ll -disable-output
# 生成 .cfg.dot 文件

# 转换为图片（需要graphviz）
dot -Tpng hello.dot -o cfg.png

# 使用llvm-viewer直接查看
llvm-viewer hello.ll
```

***

## 4. IR与源码的对应关系

### 1. 简单变量

```cpp
// C++源码
int x = 42;
int y = x + 1;
```

```llvm
; 对应的LLVM IR
%x = alloca i32                    ; 分配栈空间给x
store i32 42, i32* %x              ; x = 42
%1 = load i32, i32* %x             ; 读取x的值
%2 = add i32 %1, 1                 ; x + 1
%y = alloca i32                    ; 分配栈空间给y
store i32 %2, i32* %y              ; y = x + 1

; 注意：未优化时，每个变量都通过alloca+load/store访问
; 优化后，这些会被消除，直接使用寄存器
```

### 2. 函数调用

```cpp
// C++源码
int add(int a, int b) {
    return a + b;
}
int main() {
    int result = add(3, 4);
    return result;
}
```

```llvm
; 对应的LLVM IR
define i32 @add(i32 %a, i32 %b) {
entry:
    %result = add i32 %a, %b
    ret i32 %result
}

define i32 @main() {
entry:
    %result = call i32 @add(i32 3, i32 4)
    ret i32 %result
}
```

### 3. 类与对象

```cpp
// C++源码
class Point {
public:
    int x, y;
    int sum() { return x + y; }
};

int main() {
    Point p;
    p.x = 1;
    p.y = 2;
    return p.sum();
}
```

```llvm
; 对应的LLVM IR
%class.Point = type { i32, i32 }

; Point::sum() 方法
define i32 @_ZN5Point3sumEv(%class.Point* %this) {
entry:
    %x_ptr = getelementptr %class.Point, %class.Point* %this, i32 0, i32 0
    %x = load i32, i32* %x_ptr
    %y_ptr = getelementptr %class.Point, %class.Point* %this, i32 0, i32 1
    %y = load i32, i32* %y_ptr
    %result = add i32 %x, %y
    ret i32 %result
}

define i32 @main() {
entry:
    %p = alloca %class.Point
    ; p.x = 1
    %x_ptr = getelementptr %class.Point, %class.Point* %p, i32 0, i32 0
    store i32 1, i32* %x_ptr
    ; p.y = 2
    %y_ptr = getelementptr %class.Point, %class.Point* %p, i32 0, i32 1
    store i32 2, i32* %y_ptr
    ; p.sum()
    %result = call i32 @_ZN5Point3sumEv(%class.Point* %p)
    ret i32 %result
}
```

### 4. 虚函数调用

```cpp
// C++源码
class Base {
public:
    virtual int foo() { return 1; }
};
class Derived : public Base {
public:
    int foo() override { return 2; }
};
int call(Base* b) { return b->foo(); }
```

```llvm
; 对应的LLVM IR（简化）
define i32 @_Z4callP4Base(%class.Base* %b) {
entry:
    ; 加载vtable指针
    %vtable = load void (%class.Base*)**, void (%class.Base*)*** %b
    ; 加载foo在vtable中的条目（第2个，索引1，因为第0个是offset-to-top）
    %foo_ptr = getelementptr void (%class.Base*)*, void (%class.Base*)** %vtable, i64 1
    %foo = load void (%class.Base*)*, void (%class.Base*)** %foo_ptr
    ; 间接调用虚函数
    %result = call i32 %foo(%class.Base* %b)
    ret i32 %result
}
```

***

## 5. 如何查看LLVM IR

### 1. 使用Clang生成IR

```bash
# 生成文本格式IR（人类可读）
clang -S -emit-llvm hello.cpp -o hello.ll

# 生成位码格式IR（二进制）
clang -c -emit-llvm hello.cpp -o hello.bc

# 不同优化等级的IR
clang -O0 -S -emit-llvm hello.cpp -o hello_O0.ll   # 无优化
clang -O1 -S -emit-llvm hello.cpp -o hello_O1.ll   # 基本优化
clang -O2 -S -emit-llvm hello.cpp -o hello_O2.ll   # 标准优化
clang -O3 -S -emit-llvm hello.cpp -o hello_O3.ll   # 激进优化

# 对比优化前后的差异
diff hello_O0.ll hello_O2.ll
```

### 2. 使用LLVM工具操作IR

```bash
# 文本格式 ↔ 位码格式
llvm-as hello.ll -o hello.bc     # .ll → .bc
llvm-dis hello.bc -o hello.ll    # .bc → .ll

# 运行优化Pass
opt -O2 hello.ll -S -o hello_opt.ll              # 运行O2优化
opt -passes=instcombine hello.ll -S -o out.ll    # 运行特定Pass

# 直接执行IR（JIT方式）
lli hello.bc

# 链接多个IR模块
llvm-link a.ll b.ll -S -o combined.ll

# 查看IR中的函数列表
llvm-nm hello.bc
```

### 3. 在C++中生成和操作IR

```cpp
// 使用LLVM C++ API生成IR
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

int main() {
    // 创建LLVM上下文和模块
    LLVMContext context;
    std::unique_ptr<Module> module = std::make_unique<Module>("hello", context);

    // 创建IRBuilder
    IRBuilder<> builder(context);

    // 创建函数类型：int add(int, int)
    FunctionType* funcType = FunctionType::get(
        Type::getInt32Ty(context),           // 返回类型
        {Type::getInt32Ty(context),          // 参数1类型
         Type::getInt32Ty(context)},         // 参数2类型
        false                                 // 不接受可变参数
    );

    // 创建函数
    Function* addFunc = Function::Create(
        funcType,
        Function::ExternalLinkage,
        "add",
        module.get()
    );

    // 设置参数名
    auto args = addFunc->arg_begin();
    Value* argA = &(*args++);
    argA->setName("a");
    Value* argB = &(*args);
    argB->setName("b");

    // 创建基本块
    BasicBlock* entry = BasicBlock::Create(context, "entry", addFunc);
    builder.SetInsertPoint(entry);

    // 生成加法指令
    Value* result = builder.CreateAdd(argA, argB, "result");
    // 生成返回指令
    builder.CreateRet(result);

    // 打印生成的IR
    module->print(outs(), nullptr);

    return 0;
}
```

### 4. 使用Godbolt在线查看IR

```
Godbolt (https://godbolt.org/) 是在线编译器浏览器：
  ├── 支持多种编译器（GCC/Clang/MSVC等）
  ├── 可以直接查看IR、汇编、预处理结果
  ├── 支持多种语言
  └── 免费使用

使用方法：
  1. 打开 https://godbolt.org/
  2. 在左侧输入C++代码
  3. 在编译器选项中添加 -emit-llvm -S
  4. 右侧即可看到LLVM IR
```

***

## 6. IR的验证与调试

### 1. IR验证器

```bash
# 验证IR的正确性
opt -passes=verify hello.ll -S -o /dev/null

# 常见验证错误：
# - phi节点的前驱块数量不匹配
# - 基本块没有终止指令
# - 使用了未定义的值
# - 类型不匹配
```

### 2. 调试IR生成

```bash
# 打印每一步Pass的效果
opt -O2 -print-after-all hello.ll -S -o hello_opt.ll 2>&1 | less

# 只打印特定Pass的效果
opt -passes=instcombine -print-after-instcombine hello.ll -S -o out.ll

# 查看Pass之间的差异
opt -O2 -print-before-after hello.ll -disable-output 2>&1 | less
```

***

## 7. 极简总结

| 概念 | 一句话 |
|------|--------|
| LLVM IR | 编译器的中间表示——连接前端和后端的"世界语" |
| SSA | 静态单赋值——每个变量只赋值一次，简化优化 |
| phi函数 | 控制流汇合时的"选择器"——根据来源选择值 |
| 基本块 | 连续指令序列——只有一个入口和一个出口 |
| CFG | 控制流图——基本块+跳转边 |
| GEP | 指针运算指令——只算地址，不访问内存 |
| .ll | 文本格式IR——人类可读 |
| .bc | 位码格式IR——机器高效 |

**LLVM IR = SSA形式的强类型中间语言，用基本块和phi函数表达控制流，用GEP做指针运算，是理解现代编译器的钥匙。**

***

### 相关阅读

- [编译器是如何工作的](./00-编译器是如何工作的.md)
- [什么是LLVM Pass](./03-什么是LLVM-Pass.md)
- [什么是SSA静态单赋值](./07-什么是SSA静态单赋值.md)
- [什么是寄存器分配](./06-什么是寄存器分配.md)
- [LLVM与Clang](../10-工程实践/26-LLVM与Clang.md)