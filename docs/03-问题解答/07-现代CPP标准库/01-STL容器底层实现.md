# STL 容器底层实现
> 📖 相关章节：[STL容器](../../02-CPP/14-STL容器.md)、[STL算法](../../02-CPP/15-STL算法与迭代器.md)、[正则表达式](../../02-CPP/16-正则表达式.md)、[文件系统](../../02-CPP/19-文件系统库.md)

### 1. 精髓速览

不同 STL 容器的底层实现决定了它们的性能特征：**vector** 连续内存、**list** 双向链表、**map** 红黑树、**unordered_map** 哈希表。选择容器就是选择数据结构。

***

### 2. 顺序容器底层

| 容器 | 底层实现 | 随机访问 | 头部插入 | 尾部插入 | 中间插入 | 内存连续 |
|------|----------|:---:|:---:|:---:|:---:|:---:|
| vector | 动态数组 | O(1) | O(n) | O(1)均摊 | O(n) | 是 |
| deque | 分段连续数组 | O(1) | O(1) | O(1) | O(n) | 分段 |
| list | 双向链表 | O(n) | O(1) | O(1) | O(1) | 否 |
| forward_list | 单向链表 | O(n) | O(1) | O(n) | O(1) | 否 |
| string | 动态数组（类似vector） | O(1) | O(n) | O(1)均摊 | O(n) | 是 |

#### 1. vector：动态数组

```cpp
// vector 的内部结构
template <typename T>
class vector {
    T* begin_;        // 指向已用内存的起始
    size_t size_;     // 已用元素数量
    size_t capacity_; // 已分配内存的容量
    // [begin_, begin_+size_)        → 已构造的元素
    // [begin_+size_, begin_+capacity_) → 未构造的预留空间
};

// 内存布局：
// begin_ → [elem0][elem1][elem2]...[elemN-1][  未使用空间  ]
//          ^^^^^^^^^^^^^^^^^^^^^^^^             ^^^^^^^^^^^^
//          size_ 个元素                      capacity_ - size_
```

#### 2. deque：分段连续数组

```cpp
// deque 的内部结构
template <typename T>
class deque {
    T** map_;          // 指向多个固定大小缓冲区的指针数组
    size_t map_size_;  // map 的大小
    // 每个缓冲区（chunk/block）大小固定（如512字节）

    // 内存布局：
    // map_ → [ptr0][ptr1][ptr2][ptr3]
    //           ↓     ↓     ↓     ↓
    //         [buf0] [buf1] [buf2] [buf3]
    //         每个buf是连续内存，buf之间不连续
};

// deque 的特点：
// 1. 两端插入都是 O(1)
// 2. 随机访问需要两次解引用：map_[index/block_size][index%block_size]
// 3. 比 vector 稍慢，但比 list 快得多
```

#### 3. list：双向链表

```cpp
// list 的内部结构
template <typename T>
class list {
    struct Node {
        T data;
        Node* prev;
        Node* next;
    };
    Node* head_;  // 哨兵节点（dummy node）
    size_t size_;
};

// 内存布局：
// head_ ↔ [Node1] ↔ [Node2] ↔ [Node3] ↔ head_
// 每个节点独立分配，内存不连续
// 每个节点额外开销：2个指针（16字节，64位系统）
```

### 3. 关联容器底层

| 容器 | 底层实现 | 查找 | 插入 | 删除 | 有序 | 允许重复 |
|------|----------|:---:|:---:|:---:|:---:|:---:|
| map | 红黑树 | O(log n) | O(log n) | O(log n) | 是 | 否 |
| set | 红黑树 | O(log n) | O(log n) | O(log n) | 是 | 否 |
| multimap | 红黑树 | O(log n) | O(log n) | O(log n) | 是 | 是 |
| multiset | 红黑树 | O(log n) | O(log n) | O(log n) | 是 | 是 |
| unordered_map | 哈希表 | O(1)均摊 | O(1)均摊 | O(1)均摊 | 否 | 否 |
| unordered_set | 哈希表 | O(1)均摊 | O(1)均摊 | O(1)均摊 | 否 | 否 |

#### 1. map/set：红黑树

