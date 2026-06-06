/** @file 01_example_sequence_container.cpp
 *  @brief 顺序容器：vector、deque、list、forward_list、array
 *  @description 对应文档: 14-STL容器
 */

#include <iostream>
#include <vector>
#include <deque>
#include <list>
#include <forward_list>
#include <array>
#include <string>
#include <chrono>
#include <algorithm>

void demo_vector() {
    std::cout << "=== std::vector ===\n";

    std::vector<int> v1;
    std::vector<int> v2 = {1, 2, 3, 4, 5};
    std::vector<int> v3(10, 0);
    std::vector<int> v4(v2.begin(), v2.begin() + 3);

    std::cout << "初始化:\n";
    std::cout << "  v2 = {1,2,3,4,5}\n";
    std::cout << "  v3(10, 0): 大小=" << v3.size() << "\n";
    std::cout << "  v4(迭代器范围): ";
    for (auto x : v4) std::cout << x << " ";
    std::cout << "\n\n";

    v2.push_back(6);
    v2.emplace_back(7);
    std::cout << "push_back/emplace_back: ";
    for (auto x : v2) std::cout << x << " ";
    std::cout << "\n";

    v2.insert(v2.begin() + 2, 100);
    std::cout << "insert(位置2, 100): ";
    for (auto x : v2) std::cout << x << " ";
    std::cout << "\n";

    v2.erase(v2.begin() + 2);
    std::cout << "erase(位置2): ";
    for (auto x : v2) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "容量信息: size=" << v2.size() << ", capacity=" << v2.capacity() << "\n";
    v2.reserve(100);
    std::cout << "reserve(100)后: capacity=" << v2.capacity() << "\n";
    v2.shrink_to_fit();
    std::cout << "shrink_to_fit后: capacity=" << v2.capacity() << "\n";

    std::cout << "\nvector 特点:\n";
    std::cout << "  - 连续内存, 随机访问 O(1)\n";
    std::cout << "  - 尾部插入/删除 O(1)\n";
    std::cout << "  - 中间插入/删除 O(n)\n";
    std::cout << "  - 扩容时可能重新分配\n";

    std::cout << "\n";
}

void demo_deque() {
    std::cout << "=== std::deque ===\n";

    std::deque<int> dq = {2, 3, 4};

    dq.push_front(1);
    dq.push_back(5);
    std::cout << "push_front(1), push_back(5): ";
    for (auto x : dq) std::cout << x << " ";
    std::cout << "\n";

    dq.pop_front();
    dq.pop_back();
    std::cout << "pop_front(), pop_back(): ";
    for (auto x : dq) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "dq[0] = " << dq[0] << ", dq.at(1) = " << dq.at(1) << "\n";

    std::cout << "\ndeque 特点:\n";
    std::cout << "  - 双端队列, 两端操作 O(1)\n";
    std::cout << "  - 随机访问 O(1) (但比 vector 稍慢)\n";
    std::cout << "  - 中间插入/删除 O(n)\n";
    std::cout << "  - 内存不连续 (分段数组)\n";
    std::cout << "  - 适合: 需要头部操作的场景\n";

    std::cout << "\n";
}

void demo_list() {
    std::cout << "=== std::list ===\n";

    std::list<int> lst = {3, 1, 4, 1, 5};

    lst.push_front(0);
    lst.push_back(9);
    std::cout << "push_front(0), push_back(9): ";
    for (auto x : lst) std::cout << x << " ";
    std::cout << "\n";

    auto it = std::find(lst.begin(), lst.end(), 4);
    if (it != lst.end()) {
        lst.insert(it, 100);
        std::cout << "在4前插入100: ";
        for (auto x : lst) std::cout << x << " ";
        std::cout << "\n";
    }

    lst.sort();
    std::cout << "sort(): ";
    for (auto x : lst) std::cout << x << " ";
    std::cout << "\n";

    lst.unique();
    std::cout << "unique(): ";
    for (auto x : lst) std::cout << x << " ";
    std::cout << "\n";

    std::list<int> other = {50, 60, 70};
    lst.splice(lst.end(), other);
    std::cout << "splice(): ";
    for (auto x : lst) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "\nlist 特点:\n";
    std::cout << "  - 双向链表, 任意位置插入/删除 O(1)\n";
    std::cout << "  - 不支持随机访问\n";
    std::cout << "  - 插入/删除不会使迭代器失效\n";
    std::cout << "  - 有 sort, unique, splice 等特有算法\n";

    std::cout << "\n";
}

void demo_forward_list() {
    std::cout << "=== std::forward_list ===\n";

    std::forward_list<int> fl = {1, 2, 3, 4, 5};

    fl.push_front(0);
    std::cout << "push_front(0): ";
    for (auto x : fl) std::cout << x << " ";
    std::cout << "\n";

    fl.insert_after(fl.begin(), 100);
    std::cout << "insert_after(begin, 100): ";
    for (auto x : fl) std::cout << x << " ";
    std::cout << "\n";

    fl.erase_after(fl.begin());
    std::cout << "erase_after(begin): ";
    for (auto x : fl) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "\nforward_list 特点:\n";
    std::cout << "  - 单向链表, 比 list 更省内存\n";
    std::cout << "  - 没有 size() 方法\n";
    std::cout << "  - 只能向前遍历\n";
    std::cout << "  - 插入/删除在指定位置之后\n";
    std::cout << "  - 适合: 内存敏感, 只需前向遍历\n";

    std::cout << "\n";
}

void demo_array() {
    std::cout << "=== std::array ===\n";

    std::array<int, 5> arr = {10, 20, 30, 40, 50};

    std::cout << "内容: ";
    for (auto x : arr) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "arr[2] = " << arr[2] << "\n";
    std::cout << "arr.at(3) = " << arr.at(3) << "\n";
    std::cout << "arr.front() = " << arr.front() << "\n";
    std::cout << "arr.back() = " << arr.back() << "\n";
    std::cout << "arr.size() = " << arr.size() << "\n";

    std::sort(arr.begin(), arr.end(), std::greater<int>());
    std::cout << "降序排序: ";
    for (auto x : arr) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "\narray 特点:\n";
    std::cout << "  - 固定大小, 编译期确定\n";
    std::cout << "  - 栈上分配, 无堆开销\n";
    std::cout << "  - 随机访问 O(1)\n";
    std::cout << "  - 零开销抽象 (与C数组性能相同)\n";
    std::cout << "  - 适合: 大小已知的固定集合\n";

    std::cout << "\n";
}

int main() {
    demo_vector();
    demo_deque();
    demo_list();
    demo_forward_list();
    demo_array();

    return 0;
}
