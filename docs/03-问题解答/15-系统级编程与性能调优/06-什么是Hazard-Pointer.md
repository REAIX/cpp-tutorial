# 什么是Hazard Pointer
> 📖 相关章节：[内存管理](../../01-C语言/09-内存管理.md)、[高级内存管理](../../09-系统级编程与性能调优/01-高级内存管理.md)、[CPU缓存优化](../../09-系统级编程与性能调优/02-CPU缓存优化.md)

> 一句话概括：Hazard Pointer（危险指针）是无锁编程中解决内存回收问题的方案——读者在访问共享数据前先"挂号"（登记指针），回收者看到有人挂号就不敢回收那块内存，等没人挂了再安全回收。

***

### 1. Hazard Pointer 的原理

#### 1.1 无锁编程的内存回收难题

无锁数据结构中，一个线程从链表摘下节点后，不能立即 `delete`，因为其他线程可能还在读这个节点：

```
时间线：
1. 线程A 读取 head = node1
2. 线程B 将 node1 从链表摘除，delete node1
3. 线程A 继续访问 node1 → 野指针！段错误！
```

**核心矛盾**：节点被摘除后，何时才能安全释放？

#### 1.2 Hazard Pointer 的思路

```
1. 每个线程有一个"危险指针"（Hazard Pointer），指向自己正在访问的节点
2. 回收者把要释放的节点放入"待回收列表"
3. 回收前扫描所有线程的危险指针
4. 如果没有任何危险指针指向该节点 → 安全释放
5. 如果有 → 保留，下次再检查
```

**类比**：
- 危险指针 = 医院挂号系统
- 读者 = 病人，看病前先挂号
- 回收者 = 清洁工，看到有挂号就不进那间诊室打扫
- 没人挂号了 → 清洁工安全打扫

#### 1.3 数据结构

```
┌─────────────────────────────────────────────────┐
│              全局 Hazard Pointer 表               │
│                                                  │
│  线程0: hp[0] = &node3  ← 线程0 正在访问 node3   │
│  线程1: hp[1] = nullptr ← 线程1 没有在访问       │
│  线程2: hp[2] = &node7  ← 线程2 正在访问 node7   │
│  线程3: hp[3] = nullptr                         │
│  ...                                             │
└─────────────────────────────────────────────────┘

待回收列表：[node1, node5, node9, ...]

回收检查：
  node1 → 不在任何 hp 中 → 安全释放 ✅
  node5 → 在 hp[2] 中 → 不释放，保留 ❌
  node9 → 不在任何 hp 中 → 安全释放 ✅
```

***

### 2. Hazard Pointer 的实现

#### 2.1 基础实现

```cpp
#include <atomic>
#include <vector>
#include <array>
#include <cstdio>

// 最大线程数
constexpr int MAX_THREADS = 128;

// 每个线程的 Hazard Pointer 数量
constexpr int HP_PER_THREAD = 2;

// 全局 Hazard Pointer 表
struct HazardPointer {
    std::atomic<void*> pointer{nullptr};
    std::atomic<bool> active{false};
};

HazardPointer g_hazard_pointers[MAX_THREADS * HP_PER_THREAD];

// 获取当前线程的 Hazard Pointer 索引
thread_local int tl_hp_indices[HP_PER_THREAD] = {-1, -1};
thread_local bool tl_hp_initialized = false;

// 初始化当前线程的 Hazard Pointer
void hp_init_thread() {
    if (tl_hp_initialized) return;

    for (int i = 0; i < HP_PER_THREAD; ++i) {
        for (int j = 0; j < MAX_THREADS * HP_PER_THREAD; ++j) {
            bool expected = false;
            if (g_hazard_pointers[j].active.compare_exchange_strong(
                    expected, true, std::memory_order_acquire)) {
                tl_hp_indices[i] = j;
                break;
            }
        }
    }
    tl_hp_initialized = true;
}

// 设置 Hazard Pointer（保护一个节点）
void hp_set(int index, void* ptr) {
    g_hazard_pointers[tl_hp_indices[index]].pointer.store(
        ptr, std::memory_order_release);
}

// 清除 Hazard Pointer
void hp_clear(int index) {
    g_hazard_pointers[tl_hp_indices[index]].pointer.store(
        nullptr, std::memory_order_release);
}

// 清除当前线程所有 Hazard Pointer
void hp_clear_all() {
    for (int i = 0; i < HP_PER_THREAD; ++i) {
        hp_clear(i);
    }
}
```

