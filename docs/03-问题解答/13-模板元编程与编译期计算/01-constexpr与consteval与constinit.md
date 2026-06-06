# constexpr 与 consteval 与 constinit
> 📖 相关章节：[模板进阶](../../02-CPP/11-模板进阶.md)、[编译期计算基础](../../07-模板元编程与编译期计算/00-编译期计算基础.md)、[Type Traits](../../07-模板元编程与编译期计算/01-Type-Traits与类型操作.md)

## 核心提炼

**constexpr 是"尽量在编译期算，不行就运行期兜底"的温和派，consteval 是"必须在编译期算，否则直接报错"的强制派，constinit 则是"保证编译期初始化但运行期也能用"的安全派——三者分工明确，共同解决 C++ 常量表达式的各种痛点。**

---

## 1. 三者全景对比

```
┌─────────────────────────────────────────────────────────────────────┐
│                    C++ 常量表达式关键字家族                           │
├──────────────┬──────────────┬──────────────┬────────────────────────┤
│   关键字     │   constexpr  │  consteval   │      constinit        │
├──────────────┼──────────────┼──────────────┼────────────────────────┤
│ 引入版本     │    C++11     │    C++20     │       C++20           │
│ 适用对象     │ 函数/变量    │    函数      │       变量            │
│ 核心语义     │ 可选编译期   │ 强制编译期   │ 保证编译期初始化       │
│ 运行期回退   │     ✅ 允许  │    ❌ 禁止   │  ✅ 值可在运行期使用  │
│ 别名         │              │ "立即函数"   │                       │
└──────────────┴──────────────┴──────────────┴────────────────────────┘
```

---

## 2. constexpr：灵活的双面手

### 2.1 constexpr 函数的双重身份

```cpp
#include <iostream>
#include <array>

// constexpr 函数：编译期和运行期都能工作的"双面间谍"
constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

void demonstrate_constexpr_dual_nature() {
    // === 场景 1：编译期调用（参数是编译期常量）===
    constexpr int compile_time_result = factorial(5);  // 编译器在编译期算出 120
    std::array<int, factorial(6)> arr;                 // 用作数组大小，必须是编译期值
    static_assert(factorial(4) == 24);                // static_assert 要求编译期常量

    // === 场景 2：运行期调用（参数来自运行期输入）===
    int user_input = 7;
    int runtime_result = factorial(user_input);  // ✅ 完全合法！退化为普通函数调用
    std::cout << "factorial(" << user_input << ") = "
              << runtime_result << '\n';

    // 同一个函数，两种用法，这就是 constexpr 的精髓
}
```

### 2.2 constexpr 变量的特点

```cpp
// constexpr 变量：声明即初始化，且必须用常量表达式初始化
constexpr int MAX_BUFFER = 4096;
constexpr double PI = 3.14159265358979323846;
constexpr const char* APP_NAME = "MyApp";

// constexpr 变量的隐式 const 属性
void constexpr_variable_properties() {
    // MAX_BUFFER = 5000;  // ❌ 错误！constexpr 变量隐含 const
                         // 不能修改

    // constexpr 变量的地址可以取（但它本身是只读的）
    const int* ptr = &MAX_BUFFER;  // ✅ 取地址是可以的
    // *ptr = 123;                  // ❌ 但通过指针修改仍然不行

    // constexpr 变量的生命周期
    static constexpr int GLOBAL_CONST = 42;   // 静态存储期
    constexpr int LOCAL_CONST = 100;          // 但注意：局部 constexpr 变量
                                              // 并不意味着它有静态存储期
}
```

### 2.3 constexpr 函数的限制演变

