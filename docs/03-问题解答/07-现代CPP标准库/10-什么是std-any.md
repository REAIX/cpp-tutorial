# 什么是std::any
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[STL算法](../../02-CPP/15-STL算法与迭代器.md)、[正则表达式](../../02-CPP/16-正则表达式.md)、[文件操作与文件系统](../../02-CPP/18-文件操作与文件系统.md)

> "std::any is the type-safe successor to void* — it holds any copyable type, but never forgets what it holds." — Nicolai Josuttis

***

### 1. 本质洞察

`std::any` 是 C++17 引入的类型安全容器，可以持有任意可复制类型的值，通过 `any_cast` 安全取回，比 `void*` 安全，比 `variant` 灵活。

***

### 2. 基本概念与创建方式

`std::any` 是一个值语义的容器，内部可以存储任意可复制构造的类型。

```cpp
#include <any>
#include <string>
#include <iostream>

int main() {
    std::any a1 = 42;
    std::any a2 = 3.14;
    std::any a3 = std::string("hello");
    std::any a4;

    std::cout << "a1 有值: " << a1.has_value() << "\n";
    std::cout << "a4 有值: " << a4.has_value() << "\n";

    a4 = true;
    std::cout << "a4 赋值后: " << a4.has_value() << "\n";

    a1.reset();
    std::cout << "a1 reset后: " << a1.has_value() << "\n";

    a1 = std::vector<int>{1, 2, 3};
}
```

创建与赋值方式汇总：

| 方式 | 代码 | 说明 |
|------|------|------|
| 直接初始化 | `std::any a = 42;` | 推导类型为 int |
| 默认构造 | `std::any a;` | 空值，`has_value()` 为 false |
| 拷贝构造 | `std::any a(b);` | 拷贝另一个 any |
| 赋值 | `a = 3.14;` | 销毁旧值，存入新值 |
| `emplace` | `a.emplace<std::string>("hi");` | 原地构造，避免临时对象 |
| `make_any` | `auto a = std::make_any<int>(42);` | 辅助函数创建 |

```cpp
std::any a;
a.emplace<std::string>(5, 'x');

auto b = std::make_any<std::vector<int>>(10, 0);
```

***

### 3. make_any 与 any_cast 详解

`std::make_any` 是创建 `std::any` 的工厂函数，`std::any_cast` 是取回值的唯一安全途径。

```cpp
#include <any>
#include <string>
#include <iostream>

int main() {
    auto a = std::make_any<std::string>("world");

    std::string& ref = std::any_cast<std::string&>(a);
    ref = "modified";

    const std::string& cref = std::any_cast<const std::string&>(a);

    std::string val = std::any_cast<std::string>(a);

    std::string* ptr = std::any_cast<std::string>(&a);
    if (ptr) {
        *ptr = "via pointer";
    }

    const std::string* cptr = std::any_cast<std::string>(&std::as_const(a));
}
```

`any_cast` 的三种形式：

| 形式 | 语法 | 失败行为 | 返回类型 |
|------|------|----------|----------|
| 值类型 | `any_cast<T>(any)` | 抛出 `bad_any_cast` | `T`（拷贝） |
| 引用类型 | `any_cast<T&>(any)` | 抛出 `bad_any_cast` | `T&` |
| 指针类型 | `any_cast<T>(&any)` | 返回 `nullptr` | `T*` |

```cpp
std::any a = 42;

try {
    std::string s = std::any_cast<std::string>(a);
} catch (const std::bad_any_cast& e) {
    std::cout << "类型不匹配: " << e.what() << "\n";
}

int* pi = std::any_cast<int>(&a);
if (pi) {
    std::cout << "值: " << *pi << "\n";
}

double* pd = std::any_cast<double>(&a);
if (!pd) {
    std::cout << "不是 double\n";
}
```

> ⚠️ 注意：`any_cast<T>(any)` 要求类型精确匹配，不会做隐式转换。`any_cast<int>(a)` 中 a 存的是 `short` 也会失败。

