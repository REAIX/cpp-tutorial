# 什么是if-constexpr
> 📖 相关章节：[模板进阶](../../02-CPP/11-模板进阶.md)

> 编译期的if，让模板分支不再需要SFINAE的黑魔法。

***

### 1. 核心要义

`if constexpr` 是C++17引入的编译期条件判断：条件为常量表达式，编译器只编译匹配的分支，未选中分支的代码甚至不需要通过语法检查——这让模板中的条件分支变得简单直观。

***

### 2. if-constexpr基本语法

```cpp
#include <iostream>
#include <type_traits>
#include <string>

template<typename T>
std::string describe(T val) {
    if constexpr (std::is_integral_v<T>) {
        return "整数: " + std::to_string(val);
    } else if constexpr (std::is_floating_point_v<T>) {
        return "浮点数: " + std::to_string(val);
    } else if constexpr (std::is_pointer_v<T>) {
        return "指针指向: " + std::to_string(*val);
    } else {
        return "其他类型";
    }
}

int main() {
    std::cout << describe(42) << "\n";
    std::cout << describe(3.14) << "\n";

    int x = 100;
    std::cout << describe(&x) << "\n";
    std::cout << describe("hello") << "\n";
}
```

输出：

```
整数: 42
浮点数: 3.14
指针指向: 100
其他类型
```

`if constexpr` vs 普通 `if`：

| 特性 | if constexpr | if |
|------|-------------|-----|
| 条件 | 必须是编译期常量 | 运行时表达式 |
| 未选中分支 | 完全不编译 | 仍需语法正确 |
| 生成代码 | 只生成匹配分支 | 两个分支都生成 |
| 运行时开销 | 零 | 有分支判断 |
| 适用场景 | 模板元编程 | 运行时逻辑 |

***

### 3. 替代SFINAE的写法对比

`if constexpr` 大幅简化了原本需要SFINAE才能实现的编译期分支：

**SFINAE方式（C++11）**：

```cpp
#include <iostream>
#include <type_traits>
#include <string>
#include <vector>

template<typename T>
typename std::enable_if<std::is_integral_v<T>, std::string>::type
process(T val) {
    return "整数处理: " + std::to_string(val);
}

template<typename T>
typename std::enable_if<std::is_floating_point_v<T>, std::string>::type
process(T val) {
    return "浮点处理: " + std::to_string(val);
}

template<typename T>
typename std::enable_if<
    !std::is_integral_v<T> && !std::is_floating_point_v<T>,
    std::string
>::type
process(T val) {
    return "其他处理";
}
```

**if constexpr方式（C++17）**：

```cpp
template<typename T>
std::string process(T val) {
    if constexpr (std::is_integral_v<T>) {
        return "整数处理: " + std::to_string(val);
    } else if constexpr (std::is_floating_point_v<T>) {
        return "浮点处理: " + std::to_string(val);
    } else {
        return "其他处理";
    }
}

int main() {
    std::cout << process(42) << "\n";
    std::cout << process(3.14) << "\n";
    std::cout << process("hello") << "\n";
}
```

对比：

| 维度 | SFINAE | if constexpr |
|------|--------|-------------|
| 代码量 | 多（每个分支一个函数） | 少（一个函数内） |
| 可读性 | 差（enable_if嵌套） | 好（直观的if-else） |
| 编译错误 | 难以理解 | 清晰 |
| 函数数量 | N个重载 | 1个函数 |
| 维护性 | 低 | 高 |

***

### 4. type_traits + if-constexpr实战

```cpp
#include <iostream>
#include <type_traits>
#include <string>
#include <vector>
#include <list>

template<typename Container>
std::size_t fast_size(const Container& c) {
    if constexpr (std::is_same_v<typename Container::iterator::iterator_category,
                                  std::random_access_iterator_tag>) {
        return c.end() - c.begin();
    } else {
        return std::distance(c.begin(), c.end());
    }
}

template<typename T>
void serialize(const T& val) {
    if constexpr (std::is_arithmetic_v<T>) {
        std::cout << "数值: " << val << "\n";
    } else if constexpr (std::is_same_v<T, std::string>) {
        std::cout << "字符串: \"" << val << "\" (长度 " << val.size() << ")\n";
    } else if constexpr (std::is_pointer_v<T>) {
        std::cout << "指针: " << static_cast<const void*>(val) << "\n";
    } else {
        std::cout << "未知类型, sizeof = " << sizeof(T) << "\n";
    }
}

template<typename T>
T safe_divide(T a, T b) {
    if constexpr (std::is_integral_v<T>) {
        if (b == 0) {
            std::cout << "整数除零! 返回0\n";
            return T{0};
        }
        return a / b;
    } else {
        return a / b;
    }
}

int main() {
    std::vector<int> vec{1, 2, 3, 4, 5};
    std::list<int> lst{1, 2, 3, 4, 5};

    std::cout << "vector size: " << fast_size(vec) << "\n";
    std::cout << "list size: " << fast_size(lst) << "\n";

    serialize(42);
    serialize(3.14);
    serialize(std::string("hello"));

    std::cout << safe_divide(10, 3) << "\n";
    std::cout << safe_divide(10.0, 3.0) << "\n";
}
```