#### 2.2 待回收列表与安全回收

```cpp
#include <functional>
#include <cassert>

// 待回收节点
struct RetiredNode {
    void* pointer;
    std::function<void(void*)> destructor;
    RetiredNode* next;
};

// 每个线程的待回收列表
thread_local RetiredNode* tl_retired_list = nullptr;
thread_local int tl_retired_count = 0;

// 回收阈值：待回收数量超过此值时尝试回收
constexpr int RETIRE_THRESHOLD = MAX_THREADS * HP_PER_THREAD * 2;

// 将节点加入待回收列表
template<typename T>
void hp_retire(T* ptr) {
    auto* node = new RetiredNode{
        ptr,
        [](void* p) { delete static_cast<T*>(p); },
        tl_retired_list
    };
    tl_retired_list = node;
    tl_retired_count++;

    // 超过阈值，尝试回收
    if (tl_retired_count >= RETIRE_THRESHOLD) {
        hp_scan();
    }
}

// 扫描并回收安全的节点
void hp_scan() {
    // 第1步：收集所有活跃的 Hazard Pointer
    std::vector<void*> hazard_ptrs;
    hazard_ptrs.reserve(MAX_THREADS * HP_PER_THREAD);

    for (int i = 0; i < MAX_THREADS * HP_PER_THREAD; ++i) {
        void* ptr = g_hazard_pointers[i].pointer.load(std::memory_order_acquire);
        if (ptr != nullptr) {
            hazard_ptrs.push_back(ptr);
        }
    }

    // 第2步：对待回收列表排序（加速查找）
    std::sort(hazard_ptrs.begin(), hazard_ptrs.end());

    // 第3步：遍历待回收列表，检查是否安全
    RetiredNode* current = tl_retired_list;
    tl_retired_list = nullptr;
    tl_retired_count = 0;

    while (current) {
        RetiredNode* next = current->next;

        // 检查是否被任何 Hazard Pointer 保护
        bool is_protected = std::binary_search(
            hazard_ptrs.begin(), hazard_ptrs.end(), current->pointer);

        if (is_protected) {
            // 仍被保护，放回待回收列表
            current->next = tl_retired_list;
            tl_retired_list = current;
            tl_retired_count++;
        } else {
            // 安全释放
            current->destructor(current->pointer);
            delete current;
        }

        current = next;
    }
}
```

#### 2.3 完整的无锁栈（使用 Hazard Pointer）

