# 什么是开销（Overhead）
> 📖 相关章节：[运算符与表达式](../../01-C语言/02-运算符与表达式.md)、[控制结构](../../01-C语言/03-控制结构.md)、[函数](../../01-C语言/04-函数.md)、[基础特性](../../02-CPP/01-基础特性.md)、[命名空间](../../02-CPP/02-命名空间与编码规范.md)

### 1. 核心定义

**开销** = **执行某个操作时的额外资源消耗**（时间、内存、CPU等）

这些资源不是直接用于实现核心功能，而是为了支持该功能而产生的"附带消耗"。

```cpp
// 核心功能：两个数相加
int add(int a, int b) { return a + b; }

// 开销 = 调用这个函数所付出的额外成本
// - 参数压栈/传参
// - 创建栈帧
// - 保存/恢复寄存器
// - 跳转指令（call/ret）
// 这些都不是"加法"本身需要的，都是开销
```

### 2. 开销的三种类型

| 类型 | 说明 | 典型场景 |
|------|------|---------|
| **时间开销** | 执行耗时 | 函数调用、内存分配、锁竞争 |
| **空间开销** | 内存占用 | 虚函数表、栈帧、对齐填充 |
| **CPU开销** | 指令周期 | 上下文切换、缓存未命中、分支预测失败 |

### 3. 常见开销对比

| 操作 | 开销 | 相对成本 |
|------|------|:--------:|
| 普通函数调用 | 保存返回地址、创建栈帧、参数传递 | ~1x |
| 虚函数调用 | 通过虚函数表查找函数地址（多一次间接寻址） | ~2-3x |
| 动态内存分配 (`malloc/free`) | 查找可用内存块、更新内存管理数据结构 | ~100x |
| 系统调用 | 用户态↔内核态切换 | ~1000x |
| 线程上下文切换 | 保存/恢复寄存器、TLB刷新 | ~10000x |

### 4. 量化示例：函数调用开销

```cpp
#include <iostream>
#include <chrono>

// 直接计算（内联）
volatile int result1;

// 通过函数调用
int add(int a, int b) { return a + b; }
volatile int result2;

int main() {
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i) {
        result1 = i + i;  // 直接计算
    }
    auto mid = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 100000000; ++i) {
        result2 = add(i, i);  // 函数调用
    }
    auto end = std::chrono::high_resolution_clock::now();

    auto direct = std::chrono::duration_cast<std::chrono::milliseconds>(mid - start).count();
    auto called = std::chrono::duration_cast<std::chrono::milliseconds>(end - mid).count();
    std::cout << "直接计算: " << direct << "ms\n";
    std::cout << "函数调用: " << called << "ms\n";  // 通常比直接计算慢
    return 0;
}
```

### 5. 开销 vs 收益

开销本身不是坏事，关键看**收益是否大于开销**：

```
函数调用开销 → 换来代码复用、模块化
虚函数开销   → 换来多态、代码扩展性
动态内存分配  → 换来灵活性、动态大小
异常处理     → 换来错误隔离、健壮性
```

**核心原则：不要过早优化，但要了解开销在哪里。**

### 6. 零开销抽象（Zero-cost Abstraction）

C++ 的重要理念：**抽象不应该带来额外的运行时开销**。

示例：

- `inline` 函数消除函数调用开销
- 模板在编译期展开，无运行时开销
- RAII 自动管理资源，无额外开销
- constexpr 在编译期求值，零运行时成本

### 7. 如何识别和衡量开销

1. **性能分析（Profiling）**：用 perf / VTune 找到热点
2. **基准测试（Benchmarking）**：对比有/无某种抽象的耗时
3. **反汇编分析**：查看编译器生成的指令是否符合预期
4. **关注高频路径**：99%的开销集中在1%的代码中

### 8. 极简口诀

**开销就是「为了做一件事而额外付出的成本」— 收益大于开销就是好设计，反之就是浪费。**

---

### 9. 分支预测开销