***

### 4. has_value() 与 type() 检查

`has_value()` 检查是否持有值，`type()` 返回存储值的 `std::type_info` 引用。

```cpp
#include <any>
#include <iostream>
#include <typeinfo>

int main() {
    std::any a = 42;

    if (a.has_value()) {
        std::cout << "有值\n";
        std::cout << "类型名: " << a.type().name() << "\n";
        std::cout << "是 int: " << (a.type() == typeid(int)) << "\n";
        std::cout << "是 double: " << (a.type() == typeid(double)) << "\n";
    }

    a.reset();
    if (!a.has_value()) {
        std::cout << "已清空\n";
        std::cout << "type() == typeid(void): "
                  << (a.type() == typeid(void)) << "\n";
    }
}
```

`type()` 的注意事项：

| 要点 | 说明 |
|------|------|
| 空值时 | `type()` 返回 `typeid(void)` |
| 返回值 | `const std::type_info&` |
| 跨编译器 | `name()` 输出不可移植（如 GCC 的 mangled name） |
| 比较方式 | 用 `==` 与 `typeid(T)` 比较，不要用 `name()` 字符串 |

```cpp
// 平台差异：type_info::name() 的输出
// GCC/Linux:   a.type().name() 可能是 "i" (int)
// MSVC:       a.type().name() 可能是 "int"
// Clang/macOS: a.type().name() 可能是 "i"

// 正确做法：用 typeid 比较
if (a.type() == typeid(int)) {
    int val = std::any_cast<int>(a);
}
```

***

### 5. bad_any_cast 异常处理

当 `any_cast` 的目标类型与实际存储类型不匹配时，抛出 `std::bad_any_cast`。

```cpp
#include <any>
#include <iostream>

void safe_print(const std::any& a) {
    if (!a.has_value()) {
        std::cout << "(空)\n";
        return;
    }

    if (a.type() == typeid(int)) {
        std::cout << "int: " << std::any_cast<int>(a) << "\n";
    } else if (a.type() == typeid(double)) {
        std::cout << "double: " << std::any_cast<double>(a) << "\n";
    } else if (a.type() == typeid(std::string)) {
        std::cout << "string: " << std::any_cast<std::string>(a) << "\n";
    } else {
        std::cout << "未知类型\n";
    }
}

int main() {
    std::any a = 42;

    try {
        auto s = std::any_cast<std::string>(a);
    } catch (const std::bad_any_cast& e) {
        std::cout << "捕获异常: " << e.what() << "\n";
    }

    safe_print(42);
    safe_print(3.14);
    safe_print(std::string("hi"));
    safe_print(std::any{});
}
```

异常 vs 指针两种风格对比：

| 风格 | 代码 | 优缺点 |
|------|------|--------|
| 异常风格 | `any_cast<T>(a)` | 简洁，但异常有开销 |
| 指针风格 | `any_cast<T>(&a)` | 无异常开销，适合性能敏感场景 |

```cpp
// 性能敏感场景推荐指针风格
template <typename T>
T get_or_default(const std::any& a, T default_val) {
    const T* ptr = std::any_cast<T>(&a);
    return ptr ? *ptr : default_val;
}
```

***

### 6. std::any vs void* 对比

`void*` 是 C 语言时代的通用指针，`std::any` 是其类型安全的替代品。

```cpp
// void* 方式 —— 危险，无类型信息
void* vp = new int(42);
int val = *static_cast<int*>(vp);
double wrong = *static_cast<double*>(vp); // 未定义行为！
delete static_cast<int*>(vp);

// std::any 方式 —— 安全，有类型检查
std::any a = 42;
int val2 = std::any_cast<int>(a);
double wrong2 = std::any_cast<double>(a); // 抛出 bad_any_cast，而非 UB
```

全面对比：

