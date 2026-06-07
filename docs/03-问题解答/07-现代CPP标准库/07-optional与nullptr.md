# optional 与 nullptr 的区别
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[STL算法](../../02-CPP/15-STL算法与迭代器.md)、[正则表达式](../../02-CPP/16-正则表达式.md)、[文件操作与文件系统](../../02-CPP/18-文件操作与文件系统.md)

### 1. 核心提炼

**optional<T> 表示"可能有值也可能没有"（值语义），nullptr 是空指针常量（指针语义）。optional 更安全、更表达意图，推荐用它代替指针表示"可能无值"的场景。**

***

### 2. 核心定义

| | optional<T> | nullptr |
|---|---|---|
| 是什么 | 一个可能包含 T 值的包装器 | 空指针常量，表示指针不指向任何对象 |
| 语义 | "可能有值，也可能没有" | "指针不指向任何对象" |
| 类型安全 | 类型安全，必须检查是否有值 | 类型不安全，解引用空指针是 UB |
| 值/指针语义 | 值语义 | 指针语义 |
| C++ 版本 | C++17 | C++11（nullptr）/ C 时代（NULL） |

**本质区别**：

```cpp
// optional：值语义，自己管理对象的生命周期
std::optional<int> opt = 42;   // opt 内部包含一个 int
opt = std::nullopt;            // opt 不包含值，int 被销毁

// 指针 + nullptr：指针语义，不管理对象的生命周期
int* p = new int(42);          // p 指向堆上的 int
p = nullptr;                   // p 不指向任何对象，但原来的 int 泄漏了！
```

***

### 3. 生活类比

| | optional<T> | nullptr |
|---|---|---|
| 类比 | 快递柜（可能有包裹也可能空） | 写着"此处无门"的牌子 |
| 说明 | 你去快递柜取件，柜子要么有包裹要么空，你先看有没有再取 | 你看到一块牌子说"这里没有门"，但如果你硬推就会出事 |
| 关键区别 | 有明确的"有/无"状态，取值前必须检查 | 只是一个标记，没有强制检查机制 |

**具体场景**：

- **optional**：你去快递柜取件。柜子有指示灯，绿灯表示有包裹，红灯表示空。你必须先看指示灯（has_value），才能取包裹（value）。如果柜子是空的你还硬取，会报错（抛异常）。
- **nullptr**：你有一张地图，上面标着门的位置。如果地图上写着"此处无门"（nullptr），你硬推那扇不存在的门（解引用），就会摔跤（未定义行为）。

***

### 4. optional 的基本用法

```cpp
#include <optional>
#include <string>
#include <iostream>

// 创建 optional
std::optional<int> o1;                    // 空（不包含值）
std::optional<int> o2 = std::nullopt;     // 空（显式）
std::optional<int> o3 = 42;               // 包含值 42
std::optional<int> o4{42};                // 包含值 42
std::optional<std::string> o5 = "hello";  // 包含值 "hello"

// 检查是否有值
if (o3.has_value()) { /* 有值 */ }
if (o3) { /* 有值，隐式转 bool */ }

// 获取值
int v1 = o3.value();          // 有值则返回，无值抛 std::bad_optional_access
int v2 = *o3;                 // 有值则返回，无值是 UB（不检查）
int v3 = o3.value_or(0);     // 有值返回值，无值返回 0

// 修改
o3 = 100;                     // 赋值
o3.emplace(200);              // 原地构造
o3.reset();                   // 销毁值，变为空
o3 = std::nullopt;            // 同 reset()

// 比较
std::optional<int> a = 42, b = 42, c = std::nullopt;
// a == b → true
// a == c → false
// a == 42 → true（直接和值比较）
// c == std::nullopt → true
```

***

### 5. 与指针表示"可能无值"的对比

```cpp
// === 方式1：指针 + nullptr ===
int* findValue(const std::vector<int>& v, int target) {
    for (auto& x : v) {
        if (x == target) return &x;
    }
    return nullptr;  // 没找到
}

// 调用
auto result = findValue(vec, 42);
if (result != nullptr) {
    std::cout << *result;   // 忘记检查？解引用空指针 = UB
}

// === 方式2：optional ===
std::optional<int> findValue(const std::vector<int>& v, int target) {
    for (const auto& x : v) {
        if (x == target) return x;
    }
    return std::nullopt;  // 没找到
}

// 调用
auto result = findValue(vec, 42);
if (result.has_value()) {
    std::cout << result.value();  // 忘记检查？value() 抛异常（比 UB 好）
}
std::cout << result.value_or(-1);  // 默认值模式，更安全
```

**为什么 optional 更安全**：

| 风险 | 指针 + nullptr | optional |
|------|:---:|:---:|
| 忘记检查就访问 | UB（崩溃或更糟） | value() 抛异常（可捕获），*opt 是 UB 但更少用 |
| 默认值处理 | 需要三元运算符 | value_or() 内置支持 |
| 意图表达 | 指针可能表示"可空"也可能表示"可选参数"，含义模糊 | 明确表示"可能有值也可能没有" |
| 所有权 | 不清楚谁拥有指针指向的对象 | 值语义，optional 自己管理包含的对象 |
| 返回局部变量 | 不能返回局部变量的指针 | 可以返回局部变量的拷贝 |

