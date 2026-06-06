# STL算法与迭代器

> C++标准模板库算法与迭代器机制

---

> **Algorithms are the heart of the STL. Containers are just the body.** — Alexander Stepanov
> （算法是STL的心脏，容器只是身体。）

> **好的算法让数据结构发挥最大价值，好的迭代器让算法与容器解耦。**
> （Good algorithms maximize the value of data structures; good iterators decouple algorithms from containers.）

---

> **🎯 工欲善其事，必先利其器。**
>
> （掌握STL算法，让代码更简洁高效。）

---

> 💡 **通俗理解 - 什么是STL算法与迭代器？**
>
> 想象你在图书馆找书：
> - **迭代器** 就像"书架指针"——告诉你当前在哪本书，怎么移到下一本
> - **算法** 就像"图书管理员"——不管书架长什么样，只要能一本一本翻，就能帮你排序、查找、统计
>
> 关键思想：**算法不关心容器是什么，只关心迭代器能做什么。**

## 1. 常用算法

### 1. 排序与查找

```cpp
#include <algorithm>
#include <vector>

std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6};

// 排序
std::sort(v.begin(), v.end());                    // 升序
std::sort(v.begin(), v.end(), std::greater<int>()); // 降序

// 自定义比较
std::sort(v.begin(), v.end(), [](int a, int b) {
    return a > b;
});

// 二分查找（需要先排序）
bool found = std::binary_search(v.begin(), v.end(), 5);

// 查找
auto it = std::find(v.begin(), v.end(), 5);
auto it2 = std::find_if(v.begin(), v.end(), [](int x) {
    return x > 5;
});
```

### 2. 修改操作

```cpp
#include <algorithm>

std::vector<int> v = {1, 2, 3, 4, 5};

// 填充
std::fill(v.begin(), v.end(), 0);

// 复制
std::vector<int> v2(v.size());
std::copy(v.begin(), v.end(), v2.begin());

// 替换
std::replace(v.begin(), v.end(), 0, 10);

// 删除特定值
v.erase(std::remove(v.begin(), v.end(), 10), v.end());

// 删除满足条件的元素
v.erase(std::remove_if(v.begin(), v.end(), [](int x) {
    return x % 2 == 0;
}), v.end());
```

### 3. 聚合操作

```cpp
#include <algorithm>
#include <numeric>

std::vector<int> v = {1, 2, 3, 4, 5};

// 求和
int sum = std::accumulate(v.begin(), v.end(), 0);

// 计数
size_t n = std::count(v.begin(), v.end(), 3);
size_t n2 = std::count_if(v.begin(), v.end(), [](int x) {
    return x > 3;
});

// 最值
auto maxIt = std::max_element(v.begin(), v.end());
auto minIt = std::min_element(v.begin(), v.end());
auto [minIt2, maxIt2] = std::minmax_element(v.begin(), v.end());

// 全部/任一/无
bool all = std::all_of(v.begin(), v.end(), [](int x) { return x > 0; });
bool any = std::any_of(v.begin(), v.end(), [](int x) { return x > 3; });
bool none = std::none_of(v.begin(), v.end(), [](int x) { return x < 0; });
```

### 4. 分区算法（Partition）

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

