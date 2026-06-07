# std::move 与 std::forward 的区别
> 📖 相关章节：[移动语义与完美转发](../../02-CPP/09-移动语义与完美转发.md)

### 1. 核心提炼

**std::move = 无条件转为右值引用（贴"可搬走"标签），std::forward = 有条件转发（保持原来的值类别不变）。模板中用 forward，非模板中用 move。**

***

### 2. 核心定义

| | std::move | std::forward |
|---|---|---|
| 做了什么 | 无条件把参数转成右值引用 | 有条件地保持参数的值类别 |
| 本质 | `static_cast<T&&>(x)` | `static_cast<T&&>(x)`（但 T 包含类型信息） |
| 条件性 | 无条件，永远转右值 | 有条件，传左值就转左值，传右值就转右值 |
| 典型场景 | 明确表示"我不再需要这个对象" | 模板中转发参数，保持原始值类别 |

**源码简化版**：

```cpp
// std::move 的本质
template<typename T>
constexpr typename std::remove_reference<T>::type&&
move(T&& t) noexcept {
    return static_cast<typename std::remove_reference<T>::type&&>(t);
}
// 无论 T 是什么，返回值一定是 &&（右值引用）

// std::forward 的本质
// 情况1：T 是左值引用（如 int&）
template<typename T>
constexpr T&& forward(typename std::remove_reference<T>::type& t) noexcept {
    return static_cast<T&&>(t);
    // T = int& → T&& = int& && = int& → 返回左值引用
}

// 情况2：T 是右值引用（如 int&&）
// T = int&& → T&& = int&& && = int&& → 返回右值引用
// 引用折叠规则：T& & = T&，T&& & = T&，T& && = T&，T&& && = T&&
```

***

### 3. 生活类比

| | std::move | std::forward |
|---|---|---|
| 类比 | 给东西贴上"可搬走"标签 | 快递员原样传递包裹 |
| 说明 | 不管东西是什么，一律贴"可搬走" | 包裹是易碎品就按易碎品送，是普通件就按普通件送 |
| 风险 | 贴错了标签，东西可能被误搬 | 不改变包裹性质，安全 |

**具体场景**：

- **std::move**：你搬家时，把所有箱子都贴上"可搬走"标签。不管箱子里是贵重物品还是垃圾，搬家公司都可以拿走。
- **std::forward**：你是快递中转站，收到什么类型的包裹就按什么类型转发。易碎品（左值）还是易碎品，加急件（右值）还是加急件，不改变性质。

***

### 4. std::move 的典型用法和误区

**正确用法1：传入右值引用参数**

```cpp
class Widget {
    std::string name_;
public:
    void setName(std::string n) {
        name_ = std::move(n);  // ✅ n 是函数参数，即将销毁，move 走移动赋值
    }
};
```

**正确用法2：在容器中移动元素**

```cpp
std::vector<std::string> names;
std::string s = "hello";
names.push_back(std::move(s));  // ✅ 明确表示不再需要 s，走移动构造
// s 现在是空字符串（valid but unspecified state）
```

**正确用法3：移动构造/移动赋值中**

```cpp
class Buffer {
    int* data_;
    size_t size_;
public:
    Buffer(Buffer&& other) noexcept
        : data_(other.data_), size_(other.size_) {
        other.data_ = nullptr;
        other.size_ = 0;
    }

    Buffer& operator=(Buffer&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            other.data_ = nullptr;
            other.size_ = 0;
        }
        return *this;
    }
};
```

**误区1：对返回值用 std::move**

```cpp
// ❌ 错误：阻止 NRVO
std::string create() {
    std::string s = "hello";
    return std::move(s);  // 阻止 NRVO，强制走移动构造
}

// ✅ 正确：让编译器做 NRVO
std::string create() {
    std::string s = "hello";
    return s;  // NRVO 优先，最坏也是移动
}
```

**误区2：move 后继续使用源对象**

