# 什么是DSL领域特定语言
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)、[编译器原理](../../10-编译器与工具链开发/00-编译器原理概述.md)

> "先抓核心：DSL（Domain-Specific Language）是为特定领域量身定制的编程语言，它用领域专家熟悉的术语和语法表达问题，牺牲通用性换取表达力和效率——SQL是数据库DSL，正则表达式是文本匹配DSL，C++中的模板元编程和运算符重载可以构建嵌入式DSL。"

***

## 1. DSL的设计原则

### 1. 什么是DSL

```
DSL vs 通用编程语言（GPL）：

通用编程语言（GPL）：
  ├── C/C++/Java/Python — 可以写任何程序
  ├── 设计目标：通用性、图灵完备
  └── 代价：对特定领域表达不够简洁

领域特定语言（DSL）：
  ├── SQL — 只能操作数据库
  ├── 正则表达式 — 只能匹配文本
  ├── HTML — 只能描述网页结构
  ├── CSS — 只能描述样式
  ├── Makefile — 只能描述构建规则
  ├── LaTeX — 只能排版文档
  └── 设计目标：特定领域的表达力和效率

类比：
  GPL = 瑞士军刀 — 什么都能做，但每样都不专精
  DSL = 专用工具 — 只做一件事，但做到极致
```

### 2. DSL的设计原则

```
DSL设计的核心原则：

1. 最小化表达（Minimal Expressivity）
   └── 只提供领域需要的概念，不多不少
   └── SQL不需要循环语句（集合操作替代）
   └── 正则不需要变量（模式匹配替代）

2. 声明式优于命令式（Declarative over Imperative）
   └── 描述"想要什么"而非"怎么做"
   └── SQL: SELECT name FROM users WHERE age > 18
   └── 不需要告诉数据库如何扫描表

3. 领域专家可读（Domain Expert Readable）
   └── 领域专家不需要是程序员也能理解
   └── SQL可以被DBA直接编写
   └── 正则可以被文本处理人员使用

4. 安全性（Safety）
   └── 不可能写出领域外的程序
   └── SQL不可能写出死循环
   └── HTML不可能写病毒

5. 可组合性（Composability）
   └── 小的DSL表达式可以组合成大的
   └── SQL子查询可以嵌套
   └── 正则可以用|和()组合
```

### 3. DSL的分类

```
按实现方式分类：

1. 外部DSL（External DSL）
   ├── 有自己独立的语法和解析器
   ├── SQL、正则、HTML、CSS
   └── 需要单独的编译器/解释器

2. 内部DSL / 嵌入式DSL（Internal DSL / Embedded DSL）
   ├── 宿主语言内的API/语法构造
   ├── C++的iostream、jQuery、Rake
   └── 不需要单独的解析器

3. 语言工作台（Language Workbench）
   ├── 专用工具创建DSL
   ├── JetBrains MPS、Xtext
   └── 提供完整的IDE支持

按抽象层次分类：

低级DSL — 接近机器
  └── 汇编语言、字节码、IR

中级DSL — 特定计算模式
  └── SQL（关系代数）、正则（自动机）

高级DSL — 接近领域专家
  └── LaTeX（排版）、Verilog（硬件设计）
```

***

## 2. 嵌入式DSL vs 独立DSL

### 1. 对比

| 维度 | 嵌入式DSL | 独立DSL |
|------|----------|--------|
| 语法 | 受限于宿主语言 | 完全自定义 |
| 解析器 | 不需要 | 需要自己写 |
| 类型检查 | 复用宿主语言 | 需要自己实现 |
| 错误信息 | 宿主语言提供 | 需要自己设计 |
| 工具支持 | 复用宿主语言IDE | 需要自己开发 |
| 学习成本 | 需要懂宿主语言 | 只需懂DSL |
| 灵活性 | 受宿主语言限制 | 完全自由 |
| 开发成本 | 低 | 高 |

### 2. 独立DSL示例

