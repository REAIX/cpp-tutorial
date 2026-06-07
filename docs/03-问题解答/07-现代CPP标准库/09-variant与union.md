# variant 与 union 的区别
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[STL算法](../../02-CPP/15-STL算法与迭代器.md)、[正则表达式](../../02-CPP/16-正则表达式.md)、[文件操作与文件系统](../../02-CPP/18-文件操作与文件系统.md)

### 1. 核心要义

**variant 是类型安全的联合体（知道当前存的是什么类型），union 是不安全的传统联合体（你得自己记住存了什么）。新代码用 variant，只在需要底层控制或 C 兼容时用 union。**

***

### 2. 核心定义

| | variant | union |
|---|---|---|
| 是什么 | 类型安全的可辨识联合体（discriminated union） | 所有成员共享内存的传统联合体 |
| 类型安全 | 安全，编译器跟踪当前活跃类型 | 不安全，程序员自己跟踪当前活跃成员 |
| 访问方式 | std::get / std::visit（带类型检查） | 直接访问成员（无检查） |
| 复杂类型 | 支持带非平凡构造/析构的类型 | 有限支持（C++11 前不支持） |
| C++ 版本 | C++17 | C++98 |

**本质区别**：

```cpp
// variant：编译器帮你记住当前类型
std::variant<int, double, std::string> v = 42;     // v 当前是 int
v = 3.14;                                            // v 当前是 double
v = "hello";                                         // v 当前是 string
// 访问时必须指定类型，类型错误会抛异常

// union：你得自己记住当前成员
union Data {
    int i;
    double d;
    char s[32];
};
Data u;
u.i = 42;       // 当前是 int
u.d = 3.14;     // 现在是 double，但 i 已经无效
std::cout << u.i;  // UB！读取不活跃的成员
```

***

### 3. 生活类比

| | variant | union |
|---|---|---|
| 类比 | 智能保险箱 | 普通盒子 |
| 说明 | 保险箱知道里面放的是什么类型的东西，取的时候必须说对类型 | 盒子不记录你放了什么，你得自己记着 |
| 关键区别 | 取错类型会报警（抛异常） | 取错类型会拿到垃圾数据（UB） |

**具体场景**：

- **variant**：你有一个智能保险箱，每次放东西进去，保险箱自动记录类型。你取东西时必须告诉保险箱你要取什么类型。如果你说"取一个 int"但里面放的是 string，保险箱会报警（抛异常）。
- **union**：你有一个普通盒子，你往里面放了一个苹果。盒子不会记录你放了什么。如果你后来忘了，以为是橘子就去剥皮，结果可能弄一手苹果汁（未定义行为）。

***

### 4. union 的类型安全问题

```cpp
// 问题1：读取不活跃成员是 UB
union Value {
    int i;
    float f;
};

Value v;
v.i = 42;
std::cout << v.f;   // UB！读取不活跃的 float 成员

// 问题2：非平凡类型需要手动管理生命周期
union StringUnion {
    std::string s;
    int i;
    StringUnion() {}                    // 必须自己写构造函数
    ~StringUnion() {}                   // 必须自己写析构函数
    // 编译器不知道该调谁的析构函数
};

StringUnion su;
new (&su.s) std::string("hello");       // 手动 placement new
su.s.~basic_string();                   // 手动调用析构函数

// 问题3：没有自动的"当前类型"跟踪
enum class Tag { Int, Float };
struct TaggedUnion {
    Tag tag;
    union {
        int i;
        float f;
    };
};

TaggedUnion tu;
tu.tag = Tag::Int;
tu.i = 42;
// 如果忘记更新 tag，或者 tag 和实际不一致，就会出错
```

***

### 5. variant 的访问方式

**方式1：std::get（按类型或索引访问）**

```cpp
std::variant<int, double, std::string> v = 42;

// 按类型访问
int n = std::get<int>(v);          // 正确，返回 42
// double d = std::get<double>(v); // 抛 std::bad_variant_access！

// 按索引访问
int n2 = std::get<0>(v);           // 正确，索引 0 是 int
// auto x = std::get<1>(v);        // 抛 std::bad_variant_access！

// 检查当前类型
if (std::holds_alternative<int>(v)) {
    std::cout << "v holds int: " << std::get<int>(v) << "\n";
}

// 获取索引
std::cout << v.index();            // 0（int 是第一个类型）
```

**方式2：std::visit（访问者模式）**

```cpp
// 通用访问者
auto visitor = [](auto&& arg) {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, int>) {
        std::cout << "int: " << arg << "\n";
    } else if constexpr (std::is_same_v<T, double>) {
        std::cout << "double: " << arg << "\n";
    } else if constexpr (std::is_same_v<T, std::string>) {
        std::cout << "string: " << arg << "\n";
    }
};

std::variant<int, double, std::string> v = 42;
std::visit(visitor, v);   // 输出 "int: 42"

v = 3.14;
std::visit(visitor, v);   // 输出 "double: 3.14"

v = "hello";
std::visit(visitor, v);   // 输出 "string: hello"
```