| 对比维度 | `void*` | `std::any` |
|----------|---------|------------|
| 类型安全 | ❌ 无检查 | ✅ 运行时检查 |
| 内存管理 | 手动 new/delete | 自动 RAII |
| 类型信息 | 丢失 | 保留（`type()`） |
| 值语义 | ❌ 指针语义 | ✅ 值语义 |
| 拷贝 | 浅拷贝（危险） | 深拷贝 |
| 异常安全 | 差 | 好 |
| 性能 | 零开销 | 有 SBO 开销 |
| 适用场景 | C 接口、极低层 | C++ 通用类型擦除 |

***

### 7. std::any vs std::variant 对比

`std::variant` 是类型安全的联合体，只能持有预定义的类型列表；`std::any` 可以持有任意类型。

```cpp
#include <any>
#include <variant>
#include <string>
#include <iostream>

// variant：类型列表固定
using Value = std::variant<int, double, std::string>;

void process_variant(const Value& v) {
    std::visit([](const auto& val) {
        std::cout << val << "\n";
    }, v);
}

// any：类型不固定
void process_any(const std::any& a) {
    if (a.type() == typeid(int)) {
        std::cout << std::any_cast<int>(a) << "\n";
    } else if (a.type() == typeid(double)) {
        std::cout << std::any_cast<double>(a) << "\n";
    } else if (a.type() == typeid(std::string)) {
        std::cout << std::any_cast<std::string>(a) << "\n";
    }
}
```

| 对比维度 | `std::any` | `std::variant` |
|----------|------------|----------------|
| 类型约束 | 任意可复制类型 | 固定类型列表 |
| 访问方式 | `any_cast` + 类型检查 | `std::visit` + 模式匹配 |
| 编译期检查 | ❌ 运行时 | ✅ 编译期 |
| 内存开销 | 动态分配（大对象） | 最大类型大小 + 判别式 |
| 性能 | 较慢（类型擦除） | 较快（编译期确定） |
| 扩展性 | 新增类型无需修改 | 新增类型需改 variant 定义 |
| 适用场景 | 插件系统、脚本绑定 | 状态机、AST 节点、配置值 |

选择指南：

```
类型是否已知且有限？── 是 → std::variant
                        └── 否 → std::any
需要编译期类型安全？── 是 → std::variant
                        └── 否 → std::any
性能是否关键？─────── 是 → std::variant
                        └── 否 → std::any
```

***

### 8. SBO（Small Buffer Optimization）与性能

`std::any` 内部使用 SBO（小缓冲区优化），小对象存储在对象内部，大对象才堆分配。

```cpp
#include <any>
#include <string>
#include <iostream>

struct Small {
    int data[4];
};

struct Large {
    int data[1000];
};

int main() {
    std::any a1 = 42;
    std::any a2 = Small{};
    std::any a3 = Large{};
    std::any a4 = std::string("short");
    std::any a5 = std::string(1000, 'x');

    std::cout << "sizeof(std::any): " << sizeof(std::any) << "\n";
    std::cout << "sizeof(Small): " << sizeof(Small) << "\n";
    std::cout << "sizeof(Large): " << sizeof(Large) << "\n";
}
```

SBO 原理示意：

```
┌─────────────────────────────┐
│  std::any 内部结构           │
│  ┌───────────────────────┐  │
│  │ 管理指针 / 小对象存储  │  │  ← SBO 区域（通常 16-32 字节）
│  ├───────────────────────┤  │
│  │ type_info 指针        │  │
│  └───────────────────────┘  │
└─────────────────────────────┘

小对象（≤ SBO 大小）：直接存储在 any 内部
大对象（> SBO 大小）：堆上分配，any 内部存指针
```

各实现的 SBO 大小：

| 实现 | SBO 大小 | `sizeof(std::any)` |
|------|----------|-------------------|
| libstdc++ (GCC) | 16 字节 | 16 |
| libc++ (Clang) | 24 字节 | 24 |
| MSVC STL | 24 字节 | 40 |

性能建议：

