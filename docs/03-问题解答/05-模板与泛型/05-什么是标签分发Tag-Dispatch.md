# 什么是标签分发 Tag Dispatch
> 📖 相关章节：[模板基础](../../02-CPP/10-模板基础.md)、[模板进阶](../../02-CPP/11-模板进阶.md)、[Concepts](../../02-CPP/22-Concepts.md)

> "用类型来选择函数，让编译器在编译期做出决策。"

***

### 1. 本质洞察

标签分发是利用空结构体类型作为函数参数来引导重载决议，在编译期根据类型特征选择不同实现的一种泛型编程技术。

***

### 2. 标签分发的核心原理

标签分发的核心思想：定义一组空标签类型（tag），利用它们在函数重载决议中的优先级差异，让编译器在编译期选择正确的重载版本。

```cpp
#include <iostream>

struct InputIteratorTag {};
struct ForwardIteratorTag : InputIteratorTag {};
struct RandomAccessIteratorTag : ForwardIteratorTag {};

void advanceImpl(int& iter, int n, InputIteratorTag) {
    std::cout << "单步前进 " << n << " 次" << std::endl;
    for (int i = 0; i < n; ++i) ++iter;
}

void advanceImpl(int& iter, int n, RandomAccessIteratorTag) {
    std::cout << "直接跳跃 " << n << " 步" << std::endl;
    iter += n;
}

template <typename IteratorCategory>
void advance(int& iter, int n, IteratorCategory tag) {
    advanceImpl(iter, n, tag);
}

int main() {
    int iter1 = 0;
    advance(iter1, 5, InputIteratorTag{});

    int iter2 = 0;
    advance(iter2, 5, RandomAccessIteratorTag{});
}
```

输出：

```
单步前进 5 次
直接跳跃 5 步
```

关键机制：

| 要素 | 说明 |
|-----|------|
| 标签类型 | 空结构体，无运行时开销 |
| 继承关系 | 子标签可匹配父标签的重载 |
| 重载决议 | 编译器根据参数类型选择最佳匹配 |
| 零开销 | 标签是空类型，编译器可完全优化掉 |

### 3. 标准库中的标签体系

C++ 标准库在 `<iterator>` 中定义了完整的迭代器标签层次结构：

```cpp
#include <iostream>
#include <iterator>
#include <vector>
#include <list>

namespace std {
    struct input_iterator_tag {};
    struct output_iterator_tag {};
    struct forward_iterator_tag : input_iterator_tag {};
    struct bidirectional_iterator_tag : forward_iterator_tag {};
    struct random_access_iterator_tag : bidirectional_iterator_tag {};
}
```

使用 `iterator_traits` 提取迭代器的标签：

```cpp
#include <iostream>
#include <iterator>
#include <vector>
#include <list>
#include <typeinfo>

template <typename Iter>
void printIteratorCategory() {
    using Category = typename std::iterator_traits<Iter>::iterator_category;
    std::cout << "迭代器类别: " << typeid(Category).name() << std::endl;
}

int main() {
    printIteratorCategory<std::vector<int>::iterator>();
    printIteratorCategory<std::list<int>::iterator>();
    printIteratorCategory<int*>();
}
```

标准标签继承关系：

```
input_iterator_tag
    └── forward_iterator_tag
            └── bidirectional_iterator_tag
                    └── random_access_iterator_tag
output_iterator_tag  (独立)
```

| 标签 | 支持操作 | 典型迭代器 |
|-----|---------|-----------|
| `input_iterator_tag` | `++`, `*`, `==` | `istream_iterator` |
| `output_iterator_tag` | `++`, `*`（写入） | `ostream_iterator` |
| `forward_iterator_tag` | `++`, `*`, `==`, 多遍 | `forward_list::iterator` |
| `bidirectional_iterator_tag` | `++`, `--`, `*`, `==` | `list::iterator`, `set::iterator` |
| `random_access_iterator_tag` | `++`, `--`, `+`, `-`, `[]`, `<` | `vector::iterator`, 原生指针 |

### 4. 用标签分发实现 std::advance

这是标签分发最经典的案例——`std::advance` 的简化实现：