void partition_demo() {
    std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6};
    
    // 分区：将满足条件的元素移到前面
    auto it = std::partition(v.begin(), v.end(), [](int x) {
        return x % 2 == 1;  // 奇数在前，偶数在后
    });
    // v 可能变为 {3, 1, 1, 5, 9, 4, 2, 6}
    // it 指向第一个偶数
    
    // stable_partition 保留相对顺序
    std::vector<int> v2 = {3, 1, 4, 1, 5, 9, 2, 6};
    auto it2 = std::stable_partition(v2.begin(), v2.end(), [](int x) {
        return x % 2 == 1;
    });
    
    // partition_point：在已分区的范围中找到分区点
    auto pp = std::partition_point(v.begin(), v.end(), [](int x) {
        return x % 2 == 1;
    });
    
    // is_partitioned：检查是否已分区
    bool partitioned = std::is_partitioned(v.begin(), v.end(), [](int x) {
        return x % 2 == 1;
    });
    
    // partition_copy：将元素复制到两个输出范围
    std::vector<int> odds, evens;
    odds.resize(v.size());
    evens.resize(v.size());
    auto [odds_end, evens_end] = std::partition_copy(
        v.begin(), v.end(), odds.begin(), evens.begin(),
        [](int x) { return x % 2 == 1; }
    );
    odds.erase(odds_end, odds.end());
    evens.erase(evens_end, evens.end());
    
    std::cout << "奇数: ";
    for (int x : odds) std::cout << x << " ";
    std::cout << "\n偶数: ";
    for (int x : evens) std::cout << x << " ";
    std::cout << std::endl;
}
```

### 5. 堆算法（Heap）

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

void heap_demo() {
    std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6};
    
    // make_heap：将范围构建为最大堆
    std::make_heap(v.begin(), v.end());
    // v 可能变为 {9, 6, 4, 3, 5, 1, 2, 1}
    
    // push_heap：将最后一个元素加入堆
    v.push_back(10);
    std::push_heap(v.begin(), v.end());
    
    // pop_heap：将堆顶移到末尾
    std::pop_heap(v.begin(), v.end());
    int max_val = v.back();  // 10
    v.pop_back();
    
    // sort_heap：将堆排序为升序序列
    std::sort_heap(v.begin(), v.end());
    // v 变为 {1, 1, 2, 3, 4, 5, 6, 9}
    
    // is_heap：检查是否为堆
    std::make_heap(v.begin(), v.end());
    bool is_heap = std::is_heap(v.begin(), v.end());
    
    // is_heap_until：找到第一个破坏堆性质的位置
    auto heap_end = std::is_heap_until(v.begin(), v.end());
    
    // 使用堆实现优先队列
    std::vector<int> task_queue;
    auto add_task = [&](int priority) {
        task_queue.push_back(priority);
        std::push_heap(task_queue.begin(), task_queue.end());
    };
    auto get_highest = [&]() -> int {
        std::pop_heap(task_queue.begin(), task_queue.end());
        int top = task_queue.back();
        task_queue.pop_back();
        return top;
    };
    
    add_task(5);
    add_task(3);
    add_task(8);
    add_task(1);
    
    std::cout << "按优先级处理: ";
    while (!task_queue.empty()) {
        std::cout << get_highest() << " ";
    }
    std::cout << std::endl;  // 输出: 8 5 3 1
}
```

### 6. 排列算法（Permutation）

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

void permutation_demo() {
    std::vector<int> v = {1, 2, 3};
    
    // next_permutation：下一个字典序排列
    std::cout << "所有排列: " << std::endl;
    do {
        for (int x : v) std::cout << x << " ";
        std::cout << std::endl;
    } while (std::next_permutation(v.begin(), v.end()));
    // 输出: 1 2 3, 1 3 2, 2 1 3, 2 3 1, 3 1 2, 3 2 1
    
    // prev_permutation：上一个字典序排列
    std::vector<int> v2 = {3, 2, 1};
    std::cout << "逆序排列: " << std::endl;
    do {
        for (int x : v2) std::cout << x << " ";
        std::cout << std::endl;
    } while (std::prev_permutation(v2.begin(), v2.end()));
    
    // is_permutation：检查是否为另一个序列的排列
    std::vector<int> a = {1, 2, 3, 4};
    std::vector<int> b = {3, 1, 4, 2};
    bool perm = std::is_permutation(a.begin(), a.end(), b.begin());
    std::cout << "b是a的排列: " << (perm ? "是" : "否") << std::endl;
}
```

### 7. 算法复杂度汇总

| 算法 | 时间复杂度 | 空间复杂度 | 备注 |
|------|-----------|-----------|------|
| `sort` | O(n log n) | O(log n) | 内省排序 |
| `stable_sort` | O(n log n) | O(n) | 归并排序 |
| `partial_sort` | O(n log k) | O(log k) | 只排序前k个 |
| `nth_element` | O(n) | O(1) | 快速选择 |
| `partition` | O(n) | O(1) | 不稳定 |
| `stable_partition` | O(n) | O(n) | 稳定分区 |
| `make_heap` | O(n) | O(1) | 堆化 |
| `push_heap` | O(log n) | O(1) | 插入到堆 |
| `pop_heap` | O(log n) | O(1) | 移除堆顶 |
| `next_permutation` | O(n) | O(1) | 平均O(1) |
| `binary_search` | O(log n) | O(1) | 随机访问迭代器 |
| `lower_bound` | O(log n) | O(1) | 第一个>= |
| `upper_bound` | O(log n) | O(1) | 第一个> |

### 8. 集合算法

```cpp
#include <algorithm>
#include <vector>
#include <iostream>