***

### 5. 编译期分支的关键特性

未选中分支的代码不会被编译，这是 `if constexpr` 最强大的特性：

```cpp
#include <iostream>
#include <type_traits>
#include <string>

template<typename T>
std::string to_string_safe(T val) {
    if constexpr (std::is_same_v<T, std::string>) {
        return val;
    } else if constexpr (std::is_arithmetic_v<T>) {
        return std::to_string(val);
    } else {
        return val.toString();
    }
}

struct HasToString {
    int data;
    std::string toString() const { return "HasToString(" + std::to_string(data) + ")"; }
};

int main() {
    std::cout << to_string_safe(42) << "\n";
    std::cout << to_string_safe(std::string("hello")) << "\n";
    std::cout << to_string_safe(HasToString{99}) << "\n";
}
```

如果用普通 `if`，`std::to_string(val)` 在 `T=HasToString` 时会编译失败，因为 `std::to_string` 不接受 `HasToString`。而 `if constexpr` 让未选中分支完全不参与编译。

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void inspect(T& val) {
    if constexpr (std::is_const_v<T>) {
        std::cout << "const类型\n";
    } else {
        val = T{};
        std::cout << "非const类型，已重置\n";
    }
}

int main() {
    int x = 42;
    const int cx = 42;

    inspect(x);
    inspect(cx);
}
```

输出：

```
非const类型，已重置
const类型
```

如果 `T=const int` 时 `val = T{}` 被编译，会报赋值只读变量的错误。但 `if constexpr` 保证了这行代码不会被编译。

***

### 6. 在模板中的典型应用

```cpp
#include <iostream>
#include <type_traits>
#include <vector>
#include <string>

template<typename T>
class SmartContainer {
    std::vector<T> data_;

public:
    void add(const T& val) {
        if constexpr (std::is_same_v<T, bool>) {
            data_.push_back(val);
        } else if constexpr (std::is_arithmetic_v<T>) {
            data_.push_back(val * 2);
        } else {
            data_.push_back(val);
        }
    }

    void print() const {
        if constexpr (std::is_floating_point_v<T>) {
            std::cout.precision(2);
        }
        for (const auto& v : data_) {
            std::cout << v << " ";
        }
        std::cout << "\n";
    }

    T accumulate() const {
        if constexpr (std::is_same_v<T, std::string>) {
            std::string result;
            for (const auto& s : data_) result += s;
            return result;
        } else {
            T result{};
            for (const auto& v : data_) result += v;
            return result;
        }
    }
};

int main() {
    SmartContainer<int> ci;
    ci.add(10);
    ci.add(20);
    ci.print();
    std::cout << "累加: " << ci.accumulate() << "\n";

    SmartContainer<std::string> cs;
    cs.add("hello ");
    cs.add("world");
    cs.print();
    std::cout << "拼接: " << cs.accumulate() << "\n";
}
```

输出：

```
20 40
累加: 60
hello  world
拼接: hello world
```

***

### 7. C++20泛型Lambda中的if-constexpr

C++20允许在泛型lambda中使用 `if constexpr`，让lambda拥有模板的能力：

```cpp
#include <iostream>
#include <type_traits>
#include <vector>
#include <string>

auto process = [](auto& container) {
    using Container = std::decay_t<decltype(container)>;
    using Value = typename Container::value_type;

    if constexpr (std::is_arithmetic_v<Value>) {
        Value sum{};
        for (const auto& v : container) sum += v;
        std::cout << "数值容器求和: " << sum << "\n";
    } else if constexpr (std::is_same_v<Value, std::string>) {
        std::string result;
        for (const auto& v : container) result += v;
        std::cout << "字符串容器拼接: " << result << "\n";
    } else {
        std::cout << "未知元素类型, 大小: " << container.size() << "\n";
    }
};

