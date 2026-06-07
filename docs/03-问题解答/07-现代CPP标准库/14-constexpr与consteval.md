# constexpr 与 consteval 的区别
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[STL算法](../../02-CPP/15-STL算法与迭代器.md)、[正则表达式](../../02-CPP/16-正则表达式.md)、[文件操作与文件系统](../../02-CPP/18-文件操作与文件系统.md)

### 1. 核心速览

**constexpr = 可能编译期求值（也允许运行期），consteval = 必须编译期求值（不允许运行期）。constexpr 是"可提前准备也可现场做"，consteval 是"必须提前准备"。**

***

### 2. 核心定义

| | constexpr | consteval |
|---|---|---|
| 含义 | "可以在编译期求值" | "必须在编译期求值" |
| 求值时机 | 编译期或运行期（由上下文决定） | 只能编译期 |
| 运行期调用 | 允许 | 不允许（编译错误） |
| C++ 版本 | C++11 | C++20 |
| 函数类型 | constexpr 函数 | 立即函数（immediate function） |

**本质区别**：

```cpp
constexpr int square(int x) {
    return x * x;
}

consteval int cube(int x) {
    return x * x * x;
}

// constexpr：编译期和运行期都可以
int a = square(5);          // 可能编译期求值，也可能运行期
constexpr int b = square(5); // 一定编译期求值
int n = 5;
int c = square(n);          // 运行期求值（n 不是编译期常量）

// consteval：必须编译期求值
int d = cube(5);            // 编译期求值
constexpr int e = cube(5);  // 编译期求值
// int f = cube(n);         // 编译错误！n 不是编译期常量
```

***

### 3. 生活类比

| | constexpr | consteval |
|---|---|---|
| 类比 | 可提前准备也可现场做 | 必须提前准备 |
| 说明 | 做菜：有些菜可以提前备好（编译期），也可以客人点了再做（运行期） | 做菜：有些食材必须提前腌制好，客人点了才能直接上菜 |
| 关键区别 | 灵活，编译器根据上下文决定 | 严格，编译器强制编译期求值 |

**具体场景**：

- **constexpr**：你开了一家餐厅。有些菜可以提前预制（编译期），也可以现点现做（运行期）。如果客人提前预约了（constexpr 变量），你就提前做好；如果客人临时来点（普通变量），你就现场做。
- **consteval**：你开了一家餐厅。有些食材必须提前腌制 24 小时。客人来了才能上菜，但你不可能现场腌制。如果客人点了但食材没提前准备，就没法做（编译错误）。

***

### 4. C++11 constexpr → C++14 放宽 → C++20 consteval/constinit 的演进

**C++11：constexpr 函数的限制**

```cpp
// C++11 的 constexpr 函数：函数体只能有一条 return 语句
constexpr int factorial(int n) {
    return n <= 1 ? 1 : n * factorial(n - 1);  // 递归，不能写循环
}

// 不能有：
// - 局部变量
// - if/else 语句
// - 循环
// - 多条语句
```

**C++14：大幅放宽限制**

```cpp
// C++14 的 constexpr 函数：几乎和普通函数一样
constexpr int factorial(int n) {
    int result = 1;           // 局部变量 ✅
    for (int i = 2; i <= n; ++i) {  // 循环 ✅
        result *= i;
    }
    return result;
}

constexpr int abs_val(int x) {
    if (x < 0) return -x;    // if 语句 ✅
    return x;
}
```

**C++20：consteval 和 constinit**

```cpp
// consteval：强制编译期求值
consteval int square(int x) {
    return x * x;
}
int a = square(5);   // 必须编译期求值

// constinit：强制编译期初始化（但不是常量）
constinit int global = square(5);  // 编译期初始化，但 global 不是 const
global = 100;                       // 运行期可以修改

// constexpr 变量：编译期初始化 + 不可修改
constexpr int b = square(5);  // 编译期初始化，b 是 const
// b = 100;                   // 编译错误！
```

**演进时间线**：

| 版本 | 特性 | 说明 |
|------|------|------|
| C++11 | constexpr 函数 | 函数体只能一条 return |
| C++11 | constexpr 变量 | 编译期常量 |
| C++14 | 放宽 constexpr | 允许局部变量、循环、if |
| C++17 | constexpr if | 编译期条件分支 |
| C++20 | consteval | 强制编译期求值 |
| C++20 | constinit | 强制编译期初始化 |
| C++20 | constexpr 虚函数 | 允许 constexpr 虚函数 |
| C++23 | constexpr 更宽松 | 允许更多标准库函数为 constexpr |

***

### 5. constexpr 函数在编译期和运行期的行为

```cpp
constexpr int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// 情况1：编译期求值
constexpr int fib10 = fibonacci(10);  // 编译期计算，结果写入二进制
// fib10 是编译期常量，可以用在模板参数、数组大小等
int arr[fib10];                       // OK，fib10 是编译期常量
static_assert(fib10 == 55);

// 情况2：运行期求值
int n;
std::cin >> n;
int result = fibonacci(n);  // 运行期计算，n 是运行时值
// result 不是编译期常量

// 情况3：编译期求值但不是常量
int fib_at_10 = fibonacci(10);  // 可能编译期求值，但 fib_at_10 不是 constexpr
// int arr2[fib_at_10];         // 错误！fib_at_10 不是编译期常量
```

**关键规则**：constexpr 函数是否在编译期求值，取决于调用上下文：