void set_algorithm_demo() {
    std::vector<int> v1 = {1, 2, 3, 4, 5};
    std::vector<int> v2 = {3, 4, 5, 6, 7};
    std::vector<int> result;
    
    // 需要先排序
    std::sort(v1.begin(), v1.end());
    std::sort(v2.begin(), v2.end());
    
    // 并集
    result.resize(v1.size() + v2.size());
    auto it = std::set_union(v1.begin(), v1.end(), v2.begin(), v2.end(), result.begin());
    result.erase(it, result.end());
    // result = {1, 2, 3, 4, 5, 6, 7}
    
    // 交集
    result.clear();
    result.resize(std::min(v1.size(), v2.size()));
    it = std::set_intersection(v1.begin(), v1.end(), v2.begin(), v2.end(), result.begin());
    result.erase(it, result.end());
    // result = {3, 4, 5}
    
    // 差集（v1 - v2）
    result.clear();
    result.resize(v1.size());
    it = std::set_difference(v1.begin(), v1.end(), v2.begin(), v2.end(), result.begin());
    result.erase(it, result.end());
    // result = {1, 2}
    
    // 对称差集
    result.clear();
    result.resize(v1.size() + v2.size());
    it = std::set_symmetric_difference(v1.begin(), v1.end(), v2.begin(), v2.end(), result.begin());
    result.erase(it, result.end());
    // result = {1, 2, 6, 7}
    
    // 包含检查
    std::vector<int> subset = {3, 5};
    bool includes = std::includes(v1.begin(), v1.end(), subset.begin(), subset.end());
    std::cout << "v1包含{3,5}: " << (includes ? "是" : "否") << std::endl;
}
```

### 9. 数值算法

```cpp
#include <numeric>
#include <vector>
#include <iostream>

void numeric_algorithm_demo() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    std::vector<int> result(v.size());
    
    // 累加
    int sum = std::accumulate(v.begin(), v.end(), 0);  // 15
    
    // 内积（点积）
    int dot = std::inner_product(v.begin(), v.end(), v.begin(), 0);  // 55
    
    // 部分和
    std::partial_sum(v.begin(), v.end(), result.begin());
    // result = {1, 3, 6, 10, 15}
    
    // 相邻差
    std::adjacent_difference(v.begin(), v.end(), result.begin());
    // result = {1, 1, 1, 1, 1}
    
    // 前缀和（C++17）
    std::exclusive_scan(v.begin(), v.end(), result.begin(), 0);
    // result = {0, 1, 3, 6, 10}
    
    std::inclusive_scan(v.begin(), v.end(), result.begin());
    // result = {1, 3, 6, 10, 15}
    
    // 变换累加（C++17）
    auto transformed = std::transform_reduce(v.begin(), v.end(), 0,
        std::plus<>{}, [](int x) { return x * x; });
    // 1 + 4 + 9 + 16 + 25 = 55
}
```

---

## 2. 迭代器

### 1. 迭代器类型

| 类型 | 支持操作 |
|-----|---------|
| 输入迭代器 | 只读，单遍 |
| 输出迭代器 | 只写，单遍 |
| 前向迭代器 | 读写，多遍 |
| 双向迭代器 | ++, -- |
| 随机访问迭代器 | +, -, +=, -=, [] |

### 2. 迭代器使用

```cpp
std::vector<int> v = {1, 2, 3, 4, 5};