auto get_value = [](auto val) {
    using T = decltype(val);

    if constexpr (std::is_pointer_v<T>) {
        return *val;
    } else if constexpr (std::is_arithmetic_v<T>) {
        return val;
    } else {
        return val.size();
    }
};

int main() {
    std::vector<int> vi{1, 2, 3, 4, 5};
    std::vector<std::string> vs{"a", "b", "c"};

    process(vi);
    process(vs);

    int x = 42;
    std::string s = "hello";

    std::cout << "int值: " << get_value(x) << "\n";
    std::cout << "指针值: " << get_value(&x) << "\n";
    std::cout << "字符串大小: " << get_value(s) << "\n";
}
```

C++20前后的lambda能力对比：

| 能力 | C++14 | C++17 | C++20 |
|------|-------|-------|-------|
| 泛型lambda | `auto` 参数 | `auto` 参数 | 模板参数 `<typename T>` |
| if constexpr | ❌ | ✅ | ✅ |
| 显式模板参数 | ❌ | ❌ | `[]<typename T>(T)` |
| 包展开 | ❌ | ❌ | ✅ |

C++20显式模板lambda + if constexpr：

```cpp
#include <iostream>
#include <type_traits>

auto typed_process = []<typename T>(T val) {
    if constexpr (std::is_integral_v<T>) {
        std::cout << "整数: " << val << "\n";
    } else if constexpr (std::is_floating_point_v<T>) {
        std::cout << "浮点: " << val << "\n";
    } else {
        std::cout << "其他\n";
    }
};

int main() {
    typed_process(42);
    typed_process(3.14);
    typed_process("hello");
}
```

***

### 8. if-constexpr vs #if预处理器

```cpp
#include <iostream>
#include <type_traits>

#ifdef USE_FLOAT
using ValueType = float;
#else
using ValueType = double;
#endif

template<typename T>
void process(T val) {
    if constexpr (std::is_same_v<T, float>) {
        std::cout << "float处理: " << val << "f\n";
    } else if constexpr (std::is_same_v<T, double>) {
        std::cout << "double处理: " << val << "\n";
    }
}

int main() {
    process(ValueType{3.14});
    process(42);
}
```

对比：

| 维度 | if constexpr | #if / #ifdef |
|------|-------------|-------------|
| 时机 | 模板实例化时 | 预处理阶段 |
| 条件 | 类型信息、常量表达式 | 宏定义 |
| 作用域 | 函数/类内部 | 全局 |
| 调试 | 可在调试器中看到 | 预处理后代码消失 |
| 类型感知 | ✅ 可用type_traits | ❌ 不知类型 |
| 灵活性 | 每个实例化可不同 | 编译单元内统一 |
| 安全性 | 编译器类型检查 | 纯文本替换 |

> ⚠️ `#if` 在预处理阶段执行，此时模板尚未实例化，无法获取任何类型信息。`if constexpr` 在模板实例化时执行，可以使用 `type_traits`。

***

### 9. if-constexpr的限制

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void demo(T val) {
    if constexpr (sizeof(T) > 4) {
        std::cout << "大类型\n";
    } else {
        std::cout << "小类型\n";
    }
}

template<typename T>
std::string bad_example(T val) {
    if constexpr (std::is_integral_v<T>) {
        return std::to_string(val);
    }
}

template<typename T>
std::string good_example(T val) {
    if constexpr (std::is_integral_v<T>) {
        return std::to_string(val);
    } else {
        return "非整数";
    }
}

template<int N>
int fibonacci() {
    if constexpr (N <= 1) {
        return N;
    } else {
        return fibonacci<N - 1>() + fibonacci<N - 2>();
    }
}

extern int runtime_val;

template<typename T>
void cannot_use_runtime(T val) {
    if constexpr (val > 0) {
    }
}