```cpp
// 红黑树的性质
// 1. 每个节点是红色或黑色
// 2. 根节点是黑色
// 3. 叶子节点（NIL）是黑色
// 4. 红色节点的子节点必须是黑色
// 5. 从任一节点到其叶子的所有路径包含相同数目的黑色节点

// 红黑树节点
struct RBNode {
    bool is_red;
    RBNode* left;
    RBNode* right;
    RBNode* parent;
    // key, value...
};

// 红黑树的特点：
// 1. 保证树的高度差不超过2倍 → 查找 O(log n)
// 2. 插入/删除最多3次旋转恢复平衡
// 3. 中序遍历就是有序序列
// 4. 每个节点额外开销：3个指针 + 1个颜色标记（约25字节）
```

#### 2. unordered_map/set：哈希表

```cpp
// 哈希表的内部结构（拉链法）
template <typename Key, typename Value>
class unordered_map {
    struct Bucket {
        Node* head;  // 链表头指针
    };
    Bucket* buckets_;     // 桶数组
    size_t bucket_count_; // 桶数量
    size_t size_;         // 元素数量
    float max_load_factor_; // 最大负载因子（默认1.0）
    Hash hash_func_;      // 哈希函数
    KeyEqual key_eq_;     // 键比较函数
};

// 内存布局（拉链法）：
// buckets_: [ptr0][ptr1][ptr2][ptr3][ptr4]...
//             ↓     ↓           ↓
//           [k,v] [k,v]       [k,v]→[k,v]
//             ↓
//           [k,v]
//
// 每个桶是一个链表，冲突的元素挂在同一个桶上
```

### 4. vector 的扩容机制

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<int> v;
    std::cout << "size=" << v.size() << ", capacity=" << v.capacity() << std::endl;
    // size=0, capacity=0

    for (int i = 0; i < 20; ++i) {
        v.push_back(i);
        std::cout << "push_back(" << i
                  << "): size=" << v.size()
                  << ", capacity=" << v.capacity() << std::endl;
    }
    // 典型输出（GCC，扩容因子为2）：
    // push_back(0):  size=1,  capacity=1
    // push_back(1):  size=2,  capacity=2
    // push_back(2):  size=3,  capacity=4
    // push_back(3):  size=4,  capacity=4
    // push_back(4):  size=5,  capacity=8
    // push_back(7):  size=8,  capacity=8
    // push_back(8):  size=9,  capacity=16
    // push_back(15): size=16, capacity=16
    // push_back(16): size=17, capacity=32
}
```

**扩容过程**：

```
1. 检查 size < capacity → 直接在末尾构造
2. 如果 size == capacity → 需要扩容
   a. 分配新内存（通常翻倍：capacity * 2）
   b. 移动/拷贝旧元素到新内存
   c. 释放旧内存
   d. 更新指针
3. 所有迭代器、指针、引用失效
```

```cpp
// 避免扩容的技巧
std::vector<int> v;
v.reserve(1000);  // 预分配1000个元素的空间，避免多次扩容

// 扩容因子对比
// GCC:     2倍扩容
// MSVC:    1.5倍扩容
// 1.5倍更节省内存，2倍摊还分析更优
```

### 5. 哈希表冲突解决

```cpp
// 方法1：拉链法（separate chaining）— STL 默认
// 每个桶维护一个链表
// 优点：实现简单，删除方便
// 缺点：链表指针开销，缓存不友好

// 方法2：开放寻址法（open addressing）
// 冲突时寻找下一个空位
// 线性探测：h(k)+1, h(k)+2, h(k)+3...
// 二次探测：h(k)+1^2, h(k)+2^2, h(k)+3^2...
// 优点：缓存友好
// 缺点：删除复杂，聚集问题

// 负载因子 = 元素数量 / 桶数量
// 当负载因子超过 max_load_factor（默认1.0）时，自动 rehash
std::unordered_map<int, int> m;
m.max_load_factor(0.75);  // 设置更低的负载因子，减少冲突
m.reserve(1000);          // 预分配足够桶数
```

```cpp
// 自定义哈希函数
struct Person {
    std::string name;
    int age;
};

struct PersonHash {
    size_t operator()(const Person& p) const {
        return std::hash<std::string>()(p.name) ^ (std::hash<int>()(p.age) << 1);
    }
};

struct PersonEqual {
    bool operator()(const Person& a, const Person& b) const {
        return a.name == b.name && a.age == b.age;
    }
};