```cpp
constexpr int add(int a, int b) { return a + b; }

// 编译期求值的条件：结果用于编译期上下文
constexpr int x = add(1, 2);        // 编译期（constexpr 变量）
template<int N> struct IntConst {};
IntConst<add(1, 2)> ic;             // 编译期（模板参数）
static_assert(add(1, 2) == 3);      // 编译期（static_assert）
int arr[add(1, 2)];                 // 编译期（数组大小）

// 运行期求值：结果用于运行期上下文
int a = 1, b = 2;
int c = add(a, b);                  // 运行期（a, b 不是常量）
```

***

### 6. consteval 强制编译期

```cpp
consteval int power(int base, int exp) {
    int result = 1;
    for (int i = 0; i < exp; ++i) {
        result *= base;
    }
    return result;
}

// 必须用编译期常量调用
int a = power(2, 10);              // OK，2 和 10 是字面量
constexpr int b = power(2, 10);    // OK
constinit int c = power(2, 10);    // OK

// 不能用运行时值调用
int base = 2, exp = 10;
// int d = power(base, exp);       // 编译错误！
// consteval 函数的参数必须是编译期可求值的

// consteval 的典型用途：编译期计算、类型哈希、格式字符串检查
consteval size_t strLen(const char* s) {
    size_t len = 0;
    while (s[len]) ++len;
    return len;
}

static_assert(strLen("hello") == 5);  // 编译期检查
int arr[strLen("hello")];             // 编译期确定数组大小
```

**consteval 与 constexpr 的组合**：

```cpp
// consteval 函数可以调用 constexpr 函数
constexpr int add(int a, int b) { return a + b; }

consteval int doubleAdd(int a, int b) {
    return add(a, b) * 2;  // OK，add 是 constexpr
}

// constexpr 函数不能保证调用 consteval 函数
// 因为 constexpr 函数可能在运行期执行
consteval int triple(int x) { return x * 3; }

constexpr int compute(int x) {
    // return triple(x);  // C++23 前：编译错误
    // C++23：允许在 constexpr 函数中调用 consteval
    return x * 3;
}
```

***

### 7. constinit：编译期初始化

```cpp
// constinit 解决"静态初始化顺序惨剧"（Static Initialization Order Fiasco）

// 问题：全局变量的初始化顺序不确定
std::string globalStr = "hello";  // 动态初始化，顺序不确定

// 解决：constinit 强制编译期初始化
constinit int globalInt = 42;     // 编译期初始化，顺序确定

// constinit vs constexpr
constexpr int a = 42;    // 编译期初始化 + const（不可修改）
constinit int b = 42;    // 编译期初始化 + 非 const（可修改）
b = 100;                 // OK

// 典型用法：单例模式
class Singleton {
    static Singleton* instance_;
public:
    static Singleton& get() {
        return *instance_;
    }
};
constinit Singleton* Singleton::instance_ = nullptr;  // 编译期初始化
```

***

### 8. 对比表格

| 特性 | constexpr | consteval | constinit |
|------|:---:|:---:|:---:|
| 求值时机 | 编译期或运行期 | 必须编译期 | 编译期初始化 |
| 运行期调用 | 允许 | 不允许（编译错误） | N/A（用于变量） |
| 用于函数 | ✅ | ✅ | ❌ |
| 用于变量 | ✅（编译期常量） | ❌ | ✅（编译期初始化） |
| 变量可修改 | 不可（隐式 const） | N/A | 可 |
| C++ 版本 | C++11 | C++20 | C++20 |
| 典型用途 | 通用编译期/运行期函数 | 编译期计算、编译期检查 | 避免静态初始化顺序问题 |

***

### 9. 完整示例

```cpp
#include <iostream>
#include <array>
using namespace std;

constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

consteval int constFactorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

constexpr int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

template<int N>
struct FactorialTable {
    array<int, N + 1> values{};
    constexpr FactorialTable() {
        for (int i = 0; i <= N; ++i) {
            values[i] = factorial(i);
        }
    }
};

int main() {
    // constexpr：编译期和运行期
    constexpr int f10 = factorial(10);      // 编译期
    cout << "factorial(10) = " << f10 << "\n";

    int n = 5;
    int f5 = factorial(n);                  // 运行期
    cout << "factorial(" << n << ") = " << f5 << "\n";

    // consteval：必须编译期
    constexpr int cf5 = constFactorial(5);  // 编译期
    cout << "constFactorial(5) = " << cf5 << "\n";

    // constexpr 编译期查找表
    constexpr auto table = FactorialTable<12>();
    cout << "factorial(7) from table = " << table.values[7] << "\n";

    // constexpr if
    auto printType = [](auto val) {
        if constexpr (is_same_v<decltype(val), int>) {
            cout << "int: " << val << "\n";
        } else if constexpr (is_same_v<decltype(val), double>) {
            cout << "double: " << val << "\n";
        } else {
            cout << "other: " << val << "\n";
        }
    };
    printType(42);
    printType(3.14);
    printType("hello");

    return 0;
}
```

***

### 10. 极简总结

**constexpr = 可能编译期求值（也允许运行期）| consteval = 必须编译期求值（不允许运行期）| constinit = 编译期初始化变量（可修改）| C++11 constexpr → C++14 放宽 → C++20 consteval/constinit | constexpr 函数是否编译期求值取决于调用上下文 | 需要强制编译期用 consteval | 需要避免静态初始化顺序问题用 constinit**

***

### 相关阅读

- [什么是C++20-Modules](./15-什么是C++20-Modules.md)
- [什么是C++23新特性](./16-什么是C++23新特性.md)
- [STL容器底层实现](./01-STL容器底层实现.md)

***