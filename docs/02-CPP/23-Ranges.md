> **前置知识**：STL标准库基础见 [STL容器](./14-STL容器.md)。

# Ranges深入

> 掌握C++20 Ranges库的高级用法

***

> **Ranges make algorithms composable.** — Eric Niebler
> （Ranges让算法可组合。）

***

> **🎯 串珠成链，一气呵成。**
> 
> （Ranges通过管道操作符将多个算法串联起来，形成流畅的数据处理流水线。）

## 前置知识
- [Concepts](./22-Concepts.md)
- [C++17新特性](./21-C++17新特性.md)

## 后续内容
- [C++20与23新特性](./24-C++20与23新特性.md)

***

## 目录

- [1. 视图组合](#1-视图组合)
- [2. 管道操作](#2-管道操作)
- [3. 常用视图详解](#3-常用视图详解)
- [4. C++23新增视图](#4-c23新增视图)
- [5. Ranges底层机制](#5-ranges底层机制)
- [6. ranges算法 vs std算法](#6-ranges算法-vs-std算法)
- [7. 自定义视图](#7-自定义视图)
- [8. 投影与比较](#8-投影与比较)

***

## 1. 视图组合

### 1. 视图（View）的特点

- **惰性求值**：不立即计算，遍历时才计算
- **轻量级**：不拥有数据，不分配内存
- **可组合**：视图可以链式组合

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> nums{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // 组合多个视图
    auto result = nums
        | std::views::filter([](int n) { return n % 2 == 0; })  // 偶数
        | std::views::transform([](int n) { return n * n; });     // 平方

    for (int x : result) {
        std::cout << x << " ";  // 4 16 36 64 100
    }
}
```

***

## 2. 管道操作

### 1. 管道运算符 `|`

```cpp
// 等价写法
auto v1 = std::views::transform(std::views::filter(nums, pred), func);
auto v2 = nums | std::views::filter(pred) | std::views::transform(func);
// v1 和 v2 等价，v2 更直观
```

### 2. 组合顺序

```cpp
// 顺序很重要！
auto v1 = nums | std::views::transform(dbl) | std::views::take(3);  // 先转换再取前3
auto v2 = nums | std::views::take(3) | std::views::transform(dbl);  // 先取前3再转换
// v2 更高效（只转换3个元素）
```

***

## 3. 常用视图详解

### 1. filter - 过滤

```cpp
auto evens = nums | std::views::filter([](int n) { return n % 2 == 0; });
```

### 2. transform - 转换

```cpp
auto squared = nums | std::views::transform([](int n) { return n * n; });
```

### 3. take/drop - 取/跳过

```cpp
auto first3 = nums | std::views::take(3);    // 前3个
auto skip2 = nums | std::views::drop(2);     // 跳过前2个
```

### 4. take_while/drop_while - 条件取/跳过

```cpp
auto less5 = nums | std::views::take_while([](int n) { return n < 5; });
auto skip_less5 = nums | std::views::drop_while([](int n) { return n < 5; });
```

### 5. reverse - 反转

```cpp
auto rev = nums | std::views::reverse;
```

### 6. elements - 取元组元素

```cpp
std::vector<std::pair<std::string, int>> pairs{{"a", 1}, {"b", 2}};
auto names = pairs | std::views::elements<0>;  // "a" "b"
auto values = pairs | std::views::elements<1>; // 1 2
```

### 7. keys/values - map键值

```cpp
std::map<std::string, int> m{{"x", 1}, {"y", 2}};
auto ks = m | std::views::keys;     // "x" "y"
auto vs = m | std::views::values;   // 1 2
```

### 8. join - 展平

```cpp
std::vector<std::vector<int>> nested{{1, 2}, {3, 4}, {5}};
auto flat = nested | std::views::join;  // 1 2 3 4 5
```

### 9. split - 分割

```cpp
std::string s = "hello world foo";
auto words = s | std::views::split(' ');  // "hello" "world" "foo"
```

### 10. enumerate - 带索引遍历（C++23）

```cpp
for (auto [idx, val] : nums | std::views::enumerate) {  // C++23
    std::cout << idx << ": " << val << "\n";
}
```

### 11. zip - 并行遍历（C++23）

```cpp
std::vector<int> a{1, 2, 3};
std::vector<char> b{'a', 'b', 'c'};
for (auto [x, y] : std::views::zip(a, b)) {  // C++23
    std::cout << x << ":" << y << " ";  // 1:a 2:b 3:c
}
```

### 12. iota - 生成整数序列

```cpp
// iota(起始值) 生成无限序列
// iota(起始值, 终止值) 生成 [起始, 终止) 序列
for (int x : std::views::iota(1, 10)) {
    std::cout << x << " ";  // 1 2 3 4 5 6 7 8 9
}

// 配合take生成有限序列
for (int x : std::views::iota(0) | std::views::take(5)) {
    std::cout << x << " ";  // 0 1 2 3 4
}

// 实用场景：索引遍历（C++23之前的方式）
std::vector<std::string> names{"Alice", "Bob", "Charlie"};
for (auto i : std::views::iota(0uz, names.size())) {
    std::cout << i << ": " << names[i] << "\n";
}
```

### 13. counted - 从迭代器+计数创建子范围

```cpp
std::vector<int> v{10, 20, 30, 40, 50, 60, 70};

// 从第2个元素开始，取4个
auto sub = std::views::counted(v.begin() + 1, 4);
for (int x : sub) {
    std::cout << x << " ";  // 20 30 40 50
}

// 适用于任何迭代器，包括原始数组
int arr[] = {1, 2, 3, 4, 5};
auto first3 = std::views::counted(arr, 3);
// 1 2 3
```

### 14. common - 转换为common_range

```cpp
// common_range要求begin()和end()返回相同类型
// 某些视图（如filter视图）的迭代器和哨兵类型不同
// common将其转换为相同类型，以兼容旧API

std::vector<int> nums{1, 2, 3, 4, 5, 6};
auto evens = nums | std::views::filter([](int n) { return n % 2 == 0; });

// evens不是common_range，不能直接传给需要(begin,end)对的旧API
// auto dist = std::distance(evens.begin(), evens.end()); // 可能编译失败

// 使用common转换后即可
auto common_evens = evens | std::views::common;
auto dist = std::distance(common_evens.begin(), common_evens.end());  // 3
```

### 15. repeat - 重复元素（C++23）

```cpp
// repeat(值) — 无限重复
// repeat(值, 次数) — 重复指定次数
for (auto x : std::views::repeat(42, 5)) {
    std::cout << x << " ";  // 42 42 42 42 42
}

// 无限重复需配合take使用
for (auto x : std::views::repeat('A') | std::views::take(3)) {
    std::cout << x << " ";  // A A A
}

// 实用场景：生成测试数据
auto zeros = std::views::repeat(0, 100);  // 100个0
```

***

## 4. C++23新增视图

C++23为Ranges库引入了大量新视图，极大增强了数据处理能力。

### 1. zip - 多范围并行遍历

```cpp
#include <ranges>
#include <vector>
#include <iostream>
#include <string>

int main() {
    std::vector<std::string> names{"Alice", "Bob", "Charlie"};
    std::vector<int> ages{20, 22, 19};
    std::vector<double> scores{95.5, 87.0, 92.3};

    // 同时遍历三个范围
    for (auto [name, age, score] : std::views::zip(names, ages, scores)) {
        std::cout << name << ", " << age << "岁, " << score << "分\n";
    }

    // zip的长度取最短的范围
    std::vector<int> a{1, 2, 3, 4, 5};
    std::vector<char> b{'a', 'b', 'c'};
    for (auto [x, y] : std::views::zip(a, b)) {
        std::cout << x << ":" << y << " ";  // 1:a 2:b 3:c（只遍历3个）
    }
}
```

### 2. enumerate - 带索引遍历

```cpp
std::vector<std::string> fruits{"apple", "banana", "cherry"};

// enumerate返回 (索引, 值) 对
for (auto [idx, fruit] : fruits | std::views::enumerate) {
    std::cout << idx << ": " << fruit << "\n";
}
// 输出：
// 0: apple
// 1: banana
// 2: cherry

// 比传统的iota方式更简洁
// 旧方式：for (auto i : std::views::iota(0uz, fruits.size()))
```

### 3. chunk - 固定大小分组

```cpp
std::vector<int> data{1, 2, 3, 4, 5, 6, 7, 8, 9};

// 每3个元素分为一组
for (auto group : data | std::views::chunk(3)) {
    for (int x : group) {
        std::cout << x << " ";
    }
    std::cout << "| ";
}
// 1 2 3 | 4 5 6 | 7 8 9 |

// 最后一组不足3个
for (auto group : data | std::views::chunk(4)) {
    for (int x : group) {
        std::cout << x << " ";
    }
    std::cout << "| ";
}
// 1 2 3 4 | 5 6 7 8 | 9 |

// 实用场景：批量处理数据
std::vector<int> ids{1,2,3,4,5,6,7,8,9,10};
for (auto batch : ids | std::views::chunk(3)) {
    // 每批最多3个，适合分页或批量请求
    process_batch(batch);
}
```

### 4. slide - 滑动窗口

```cpp
std::vector<int> data{1, 2, 3, 4, 5};

// 大小为3的滑动窗口
for (auto window : data | std::views::slide(3)) {
    for (int x : window) {
        std::cout << x << " ";
    }
    std::cout << "| ";
}
// 1 2 3 | 2 3 4 | 3 4 5 |

// 实用场景：计算移动平均
std::vector<double> prices{100.0, 102.5, 98.3, 105.0, 103.2, 107.8};
for (auto window : prices | std::views::slide(3)) {
    double sum = 0;
    for (double p : window) sum += p;
    std::cout << "移动平均: " << sum / 3 << "\n";
}
```

### 5. adjacent / adjacent_transform - 相邻元素

```cpp
std::vector<int> data{1, 3, 2, 5, 4};

// adjacent<2>：每次取2个相邻元素（返回引用的tuple）
for (auto [a, b] : data | std::views::adjacent<2>) {
    std::cout << "(" << a << "," << b << ") ";
}
// (1,3) (3,2) (2,5) (5,4)

// adjacent<3>：每次取3个相邻元素
for (auto [a, b, c] : data | std::views::adjacent<3>) {
    std::cout << "(" << a << "," << b << "," << c << ") ";
}
// (1,3,2) (3,2,5) (2,5,4)

// adjacent_transform：对相邻元素进行变换
// 计算相邻元素的差值
for (auto diff : data | std::views::adjacent_transform<2>(std::minus{})) {
    std::cout << diff << " ";  // -2 1 -3 1
}
```

### 6. cartesian_product - 笛卡尔积

```cpp
std::vector<char> suits{'S', 'H', 'D', 'C'};  // 花色
std::vector<int> ranks{1, 2, 3, 4, 5};         // 点数

// 生成所有组合
for (auto [suit, rank] : std::views::cartesian_product(suits, ranks)) {
    std::cout << suit << rank << " ";
}
// S1 S2 S3 S4 S5 H1 H2 H3 H4 H5 D1 D2 D3 D4 D5 C1 C2 C3 C4 C5

// 三个范围的笛卡尔积
std::vector<int> a{0, 1};
std::vector<int> b{0, 1};
std::vector<int> c{0, 1};
for (auto [x, y, z] : std::views::cartesian_product(a, b, c)) {
    std::cout << "(" << x << y << z << ") ";
}
// (000) (001) (010) (011) (100) (101) (110) (111)
```

***

## 5. Ranges底层机制

### 1. range概念

一个类型要成为`range`，只需满足：拥有`begin()`和`end()`。

```cpp
#include <ranges>
#include <vector>
#include <list>
#include <iostream>

// range的核心要求
static_assert(std::ranges::range<std::vector<int>>);       // true
static_assert(std::ranges::range<std::list<int>>);         // true
static_assert(std::ranges::range<int[5]>);                 // true
static_assert(!std::ranges::range<int>);                   // false

// 更细分的range概念
// sized_range：有O(1)的size()
static_assert(std::ranges::sized_range<std::vector<int>>); // true

// common_range：begin()和end()返回相同类型
static_assert(std::ranges::common_range<std::vector<int>>);// true

// viewable_range：可以安全地转换为view
static_assert(std::ranges::viewable_range<std::vector<int>&>);  // true（左值引用）
static_assert(!std::ranges::viewable_range<std::vector<int>>);  // false（纯右值不能直接转view）
```

### 2. borrowed_range与dangling防护

```cpp
#include <ranges>
#include <vector>
#include <iostream>

// borrowed_range：即使持有迭代器，也不会因范围销毁而悬垂
// 标准库中，左值引用的容器是borrowed_range
static_assert(std::ranges::borrowed_range<std::vector<int>&>);   // true
static_assert(!std::ranges::borrowed_range<std::vector<int>>);   // false

// dangling防护：ranges算法对非borrowed_range的右值返回dangling
auto make_vec() {
    return std::vector<int>{1, 2, 3};
}

// 危险！返回局部容器的迭代器
auto it = std::ranges::find(make_vec(), 2);
// it的类型是std::ranges::dangling，不是迭代器！
// 编译期就能发现这个错误

// 正确做法：先存储容器
auto v = make_vec();
auto it2 = std::ranges::find(v, 2);  // OK，it2是有效迭代器

// dangling是一个空类型，访问它会在编译期报错
// static_assert(!std::is_same_v<decltype(it), std::vector<int>::iterator>);  // true
// static_assert(std::is_same_v<decltype(it), std::ranges::dangling>);        // true
```

### 3. owning_view vs borrowed_view

```cpp
#include <ranges>
#include <vector>
#include <iostream>

// owning_view：拥有数据的视图
// 当右值容器通过管道传给视图时，会生成owning_view
auto ov = std::vector<int>{1, 2, 3, 4, 5}
    | std::views::filter([](int n) { return n > 2; });
// ov的类型包含owning_view<std::vector<int>>
// owning_view拥有vector的所有权，ov的生命周期内数据有效

// borrowed_view：借用数据的视图
// 当左值容器通过管道传给视图时，只是借用引用
std::vector<int> v{1, 2, 3, 4, 5};
auto bv = v | std::views::filter([](int n) { return n > 2; });
// bv只是持有v的引用，v销毁后bv悬垂

// owning_view的实用场景：函数返回组合视图
auto get_evens() {
    return std::vector<int>{1, 2, 3, 4, 5, 6}
        | std::views::filter([](int n) { return n % 2 == 0; });
    // 安全！vector被owning_view持有，生命周期延续
}
```

### 4. view概念的要求

```cpp
// view概念要求：O(1)时间可拷贝、可移动、可析构
// 这确保视图是轻量级的

#include <ranges>
static_assert(std::ranges::view<std::ranges::iota_view<int>>);          // true
static_assert(std::ranges::view<std::ranges::filter_view<...>>);        // true
static_assert(!std::ranges::view<std::vector<int>>);                    // false！vector不是view

// vector不满足view要求的原因：
// 1. 拷贝是O(n)而非O(1)
// 2. 析构可能释放内存
// 3. 拥有数据，不是轻量级引用

// view的O(1)要求意味着：
// - 视图只存储指向底层范围的指针/引用+少量状态
// - 拷贝视图只是拷贝这些指针/引用
// - 析构视图不需要释放任何资源
```

### 5. 常见陷阱：返回局部容器的视图

```cpp
#include <ranges>
#include <vector>
#include <iostream>

// ❌ 陷阱1：返回局部容器的filter视图（左值引用）
auto bad_filter() {
    std::vector<int> v{1, 2, 3, 4, 5};
    return v | std::views::filter([](int n) { return n > 2; });
    // v在函数返回后销毁，返回的视图持有悬垂引用！
}

// ✅ 正确做法1：使用owning_view（右值容器）
auto good_filter() {
    return std::vector<int>{1, 2, 3, 4, 5}
        | std::views::filter([](int n) { return n > 2; });
    // 右值vector被owning_view持有，生命周期安全
}

// ❌ 陷阱2：视图存储了临时变量的引用
auto bad_transform() {
    auto temp = std::vector<int>{1, 2, 3};
    auto result = temp | std::views::transform([](int n) { return n * 2; });
    return result;  // temp销毁，result悬垂
}

// ✅ 正确做法2：直接返回owning_view
auto good_transform() {
    return std::vector<int>{1, 2, 3}
        | std::views::transform([](int n) { return n * 2; });
}

// ❌ 陷阱3：在表达式中混用临时容器
auto bad_mixed() {
    std::vector<int> v1{1, 2, 3};
    return v1 | std::views::transform([](int n) { return n * 2; });
    // v1是局部变量，返回后悬垂
}
```

***

## 6. ranges算法 vs std算法

### 1. ranges算法的优势

```cpp
#include <algorithm>
#include <ranges>
#include <vector>
#include <iostream>

// 优势1：投影参数（projection）
// ranges算法内置投影支持，无需额外编写lambda
struct Person {
    std::string name;
    int age;
    double score;
};

std::vector<Person> people{
    {"Alice", 20, 95.5},
    {"Bob", 22, 87.0},
    {"Charlie", 19, 92.3}
};

// std算法需要手写比较lambda
std::sort(people.begin(), people.end(),
    [](const Person& a, const Person& b) { return a.age < b.age; });

// ranges算法直接使用投影参数
std::ranges::sort(people, {}, &Person::age);  // 更简洁！

// 优势2：约束检查
// ranges算法通过concept约束参数，错误信息更友好
// std::sort(people.begin(), people.end());  // 编译错误：Person没有operator<
// 错误信息冗长难以理解

// std::ranges::sort(people);  // 编译错误：concept约束不满足
// 错误信息直接指出Person不满足sortable概念

// 优势3：可以直接传range，不需要begin/end
std::vector<int> v{5, 3, 1, 4, 2};
std::ranges::sort(v);  // 直接传range
// 等价于 std::sort(v.begin(), v.end());
```

### 2. ranges::sort vs std::sort

```cpp
#include <algorithm>
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v1{5, 3, 1, 4, 2};
    std::vector<int> v2{5, 3, 1, 4, 2};

    // 基本用法等价
    std::sort(v1.begin(), v1.end());
    std::ranges::sort(v2);  // 更简洁

    // 自定义比较
    std::sort(v1.begin(), v1.end(), std::greater{});
    std::ranges::sort(v2, std::greater{});  // 直接传比较器

    // ranges独有：投影参数
    std::vector<Person> people{
        {"Alice", 20, 95.5},
        {"Bob", 22, 87.0},
        {"Charlie", 19, 92.3}
    };

    // 按age排序，{}表示使用默认比较（less）
    std::ranges::sort(people, {}, &Person::age);

    // 按name长度排序
    std::ranges::sort(people, {}, [](const auto& p) { return p.name.size(); });

    // 降序 + 投影
    std::ranges::sort(people, std::greater{}, &Person::score);
}
```

### 3. 投影参数的用法

```cpp
#include <algorithm>
#include <ranges>
#include <vector>
#include <string>
#include <iostream>

struct Product {
    std::string name;
    double price;
    int stock;
};

int main() {
    std::vector<Product> products{
        {"Widget", 9.99, 100},
        {"Gadget", 24.99, 50},
        {"Doohickey", 4.99, 200},
        {"Thingamajig", 14.99, 75}
    };

    // 投影参数：第三个参数指定"提取什么来比较"
    // 语法：ranges::algorithm(range, comparator, projection)

    // 按价格排序（默认升序）
    std::ranges::sort(products, {}, &Product::price);

    // 按库存降序排序
    std::ranges::sort(products, std::greater{}, &Product::stock);

    // 按名称长度排序（投影可以是任意可调用对象）
    std::ranges::sort(products, {}, [](const Product& p) { return p.name.size(); });

    // find也支持投影
    auto it = std::ranges::find(products, "Gadget", &Product::name);
    if (it != products.end()) {
        std::cout << "找到: " << it->name << ", 价格: " << it->price << "\n";
    }

    // count_if也支持投影
    auto expensive = std::ranges::count_if(products,
        [](double p) { return p > 10.0; },  // lambda接收投影后的值
        &Product::price);                     // 投影：提取price
    std::cout << "价格超过10的商品数: " << expensive << "\n";

    // 投影 + 视图组合
    auto affordable = products
        | std::views::filter([](const Product& p) { return p.price < 15.0; })
        | std::views::transform(&Product::name);
    for (const auto& name : affordable) {
        std::cout << name << " ";
    }
    // Widget Doohickey Thingamajig
}
```

***

## 7. 自定义视图

```cpp
#include <ranges>

template<std::ranges::range R>
auto square_view(R&& r) {
    return std::forward<R>(r)
        | std::views::transform([](auto x) { return x * x; });
}

template<std::ranges::range R>
auto even_view(R&& r) {
    return std::forward<R>(r)
        | std::views::filter([](auto x) { return x % 2 == 0; });
}

// 使用
std::vector<int> v{1, 2, 3, 4, 5, 6};
for (int x : square_view(even_view(v))) {
    std::cout << x << " ";  // 4 16 36
}
```

***

## 8. 投影与比较

```cpp
#include <algorithm>
#include <ranges>
#include <vector>
#include <string>

struct Student {
    std::string name;
    int age;
    double score;
};

int main() {
    std::vector<Student> students{
        {"Alice", 20, 95.5},
        {"Bob", 22, 87.0},
        {"Charlie", 19, 92.3}
    };

    // 按分数排序（使用投影）
    std::ranges::sort(students, {}, &Student::score);

    // 按名字长度排序
    std::ranges::sort(students, {}, [](const auto& s) { return s.name.size(); });

    // 查找年龄大于20的学生
    auto older = students | std::views::filter([](const auto& s) { return s.age > 20; });

    // 取名字
    auto names = students | std::views::transform(&Student::name);
}
```

***

## 9. 本章小结

| 主题 | 核心内容 |
|------|---------|
| **视图** | 惰性求值、轻量级、可组合 |
| **管道** | \| 运算符链式调用 |
| **常用视图** | filter/transform/take/drop/reverse/join/split/iota/counted/common/repeat |
| **C++23视图** | zip/enumerate/chunk/slide/adjacent/cartesian_product |
| **底层机制** | range概念/borrowed_range/dangling防护/owning_view/view的O(1)要求 |
| **ranges算法** | 投影参数、约束检查、直接传range |
| **自定义** | 组合现有视图 |
| **投影** | ranges算法支持投影参数 |

***

**上一章：** [第22章：Concepts](./22-Concepts.md)\
**下一章：** [第24章：C++20与23新特性](./24-C++20与23新特性.md)
