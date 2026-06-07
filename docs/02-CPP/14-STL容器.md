# STL标准库

> C++标准模板库核心组件

---

> **STL is the library that changed C++ forever.** — someone
> （STL是改变C++的库。）

> **不要重复造轮子，STL已经为你准备好了一切。**
> （Don't reinvent the wheel. STL has everything ready for you.）

---

> **🎯 工欲善其事，必先利其器。**
> 
> （学会使用STL，让编程更高效。）

> 💡 **通俗理解 - 什么是STL？**

想象你要搬家：
- 没有工具：你需要自己做箱子、租车、打包...
- 有工具：直接用现成的纸箱、推车、搬家公司的服务

**STL就是C++的"搬家工具箱"！**

- **容器** 就像各种"箱子"：  - `vector` 就像"可变大小的行李箱"，可以往里一直塞东西
  - `map` 就像"字典"，可以按名字找东西
  - `set` 就像"不重复的集合"，保证东西不重复

- **算法** 就像"搬家服务"：  - 排序就像"把东西按大小整理好"
  - 查找就像"找某个东西在哪"
  - 遍历就像"检查每个箱子"

- **迭代器** 就像"箱子的编号"：  - 告诉你现在在哪个箱子
  - 让你可以一个个访问箱子里的东西

**为什么要用STL？**
- 就像你不用自己造汽车，直接买现成的
- 别人已经帮你写好了各种常用功能
- 你只需要会用就行，省时省力！

> 🔬 **抽象理解 - STL的设计思想**：
> - **泛型编程**：STL的核心是"算法与数据类型分离"，同一个算法可以适用于不同数据类型
> - **容器**：是一种"数据结构"，用于存储和组织数据
> - **迭代器**：是一种"抽象的指针"，提供统一的访问接口，屏蔽底层实现细节
> - **算法**：是一种"操作"，对数据进行排序、查找、变换等操作
> - **STL的本质**：是"数据结构和算法的标准库"，提供可复用的组件

---

## 前置知识
- [智能指针与内存管理](08-智能指针与内存管理.md)
## 后续内容
- [正则表达式](16-正则表达式.md)
## 目录