```cpp
// ========== C++11：非常严格的 constexpr ==========
namespace cpp11 {
    constexpr int old_factorial(int n) {
        // ❌ 不能有局部变量
        // int result = 1;

        // ❌ 不能有循环语句
        // for (...) { }

        // ❌ 不能有 if 语句（除非是条件运算符）
        // if (n < 0) return 0;

        // 只能用递归 + 条件运算符
        return (n <= 1) ? 1 : n * old_factorial(n - 1);
    }
}

// ========== C++14：大幅放宽 ==========
namespace cpp14 {
    constexpr int modern_factorial(int n) {
        int result = 1;                    // ✅ 局部变量
        for (int i = 2; i <= n; ++i) {     // ✅ for 循环
            result *= i;
        }
        if (result < 0) {                  // ✅ if 语句
            result = 0;                    // ✅ 修改局部变量
        }
        return result;
    }

    constexpr int gcd(int a, int b) {
        while (b != 0) {                   // ✅ while 循环
            int temp = b;
            b = a % b;
            a = temp;
        }
        return a;
    }
}

// ========== C++17：if constexpr ==========
namespace cpp17 {
    template<typename T>
    constexpr auto get_default_value() {
        if constexpr (std::is_integral_v<T>) {
            return T{0};       // 整型返回 0
        } else if constexpr (std::is_floating_point_v<T>) {
            return T{0.0};     // 浮点返回 0.0
        } else {
            return T{};        // 其他类型默认构造
        }
        // 未被选中的分支不会被实例化！避免编译错误
    }
}

// ========== C++20：进一步扩展 ==========
namespace cpp20 {
    // 允许在 constexpr 中进行更多的操作
    #include <vector>
    #include <memory>

    constexpr std::vector<int> make_vector() {
        std::vector<int> v;           // ✅ C++20 允许使用部分标准容器
        v.push_back(1);
        v.push_back(2);
        v.push_back(3);
        return v;
    }

    // C++20: union 的活跃成员可以更改
    constexpr int test_union() {
        union U { int a; double b; };
        U u{};
        u.a = 42;
        u.b = 3.14;  // ✅ C++20 允许切换 union 成员
        return static_cast<int>(u.b);
    }
}
```

---

## 3. consteval：强制编译期的"立即函数"

### 3.1 为什么需要 consteval？

```cpp
#include <iostream>

// 问题场景：有些函数"只应该在编译期被调用"
// 如果你意外地在运行期调用了它，说明你的用法有 bug

constexpr int compute_table_size(int base) {
    // 这个函数的设计意图是：只在编译期用来计算数组大小
    // 如果有人在运行期调用它，可能是误用
    return base * base * 2 + 16;
}

// 危险：有人可能在运行期不小心用了它
void dangerous_usage() {
    int user_input = 10;
    int size = compute_table_size(user_input);  // ⚠️ 编译通过！但可能不是预期用法
                                                // 如果 compute_table_size 内部有 bug，
                                                // 运行期才会暴露
}

// 解决方案：用 consteval 明确禁止运行期调用
consteval int strict_table_size(int base) {
    return base * base * 2 + 16;
}

void safe_usage() {
    constexpr int size1 = strict_table_size(5);   // ✅ 编译期调用，OK
    // int size2 = strict_table_size(5);          // ❌ 编译错误！
                                                  // "立即函数不能在常量表达式之外调用"
}
```

### 3.2 consteval 的典型应用场景

```cpp
// 场景 1：编译期字符串哈希（绝对不应该在运行期重复计算）
consteval uint32_t compile_time_hash(const char* str) {
    uint32_t hash = 5381;
    while (*str) {
        hash = hash * 33 + static_cast<uint32_t>(*str++);
    }
    return hash;
}

// 使用编译期哈希优化字符串分发
void dispatch(const char* command) {
    switch (compile_time_hash(command)) {  // ⚠️ 这里 command 可能是运行期值！
                                           // 所以 compile_time_hash 应该是 consteval
        case compile_time_hash("start"):
            std::cout << "启动服务\n"; break;
        case compile_time_hash("stop"):
            std::cout << "停止服务\n"; break;
        default:
            std::cout << "未知命令\n"; break;
    }
}

// 但上面的写法有问题：command 是运行期值，不能传给 consteval 函数！
// 正确的做法是用一层包装：

constexpr uint32_t runtime_safe_hash(const char* str) {
    return compile_time_hash(str);  // 如果 str 是编译期常量，走 consteval 路径
                                    // 如果不是...哦等等，这也不行
}

// 实际上正确的做法：
consteval uint32_t ct_hash(const char* str) {
    uint32_t h = 5381;
    while (*str) h = h * 33 + static_cast<uint32_t>(*str++);
    return h;
}

// 预先计算好所有可能的哈希值
constexpr uint32_t HASH_START = ct_hash("start");
constexpr uint32_t HASH_STOP  = ct_hash("stop");

void correct_dispatch(const char* command) {
    // 运行期计算命令的哈希，与编译期预计算的值比较
    uint32_t cmd_hash = /* 运行期哈希函数 */(command);  // 需要 runtime 版本
    // ... 比较 ...
}

// 场景 2：类型特征的编译期查询
template<typename T>
consteval const char* type_name() {
    if constexpr (std::is_same_v<T, int>) return "int";
    else if constexpr (std::is_same_v<T, double>) return "double";
    else if constexpr (std::is_same_v<T, std::string>) return "std::string";
    else return "unknown";
}

// 场景 3：编译期 API 版本检查
consteval bool api_version_supported(int major, int minor) {
    return (major == 2 && minor >= 0) || major > 2;
}

static_assert(api_version_supported(2, 1));  // ✅
// static_assert(api_version_supported(1, 0)); // ❌ 编译期就拒绝旧版 API
```