```cpp
std::string s = "hello";
std::string s2 = std::move(s);

// s 处于"valid but unspecified"状态
// 可以赋值、可以析构，但值不确定
cout << s;   // 可能输出空，也可能输出 "hello"，取决于实现
s = "world"; // ✅ 赋值是安全的
cout << s;   // "world"
```

**误区3：在函数内对参数重复 move**

```cpp
void process(std::string&& s) {
    std::string a = std::move(s);  // 第一次 move，s 的资源被偷走
    std::string b = std::move(s);  // 第二次 move，s 已经空了，b 得到空字符串
    // 这不是 bug（不会崩溃），但通常不是你想要的
}
```

***

### 5. std::forward 在模板中的用法

**核心场景：完美转发（Perfect Forwarding）**

```cpp
// 不用 forward：值类别丢失
template<typename T>
void wrapper(T&& arg) {
    process(arg);  // arg 永远是 lvalue！不管外面传的是左值还是右值
}

// 用 forward：保持原始值类别
template<typename T>
void wrapper(T&& arg) {
    process(std::forward<T>(arg));  // 传左值就转发左值，传右值就转发右值
}
```

**为什么需要 forward**：因为具名的右值引用是左值（见 FAQ 79），forward 恢复它的原始值类别。

**完整示例：万能包装器**

```cpp
class Widget {
    std::string name_;
    int value_;
public:
    // 不用 forward：总是走拷贝
    template<typename S>
    void setName(S&& s) {
        name_ = s;  // 总是拷贝，即使外面传的是右值
    }

    // 用 forward：传左值走拷贝，传右值走移动
    template<typename S>
    void setName2(S&& s) {
        name_ = std::forward<S>(s);  // S = string& → 拷贝；S = string&& → 移动
    }
};

std::string s = "hello";
Widget w;
w.setName2(s);                  // S = std::string&，forward 转发左值 → 拷贝
w.setName2(std::move(s));       // S = std::string&&，forward 转发右值 → 移动
w.setName2(std::string("hi"));  // S = std::string&&，forward 转发右值 → 移动
```

**多参数完美转发**

```cpp
template<typename... Args>
void makeWidget(Args&&... args) {
    // forward 每个参数，保持各自的值类别
    auto ptr = std::make_unique<Widget>(std::forward<Args>(args)...);
}
```

***

### 6. 两者在非模板场景下的行为差异

```cpp
void foo(std::string& s) {        // 左值引用版本
    cout << "lvalue\n";
}
void foo(std::string&& s) {       // 右值引用版本
    cout << "rvalue\n";
}

// === 非模板场景 ===
void nonTemplate(std::string&& s) {
    // s 的类型是 string&&，但作为表达式是 lvalue

    foo(std::move(s));      // ✅ 无条件转右值 → 调用 rvalue 版本
    foo(std::forward<std::string&&>(s));  // ✅ 结果一样，但写法啰嗦
    foo(std::forward<std::string>(s));    // ✅ 结果也一样（T=string 不是引用 → T&&=string&&）

    // 在非模板中，move 和 forward 效果相同
    // 但 move 更直白，语义更清晰
}

// === 模板场景 ===
template<typename T>
void templated(T&& s) {
    // T 可能是 string& 也可能是 string&&

    foo(std::move(s));       // ❌ 总是转右值！传左值进来也被转成右值了
    foo(std::forward<T>(s)); // ✅ 保持原始值类别
}

std::string str = "hello";
templated(str);              // T = std::string& → forward 转发左值 → 调用 lvalue 版本
templated(std::move(str));   // T = std::string&& → forward 转发右值 → 调用 rvalue 版本
```

**关键区别**：在模板中，`T&&` 是万能引用（forwarding reference），`T` 的推导结果包含了值类别信息。`std::forward<T>` 利用这个信息恢复原始值类别。`std::move` 无视这些信息，一律转右值。

***

### 7. 对比表格