```cpp
#include <atomic>
#include <cstdio>

template<typename T>
class LockFreeStackHP {
private:
    struct Node {
        T data;
        Node* next;
        Node(T val) : data(std::move(val)), next(nullptr) {}
    };

    std::atomic<Node*> head{nullptr};

public:
    void push(T value) {
        Node* new_node = new Node(std::move(value));
        new_node->next = head.load(std::memory_order_relaxed);

        while (!head.compare_exchange_weak(
                   new_node->next, new_node,
                   std::memory_order_release,
                   std::memory_order_relaxed)) {
            // CAS 失败，重试（new_node->next 已自动更新）
        }
    }

    bool pop(T& result) {
        hp_init_thread();

        while (true) {
            Node* old_head = head.load(std::memory_order_acquire);

            if (!old_head) {
                return false;  // 栈空
            }

            // 设置 Hazard Pointer 保护 old_head
            hp_set(0, old_head);

            // 再次确认 head 没变（防止设置 HP 前节点已被释放）
            if (head.load(std::memory_order_acquire) != old_head) {
                continue;  // head 变了，重试
            }

            // 尝试弹出
            if (head.compare_exchange_strong(
                    old_head, old_head->next,
                    std::memory_order_acquire,
                    std::memory_order_relaxed)) {
                result = std::move(old_head->data);

                // 清除 Hazard Pointer
                hp_clear(0);

                // 将 old_head 加入待回收列表
                hp_retire(old_head);

                return true;
            }

            // CAS 失败，清除 HP 重试
            hp_clear(0);
        }
    }
};

int main() {
    LockFreeStackHP<int> stack;

    // 多线程安全使用
    stack.push(1);
    stack.push(2);
    stack.push(3);

    int val;
    while (stack.pop(val)) {
        printf("弹出: %d\n", val);
    }

    return 0;
}
```

***

### 3. Hazard Pointer 与 RCU/Epoch 的对比

#### 3.1 三种方案对比

| 维度 | Hazard Pointer | RCU | Epoch-based |
|------|---------------|-----|-------------|
| 读开销 | 1-2次原子写（设置HP） | 几乎零开销 | 1次原子写（进入epoch） |
| 写开销 | 扫描所有HP | 等待宽限期 | 扫描epoch计数器 |
| 延迟 | 立即回收（无保护时） | 宽限期后回收 | epoch 结束后回收 |
| 内存占用 | 较少（精确保护） | 较多（宽限期内所有节点） | 中等 |
| 实现复杂度 | 中等 | 高（需要内核支持或复杂用户态实现） | 中等 |
| 适用场景 | 精确保护少量节点 | 读多写少，读侧零开销 | 通用无锁回收 |

#### 3.2 读侧开销对比

```
Hazard Pointer 读操作：
  hp_set(0, ptr);     // 原子写（~10ns）
  // 读取数据
  hp_clear(0);        // 原子写（~10ns）
  总开销：~20ns

RCU 读操作：
  rcu_read_lock();    // 仅修改线程局部变量（~1ns）
  // 读取数据
  rcu_read_unlock();  // 仅修改线程局部变量（~1ns）
  总开销：~2ns

Epoch 读操作：
  enter_epoch();      // 原子操作（~5ns）
  // 读取数据
  leave_epoch();      // 原子操作（~5ns）
  总开销：~10ns
```

#### 3.3 选择建议

| 场景 | 推荐 | 原因 |
|------|------|------|
| 读操作极频繁，要求零开销 | RCU | 读侧几乎无开销 |
| 需要精确保护特定节点 | Hazard Pointer | 只保护正在访问的节点 |
| 通用无锁数据结构 | Epoch | 实现相对简单，开销适中 |
| 内存敏感（不能大量缓存旧数据） | Hazard Pointer | 精确回收，内存占用最少 |
| Linux 内核模块 | RCU | 内核原生支持 |

***

### 4. Hazard Pointer 的优化

#### 4.1 批量回收

```cpp
// 不是每次 retire 都扫描，而是积累到阈值再扫描
// 减少扫描频率，提高效率

void hp_retire_batch(void* ptr, std::function<void(void*)> destructor) {
    auto* node = new RetiredNode{ptr, destructor, tl_retired_list};
    tl_retired_list = node;
    tl_retired_count++;

    // 阈值通常设为 HP总数的 2 倍
    if (tl_retired_count >= RETIRE_THRESHOLD) {
        hp_scan();
    }
}
```

#### 4.2 减少扫描开销