**方式3：函数对象访问者（更结构化）**

```cpp
struct Printer {
    void operator()(int i) const { std::cout << "int: " << i << "\n"; }
    void operator()(double d) const { std::cout << "double: " << d << "\n"; }
    void operator()(const std::string& s) const { std::cout << "string: " << s << "\n"; }
};

std::variant<int, double, std::string> v = "hello";
std::visit(Printer{}, v);  // 输出 "string: hello"
```

**方式4：返回值的 visit**

```cpp
struct SizeOf {
    size_t operator()(int i) const { return sizeof(i); }
    size_t operator()(double d) const { return sizeof(d); }
    size_t operator()(const std::string& s) const { return s.size(); }
};

std::variant<int, double, std::string> v = "hello";
size_t sz = std::visit(SizeOf{}, v);  // 5（string 的长度）
```

***

### 6. 性能对比

```cpp
// 内存布局对比

// variant：包含判别式 + 最大成员
// sizeof(variant<int, double, string>)
// ≈ max(sizeof(int), sizeof(double), sizeof(string)) + sizeof(size_t) + padding
// 通常比 union 多一个 size_t 的判别式

// union：只包含最大成员
// sizeof(union { int i; double d; char s[32]; })
// = max(sizeof(int), sizeof(double), 32)

// 性能对比
| 操作 | variant | union |
|------|:---:|:---:|
| 构造 | 略慢（需设置判别式） | 快 |
| 析构 | 自动调用正确析构函数 | 需手动 |
| 访问 | get 有类型检查开销 | 直接访问，零开销 |
| visit | 编译器可优化为跳表 | 需手动 switch |
| 内存 | 多一个判别式 | 无额外开销 |
```

**实际性能影响**：variant 的额外开销通常可以忽略。判别式只是一个 size_t，类型检查在 debug 模式下有开销，release 模式下 get 的检查通常被优化掉。visit 在编译器优化后通常和手写 switch 一样快。

***

### 7. 对比表格

| 特性 | variant | union |
|------|:---:|:---:|
| 类型安全 | 安全，编译器跟踪活跃类型 | 不安全，程序员手动跟踪 |
| 访问方式 | std::get / std::visit | 直接访问成员 |
| 错误访问 | 抛 std::bad_variant_access | 未定义行为 |
| 复杂类型支持 | 支持（自动管理生命周期） | 有限（需手动 placement new/析构） |
| 判别式 | 内置（index()） | 无（需手动维护 tag） |
| 内存开销 | 判别式 + 对齐填充 | 仅最大成员 |
| 默认构造 | 首个类型默认构造 | 需要手动定义 |
| C++ 版本 | C++17 | C++98 |
| 典型用途 | 类型安全的多种返回值、状态机 | 底层内存操作、C 兼容、零开销 |

***

### 8. 完整示例

```cpp
#include <iostream>
#include <string>
#include <variant>
#include <vector>
using namespace std;

using Value = variant<int, double, string>;

struct Formatter {
    string operator()(int i) const { return "int(" + to_string(i) + ")"; }
    string operator()(double d) const { return "double(" + to_string(d) + ")"; }
    string operator()(const string& s) const { return "string(\"" + s + "\")"; }
};

struct Adder {
    Value operator()(int a, int b) const { return a + b; }
    Value operator()(double a, double b) const { return a + b; }
    Value operator()(const string& a, const string& b) const { return a + b; }
    Value operator()(auto a, auto b) const { return "type mismatch"; }
};

int main() {
    vector<Value> values = {42, 3.14, "hello", 100};

    cout << "=== Print all values ===\n";
    for (const auto& v : values) {
        cout << "  index=" << v.index() << " → " << visit(Formatter{}, v) << "\n";
    }

    cout << "\n=== Type checking ===\n";
    for (const auto& v : values) {
        if (holds_alternative<int>(v)) {
            cout << "  int: " << get<int>(v) << "\n";
        } else if (holds_alternative<double>(v)) {
            cout << "  double: " << get<double>(v) << "\n";
        } else if (holds_alternative<string>(v)) {
            cout << "  string: " << get<string>(v) << "\n";
        }
    }

    cout << "\n=== Modify values ===\n";
    Value v = 42;
    cout << "Before: " << visit(Formatter{}, v) << "\n";
    v = 3.14;
    cout << "After assign double: " << visit(Formatter{}, v) << "\n";
    v = "world";
    cout << "After assign string: " << visit(Formatter{}, v) << "\n";

    return 0;
}
```

***

### 9. 极简总结

**variant = 类型安全的联合体（内置判别式）| union = 不安全的传统联合体（手动跟踪类型）| 新代码用 variant | union 只用于底层控制或 C 兼容 | 访问 variant 用 std::get（按类型）或 std::visit（访问者模式）| variant 自动管理复杂类型的生命周期 | variant 的额外开销通常可忽略**

***

### 相关阅读

- [optional与nullptr](./07-optional与nullptr.md)
- [什么是std-any](./10-什么是std-any.md)
- [expected与optional](./08-expected与optional.md)

***