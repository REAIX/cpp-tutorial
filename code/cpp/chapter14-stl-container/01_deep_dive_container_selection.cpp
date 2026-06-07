/** @file 01_deep_dive_container_selection.cpp
 *  @brief 容器选择指南：何时用哪个、性能对比、内存布局、小缓冲优化
 *  @description 对应文档: 14-STL容器 | 举一反三：掌握容器选择的决策框架
 */

#include <iostream>
#include <vector>
#include <deque>
#include <list>
#include <forward_list>
#include <array>
#include <map>
#include <unordered_map>
#include <set>
#include <unordered_set>
#include <string>
#include <chrono>

void demo_container_selection_guide() {
    std::cout << "=== 容器选择决策树 ===\n";

    std::cout << "1. 需要键值对?\n";
    std::cout << "   是 => 需要有序?\n";
    std::cout << "         是 => map / multimap\n";
    std::cout << "         否 => unordered_map / unordered_multimap\n";
    std::cout << "   否 => 继续...\n\n";

    std::cout << "2. 需要有序/唯一元素?\n";
    std::cout << "   是 => set / multiset (有序)\n";
    std::cout << "        unordered_set (无序, 更快)\n";
    std::cout << "   否 => 继续...\n\n";

    std::cout << "3. 大小已知且固定?\n";
    std::cout << "   是 => array\n";
    std::cout << "   否 => 继续...\n\n";

    std::cout << "4. 主要在尾部操作?\n";
    std::cout << "   是 => vector\n";
    std::cout << "   否 => 继续...\n\n";

    std::cout << "5. 需要头部操作?\n";
    std::cout << "   是 => deque\n";
    std::cout << "   否 => list / forward_list\n\n";

    std::cout << "默认选择: vector (除非有明确理由选其他)\n";

    std::cout << "\n";
}

void demo_performance_comparison() {
    std::cout << "=== 性能对比 ===\n";

    const int N = 100000;

    auto test_push_back = [&](auto& container, const std::string& name) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; ++i) container.push_back(i);
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };

    auto test_iteration = [&](auto& container, const std::string& name) {
        volatile long long sum = 0;  // volatile 防止优化消除, 生产级基准测试建议用 Google Benchmark
        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& x : container) sum += x;
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };

    std::vector<int> vec;
    std::deque<int> deq;
    std::list<int> lst;

    std::cout << "push_back " << N << " 个元素:\n";
    std::cout << "  vector: " << test_push_back(vec, "vector") << " us\n";
    std::cout << "  deque:  " << test_push_back(deq, "deque") << " us\n";
    std::cout << "  list:   " << test_push_back(lst, "list") << " us\n\n";

    std::cout << "遍历 " << N << " 个元素:\n";
    std::cout << "  vector: " << test_iteration(vec, "vector") << " us\n";
    std::cout << "  deque:  " << test_iteration(deq, "deque") << " us\n";
    std::cout << "  list:   " << test_iteration(lst, "list") << " us\n";

    std::cout << "\n性能排序 (一般情况):\n";
    std::cout << "  随机访问: array ≈ vector > deque >> list\n";
    std::cout << "  尾部插入: vector ≈ deque > list\n";
    std::cout << "  头部插入: deque >> list >> vector\n";
    std::cout << "  中间插入: list > deque ≈ vector\n";
    std::cout << "  遍历: vector > deque >> list\n";
    std::cout << "  查找: unordered_* >> 有序容器 >> 顺序容器\n";

    std::cout << "\n";
}

void demo_memory_layout() {
    std::cout << "=== 内存布局 ===\n";

    std::cout << "vector: 连续内存\n";
    std::cout << "  [elem0][elem1][elem2]...[elemN]\n";
    std::cout << "  缓存友好, 随机访问快\n\n";

    std::cout << "deque: 分段连续\n";
    std::cout << "  [map] -> [block0][block1][block2]...\n";
    std::cout << "  每块内部连续, 块间不连续\n\n";

    std::cout << "list: 链表节点\n";
    std::cout << "  [prev|data|next] -> [prev|data|next] -> ...\n";
    std::cout << "  每个节点独立分配, 缓存不友好\n\n";

    std::cout << "unordered_map: 哈希表\n";
    std::cout << "  [桶0] -> [kv] -> [kv]\n";
    std::cout << "  [桶1] -> [kv]\n";
    std::cout << "  [桶2] -> [kv] -> [kv] -> [kv]\n";
    std::cout << "  桶数组 + 链表\n\n";

    std::cout << "内存开销对比:\n";
    std::cout << "  vector: 仅数据 (可能有多余容量)\n";
    std::cout << "  list: 每个元素额外 2 个指针\n";
    std::cout << "  forward_list: 每个元素额外 1 个指针\n";
    std::cout << "  unordered_map: 桶数组 + 每个元素额外指针\n";
    std::cout << "  map/set: 每个节点额外 3 个指针 (红黑树)\n";

    std::cout << "\n";
}

