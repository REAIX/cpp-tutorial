# 什么是 STL
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[STL算法与迭代器](../../02-CPP/15-STL算法与迭代器.md)

> **The STL is C++'s greatest gift to programmers: generic, efficient, and reusable.** — STL 是 C++ 赠予程序员最伟大的礼物：泛型、高效、可复用。

***

### 1. 一句话概括

**STL**（Standard Template Library，标准模板库）= C++ 标准库的核心部分，提供通用的**容器**、**迭代器**和**算法**，让你不必从零实现常见的数据结构和算法。

***

### 2. 生活类比

把 STL 比作**工具箱**：

- **容器** → 不同形状的收纳盒：抽屉（vector）、格子柜（map）、管道（queue）
- **迭代器** → 每个收纳盒配的取物夹，用统一的方式取出物品
- **算法** → 说明书：不管哪个收纳盒，排序、查找、统计的方法都一样

你不需要自己打造收纳盒和取物夹，工具箱里全都有，拿来就用。

### 3. STL 的三大组件

```
┌──────────────────────────────────────┐
│              算法 (Algorithms)         │
│   sort, find, copy, transform...     │
├──────────────────────────────────────┤
│          迭代器 (Iterators)           │
│   begin(), end(), ++, *, ==          │
├──────────────────────────────────────┤
│           容器 (Containers)           │
│   vector, list, map, set...          │
└──────────────────────────────────────┘
```

| 组件 | 作用 | 示例 |
|------|------|------|
| 容器 | 存储和组织数据 | `vector`, `map`, `set` |
| 迭代器 | 统一遍历容器的方式 | `begin()`, `end()` |
| 算法 | 通用操作 | `sort()`, `find()`, `count()` |

### 4. 容器

STL 容器分为四大类：

| 类别 | 容器 | 特点 |
|------|------|------|
| 序列容器 | `vector`, `deque`, `list`, `forward_list`, `array` | 按插入顺序存储 |
| 关联容器 | `set`, `map`, `multiset`, `multimap` | 有序，基于红黑树 |
| 无序关联容器 | `unordered_set`, `unordered_map` 等 | 哈希表实现，查找O(1) |
| 容器适配器 | `stack`, `queue`, `priority_queue` | 对序列容器的封装 |

```cpp
#include <iostream>
#include <vector>
#include <map>
#include <unordered_map>
#include <stack>

int main() {
    // 序列容器：动态数组
    std::vector<int> nums = {3, 1, 4, 1, 5};
    nums.push_back(9);

    // 关联容器：键值对（有序）
    std::map<std::string, int> ages;
    ages["Alice"] = 25;
    ages["Bob"] = 30;

    // 无序关联容器：键值对（哈希）
    std::unordered_map<std::string, int> scores;
    scores["Math"] = 95;

    // 容器适配器：栈
    std::stack<int> stk;
    stk.push(1);
    stk.push(2);
    std::cout << stk.top() << std::endl;  // 2
}
```

### 5. 迭代器

迭代器是容器和算法之间的桥梁，提供统一的遍历接口：

```cpp
#include <iostream>
#include <vector>
#include <list>

template<typename Container>
void print_all(const Container& c) {
    // 用迭代器统一遍历，不管容器类型
    for (auto it = c.begin(); it != c.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
}

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    std::list<double> l = {1.1, 2.2, 3.3};

    print_all(v);  // 1 2 3 4 5
    print_all(l);  // 1.1 2.2 3.3

    // C++11 范围 for（底层也是迭代器）
    for (const auto& elem : v) {
        std::cout << elem << " ";
    }
}
```

| 迭代器类型 | 能力 | 示例容器 |
|-----------|------|---------|
| 输入迭代器 | 只读、单向 | `istream_iterator` |
| 输出迭代器 | 只写、单向 | `ostream_iterator` |
| 前向迭代器 | 读写、单向 | `forward_list` |
| 双向迭代器 | 读写、双向 | `list`, `set`, `map` |
| 随机访问迭代器 | 读写、随机跳跃 | `vector`, `deque`, `array` |

### 6. 算法

STL 提供了 100+ 通用算法，通过迭代器操作容器：

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <numeric>