CPU 采用流水线执行指令，遇到分支（if/switch）时会**预测**走哪条路并提前执行。预测正确则无开销，预测失败则需要**丢弃流水线中的指令并重新取指**，代价极大。

**分支预测失败的代价：15-20 个时钟周期**（约等于 50-100 条指令的执行时间）

```cpp
// 分支预测失败的场景：数据随机，预测几乎无法命中
#include <algorithm>
#include <vector>
#include <random>

void branch_random(std::vector<int>& data) {
    // data 是随机排列的，if 分支约 50% 概率命中
    // CPU 无法有效预测，频繁预测失败
    for (auto& x : data) {
        if (x >= 128) {       // 随机数据，预测成功率 ≈ 50%
            x += 1;
        }
    }
}

void branch_sorted(std::vector<int>& data) {
    // 先排序，同一分支连续命中，预测成功率极高
    std::sort(data.begin(), data.end());
    for (auto& x : data) {
        if (x >= 128) {       // 排序后，分支模式稳定，预测成功率 ≈ 100%
            x += 1;
        }
    }
}

// 无分支版本：彻底消除分支预测开销
void branch_free(std::vector<int>& data) {
    for (auto& x : data) {
        // 用位运算代替分支，无论数据如何排列都无预测失败
        int mask = (x - 128) >> 31;   // x >= 128 时 mask = 0，否则 mask = -1
        x += 1 & ~mask;               // 仅当 x >= 128 时加 1
    }
}
```

**likely / unlikely 提示**：告诉编译器哪个分支更可能执行，帮助优化代码布局

```cpp
#define likely(x)   __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)

int process(int* ptr) {
    if (unlikely(ptr == nullptr)) {   // 空指针是罕见情况，放远端
        return -1;
    }
    // 正常路径放近端，指令缓存命中更好
    return *ptr + 1;
}
```

**分支预测失败率参考数据**：

| 场景 | 预测成功率 | 说明 |
|------|:----------:|------|
| 循环条件（i < N） | > 99% | 模式极其规律 |
| 排序数据的 if | > 95% | 分支模式稳定 |
| 随机数据的 if | ≈ 50% | 等于瞎猜 |
| 间接跳转（虚函数） | 60-80% | 取决于多态类型数量 |

### 10. 缓存未命中开销

CPU 缓存是分层的，越靠近核心越小越快，越远越大越慢。**缓存未命中**是性能杀手之一。

**各级存储访问延迟对比**：

| 存储层级 | 典型延迟 | 相对 L1 的倍数 | 容量（典型值） |
|----------|:--------:|:--------------:|:--------------:|
| L1 缓存 | ~1 ns | 1x | 32-64 KB |
| L2 缓存 | ~4 ns | 4x | 256 KB-1 MB |
| L3 缓存 | ~12 ns | 12x | 4-64 MB |
| 主存（DRAM） | 40-100 ns | 40-100x | 8-128 GB |
| SSD | ~100 μs | 100,000x | 256 GB-4 TB |

**缓存行（Cache Line）= 64 字节**：CPU 不逐字节读取内存，而是以 64 字节为最小单位整行加载。

```cpp
// 缓存行对性能的巨大影响示例
#include <chrono>
#include <iostream>

struct alignas(64) PaddedInt {   // 每个变量独占一个缓存行
    int value;
    char padding[60];
};

struct CompactInt {              // 两个变量共享缓存行
    int value;
};

// 场景：两个线程各写自己的变量
// PaddedInt 版本：各写各的缓存行，无伪共享 → 快
// CompactInt 版本：两个变量在同一缓存行，互相使对方的缓存行失效 → 慢 5-10 倍

int main() {
    constexpr int N = 100'000'000;

    // 紧凑布局：两个变量在同一缓存行
    CompactInt a{}, b{};
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) { a.value += i; }
    for (int i = 0; i < N; ++i) { b.value += i; }
    auto t2 = std::chrono::high_resolution_clock::now();

    // 填充布局：每个变量独占缓存行
    PaddedInt c{}, d{};
    auto t3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) { c.value += i; }
    for (int i = 0; i < N; ++i) { d.value += i; }
    auto t4 = std::chrono::high_resolution_clock::now();

    auto compact_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    auto padded_ms  = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count();
    std::cout << "紧凑布局: " << compact_ms << "ms\n";
    std::cout << "填充布局: " << padded_ms  << "ms\n";
    return 0;
}
```