```
SQL — 数据库查询DSL

-- 查询年龄大于18的用户姓名
SELECT name, email
FROM users
WHERE age > 18
ORDER BY name;

-- 聚合查询
SELECT department, COUNT(*), AVG(salary)
FROM employees
GROUP BY department
HAVING COUNT(*) > 5;

SQL的设计特点：
  ├── 声明式 — 只说"要什么"，不说"怎么做"
  ├── 集合导向 — 操作整个集合，不是逐行
  ├── 无循环 — 不需要也不允许
  └── 优化器 — 数据库决定最优执行计划
```

```
正则表达式 — 文本匹配DSL

// 匹配邮箱地址
[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}

// 匹配IP地址
\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}

正则的设计特点：
  ├── 声明式 — 描述模式，不是匹配算法
  ├── 可组合 — |选择、()分组、*量词
  ├── 紧凑 — 一个表达式等价数十行代码
  └── 自动机理论 — 有坚实的数学基础
```

### 3. 嵌入式DSL示例

```
C++ iostream — I/O操作DSL

// 传统C风格
fprintf(stdout, "Name: %s, Age: %d\n", name, age);

// C++ iostream DSL风格
std::cout << "Name: " << name << ", Age: " << age << "\n";

iostream DSL的设计：
  ├── 运算符重载 << — 链式调用
  ├── 类型安全 — 编译期检查类型
  ├── 可扩展 — 自定义operator<<
  └── 嵌入C++ — 不需要额外解析器
```

***

## 3. C++中实现DSL的方式

### 1. 运算符重载

```cpp
// 矩阵运算DSL——运算符重载实现
#include <vector>
#include <cstdio>

class Matrix {
private:
    std::vector<std::vector<double>> data;
    int rows, cols;

public:
    Matrix(int r, int c) : rows(r), cols(c), data(r, std::vector<double>(c, 0.0)) {}

    double& operator()(int i, int j) { return data[i][j]; }
    const double& operator()(int i, int j) const { return data[i][j]; }

    // 矩阵加法
    Matrix operator+(const Matrix& other) const {
        Matrix result(rows, cols);
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                result(i, j) = data[i][j] + other(i, j);
        return result;
    }

    // 矩阵乘法
    Matrix operator*(const Matrix& other) const {
        Matrix result(rows, other.cols);
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < other.cols; j++)
                for (int k = 0; k < cols; k++)
                    result(i, j) += data[i][k] * other(k, j);
        return result;
    }

    // 标量乘法
    Matrix operator*(double scalar) const {
        Matrix result(rows, cols);
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < cols; j++)
                result(i, j) = data[i][j] * scalar;
        return result;
    }

    void print() const {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++)
                printf("%8.2f", data[i][j]);
            printf("\n");
        }
    }
};

// 使用DSL风格
int main() {
    Matrix A(2, 2);
    A(0, 0) = 1; A(0, 1) = 2;
    A(1, 0) = 3; A(1, 1) = 4;

    Matrix B(2, 2);
    B(0, 0) = 5; B(0, 1) = 6;
    B(1, 0) = 7; B(1, 1) = 8;

    // DSL风格：数学表达式直接写
    Matrix C = A * B + A * 2.0;
    C.print();

    return 0;
}
```

### 2. Builder模式

```cpp
// SQL查询构建器DSL
#include <string>
#include <sstream>

class QueryBuilder {
private:
    std::string table;
    std::string columns;
    std::string whereClause;
    std::string orderByClause;
    int limitVal = -1;

public:
    // SELECT子句
    QueryBuilder& select(const std::string& cols) {
        columns = cols;
        return *this;
    }

    // FROM子句
    QueryBuilder& from(const std::string& tbl) {
        table = tbl;
        return *this;
    }

    // WHERE子句
    QueryBuilder& where(const std::string& condition) {
        whereClause = condition;
        return *this;
    }

    // ORDER BY子句
    QueryBuilder& orderBy(const std::string& col) {
        orderByClause = col;
        return *this;
    }

    // LIMIT子句
    QueryBuilder& limit(int n) {
        limitVal = n;
        return *this;
    }

    // 构建SQL字符串
    std::string build() const {
        std::ostringstream sql;
        sql << "SELECT " << (columns.empty() ? "*" : columns);
        sql << " FROM " << table;
        if (!whereClause.empty()) sql << " WHERE " << whereClause;
        if (!orderByClause.empty()) sql << " ORDER BY " << orderByClause;
        if (limitVal > 0) sql << " LIMIT " << limitVal;
        return sql.str();
    }
};

// 使用DSL风格构建查询
int main() {
    std::string sql = QueryBuilder()
        .select("name, email, age")
        .from("users")
        .where("age > 18 AND status = 'active'")
        .orderBy("name")
        .limit(10)
        .build();

    printf("SQL: %s\n", sql.c_str());
    // 输出: SELECT name, email, age FROM users WHERE age > 18 AND status = 'active' ORDER BY name LIMIT 10

    return 0;
}
```

