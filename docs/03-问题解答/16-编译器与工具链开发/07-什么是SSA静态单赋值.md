# 什么是SSA静态单赋值
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)、[编译器原理](../../10-编译器与工具链开发/00-编译器原理概述.md)

> "精髓速览：SSA（Static Single Assignment）要求每个变量只被赋值一次，这看似简单的约束彻底改变了编译器的优化方式——它消除了同名变量的假依赖，让数据流分析变得简单直接，phi函数解决控制流汇合处的多定义问题，现代编译器（LLVM/GCC）都在SSA上做优化。"

***

## 1. SSA的原理

### 1. 为什么需要SSA

```
非SSA代码的问题：
  x = 1           ; 定义1
  x = 2           ; 定义2——同名变量被重复赋值
  y = x + 1       ; 使用x——但x有两个可能的值

问题：
  └── 同名变量的不同定义造成假依赖
  └── 编译器需要追踪每个定义的传播范围
  └── 优化算法需要额外工作来区分不同定义

SSA的解决方案：
  x1 = 1          ; 每次赋值使用新名称
  x2 = 2          ; 不同的定义，不同的名字
  y1 = x2 + 1     ; 明确使用哪个定义

好处：
  └── 每个变量只有一个定义点——数据流清晰
  └── use-def链是隐式的——变量名即定义
  └── 优化算法更简单——不需要追踪定义链
```

### 2. SSA的形式定义

```
SSA的严格定义：
  在程序的静态表示中，每个变量只被赋值一次

注意：
  └── "静态"——不是运行时只赋值一次
  └── 同一个变量在循环中每次迭代看起来赋值多次
  └── 但在SSA中，每次赋值都使用不同的名称

示例：
  C代码：
    int x = 1;
    if (cond) x = 2;
    printf("%d", x);

  SSA形式：
    x1 = 1
    if (cond) goto L1 else goto L2
  L1:
    x2 = 2
    goto L3
  L2:
    goto L3
  L3:
    x3 = phi(x1, x2)  ← 根据来源选择值
    printf("%d", x3)
```

### 3. 非SSA vs SSA对比

```cpp
// 非SSA形式
int compute(int a, int b) {
    int x = a + b;      // x定义1
    int y = x * 2;      // 使用x定义1
    x = a - b;          // x定义2——覆盖了定义1
    int z = x + y;      // 使用x定义2和y
    return z;
}

// SSA形式
int compute(int a, int b) {
    int x1 = a + b;     // x1只赋值一次
    int y1 = x1 * 2;    // 明确使用x1
    int x2 = a - b;     // x2是新的变量名
    int z1 = x2 + y1;   // 明确使用x2和y1
    return z1;
}
```

```
数据流对比：

非SSA的数据流图（复杂）：
  x的定义1 ──→ y = x * 2
  x的定义2 ──→ z = x + y
  └── 需要分析x的哪个定义到达了每个使用点

SSA的数据流图（简单）：
  x1 ──→ y1 = x1 * 2
  x2 ──→ z1 = x2 + y1
  └── 每个使用点直接对应唯一的定义
```

***

## 2. phi函数

### 1. 为什么需要phi函数

```
问题：控制流汇合时，同一个变量可能来自不同的定义

C代码：
  int x;
  if (cond) {
      x = 1;       // 定义A
  } else {
      x = 2;       // 定义B
  }
  int y = x;       // 使用x——但x来自定义A还是B？

SSA中：
  x1 = 1           // 定义A → x1
  x2 = 2           // 定义B → x2
  // y = x → 但x是x1还是x2？
  // 需要"选择"——这就是phi函数

  y1 = phi(x1, x2) // 如果从A来，y1=x1；如果从B来，y1=x2
```

### 2. phi函数的语义

```
phi函数的严格语义：
  phi(v1:BB1, v2:BB2, ..., vn:BBn)

  含义：根据控制流来自哪个前驱基本块，选择对应的值
  └── 如果从BB1来 → 取v1
  └── 如果从BB2来 → 取v2
  └── ...

注意：
  └── phi函数不是真正的指令——它不执行任何操作
  └── phi函数只在基本块入口处"概念性地"选择值
  └── 在实际代码生成时，phi函数会被消除（插入移动指令）
```

### 3. phi函数的LLVM IR表示

```llvm
; C代码：
; int max(int a, int b) {
;     int result;
;     if (a > b) result = a;
;     else result = b;
;     return result;
; }

define i32 @max(i32 %a, i32 %b) {
entry:
    %cmp = icmp sgt i32 %a, %b
    br i1 %cmp, label %if.then, label %if.else

if.then:
    br label %merge

if.else:
    br label %merge

merge:
    ; phi函数：从if.then来取%a，从if.else来取%b
    %result = phi i32 [%a, %if.then], [%b, %if.else]
    ret i32 %result
}
```

