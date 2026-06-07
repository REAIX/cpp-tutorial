# std::format 与 printf 的区别
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[STL算法](../../02-CPP/15-STL算法与迭代器.md)、[正则表达式](../../02-CPP/16-正则表达式.md)、[文件操作与文件系统](../../02-CPP/18-文件操作与文件系统.md)

> "填表时手写还是用电子表单"——printf 手写类型不报错，std::format 类型不匹配自动报错。

***

### 1. 核心定义

- **printf** = C 语言的变参格式化函数，格式字符串与参数类型不匹配时行为未定义
- **std::format** = C++20 的类型安全格式化库，编译期检查格式字符串，参数类型自动推导

关键点：**printf 是类型不安全的"盲打"，std::format 是类型安全的"智能填表"**。

***

### 2. 生活类比

**填表方式**：

| 概念 | 类比 | 对应代码 |
|------|------|---------|
| printf | 手写填表：姓名栏填了数字、年龄栏填了文字，没人拦你 | `%d` 传 `string`，编译不报错，运行崩溃 |
| std::format | 电子表单：姓名栏只能填文字，年龄栏只能填数字，填错自动报错 | 类型不匹配直接编译错误 |

**printf 的危险**：

```
电子表格：姓名栏
你填了：3.14
→ 没有验证，直接提交
→ 数据库里姓名变成了 3.14
→ 后续处理全部出错

printf：姓名栏用 %s
你传了：3.14（double）
→ 编译器不检查类型
→ 运行时把 double 的二进制当字符串读
→ 乱码或崩溃
```

**std::format 的安全**：

```
电子表单：姓名栏（只接受 string）
你填了：3.14
→ 红色提示：类型不匹配！
→ 必须修正才能提交

std::format：姓名栏用 {}
你传了：3.14（double）
→ 编译错误：参数类型不匹配
→ 必须修正才能编译
```

***

### 3. 类型安全

#### 1. printf 的类型不安全

```cpp
#include <cstdio>
#include <string>

int main() {
    int age = 25;
    const char* name = "Alice";
    double score = 95.5;

    // 正确用法
    printf("Name: %s, Age: %d, Score: %.1f\n", name, age, score);

    // 类型不匹配——编译通过，运行出错！
    printf("Age: %s\n", age);           // %s 对应 int → 崩溃或乱码
    printf("Name: %d\n", name);         // %d 对应 const char* → 输出地址
    printf("Score: %d\n", score);       // %d 对应 double → 输出错误值

    // 格式字符串与参数数量不匹配
    printf("Age: %d, Score: %.1f\n", age);  // 缺少参数 → 未定义行为
}
```

**printf 的问题**：格式说明符（`%d`、`%s`、`%f`）和实际参数类型之间没有编译期检查，完全靠程序员保证正确。

#### 2. std::format 的类型安全

```cpp
#include <format>
#include <iostream>
#include <string>

int main() {
    int age = 25;
    std::string name = "Alice";
    double score = 95.5;

    // 正确用法——{} 自动推导类型
    std::cout << std::format("Name: {}, Age: {}, Score: {:.1f}\n", name, age, score);

    // 类型不匹配——编译错误！
    // std::cout << std::format("Age: {:s}\n", age);
    // 编译错误：format specifier 's' 不适用于 int 类型

    // 参数数量不匹配——编译错误！
    // std::cout << std::format("Age: {}, Score: {:.1f}\n", age);
    // 编译错误：参数数量不足
}
```

**std::format 的优势**：格式说明符与参数类型在编译期检查，类型不匹配直接编译失败。

#### 3. 编译期格式字符串检查（C++23 std::print）

```cpp
#include <print>

int main() {
    int age = 25;

    // C++23 std::print：格式字符串在编译期检查
    std::print("Age: {}\n", age);       // OK
    // std::print("Age: {:s}\n", age);  // 编译错误！
}
```

***

### 4. 格式化语法对比

#### 1. 基本格式化

```cpp
#include <cstdio>
#include <format>
#include <iostream>

int main() {
    int value = 42;
    double pi = 3.14159265;

    // printf
    printf("Value: %d\n", value);
    printf("Pi: %.2f\n", pi);
    printf("Hex: 0x%x\n", value);
    printf("Padded: %5d\n", value);
    printf("Left: %-5d|\n", value);

    // std::format
    std::cout << std::format("Value: {}\n", value);
    std::cout << std::format("Pi: {:.2f}\n", pi);
    std::cout << std::format("Hex: 0x{:x}\n", value);
    std::cout << std::format("Padded: {:5d}\n", value);
    std::cout << std::format("Left: {:<5d}|\n", value);
}
```

#### 2. 位置参数

```cpp
// printf：不支持位置参数（除非用 POSIX 扩展 %1$d）
// 无法重排参数顺序

// std::format：支持位置参数
std::cout << std::format("{0} {1} {0}\n", "Hello", "World");
// 输出：Hello World Hello

std::cout << std::format("{1} {0}\n", "first", "second");
// 输出：second first
```

#### 3. 宽度与精度

```cpp
// printf
printf("%10.2f\n", 3.14159);   // 宽度10，精度2
printf("%-10s|\n", "Hello");    // 左对齐，宽度10

// std::format
std::cout << std::format("{:10.2f}\n", 3.14159);
std::cout << std::format("{:<10}|\n", "Hello");
```

#### 4. 格式说明符对照