### 3. 表达式模板

```cpp
// 表达式模板——零开销的向量运算DSL
#include <vector>
#include <cstdio>

// 表达式模板的核心：延迟计算，消除临时对象

// 向量类
template<typename T>
class Vec {
private:
    std::vector<T> data;

public:
    Vec(int n) : data(n) {}
    Vec(std::initializer_list<T> init) : data(init) {}

    T& operator[](int i) { return data[i]; }
    const T& operator[](int i) const { return data[i]; }
    int size() const { return data.size(); }

    // 从表达式模板赋值
    template<typename Expr>
    Vec& operator=(const Expr& expr) {
        for (int i = 0; i < size(); i++)
            data[i] = expr[i];
        return *this;
    }
};

// 表达式模板：加法
template<typename L, typename R>
class VecAdd {
private:
    const L& left;
    const R& right;

public:
    VecAdd(const L& l, const R& r) : left(l), right(r) {}

    double operator[](int i) const { return left[i] + right[i]; }
    int size() const { return left.size(); }
};

// 表达式模板：乘法（标量×向量）
template<typename T>
class VecScalarMul {
private:
    T scalar;
    const Vec<T>& vec;

public:
    VecScalarMul(T s, const Vec<T>& v) : scalar(s), vec(v) {}

    T operator[](int i) const { return scalar * vec[i]; }
    int size() const { return vec.size(); }
};

// 运算符重载
template<typename T>
VecAdd<Vec<T>, Vec<T>> operator+(const Vec<T>& l, const Vec<T>& r) {
    return VecAdd<Vec<T>, Vec<T>>(l, r);
}

template<typename L, typename R>
VecAdd<L, R> operator+(const L& l, const R& r) {
    return VecAdd<L, R>(l, r);
}

template<typename T>
VecScalarMul<T> operator*(T scalar, const Vec<T>& v) {
    return VecScalarMul<T>(scalar, v);
}

// 使用
int main() {
    Vec<double> a = {1.0, 2.0, 3.0};
    Vec<double> b = {4.0, 5.0, 6.0};
    Vec<double> c(3);

    // DSL风格：数学表达式
    // 没有临时Vec对象，一次循环完成
    c = a + b;              // 等价于: c[i] = a[i] + b[i]
    c = 2.0 * a + b;       // 等价于: c[i] = 2*a[i] + b[i]

    for (int i = 0; i < c.size(); i++)
        printf("c[%d] = %.1f\n", i, c[i]);

    return 0;
}
```

### 4. 模板元编程DSL