### 4. 复杂的phi函数场景

```llvm
; C代码：循环中的变量
; int sum = 0;
; for (int i = 0; i < n; i++) {
;     sum = sum + i;
; }

define i32 @loop_sum(i32 %n) {
entry:
    %cmp0 = icmp sgt i32 %n, 0
    br i1 %cmp0, label %loop.header, label %exit

loop.header:
    ; i的phi：第一次从entry来（初始值0），后续从loop.body来（i+1）
    %i = phi i32 [0, %entry], [%i.next, %loop.body]
    ; sum的phi：第一次从entry来（初始值0），后续从loop.body来（sum+i）
    %sum = phi i32 [0, %entry], [%sum.next, %loop.body]
    %cmp = icmp slt i32 %i, %n
    br i1 %cmp, label %loop.body, label %exit

loop.body:
    %sum.next = add i32 %sum, %i    ; sum = sum + i
    %i.next = add i32 %i, 1         ; i++
    br label %loop.header

exit:
    ; 从entry或loop.header来
    %sum.final = phi i32 [0, %entry], [%sum, %loop.header]
    ret i32 %sum.final
}
```

***

## 3. SSA的构造算法

### 1. 支配边界

```
支配边界（Dominance Frontier）是SSA构造的关键概念：

定义：
  节点B的支配边界DF(B) = {Y | B支配Y的某个前驱，但B不严格支配Y}

通俗理解：
  └── 支配边界 = "B的支配作用消失的边界"
  └── 在这些位置，需要插入phi函数

为什么：
  └── 如果变量v在B中定义
  └── 那么在B的支配边界处，v可能需要和来自其他路径的定义合并
  └── 合并 = 插入phi函数

示例：
        [entry]
           │
           ▼
        ┌─[A]─┐
        │      │
        ▼      ▼
      [B]    [C]
        │      │
        ▼      ▼
        └─[D]─┘
           │
           ▼
        [exit]

  A支配B、C、D、exit
  DF(A) = {D}  （A支配B和C，但不严格支配D，D是B和C的汇合点）
  DF(B) = {D}  （B支配...，D是B的支配边界）
  DF(C) = {D}
  DF(D) = {}   （D没有支配边界）
```

### 2. SSA构造的完整算法

```
步骤1：插入phi函数
  对每个变量v：
    1. 找到v的所有定义点（def(v)）
    2. 计算迭代支配边界 IDF(def(v))
    3. 在IDF的每个基本块入口插入phi(v)

步骤2：变量重命名
  1. 从入口基本块开始DFS遍历
  2. 维护每个变量的当前版本号栈
  3. 遇到定义：创建新版本号，压栈
  4. 遇到使用：使用栈顶的版本号
  5. 遇到phi函数：创建新版本号
  6. 离开基本块时：弹出该块中定义的版本号

步骤3：填充phi函数的操作数
  └── 根据前驱基本块中变量的版本号填充
```

### 3. 构造算法示例

```
原始代码：
  B1: x = 1
      if (cond) goto B2 else goto B3
  B2: x = 2
      goto B4
  B3: x = 3
      goto B4
  B4: y = x + 1

步骤1：插入phi函数
  x在B1、B2、B3中定义
  IDF({B1, B2, B3}) = {B4}  （B4是B2和B3的汇合点）
  在B4入口插入phi(x)

步骤2：重命名
  B1: x1 = 1
      if (cond) goto B2 else goto B3
  B2: x2 = 2
      goto B4
  B3: x3 = 3
      goto B4
  B4: x4 = phi(x1: B1, x2: B2, x3: B3)  ← 等等，B1不直接连到B4
      实际上：x4 = phi(x2: B2, x3: B3)
      y1 = x4 + 1

注意：phi函数的操作数来自直接前驱
  B4的前驱是B2和B3，所以phi(x2, x3)
  但x1可能通过B1→B3→B4到达，所以需要考虑
  实际上x1在B3被x3覆盖，所以B4的phi只需要x2和x3
```

### 4. 简化的构造实现