```cpp
#include <iostream>
#include <iterator>
#include <vector>
#include <list>

template <typename InputIter, typename Distance>
void advanceHelper(InputIter& it, Distance n, std::input_iterator_tag) {
    std::cout << "[input] 单步前进" << std::endl;
    for (Distance i = 0; i < n; ++i) ++it;
}

template <typename BidirectionalIter, typename Distance>
void advanceHelper(BidirectionalIter& it, Distance n, std::bidirectional_iterator_tag) {
    std::cout << "[bidirectional] 双向前进" << std::endl;
    if (n >= 0) {
        for (Distance i = 0; i < n; ++i) ++it;
    } else {
        for (Distance i = 0; i < -n; ++i) --it;
    }
}

template <typename RandomAccessIter, typename Distance>
void advanceHelper(RandomAccessIter& it, Distance n, std::random_access_iterator_tag) {
    std::cout << "[random_access] O(1) 跳跃" << std::endl;
    it += n;
}

template <typename Iter, typename Distance>
void myAdvance(Iter& it, Distance n) {
    using Category = typename std::iterator_traits<Iter>::iterator_category;
    advanceHelper(it, n, Category{});
}

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto vit = v.begin();
    myAdvance(vit, 3);
    std::cout << "vector: " << *vit << std::endl;

    std::list<int> lst = {10, 20, 30, 40, 50};
    auto lit = lst.begin();
    myAdvance(lit, 2);
    std::cout << "list: " << *lit << std::endl;
}
```

输出：

```
[random_access] O(1) 跳跃
vector: 4
[bidirectional] 双向前进
list: 30
```

### 5. 自定义标签分发

标签分发不限于标准库，你可以为自己的类型体系创建标签分发：

```cpp
#include <iostream>
#include <string>
#include <chrono>

struct SequentialTag {};
struct ParallelTag {};
struct GpuTag {};

template <typename Tag>
class ComputeEngine {
public:
    void execute(const std::string& task) {
        executeImpl(task, Tag{});
    }

private:
    void executeImpl(const std::string& task, SequentialTag) {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "[顺序执行] " << task << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "  耗时: 1.0s" << std::endl;
    }

    void executeImpl(const std::string& task, ParallelTag) {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "[并行执行] " << task << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "  耗时: 0.3s" << std::endl;
    }

    void executeImpl(const std::string& task, GpuTag) {
        auto start = std::chrono::high_resolution_clock::now();
        std::cout << "[GPU 执行] " << task << std::endl;
        auto end = std::chrono::high_resolution_clock::now();
        std::cout << "  耗时: 0.05s" << std::endl;
    }
};

template <typename Tag>
void runBenchmark(const std::string& name) {
    ComputeEngine<Tag> engine;
    std::cout << "--- " << name << " ---" << std::endl;
    engine.execute("矩阵乘法 1024x1024");
    engine.execute("图像高斯模糊");
}

int main() {
    runBenchmark<SequentialTag>("顺序引擎");
    runBenchmark<ParallelTag>("并行引擎");
    runBenchmark<GpuTag>("GPU 引擎");
}
```

输出：

```
--- 顺序引擎 ---
[顺序执行] 矩阵乘法 1024x1024
  耗时: 1.0s
[顺序执行] 图像高斯模糊
  耗时: 1.0s
--- 并行引擎 ---
[并行执行] 矩阵乘法 1024x1024
  耗时: 0.3s
[并行执行] 图像高斯模糊
  耗时: 0.3s
--- GPU 引擎 ---
[GPU 执行] 矩阵乘法 1024x1024
  耗时: 0.05s
[GPU 执行] 图像高斯模糊
  耗时: 0.05s
```

### 6. 标签分发 vs SFINAE

SFINAE 也能实现编译期分支选择，但方式不同：

