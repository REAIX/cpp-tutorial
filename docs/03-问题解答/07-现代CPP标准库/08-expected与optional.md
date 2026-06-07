# expected 与 optional 的区别
> 📖 相关章节：[异常处理](../../02-CPP/07-异常处理.md)、[C++20与23新特性](../../02-CPP/24-C++20与23新特性.md)

### 1. 要义概览

**expected<T, E> 要么有值要么有错误信息（知道为什么失败），optional<T> 要么有值要么没有（不知道为什么没有）。需要错误信息用 expected，只需"有无"用 optional。**

***

### 2. 核心定义

| | expected<T, E> | optional<T> |
|---|---|---|
| 是什么 | 包含 T 值或 E 类型错误信息的和类型 | 包含 T 值或空状态的包装器 |
| 语义 | "有值，或者告诉你为什么没有" | "有值，或者没有" |
| 失败信息 | 有（E 类型） | 无（只知道没有值） |
| C++ 版本 | C++23 | C++17 |

**本质区别**：

```cpp
// optional：只有"有/无"两种状态
std::optional<int> opt = std::nullopt;  // 没有，但不知道为什么没有

// expected：有"值"或"错误"两种状态
std::expected<int, std::string> ex = std::unexpected("file not found");
// 没有，而且知道是因为 "file not found"

std::expected<int, std::string> ex2 = 42;
// 有值 42
```

***

### 3. 生活类比

| | optional<T> | expected<T, E> |
|---|---|---|
| 类比 | 快递柜（有包裹或空） | 快递柜 + 留言板（有包裹或有留言说明为什么没包裹） |
| 说明 | 你去快递柜取件，要么有包裹，要么柜子空着 | 你去快递柜取件，要么有包裹，要么柜子上贴了留言告诉你原因 |
| 关键区别 | 空了就是空了，不知道原因 | 空了会告诉你为什么 |

**具体场景**：

- **optional**：你去快递柜取件。柜子要么有包裹，要么空着。如果空着，你不知道是快递员还没放进来，还是你取错了柜子，还是包裹丢了。
- **expected**：你去快递柜取件。柜子要么有包裹，要么贴了一张留言："您的包裹因地址不详被退回"。你知道具体原因，可以采取对应措施。

***

### 4. expected 的基本用法

```cpp
#include <expected>
#include <string>
#include <iostream>

// 创建 expected
std::expected<int, std::string> e1 = 42;                          // 有值
std::expected<int, std::string> e2 = std::unexpected("error");    // 有错误

// 检查状态
if (e1.has_value()) { /* 有值 */ }
if (e1) { /* 有值，隐式转 bool */ }

// 获取值
int v1 = e1.value();          // 有值返回，无值抛 std::bad_expected_access
int v2 = *e1;                 // 有值返回，无值是 UB
int v3 = e1.value_or(0);     // 有值返回值，无值返回 0

// 获取错误
if (!e2.has_value()) {
    std::string err = e2.error();   // 获取错误信息
    std::cout << "Error: " << err << "\n";
}

// 自定义错误类型
enum class ParseError { Empty, InvalidChar, Overflow };

std::expected<int, ParseError> parseInt(std::string_view s) {
    if (s.empty()) return std::unexpected(ParseError::Empty);
    // ... 解析逻辑
    return 42;
}

auto result = parseInt("123");
if (result) {
    std::cout << "Value: " << *result << "\n";
} else {
    switch (result.error()) {
        case ParseError::Empty:      std::cout << "Empty input\n"; break;
        case ParseError::InvalidChar: std::cout << "Invalid character\n"; break;
        case ParseError::Overflow:   std::cout << "Overflow\n"; break;
    }
}
```

***

### 5. 与 optional / 异常 / 错误码的对比

**四种错误处理方式对比**：

```cpp
#include <expected>
#include <optional>
#include <string>
#include <stdexcept>

// 方式1：错误码
int divide_ec(int a, int b, int& error) {
    if (b == 0) { error = 1; return 0; }
    return a / b;
}

// 方式2：optional（丢失错误原因）
std::optional<int> divide_opt(int a, int b) {
    if (b == 0) return std::nullopt;  // 为什么失败？不知道
    return a / b;
}

// 方式3：异常
int divide_exc(int a, int b) {
    if (b == 0) throw std::invalid_argument("division by zero");
    return a / b;
}

// 方式4：expected（保留错误原因，无异常开销）
std::expected<int, std::string> divide_exp(int a, int b) {
    if (b == 0) return std::unexpected("division by zero");
    return a / b;
}

int main() {
    // 错误码：容易忘记检查
    int err = 0;
    int r1 = divide_ec(10, 0, err);
    if (err) { /* 处理错误 */ }

    // optional：不知道原因
    auto r2 = divide_opt(10, 0);
    if (!r2) { /* 只知道失败了 */ }

    // 异常：有原因但有开销
    try {
        int r3 = divide_exc(10, 0);
    } catch (const std::exception& e) {
        std::cout << e.what() << "\n";  // "division by zero"
    }

    // expected：有原因，无异常开销
    auto r4 = divide_exp(10, 0);
    if (!r4) {
        std::cout << r4.error() << "\n";  // "division by zero"
    }

    return 0;
}
```

***

### 6. 错误处理范式演进

