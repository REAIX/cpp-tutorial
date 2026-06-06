# 什么是完美转发Perfect Forwarding
> 📖 相关章节：[类与对象](../../02-CPP/03-类与对象.md)、[继承与多态](../../02-CPP/04-继承与多态.md)、[核心机制](../../02-CPP/05-核心机制.md)、[异常处理](../../02-CPP/07-异常处理.md)、[移动语义](../../02-CPP/09-移动语义与完美转发.md)

> 转发不是搬运，是原封不动地传递。

***

### 1. 要义概览

完美转发（Perfect Forwarding）是C++11引入的机制，通过转发引用（T&&）和std::forward，将参数的值类别（左值/右值）和const属性原封不动地传递给另一个函数，避免不必要的拷贝。

***

### 2. 为什么需要完美转发

不使用完美转发时，参数在传递过程中会丢失原始的值类别信息：

```cpp
#include <iostream>
#include <string>
#include <utility>

class Widget {
public:
    Widget() { std::cout << "默认构造\n"; }
    Widget(const Widget&) { std::cout << "拷贝构造\n"; }
    Widget(Widget&&) { std::cout << "移动构造\n"; }
};

void process(Widget& w) {
    std::cout << "处理左值\n";
}

void process(const Widget& w) {
    std::cout << "处理const左值\n";
}

void process(Widget&& w) {
    std::cout << "处理右值\n";
}

template<typename T>
void bad_wrapper(T w) {
    std::cout << "bad_wrapper: ";
    process(w);
}

int main() {
    Widget w;

    std::cout << "--- 直接调用 ---\n";
    process(w);
    process(std::move(w));

    std::cout << "--- bad_wrapper ---\n";
    bad_wrapper(w);
    bad_wrapper(std::move(w));
}
```

输出：

```
默认构造
--- 直接调用 ---
处理左值
处理右值
--- bad_wrapper ---
bad_wrapper: 处理左值
bad_wrapper: 处理左值      ← 右值信息丢失！
```

问题本质：

| 传递方式 | 左值参数 | 右值参数 | 额外开销 |
|---------|---------|---------|---------|
| 值传递(T) | 拷贝 | 移动 | 有拷贝/移动 |
| 左值引用(T&) | 原样 | ❌无法接收 | - |
| const引用(const T&) | 原样 | 丢失右值性 | 可能多余拷贝 |
| 完美转发(T&& + forward) | 原样 | 原样 | 零开销 |

***

### 3. 转发引用T&&的本质

`T&&` 在模板中并非"右值引用"，而是"转发引用"（Forwarding Reference），它能匹配任何值类别：

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void check(T&& param) {
    using ParamType = decltype(param);

    std::cout << "是否左值引用: " << std::is_lvalue_reference_v<ParamType> << "\n";
    std::cout << "是否右值引用: " << std::is_rvalue_reference_v<ParamType> << "\n";
}

int main() {
    int x = 10;
    const int cx = 20;

    std::cout << "--- 传入左值 ---\n";
    check(x);

    std::cout << "--- 传入const左值 ---\n";
    check(cx);

    std::cout << "--- 传入右值 ---\n";
    check(10);

    std::cout << "--- 传入std::move ---\n";
    check(std::move(x));
}
```

转发引用的推导规则：

| 调用表达式 | 参数类型 | T推导为 | param类型 |
|-----------|---------|--------|----------|
| `check(x)` | int左值 | `int&` | `int& &&` → `int&` |
| `check(cx)` | const int左值 | `const int&` | `const int& &&` → `const int&` |
| `check(10)` | int右值 | `int` | `int&&` |
| `check(std::move(x))` | int右值 | `int` | `int&&` |

> ⚠️ 转发引用仅出现在模板类型推导中。`void f(std::string&&)` 是右值引用，不是转发引用。

***

### 4. 引用折叠四条规则

当模板实例化时可能出现"引用的引用"，C++定义了引用折叠（Reference Collapsing）规则：

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void show_type() {
    using Ref = T&&;
    std::cout << "T = " << typeid(T).name()
              << ", T&& = " << typeid(Ref).name()
              << ", 是否左值引用: " << std::is_lvalue_reference_v<Ref>
              << "\n";
}

int main() {
    show_type<int>();
    show_type<int&>();
    show_type<int&&>();
    show_type<const int&>();
}
```

