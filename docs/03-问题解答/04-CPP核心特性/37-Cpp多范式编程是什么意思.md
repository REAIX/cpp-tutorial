# C++ 多范式编程是什么意思
> 📖 相关章节：[C++概述](../../02-CPP/00-C++概述.md)

### 1. 精髓速览

**多范式** = C++ 支持多种编程风格：**过程式**、**面向对象**、**泛型**、**函数式**。你可以在同一个项目中混用这些范式，根据问题特征选择最合适的风格。

***

### 2. 四大范式概览

| 范式 | 核心思想 | 关键机制 | 代表语言 |
|------|----------|----------|----------|
| 过程式 | 按步骤执行 | 函数、控制流 | C、Pascal |
| 面向对象 | 对象与交互 | 类、继承、多态 | Java、C# |
| 泛型 | 类型无关编程 | 模板、概念 | C++、Haskell |
| 函数式 | 数学函数组合 | Lambda、不可变 | Haskell、Lisp |

C++ 不是单一范式语言，而是**多范式语言**——它同时支持以上四种范式，并且鼓励根据场景灵活组合。

### 3. 过程式编程（C 风格）

过程式编程是最基础的范式：程序由一系列过程（函数）组成，数据和处理函数分离。

```cpp
#include <cstdio>

// 数据结构
struct Point {
    double x, y;
};

// 过程：操作数据
double distance(const Point& a, const Point& b) {
    double dx = a.x - b.x;
    double dy = a.y - b.y;
    return sqrt(dx * dx + dy * dy);
}

Point midpoint(const Point& a, const Point& b) {
    return {(a.x + b.x) / 2, (a.y + b.y) / 2};
}

void print_point(const Point& p) {
    printf("(%.2f, %.2f)", p.x, p.y);
}

int main() {
    Point p1 = {0, 0};
    Point p2 = {3, 4};

    printf("distance: %.2f\n", distance(p1, p2));  // 5.00
    Point mid = midpoint(p1, p2);
    print_point(mid);  // (1.50, 2.00)
}
```

**特点**：
- 数据和函数分离
- 自顶向下的流程控制
- 简单直观，适合小型程序
- C 语言的核心范式

**局限**：
- 数据没有封装，容易被意外修改
- 大型项目难以组织
- 没有继承和多态机制

### 4. 面向对象编程

面向对象编程将数据和操作封装为对象，通过继承和多态实现代码复用和扩展。

```cpp
#include <iostream>
#include <string>
#include <vector>
#include <memory>

// 抽象基类
class Shape {
public:
    virtual double area() const = 0;
    virtual double perimeter() const = 0;
    virtual std::string name() const = 0;
    virtual ~Shape() = default;

    // 通用方法
    void print() const {
        std::cout << name() << ": area=" << area()
                  << ", perimeter=" << perimeter() << std::endl;
    }
};

// 具体类
class Circle : public Shape {
    double radius_;
public:
    explicit Circle(double r) : radius_(r) {}
    double area() const override { return 3.14159265 * radius_ * radius_; }
    double perimeter() const override { return 2 * 3.14159265 * radius_; }
    std::string name() const override { return "Circle"; }
};

class Rectangle : public Shape {
    double width_, height_;
public:
    Rectangle(double w, double h) : width_(w), height_(h) {}
    double area() const override { return width_ * height_; }
    double perimeter() const override { return 2 * (width_ + height_); }
    std::string name() const override { return "Rectangle"; }
};

// 多态使用
void print_all(const std::vector<std::unique_ptr<Shape>>& shapes) {
    for (const auto& s : shapes) {
        s->print();
    }
}

int main() {
    std::vector<std::unique_ptr<Shape>> shapes;
    shapes.push_back(std::make_unique<Circle>(5.0));
    shapes.push_back(std::make_unique<Rectangle>(3.0, 4.0));
    print_all(shapes);
}
```

**特点**：
- 封装：数据和方法绑定在一起
- 继承：代码复用和层次结构
- 多态：同一接口不同实现

**局限**：
- 继承层次过深时难以维护
- 所有操作必须属于某个类
- 性能开销（虚函数表）

### 5. 泛型编程

泛型编程编写与类型无关的代码，STL 是其最成功的实践。

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

// 泛型函数：适用于任何可比较的类型
template <typename T>
const T& my_max(const T& a, const T& b) {
    return (a > b) ? a : b;
}