**数据局部性原则**：

| 局部性类型 | 说明 | 优化手段 |
|-----------|------|---------|
| 时间局部性 | 最近访问的数据很快会再次访问 | 循环中反复使用的变量放寄存器 |
| 空间局部性 | 相邻地址的数据很可能被连续访问 | 顺序遍历优于随机访问 |
| 对齐填充 | 结构体成员按对齐要求排列 | 热路径结构体按大小降序排列成员 |

```cpp
// 优化前：缓存不友好（随机访问）
struct Node {
    int value;
    Node* next;   // 链表节点在内存中不连续
};
// 遍历链表：几乎每次都是缓存未命中

// 优化后：缓存友好（连续存储）
std::vector<int> values;   // 数据在内存中连续排列
// 顺序遍历：第一个元素加载后，后续元素自动进入缓存行
```

### 11. 虚函数开销深入分析

虚函数的开销不仅仅是"多一次间接寻址"，它对 CPU 流水线有多重负面影响。

**vtable 间接寻址的汇编对比**：

```cpp
struct Base {
    virtual int compute(int x) { return x * 2; }
};

struct Derived : Base {
    int compute(int x) override { return x * 3; }
};

int call_virtual(Base* obj, int x) {
    return obj->compute(x);   // 虚函数调用
}

int call_direct(Derived* obj, int x) {
    return obj->compute(x);   // 直接调用（编译期确定）
}
```

对应汇编（x86-64，-O2）：

```asm
; 虚函数调用 call_virtual
mov    rax, QWORD PTR [rdi]       ; 1. 加载 vtable 指针
jmp    QWORD PTR [rax]            ; 2. 间接跳转到 vtable 中的函数地址
                                 ;    → CPU 无法预测跳转目标，分支预测失败率高

; 直接调用 call_direct
imul   esi, edi, 3                ; 直接内联为 x * 3，无任何间接跳转
ret
```

**虚函数对分支预测的影响**：

```cpp
// 虚函数调用的间接跳转是 CPU 分支预测的噩梦
// 条件分支（if）只有两条路，预测成功率很高
// 间接跳转（虚函数）可能有 N 个目标，预测难度随 N 增大而急剧上升

void process_many(std::vector<Base*>& objects) {
    for (auto* obj : objects) {
        obj->compute(42);   // 每次调用都可能跳到不同的函数
                            // 如果对象类型混杂，间接分支预测失败率可达 20-40%
    }
}
```

**CRTP 静态多态：零开销的替代方案**

```cpp
// 传统虚函数多态（运行时决议，有开销）
struct ShapeV {
    virtual double area() const = 0;
    virtual ~ShapeV() = default;
};
struct CircleV : ShapeV {
    double r;
    double area() const override { return 3.14159 * r * r; }
};

// CRTP 静态多态（编译期决议，零开销）
template <typename Derived>
struct ShapeS {
    double area() const {
        return static_cast<const Derived*>(this)->area_impl();
    }
};
struct CircleS : ShapeS<CircleS> {
    double r;
    double area_impl() const { return 3.14159 * r * r; }
};

// 对比
double compute_virtual(const ShapeV& s) { return s.area(); }   // 虚函数调用
double compute_static(const CircleS& s)  { return s.area(); }   // 完全内联，零开销
```

| 方案 | 决议时机 | 调用开销 | 灵活性 | 代码膨胀 |
|------|---------|---------|--------|---------|
| 虚函数 | 运行时 | vtable 间接跳转 | 高（可跨动态库） | 无 |
| CRTP | 编译期 | 零（完全内联） | 中（需编译期已知类型） | 有（每种类型生成一份代码） |