- [1. 容器概览](#1-容器概览)
- [2. vector动态数组](#2-vector动态数组)
- [3. deque双端队列](#3-deque双端队列)
- [4. list双向链表](#4-list双向链表)
- [5. 容器适配器](#5-容器适配器)
- [6. 现代C++工具类型](#6-现代c工具类型)
- [7. string字符串](#7-string字符串)
- [8. map与unordered_map](#8-map与unordered_map)
- [9. set与unordered_set](#9-set与unordered_set)

---

## 1. 容器概览

### 1. 概念与定义

**STL（Standard Template Library）**：C++标准模板库，包含容器、算法、迭代器等组件。STL可以提高开发效率，减少代码冗余。

**容器（container）**：STL中用于存储数据的数据结构。容器可以分为序列容器、关联容器、容器适配器、无序关联容器等。

**序列容器（sequence container）**：按顺序存储元素的容器。例如`vector`、`list`、`deque`、`array`、`forward_list`等。

**关联容器（associative container）**：按键存储元素的容器。例如`map`、`set`、`multimap`、`multiset`等。

**容器适配器（container adapter）**：基于其他容器实现的容器。例如`stack`、`queue`、`priority_queue`等。

**无序关联容器（unordered associative container）**：C++11引入的新特性，基于哈希表实现的容器。例如`unordered_map`、`unordered_set`、`unordered_multimap`、`unordered_multiset`等。

**迭代器（iterator）**：STL中用于遍历容器的对象。迭代器可以分为输入迭代器、输出迭代器、前向迭代器、双向迭代器、随机访问迭代器等。

**算法（algorithm）**：STL中用于操作容器的函数。例如`sort`、`find`、`for_each`、`transform`等。

### 2. 容器分类

```
┌─────────────────────────────────────────────────────────────┐
│                        STL 容器                              │
├─────────────────┬─────────────────┬─────────────────────────┤
│    序列容器      │    关联容器      │      容器适配器         │
├─────────────────┼─────────────────┼─────────────────────────┤
│ vector          │ map             │ stack                   │
│ list            │ set             │ queue                   │
│ deque           │ multimap        │ priority_queue          │
│ array           │ multiset        │                         │
│ forward_list    │                 │                         │
├─────────────────┼─────────────────┴─────────────────────────┤
│  无序关联容器    │                                                 │
├─────────────────┤                                                 │
│ unordered_map   │                                                 │
│ unordered_set   │                                                 │
│ unordered_multimap│                                               │
│ unordered_multiset│                                              │
└─────────────────┴─────────────────────────────────────────────┘
```

### 3. 容器选择指南

| 需求 | 推荐容器 |
|-----|---------|
| 随机访问 | `vector` |
| 中间插入/删除 | `list` |
| 两端操作 | `deque` |
| 键值对查找 | `unordered_map` |
| 有序键值对 | `map` |
| 去重 | `unordered_set` |
| 有序去重 | `set` |
| LIFO | `stack` |
| FIFO | `queue` |

---

## 2. vector动态数组

### 1. vector内部结构

```
vector<int> vec = {1, 2, 3, 4, 5};

堆内存：
┌──────┬──────┬──────┬──────┬──────┬──────┬──────┐
│  1   │  2   │  3   │  4   │  5   │  ?   │  ?   │
└──────┴──────┴──────┴──────┴──────┴──────┴──────┘
  ↑                               ↑              ↑
  │                               │              │
 begin()                         end()      capacity end

vector对象内部：
┌─────────────────────────────────────┐
│ start ──────────────────→ [1的地址]  │
│ finish ─────────────────→ [5的地址]  │
│ end_of_storage ─────────→ [7的地址]  │
└─────────────────────────────────────┘

size() = finish - start = 5
capacity() = end_of_storage - start = 7
```

**扩容机制：**
- 当 `size() == capacity()` 时，再添加元素会触发扩容
- 新容量通常是旧容量的 2 倍
- 扩容需要重新分配内存并移动所有元素

### 2. 基本操作

```cpp
#include <vector>

std::vector<int> vec;

// 添加元素
vec.push_back(1);
vec.push_back(2);
vec.push_back(3);

// 访问元素
int first = vec[0];           // 不检查边界
int second = vec.at(1);       // 检查边界，越界抛异常
int last = vec.back();        // 最后元素
int* ptr = vec.data();        // 原始指针

// 大小操作
size_t size = vec.size();
bool empty = vec.empty();
vec.resize(10);               // 调整大小
vec.reserve(100);             // 预留容量

// 删除元素
vec.pop_back();               // 删除最后元素
vec.erase(vec.begin() + 1);   // 删除指定位置
vec.clear();                  // 清空
```

### 3. 初始化方式

```cpp
std::vector<int> v1;                    // 空
std::vector<int> v2(10);                // 10个默认值
std::vector<int> v3(10, 5);             // 10个5
std::vector<int> v4 = {1, 2, 3, 4, 5};  // 初始化列表
std::vector<int> v5(v4);                // 拷贝
std::vector<int> v6(std::move(v4));     // 移动
```

### 4. 遍历方式

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5};

// 1. 下标遍历
for (size_t i = 0; i < vec.size(); ++i) {
    std::cout << vec[i] << " ";
}

// 2. 迭代器遍历
for (auto it = vec.begin(); it != vec.end(); ++it) {
    std::cout << *it << " ";
}

// 3. 范围for
for (int x : vec) {
    std::cout << x << " ";
}

// 4. 引用修改
for (int& x : vec) {
    x *= 2;
}
```

---

## 3. deque双端队列

### 1. deque的特点

**deque（Double-Ended Queue）** 是一种双端队列容器，支持在两端高效地插入和删除元素，同时支持随机访问。

```
deque<int> dq = {1, 2, 3, 4, 5};

分段连续内存结构：
┌─────────┐    ┌─────────┐    ┌─────────┐
│ 1 │ 2 │ 3│    │ 4 │ 5 │ ?│    │ ? │ ? │ ?│  ← 各段固定大小
└────┬────┘    └────┬────┘    └────┬────┘
     │              │              │
     └──────────────┼──────────────┘
                    │
            ┌───────┴───────┐
            │  中控数组(map) │  ← 存储各段地址
            └───────────────┘

特性：
- 两端插入/删除：O(1)
- 随机访问：O(1)（略慢于vector，需要一次间接寻址）
- 中间插入/删除：O(n)
- 内存：分段连续，不需要整体重新分配
```

### 2. deque与vector对比

| 特性 | deque | vector |
|-----|-------|--------|
| 头部插入/删除 | O(1) | O(n) |
| 尾部插入/删除 | O(1) | O(1) 均摊 |
| 中间插入/删除 | O(n) | O(n) |
| 随机访问 | O(1) | O(1) |
| 内存布局 | 分段连续 | 连续 |
| 扩容代价 | 低（新增段即可） | 高（整体搬移） |
| 迭代器失效 | 插入/删除两端外均失效 | 插入可能全部失效 |
| `data()` | 无 | 有 |

### 3. 常用操作

```cpp
#include <deque>

std::deque<int> dq;

// 两端插入
dq.push_back(2);       // 尾部插入：{2}
dq.push_front(1);      // 头部插入：{1, 2}
dq.push_back(3);       // 尾部插入：{1, 2, 3}

// 两端删除
dq.pop_front();        // 删除头部：{2, 3}
dq.pop_back();         // 删除尾部：{2}

// 随机访问
int val = dq[0];       // 不检查边界
int val2 = dq.at(0);   // 检查边界，越界抛异常

// 其他操作与vector类似
dq.size();
dq.empty();
dq.clear();
dq.insert(dq.begin() + 1, 10);
dq.erase(dq.begin());
dq.resize(10);
```

### 4. 适用场景

| 场景 | 说明 |
|-----|------|
| **滑动窗口** | 需要频繁在头部删除、尾部插入 |
| **工作窃取队列** | 多线程中一端插入、另一端取出 |
| **BFS广度优先搜索** | 需要头部弹出、尾部插入 |
| **不确定数据增长方向** | 两端都可能增长时优于vector |

### 5. 代码示例：滑动窗口最大值

```cpp
#include <deque>
#include <vector>
#include <iostream>

std::vector<int> maxSlidingWindow(const std::vector<int>& nums, int k) {
    std::deque<int> dq;
    std::vector<int> result;

    for (int i = 0; i < static_cast<int>(nums.size()); ++i) {
        // 移除超出窗口范围的元素下标
        while (!dq.empty() && dq.front() <= i - k) {
            dq.pop_front();
        }
        // 移除比当前元素小的元素（它们不可能成为最大值）
        while (!dq.empty() && nums[dq.back()] < nums[i]) {
            dq.pop_back();
        }
        dq.push_back(i);
        // 窗口形成后，队头就是最大值
        if (i >= k - 1) {
            result.push_back(nums[dq.front()]);
        }
    }
    return result;
}

int main() {
    std::vector<int> nums = {1, 3, -1, -3, 5, 3, 6, 7};
    int k = 3;
    auto res = maxSlidingWindow(nums, k);
    for (int x : res) {
        std::cout << x << " ";
    }
    std::cout << std::endl;
    return 0;
}
```

---

## 4. list双向链表

### 1. list的特点

**list** 是一种双向链表容器，支持在任意位置O(1)时间插入和删除元素，但不支持随机访问。

```
std::list<int> lst = {1, 2, 3};

节点结构：
┌──────────────────────────────────────────────────────┐
│                                                      │
│  ┌──────┐    ┌──────┐    ┌──────┐                   │
│  │ prev │←───│ prev │←───│ prev │                   │
│  │  1   │    │  2   │    │  3   │                   │
│  │ next │───→│ next │───→│ next │                   │
│  └──────┘    └──────┘    └──────┘                   │
│      ↑                       ↑                      │
│      └───────────────────────┘（哨兵节点连接首尾）     │
└──────────────────────────────────────────────────────┘

特性：
- 任意位置插入/删除：O(1)（已知位置时）
- 随机访问：不支持（只能遍历，O(n)）
- 内存：每个节点独立分配，不连续
- 迭代器：双向迭代器（支持++和--，不支持+和-）
- 插入/删除不会使其他迭代器失效
```

### 2. list与vector/deque对比

| 特性 | list | vector | deque |
|-----|------|--------|-------|
| 随机访问 | 不支持 O(n) | O(1) | O(1) |
| 头部插入/删除 | O(1) | O(n) | O(1) |
| 尾部插入/删除 | O(1) | O(1) 均摊 | O(1) |
| 中间插入/删除 | O(1)* | O(n) | O(n) |
| 内存布局 | 不连续 | 连续 | 分段连续 |
| 迭代器失效 | 插入不失效，删除仅失效被删元素 | 插入可能全部失效 | 两端外均失效 |
| 额外内存开销 | 每节点2个指针 | 无 | 中控数组 |
| 缓存友好性 | 差 | 好 | 较好 |

> *O(1)指已知插入位置时，查找位置仍需O(n)

### 3. 特有操作

```cpp
#include <list>

std::list<int> lst = {3, 1, 4, 1, 5};

// splice：将另一个list的元素转移到当前位置（O(1)，无拷贝）
std::list<int> other = {10, 20};
lst.splice(lst.begin(), other);  // other的元素转移到lst头部

// sort：list自带的排序（不能使用std::sort，因为需要随机访问迭代器）
lst.sort();                       // 升序
lst.sort(std::greater<int>());    // 降序

// merge：合并两个有序list
std::list<int> a = {1, 3, 5};
std::list<int> b = {2, 4, 6};
a.merge(b);  // a = {1,2,3,4,5,6}，b变为空

// reverse：反转
lst.reverse();

// unique：去重（相邻重复元素只保留一个，需先排序）
lst.sort();
lst.unique();

// remove/remove_if：按值或条件删除
lst.remove(1);  // 删除所有值为1的元素
lst.remove_if([](int x) { return x < 0; });  // 删除所有负数
```

### 4. forward_list（C++11单向链表）

```cpp
#include <forward_list>

// forward_list：单向链表，比list更节省内存（每节点只需1个指针）
std::forward_list<int> fl = {1, 2, 3};

// 只能向前遍历
for (auto it = fl.begin(); it != fl.end(); ++it) {
    std::cout << *it << " ";
}

// 没有size()方法（为了节省开销）
// 插入使用insert_after（在指定位置之后插入）
fl.push_front(0);                  // 头部插入
fl.insert_after(fl.begin(), 10);   // 在第一个元素之后插入10

// 删除使用erase_after
fl.erase_after(fl.begin());        // 删除第一个元素之后的元素

// forward_list vs list
// | 特性         | forward_list | list       |
// |-------------|-------------|------------|
// | 遍历方向     | 单向         | 双向        |
// | 每节点指针数 | 1            | 2          |
// | 内存占用     | 更小         | 较大        |
// | size()      | 无           | 有          |
// | 适用场景     | 只需前向遍历  | 需要双向遍历  |
```

### 5. 适用场景

| 场景 | 说明 |
|-----|------|
| **频繁中间插入/删除** | 已知位置时O(1)，不会导致迭代器大面积失效 |
| **不需要随机访问** | 只需顺序遍历时 |
| **迭代器稳定性要求高** | 插入/删除不影响其他迭代器 |
| **大对象频繁移动** | 链表只需修改指针，无需搬移元素 |

### 6. 代码示例：LRU缓存核心逻辑

```cpp
#include <list>
#include <unordered_map>
#include <string>
#include <iostream>

template<typename K, typename V>
class LRUCache {
    using ListIt = typename std::list<std::pair<K, V>>::iterator;
    int capacity;
    std::list<std::pair<K, V>> cacheList;
    std::unordered_map<K, ListIt> cacheMap;

public:
    LRUCache(int cap) : capacity(cap) {}

    V* get(const K& key) {
        auto it = cacheMap.find(key);
        if (it == cacheMap.end()) {
            return nullptr;
        }
        // 将访问的元素移到链表头部（最近使用）
        cacheList.splice(cacheList.begin(), cacheList, it->second);
        return &(it->second->second);
    }

    void put(const K& key, const V& value) {
        auto it = cacheMap.find(key);
        if (it != cacheMap.end()) {
            it->second->second = value;
            cacheList.splice(cacheList.begin(), cacheList, it->second);
            return;
        }
        if (static_cast<int>(cacheMap.size()) >= capacity) {
            // 淘汰最久未使用（链表尾部）
            auto last = cacheList.back();
            cacheMap.erase(last.first);
            cacheList.pop_back();
        }
        cacheList.push_front({key, value});
        cacheMap[key] = cacheList.begin();
    }
};

int main() {
    LRUCache<int, std::string> cache(3);
    cache.put(1, "one");
    cache.put(2, "two");
    cache.put(3, "three");
    cache.get(1);           // 访问1，使其变为最近使用
    cache.put(4, "four");   // 淘汰2（最久未使用）
    std::cout << (cache.get(2) ? *cache.get(2) : "null") << std::endl;
    return 0;
}
```

---

## 5. 容器适配器

> 💡 **通俗理解 - 什么是容器适配器？**
>
> 容器适配器就像给现有容器"穿上一层外衣"，限制其接口，使其表现出特定的行为。
> - **stack** 就像"叠盘子"：只能从顶部放/取（后进先出）
> - **queue** 就像"排队"：从后面进，前面出（先进先出）
> - **priority_queue** 就像"VIP通道"：优先级高的先出

### 1. stack栈（LIFO）

```
stack（栈）：后进先出（Last In, First Out）

操作示意：
push(1)  →  | 1 |        push(2)  →  | 2 |  ← top
            |___|                     | 1 |
                                      |___|

pop()    →  | 1 |  ← top    top() → 返回栈顶元素
            |___|                     size() → 元素数量

底层默认使用deque，也可使用vector或list
```

```cpp
#include <stack>

std::stack<int> stk;

// 入栈
stk.push(10);
stk.push(20);
stk.push(30);  // 栈：|30|20|10|，30在栈顶

// 访问栈顶
int top = stk.top();  // 30

// 出栈
stk.pop();  // 移除30，栈：|20|10|

// 状态查询
bool empty = stk.empty();
size_t size = stk.size();

// 使用vector作为底层容器
std::stack<int, std::vector<int>> stk_vec;
```

**代码示例：括号匹配**

```cpp
#include <stack>
#include <string>
#include <iostream>

bool isValidParentheses(const std::string& s) {
    std::stack<char> stk;
    for (char c : s) {
        if (c == '(' || c == '[' || c == '{') {
            stk.push(c);
        } else {
            if (stk.empty()) return false;
            char top = stk.top();
            stk.pop();
            if ((c == ')' && top != '(') ||
                (c == ']' && top != '[') ||
                (c == '}' && top != '{')) {
                return false;
            }
        }
    }
    return stk.empty();
}

int main() {
    std::cout << std::boolalpha;
    std::cout << isValidParentheses("()[]{}") << std::endl;  // true
    std::cout << isValidParentheses("([)]") << std::endl;    // false
    std::cout << isValidParentheses("{[]}") << std::endl;    // true
    return 0;
}
```

**代码示例：表达式求值**

```cpp
#include <stack>
#include <string>
#include <iostream>

int evaluatePostfix(const std::string& expr) {
    std::stack<int> stk;
    for (size_t i = 0; i < expr.size(); ++i) {
        char c = expr[i];
        if (std::isdigit(c)) {
            stk.push(c - '0');
        } else if (c == '+' || c == '-' || c == '*' || c == '/') {
            int b = stk.top(); stk.pop();
            int a = stk.top(); stk.pop();
            switch (c) {
                case '+': stk.push(a + b); break;
                case '-': stk.push(a - b); break;
                case '*': stk.push(a * b); break;
                case '/': stk.push(a / b); break;
            }
        }
    }
    return stk.top();
}

int main() {
    std::cout << evaluatePostfix("23+5*") << std::endl;  // (2+3)*5 = 25
    return 0;
}
```

### 2. queue队列（FIFO）

```
queue（队列）：先进先出（First In, First Out）

操作示意：
push(1)  →  | 1 |        push(2)  →  | 1 | 2 |
             ^^^^                     ^front  ^back

pop()    →  | 2 |        front() → 返回队头元素
             ^^^^        back()  → 返回队尾元素

底层默认使用deque，也可使用list
```

```cpp
#include <queue>

std::queue<int> q;

// 入队
q.push(10);
q.push(20);
q.push(30);  // 队列：10(前) → 20 → 30(后)

// 访问
int front = q.front();  // 10
int back = q.back();    // 30

// 出队
q.pop();  // 移除10，队列：20 → 30

// 状态查询
bool empty = q.empty();
size_t size = q.size();
```

**代码示例：BFS广度优先搜索**

```cpp
#include <queue>
#include <vector>
#include <iostream>

void bfs(const std::vector<std::vector<int>>& graph, int start) {
    std::vector<bool> visited(graph.size(), false);
    std::queue<int> q;

    visited[start] = true;
    q.push(start);

    while (!q.empty()) {
        int node = q.front();
        q.pop();
        std::cout << node << " ";

        for (int neighbor : graph[node]) {
            if (!visited[neighbor]) {
                visited[neighbor] = true;
                q.push(neighbor);
            }
        }
    }
}

int main() {
    // 邻接表表示的图
    std::vector<std::vector<int>> graph = {
        {1, 2},    // 节点0的邻居
        {0, 3},    // 节点1的邻居
        {0, 3},    // 节点2的邻居
        {1, 2}     // 节点3的邻居
    };
    bfs(graph, 0);  // 输出：0 1 2 3
    return 0;
}
```

### 3. priority_queue优先队列（默认最大堆）

```
priority_queue（优先队列）：默认最大堆

操作示意：
push(3)  →  | 3 |     push(1)  →  | 3 |     push(5)  →  | 5 |
             ^^^^                   | 1 |                   | 3 |
                                    ^^^^                   | 1 |
                                                           ^^^^

top()  → 返回最大元素（5）
pop()  → 移除最大元素，堆自动调整

底层使用vector，默认使用std::less（最大堆）
```

```cpp
#include <queue>

// 默认最大堆
std::priority_queue<int> maxHeap;
maxHeap.push(3);
maxHeap.push(1);
maxHeap.push(5);
maxHeap.push(2);

int top = maxHeap.top();  // 5（最大值）
maxHeap.pop();            // 移除5
top = maxHeap.top();      // 2（新的最大值）

// 最小堆：使用std::greater
std::priority_queue<int, std::vector<int>, std::greater<int>> minHeap;
minHeap.push(3);
minHeap.push(1);
minHeap.push(5);
int smallest = minHeap.top();  // 1（最小值）

// 自定义类型
struct Task {
    int priority;
    std::string name;
    bool operator<(const Task& other) const {
        return priority < other.priority;  // 优先级高的先出
    }
};

std::priority_queue<Task> taskQueue;
taskQueue.push({1, "低优先级任务"});
taskQueue.push({3, "高优先级任务"});
taskQueue.push({2, "中优先级任务"});

Task topTask = taskQueue.top();  // 高优先级任务
```

**代码示例：Top-K问题**

```cpp
#include <queue>
#include <vector>
#include <iostream>
#include <functional>

std::vector<int> topKFrequent(const std::vector<int>& nums, int k) {
    // 统计频率
    std::unordered_map<int, int> freq;
    for (int num : nums) {
        freq[num]++;
    }

    // 最小堆，维护前K个高频元素
    using Pair = std::pair<int, int>;
    std::priority_queue<Pair, std::vector<Pair>, std::greater<Pair>> minHeap;

    for (const auto& [num, count] : freq) {
        minHeap.emplace(count, num);
        if (static_cast<int>(minHeap.size()) > k) {
            minHeap.pop();
        }
    }

    std::vector<int> result;
    while (!minHeap.empty()) {
        result.push_back(minHeap.top().second);
        minHeap.pop();
    }
    return result;
}

int main() {
    std::vector<int> nums = {1, 1, 1, 2, 2, 3};
    auto res = topKFrequent(nums, 2);
    for (int x : res) {
        std::cout << x << " ";
    }
    return 0;
}
```

**代码示例：任务调度**

```cpp
#include <queue>
#include <vector>
#include <iostream>
#include <functional>

struct Task {
    int priority;
    std::string name;
    bool operator<(const Task& other) const {
        return priority < other.priority;
    }
};

int main() {
    std::priority_queue<Task> scheduler;

    scheduler.push({2, "发送邮件"});
    scheduler.push({5, "处理支付"});
    scheduler.push({1, "清理日志"});
    scheduler.push({3, "生成报表"});

    // 按优先级从高到低执行
    while (!scheduler.empty()) {
        auto& task = scheduler.top();
        std::cout << "执行: " << task.name
                  << " (优先级: " << task.priority << ")" << std::endl;
        scheduler.pop();
    }
    return 0;
}
```

### 4. 底层容器选择

| 适配器 | 默认底层 | 可选底层 | 选择依据 |
|-------|---------|---------|---------|
| `stack` | `deque` | `vector`, `list` | 只需尾部操作，三者均可 |
| `queue` | `deque` | `list` | 需要头部和尾部操作，vector头部操作O(n) |
| `priority_queue` | `vector` | `deque` | 需要随机访问以维护堆结构，不能用list |

```cpp
// 不同底层容器的声明方式
std::stack<int, std::vector<int>> s1;      // vector底层
std::stack<int, std::deque<int>> s2;       // deque底层（默认）
std::stack<int, std::list<int>> s3;        // list底层

std::queue<int, std::deque<int>> q1;       // deque底层（默认）
std::queue<int, std::list<int>> q2;        // list底层

std::priority_queue<int, std::vector<int>> p1;              // vector底层（默认）
std::priority_queue<int, std::deque<int>> p2;               // deque底层
std::priority_queue<int, std::vector<int>, std::greater<int>> p3;  // 最小堆
```

---

## 6. 现代C++工具类型

> 💡 **通俗理解 - 为什么需要这些工具类型？**
>
> - **optional** 就像"可能有也可能没有的包裹"：签收时可能为空
> - **variant** 就像"多功能插座"：同一个位置可以插不同类型的插头
> - **any** 就像"万能容器"：什么都能装，但取出时需要知道类型
> - **tuple** 就像"打包盒"：把不同类型的东西打包在一起

### 1. std::optional<T>（C++17）

**optional** 表示一个"可能包含值"的对象，用于替代哨兵值和nullptr。

```cpp
#include <optional>
#include <string>
#include <iostream>

// 创建
std::optional<int> o1;                   // 空（不包含值）
std::optional<int> o2 = 42;              // 包含值42
std::optional<int> o3 = std::nullopt;    // 显式空
std::optional<std::string> o4 = "hello"; // 包含字符串

// 访问
if (o2.has_value()) {
    int val = o2.value();       // 获取值，空时抛出std::bad_optional_access
    int val2 = *o2;             // 获取值，空时未定义行为
    int val3 = o2.value_or(0);  // 获取值，空时返回默认值0
}

// 赋值与重置
o1 = 10;               // 赋值
o1 = std::nullopt;     // 置空
o1.reset();            // 置空（另一种方式）
o1.emplace(20);        // 原地构造
```

**optional vs nullptr/哨兵值**

| 方式 | 优点 | 缺点 |
|-----|------|------|
| `optional<T>` | 类型安全、语义明确 | C++17、有额外开销 |
| 返回`-1`/`nullptr` | 兼容性好 | 语义不清、容易忘记检查 |
| 输出参数`bool func(T& out)` | 兼容性好 | 代码啰嗦 |

**代码示例：查找函数返回optional**

```cpp
#include <optional>
#include <vector>
#include <string>
#include <iostream>

std::optional<size_t> findUser(const std::vector<std::string>& users,
                                const std::string& name) {
    for (size_t i = 0; i < users.size(); ++i) {
        if (users[i] == name) {
            return i;
        }
    }
    return std::nullopt;
}

int main() {
    std::vector<std::string> users = {"Alice", "Bob", "Charlie"};

    // 使用optional返回值
    auto result = findUser(users, "Bob");
    if (result.has_value()) {
        std::cout << "找到Bob，索引: " << result.value() << std::endl;
    }

    // 使用value_or提供默认值
    auto notFound = findUser(users, "David");
    size_t index = notFound.value_or(users.size());
    std::cout << "David的索引: " << index << std::endl;

    return 0;
}
```

### 2. std::variant<T...>（C++17）

**variant** 是类型安全的联合体，同一时刻只能持有其中一种类型的值。

```cpp
#include <variant>
#include <string>
#include <iostream>

// 创建
using Value = std::variant<int, double, std::string>;

Value v1 = 42;           // 持有int
Value v2 = 3.14;         // 持有double
Value v3 = "hello"s;     // 持有string

// 访问
int i = std::get<int>(v1);            // 按类型访问，类型错误抛出std::bad_variant_access
double d = std::get<1>(v2);           // 按索引访问
int* pi = std::get_if<int>(&v1);      // 按类型访问，返回指针，类型错误返回nullptr

// 检查当前持有的类型
bool isInt = std::holds_alternative<int>(v1);  // true
bool isStr = std::holds_alternative<std::string>(v1);  // false

// 获取当前类型的索引
size_t idx = v1.index();  // 0（int是第0个类型）

// 使用std::visit访问（类型安全的模式匹配）
struct Printer {
    void operator()(int n) const { std::cout << "整数: " << n << std::endl; }
    void operator()(double d) const { std::cout << "浮点: " << d << std::endl; }
    void operator()(const std::string& s) const { std::cout << "字符串: " << s << std::endl; }
};

std::visit(Printer{}, v1);  // 整数: 42
std::visit(Printer{}, v3);  // 字符串: hello

// 使用泛型lambda（C++20可以简化）
std::visit([](const auto& val) {
    std::cout << val << std::endl;
}, v2);  // 3.14
```

**variant vs union**

| 特性 | variant | union |
|-----|---------|-------|
| 类型安全 | 是 | 否（需手动跟踪类型） |
| 支持复杂类型 | 是（string等） | 有限（非平凡类型需手动管理） |
| 访问方式 | `std::get`/`std::visit` | 直接访问成员 |
| 内存开销 | 额外存储类型索引 | 无额外开销 |
| 标准 | C++17 | C++98 |

**代码示例：表达式求值（多类型）**

```cpp
#include <variant>
#include <string>
#include <iostream>
#include <cmath>

using ExprValue = std::variant<int, double, std::string>;

struct Display {
    void operator()(int n) const { std::cout << "整数: " << n; }
    void operator()(double d) const { std::cout << "浮点: " << d; }
    void operator()(const std::string& s) const { std::cout << "字符串: " << s; }
};

ExprValue evaluate(const ExprValue& a, const ExprValue& b, char op) {
    // 将两个值都转为double进行运算
    auto toDouble = [](const ExprValue& v) -> double {
        if (std::holds_alternative<int>(v)) return static_cast<double>(std::get<int>(v));
        if (std::holds_alternative<double>(v)) return std::get<double>(v);
        return std::stod(std::get<std::string>(v));
    };

    double da = toDouble(a);
    double db = toDouble(b);
    double result = 0;

    switch (op) {
        case '+': result = da + db; break;
        case '-': result = da - db; break;
        case '*': result = da * db; break;
        case '/': result = db != 0 ? da / db : 0; break;
    }

    // 如果结果为整数则返回int，否则返回double
    if (result == std::floor(result)) {
        return static_cast<int>(result);
    }
    return result;
}

int main() {
    ExprValue a = 10;
    ExprValue b = 3.14;
    ExprValue c = "2.5";

    auto r1 = evaluate(a, b, '+');
    std::visit(Display{}, r1);  // 浮点: 13.14

    auto r2 = evaluate(a, a, '+');
    std::visit(Display{}, r2);  // 整数: 20

    return 0;
}
```

### 3. std::any（C++17）

**any** 是一种可以持有任意类型值的类型安全容器。

```cpp
#include <any>
#include <string>
#include <iostream>
#include <typeinfo>

// 创建
std::any a1;                    // 空
std::any a2 = 42;               // 持有int
std::any a3 = 3.14;             // 持有double
std::any a4 = std::string("hi"); // 持有string
std::any a5 = std::vector<int>{1,2,3}; // 持有vector

// 访问（使用any_cast）
int i = std::any_cast<int>(a2);           // 按类型获取，类型错误抛出std::bad_any_cast
int* pi = std::any_cast<int>(&a2);        // 返回指针，类型错误返回nullptr

// 检查
bool hasValue = a2.has_value();            // true
const std::type_info& type = a2.type();    // typeid(int)

// 修改
a2 = std::string("changed");  // 改变类型
a2.reset();                    // 置空
a2.emplace<std::vector<int>>(5, 10);  // 原地构造vector<int>(5, 10)
```

**any vs void***

| 特性 | any | void* |
|-----|-----|-------|
| 类型安全 | 是 | 否 |
| 支持复杂类型 | 是 | 需要手动管理生命周期 |
| 知道存储类型 | 是（`type()`） | 否 |
| 内存开销 | 有（类型信息+小对象优化） | 仅一个指针 |
| 访问方式 | `any_cast` | 强制转换 |

**代码示例：配置系统**

```cpp
#include <any>
#include <string>
#include <unordered_map>
#include <iostream>

class Config {
    std::unordered_map<std::string, std::any> data;

public:
    template<typename T>
    void set(const std::string& key, const T& value) {
        data[key] = value;
    }

    template<typename T>
    T get(const std::string& key, const T& defaultValue = T{}) const {
        auto it = data.find(key);
        if (it == data.end()) return defaultValue;
        try {
            return std::any_cast<T>(it->second);
        } catch (const std::bad_any_cast&) {
            return defaultValue;
        }
    }

    bool has(const std::string& key) const {
        return data.find(key) != data.end();
    }
};

int main() {
    Config cfg;
    cfg.set("width", 1920);
    cfg.set("height", 1080);
    cfg.set("title", std::string("我的应用"));
    cfg.set("fullscreen", true);
    cfg.set("volume", 0.8);

    int width = cfg.get<int>("width");
    std::string title = cfg.get<std::string>("title");
    bool fullscreen = cfg.get<bool>("fullscreen");
    double volume = cfg.get<double>("volume");

    std::cout << "宽度: " << width << std::endl;
    std::cout << "标题: " << title << std::endl;
    std::cout << "全屏: " << std::boolalpha << fullscreen << std::endl;
    std::cout << "音量: " << volume << std::endl;

    return 0;
}
```

### 4. std::tuple（多元组）

**tuple** 是一种可以持有不同类型元素的固定大小集合。

```cpp
#include <tuple>
#include <string>
#include <iostream>

// 创建
auto t1 = std::make_tuple(42, 3.14, "hello");
std::tuple<int, double, std::string> t2(1, 2.0, "world");

// 访问
int n = std::get<0>(t1);           // 按索引访问：42
double d = std::get<1>(t1);        // 按索引访问：3.14
std::string s = std::get<2>(t1);   // 按索引访问："hello"

// C++14支持按类型访问（类型必须唯一）
auto val = std::get<double>(t1);   // 3.14

// 获取元组大小和元素类型
constexpr size_t size = std::tuple_size<decltype(t1)>::value;  // 3
using FirstType = std::tuple_element<0, decltype(t1)>::type;   // int

// 结构化绑定（C++17）
auto [x, y, z] = t1;  // x=42, y=3.14, z="hello"

// std::tie：创建左值引用元组
int a, b;
std::string c;
std::tie(a, b, c) = t1;  // a=42, b=3.14, c="hello"

// 忽略某些值
int first;
std::tie(first, std::ignore, std::ignore) = t1;  // 只取第一个

// 比较和拼接
auto t3 = std::tuple_cat(t1, t2);  // 拼接两个tuple
bool eq = (t1 == t2);               // 逐元素比较
bool lt = (t1 < t2);                // 字典序比较
```

**代码示例：多返回值**

```cpp
#include <tuple>
#include <string>
#include <vector>
#include <iostream>

// 返回多个值：最小值、最大值、平均值
std::tuple<double, double, double> calcStats(const std::vector<double>& data) {
    if (data.empty()) {
        return {0.0, 0.0, 0.0};
    }
    double minVal = data[0];
    double maxVal = data[0];
    double sum = 0.0;
    for (double v : data) {
        if (v < minVal) minVal = v;
        if (v > maxVal) maxVal = v;
        sum += v;
    }
    return {minVal, maxVal, sum / data.size()};
}

// 使用结构化绑定接收多个返回值（C++17）
int main() {
    std::vector<double> data = {1.5, 2.3, 3.7, 0.8, 4.1};

    auto [minVal, maxVal, avg] = calcStats(data);
    std::cout << "最小值: " << minVal << std::endl;
    std::cout << "最大值: " << maxVal << std::endl;
    std::cout << "平均值: " << avg << std::endl;

    // 使用tie接收（C++11兼容）
    double mn, mx, av;
    std::tie(mn, mx, av) = calcStats(data);

    return 0;
}
```

---

## 7. string字符串

### 1. 基本操作

```cpp
#include <string>

std::string s1 = "hello";
std::string s2("world");
std::string s3(5, 'a');  // "aaaaa"

// 拼接
std::string s4 = s1 + " " + s2;
s1 += " world";

// 访问
char c = s1[0];
char c2 = s1.at(0);

// 大小
size_t len = s1.length();
size_t len2 = s1.size();
bool e = s1.empty();

// 子串
std::string sub = s1.substr(0, 5);

// 查找
size_t pos = s1.find("world");
size_t pos2 = s1.rfind("l");

// 替换
s1.replace(0, 5, "hi");

// 插入
s1.insert(0, "prefix");

// 删除
s1.erase(0, 3);
```

### 2. 字符串转换

```cpp
#include <string>

// 数字转字符串
std::string s1 = std::to_string(42);
std::string s2 = std::to_string(3.14);

// 字符串转数字
int i = std::stoi("42");
long l = std::stol("123456");
double d = std::stod("3.14");
float f = std::stof("2.5");
```

### 3. 字符串分割

```cpp
#include <sstream>

std::string str = "hello,world,foo,bar";
std::stringstream ss(str);
std::string token;
std::vector<std::string> tokens;

while (std::getline(ss, token, ',')) {
    tokens.push_back(token);
}
```

---

## 8. map与unordered_map

### 1. map内部结构（红黑树）

```
std::map<int, std::string> 的红黑树结构：

插入数据：{30:"C"}, {10:"A"}, {20:"B"}, {40:"D"}, {35:"E"}

                    30(黑)
                   /      \
              10(红)      40(黑)
                 \        /
               20(黑)  35(红)

节点结构：
┌──────────────────────┐
│ key: 30              │
│ value: "C"           │
│ color: BLACK         │
│ left ────→ 左子节点   │
│ right ───→ 右子节点   │
│ parent ──→ 父节点     │
└──────────────────────┘

特性：
- 自动排序（按键值）
- 查找/插入/删除：O(log n)
- 红黑树保持平衡
- 迭代器按中序遍历（升序）
```

### 2. map（有序映射）

```cpp
#include <map>

std::map<std::string, int> scores;

// 插入
scores["Alice"] = 90;
scores.insert({"Bob", 85});
scores.emplace("Charlie", 88);

// 访问
int alice = scores["Alice"];
int bob = scores.at("Bob");

// 查找
auto it = scores.find("Alice");
if (it != scores.end()) {
    std::cout << it->first << ": " << it->second << std::endl;
}

// 遍历
for (const auto& p : scores) {
    std::cout << p.first << ": " << p.second << std::endl;
}

// 删除
scores.erase("Bob");

// 大小
size_t n = scores.size();
bool e = scores.empty();
```

### 3. unordered_map（无序映射）

```cpp
#include <unordered_map>

std::unordered_map<std::string, int> umap;

// 操作与map相同，但查找更快（平均O(1)）
umap["key"] = 100;

// 自定义键类型需要提供hash函数
struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

namespace std {
    template<>
    struct hash<Point> {
        size_t operator()(const Point& p) const {
            size_t h1 = hash<int>()(p.x);
            size_t h2 = hash<int>()(p.y);
            return h1 ^ (h2 << 1);  // 避免对称点（如{1,2}和{2,1}）产生相同哈希值
        }
    };
}
```

### 4. map vs unordered_map

| 特性 | map | unordered_map |
|-----|-----|---------------|
| 实现 | 红黑树 | 哈希表 |
| 查找复杂度 | O(log n) | 平均O(1) |
| 有序性 | 有序 | 无序 |
| 内存占用 | 较小 | 较大 |

---

## 9. set与unordered_set

### 1. set（有序集合）

```cpp
#include <set>

std::set<int> s;

// 插入
s.insert(3);
s.insert(1);
s.insert(2);
// 自动排序：1, 2, 3

// 查找
bool found = s.count(2);  // 0或1
auto it = s.find(2);

// 删除
s.erase(2);

// 遍历
for (int x : s) {
    std::cout << x << " ";
}

// 范围操作
auto lower = s.lower_bound(2);  // 第一个>=2的元素
auto upper = s.upper_bound(2);  // 第一个>2的元素
```

### 2. multiset（允许重复）

```cpp
#include <set>

std::multiset<int> ms;

ms.insert(1);
ms.insert(1);
ms.insert(2);

// count返回实际数量
size_t n = ms.count(1);  // 2
```

---

**上一章：** [第13章：Lambda与函数对象](13-Lambda与函数对象.md)\
**下一章：** [第15章：STL算法与迭代器](15-STL算法与迭代器.md)

---

### 相关章节

- [STL容器底层实现](../03-问题解答/07-现代CPP标准库/01-STL容器底层实现.md) — vector/list/map等底层结构
- [迭代器失效问题](../03-问题解答/04-CPP核心特性/26-迭代器失效问题.md) — 插入/删除导致迭代器失效
- [emplace-back与push-back](../03-问题解答/07-现代CPP标准库/02-emplace-back与push-back.md) — 原地构造vs拷贝

---
