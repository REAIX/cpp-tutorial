# Lambda与函数对象

> 掌握C++ Lambda表达式与函数对象

---

> **Lambda: the ultimate way to write inline functions.** — someone
> （Lambda：写内联函数的终极方式。）

> **Lambda：匿名函数的复兴。**
> （Lambda: the renaissance of anonymous functions.）

---

> **🎯 代码如诗，Lambda让代码更优雅。**
> 
> （Lambda让代码更简洁、更易读。）

---

> 💡 **通俗理解 - Lambda是什么？**

想象你在餐厅点菜：
- **普通函数**：就像"去后厨告诉厨师要做宫保鸡丁"
- **Lambda**：就像"在菜单上勾选宫保鸡丁"

**Lambda就是".inline的简短操作"！**

```cpp
// 普通函数
void print(int x) {
    cout << x << endl;
}

// Lambda -  inline 的简短函数
auto print = [](int x) {
    cout << x << endl;
};
```

**Lambda的特点：**
- 不用取名（匿名）
- 可以在函数里定义（inline）
- 可以"捕获"周围的变量

**捕获就像"带材料"：**
- `[a]` - 带着a的值去（复制）
- `[&a]` - 带着a的"遥控器"去（引用）
- `[=]` - 把周围所有变量都复制一份
- `[&]` - 把周围所有变量的"遥控器"都带上

> 🔬 **抽象理解 - 函数式编程思想**：
> - **函数对象**：是"可以像函数一样调用的对象"，本质是"operator()重载"
> - **Lambda**：是"匿名函数对象"，是函数式编程的基础构件
> - **闭包**：是Lambda及其捕获变量的组合，"捕获"实现了闭包
> - **函数式编程**：是一种"声明式"编程范式，强调"做什么"而非"怎么做"
> - **Lambda的价值**：在于创建"一次性"的简短逻辑，使代码更简洁、表达力更强

---

## 前置知识
- [模板进阶](11-模板进阶.md)
## 后续内容
- [STL容器](14-STL容器.md)
## 目录