### 12. 异常处理开销

C++ 异常采用**零成本模型（Zero-cost Exception Model）**：

- **无异常抛出时**：try 块几乎零开销（不产生任何额外指令）
- **抛出异常时**：开销极高（需要栈展开、查找 catch 块、析构局部对象）

```cpp
#include <chrono>
#include <iostream>
#include <stdexcept>

// 方式一：错误码
int divide_code(int a, int b, int& result) {
    if (b == 0) return -1;   // 错误码
    result = a / b;
    return 0;                 // 成功
}

// 方式二：异常
int divide_exception(int a, int b) {
    if (b == 0) throw std::invalid_argument("除零");
    return a / b;
}

int main() {
    constexpr int N = 100'000'000;
    int result = 0;

    // 正常路径：错误码 vs 异常（异常几乎无额外开销）
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        divide_code(i + 1, 2, result);
    }
    auto t2 = std::chrono::high_resolution_clock::now();

    auto t3 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        try { result = divide_exception(i + 1, 2); }
        catch (...) {}
    }
    auto t4 = std::chrono::high_resolution_clock::now();

    auto code_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
    auto exc_ms  = std::chrono::duration_cast<std::chrono::milliseconds>(t4 - t3).count();
    std::cout << "错误码（正常路径）: " << code_ms << "ms\n";
    std::cout << "异常  （正常路径）: " << exc_ms  << "ms\n";
    // 正常路径下两者性能接近，异常版本几乎零额外开销

    // 异常路径：异常的开销远超错误码
    auto t5 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; ++i) {
        divide_code(1, 0, result);   // 返回错误码，极快
    }
    auto t6 = std::chrono::high_resolution_clock::now();

    auto t7 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; ++i) {
        try { divide_exception(1, 0); }
        catch (...) {}               // 栈展开 + 查找 catch，极慢
    }
    auto t8 = std::chrono::high_resolution_clock::now();

    auto err_code_ms = std::chrono::duration_cast<std::chrono::milliseconds>(t6 - t5).count();
    auto err_exc_ms  = std::chrono::duration_cast<std::chrono::milliseconds>(t8 - t7).count();
    std::cout << "错误码（异常路径）: " << err_code_ms << "ms\n";
    std::cout << "异常  （异常路径）: " << err_exc_ms  << "ms\n";
    // 异常路径下，异常比错误码慢 100-1000 倍
    return 0;
}
```

**-fno-exceptions 的影响**：

| 编译选项 | 代码体积 | 正常路径 | 异常路径 | 适用场景 |
|---------|---------|---------|---------|---------|
| 默认（启用异常） | 较大（含 .eh_frame） | 几乎零开销 | 极高开销 | 通用 C++ 项目 |
| -fno-exceptions | 较小 | 零开销 | 编译错误 | 嵌入式、游戏引擎、实时系统 |

> 注意：禁用异常后，标准库中 throw 将变为 `std::terminate`，`try/catch` 语法将无法编译。

### 13. RTTI 开销

RTTI（Run-Time Type Information，运行时类型信息）是 C++ 支持运行时类型识别的机制，主要通过 `typeid` 和 `dynamic_cast` 使用。

**type_info 的存储开销**：

```cpp
#include <typeinfo>
#include <iostream>

struct Base { virtual ~Base() = default; };
struct Derived : Base {};

int main() {
    // 每个多态类都会生成一个 type_info 对象
    // type_info 通常包含：类名（字符串）、继承关系、基类偏移表
    // 每个多态类的 type_info 约占 40-80 字节（取决于实现）
    std::cout << "Base    type_info: " << typeid(Base).name()    << "\n";
    std::cout << "Derived type_info: " << typeid(Derived).name() << "\n";
    return 0;
}
```

**dynamic_cast 的运行时开销**：