```cpp
#include <iostream>
#include <type_traits>
#include <vector>
#include <list>

template <typename Iter>
typename std::enable_if<
    std::is_same<
        typename std::iterator_traits<Iter>::iterator_category,
        std::random_access_iterator_tag
    >::value
>::type
advanceSFINAE(Iter& it, int n) {
    std::cout << "SFINAE: O(1) 跳跃" << std::endl;
    it += n;
}

template <typename Iter>
typename std::enable_if<
    !std::is_same<
        typename std::iterator_traits<Iter>::iterator_category,
        std::random_access_iterator_tag
    >::value
>::type
advanceSFINAE(Iter& it, int n) {
    std::cout << "SFINAE: 单步前进" << std::endl;
    for (int i = 0; i < n; ++i) ++it;
}

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto vit = v.begin();
    advanceSFINAE(vit, 3);
    std::cout << *vit << std::endl;

    std::list<int> lst = {10, 20, 30};
    auto lit = lst.begin();
    advanceSFINAE(lit, 2);
    std::cout << *lit << std::endl;
}
```

| 特性 | 标签分发 | SFINAE |
|-----|---------|--------|
| 代码可读性 | ✅ 高，逻辑清晰 | ❌ 低，模板元编程复杂 |
| 错误信息 | ✅ 友好 | ❌ 冗长难懂 |
| 扩展性 | ✅ 添加新标签即可 | ❌ 需修改 enable_if 条件 |
| 继承回退 | ✅ 天然支持 | ❌ 需手动处理 |
| 编译速度 | ✅ 较快 | ❌ 模板实例化开销大 |
| 条件灵活性 | ❌ 基于类型分类 | ✅ 任意布尔条件 |
| 优先级控制 | ✅ 继承链自动处理 | ❌ 需要手动排优先级 |

### 7. 标签分发 vs if constexpr

C++17 的 `if constexpr` 提供了更直观的编译期分支：

```cpp
#include <iostream>
#include <iterator>
#include <vector>
#include <list>
#include <type_traits>

template <typename Iter>
void advanceConstexpr(Iter& it, int n) {
    using Category = typename std::iterator_traits<Iter>::iterator_category;

    if constexpr (std::is_same_v<Category, std::random_access_iterator_tag>) {
        std::cout << "if constexpr: O(1) 跳跃" << std::endl;
        it += n;
    } else if constexpr (std::is_base_of_v<std::bidirectional_iterator_tag, Category>) {
        std::cout << "if constexpr: 双向前进" << std::endl;
        if (n >= 0) for (int i = 0; i < n; ++i) ++it;
        else for (int i = 0; i < -n; ++i) --it;
    } else {
        std::cout << "if constexpr: 单步前进" << std::endl;
        for (int i = 0; i < n; ++i) ++it;
    }
}

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto vit = v.begin();
    advanceConstexpr(vit, 3);
    std::cout << *vit << std::endl;

    std::list<int> lst = {10, 20, 30};
    auto lit = lst.begin();
    advanceConstexpr(lit, 2);
    std::cout << *lit << std::endl;
}
```

| 特性 | 标签分发 | if constexpr |
|-----|---------|-------------|
| 代码位置 | 分散在多个重载 | 集中在一个函数 |
| 可读性 | ✅ 分支隔离清晰 | ✅ 逻辑集中直观 |
| 编译期错误 | 不匹配时无重载 | 分支内语法仍需合法 |
| 二进制体积 | 每个重载独立编译 | 只编译匹配的分支 |
| 扩展新分支 | 添加新重载即可 | 修改函数体 |
| 开放封闭原则 | ✅ 开放扩展 | ❌ 修改已有代码 |
| C++ 版本要求 | C++98 | C++17 |

### 8. 标签分发 vs C++20 Concepts

C++20 Concepts 是最现代的编译期约束方式：

```cpp
#include <iostream>
#include <iterator>
#include <vector>
#include <list>
#include <concepts>

template <typename Iter>
requires std::random_access_iterator<Iter>
void advanceConcepts(Iter& it, int n) {
    std::cout << "Concepts: O(1) 跳跃" << std::endl;
    it += n;
}

template <typename Iter>
requires std::bidirectional_iterator<Iter>
         && (!std::random_access_iterator<Iter>)
void advanceConcepts(Iter& it, int n) {
    std::cout << "Concepts: 双向前进" << std::endl;
    if (n >= 0) for (int i = 0; i < n; ++i) ++it;
    else for (int i = 0; i < -n; ++i) --it;
}

template <typename Iter>
requires std::input_iterator<Iter>
         && (!std::bidirectional_iterator<Iter>)
void advanceConcepts(Iter& it, int n) {
    std::cout << "Concepts: 单步前进" << std::endl;
    for (int i = 0; i < n; ++i) ++it;
}

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    auto vit = v.begin();
    advanceConcepts(vit, 3);
    std::cout << *vit << std::endl;

    std::list<int> lst = {10, 20, 30};
    auto lit = lst.begin();
    advanceConcepts(lit, 2);
    std::cout << *lit << std::endl;
}
```