std::unordered_map<Person, std::string, PersonHash, PersonEqual> person_map;
```

### 6. 容器适配器

| 适配器 | 底层默认容器 | 可替换为 | 特点 |
|--------|-------------|----------|------|
| stack | deque | vector, list | 后进先出（LIFO） |
| queue | deque | list | 先进先出（FIFO） |
| priority_queue | vector + 堆算法 | deque | 最大值优先 |

```cpp
// stack：封装底层容器的 push_back/pop_back
std::stack<int> s;                          // 默认用 deque
std::stack<int, std::vector<int>> sv;       // 用 vector
std::stack<int, std::list<int>> sl;         // 用 list

// queue：封装底层容器的 push_back/pop_front
std::queue<int> q;                          // 默认用 deque
std::queue<int, std::list<int>> ql;         // 用 list（vector 不能 pop_front）

// priority_queue：底层 vector + make_heap/push_heap/pop_heap
std::priority_queue<int> pq;                // 最大堆（默认）
std::priority_queue<int, std::vector<int>, std::greater<int>> min_pq;  // 最小堆

pq.push(3);
pq.push(1);
pq.push(4);
pq.top();   // 4（最大值）
pq.pop();   // 移除4
pq.top();   // 3
```

### 7. 容器选择指南

```
你的需求是什么？
│
├─ 需要随机访问？
│   ├─ 主要在尾部操作？ → vector（首选）
│   ├─ 两端都需要操作？ → deque
│   └─ 中间频繁插入？ → 考虑 list（但先测 vector）
│
├─ 需要快速查找？
│   ├─ 需要有序遍历？ → map/set（红黑树）
│   ├─ 不需要有序？ → unordered_map/set（哈希表）
│   └─ 需要重复键？ → multimap/multiset / unordered_multimap/multiset
│
├─ 需要栈/队列？
│   ├─ 后进先出？ → stack
│   ├─ 先进先出？ → queue
│   └─ 优先级队列？ → priority_queue
│
└─ 特殊需求？
    ├─ 固定大小？ → array
    ├─ 单向链表（省内存）？ → forward_list
    └─ 位操作？ → bitset / vector<bool>
```

### 8. 性能对比

#### 1. 插入性能

```cpp
#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <unordered_map>
#include <chrono>

void benchmark_insertion() {
    const int N = 100000;

    // vector 尾部插入
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        std::vector<int> v;
        v.reserve(N);
        for (int i = 0; i < N; ++i) v.push_back(i);
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "vector push_back: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()
                  << " us" << std::endl;
    }

    // list 尾部插入
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        std::list<int> l;
        for (int i = 0; i < N; ++i) l.push_back(i);
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "list push_back: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()
                  << " us" << std::endl;
    }

    // map 插入
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        std::map<int, int> m;
        for (int i = 0; i < N; ++i) m[i] = i;
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "map insert: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()
                  << " us" << std::endl;
    }

    // unordered_map 插入
    {
        auto t1 = std::chrono::high_resolution_clock::now();
        std::unordered_map<int, int> um;
        um.reserve(N);
        for (int i = 0; i < N; ++i) um[i] = i;
        auto t2 = std::chrono::high_resolution_clock::now();
        std::cout << "unordered_map insert: "
                  << std::chrono::duration_cast<std::chrono::microseconds>(t2-t1).count()
                  << " us" << std::endl;
    }
}
```

#### 2. 典型性能对比（相对值）

| 操作 | vector | deque | list | map | unordered_map |
|------|:---:|:---:|:---:|:---:|:---:|
| 随机访问 | 1x | 1-2x | 50x+ | — | — |
| 尾部插入 | 1x | 1x | 3x | — | — |
| 头部插入 | 50x+ | 1x | 1x | — | — |
| 中间插入 | 50x+ | 50x+ | 1x | — | — |
| 查找 by key | — | — | — | 3-5x | 1x |
| 遍历 | 1x | 1-2x | 3-5x | 3-5x | 3-5x |
| 内存开销 | 最低 | 低 | 高 | 高 | 中 |

### 9. 常见陷阱

#### 1. 陷阱1：vector 中间插入导致迭代器失效

```cpp
std::vector<int> v = {1, 2, 3, 4, 5};
auto it = v.begin() + 2;
v.insert(v.begin(), 0);  // it 失效！所有迭代器失效
// *it;  // 未定义行为