// 正向迭代器
for (auto it = v.begin(); it != v.end(); ++it) {
    std::cout << *it << " ";
}

// 反向迭代器
for (auto it = v.rbegin(); it != v.rend(); ++it) {
    std::cout << *it << " ";
}

// const迭代器
for (auto it = v.cbegin(); it != v.cend(); ++it) {
    // *it = 10;  // 错误：不能修改
}
```

### 3. 迭代器失效

```cpp
std::vector<int> v = {1, 2, 3, 4, 5};

// 错误：迭代器失效
for (auto it = v.begin(); it != v.end(); ++it) {
    if (*it == 3) {
        v.erase(it);  // 迭代器失效！
    }
}

// 正确：使用返回值
for (auto it = v.begin(); it != v.end(); ) {
    if (*it == 3) {
        it = v.erase(it);
    } else {
        ++it;
    }
}
```

### 4. 迭代器适配器

```cpp
#include <iterator>
#include <vector>
#include <iostream>

void iterator_adapter_demo() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    
    // back_inserter：在尾部追加
    std::vector<int> v1;
    std::copy(v.begin(), v.end(), std::back_inserter(v1));
    // v1 = {1, 2, 3, 4, 5}
    
    // front_inserter：在头部插入（仅适用于list/deque）
    std::deque<int> d;
    std::copy(v.begin(), v.end(), std::front_inserter(d));
    // d = {5, 4, 3, 2, 1}
    
    // inserter：在指定位置插入
    std::set<int> s = {10, 20, 30};
    std::copy(v.begin(), v.end(), std::inserter(s, s.begin()));
    // s = {1, 2, 3, 4, 5, 10, 20, 30}
    
    // move_iterator：移动元素
    std::vector<std::string> src = {"hello", "world"};
    std::vector<std::string> dst;
    std::copy(std::make_move_iterator(src.begin()),
              std::make_move_iterator(src.end()),
              std::back_inserter(dst));
    // src中的元素被移走
    
    // reverse_iterator
    std::reverse_iterator<std::vector<int>::iterator> rit(v.end());
    std::reverse_iterator<std::vector<int>::iterator> rend(v.begin());
    for (auto it = rit; it != rend; ++it) {
        std::cout << *it << " ";  // 5 4 3 2 1
    }
    std::cout << std::endl;
}
```

### 5. istream_iterator / ostream_iterator

```cpp
#include <iterator>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>

void stream_iterator_demo() {
    // 从输入流读取整数
    std::istringstream input("1 2 3 4 5");
    std::vector<int> v;
    
    std::copy(std::istream_iterator<int>(input),
              std::istream_iterator<int>(),
              std::back_inserter(v));
    // v = {1, 2, 3, 4, 5}
    
    // 输出到输出流
    std::ostringstream output;
    std::copy(v.begin(), v.end(),
              std::ostream_iterator<int>(output, " "));
    std::cout << output.str() << std::endl;  // "1 2 3 4 5 "
    
    // 带分隔符的输出
    std::ostringstream output2;
    std::copy(v.begin(), v.end(),
              std::ostream_iterator<int>(output2, ", "));
    std::cout << output2.str() << std::endl;  // "1, 2, 3, 4, 5, "
    
    // 从标准输入读取（交互式）
    // std::copy(std::istream_iterator<int>(std::cin),
    //           std::istream_iterator<int>(),
    //           std::back_inserter(v));
    
    // 读取字符串
    std::istringstream input2("apple banana cherry");
    std::vector<std::string> words;
    std::copy(std::istream_iterator<std::string>(input2),
              std::istream_iterator<std::string>(),
              std::back_inserter(words));
    // words = {"apple", "banana", "cherry"}
}
```

### 6. 自定义迭代器实现

```cpp
#include <iterator>
#include <iostream>
#include <algorithm>
#include <vector>