// 泛型算法：适用于任何容器
template <typename Iter, typename Func>
void my_for_each(Iter begin, Iter end, Func func) {
    for (auto it = begin; it != end; ++it) {
        func(*it);
    }
}

// 泛型类：适用于任何元素类型
template <typename T, int N>
class Stack {
    T data_[N];
    int top_ = 0;
public:
    void push(const T& val) {
        if (top_ < N) data_[top_++] = val;
    }
    T pop() {
        return data_[--top_];
    }
    bool empty() const { return top_ == 0; }
};

int main() {
    // 泛型函数
    std::cout << my_max(3, 7) << std::endl;          // 7
    std::cout << my_max(3.14, 2.72) << std::endl;    // 3.14
    std::cout << my_max(std::string("abc"), std::string("xyz")) << std::endl; // xyz

    // STL 泛型算法
    std::vector<int> v = {5, 2, 8, 1, 9};
    std::sort(v.begin(), v.end());  // 泛型排序

    my_for_each(v.begin(), v.end(), [](int x) {
        std::cout << x << " ";  // 1 2 5 8 9
    });
    std::cout << std::endl;

    // 泛型类
    Stack<int, 10> int_stack;
    int_stack.push(1);
    int_stack.push(2);

    Stack<std::string, 5> str_stack;
    str_stack.push("hello");
}
```

**特点**：
- 类型无关：一套代码适用于多种类型
- 编译期检查：错误在编译期发现
- 零开销抽象：编译器优化后无额外成本
- STL 的核心范式

**局限**：
- 编译错误信息复杂
- 代码膨胀（每个类型生成一份）
- 编译速度慢

### 6. 函数式编程

C++ 从 C++11 开始逐步引入函数式特性，虽然不如 Haskell 纯粹，但足以实现核心思想。

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>
#include <functional>

// Lambda 表达式
void lambda_demo() {
    auto add = [](int a, int b) { return a + b; };
    auto mul = [](int a, int b) { return a * b; };

    std::cout << add(3, 4) << std::endl;  // 7
    std::cout << mul(3, 4) << std::endl;  // 12

    // 捕获变量
    int factor = 10;
    auto scale = [factor](int x) { return x * factor; };
    std::cout << scale(5) << std::endl;  // 50
}

// 高阶函数：函数作为参数
template <typename T, typename Func>
std::vector<T> my_transform(const std::vector<T>& input, Func func) {
    std::vector<T> result;
    result.reserve(input.size());
    for (const auto& x : input) {
        result.push_back(func(x));
    }
    return result;
}

// 函数组合
void composition_demo() {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // 链式操作：过滤 → 变换 → 累加
    // 1. 过滤偶数
    std::vector<int> evens;
    std::copy_if(data.begin(), data.end(), std::back_inserter(evens),
                 [](int x) { return x % 2 == 0; });

    // 2. 平方
    std::vector<int> squares = my_transform(evens, [](int x) { return x * x; });

    // 3. 累加
    int sum = std::accumulate(squares.begin(), squares.end(), 0);
    std::cout << "sum of squares of evens: " << sum << std::endl;  // 120
}

// 不可变数据
void immutable_demo() {
    const std::vector<int> original = {1, 2, 3};

    // 不修改 original，创建新数据
    const auto doubled = my_transform(original, [](int x) { return x * 2; });

    // original 仍然是 {1, 2, 3}
    // doubled 是 {2, 4, 6}
}

// std::function 和高阶函数
using Transform = std::function<int(int)>;

Transform compose(Transform f, Transform g) {
    return [f, g](int x) { return f(g(x)); };
}

void compose_demo() {
    auto double_it = [](int x) { return x * 2; };
    auto add_one = [](int x) { return x + 1; };

    auto double_then_add = compose(add_one, double_it);
    auto add_then_double = compose(double_it, add_one);

    std::cout << double_then_add(5) << std::endl;  // 11 (5*2+1)
    std::cout << add_then_double(5) << std::endl;  // 12 ((5+1)*2)
}

int main() {
    lambda_demo();
    composition_demo();
    compose_demo();
}
```

**特点**：
- 函数是一等公民
- 不可变数据，避免副作用
- 高阶函数和函数组合
- 声明式风格（描述"做什么"而非"怎么做"）

**局限**：
- C++ 不是纯函数式语言，不可变性靠约定
- 性能可能受影响（频繁拷贝）
- 递归深度受限（C++ 没有尾递归优化保证）