```cpp
struct Base { virtual ~Base() = default; };
struct Derived1 : Base { int data1; };
struct Derived2 : Base { int data2; };

void process(Base* ptr) {
    // dynamic_cast 需要遍历继承层次结构
    // 单继承：O(1)，只需检查一次 vtable 中的 type_info
    // 多继承：O(N)，需要遍历基类列表查找匹配类型
    // 菱形继承：更复杂，需要回溯虚基类
    auto d1 = dynamic_cast<Derived1*>(ptr);   // 运行时类型检查
    auto d2 = dynamic_cast<Derived2*>(ptr);
    if (d1) { d1->data1 = 42; }
    if (d2) { d2->data2 = 99; }
}
```

| RTTI 操作 | 开销 | 替代方案 |
|-----------|------|---------|
| `typeid` | 读取 vtable 中的 type_info 指针 | 编译期 `decltype` / `std::is_same_v` |
| `dynamic_cast`（单继承） | ~2-3 条指令 | `static_cast` + 手动类型标记 |
| `dynamic_cast`（多继承） | 可能遍历继承图 | 访问者模式 / 类型枚举 |
| `type_info` 存储 | 每个多态类 40-80 字节 | -fno-rtti 禁用 |

> 游戏引擎和嵌入式项目常使用 `-fno-rtti` 禁用 RTTI，用手动类型枚举替代 `dynamic_cast`。

### 14. 零开销抽象深入

C++ 的核心设计哲学：**你不需要为你没有使用的东西付出代价**（You don't pay for what you don't use）。

以下用完整示例证明：C++ 的高级抽象在优化后与 C 风格代码生成相同的机器码。

```cpp
// ===== C 风格 =====
int sum_c(const int* arr, int n) {
    int total = 0;
    for (int i = 0; i < n; ++i) {
        total += arr[i];
    }
    return total;
}

// ===== C++ 抽象风格 =====
#include <vector>
#include <numeric>

int sum_cpp(const std::vector<int>& v) {
    return std::accumulate(v.begin(), v.end(), 0);
}

// ===== 模板 + constexpr 风格 =====
template <typename T, int N>
struct Array {
    T data[N];

    constexpr T sum() const {
        T total = 0;
        for (int i = 0; i < N; ++i) {
            total += data[i];
        }
        return total;
    }
};

// 编译期求值，零运行时成本
constexpr Array<int, 5> arr = {{1, 2, 3, 4, 5}};
constexpr int total = arr.sum();   // 编译期已算出 = 15，运行时无任何计算
static_assert(total == 15);        // 编译期断言

// 三种风格在 -O2 下生成的汇编完全相同（核心循环都是 add + 循环）
// C++ 抽象没有引入任何额外指令
```

**constexpr 编译期求值示例**：

```cpp
#include <array>
#include <iostream>

// 编译期计算斐波那契数列
constexpr std::array<long long, 30> fibonacci() {
    std::array<long long, 30> result{};
    result[0] = 0;
    result[1] = 1;
    for (int i = 2; i < 30; ++i) {
        result[i] = result[i - 1] + result[i - 2];
    }
    return result;
}

// 编译期直接算好，运行时只是读取常量
constexpr auto fib = fibonacci();

int main() {
    std::cout << "fib(29) = " << fib[29] << "\n";   // 运行时零计算开销
    return 0;
}
```

**抽象无额外开销的证明**：

| 抽象机制 | 编译期行为 | 运行时额外开销 |
|---------|-----------|:-------------:|
| `inline` 函数 | 内联展开 | 0 |
| 模板实例化 | 为每种类型生成专用代码 | 0 |
| `constexpr` | 编译期求值 | 0 |
| RAII | 析构调用在确定位置生成 | 0 |
| `std::array` | 与 C 数组内存布局相同 | 0 |
| `std::sort` vs `qsort` | 模板内联比较函数 vs 函数指针 | `std::sort` 更快 |