| 特性 | std::move | std::forward |
|------|-----------|-------------|
| 条件性 | 无条件，永远转右值 | 有条件，保持原始值类别 |
| 参数类型 | 任意类型 | 需要模板类型参数 T |
| 返回类型 | `remove_reference_t<T>&&` | `T&&`（依赖引用折叠） |
| 典型场景 | 非模板中明确表示"可移动" | 模板中转发参数 |
| 常见误用 | 对返回值用 move（阻止 NRVO） | 在非模板中用 forward（语义不清） |
| 语义 | "我不再需要这个对象" | "原样传递，不改变性质" |
| 安全性 | 可能误搬不该搬的东西 | 保持原始语义，更安全 |

***

### 8. 关键规则

**规则1：在模板中用 forward**

```cpp
template<typename T>
void wrapper(T&& arg) {
    target(std::forward<T>(arg));  // ✅ 保持值类别
}
```

**规则2：在非模板中用 move**

```cpp
void setName(std::string name) {
    name_ = std::move(name);  // ✅ name 是值参数，即将销毁，move 语义清晰
}

void process(std::string&& s) {
    target(std::move(s));  // ✅ 非模板，move 更直白
    // 不要写 std::forward<std::string>(s)，啰嗦且不直观
}
```

**规则3：不要对返回值用 move**

```cpp
// ❌
std::string create() {
    std::string s = "hello";
    return std::move(s);  // 阻止 NRVO
}

// ✅
std::string create() {
    std::string s = "hello";
    return s;  // NRVO 优先
}
```

**规则4：不要在非模板中用 forward**

```cpp
// ❌ 啰嗦且容易写错
void process(std::string&& s) {
    target(std::forward<std::string&&>(s));
}

// ✅ 直白
void process(std::string&& s) {
    target(std::move(s));
}
```

**规则5：move 后不要使用源对象的值**

```cpp
std::string s = "hello";
std::string s2 = std::move(s);
// s 处于 valid but unspecified 状态
// 可以对 s 赋值、可以析构 s，但不要读取 s 的值
s = "new value";  // ✅ 赋值是安全的
```

***

### 9. 完整示例

```cpp
#include <iostream>
#include <string>
#include <utility>
using namespace std;

struct Data {
    string name;
    int value;

    Data(const string& n, int v) : name(n), value(v) {
        cout << "  Data(lvalue, int): " << name << "\n";
    }
    Data(string&& n, int v) : name(std::move(n)), value(v) {
        cout << "  Data(rvalue, int): " << name << "\n";
    }
};

class Container {
    Data data_;
public:
    // 模板构造函数：完美转发
    template<typename S>
    Container(S&& name, int v)
        : data_(std::forward<S>(name), v) {
        cout << "  Container constructed\n";
    }
};

int main() {
    string s = "hello";

    cout << "=== 传左值 ===\n";
    Container c1(s, 42);
    // forward<S>(name) → S = string& → 转发左值 → Data(lvalue, int)

    cout << "\n=== 传右值 ===\n";
    Container c2(std::move(s), 42);
    // forward<S>(name) → S = string&& → 转发右值 → Data(rvalue, int)

    cout << "\n=== 传临时对象 ===\n";
    Container c3(string("world"), 42);
    // forward<S>(name) → S = string&& → 转发右值 → Data(rvalue, int)

    return 0;
}
```

**预期输出**：

```
=== 传左值 ===
  Data(lvalue, int): hello
  Container constructed

=== 传右值 ===
  Data(rvalue, int): hello
  Container constructed

=== 传临时对象 ===
  Data(rvalue, int): world
  Container constructed
```

***

### 10. 极简总结

**move = 无条件转右值（贴"可搬走"标签）| forward = 有条件转发（保持原样）| 模板中用 forward | 非模板中用 move | 不要对返回值用 move（阻止 NRVO）| move 后不要读源对象的值 | forward 依赖引用折叠恢复值类别**

***

### 相关阅读

- [左值右值与将亡值](18-左值右值与将亡值.md)
- [什么是完美转发Perfect-Forwarding](20-什么是完美转发Perfect-Forwarding.md)
- [Rule-of-Five与Rule-of-Zero](29-Rule-of-Five与Rule-of-Zero.md)