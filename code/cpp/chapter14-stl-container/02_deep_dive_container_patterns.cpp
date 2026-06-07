/** @file 02_deep_dive_container_patterns.cpp
 *  @brief 容器模式：适配器、flat_map概念、稳定操作、迭代器失效规则
 *  @description 对应文档: 14-STL容器 | 举一反三：掌握容器的高级用法和陷阱
 */

#include <iostream>
#include <stack>
#include <queue>
#include <vector>
#include <list>
#include <string>
#include <map>
#include <algorithm>
#include <functional>

void demo_stack() {
    std::cout << "=== std::stack ===\n";

    std::stack<int> s;
    s.push(10);
    s.push(20);
    s.push(30);

    std::cout << "push(10,20,30) 后:\n";
    std::cout << "  top() = " << s.top() << "\n";
    std::cout << "  size() = " << s.size() << "\n";

    s.pop();
    std::cout << "pop() 后 top() = " << s.top() << "\n";

    std::cout << "\n遍历 stack (需要先拷贝):\n";
    std::stack<int> copy = s;
    while (!copy.empty()) {
        std::cout << "  " << copy.top() << "\n";
        copy.pop();
    }

    std::cout << "\nstack 特点:\n";
    std::cout << "  - LIFO (后进先出)\n";
    std::cout << "  - 默认基于 deque\n";
    std::cout << "  - 只有 top, push, pop 操作\n";

    std::cout << "\n";
}

void demo_queue() {
    std::cout << "=== std::queue ===\n";

    std::queue<std::string> q;
    q.push("第一个");
    q.push("第二个");
    q.push("第三个");

    std::cout << "push 3个元素后:\n";
    std::cout << "  front() = " << q.front() << "\n";
    std::cout << "  back() = " << q.back() << "\n";

    q.pop();
    std::cout << "pop() 后 front() = " << q.front() << "\n";

    std::cout << "\nqueue 特点:\n";
    std::cout << "  - FIFO (先进先出)\n";
    std::cout << "  - 默认基于 deque\n";
    std::cout << "  - 只有 front, back, push, pop 操作\n";

    std::cout << "\n";
}

void demo_priority_queue() {
    std::cout << "=== std::priority_queue ===\n";

    std::priority_queue<int> pq;
    pq.push(30);
    pq.push(10);
    pq.push(50);
    pq.push(20);
    pq.push(40);

    std::cout << "默认 (大顶堆) 出队顺序: ";
    while (!pq.empty()) {
        std::cout << pq.top() << " ";
        pq.pop();
    }
    std::cout << "\n\n";

    std::priority_queue<int, std::vector<int>, std::greater<int>> min_pq;
    min_pq.push(30);
    min_pq.push(10);
    min_pq.push(50);
    min_pq.push(20);
    min_pq.push(40);

    std::cout << "小顶堆出队顺序: ";
    while (!min_pq.empty()) {
        std::cout << min_pq.top() << " ";
        min_pq.pop();
    }
    std::cout << "\n\n";

    struct Task {
        std::string name;
        int priority;
        bool operator<(const Task& other) const {
            return priority < other.priority;
        }
    };

    std::priority_queue<Task> tasks;
    tasks.push({"低优先级", 1});
    tasks.push({"高优先级", 10});
    tasks.push({"中优先级", 5});

    std::cout << "任务调度:\n";
    while (!tasks.empty()) {
        std::cout << "  " << tasks.top().name << " (优先级=" << tasks.top().priority << ")\n";
        tasks.pop();
    }

    std::cout << "\npriority_queue 特点:\n";
    std::cout << "  - 堆实现 (默认大顶堆)\n";
    std::cout << "  - 基于 vector\n";
    std::cout << "  - 插入/删除 O(log n)\n";
    std::cout << "  - top() O(1)\n";

    std::cout << "\n";
}

void demo_flat_map_concept() {
    std::cout << "=== flat_map 概念 ===\n";

    std::cout << "flat_map: 用排序 vector 实现的 map\n";
    std::cout << "C++23 引入, 这里手动实现概念:\n\n";

    class FlatMap {
    public:
        int& operator[](const std::string& key) {
            auto it = std::lower_bound(keys_.begin(), keys_.end(), key);
            if (it == keys_.end() || *it != key) {
                auto dist = it - keys_.begin();
                keys_.insert(it, key);
                values_.insert(values_.begin() + dist, 0);
                return values_[dist];  // insert 后 it 失效, 必须用 dist
            }
            return values_[it - keys_.begin()];
        }

        int* find(const std::string& key) {
            auto it = std::lower_bound(keys_.begin(), keys_.end(), key);
            if (it != keys_.end() && *it == key) {
                return &values_[it - keys_.begin()];
            }
            return nullptr;
        }

        void print() const {
            for (size_t i = 0; i < keys_.size(); ++i) {
                std::cout << "  " << keys_[i] << ": " << values_[i] << "\n";
            }
        }
    private:
        std::vector<std::string> keys_;
        std::vector<int> values_;
    };

    FlatMap fm;
    fm["Charlie"] = 92;
    fm["Alice"] = 95;
    fm["Bob"] = 87;

    std::cout << "FlatMap 内容:\n";
    fm.print();

    auto* val = fm.find("Bob");
    if (val) std::cout << "find(\"Bob\"): " << *val << "\n";

    std::cout << "\nflat_map 的优缺点:\n";
    std::cout << "  优点: 缓存友好, 小数据集更快, 内存连续\n";
    std::cout << "  缺点: 插入/删除 O(n), 不适合频繁修改\n";
    std::cout << "  适合: 数据量小, 读多写少\n";

    std::cout << "\n";
}