### 7. C 语言支持哪些

| 范式 | C | C++ | 说明 |
|------|:---:|:---:|------|
| 过程式 | 完全支持 | 完全支持 | C 的核心范式 |
| 面向对象 | 可模拟 | 完全支持 | C 可用函数指针模拟多态 |
| 泛型 | 宏模拟 | 完全支持 | C 用 `void*` 或宏实现泛型 |
| 函数式 | 极有限 | 部分支持 | C 有函数指针，无 Lambda |

#### 1. C 语言模拟面向对象

```c
// C 模拟面向对象：用函数指针模拟虚函数
struct ShapeVTable {
    double (*area)(const void* self);
    void (*print)(const void* self);
};

typedef struct {
    const ShapeVTable* vtable;
    double radius;
} Circle;

double circle_area(const void* self) {
    const Circle* c = (const Circle*)self;
    return 3.14159 * c->radius * c->radius;
}

void circle_print(const void* self) {
    const Circle* c = (const Circle*)self;
    printf("Circle(r=%.2f), area=%.2f\n", c->radius, circle_area(self));
}

static const ShapeVTable circle_vtable = {
    .area = circle_area,
    .print = circle_print,
};

void circle_init(Circle* c, double r) {
    c->vtable = &circle_vtable;
    c->radius = r;
}

// 多态调用
void shape_print(const void* self, const ShapeVTable* vtable) {
    vtable->print(self);
}
```

#### 2. C 语言模拟泛型

```c
// C 用 void* 实现泛型
void swap(void* a, void* b, size_t size) {
    char temp[size];
    memcpy(temp, a, size);
    memcpy(a, b, size);
    memcpy(b, temp, size);
}

// C 用宏实现泛型
#define MAX(a, b) ((a) > (b) ? (a) : (b))

// C11 泛型选择
#define print_type(x) _Generic((x), \
    int: printf("int: %d\n", x),     \
    double: printf("double: %f\n", x), \
    default: printf("unknown\n")      \
)
```

### 8. 什么时候用哪个范式

| 场景 | 推荐范式 | 原因 |
|------|----------|------|
| 简单算法/工具函数 | 过程式 | 直观、无封装开销 |
| 有明确对象层次（图形/游戏） | 面向对象 | 封装+多态自然表达 |
| 类型无关算法（排序/查找） | 泛型 | 一套代码适用所有类型 |
| 数据变换管道（ETL/处理链） | 函数式 | 组合性好、声明式 |
| 嵌入式/底层驱动 | 过程式 | 可控、无隐式开销 |
| 大型框架/库 | 混合 | 各取所长 |
| 模板库（STL 级别） | 泛型 | 类型安全+零开销 |

### 9. 混合使用范式

C++ 的强大之处在于可以在同一个项目中混合使用多种范式：

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <memory>
#include <numeric>

// 面向对象：定义类层次
class Animal {
public:
    virtual std::string sound() const = 0;
    virtual ~Animal() = default;
};

class Dog : public Animal {
public:
    std::string sound() const override { return "Woof"; }
};

class Cat : public Animal {
public:
    std::string sound() const override { return "Meow"; }
};

// 泛型：类型无关的容器操作
template <typename Container>
size_t count_items(const Container& c) {
    return c.size();
}

// 函数式：数据变换管道
std::vector<std::string> get_sounds(const std::vector<std::unique_ptr<Animal>>& animals) {
    std::vector<std::string> sounds;
    // transform + Lambda（函数式）
    std::transform(animals.begin(), animals.end(), std::back_inserter(sounds),
        [](const std::unique_ptr<Animal>& a) { return a->sound(); });
    return sounds;
}

// 过程式：简单的工具函数
void print_line(const std::string& text) {
    std::cout << text << std::endl;
}