int main() {
    std::vector<int> v = {5, 2, 8, 1, 9, 3};

    // 排序
    std::sort(v.begin(), v.end());
    // v = {1, 2, 3, 5, 8, 9}

    // 查找
    auto it = std::find(v.begin(), v.end(), 5);
    if (it != v.end()) {
        std::cout << "找到: " << *it << std::endl;
    }

    // 计数
    int cnt = std::count(v.begin(), v.end(), 1);

    // 累加
    int sum = std::accumulate(v.begin(), v.end(), 0);
    std::cout << "总和: " << sum << std::endl;

    // 判断
    bool all_positive = std::all_of(v.begin(), v.end(),
        [](int x) { return x > 0; });

    // 变换
    std::transform(v.begin(), v.end(), v.begin(),
        [](int x) { return x * 2; });
}
```

### 7. STL 的历史

| 时间 | 事件 |
|------|------|
| 1979 年 | Alexander Stepanov 开始研究泛型编程 |
| 1994 年 | STL 被 HP 公司捐赠给 C++ 标准委员会 |
| 1998 年 | C++98 标准正式包含 STL |
| 2011 年 | C++11 增加 `unordered_map`、`array`、`forward_list` |
| 2017 年 | C++17 增加 `optional`、`variant`、`any` |
| 2020 年 | C++20 增加 `span`、Ranges 库 |

### 8. STL vs 其他语言的标准库

| 语言 | 标准库 | 容器 | 算法 | 特点 |
|------|--------|------|------|------|
| C++ | STL | 丰富 | 100+ | 泛型、零开销 |
| Java | Collections Framework | 丰富 | 较少 | 面向对象、GC |
| Python | 内置类型 + itertools | 简洁 | 丰富 | 动态类型、易用 |
| Rust | std::collections | 适中 | 适中 | 内存安全 |
| Go | container 包 | 较少 | 较少 | 简洁 |

**STL 的独特优势**：
- **泛型**：一套算法适用于所有容器
- **零开销**：不比你手写的代码慢
- **类型安全**：编译期检查类型
- **可组合**：容器+迭代器+算法自由搭配

### 9. 为什么 STL 如此重要

```cpp
// 不用 STL：手动实现动态数组
class MyArray {
    int* data_;
    int size_;
    int capacity_;
public:
    MyArray() : data_(new int[4]), size_(0), capacity_(4) {}
    ~MyArray() { delete[] data_; }
    void push_back(int val) {
        if (size_ == capacity_) {
            capacity_ *= 2;
            int* new_data = new int[capacity_];
            for (int i = 0; i < size_; ++i) new_data[i] = data_[i];
            delete[] data_;
            data_ = new_data;
        }
        data_[size_++] = val;
    }
    // 还需要：拷贝构造、赋值运算符、边界检查...
};

// 用 STL：一行搞定
std::vector<int> arr;
arr.push_back(42);
```

| 不用 STL | 用 STL |
|---------|--------|
| 手动管理内存 | 自动管理 |
| 容易出 Bug | 经过充分测试 |
| 重复造轮子 | 开箱即用 |
| 不可复用 | 泛型可复用 |
| 性能不确定 | 高度优化 |

### 10. 极简总结

**STL = C++ 标准模板库 → 三大组件：容器（存数据）、迭代器（遍历数据）、算法（处理数据）→ 容器分序列/关联/无序/适配器四类 → 迭代器是容器和算法的桥梁 → 算法通过迭代器操作任意容器 → 泛型+零开销是 STL 的核心优势**

***

> 📌 **相关阅读**
> - [STL容器底层实现](./01-STL容器底层实现.md) — 容器怎么实现
> - [迭代器是什么](../04-CPP核心特性/25-迭代器是什么.md) — STL的桥梁
> - [标准库学习方法论](./19-标准库学习方法论.md) — 怎么学STL

***

### 相关阅读

- [STL容器底层实现](./01-STL容器底层实现.md)
- [迭代器是什么](../04-CPP核心特性/25-迭代器是什么.md)
- [emplace-back与push-back](./02-emplace-back与push-back.md)