四条折叠规则：

| T的类型 | T&&的展开 | 折叠结果 | 值类别 |
|--------|----------|---------|--------|
| `int` | `int &&` | `int&&` | 右值引用 |
| `int&` | `int& &&` | `int&` | 左值引用 |
| `int&&` | `int&& &&` | `int&&` | 右值引用 |
| `const int&` | `const int& &&` | `const int&` | 左值引用 |

折叠口诀：**只要有一个是左值引用，结果就是左值引用**。

```
&  + &  → &     (左值 + 左值 → 左值)
&  + && → &     (左值 + 右值 → 左值)
&& + &  → &     (右值 + 左值 → 左值)
&& + && → &&    (右值 + 右值 → 右值)
```

***

### 5. std::forward的工作原理

`std::forward` 是有条件转换：当T是左值引用时返回左值，否则返回右值：

```cpp
#include <iostream>
#include <utility>
#include <string>

void target(int& x)  { std::cout << "左值重载: " << x << "\n"; }
void target(int&& x) { std::cout << "右值重载: " << x << "\n"; }

template<typename T>
void perfect_forward(T&& param) {
    target(std::forward<T>(param));
}

template<typename T>
void always_lvalue(T&& param) {
    target(param);
}

int main() {
    int x = 42;

    std::cout << "--- 完美转发 ---\n";
    perfect_forward(x);
    perfect_forward(100);

    std::cout << "--- 不用forward ---\n";
    always_lvalue(x);
    always_lvalue(100);
}
```

输出：

```
--- 完美转发 ---
左值重载: 42
右值重载: 100
--- 不用forward ---
左值重载: 42
左值重载: 100     ← 右值变成了左值！
```

`std::forward` 的简化实现：

```cpp
template<typename T>
constexpr T&& forward(std::remove_reference_t<T>& t) noexcept {
    return static_cast<T&&>(t);
}

template<typename T>
constexpr T&& forward(std::remove_reference_t<T>&& t) noexcept {
    static_assert(!std::is_lvalue_reference_v<T>,
                  "不能将左值转发为右值");
    return static_cast<T&&>(t);
}
```

关键理解：

| T推导为 | `static_cast<T&&>(param)` 结果 | 行为 |
|--------|-------------------------------|------|
| `int&` | `static_cast<int& &&>` → `int&` | 转发为左值 |
| `int` | `static_cast<int&&>` | 转发为右值 |

***

### 6. make_unique/make_shared如何使用完美转发

```cpp
#include <iostream>
#include <memory>
#include <string>
#include <utility>

struct User {
    std::string name;
    int age;

    User(const std::string& n, int a) : name(n), age(a) {
        std::cout << "构造 User: " << name << ", " << age << "\n";
    }
};

struct BigData {
    std::string payload;

    BigData(std::string&& p) : payload(std::move(p)) {
        std::cout << "移动构造 BigData\n";
    }

    BigData(const BigData&) = delete;
    BigData(BigData&&) = default;
};

template<typename T, typename... Args>
std::unique_ptr<T> my_make_unique(Args&&... args) {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
}

int main() {
    auto u1 = my_make_unique<User>("Alice", 30);

    std::string data(1000, 'X');
    auto u2 = my_make_unique<BigData>(std::move(data));

    std::cout << "data 移动后大小: " << data.size() << "\n";
}
```

输出：

```
构造 User: Alice, 30
移动构造 BigData
data 移动后大小: 0
```

如果不用完美转发，`std::move(data)` 传入后会退化为左值引用，导致拷贝而非移动。