```cpp
// 优化：使用哈希表代替排序+二分查找
#include <unordered_set>

void hp_scan_optimized() {
    // 收集所有活跃的 Hazard Pointer 到哈希表
    std::unordered_set<void*> hazard_set;
    for (int i = 0; i < MAX_THREADS * HP_PER_THREAD; ++i) {
        void* ptr = g_hazard_pointers[i].pointer.load(std::memory_order_acquire);
        if (ptr) hazard_set.insert(ptr);
    }

    // 遍历待回收列表
    RetiredNode* current = tl_retired_list;
    tl_retired_list = nullptr;
    tl_retired_count = 0;

    while (current) {
        RetiredNode* next = current->next;
        if (hazard_set.count(current->pointer)) {
            current->next = tl_retired_list;
            tl_retired_list = current;
            tl_retired_count++;
        } else {
            current->destructor(current->pointer);
            delete current;
        }
        current = next;
    }
}
```

#### 4.3 多 Hazard Pointer 的场景

```cpp
// 链表遍历需要 2 个 Hazard Pointer
template<typename T>
bool lock_free_find(typename LockFreeList<T>::Node* head, const T& target) {
    hp_init_thread();

    Node* prev = head;
    hp_set(0, prev);

    Node* curr = prev->next.load(std::memory_order_acquire);
    hp_set(1, curr);

    while (curr) {
        // 确认 prev->next 仍然指向 curr
        if (prev->next.load(std::memory_order_acquire) != curr) {
            // 链表被修改，重新遍历
            prev = head;
            hp_set(0, prev);
            curr = prev->next.load(std::memory_order_acquire);
            hp_set(1, curr);
            continue;
        }

        if (curr->data == target) {
            hp_clear_all();
            return true;  // 找到了
        }

        // 前进
        hp_set(0, curr);     // prev = curr
        prev = curr;
        curr = curr->next.load(std::memory_order_acquire);
        hp_set(1, curr);     // 保护新的 curr
    }

    hp_clear_all();
    return false;  // 没找到
}
```

***

### 5. Hazard Pointer 的适用场景

#### 5.1 适合使用 Hazard Pointer 的场景

| 场景 | 原因 |
|------|------|
| 无锁链表 | 遍历时需要保护当前和下一个节点 |
| 无锁栈 | pop 时需要保护栈顶节点 |
| 无锁队列 | 出队时需要保护头节点 |
| 内存敏感场景 | 精确保护，不会大量缓存旧数据 |
| 读操作不太频繁 | HP 的读侧开销可接受 |

#### 5.2 不适合使用 Hazard Pointer 的场景

| 场景 | 原因 | 替代方案 |
|------|------|---------|
| 读操作极频繁 | HP 的读侧开销（~20ns）可能不可接受 | RCU |
| 需要保护大量节点 | HP 数量有限 | Epoch |
| 单线程场景 | 不需要无锁回收 | 直接 delete |
| 短生命周期对象 | HP 的登记/清除开销不值得 | 引用计数 |

***

### 6. 常见误区

| 误区 | 事实 |
|------|------|
| "Hazard Pointer 能防止 ABA 问题" | HP 只解决内存安全，ABA 需要版本号解决 |
| "Hazard Pointer 读侧零开销" | 读侧需要原子写（~10ns），比 RCU 慢 |
| "HP 数量越多越好" | HP 越多，扫描越慢，通常每个线程 2 个足够 |
| "HP 扫描很慢" | 批量扫描 + 哈希优化，均摊开销很小 |
| "HP 只能用于链表" | 任何需要延迟回收的无锁结构都可以用 |

***

### 7. 总结

| 要点 | 说明 |
|------|------|
| Hazard Pointer | 读者登记指针，回收者检查后安全释放 |
| 读侧开销 | 1-2 次原子写（~20ns），比 RCU 慢但比锁快 |
| 写侧开销 | 扫描所有 HP，批量回收均摊后较小 |
| 内存占用 | 精确保护，内存占用最少 |
| 适用场景 | 无锁链表/栈/队列，内存敏感场景 |
| 对比 RCU | HP 读侧有开销但内存更少，RCU 读侧零开销但内存更多 |
| 对比 Epoch | HP 更精确，Epoch 更简单 |