void demo_stable_container_operations() {
    std::cout << "=== 稳定容器操作 ===\n";

    std::cout << "稳定: 操作不改变元素的相对顺序\n\n";

    std::cout << "stable_sort vs sort:\n";
    {
        struct Item {
            std::string name;
            int priority;
        };

        std::vector<Item> items = {
            {"A", 2}, {"B", 1}, {"C", 2}, {"D", 1}, {"E", 2}
        };

        auto items2 = items;
        std::stable_sort(items2.begin(), items2.end(),
            [](const Item& a, const Item& b) { return a.priority < b.priority; });

        std::cout << "  stable_sort 按优先级排序:\n";
        for (const auto& item : items2) {
            std::cout << "    " << item.name << "(" << item.priority << ")\n";
        }
        std::cout << "  同优先级的元素保持原始顺序\n";
    }

    std::cout << "\nstable_partition vs partition:\n";
    {
        std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8};

        auto v2 = v;
        std::stable_partition(v2.begin(), v2.end(), [](int x) { return x % 2 == 0; });
        std::cout << "  stable_partition (偶数在前): ";
        for (int x : v2) std::cout << x << " ";
        std::cout << "\n  各组内保持原始顺序\n";
    }

    std::cout << "\n";
}

void demo_iterator_invalidation() {
    std::cout << "=== 迭代器失效规则 ===\n";

    std::cout << "--- vector 迭代器失效 ---\n";
    {
        std::vector<int> v = {1, 2, 3, 4, 5};
        auto it = v.begin() + 2;

        v.push_back(6);
        std::cout << "  push_back 可能导致 it 失效 (如果扩容)\n";

        v.reserve(100);
        it = v.begin() + 2;
        v.push_back(7);
        std::cout << "  reserve后 push_back: it 仍有效 (未扩容)\n";
        std::cout << "  *it = " << *it << "\n";

        v.insert(v.begin() + 1, 100);
        std::cout << "  insert 后 it 失效 (位置移动)\n";

        v.erase(v.begin());
        std::cout << "  erase 后 it 及之后的迭代器失效\n";
    }

    std::cout << "\n--- list 迭代器不失效 ---\n";
    {
        std::list<int> lst = {1, 2, 3, 4, 5};
        auto it = std::next(lst.begin(), 2);

        lst.push_front(0);
        lst.push_back(6);
        lst.insert(lst.begin(), -1);

        std::cout << "  插入操作后 *it = " << *it << " (仍有效)\n";

        lst.erase(lst.begin());
        std::cout << "  删除其他位置后 *it = " << *it << " (仍有效)\n";
    }

    std::cout << "\n--- 关联容器迭代器不失效 ---\n";
    {
        std::map<int, std::string> m = {{1, "a"}, {2, "b"}, {3, "c"}};
        auto it = m.find(2);

        m[4] = "d";
        m.erase(1);

        std::cout << "  插入/删除其他元素后 it->second = " << it->second << " (仍有效)\n";
    }

    std::cout << "\n迭代器失效总结:\n";
    std::cout << "  vector: 插入/删除可能导致全部失效\n";
    std::cout << "  deque: 插入/删除导致全部失效\n";
    std::cout << "  list: 只有被删除的元素失效\n";
    std::cout << "  map/set: 只有被删除的元素失效\n";
    std::cout << "  unordered_*: 插入可能导致全部失效 (rehash)\n";

    std::cout << "\n安全删除模式:\n";
    std::cout << "  for (auto it = v.begin(); it != v.end(); ) {\n";
    std::cout << "    if (condition) it = v.erase(it);\n";
    std::cout << "    else ++it;\n";
    std::cout << "  }\n";

    std::cout << "\n";
}

int main() {
    demo_stack();
    demo_queue();
    demo_priority_queue();
    demo_flat_map_concept();
    demo_stable_container_operations();
    demo_iterator_invalidation();

    return 0;
}