| 功能 | printf | std::format |
|------|--------|-------------|
| 十进制整数 | `%d` | `{:d}` 或 `{}` |
| 十六进制 | `%x` / `%X` | `{:x}` / `{:X}` |
| 八进制 | `%o` | `{:o}` |
| 浮点数 | `%f` | `{:f}` |
| 科学计数法 | `%e` / `%E` | `{:e}` / `{:E}` |
| 字符串 | `%s` | `{}` |
| 字符 | `%c` | `{:c}` |
| 指针 | `%p` | `{:p}` |
| 宽度 | `%10d` | `{:10d}` |
| 精度 | `%.2f` | `{:.2f}` |
| 左对齐 | `%-10d` | `{:<10d}` |
| 前导零 | `%05d` | `{:05d}` |
| 正号 | `%+d` | `{:+d}` |
| 位置参数 | 不支持 | `{0}` `{1}` |

***

### 5. 性能对比

```cpp
#include <cstdio>
#include <format>
#include <iostream>
#include <chrono>
#include <string>

int main() {
    const int N = 1'000'000;

    // printf 性能测试
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        char buf[128];
        std::snprintf(buf, sizeof(buf), "Name: %s, Age: %d, Score: %.2f",
                      "Alice", 25, 95.5);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto printf_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // std::format 性能测试
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < N; ++i) {
        std::string s = std::format("Name: {}, Age: {}, Score: {:.2f}",
                                    "Alice", 25, 95.5);
    }
    end = std::chrono::high_resolution_clock::now();
    auto format_ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "printf:      " << printf_ms << "ms\n";
    std::cout << "std::format: " << format_ms << "ms\n";
}
```

**典型结果**：

| 场景 | printf | std::format | 说明 |
|------|:---:|:---:|------|
| 简单格式化 | 快 | 略慢 | printf 直接写入缓冲区 |
| 复杂格式化 | 快 | 接近 | 编译器优化后差距缩小 |
| 返回 string | snprintf + string 构造 | 直接返回 string | std::format 更方便 |
| 编译期检查 | 无 | 有 | std::format 安全性远胜 |

**结论**：printf 在原始性能上略优，但 std::format 的类型安全和便利性远比微小的性能差异重要。

***

### 6. 自定义格式化（formatter 特化）

std::format 支持为自定义类型特化 `std::formatter`，这是 printf 无法做到的。

#### 1. 基本自定义格式化

```cpp
#include <format>
#include <iostream>
#include <string>

struct Point {
    double x, y;
};

template <>
struct std::formatter<Point> : std::formatter<std::string> {
    auto format(const Point& p, format_context& ctx) const {
        return std::formatter<std::string>::format(
            std::format("({}, {})", p.x, p.y), ctx);
    }
};

int main() {
    Point p{3.14, 2.72};
    std::cout << std::format("Point: {}\n", p);
    // 输出：Point: (3.14, 2.72)
}
```

#### 2. 支持格式说明符的自定义格式化

```cpp
#include <format>
#include <iostream>

struct Color {
    int r, g, b;
};

template <>
struct std::formatter<Color> : std::formatter<std::string> {
    bool hex_format = false;

    constexpr auto parse(format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && *it == 'x') {
            hex_format = true;
            ++it;
        }
        return it;
    }

    auto format(const Color& c, format_context& ctx) const {
        if (hex_format) {
            return std::formatter<std::string>::format(
                std::format("#{:02x}{:02x}{:02x}", c.r, c.g, c.b), ctx);
        }
        return std::formatter<std::string>::format(
            std::format("rgb({}, {}, {})", c.r, c.g, c.b), ctx);
    }
};

int main() {
    Color c{255, 128, 64};
    std::cout << std::format("Color: {}\n", c);   // rgb(255, 128, 64)
    std::cout << std::format("Color: {:x}\n", c);  // #ff8040
}
```

**printf 无法做到**：printf 没有扩展机制，自定义类型只能先转成 string 再传入。

***

### 7. 对比表格

| 特性 | printf | std::format |
|------|:---:|:---:|
| 类型安全 | ❌ 不检查 | ✅ 编译期检查 |
| 编译期检查 | ❌ 运行时才发现 | ✅ 编译时发现 |
| 扩展性 | ❌ 无法自定义类型 | ✅ formatter 特化 |
| C++ 版本 | C89 / C++98 | C++20 |
| 位置参数 | ❌ 不支持 | ✅ `{0}` `{1}` |
| Unicode 支持 | ❌ 无 | ✅ C++23 改进 |
| 返回 string | snprintf 间接 | 直接返回 |
| 格式字符串 | `%d` `%s` `%f` | `{}` `{:d}` `{:.2f}` |
| 头文件 | `<cstdio>` | `<format>` |
| 运行时性能 | 略快 | 略慢（差距小） |
| 国际化 | 困难 | 更友好（位置参数） |

***

### 8. 极简总结

**printf 是盲打，std::format 是智能填表**

| 要点 | 说明 |
|------|------|
| 核心区别 | printf 类型不安全，std::format 编译期类型检查 |
| 类型安全 | printf 格式符与参数类型不匹配是 UB，std::format 编译报错 |
| 扩展性 | std::format 支持 formatter 特化，printf 无法扩展 |
| C++ 版本 | printf: C89/C++98，std::format: C++20 |
| 性能 | printf 略快，但差距微小，安全性的价值远超性能 |
| 一句话 | printf 手写填表不报错，std::format 电子表单自动校验 |

***

### 相关阅读

- [什么是C++20-Modules](./15-什么是C++20-Modules.md)
- [什么是C++23新特性](./16-什么是C++23新特性.md)
- [C语言标准库分类与使用](./17-C语言标准库分类与使用.md)