***

### 7. emplace_back与完美转发

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <utility>

struct Log {
    std::string msg;

    Log(const std::string& s) : msg(s) {
        std::cout << "拷贝构造: " << msg << "\n";
    }

    Log(std::string&& s) : msg(std::move(s)) {
        std::cout << "移动构造: " << msg << "\n";
    }

    Log(const char* s) : msg(s) {
        std::cout << "const char*构造: " << msg << "\n";
    }
};

int main() {
    std::vector<Log> v;
    v.reserve(6);

    std::cout << "--- push_back ---\n";
    std::string s1 = "hello";
    v.push_back(s1);
    v.push_back(std::move(s1));

    std::cout << "--- emplace_back ---\n";
    std::string s2 = "world";
    v.emplace_back(s2);
    v.emplace_back(std::move(s2));
    v.emplace_back("direct");
}
```

输出：

```
--- push_back ---
拷贝构造: hello
移动构造: hello
--- emplace_back ---
拷贝构造: world
移动构造: world
const char*构造: direct     ← 直接原地构造，无需临时对象
```

对比：

| 操作 | push_back | emplace_back |
|------|-----------|-------------|
| 传左值 | 创建临时对象→拷贝 | 原地拷贝构造 |
| 传右值 | 创建临时对象→移动 | 原地移动构造 |
| 传构造参数 | ❌不支持 | ✅直接原地构造 |
| 完美转发 | 不涉及 | 内部使用 |

***

### 8. 完美转发失效的场景

完美转发并非万能，以下场景会失效：

```cpp
#include <iostream>
#include <utility>
#include <vector>

template<typename T>
void forwarder(T&& arg) {
    target(std::forward<T>(arg));
}

void target(int) { std::cout << "int\n"; }

void demo_brace_init() {
    std::vector<int> v;
    v.emplace_back(42);
    v.emplace_back({1, 2, 3});
}

template<typename T>
void wrap(T&& arg) {
    inner(std::forward<T>(arg));
}

void inner(int& x)  { std::cout << "左值: " << x << "\n"; }
void inner(int&& x) { std::cout << "右值: " << x << "\n"; }

int global_val = 100;

template<typename T>
void forward_global() {
    inner(std::forward<T>(global_val));
}

int main() {
    std::cout << "--- 位域 ---\n";

    struct Bits { unsigned a : 3; unsigned b : 5; };
    Bits bits{3, 5};
    auto val = bits.a;
    inner(std::forward<decltype(val)>(val));

    std::cout << "--- 全局变量 ---\n";
    forward_global<int&>();
    forward_global<int>();

    std::cout << "--- static_cast场景 ---\n";
    short s = 42;
    inner(std::forward<int>(static_cast<int>(s)));
}
```

完美转发失效场景汇总：

| 场景 | 原因 | 解决方案 |
|------|------|---------|
| 花括号初始化列表 | `{1,2,3}`无法推导为`initializer_list` | 先声明auto变量再转发 |
| 位域(bit-field) | 无法取地址 | 先拷贝到临时变量 |
| 重载函数名/函数指针歧义 | 无法推导唯一T | 用static_cast明确类型 |
| 0/NULL作为指针 | 0推导为int而非指针 | 用nullptr |
| 静态数组退化 | 数组退化为指针 | 用引用或std::array |

```cpp
#include <iostream>
#include <utility>
#include <initializer_list>

template<typename T>
void call(T&& arg) {
    std::cout << "调用成功\n";
}

template<typename T>
void perfect_call(T&& arg) {
    call(std::forward<T>(arg));
}

int main() {
    auto il = {1, 2, 3};
    perfect_call(il);

    struct Bits { unsigned x : 4; };
    Bits b{7};
    unsigned tmp = b.x;
    perfect_call(tmp);
}
```

***

### 9. std::forward vs std::move

```cpp
#include <iostream>
#include <utility>
#include <string>