### 3.3 consteval 与立即函数的特殊规则

```cpp
// consteval 函数（立即函数）的一些特殊性质

// 规则 1：consteval 函数隐式 inline
consteval int implicit_inline_func(int x) {
    return x * x;
}
// 不需要在类外再定义，不需要 inline 关键字

// 规则 2：consteval 函数可以是模板
template<typename T>
consteval T identity(T value) {
    return value;
}

// 规则 3：consteval 函数内可以调用 constexpr 函数
constexpr int regular_constexpr(int x) {
    return x + 1;
}

consteval int calling_constexpr(int x) {
    return regular_constexpr(x) + 10;  // ✅ consteval 内部调用 constexpr
}

// 规则 4：consteval 函数之间可以互相调用
consteval int helper(int x) {
    return x * 2;
}

consteval int composed(int x) {
    return helper(helper(x));  // ✅ consteval 调用 consteval
}

// 规则 5：consteval 函数不能取地址（因为不存在运行期实体）
void address_rules() {
    // auto fn_ptr = &identity<int>;  // ❌ 不能取 consteval 函数的地址
                                       // 因为它在运行期根本不存在
}
```

---

## 4. constinit：解决静态初始化顺序灾难

### 4.1 经典的静态初始化顺序问题（Static Initialization Order Fiasco）

```cpp
// 文件 A.cpp
extern int global_config_value;

class Logger {
public:
    Logger() {
        // 构造函数中使用了另一个翻译单元的全局变量
        if (global_config_value > 0) {
            // 根据 config 初始化日志级别
        }
    }
};

Logger global_logger;  // 全局对象，静态初始化阶段创建

// 文件 B.cpp
int global_config_value = 42;  // 另一个全局变量

// 问题：global_logger 和 global_config_value 谁先初始化？
// C++ 标准：同一翻译单元内按定义顺序，不同翻译单元之间 **未定义**！
//
// 如果 global_logger 先于 global_config_value 初始化：
//   global_config_value 此时还是 0（静态初始化的零初始化）
//   Logger 的构造函数看到错误的 config 值！
// 这是经典的"静态初始化顺序灾难"
```

### 4.2 constinit 如何解决这个问题

```cpp
// constinit 承诺：这个变量一定在编译期（或常量初始化阶段）完成初始化
// 从而避开动态初始化的顺序问题

// 文件 A.cpp（修复版）
constinit int global_config_value = 42;  // constinit 保证编译期初始化

class SafeLogger {
public:
    SafeLogger() {
        // 现在 global_config_value 一定已经正确初始化了
        // 因为它是常量初始化，在任何动态初始化之前完成
        if (global_config_value > 0) {
            // 安全地使用
        }
    }
};

SafeLogger safe_global_logger;  // 现在安全了

// constinit 的关键保证：
// 1. 变量具有静态或线程存储期
// 2. 初始化必须是常量初始化（constant initialization）
// 3. 如果编译器无法在编译期完成初始化，**编译报错**
```

### 4.3 constinit 的完整示例