```
C 风格错误码 (errno / 返回值)
  ↓ 问题：容易忘记检查，错误码和有效值混在一起
C++ 异常 (try/catch)
  ↓ 问题：运行时开销、控制流不透明、不适合嵌入式
C++17 optional
  ↓ 问题：只知道"没有"，不知道"为什么没有"
C++23 expected
  ↓ 优势：保留错误信息 + 无异常开销 + 强制检查
```

**演进的核心诉求**：

| 阶段 | 解决的问题 | 遗留的问题 |
|------|---|---|
| 错误码 | 基本错误传播 | 容易忘记检查、类型不安全 |
| 异常 | 强制处理、类型安全 | 运行时开销、控制流跳跃 |
| optional | 轻量级"有无"表示 | 丢失错误原因 |
| expected | 轻量级 + 保留错误原因 | C++23 才可用 |

***

### 7. expected 的链式操作

```cpp
#include <expected>
#include <string>
#include <iostream>

std::expected<int, std::string> parseAge(std::string_view s) {
    if (s.empty()) return std::unexpected("empty input");
    int age = 0;
    for (char c : s) {
        if (c < '0' || c > '9') return std::unexpected("invalid character");
        age = age * 10 + (c - '0');
    }
    if (age < 0 || age > 150) return std::unexpected("age out of range");
    return age;
}

std::expected<std::string, std::string> formatProfile(int age) {
    return "Age: " + std::to_string(age);
}

int main() {
    // and_then：成功时继续处理，失败时直接传播错误
    auto result = parseAge("25")
        .and_then(formatProfile);

    if (result) {
        std::cout << *result << "\n";   // "Age: 25"
    } else {
        std::cout << "Error: " << result.error() << "\n";
    }

    // 失败时自动传播
    auto bad = parseAge("abc")
        .and_then(formatProfile);
    if (!bad) {
        std::cout << "Error: " << bad.error() << "\n";  // "invalid character"
    }

    // transform：成功时变换值
    auto doubled = parseAge("25")
        .transform([](int age) { return age * 2; });
    if (doubled) {
        std::cout << "Doubled: " << *doubled << "\n";  // 50
    }

    // or_else：失败时处理错误
    auto fallback = parseAge("")
        .or_else([](const std::string& err) -> std::expected<int, std::string> {
            std::cout << "Parse failed: " << err << ", using default\n";
            return 18;
        });
    std::cout << "Result: " << *fallback << "\n";  // 18

    return 0;
}
```

***

### 8. 对比表格

| 特性 | expected<T, E> | optional<T> | 异常 | 错误码 |
|------|:---:|:---:|:---:|:---:|
| 表达能力 | 值 + 错误信息 | 值 + 空 | 值 + 异常对象 | 值 + 整数 |
| 错误信息 | 丰富（E 类型） | 无 | 丰富（exception） | 有限（整数） |
| 强制检查 | 隐式 bool 检查 | 隐式 bool 检查 | try/catch | 无强制 |
| 运行时开销 | 极小 | 极小 | 可能较大 | 无 |
| 控制流 | 透明（顺序） | 透明（顺序） | 跳跃（栈展开） | 透明（顺序） |
| 链式操作 | and_then/transform | and_then/transform | 无 | 无 |
| C++ 版本 | C++23 | C++17 | C++98 | C |

***

### 9. 完整示例

```cpp
#include <expected>
#include <string>
#include <iostream>
#include <fstream>

enum class FileError {
    NotFound,
    PermissionDenied,
    ReadError
};

std::expected<std::string, FileError> readFileContent(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        return std::unexpected(FileError::NotFound);
    }
    std::string content((std::istreambuf_iterator<char>(file)),
                         std::istreambuf_iterator<char>());
    if (file.bad()) {
        return std::unexpected(FileError::ReadError);
    }
    return content;
}

std::expected<size_t, FileError> countLines(const std::string& content) {
    size_t count = 0;
    for (char c : content) {
        if (c == '\n') ++count;
    }
    return count + (content.empty() ? 0 : 1);
}

const char* fileErrorToString(FileError e) {
    switch (e) {
        case FileError::NotFound:         return "File not found";
        case FileError::PermissionDenied: return "Permission denied";
        case FileError::ReadError:        return "Read error";
    }
    return "Unknown error";
}

int main() {
    std::string path = "test.txt";

    auto result = readFileContent(path)
        .and_then(countLines);

    if (result) {
        std::cout << "Lines: " << *result << "\n";
    } else {
        std::cout << "Error: " << fileErrorToString(result.error()) << "\n";
    }

    // 对比 optional 版本：丢失错误原因
    // std::optional<size_t> countLinesOpt(const std::string& content);
    // 如果返回 nullopt，不知道是文件不存在还是读取失败

    return 0;
}
```

***

### 10. 极简总结

**expected = 有值或有错误信息 | optional = 有值或空 | expected 保留失败原因，optional 不保留 | expected 替代 optional+异常的中间地带 | 链式操作 and_then/transform/or_else | C++23 | 需要知道"为什么失败"用 expected，只需知道"有没有"用 optional**

***

### 相关阅读

- [optional与nullptr](./07-optional与nullptr.md)
- [variant与union](./09-variant与union.md)
- [什么是C++23新特性](./16-什么是C++23新特性.md)

***