***

### 6. optional 在函数返回值中的应用

```cpp
#include <optional>
#include <string>
#include <map>
#include <iostream>

// 场景1：查找操作
std::optional<std::string> lookup(const std::map<std::string, std::string>& db,
                                   const std::string& key) {
    auto it = db.find(key);
    if (it != db.end()) return it->second;
    return std::nullopt;
}

// 场景2：解析操作
std::optional<int> parseInt(std::string_view s) {
    try {
        size_t pos = 0;
        int val = std::stoi(std::string(s), &pos);
        if (pos == s.size()) return val;
        return std::nullopt;
    } catch (...) {
        return std::nullopt;
    }
}

// 场景3：配置项
struct Config {
    std::optional<int> port;        // 可选配置
    std::optional<std::string> log; // 可选配置
    int timeout = 30;               // 必须有默认值
};

// 场景4：链式调用
std::optional<std::string> getUserEmail(int userId);
std::optional<std::string> getGravatar(std::string_view email);

std::optional<std::string> getUserGravatar(int userId) {
    auto email = getUserEmail(userId);
    if (!email) return std::nullopt;
    return getGravatar(*email);
}

int main() {
    // 查找
    std::map<std::string, std::string> db = {{"name", "Alice"}, {"city", "Beijing"}};
    auto name = lookup(db, "name");
    auto age = lookup(db, "age");

    std::cout << "name: " << name.value_or("(not found)") << "\n";
    std::cout << "age: " << age.value_or("(not found)") << "\n";

    // 解析
    auto n1 = parseInt("42");
    auto n2 = parseInt("abc");
    std::cout << "parseInt(\"42\"): " << n1.value_or(-1) << "\n";
    std::cout << "parseInt(\"abc\"): " << n2.value_or(-1) << "\n";

    // 配置
    Config cfg;
    cfg.port = 8080;
    // cfg.log 不赋值，保持空

    if (cfg.port) {
        std::cout << "Port: " << *cfg.port << "\n";
    }
    std::cout << "Log: " << cfg.log.value_or("(default log)") << "\n";

    return 0;
}
```

***

### 7. optional 的注意事项

**注意1：optional 不支持引用类型**

```cpp
std::optional<int&> o;   // 编译错误！C++17 不支持
// 替代方案1：optional<reference_wrapper<int>>
std::optional<std::reference_wrapper<int>> o1;
// 替代方案2：用指针
int* ptr = nullptr;
```

**注意2：optional 的开销**

```cpp
// optional<T> 通常比 T 多一个 bool 标志
static_assert(sizeof(std::optional<int>) > sizeof(int));  // 通常 8 字节 vs 4 字节

// 但与指针相比
static_assert(sizeof(std::optional<int>) == sizeof(std::pair<int, bool>));
// 开销很小，通常可以忽略
```

**注意3：optional 与 in-place 构造**

```cpp
struct Point {
    double x, y;
    Point(double x, double y) : x(x), y(y) {}
};

// 直接构造
std::optional<Point> o1 = Point(1.0, 2.0);  // 构造临时对象 + 移动

// in-place 构造，避免临时对象
std::optional<Point> o2{std::in_place, 1.0, 2.0};

// 也可以用 emplace
o2.emplace(3.0, 4.0);
```

**注意4：不要用 optional 表示"错误"**

```cpp
// 不好的做法：用 optional 表示错误
std::optional<int> divide(int a, int b) {
    if (b == 0) return std::nullopt;  // 丢失了错误原因
    return a / b;
}

// 更好的做法：用 expected（C++23）或自定义类型
// 或者抛异常
int divide(int a, int b) {
    if (b == 0) throw std::invalid_argument("division by zero");
    return a / b;
}
```

***

### 8. 对比表格

| 特性 | optional<T> | T* + nullptr |
|------|:---:|:---:|
| 语义 | "可能有值也可能没有" | "指针可能指向对象也可能为空" |
| 所有权 | 值语义，自己管理对象 | 指针语义，不管理对象 |
| 空状态 | nullopt | nullptr |
| 检查是否有值 | has_value() 或隐式 bool | != nullptr |
| 安全访问 | value()（抛异常） | 无安全访问方式 |
| 默认值 | value_or() | 三元运算符 |
| 未检查访问 | *opt（UB） | *ptr（UB） |
| 局部变量安全 | 安全（值拷贝） | 不安全（返回局部变量指针是 UB） |
| 额外开销 | 一个 bool 标志 | 无 |
| 引用支持 | 不支持（C++17） | 支持 |
| C++ 版本 | C++17 | C++98 |

***

### 9. 极简总结

**optional = 值语义的"可能有值"包装器 | nullptr = 指针语义的空标记 | optional 更安全（value() 抛异常、value_or() 默认值）| 函数返回"可能无值"首选 optional | 不要用 optional 表示错误（丢失原因）| 不要用指针+nullptr 代替 optional 表示"可能无值"**

***

### 相关阅读

- [variant与union](./09-variant与union.md)
- [expected与optional](./08-expected与optional.md)
- [什么是std-any](./10-什么是std-any.md)

***