```cpp
#include <string_view>
#include <cstring>

// constinit 用于基本类型
constinit int max_connections = 100;
constinit double pi_approximation = 3.14;
constinit bool debug_mode = false;

// constinit 用于指针（指向字符串字面量）
constinit const char* application_name = "SuperApp";

// constinit 用于结构体（聚合初始化，所有成员都是常量初始化）
struct Config {
    int max_threads;
    int buffer_size;
    bool enable_cache;
};

constinit Config app_config = {
    .max_threads = 8,
    .buffer_size = 4096,
    .enable_cache = true
};

// constinit 用于引用
constinit int& config_ref = max_connections;

// ❌ constinit 不能用于以下情况：
// void bad_constinit_examples() {
//     constinit int runtime_val = get_runtime_value();  // ❌ 不是常量初始化
//     constinit std::string s = "hello";               // ❌ std::string 构造不是常量初始化
//     constinit int local = 42;                        // ❌ 没有静态/线程存储期
// }

// constinit vs constexpr 的关键区别
void constinit_vs_constexpr() {
    // constexpr 变量隐含 const，不能修改，也不能取地址做修改
    constexpr int CE_VAR = 100;
    // CE_VAR = 200;      // ❌ 不能修改

    // constinit 变量不隐含 const！可以修改！
    constinit int CI_VAR = 100;
    CI_VAR = 200;         // ✅ 可以修改！constinit 只保证初始化时机

    // constinit 的主要用途就是那些需要是全局可变状态，
    // 但又需要安全初始化的场景
}

// 实战案例：跨模块共享的可变配置
namespace shared_config {
    constinit int log_level = 1;       // 默认 INFO 级别
    constinit bool verbose = false;    // 默认非详细模式

    // 这些可以在运行时被修改
    void set_log_level(int level) { log_level = level; }
    void set_verbose(bool v) { verbose = v; }
}
```

### 4.4 constinit 与线程安全

```cpp
// constinit 也适用于线程局部存储
constinit thread_local int thread_id_counter = 0;

// 这保证了每个线程的 thread_id_counter 都在编译期初始化为 0
// 避免了线程首次访问时的竞争条件

// 对比：没有 constinit 的潜在问题
thread_local int unsafe_counter = 0;  // 可能涉及动态初始化
                                      // 多个线程首次访问时可能有竞态

// constinit + constexpr 组合使用
constinit constexpr int ABSOLUTE_CONSTANT = 42;
// constexpr 已经隐含了 constinit 的保证
// 但显式写出 constinit 可以强调意图
```

---

## 5. 三者的协同使用

### 5.1 典型的分层架构

```cpp
// 第一层：consteval —— 纯编译期工具函数
consteval uint32_t static_string_hash(const char* s) {
    uint32_t h = 2166136261u;
    while (*s) {
        h ^= static_cast<uint32_t>(*s++);
        h *= 16777619u;
    }
    return h;
}

// 第二层：constexpr —— 既可编译期也可运行期的通用函数
constexpr bool is_power_of_two(uint32_t n) {
    return n > 0 && (n & (n - 1)) == 0;
}

constexpr uint32_t next_power_of_two(uint32_t n) {
    if (is_power_of_two(n)) return n;
    n--;
    n |= n >> 1;
    n |= n >> 2;
    n |= n >> 4;
    n |= n >> 8;
    n |= n >> 16;
    return n + 1;
}

// 第三层：constinit —— 全局状态的安全初始化
constinit uint32_t DEFAULT_CAPACITY = static_string_hash("default");
constinit uint32_t INITIAL_HASH_SEED = 2166136261u;
```

### 5.2 实战综合示例

```cpp
#include <array>
#include <string_view>
#include <iostream>

// 一个完整的编译期友好的配置系统

namespace compiletime_config {

// ====== consteval: 编译期专用的工具函数 ======

consteval std::size_t string_length(const char* s) {
    std::size_t len = 0;
    while (s[len]) ++len;
    return len;
}

consteval bool is_valid_identifier(const char* name) {
    if (!name || !(*name)) return false;
    if (!((*name >= 'a' && *name <= 'z') ||
          (*name >= 'A' && *name <= 'Z') || *name == '_')) return false;
    for (std::size_t i = 1; name[i]; ++i) {
        char c = name[i];
        if (!((c >= 'a' && c <= 'z') ||
              (c >= 'A' && c <= 'Z') ||
              (c >= '0' && c <= '9') || c == '_'))
            return false;
    }
    return true;
}

// ====== constexpr: 通用的配置计算函数 ======

constexpr int align_up(int value, int alignment) {
    return (value + alignment - 1) & ~(alignment - 1);
}

constexpr int calculate_buffer_size(int min_size, int alignment) {
    int size = align_up(min_size, alignment);
    // 至少 64 字节
    return size < 64 ? 64 : size;
}

// ====== constinit: 全局配置变量的安全初始化 ======

constinit const int MIN_ALIGNMENT = 16;
constinit const int DEFAULT_MIN_BUFFER = 128;
constinit const int COMPUTED_BUFFER_SIZE =
    calculate_buffer_size(DEFAULT_MIN_BUFFER, MIN_ALIGNMENT);

// 编译期验证配置有效性
static_assert(is_valid_identifier("my_config"), "配置名不合法");
static_assert(COMPUTED_BUFFER_SIZE >= 64, "缓冲区太小");
static_assert(COMPUTED_BUFFER_SIZE % MIN_ALIGNMENT == 0, "未对齐");

} // namespace compiletime_config

// 使用示例
void demo_config_system() {
    using namespace compiletime_config;

    std::cout << "计算得到的缓冲区大小: " << COMPUTED_BUFFER_SIZE << "\n";
    // 所有配置在编译期就已验证并确定
}
```