```cpp
// 类型列表DSL——编译期操作类型集合
#include <type_traits>
#include <cstdio>

// 类型列表
template<typename... Ts>
struct TypeList {};

// 获取长度
template<typename List>
struct Length;

template<typename... Ts>
struct Length<TypeList<Ts...>> {
    static constexpr int value = sizeof...(Ts);
};

// 头部元素
template<typename List>
struct Head;

template<typename T, typename... Ts>
struct Head<TypeList<T, Ts...>> {
    using type = T;
};

// 尾部列表
template<typename List>
struct Tail;

template<typename T, typename... Ts>
struct Tail<TypeList<T, Ts...>> {
    using type = TypeList<Ts...>;
};

// 追加元素
template<typename List, typename T>
struct Append;

template<typename... Ts, typename T>
struct Append<TypeList<Ts...>, T> {
    using type = TypeList<Ts..., T>;
};

// 类型过滤
template<typename List, template<typename> class Pred>
struct Filter;

template<template<typename> class Pred>
struct Filter<TypeList<>, Pred> {
    using type = TypeList<>;
};

template<typename T, typename... Ts, template<typename> class Pred>
struct Filter<TypeList<T, Ts...>, Pred> {
    using rest = typename Filter<TypeList<Ts...>, Pred>::type;
    using type = std::conditional_t<
        Pred<T>::value,
        typename Append<rest, T>::type,
        rest
    >;
};

// 使用DSL
int main() {
    using MyTypes = TypeList<int, float, double, char, long>;

    // 获取长度
    printf("类型数量: %d\n", Length<MyTypes>::value);  // 5

    // 获取头部
    using First = Head<MyTypes>::type;  // int
    printf("第一个类型是int: %s\n",
           std::is_same<First, int>::value ? "是" : "否");

    // 过滤：只保留浮点类型
    using Floats = Filter<MyTypes, std::is_floating_point>::type;
    printf("浮点类型数量: %d\n", Length<Floats>::value);  // 2

    return 0;
}
```

### 5. C++20 Concepts DSL

```cpp
// 使用Concepts定义约束DSL
#include <concepts>
#include <vector>
#include <cstdio>

// 定义概念——DSL的"词汇"
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
};

template<typename T>
concept Printable = requires(T t) {
    { printf("%d", t) };
};

template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

// 使用概念约束模板——DSL的"语法"
template<Addable T>
T sum(const std::vector<T>& vec) {
    T s = T{};
    for (const auto& v : vec) s = s + v;
    return s;
}

template<Numeric T>
T average(const std::vector<T>& vec) {
    T s = T{};
    for (const auto& v : vec) s += v;
    return s / static_cast<T>(vec.size());
}

// 组合概念
template<typename T>
concept Sortable = requires(std::vector<T>& v) {
    { std::sort(v.begin(), v.end()) };
};

int main() {
    std::vector<int> nums = {1, 2, 3, 4, 5};
    printf("sum = %d\n", sum(nums));       // OK: int满足Addable
    printf("avg = %d\n", average(nums));   // OK: int满足Numeric

    // std::vector<std::string> strs = {"a", "b"};
    // average(strs);  // 编译错误: string不满足Numeric

    return 0;
}
```

***

## 4. DSL的编译与执行

### 1. DSL的实现策略

```
DSL的编译与执行方式：

1. 解释执行
   └── 逐行/逐表达式解释执行
   └── 适合简单的DSL（正则、SQL）
   └── 优点：实现简单
   └── 缺点：性能较低

2. 编译为宿主语言
   └── 将DSL翻译为C/C++/Java代码
   └── 然后用宿主语言编译器编译
   └── 优点：复用宿主语言优化器
   └── 缺点：编译速度慢

3. 编译为IR
   └── 将DSL翻译为LLVM IR
   └── 然后用LLVM优化和代码生成
   └── 优点：高性能
   └── 缺点：实现复杂

4. 编译为字节码 + JIT
   └── 先编译为字节码
   └── 运行时JIT编译为机器码
   └── 优点：启动快+运行快
   └── 缺点：内存开销大
```

### 2. 将DSL编译为LLVM IR

```cpp
// 简单计算器DSL → LLVM IR
// 支持: 整数、加减乘除、变量赋值、if-else

#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/Support/raw_ostream.h"

using namespace llvm;

class CalculatorDSLCompiler {
private:
    LLVMContext context;
    std::unique_ptr<Module> module;
    IRBuilder<> builder;

public:
    CalculatorDSLCompiler()
        : module(std::make_unique<Module>("calc_dsl", context)),
          builder(context) {}

    // 编译表达式
    Value* compileExpr(const std::string& expr) {
        // 简化：直接生成IR
        // 实际中需要解析DSL语法

        // 创建函数
        FunctionType* ft = FunctionType::get(
            Type::getInt32Ty(context), false);
        Function* func = Function::Create(
            ft, Function::ExternalLinkage, "calc", module.get());

        BasicBlock* entry = BasicBlock::Create(context, "entry", func);
        builder.SetInsertPoint(entry);

        // 假设表达式是 "3 + 4 * 2"
        Value* four = ConstantInt::get(Type::getInt32Ty(context), 4);
        Value* two = ConstantInt::get(Type::getInt32Ty(context), 2);
        Value* three = ConstantInt::get(Type::getInt32Ty(context), 3);

        Value* mul = builder.CreateMul(four, two, "mul");     // 4 * 2
        Value* add = builder.CreateAdd(three, mul, "add");    // 3 + (4*2)
        builder.CreateRet(add);

        return add;
    }

    void printIR() {
        module->print(outs(), nullptr);
    }
};
```