// 实现一个步进迭代器（每次跳过N个元素）
template<typename Iterator>
class StepIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    using difference_type = typename std::iterator_traits<Iterator>::difference_type;
    using pointer = typename std::iterator_traits<Iterator>::pointer;
    using reference = typename std::iterator_traits<Iterator>::reference;
    
    StepIterator(Iterator current, Iterator end, size_t step)
        : current_(current), end_(end), step_(step) {}
    
    reference operator*() const { return *current_; }
    pointer operator->() const { return current_; }
    
    StepIterator& operator++() {
        for (size_t i = 0; i < step_ && current_ != end_; ++i) {
            ++current_;
        }
        return *this;
    }
    
    StepIterator operator++(int) {
        StepIterator tmp = *this;
        ++(*this);
        return tmp;
    }
    
    bool operator==(const StepIterator& other) const {
        return current_ == other.current_;
    }
    
    bool operator!=(const StepIterator& other) const {
        return !(*this == other);
    }
    
private:
    Iterator current_;
    Iterator end_;
    size_t step_;
};

// 辅助函数
template<typename Iterator>
auto make_step_iterator(Iterator begin, Iterator end, size_t step) {
    return StepIterator<Iterator>(begin, end, step);
}

// 使用示例
void custom_iterator_demo() {
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    std::cout << "每隔2个元素: ";
    auto step_begin = make_step_iterator(v.begin(), v.end(), 2);
    auto step_end = make_step_iterator(v.end(), v.end(), 2);
    for (auto it = step_begin; it != step_end; ++it) {
        std::cout << *it << " ";  // 1 4 7 10
    }
    std::cout << std::endl;
}

// 实现过滤迭代器
template<typename Iterator, typename Predicate>
class FilterIterator {
public:
    using iterator_category = std::forward_iterator_tag;
    using value_type = typename std::iterator_traits<Iterator>::value_type;
    using reference = typename std::iterator_traits<Iterator>::reference;
    
    FilterIterator(Iterator current, Iterator end, Predicate pred)
        : current_(current), end_(end), pred_(pred) {
        advance_to_valid();
    }
    
    reference operator*() const { return *current_; }
    
    FilterIterator& operator++() {
        ++current_;
        advance_to_valid();
        return *this;
    }
    
    FilterIterator operator++(int) {
        FilterIterator tmp = *this;
        ++(*this);
        return tmp;
    }
    
    bool operator==(const FilterIterator& other) const {
        return current_ == other.current_;
    }
    
    bool operator!=(const FilterIterator& other) const {
        return !(*this == other);
    }
    
private:
    void advance_to_valid() {
        while (current_ != end_ && !pred_(*current_)) {
            ++current_;
        }
    }
    
    Iterator current_;
    Iterator end_;
    Predicate pred_;
};

void filter_iterator_demo() {
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    auto is_odd = [](int x) { return x % 2 == 1; };
    FilterIterator filter_begin(v.begin(), v.end(), is_odd);
    FilterIterator filter_end(v.end(), v.end(), is_odd);
    
    std::cout << "过滤后的奇数: ";
    for (auto it = filter_begin; it != filter_end; ++it) {
        std::cout << *it << " ";  // 1 3 5 7 9
    }
    std::cout << std::endl;
}
```

### 7. C++20 Range适配器

```cpp
#include <ranges>
#include <vector>
#include <iostream>

void range_adapter_demo() {
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // 链式操作：过滤 → 变换 → 取前3个
    auto result = v
        | std::views::filter([](int x) { return x % 2 == 0; })
        | std::views::transform([](int x) { return x * x; })
        | std::views::take(3);
    
    std::cout << "前3个偶数的平方: ";
    for (int x : result) {
        std::cout << x << " ";  // 4 16 36
    }
    std::cout << std::endl;
    
    // 其他Range适配器
    auto reversed = v | std::views::reverse;     // 反转
    auto dropped = v | std::views::drop(3);       // 跳过前3个
    auto keys = std::views::keys;                 // map的key
    auto values = std::views::values;             // map的value
    
    // 区间构造（iota_view）
    auto numbers = std::views::iota(1, 10);       // 1到9
    
    // 带步长的区间
    auto step2 = std::views::iota(0, 20)
               | std::views::filter([](int x) { return x % 2 == 0; })
               | std::views::take(5);
    
    // 惰性求值：只在遍历时计算
    std::cout << "范围for+过滤: ";
    for (int x : v | std::views::filter([](int n) { return n > 5; })) {
        std::cout << x << " ";  // 6 7 8 9 10
    }
    std::cout << std::endl;
}