```cpp
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <stack>

struct BasicBlock {
    std::string name;
    std::vector<BasicBlock*> predecessors;
    std::vector<BasicBlock*> successors;
    std::unordered_set<std::string> defs;   // 在此块定义的变量
    std::unordered_set<std::string> uses;   // 在此块使用的变量
};

class SSAConstructor {
private:
    std::vector<BasicBlock*> blocks;

    // 计算支配边界
    std::unordered_map<BasicBlock*, std::unordered_set<BasicBlock*>> domFrontier;

    void computeDominanceFrontier() {
        // 简化实现：对每个基本块B
        // DF(B) = {Y | 存在B的前驱P，使得B支配P但B不严格支配Y}
        for (BasicBlock* B : blocks) {
            for (BasicBlock* succ : B->successors) {
                BasicBlock* runner = B;
                while (runner != getIDom(succ)) {
                    domFrontier[runner].insert(succ);
                    runner = getIDom(runner);
                }
            }
        }
    }

    BasicBlock* getIDom(BasicBlock* B) {
        // 返回B的直接支配者（简化实现）
        // 实际需要先计算支配树
        return nullptr;
    }

public:
    // 插入phi函数
    void insertPhiFunctions() {
        computeDominanceFrontier();

        // 对每个变量，在其定义的迭代支配边界插入phi
        std::unordered_map<std::string, std::unordered_set<BasicBlock*>> varDefs;

        // 收集每个变量的定义点
        for (BasicBlock* B : blocks) {
            for (const auto& var : B->defs) {
                varDefs[var].insert(B);
            }
        }

        // 在迭代支配边界插入phi函数
        for (auto& [var, defBlocks] : varDefs) {
            std::unordered_set<BasicBlock*> workList(defBlocks.begin(), defBlocks.end());
            std::unordered_set<BasicBlock*> visited;

            while (!workList.empty()) {
                BasicBlock* B = *workList.begin();
                workList.erase(workList.begin());

                for (BasicBlock* df : domFrontier[B]) {
                    if (visited.find(df) == visited.end()) {
                        visited.insert(df);
                        // 在df入口插入phi(var)
                        // 实际实现中需要修改IR
                        workList.insert(df);
                    }
                }
            }
        }
    }
};
```

***

## 4. SSA对优化的帮助

### 1. 常量传播

```
非SSA的常量传播：
  x = 1
  x = 2       ← 需要分析x的哪个定义到达使用点
  y = x       ← x可能是1或2？

SSA的常量传播：
  x1 = 1
  x2 = 2
  y1 = x2     ← 直接知道y1 = 2，无需分析

SSA让常量传播变得简单直接：
  └── 每个使用点对应唯一的定义
  └── 如果定义是常量，使用点可以直接替换
```

### 2. 死代码消除

```
非SSA的死代码消除：
  x = 1
  x = 2       ← x=1是死代码吗？需要检查x=1是否被使用
  y = x       ← 使用x=2

SSA的死代码消除：
  x1 = 1      ← 没有使用x1的地方 → 死代码
  x2 = 2
  y1 = x2

SSA让死代码消除更简单：
  └── 检查变量是否有使用点
  └── 没有使用点 → 死代码
  └── use-def链是隐式的（变量名即定义）
```

### 3. 公共子表达式消除

```
非SSA的CSE：
  a = b + c
  d = b + c   ← 和上面相同？需要分析b和c是否被修改

SSA的CSE：
  a1 = b1 + c1
  d1 = b1 + c1  ← b1和c1不可能被修改（SSA保证）
                  → a1和d1相同，d1 = a1

SSA让CSE更简单：
  └── 操作数相同 → 结果必然相同
  └── 不需要担心操作数被修改
```

### 4. 值编号（Value Numbering）

```cpp
// SSA形式下的值编号
// 如果两个表达式使用相同的操作数和运算符，它们的结果相同

#include <unordered_map>
#include <string>

class SSValueNumbering {
private:
    // 表达式 → 值编号
    std::unordered_map<std::string, int> exprToNumber;
    // 值编号 → 代表变量
    std::unordered_map<int, std::string> numberToVar;
    int nextNumber = 0;

public:
    // 为表达式分配值编号
    std::string processExpr(const std::string& op,
                           const std::string& lhs,
                           const std::string& rhs) {
        // 构造规范化的表达式键
        std::string key = op + " " + lhs + " " + rhs;

        auto it = exprToNumber.find(key);
        if (it != exprToNumber.end()) {
            // 已存在相同表达式，返回之前的变量
            return numberToVar[it->second];
        }

        // 新表达式
        int num = nextNumber++;
        exprToNumber[key] = num;
        return "";  // 新表达式，需要计算
    }
};

// 示例：
// x1 = a1 + b1  → 编号0，需要计算
// y1 = a1 + b1  → 编号0已存在，y1 = x1（消除冗余计算）
// z1 = a2 + b1  → 编号1，需要计算（a2 ≠ a1）
```

***

## 5. 为什么现代编译器都用SSA