| 建议 | 原因 |
|------|------|
| 优先存小类型 | 避免堆分配 |
| 避免频繁赋值 | 每次赋值可能触发析构+构造 |
| 指针风格 `any_cast<T>(&a)` | 避免异常开销 |
| 能用 `variant` 就不用 `any` | `variant` 编译期确定，无类型擦除开销 |

***

### 9. 实际应用场景

**场景一：插件/脚本系统的属性映射**

```cpp
#include <any>
#include <string>
#include <unordered_map>
#include <iostream>

using Properties = std::unordered_map<std::string, std::any>;

class GameObject {
    Properties props_;
public:
    template <typename T>
    void set(const std::string& key, T value) {
        props_[key] = std::move(value);
    }

    template <typename T>
    std::optional<T> get(const std::string& key) const {
        auto it = props_.find(key);
        if (it == props_.end()) return std::nullopt;
        const T* ptr = std::any_cast<T>(&it->second);
        if (!ptr) return std::nullopt;
        return *ptr;
    }
};

int main() {
    GameObject obj;
    obj.set("health", 100);
    obj.set("name", std::string("Hero"));
    obj.set("position", std::vector<double>{1.0, 2.0, 3.0});

    auto hp = obj.get<int>("health");
    auto name = obj.get<std::string>("name");
    auto pos = obj.get<std::vector<double>>("position");
}
```

**场景二：消息队列**

```cpp
#include <any>
#include <queue>
#include <functional>
#include <iostream>

class MessageBus {
    std::queue<std::pair<std::string, std::any>> queue_;
public:
    template <typename T>
    void push(const std::string& topic, T msg) {
        queue_.emplace(topic, std::move(msg));
    }

    template <typename T>
    bool try_pop(const std::string& topic, T& out) {
        if (queue_.empty()) return false;
        auto& [t, a] = queue_.front();
        if (t != topic) return false;
        const T* ptr = std::any_cast<T>(&a);
        if (!ptr) return false;
        out = *ptr;
        queue_.pop();
        return true;
    }
};
```

**场景三：动态配置**

```cpp
#include <any>
#include <map>
#include <string>

class Config {
    std::map<std::string, std::any> values_;
public:
    template <typename T>
    void set(const std::string& key, T val) {
        values_[key] = std::make_any<T>(std::move(val));
    }

    template <typename T>
    T get_or(const std::string& key, T default_val) const {
        auto it = values_.find(key);
        if (it == values_.end()) return default_val;
        const T* ptr = std::any_cast<T>(&it->second);
        return ptr ? *ptr : default_val;
    }
};

int main() {
    Config cfg;
    cfg.set("width", 800);
    cfg.set("height", 600);
    cfg.set("title", std::string("My App"));
    cfg.set("fullscreen", false);

    int w = cfg.get_or<int>("width", 1024);
    bool fs = cfg.get_or<bool>("fullscreen", false);
}
```

***

### 10. 极简总结

| 概念 | 关键点 |
|------|--------|
| 本质 | 类型安全的 `void*`，持有任意可复制类型 |
| 创建 | `std::any a = val;` / `std::make_any<T>(args)` |
| 取值 | `std::any_cast<T>(a)` 抛异常 / `std::any_cast<T>(&a)` 返回指针 |
| 检查 | `has_value()` 判空 / `type() == typeid(T)` 判类型 |
| 异常 | `std::bad_any_cast`，类型不匹配时抛出 |
| SBO | 小对象存内部，大对象堆分配 |
| vs `void*` | 安全、自动内存管理、保留类型信息 |
| vs `variant` | `any` 更灵活但更慢，`variant` 更安全更快但类型固定 |
| 适用场景 | 插件属性、消息总线、动态配置、脚本绑定 |
| 不适用 | 性能关键路径、类型已知且有限时用 `variant` |

***

### 相关阅读

- [什么是SBO小缓冲区优化](./13-什么是SBO小缓冲区优化.md)
- [variant与union](./09-variant与union.md)
- [optional与nullptr](./07-optional与nullptr.md)

***