// C++20的std::ranges命名空间算法
void ranges_algorithm_demo() {
    std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6};
    
    // 直接传容器（不需要begin/end）
    std::ranges::sort(v);
    
    // 使用投影（projection）
    struct Person { std::string name; int age; };
    std::vector<Person> people = {{"Alice", 30}, {"Bob", 25}, {"Charlie", 35}};
    
    // 按年龄排序
    std::ranges::sort(people, std::less<>{}, &Person::age);
    
    // 范围查找
    auto it = std::ranges::find(v, 5);
    
    // 范围视图直接传递给算法
    auto even_view = v | std::views::filter([](int n) { return n % 2 == 0; });
    for (int x : even_view) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
}
```

---

## 3. 本章小结

### 1. 核心概念回顾

| 概念 | 说明 | 示例 |
|-----|------|------|
| **vector** | 动态数组 | `std::vector<int>` |
| **deque** | 双端队列 | `std::deque<int>` |
| **list** | 双向链表 | `std::list<int>` |
| **string** | 字符串 | `std::string s = "hello"` |
| **map** | 有序映射 | `std::map<string, int>` |
| **unordered_map** | 无序映射 | `std::unordered_map<K, V>` |
| **set** | 有序集合 | `std::set<int>` |
| **stack** | 栈（LIFO） | `std::stack<int>` |
| **queue** | 队列（FIFO） | `std::queue<int>` |
| **priority_queue** | 优先队列 | `std::priority_queue<int>` |
| **optional** | 可选值 | `std::optional<int>` |
| **variant** | 类型安全联合体 | `std::variant<int, string>` |
| **any** | 任意类型容器 | `std::any` |
| **tuple** | 多元组 | `std::tuple<int, double, string>` |
| **algorithm** | 算法库 | `std::sort`, `std::find` |

### 2. 关键语法要点

1. **vector**：随机访问高效，中间插入/删除低效
2. **deque**：两端操作高效，分段连续内存
3. **list**：任意位置O(1)插入删除（已知位置），不支持随机访问
4. **stack/queue/priority_queue**：容器适配器，限制底层容器接口
5. **optional**：替代哨兵值，类型安全地表示"可能没有值"
6. **variant**：替代union，类型安全的多类型容器
7. **any**：任意类型容器，需要`any_cast`安全访问
8. **tuple**：多返回值，结构化绑定简化访问
9. **map**：有序，查找O(log n)
10. **unordered_map**：无序，查找平均O(1)
11. **算法**：与容器分离，通过迭代器操作
12. **迭代器失效**：插入/删除时注意

### 3. 最佳实践

1. 使用 `reserve` 预分配 vector 容量
2. 使用 `emplace_back` 替代 `push_back`
3. 使用范围 for 简化遍历
4. 选择合适的容器类型
5. 注意迭代器失效问题
6. 需要头部操作时优先选 `deque` 而非 `vector`
7. 频繁中间插入删除时考虑 `list`，但注意缓存不友好
8. 优先使用 `optional` 替代哨兵值表示"可能没有值"
9. 优先使用 `variant` 替代 `union`，获得类型安全
10. 使用结构化绑定简化 `tuple` 和 `pair` 的访问
11. 使用 `back_inserter` 配合 `copy` 简化容器填充
12. 使用 `istream_iterator` 简化输入解析

---

**上一章：** [第3章：智能指针与内存管理](08-智能指针与内存管理.md)\
**下一章：** [第5章：正则表达式](16-正则表达式.md)

***

### 4. 相关章节

- [Ranges](./24-Ranges.md) — C++20视图与管道操作，STL算法的现代化演进
- [C++标准库与第三方库学习指南](../03-问题解答/07-现代CPP标准库/18-C++标准库与第三方库.md) — 标准库分类全览、学习方法