int main() {
    demo(42);
    demo(3.14);

    std::cout << good_example(42) << "\n";
    std::cout << good_example(3.14) << "\n";

    std::cout << "fib(10) = " << fibonacci<10>() << "\n";
}
```

限制汇总：

| 限制 | 说明 | 解决方案 |
|------|------|---------|
| 条件必须是常量表达式 | 不能用运行时变量 | 用普通if |
| 所有分支必须语法合法 | 未选中分支的模板参数仍需有效 | 确保模板参数合法 |
| 无else分支时可能无返回 | 非void函数需要所有路径有返回 | 加else分支 |
| 不能替代SFINAE所有场景 | 函数签名不变，不能改变重载集 | 仍需SFINAE/Concepts |
| 外部模板实例化 | 未选中分支可能被外部实例化触发 | 注意ODR |

**关键区别：if constexpr不能改变函数签名**

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
auto get_value(T t) {
    if constexpr (std::is_pointer_v<T>) {
        return *t;
    } else {
        return t;
    }
}

int main() {
    int x = 42;
    std::cout << get_value(x) << "\n";
    std::cout << get_value(&x) << "\n";
}
```

虽然返回类型不同，但 `auto` 推导让这成为可能。如果需要函数签名本身不同（如不同的模板参数），仍需SFINAE或Concepts。

***

### 10. if-constexpr高级模式

**编译期字符串处理**：

```cpp
#include <iostream>
#include <string_view>

template<std::size_t N>
constexpr std::size_t count_digits() {
    if constexpr (N < 10) return 1;
    else if constexpr (N < 100) return 2;
    else if constexpr (N < 1000) return 3;
    else if constexpr (N < 10000) return 4;
    else return 5;
}

template<typename T>
constexpr const char* type_suffix() {
    if constexpr (std::is_same_v<T, float>) return "f";
    else if constexpr (std::is_same_v<T, double>) return "";
    else if constexpr (std::is_same_v<T, long double>) return "L";
    else if constexpr (std::is_same_v<T, unsigned int>) return "U";
    else if constexpr (std::is_same_v<T, long>) return "L";
    else return "";
}

template<bool IsConst, bool IsVolatile>
void print_cv_qualifiers() {
    std::cout << "限定符: ";
    if constexpr (IsConst) std::cout << "const ";
    if constexpr (IsVolatile) std::cout << "volatile ";
    if constexpr (!IsConst && !IsVolatile) std::cout << "无";
    std::cout << "\n";
}

int main() {
    std::cout << "42 的位数: " << count_digits<42>() << "\n";
    std::cout << "1234 的位数: " << count_digits<1234>() << "\n";

    std::cout << "float后缀: " << type_suffix<float>() << "\n";
    std::cout << "double后缀: " << type_suffix<double>() << "\n";

    print_cv_qualifiers<true, false>();
    print_cv_qualifiers<false, true>();
    print_cv_qualifiers<false, false>();
}
```

**递归编译期计算**：

```cpp
#include <iostream>
#include <type_traits>

template<unsigned N>
struct Factorial {
    static constexpr unsigned value = N * Factorial<N - 1>::value;
};

template<>
struct Factorial<0> {
    static constexpr unsigned value = 1;
};

constexpr unsigned factorial(unsigned n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

template<unsigned N>
constexpr unsigned factorial_constexpr() {
    if constexpr (N <= 1) return 1;
    else return N * factorial_constexpr<N - 1>();
}

int main() {
    std::cout << "模板元编程: " << Factorial<10>::value << "\n";
    std::cout << "constexpr函数: " << factorial(10) << "\n";
    std::cout << "if constexpr: " << factorial_constexpr<10>() << "\n";
}
```

三种编译期计算方式对比：

| 方式 | C++版本 | 可读性 | 调试性 | 灵活性 |
|------|---------|--------|--------|--------|
| 模板特化递归 | C++98 | 差 | 差 | 低 |
| constexpr函数 | C++11 | 好 | 好 | 中 |
| if constexpr | C++17 | 最好 | 好 | 高 |

***

### 11. 极简总结

| 概念 | 要点 |
|------|------|
| if constexpr | C++17编译期条件判断 |
| 核心特性 | 未选中分支不编译 |
| 替代SFINAE | 大幅简化模板条件分支 |
| 条件要求 | 必须是编译期常量表达式 |
| C++20增强 | 泛型lambda中可直接使用 |
| vs #if | if constexpr感知类型信息 |
| 限制 | 不能改变函数签名、条件必须编译期可求值 |
| 最佳实践 | 优先用if constexpr替代SFINAE简单场景 |

***

### 相关阅读

- [SFINAE与TypeTraits](./00-SFINAE与TypeTraits.md)
- [什么是标签分发Tag-Dispatch](./05-什么是标签分发Tag-Dispatch.md)
- [什么是C++20-Concepts](./06-什么是C++20-Concepts.md)

***