- [1. Lambda表达式](#1-lambda表达式)
- [2. 函数对象与std::function](#2-函数对象与stdfunction)
- [3. 模板最佳实践](#3-模板最佳实践)

---

## 1. Lambda表达式

### 1. 概念与定义

**Lambda表达式（lambda expression）**：C++11引入的新特性，用于创建匿名函数。Lambda表达式可以简化代码，提高可读性。例如`auto f = []() { std::cout << "Hello, World!" << std::endl; };`。

**捕获列表（capture list）**：Lambda表达式中用于捕获外部变量的机制。捕获列表可以分为值捕获、引用捕获、混合捕获等。例如`[a]`表示值捕获变量`a`，`[&a]`表示引用捕获变量`a`，`[=]`表示值捕获所有变量，`[&]`表示引用捕获所有变量。

**值捕获（value capture）**：Lambda表达式中按值捕获外部变量。值捕获的变量是只读的，除非加`mutable`关键字。例如`[a]() mutable { a = 100; }`表示值捕获变量`a`并允许修改。

**引用捕获（reference capture）**：Lambda表达式中按引用捕获外部变量。引用捕获的变量是可修改的。例如`[&a]() { a = 100; }`表示引用捕获变量`a`并允许修改。

**混合捕获（mixed capture）**：Lambda表达式中混合使用值捕获和引用捕获。例如`[=, &a]() { a = 100; }`表示值捕获所有变量，除了变量`a`按引用捕获。

**函数对象（function object）**：C++中用于实现函数调用的对象。函数对象可以用于STL算法、模板元编程等。例如`class Add { public: int operator()(int a, int b) const { return a + b; } };`。

**仿函数（functor）**：函数对象的别名。仿函数可以用于实现函数调用。例如`Add add; int result = add(1, 2);`。

**std::function**：C++11引入的新特性，用于存储可调用对象。`std::function`可以存储函数指针、Lambda表达式、函数对象等。例如`std::function<int(int, int)> add = [](int a, int b) { return a + b; };`。

### 2. 基本语法

```cpp
// Lambda 完整语法
[capture](parameters) mutable -> return_type { body }

// 各部分说明：
// [capture]    - 捕获列表，指定如何捕获外部变量
// (parameters) - 参数列表（可省略）
// mutable      - 允许修改按值捕获的变量（可选）
// -> return_type - 尾置返回类型（可省略，自动推导）
// { body }     - 函数体
```

### 3. 捕获方式详解

```cpp
#include <iostream>
#include <string>

int main() {
    int a = 10;
    int b = 20;
    const int c = 30;
    
    // 1. 不捕获任何变量
    auto f1 = []() { 
        std::cout << "No capture\n"; 
    };
    f1();
    
    // 2. 按值捕获单个变量（只读，除非加 mutable）
    auto f2 = [a]() { 
        // a = 100;  // 错误：不能修改值捕获的变量
        std::cout << "Value capture a = " << a << "\n"; 
    };
    f2();
    
    // 3. 按引用捕获单个变量（可修改）
    auto f3 = [&a]() { 
        a = 100;  // OK：修改外部变量
        std::cout << "Reference capture a = " << a << "\n"; 
    };
    f3();  // a 变为 100
    
    // 4. 按值捕获所有变量
    auto f4 = [=]() { 
        std::cout << "a=" << a << ", b=" << b << ", c=" << c << "\n"; 
    };
    f4();
    
    // 5. 按引用捕获所有变量
    auto f5 = [&]() { 
        a = 1; 
        b = 2; 
        // c = 3;  // 错误：const 变量不能修改
        std::cout << "a=" << a << ", b=" << b << "\n"; 
    };
    f5();
    
    // 6. 混合捕获：默认值捕获，a 按引用
    auto f6 = [=, &a]() { 
        a = 5;  // OK：a 是引用捕获
        // b = 10;  // 错误：b 是值捕获
        std::cout << "a=" << a << ", b=" << b << "\n"; 
    };
    f6();
    
    // 7. 混合捕获：默认引用捕获，b 按值
    auto f7 = [&, b]() { 
        a = 10;  // OK：a 是引用捕获
        // b = 20;  // 错误：b 是值捕获
        std::cout << "a=" << a << ", b=" << b << "\n"; 
    };
    f7();
    
    // 8. 初始化捕获（C++14）：移动语义
    std::string s = "Hello";
    auto f8 = [str = std::move(s)]() { 
        std::cout << "Moved: " << str << "\n"; 
    };
    f8();
    std::cout << "Original: " << (s.empty() ? "(empty)" : s) << "\n";  // s 已被移动
    
    // 9. 初始化捕获：表达式
    auto f9 = [x = a + b]() { 
        std::cout << "Expression: " << x << "\n"; 
    };
    f9();
    
    return 0;
}
```

### 4. 捕获对照表

| 捕获方式 | 说明 | 可修改外部变量 |
|---------|------|--------------|
| `[]` | 不捕获任何变量 | ❌ |
| `[=]` | 按值捕获所有变量 | ❌ |
| `[&]` | 按引用捕获所有变量 | ✅ |
| `[a]` | 按值捕获 a | ❌ |
| `[&a]` | 按引用捕获 a | ✅ |
| `[=, &a]` | 值捕获所有，a 按引用 | a 可修改 |
| `[&, a]` | 引用捕获所有，a 按值 | a 不可修改 |
| `[a = expr]` | 初始化捕获 | 取决于类型 |
| `[this]` | 捕获 this 指针 | ✅（成员变量） |
| `[*this]` | 捕获 *this 副本 | ❌ |

### 5. mutable 关键字

```cpp
#include <iostream>

int main() {
    int x = 10;
    
    // 不加 mutable：按值捕获的变量是只读的
    auto f1 = [x]() {
        // x = 20;  // 错误：不能修改
        return x;
    };
    
    // 加 mutable：可以修改捕获的副本（不影响原变量）
    auto f2 = [x]() mutable {
        x = 20;  // OK：修改的是副本
        return x;
    };
    
    std::cout << "f1: " << f1() << "\n";  // 10
    std::cout << "f2: " << f2() << "\n";  // 20
    std::cout << "x: " << x << "\n";       // 10（原变量未变）
    
    // 多次调用 f2，副本状态保持
    std::cout << "f2 again: " << f2() << "\n";  // 20（副本已被修改）
    
    return 0;
}
```

### 6. 泛型Lambda（C++14）

```cpp
#include <iostream>
#include <string>
#include <vector>

int main() {
    // 泛型 Lambda：使用 auto 参数
    // 相当于模板函数
    auto add = [](auto a, auto b) {
        return a + b;
    };
    
    std::cout << add(1, 2) << "\n";                           // int + int = 3
    std::cout << add(1.5, 2.5) << "\n";                       // double + double = 4
    std::cout << add(std::string("Hello, "), std::string("World!")) << "\n";
    
    // 泛型 Lambda 用于算法
    auto print = [](const auto& container) {
        for (const auto& item : container) {
            std::cout << item << " ";
        }
        std::cout << "\n";
    };
    
    std::vector<int> v1 = {1, 2, 3};
    std::vector<std::string> v2 = {"a", "b", "c"};
    
    print(v1);  // 1 2 3
    print(v2);  // a b c
    
    // 泛型 Lambda 本质上是模板
    // 编译器生成的类类似于：
    // struct __lambda {
    //     template<typename T1, typename T2>
    //     auto operator()(T1 a, T2 b) const { return a + b; }
    // };
    
    return 0;
}
```

### 7. 模板Lambda（C++20）

```cpp
#include <iostream>
#include <vector>
#include <array>

int main() {
    // C++20: 模板 Lambda
    // 在捕获列表后使用 template<typename...>
    auto templated = []<typename T>(T a, T b) {
        return a + b;
    };
    
    // 约束模板 Lambda
    auto add_numbers = []<std::arithmetic T>(T a, T b) {
        return a + b;
    };
    
    std::cout << add_numbers(1, 2) << "\n";     // 3
    std::cout << add_numbers(1.5, 2.5) << "\n"; // 4
    
    // 完美转发 Lambda
    auto forwarder = []<typename T>(T&& arg) {
        return std::forward<T>(arg);
    };
    
    int x = 10;
    int& ref = forwarder(x);      // 左值引用
    int&& rref = forwarder(20);   // 右值引用
    
    // 数组大小推导
    auto get_size = []<typename T, size_t N>(const T(&arr)[N]) {
        return N;
    };
    
    int arr[] = {1, 2, 3, 4, 5};
    std::cout << "Array size: " << get_size(arr) << "\n";  // 5
    
    return 0;
}
```

### 8. 实际应用示例

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>

int main() {
    std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3, 5};
    
    // 1. 排序：自定义比较器
    std::sort(v.begin(), v.end(), [](int a, int b) {
        return a > b;  // 降序排序
    });
    // v: 9 6 5 5 5 4 3 3 2 1 1
    
    // 2. 查找：条件查找
    auto it = std::find_if(v.begin(), v.end(), [](int n) {
        return n > 5;  // 查找第一个大于5的元素
    });
    if (it != v.end()) {
        std::cout << "Found: " << *it << "\n";  // 9
    }
    
    // 3. 计数：条件计数
    int threshold = 5;
    int count = std::count_if(v.begin(), v.end(), [threshold](int n) {
        return n > threshold;  // 捕获 threshold
    });
    std::cout << "Count > " << threshold << ": " << count << "\n";  // 2
    
    // 4. 变换：元素变换
    std::vector<int> doubled;
    std::transform(v.begin(), v.end(), std::back_inserter(doubled),
                   [](int n) { return n * 2; });
    
    // 5. 累积：带初始值的累积
    int sum = std::accumulate(v.begin(), v.end(), 0, 
                              [](int acc, int n) { return acc + n; });
    std::cout << "Sum: " << sum << "\n";
    
    // 6. 遍历：带副作用
    int total = 0;
    std::for_each(v.begin(), v.end(), [&total](int n) {
        total += n;
    });
    std::cout << "Total: " << total << "\n";
    
    // 7. 删除：条件删除
    v.erase(std::remove_if(v.begin(), v.end(), [](int n) {
        return n < 3;  // 删除小于3的元素
    }), v.end());
    
    // 8. 分区：条件分区
    auto partition_point = std::partition(v.begin(), v.end(), 
                                          [](int n) { return n % 2 == 0; });
    // 偶数在前，奇数在后
    
    // 9. 立即调用的 Lambda（IIFE）
    int result = [](int a, int b) {
        return a + b;
    }(3, 4);
    std::cout << "IIFE result: " << result << "\n";  // 7
    
    // 10. 递归 Lambda（需要 std::function）
    std::function<int(int)> factorial = [&factorial](int n) {
        return (n <= 1) ? 1 : n * factorial(n - 1);
    };
    std::cout << "5! = " << factorial(5) << "\n";  // 120
    
    // C++23: 递归 Lambda（使用 this 捕获）
    // auto factorial_cpp23 = [](this auto&& self, int n) {
    //     return (n <= 1) ? 1 : n * self(n - 1);
    // };
    
    return 0;
}
```

---

## 2. 函数对象与std::function

### 1. 函数对象（Functor）

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

// 函数对象：重载 operator() 的类
// 也称为仿函数（Functor）
class Multiplier {
private:
    int factor_;  // 内部状态
    
public:
    // 构造函数：初始化因子
    explicit Multiplier(int f) : factor_(f) {}
    
    // 函数调用运算符
    int operator()(int x) const {
        return x * factor_;
    }
    
    // 获取因子
    int get_factor() const { return factor_; }
};

// 带状态的函数对象
class Counter {
private:
    int count_ = 0;
    
public:
    int operator()() {
        return ++count_;  // 每次调用计数+1
    }
    
    int get_count() const { return count_; }
};

int main() {
    // 创建函数对象
    Multiplier triple(3);
    Multiplier quadruple(4);
    
    // 像函数一样调用
    std::cout << triple(5) << "\n";      // 15 (5 * 3)
    std::cout << quadruple(5) << "\n";   // 20 (5 * 4)
    
    // 在算法中使用
    std::vector<int> v = {1, 2, 3, 4, 5};
    std::vector<int> result;
    
    std::transform(v.begin(), v.end(), 
                   std::back_inserter(result),
                   Multiplier(2));  // 每个元素乘以2
    
    for (int x : result) {
        std::cout << x << " ";  // 2 4 6 8 10
    }
    std::cout << "\n";
    
    // 带状态的函数对象
    Counter counter;
    std::cout << counter() << "\n";  // 1
    std::cout << counter() << "\n";  // 2
    std::cout << counter() << "\n";  // 3
    
    return 0;
}
```

### 2. std::function

```cpp
#include <iostream>
#include <functional>
#include <vector>
#include <string>

// 普通函数
int add(int a, int b) {
    return a + b;
}

// 函数对象
struct Multiply {
    int operator()(int a, int b) const {
        return a * b;
    }
};

int main() {
    // std::function 可以存储任何可调用对象
    // 模板参数是函数签名：返回类型(参数类型...)
    std::function<int(int, int)> op;
    
    // 1. 存储普通函数
    op = add;
    std::cout << "add: " << op(3, 4) << "\n";  // 7
    
    // 2. 存储函数对象
    op = Multiply();
    std::cout << "multiply: " << op(3, 4) << "\n";  // 12
    
    // 3. 存储 Lambda
    op = [](int a, int b) { return a - b; };
    std::cout << "subtract: " << op(3, 4) << "\n";  // -1
    
    // 4. 存储成员函数指针（需要绑定对象）
    struct Calculator {
        int divide(int a, int b) { return a / b; }
    };
    
    Calculator calc;
    op = [&calc](int a, int b) { return calc.divide(a, b); };
    std::cout << "divide: " << op(10, 2) << "\n";  // 5
    
    // 5. 存储可调用对象的容器
    std::vector<std::function<int(int, int)>> operations = {
        add,
        Multiply(),
        [](int a, int b) { return a + b; },
        [](int a, int b) { return a * a + b * b; }
    };
    
    for (const auto& f : operations) {
        std::cout << f(3, 4) << " ";
    }
    std::cout << "\n";  // 7 12 7 25
    
    return 0;
}
```

### 3. std::bind

```cpp
#include <iostream>
#include <functional>
#include <string>

// 普通函数
int power(int base, int exp) {
    int result = 1;
    for (int i = 0; i < exp; ++i) {
        result *= base;
    }
    return result;
}

// 成员函数
struct Person {
    std::string name;
    int age;
    
    void introduce() const {
        std::cout << "I'm " << name << ", " << age << " years old.\n";
    }
    
    void set_age(int a) { age = a; }
};

int main() {
    using namespace std::placeholders;
    // _1, _2, ... 是占位符，表示调用时传入的参数
    
    // 1. 绑定普通函数：固定部分参数
    auto square = std::bind(power, _1, 2);  // 固定 exp = 2
    auto cube = std::bind(power, _1, 3);    // 固定 exp = 3
    
    std::cout << "5^2 = " << square(5) << "\n";  // 25
    std::cout << "3^3 = " << cube(3) << "\n";    // 27
    
    // 2. 重排参数
    auto subtract = [](int a, int b) { return a - b; };
    auto reversed = std::bind(subtract, _2, _1);  // 参数顺序反转
    
    std::cout << "5-3 = " << subtract(5, 3) << "\n";    // 2
    std::cout << "3-5 = " << reversed(5, 3) << "\n";    // -2（参数反转）
    
    // 3. 绑定成员函数
    Person p{"Alice", 30};
    
    // 绑定成员函数需要传入对象（指针或引用）
    auto introduce = std::bind(&Person::introduce, &p);
    introduce();  // I'm Alice, 30 years old.
    
    auto set_age = std::bind(&Person::set_age, &p, _1);
    set_age(25);
    std::cout << p.age << "\n";  // 25
    
    // 4. 绑定成员变量
    auto get_name = std::bind(&Person::name, &p);
    std::cout << "Name: " << get_name() << "\n";  // Alice
    
    // 注意：现代 C++ 推荐使用 Lambda 替代 std::bind
    // Lambda 更清晰、更高效、更易读
    
    auto square_lambda = [](int base) { return power(base, 2); };
    std::cout << "5^2 = " << square_lambda(5) << "\n";  // 25
    
    return 0;
}
```

### 4. Lambda vs std::bind

```cpp
#include <iostream>
#include <functional>
#include <memory>

void demo(int a, int b, int c) {
    std::cout << "a=" << a << ", b=" << b << ", c=" << c << "\n";
}

int main() {
    // std::bind 的问题
    
    // 1. 可读性差
    using namespace std::placeholders;
    auto bound1 = std::bind(demo, _2, 100, _1);  // 参数顺序反转
    bound1(1, 2);  // a=2, b=100, c=1（难以理解）
    
    // Lambda 更清晰
    auto bound2 = [](int c, int a) { demo(a, 100, c); };
    bound2(1, 2);  // a=2, b=100, c=1
    
    // 2. 性能问题
    // std::bind 可能产生额外的间接调用
    
    // 3. 移动语义支持
    auto ptr = std::make_unique<int>(42);
    
    // Lambda 可以移动捕获
    auto lambda = [p = std::move(ptr)]() { 
        std::cout << *p << "\n"; 
    };
    
    // std::bind 不支持移动语义
    
    // 4. 泛型支持
    auto generic_lambda = [](auto x, auto y) { return x + y; };
    // std::bind 不支持泛型
    
    // std::bind 的适用场景
    // - 需要与 C 接口交互
    // - 需要参数重排
    // - 兼容旧代码
    
    return 0;
}
```

### 5. 标准库函数对象

```cpp
#include <iostream>
#include <functional>
#include <algorithm>
#include <vector>

int main() {
    // 算术运算
    std::plus<int> add;           // x + y
    std::minus<int> sub;          // x - y
    std::multiplies<int> mul;     // x * y
    std::divides<int> div;        // x / y
    std::modulus<int> mod;        // x % y
    std::negate<int> neg;         // -x
    
    std::cout << add(3, 4) << "\n";   // 7
    std::cout << mul(3, 4) << "\n";   // 12
    
    // 比较运算
    std::equal_to<int> eq;        // x == y
    std::not_equal_to<int> ne;    // x != y
    std::greater<int> gt;         // x > y
    std::less<int> lt;            // x < y
    std::greater_equal<int> ge;   // x >= y
    std::less_equal<int> le;      // x <= y
    
    std::cout << gt(5, 3) << "\n";    // 1 (true)
    
    // 逻辑运算
    std::logical_and<bool> and_;  // x && y
    std::logical_or<bool> or_;    // x || y
    std::logical_not<bool> not_;  // !x
    
    // 位运算
    std::bit_and<int> bit_and;    // x & y
    std::bit_or<int> bit_or;      // x | y
    std::bit_xor<int> bit_xor;    // x ^ y
    std::bit_not<int> bit_not;    // ~x
    
    // 在算法中使用
    std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6};
    
    // 降序排序
    std::sort(v.begin(), v.end(), std::greater<int>());
    
    // 查找第一个小于 3 的元素
    auto it = std::find_if(v.begin(), v.end(), 
                           std::bind(std::less<int>(), std::placeholders::_1, 3));
    
    // C++14: 透明运算符（可以省略类型）
    std::plus<> add_generic;      // 自动推导类型
    std::cout << add_generic(3, 4.5) << "\n";  // 7.5
    
    return 0;
}
```

---

## 3. 模板最佳实践

### 1. 编译时优化

```cpp
#include <iostream>
#include <type_traits>

// 1. 使用 constexpr 函数替代模板元编程
// 传统方式（复杂）
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N-1>::value;
};
template<>
struct Factorial<0> { static constexpr int value = 1; };

// 现代方式（简洁）
constexpr int factorial(int n) {
    return (n <= 1) ? 1 : n * factorial(n - 1);
}

// 2. 使用 if constexpr 替代 SFINAE
template<typename T>
auto process(T value) {
    if constexpr (std::is_integral_v<T>) {
        return value * 2;
    } else if constexpr (std::is_floating_point_v<T>) {
        return value * 2.0;
    } else {
        return value;
    }
}

// 3. 使用 Concepts 简化约束
template<std::integral T>
T optimized_process(T value) {
    return value * 2;
}

// 4. 避免不必要的模板实例化
template<typename T>
class Container {
public:
    // 只在需要时实例化成员函数
    void method1() { /* ... */ }
    void method2() { /* ... */ }
    
    // 使用 enable_if 控制实例化
    template<typename U = T>
    std::enable_if_t<std::is_arithmetic_v<U>, U>
    compute() {
        return U{};
    }
};

int main() {
    static_assert(Factorial<5>::value == 120);
    static_assert(factorial(5) == 120);
    
    std::cout << process(42) << "\n";     // 84
    std::cout << process(3.14) << "\n";   // 6.28
    
    return 0;
}
```

### 2. 错误处理与诊断

```cpp
#include <iostream>
#include <concepts>
#include <type_traits>

// 1. 使用 static_assert 提供清晰的错误信息
template<typename T>
void process_array(T* arr, size_t size) {
    static_assert(std::is_arithmetic_v<T>, 
        "process_array requires arithmetic types (int, float, etc.)");
    // ...
}

// 2. 使用 Concepts 提供更好的错误诊断
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<Numeric T>
T add(T a, T b) {
    return a + b;
}

// 3. 自定义诊断信息
template<typename T>
concept Printable = requires(std::ostream& os, T value) {
    { os << value } -> std::same_as<std::ostream&>;
};

template<Printable T>
void print(const T& value) {
    std::cout << value << "\n";
}

// 4. 使用 requires 提供详细约束
template<typename T>
requires requires(T t) {
    { t.begin() } -> std::same_as<typename T::iterator>;
    { t.end() } -> std::same_as<typename T::iterator>;
    { t.size() } -> std::convertible_to<size_t>;
}
void print_container(const T& c) {
    for (const auto& item : c) {
        std::cout << item << " ";
    }
    std::cout << "\n";
}

int main() {
    add(1, 2);  // OK
    // add("a", "b");  // 编译错误：清晰的约束信息
    
    print(42);  // OK
    // print(std::vector<int>{});  // 编译错误：不满足 Printable
    
    return 0;
}
```

### 3. 代码组织

```cpp
// ========== 最佳实践：模板代码组织 ==========

// 1. 头文件结构
// ========== my_container.hpp ==========
#ifndef MY_CONTAINER_HPP
#define MY_CONTAINER_HPP

#include <iostream>
#include <iterator>

// 前向声明
template<typename T> class MyContainer;
template<typename T> class MyContainerIterator;

// 主模板定义
template<typename T>
class MyContainer {
public:
    using value_type = T;
    using iterator = MyContainerIterator<T>;
    
    // 公共接口
    void push_back(const T& value);
    iterator begin();
    iterator end();
    
private:
    // 实现细节
    struct Node {
        T data;
        Node* next;
    };
    Node* head_ = nullptr;
};

// 2. 成员函数定义（同一头文件）
template<typename T>
void MyContainer<T>::push_back(const T& value) {
    // 实现...
}

// 3. 显式实例化声明（可选，减少编译时间）
extern template class MyContainer<int>;
extern template class MyContainer<double>;

#endif // MY_CONTAINER_HPP

// ========== my_container.cpp ==========
#include "my_container.hpp"

// 显式实例化定义
template class MyContainer<int>;
template class MyContainer<double>;

// 4. 使用别名模板简化
template<typename T>
using Vec = std::vector<T, std::allocator<T>>;

// 5. 使用变量模板
template<typename T>
constexpr T pi = T(3.14159265358979323846);

// 6. 使用 using 声明而非 typedef
template<typename T>
using MyPtr = std::unique_ptr<T>;
```

### 4. 性能考虑

```cpp
#include <iostream>
#include <vector>
#include <memory>

// 1. 避免代码膨胀
// 问题：每个模板实例都会生成代码
template<typename T, int Size>
class FixedArray {
    T data[Size];
    // 大量代码...
};

// 解决：提取公共代码到基类
class ArrayBase {
protected:
    void common_operation() { /* 公共实现 */ }
};

template<typename T, int Size>
class FixedArrayOptimized : private ArrayBase {
    T data[Size];
    void operation() {
        common_operation();  // 复用基类代码
    }
};

// 2. 使用引用避免拷贝
template<typename T>
void process(const T& value) {  // 使用 const 引用
    // ...
}

// 3. 完美转发
template<typename T>
void wrapper(T&& arg) {
    process(std::forward<T>(arg));  // 保持值类别
}

// 4. 内联小函数
template<typename T>
inline T min(T a, T b) {  // inline 提示编译器内联
    return (a < b) ? a : b;
}

// 5. 编译期计算
constexpr int square(int n) {
    return n * n;
}

int main() {
    // 编译期计算
    int arr[square(5)];  // 大小为 25
    
    // 运行时使用编译期结果
    std::cout << square(5) << "\n";  // 可能编译期计算
    
    return 0;
}
```

### 5. 常见陷阱与解决方案

```cpp
#include <iostream>
#include <memory>
#include <vector>

// 陷阱1：模板定义分离
// 错误：模板定义放在 .cpp 文件
// 解决：模板定义放在头文件

// 陷阱2：类型推导失败
template<typename T>
void func(T a, T b) {}

void test_deduction() {
    // func(1, 2.0);  // 错误：T 推导为 int 和 double
    func(1, static_cast<int>(2.0));  // OK
    func<double>(1, 2.0);  // OK：显式指定
}

// 陷阱3：依赖类型缺少 typename
template<typename T>
void bad(T container) {
    // T::value_type v;  // 错误：缺少 typename
    typename T::value_type v;  // OK
}

// 陷阱4：成员函数模板隐藏基类函数
class Base {
public:
    void func(int) { std::cout << "Base::func(int)\n"; }
};

template<typename T>
class Derived : public Base {
public:
    template<typename U>
    void func(U) { std::cout << "Derived::func(U)\n"; }
    
    void call_base() {
        // func(42);  // 调用 Derived::func(int)，不是 Base::func(int)
        Base::func(42);  // OK：显式调用基类
    }
};

// 陷阱5：SFINAE 陷阱
template<typename T>
typename std::enable_if<std::is_integral<T>::value>::type
process(T) { std::cout << "Integral\n"; }

// 错误：重载歧义
// template<typename T>
// typename std::enable_if<std::is_arithmetic<T>::value>::type
// process(T) { std::cout << "Arithmetic\n"; }  // 与上面冲突

// 解决：使用更精确的约束
template<typename T>
requires std::integral<T>
void process_v2(T) { std::cout << "Integral\n"; }

template<typename T>
requires std::floating_point<T>
void process_v2(T) { std::cout << "Floating\n"; }

int main() {
    Derived<int> d;
    d.call_base();
    
    process_v2(42);    // Integral
    process_v2(3.14);  // Floating
    
    return 0;
}
```

---

## 4. 本章小结

### 1. 核心概念回顾

| 概念 | 说明 | 示例 |
|-----|------|------|
| **Lambda** | 匿名函数对象 | `[=](int x){ return x*2; }` |
| **函数对象** | 重载operator()的类 | `struct Add { int operator()(int a, int b); };` |
| **std::function** | 通用函数包装器 | `std::function<int(int,int)>` |
| **std::bind** | 函数绑定器 | `std::bind(f, _1, 100)` |

### 2. 关键语法要点

1. **Lambda捕获**：`[]`, `[=]`, `[&]`, `[a]`, `[&a]`, `[a = expr]`
2. **泛型Lambda**：`[](auto x, auto y){ return x + y; }`
3. **函数对象**：重载 `operator()`
4. **std::function**：存储任意可调用对象

### 3. 最佳实践

1. **优先使用 Lambda**：比 std::bind 更易读
2. **模板定义放头文件**：避免链接错误
3. **使用 constexpr**：编译期计算
4. **使用 if constexpr**：编译期分支

### 4. 常见陷阱

```cpp
// 陷阱1：模板定义分离
// 解决：模板定义放在头文件

// 陷阱2：类型推导冲突
func(1, 2.0);  // 错误：T 推导为 int 和 double
func<double>(1, 2.0);  // OK

// 陷阱3：忘记 typename
typename T::value_type v;  // 依赖类型需要 typename

// 陷阱4：Lambda 悬空引用
auto f = [&]() { return x; };  // x 销毁后调用 f 是 UB
```

---

**上一章：** [第12章：类型推导](12-类型推导.md)\
**下一章：** [第14章：STL容器](14-STL容器.md)

***

### 5. 相关章节

- [std::function与函数指针与Lambda的区别](../03-问题解答/06-并发编程/27-std-function与函数指针与Lambda.md) — 性能/灵活性/适用场景对比