### 1. SSA的优势总结

```
1. 简化数据流分析
   └── 每个变量只有一个定义
   └── use-def链是隐式的
   └── 不需要到达定义分析

2. 优化算法更简单
   └── 常量传播：直接替换
   └── 死代码消除：检查使用点
   └── CSE：操作数相同则结果相同
   └── 值编号：自然支持

3. 更好的优化效果
   └── 消除假依赖
   └── 更精确的别名分析
   └── 更激进的代码移动

4. 统一的中间表示
   └── LLVM IR基于SSA
   └── 所有优化都在SSA上做
   └── 只在代码生成前退出SSA

5. 寄存器分配更简单
   └── 活跃区间更短
   └── 干扰图更稀疏
```

### 2. 使用SSA的编译器

```
使用SSA的编译器：
  ├── LLVM — LLVM IR完全基于SSA
  ├── GCC  — GIMPLE IR使用SSA（从4.0开始）
  ├── V8   — TurboFan IR基于Sea-of-Nodes（SSA变体）
  ├── HotSpot — C2编译器使用Sea-of-Nodes
  ├── Rust — rustc生成SSA形式的MIR
  ├── Swift — SIL使用SSA
  └── Java — GraalVM使用SSA

不使用SSA的编译器：
  └── 早期GCC（4.0之前）— 使用非SSA的RTL
  └── 一些简单的JIT编译器 — 直接操作字节码
```

### 3. SSA的代价

```
SSA的额外开销：

1. phi函数
   └── 增加IR的大小
   └── 需要额外的处理（构造和消除）
   └── 在代码生成时需要插入移动指令

2. 构造成本
   └── 需要计算支配边界
   └── 需要变量重命名
   └── 时间复杂度O(N)（N为指令数）

3. 退出SSA
   └── 代码生成前需要消除phi函数
   └── 插入移动指令可能增加寄存器压力

4. 调试信息
   └── SSA的变量名和源码变量名不同
   └── 需要维护映射关系

总体评价：
  └── SSA的好处远大于代价
  └── 现代编译器都选择了SSA
```

***

## 6. SSA的退出——phi消除

### 1. 为什么需要消除phi

```
phi函数是"虚拟"指令：
  └── 它不对应任何机器指令
  └── CPU不能执行phi
  └── 在代码生成前必须消除

消除方法：
  在每个前驱基本块的末尾插入移动指令

示例：
  消除前：
    B1: x1 = 1; goto B3
    B2: x2 = 2; goto B3
    B3: x3 = phi(x1: B1, x2: B2)

  消除后：
    B1: x1 = 1; x3 = x1; goto B3
    B2: x2 = 2; x3 = x2; goto B3
    B3: (phi已消除，直接使用x3)
```

### 2. phi消除的陷阱——丢失拷贝问题

```
问题场景：
  B3: x3 = phi(x1: B1, x2: B2)
  B4: x4 = phi(x2: B1, x1: B2)  ← 注意x1和x2交叉

简单消除：
  B1: x3 = x1; x4 = x2; goto B3/B4
  B2: x3 = x2; x4 = x1; goto B3/B4

问题：在B1中，x3 = x1; x4 = x2;
  如果x1和x2映射到同一物理寄存器，x3 = x1会覆盖x1
  然后x4 = x2就得到了错误的值

解决方案：
  1. 使用临时变量
     B1: t = x1; x3 = x1; x4 = t;
  2. 仔细排序移动指令
  3. 使用并行拷贝语义（所有赋值同时发生）
```

***

## 7. 极简总结

| 概念 | 一句话 |
|------|--------|
| SSA | 每个变量只赋值一次——消除假依赖，简化优化 |
| phi函数 | 控制流汇合的选择器——根据来源选值 |
| 支配边界 | phi函数的插入位置——支配作用消失的边界 |
| 常量传播 | SSA下直接替换——use-def链隐式 |
| 死代码消除 | SSA下检查使用点——无使用即死代码 |
| CSE | SSA下操作数相同→结果相同 |
| phi消除 | 代码生成前必须做——插入移动指令 |
| 丢失拷贝 | phi消除的陷阱——需要临时变量或排序 |

**SSA = 每个变量只赋值一次 + phi函数处理控制流汇合，让数据流分析从"追踪定义链"变为"变量名即定义"，是现代编译器优化的基石。**

***

### 相关阅读

- [编译器是如何工作的](./00-编译器是如何工作的.md)
- [什么是LLVM IR](./01-什么是LLVM-IR.md)
- [什么是寄存器分配](./06-什么是寄存器分配.md)
- [什么是LLVM Pass](./03-什么是LLVM-Pass.md)