// 正确：插入后重新获取迭代器
v.insert(v.begin(), 0);
it = v.begin() + 3;  // 重新定位
```

#### 2. 陷阱2：遍历时删除元素

```cpp
// 错误：遍历时删除
std::vector<int> v = {1, 2, 3, 4, 5};
for (auto it = v.begin(); it != v.end(); ++it) {
    if (*it == 3) v.erase(it);  // it 失效！++it 未定义行为
}

// 正确：erase-erase 惯用法
for (auto it = v.begin(); it != v.end(); ) {
    if (*it == 3) {
        it = v.erase(it);  // erase 返回下一个有效迭代器
    } else {
        ++it;
    }
}

// C++20 更简洁
// std::erase(v, 3);
```

#### 3. 陷阱3：unordered_map 的最坏情况

```cpp
// 哈希冲突严重时，unordered_map 退化为 O(n)
std::unordered_map<int, int> m;
// 如果哈希函数对所有键返回相同值 → 所有元素在同一个桶 → O(n)

// 防御措施：
// 1. 使用好的哈希函数
// 2. 设置合理的 max_load_factor
// 3. 预留足够空间
m.reserve(expected_size);
m.max_load_factor(0.5);
```

### 10. 完整示例：容器选择实战

```cpp
#include <iostream>
#include <vector>
#include <list>
#include <deque>
#include <map>
#include <unordered_map>
#include <set>
#include <algorithm>
#include <chrono>

// 场景1：日志收集 — 追加为主 → vector
class Logger {
    std::vector<std::string> logs_;
public:
    void log(const std::string& msg) {
        logs_.push_back(msg);  // O(1) 均摊
    }
    const std::string& get(size_t idx) const {
        return logs_[idx];  // O(1) 随机访问
    }
    size_t count() const { return logs_.size(); }
};

// 场景2：任务队列 — 先进先出 → deque
class TaskQueue {
    std::deque<std::string> tasks_;
public:
    void push_front(const std::string& task) { tasks_.push_front(task); }
    void push_back(const std::string& task) { tasks_.push_back(task); }
    std::string pop_front() {
        std::string t = std::move(tasks_.front());
        tasks_.pop_front();
        return t;
    }
};

// 场景3：字典查找 — 快速查找 → unordered_map
class Dictionary {
    std::unordered_map<std::string, std::string> dict_;
public:
    void add(const std::string& word, const std::string& def) {
        dict_[word] = def;  // O(1) 均摊
    }
    std::string lookup(const std::string& word) const {
        auto it = dict_.find(word);  // O(1) 均摊
        return it != dict_.end() ? it->second : "not found";
    }
};

// 场景4：排行榜 — 有序 + 不重复 → set
class Leaderboard {
    std::set<int, std::greater<int>> scores_;
public:
    void add_score(int score) { scores_.insert(score); }
    int top() const { return *scores_.begin(); }
    int rank(int score) const {
        return std::distance(scores_.begin(), scores_.find(score)) + 1;
    }
};

int main() {
    Logger logger;
    logger.log("App started");
    logger.log("User login");
    std::cout << logger.get(0) << std::endl;  // App started

    TaskQueue queue;
    queue.push_back("task1");
    queue.push_back("task2");
    std::cout << queue.pop_front() << std::endl;  // task1

    Dictionary dict;
    dict.add("hello", "a greeting");
    std::cout << dict.lookup("hello") << std::endl;  // a greeting

    Leaderboard lb;
    lb.add_score(100);
    lb.add_score(200);
    lb.add_score(150);
    std::cout << "top: " << lb.top() << std::endl;  // 200
}
```

### 11. 极简总结

**vector=动态数组 | deque=分段数组 | list=链表 | map=红黑树 | unordered_map=哈希表**

| 需求 | 首选容器 | 原因 |
|------|----------|------|
| 随机访问 + 尾部追加 | vector | 连续内存，缓存友好 |
| 两端操作 | deque | 双端 O(1) |
| 频繁中间插入 | list（先测 vector） | O(1) 插入 |
| 有序键值对 | map | 红黑树 O(log n) |
| 快速查找 | unordered_map | 哈希表 O(1) |
| 栈 | stack | LIFO |
| 队列 | queue | FIFO |
| 优先级 | priority_queue | 堆 |

***

### 相关阅读

- [什么是SSO小字符串优化](../02-内存与底层/12-什么是SSO小字符串优化.md)
- [什么是EBO空基类优化](../02-内存与底层/16-什么是EBO空基类优化.md)
- [emplace-back与push-back](./01-emplace-back与push-back.md)

***