| 特性 | 标签分发 | Concepts |
|-----|---------|---------|
| 语法复杂度 | 中等 | 低 |
| 错误信息 | 较好 | ✅ 最佳 |
| 语义表达 | 隐式（通过标签类型） | ✅ 显式（约束声明） |
| 约束组合 | 继承链 | `&&`, `||` |
| C++ 版本 | C++98 | C++20 |
| 标准库支持 | `<iterator>` 标签 | `<concepts>` 概念 |
| 推荐度（C++20+） | 向后兼容 | ✅ 首选 |

### 9. 标签分发的继承回退机制

标签继承使得"最佳匹配"自动生效，这是标签分发的精髓：

```cpp
#include <iostream>

struct Level1Tag {};
struct Level2Tag : Level1Tag {};
struct Level3Tag : Level2Tag {};

void handle(int value, Level1Tag) {
    std::cout << "Level1 处理: " << value << std::endl;
}

void handle(int value, Level2Tag) {
    std::cout << "Level2 处理: " << value << std::endl;
}

void handle(int value, Level3Tag) {
    std::cout << "Level3 处理: " << value << std::endl;
}

template <typename Tag>
void dispatch(int value, Tag tag) {
    handle(value, tag);
}

int main() {
    dispatch(1, Level1Tag{});
    dispatch(2, Level2Tag{});
    dispatch(3, Level3Tag{});
}
```

输出：

```
Level1 处理: 1
Level2 处理: 2
Level3 处理: 3
```

当 `Level3Tag` 传入时，虽然它也能转换为 `Level2Tag` 和 `Level1Tag`，但编译器选择最匹配的 `handle(int, Level3Tag)`。

如果删除 `Level3Tag` 的重载，`Level3Tag{}` 会回退到 `Level2Tag` 的重载：

```cpp
void handle(int value, Level1Tag) {
    std::cout << "Level1 处理: " << value << std::endl;
}

void handle(int value, Level2Tag) {
    std::cout << "Level2 处理: " << value << std::endl;
}

int main() {
    handle(3, Level3Tag{});
}
```

输出：`Level2 处理: 3`

| 传入标签 | 可匹配重载 | 选择结果 |
|---------|-----------|---------|
| `Level1Tag` | Level1 | Level1 |
| `Level2Tag` | Level1, Level2 | Level2（更匹配） |
| `Level3Tag` | Level1, Level2, Level3 | Level3（最匹配） |
| `Level3Tag`（无 L3 重载） | Level1, Level2 | Level2（回退） |

### 10. 极简总结

| 要点 | 说明 |
|-----|------|
| **定义** | 用空标签类型引导函数重载决议，实现编译期分支选择 |
| **核心机制** | 标签类型 + 函数重载 + 继承回退 |
| **零开销** | 标签是空类，编译器完全优化 |
| **标准库应用** | `iterator_traits`、`type_traits` 标签体系 |
| **vs SFINAE** | 更可读、更易扩展、错误信息更友好 |
| **vs if constexpr** | 遵循开放封闭原则，但代码分散 |
| **vs Concepts** | Concepts 是 C++20 首选，标签分发用于向后兼容 |
| **适用场景** | 类型分类体系、迭代器类别、算法策略选择 |
| **关键设计** | 标签继承链决定回退优先级 |

**口诀**：空类做标签，重载来分发；继承链回退，编译零开销。

***

### 相关阅读

- [SFINAE与TypeTraits](./00-SFINAE与TypeTraits.md)
- [什么是if-constexpr](./04-什么是if-constexpr.md)
- [什么是C++20-Concepts](./06-什么是C++20-Concepts.md)

***