> `std::sort` 比 C 的 `qsort` 更快是经典案例：`qsort` 通过函数指针调用比较函数，无法内联；`std::sort` 的比较是模板参数，编译器完全内联，省去了函数调用开销。

### 15. Profiling 工具实战

了解开销在哪里，比猜测更重要。以下是定位热点代码的实战方法。

**perf（Linux 性能分析利器）**：

```bash
# 1. 采样记录：运行程序并采集 CPU 性能数据
perf record -g ./my_program

# 2. 查看报告：按函数排序显示热点
perf report

# 3. 常用分析命令
perf stat ./my_program               # 查看缓存未命中、分支预测失败等硬件事件
perf record -e cache-misses ./my_program   # 专门采集缓存未命中事件
perf record -e branch-misses ./my_program  # 专门采集分支预测失败事件

# 4. 输出示例
# Overhead  Command    Shared Object  Symbol
#  45.23%   my_prog    my_prog        [.] process_data
#  18.67%   my_prog    my_prog        [.] compute_hash
#   8.12%   my_prog    libc.so        [.] malloc
#  ↑ 45% 的时间花在 process_data，这就是优化重点
```

**Google Benchmark（微基准测试框架）**：

```cpp
#include <benchmark/benchmark.h>
#include <vector>
#include <algorithm>

// 基准测试：排序后的分支预测
static void BM_BranchSorted(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    for (int i = 0; i < state.range(0); ++i) data[i] = i;
    for (auto _ : state) {
        int sum = 0;
        for (auto x : data) {
            if (x >= 128) sum += x;
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_BranchSorted)->Arg(10000);

// 基准测试：随机数据的分支预测
static void BM_BranchRandom(benchmark::State& state) {
    std::vector<int> data(state.range(0));
    for (int i = 0; i < state.range(0); ++i) data[i] = i;
    std::shuffle(data.begin(), data.end(), std::mt19937{42});
    for (auto _ : state) {
        int sum = 0;
        for (auto x : data) {
            if (x >= 128) sum += x;
        }
        benchmark::DoNotOptimize(sum);
    }
}
BENCHMARK(BM_BranchRandom)->Arg(10000);

BENCHMARK_MAIN();

// 编译：g++ -O2 benchmark_demo.cpp -lbenchmark -lpthread -o benchmark_demo
// 运行：./benchmark_demo
// 输出示例：
// BM_BranchSorted/10000   1200 ns   1205 ns   581234
// BM_BranchRandom/10000   2800 ns   2803 ns   249876
// ↑ 随机数据慢 2.3 倍，这就是分支预测失败的代价
```

**定位热点代码的流程**：

```
1. perf record -g ./program        ← 采集性能数据
2. perf report                     ← 找到最耗时的函数（热点）
3. 针对热点写 Google Benchmark     ← 精确测量优化前后的耗时
4. 优化代码                        ← 针对性优化
5. 重复步骤 1-4                    ← 验证优化效果
```

**常见热点模式与对策**：

| 热点特征 | 可能原因 | 对策 |
|---------|---------|------|
| 高 cache-misses | 数据布局不友好 | 重排结构体成员、使用连续容器 |
| 高 branch-misses | 分支模式随机 | 排序数据、用位运算消除分支 |
| 大量时间在 malloc/free | 频繁动态分配 | 对象池、预分配、栈上分配 |
| 虚函数调用占比高 | 多态开销 | CRTP、类型枚举 + switch |
| 大量时间在 memcpy | 不必要的拷贝 | 移动语义、引用传递、span |

***

**极简总结：开销是 C++ 程序员必须理解的底层概念 — 分支预测失败 15-20 周期、缓存未命中 40-100ns、虚函数间接跳转、异常抛出极慢 — 了解它们，才能在需要时做出正确的取舍。**

***

### 相关阅读

- [什么是缓存命中率](./04-什么是缓存命中率.md)
- [什么是零开销抽象Zero-overhead](./32-什么是零开销抽象Zero-overhead.md)
- [inline关键字的真实含义](./14-inline关键字的真实含义.md)