void show(std::string& s)  { std::cout << "左值: " << s << "\n"; }
void show(std::string&& s) { std::cout << "右值: " << s << "\n"; }

template<typename T>
void use_forward(T&& x) {
    show(std::forward<T>(x));
}

template<typename T>
void use_move(T&& x) {
    show(std::move(x));
}

int main() {
    std::string s = "hello";

    std::cout << "--- std::forward ---\n";
    use_forward(s);
    use_forward(std::move(s));

    s = "world";
    std::cout << "--- std::move ---\n";
    use_move(s);
    use_move(std::move(s));
}
```

输出：

```
--- std::forward ---
左值: hello
右值: hello
--- std::move ---
右值: world      ← 左值被强制转为右值！
右值: world
```

核心区别：

| 特性 | std::move | std::forward |
|------|-----------|-------------|
| 本质 | 无条件转为右值 | 有条件转为右值 |
| 用途 | 明确表示"我要移动" | 模板中保持值类别 |
| 使用场景 | 调用方决定移动 | 中间层透传 |
| 参数 | 无模板参数 | 必须指定T |
| 误用后果 | 左值被意外移动 | 编译期安全检查 |

**使用原则**：

- 在**最终消费**参数时用 `std::move`
- 在**转发**参数时用 `std::forward`
- 永远不要对 `std::forward` 使用不带模板参数的写法

***

### 10. 完美转发的工程实践模式

```cpp
#include <iostream>
#include <utility>
#include <string>
#include <vector>

class Logger {
public:
    enum class Level { Debug, Info, Warn, Error };

    template<typename... Args>
    void log(Level lvl, Args&&... args) {
        std::cout << "[" << level_str(lvl) << "] ";
        log_impl(std::forward<Args>(args)...);
        std::cout << "\n";
    }

private:
    const char* level_str(Level l) {
        switch (l) {
            case Level::Debug: return "DEBUG";
            case Level::Info:  return "INFO";
            case Level::Warn:  return "WARN";
            case Level::Error: return "ERROR";
        }
        return "UNKNOWN";
    }

    template<typename T, typename... Rest>
    void log_impl(T&& first, Rest&&... rest) {
        std::cout << std::forward<T>(first);
        log_impl(std::forward<Rest>(rest)...);
    }

    void log_impl() {}
};

struct Event {
    std::string name;
    int code;

    Event(std::string n, int c) : name(std::move(n)), code(c) {}
};

int main() {
    Logger logger;

    std::string msg = "连接失败";
    logger.log(Logger::Level::Error, "事件: ", msg, " 代码: ", 500);

    std::string event_name = "超时";
    logger.log(Logger::Level::Warn, "事件名: ", std::move(event_name));
}
```

完美转发使用检查清单：

| 检查项 | 正确做法 |
|--------|---------|
| 函数签名 | `template<typename T> void f(T&& param)` |
| 转发调用 | `std::forward<T>(param)` |
| 多参数 | 每个参数独立 `T1&&, T2&&` |
| 类模板 | 需要辅助模板函数 |
| 返回值转发 | `decltype(auto)` + `return std::forward<T>(param)` |
| 成员初始化 | 构造函数中 `std::forward<T>(param)` |

***

### 11. 极简总结

| 概念 | 要点 |
|------|------|
| 转发引用 | 模板中 `T&&`，能匹配左值和右值 |
| 引用折叠 | 有左值引用则折叠为左值引用 |
| std::forward | 有条件转换：T为左值引用则转发左值，否则转发右值 |
| std::move | 无条件转为右值 |
| 标准库应用 | make_unique、make_shared、emplace_back |
| 失效场景 | 花括号初始化、位域、重载歧义 |
| 核心价值 | 零开销透传参数的值类别 |

***

### 相关阅读

- [左值右值与将亡值](./21-左值右值与将亡值.md)
- [std-move与std-forward](./22-std-move与std-forward.md)
- [decltype与auto](./33-decltype与auto.md)