### 3. DSL性能优化策略

```
DSL的优化层次：

1. DSL层优化 — 在DSL语义层面优化
   └── SQL: 查询重写、子查询消除
   └── 正则: 自动机最小化、NFA→DFA

2. IR层优化 — 在中间表示层面优化
   └── 常量折叠、死代码消除
   └── 循环优化、内联

3. 机器码层优化 — 在目标代码层面优化
   └── 指令选择、寄存器分配
   └── 向量化、流水线优化

关键优化技术：
  ├── 特化（Specialization）— 为特定参数值生成代码
  ├── 融合（Fusion）— 合并多个操作为一个
  ├── 向量化（Vectorization）— SIMD并行执行
  └── 懒求值（Lazy Evaluation）— 只在需要时计算
```

***

## 5. 真实世界DSL案例

### 1. Halide——图像处理DSL

```
Halide: 图像处理领域的DSL

// Halide代码（伪代码）
Func blur_x, blur_y;
Var x, y;

// 水平模糊
blur_x(x, y) = (input(x-1, y) + input(x, y) + input(x+1, y)) / 3;

// 垂直模糊
blur_y(x, y) = (blur_x(x, y-1) + blur_x(x, y) + blur_x(x, y+1)) / 3;

// 调度策略（与算法分离）
blur_y.split(y, y, yi, 8).parallel(y).vectorize(x, 8);

Halide的核心创新：
  └── 算法与调度分离
  └── 同一个算法可以有不同的调度策略
  └── 自动向量化、并行化
  └── 比手写SSE/AVX代码更快
```

### 2. TensorFlow——深度学习DSL

```
TensorFlow: 深度学习计算图DSL

# 构建计算图
x = tf.placeholder(tf.float32, [None, 784])
W = tf.Variable(tf.zeros([784, 10]))
b = tf.Variable(tf.zeros([10]))
y = tf.matmul(x, W) + b

# 损失函数
loss = tf.reduce_mean(
    tf.nn.softmax_cross_entropy_with_logits(labels=y_, logits=y))

# 优化器
train_step = tf.train.GradientDescentOptimizer(0.01).minimize(loss)

TensorFlow的DSL特性：
  ├── 声明式计算图 — 先定义，后执行
  ├── 自动微分 — 自动计算梯度
  ├── 分布式执行 — 图可以分布在多个设备
  └── 多语言前端 — Python/C++/Java
```

***

## 6. 极简总结

| 概念 | 一句话 |
|------|--------|
| DSL | 领域特定语言——为特定领域量身定制 |
| 外部DSL | 独立语法——SQL/正则/HTML |
| 嵌入式DSL | 宿主语言内——iostream/Builder |
| 运算符重载 | C++ DSL基础——让代码像数学公式 |
| 表达式模板 | 零开销DSL——消除临时对象 |
| Builder模式 | 流式API——链式调用构建复杂对象 |
| Concepts | 约束DSL——声明式类型约束 |
| 算法与调度分离 | Halide理念——同一算法不同优化策略 |

**DSL = 用领域语言表达领域问题，C++通过运算符重载、表达式模板、Builder模式等实现嵌入式DSL，外部DSL则需要独立的解析器和编译器。**

***

### 相关阅读

- [编译器是如何工作的](./00-编译器是如何工作的.md)
- [什么是LLVM IR](./01-什么是LLVM-IR.md)
- [什么是JIT编译](./04-什么是JIT编译.md)
- [如何编写Clang插件](./02-如何编写Clang插件.md)