int main() {
    // 面向对象：创建对象
    std::vector<std::unique_ptr<Animal>> zoo;
    zoo.push_back(std::make_unique<Dog>());
    zoo.push_back(std::make_unique<Cat>());
    zoo.push_back(std::make_unique<Dog>());

    // 泛型：通用操作
    print_line("Count: " + std::to_string(count_items(zoo)));

    // 函数式：变换
    auto sounds = get_sounds(zoo);

    // 过程式：输出
    for (const auto& s : sounds) {
        print_line(s);
    }
}
```

### 10. 范式选择指南

```
你的问题是什么？
│
├─ 需要封装数据和操作？ → 面向对象
│   └─ 有继承层次？ → 虚函数多态
│   └─ 无继承层次？ → 简单类即可
│
├─ 需要类型无关？ → 泛型
│   └─ 编译期类型确定？ → 模板
│   └─ 运行时类型变化？ → 虚函数 + 面向对象
│
├─ 需要数据变换管道？ → 函数式
│   └─ 简单变换？ → Lambda + algorithm
│   └─ 复杂管道？ → ranges（C++20）
│
└─ 简单逻辑/底层代码？ → 过程式
    └─ 驱动/嵌入式？ → C 风格过程式
    └─ 工具函数？ → 命名空间 + 自由函数
```

### 11. 常见陷阱

#### 1. 陷阱1：过度使用面向对象

```cpp
// 不推荐：所有东西都是类
class Adder {
public:
    int add(int a, int b) { return a + b; }
};
Adder a;
a.add(1, 2);

// 推荐：简单函数即可
int add(int a, int b) { return a + b; }
```

#### 2. 陷阱2：过度使用模板

```cpp
// 不推荐：简单逻辑不需要模板
template <typename T>
T add(T a, T b) { return a + b; }

// 推荐：只在真正需要类型无关时用模板
int add(int a, int b) { return a + b; }
```

#### 3. 陷阱3：过度函数式

```cpp
// 不推荐：C++ 不是 Haskell，过度函数式反而难读
auto result = ranges::views::filter(data, pred)
            | ranges::views::transform(func)
            | ranges::views::take(10);

// 推荐：适度使用函数式，保持可读性
std::vector<int> filtered;
for (int x : data) {
    if (pred(x)) {
        filtered.push_back(func(x));
        if (filtered.size() >= 10) break;
    }
}
```

### 12. 完整示例：多范式混合项目

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>
#include <memory>

// ===== 过程式：工具函数 =====
namespace util {
    void log(const std::string& msg) {
        std::cout << "[LOG] " << msg << std::endl;
    }

    int clamp(int val, int lo, int hi) {
        return std::max(lo, std::min(val, hi));
    }
}

// ===== 面向对象：实体类 =====
class Item {
    std::string name_;
    int value_;
public:
    Item(std::string name, int value)
        : name_(std::move(name)), value_(util::clamp(value, 0, 100)) {}

    const std::string& name() const { return name_; }
    int value() const { return value_; }

    void print() const {
        std::cout << name_ << ": " << value_ << std::endl;
    }
};

// ===== 泛型：容器操作 =====
template <typename Container, typename Predicate>
auto filter(const Container& c, Predicate pred) {
    using T = typename Container::value_type;
    std::vector<T> result;
    std::copy_if(c.begin(), c.end(), std::back_inserter(result), pred);
    return result;
}

template <typename Container, typename Func>
auto map(const Container& c, Func func) {
    using T = decltype(func(*c.begin()));
    std::vector<T> result;
    std::transform(c.begin(), c.end(), std::back_inserter(result), func);
    return result;
}

// ===== 函数式：数据管道 =====
int main() {
    std::vector<Item> items = {
        Item("Sword", 50),
        Item("Shield", 30),
        Item("Potion", 10),
        Item("Armor", 80),
        Item("Ring", 5)
    };

    util::log("All items:");
    for (const auto& item : items) item.print();

    // 函数式管道：过滤 + 变换
    auto valuable = filter(items, [](const Item& i) { return i.value() > 20; });
    auto names = map(valuable, [](const Item& i) { return i.name(); });

    util::log("Valuable items:");
    for (const auto& name : names) {
        std::cout << "  " << name << std::endl;
    }

    return 0;
}
```

### 13. 极简总结

**C++ 多范式 = 过程式 + 面向对象 + 泛型 + 函数式 → 根据场景选择 → 可混用**

| 范式 | 一句话 | 适用场景 |
|------|--------|----------|
| 过程式 | 按步骤执行 | 简单逻辑、底层代码 |
| 面向对象 | 封装+继承+多态 | 对象层次、框架设计 |
| 泛型 | 类型无关编程 | 算法库、容器 |
| 函数式 | 函数组合+不可变 | 数据变换管道 |

***

### 相关阅读

- [CRTP模式与静态多态](36-CRTP模式与静态多态.md)
- [什么是反射](38-什么是反射.md)
- [什么是类型擦除](21-什么是类型擦除.md)