---

## 6. 常见误区与陷阱

### 误区 1：认为 constexpr 函数总是会在编译期执行

```cpp
constexpr int maybe_compiletime(int x) {
    return x * x + 2 * x + 1;
}

void misconception_1() {
    int a = 5;
    int b = maybe_compiletime(a);  // ⚠️ 这是运行期调用！a 不是编译期常量

    // 要强制编译期计算：
    constexpr int c = maybe_compiletime(5);  // ✅ 这次才是编译期

    // 或者用 consteval：
    // consteval int forced_ct(int x) { return x * x; }
    // constexpr int d = forced_ct(a);  // ❌ 编译错误！a 不是常量
}
```

### 误区 2：认为 constinit 就是 constexpr 的别名

```cpp
void misconception_2() {
    constexpr int ce = 10;
    // ce = 20;  // ❌ constexpr 隐含 const

    constinit int ci = 10;
    ci = 20;     // ✅ constinit 不隐含 const！这是关键区别

    // constinit 的唯一承诺是"初始化发生在编译期/常量初始化阶段"
    // 它不承诺值的不可变性
}
```

### 误区 3：忽略 consteval 的参数要求

```cpp
consteval int require_constant(int x) {
    return x * x;
}

void misconception_3() {
    constexpr int a = require_constant(5);    // ✅ 5 是常量

    int b = 10;
    // int c = require_constant(b);           // ❌ b 不是常量表达式！
                                             // 即使 b 的值在运行时是确定的
                                             // consteval 也不接受
}
```

### 误区 4：在应该用 constinit 的地方用了 constexpr 导致的问题

```cpp
// 场景：需要一个全局计数器，初始值为 0，但在运行时会增加

// 错误做法：
// constexpr int global_counter = 0;  // ❌ constexpr 隐含 const
// void increment() { global_counter++; }  // ❌ 无法修改

// 正确做法：
constinit int global_counter = 0;  // ✅ 保证初始化安全，同时允许修改
void increment() { global_counter++; }  // ✅ 可以修改
```

### 误区 5：混淆"编译期可求值"和"实际在编译期求值"

```cpp
constexpr int expensive computation(int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            ++sum;
    return sum;
}

void misconception_5() {
    // 下面两行的语义不同：
    int a = expensive_computation(1000);  // 运行期计算，编译快
    constexpr int b = expensive_computation(1000);  // 编译期计算，编译慢！

    // 即使 constexpr 函数有能力在编译期计算，
    // 只有当你真正需要编译期常量时它才会在编译期执行
}
```

---

## 7. 总结速查表

| 特性 | constexpr | consteval | constinit |
|------|-----------|-----------|-----------|
| **引入版本** | C++11 | C++20 | C++20 |
| **适用目标** | 函数、变量 | 函数（立即函数） | 变量 |
| **编译期求值** | 尽力而为 | **强制要求** | 初始化阶段强制 |
| **运行期回退** | ✅ 允许 | ❌ 编译错误 | ✅ 值可运行期使用 |
| **隐含 const** | 对于变量 ✅ | N/A | ❌ 不隐含 |
| **可取地址** | ✅ | ❌ | ✅ |
| **主要用途** | 通用编译期计算 | 确保编译期执行 | 安全的全局初始化 |
| **典型场景** | 数学函数、简单算法 | 哈希、反射工具 | 全局配置、单例状态 |

**口诀记忆**：
- **constexpr** = con**stant ex**pression（常量表达式，灵活双模）
- **consteval** = con**stant ev**aluation（常量求值，强制编译）
- **constinit** = con**stant init**ialization（常量初始化，安全起步）