void demo_small_buffer_optimization() {
    std::cout << "=== 小缓冲优化 (SBO) ===\n";

    std::cout << "SBO: 小对象存储在对象内部, 不分配堆内存\n\n";

    std::cout << "std::string 的 SBO:\n";
    {
        std::string short_str = "Hi";
        std::string long_str = "这是一个很长的字符串, 超过了小缓冲区的大小";

        std::cout << "  短字符串大小: " << sizeof(short_str) << " 字节\n";
        std::cout << "  长字符串大小: " << sizeof(long_str) << " 字节\n";
        std::cout << "  两者大小相同, 但短字符串使用内部缓冲\n\n";
    }

    std::cout << "SBO 对性能的影响:\n";
    std::cout << "  1. 小对象: 无堆分配, 更快\n";
    std::cout << "  2. 拷贝: 小对象直接拷贝缓冲区\n";
    std::cout << "  3. 移动: 小对象需要拷贝 (不能只换指针)\n";
    std::cout << "  4. std::function 也有 SBO\n";

    std::cout << "\n";
}

void demo_container_size_comparison() {
    std::cout << "=== 容器对象大小对比 ===\n";

    std::cout << "空容器的大小 (字节):\n";
    std::cout << "  vector<int>:       " << sizeof(std::vector<int>) << "\n";
    std::cout << "  deque<int>:        " << sizeof(std::deque<int>) << "\n";
    std::cout << "  list<int>:         " << sizeof(std::list<int>) << "\n";
    std::cout << "  forward_list<int>: " << sizeof(std::forward_list<int>) << "\n";
    std::cout << "  array<int,10>:     " << sizeof(std::array<int, 10>) << "\n";
    std::cout << "  map<int,int>:      " << sizeof(std::map<int, int>) << "\n";
    std::cout << "  unordered_map<int,int>: " << sizeof(std::unordered_map<int, int>) << "\n";
    std::cout << "  set<int>:          " << sizeof(std::set<int>) << "\n";
    std::cout << "  string:            " << sizeof(std::string) << "\n";

    std::cout << "\n注意: 容器对象大小 ≠ 容器占用总内存\n";
    std::cout << "  vector 对象只有3个指针, 但数据在堆上\n";
    std::cout << "  array 对象包含所有数据 (在栈上)\n";

    std::cout << "\n";
}

void demo_vector_reserve_strategy() {
    std::cout << "=== vector 预分配策略 ===\n";

    std::vector<int> v1;
    size_t last_cap = 0;
    std::cout << "不预分配的扩容过程:\n";
    for (int i = 0; i < 30; ++i) {
        v1.push_back(i);
        if (v1.capacity() != last_cap) {
            std::cout << "  size=" << v1.size() << " => capacity=" << v1.capacity() << "\n";
            last_cap = v1.capacity();
        }
    }

    std::vector<int> v2;
    v2.reserve(100);
    std::cout << "\n预分配 reserve(100):\n";
    std::cout << "  初始 capacity=" << v2.capacity() << "\n";
    for (int i = 0; i < 100; ++i) v2.push_back(i);
    std::cout << "  插入100个后 capacity=" << v2.capacity() << " (无扩容)\n";

    std::cout << "\n预分配的好处:\n";
    std::cout << "  1. 避免多次扩容 (每次扩容都要拷贝)\n";
    std::cout << "  2. 避免迭代器失效\n";
    std::cout << "  3. 减少内存碎片\n";
    std::cout << "  4. 如果知道大小, 一定要 reserve\n";

    std::cout << "\n";
}

int main() {
    demo_container_selection_guide();
    demo_performance_comparison();
    demo_memory_layout();
    demo_small_buffer_optimization();
    demo_container_size_comparison();
    demo_vector_reserve_strategy();

    return 0;
}
