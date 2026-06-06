/** @file 01_deep_dive_iterator_categories.cpp
 *  @brief 迭代器分类：输入/输出/前向/双向/随机访问迭代器、迭代器特征、反向迭代器、插入迭代器
 *  @description 对应文档: 15-STL算法与迭代器 | 举一反三：理解迭代器的分类与机制
 */

#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <deque>
#include <set>
#include <map>
#include <iterator>
#include <algorithm>
#include <string>

void demo_iterator_categories() {
    std::cout << "=== 迭代器分类 ===\n";

    std::cout << "1. 输入迭代器 (Input Iterator):\n";
    std::cout << "   - 只读, 单次遍历\n";
    std::cout << "   - 操作: ++, *, ==, !=\n";
    std::cout << "   - 示例: istream_iterator\n\n";

    std::cout << "2. 输出迭代器 (Output Iterator):\n";
    std::cout << "   - 只写, 单次遍历\n";
    std::cout << "   - 操作: ++, * (赋值)\n";
    std::cout << "   - 示例: ostream_iterator, back_insert_iterator\n\n";

    std::cout << "3. 前向迭代器 (Forward Iterator):\n";
    std::cout << "   - 读写, 多次遍历\n";
    std::cout << "   - 操作: ++, *, ==, !=\n";
    std::cout << "   - 示例: forward_list::iterator\n\n";

    std::cout << "4. 双向迭代器 (Bidirectional Iterator):\n";
    std::cout << "   - 读写, 可前进可后退\n";
    std::cout << "   - 操作: ++, --, *, ==, !=\n";
    std::cout << "   - 示例: list::iterator, map::iterator\n\n";

    std::cout << "5. 随机访问迭代器 (Random Access Iterator):\n";
    std::cout << "   - 读写, 随机访问\n";
    std::cout << "   - 操作: ++, --, +, -, [], <, >\n";
    std::cout << "   - 示例: vector::iterator, deque::iterator\n\n";

    std::cout << "层次关系:\n";
    std::cout << "  输入 <- 前向 <- 双向 <- 随机访问\n";
    std::cout << "  输出 <- 前向\n";

    std::cout << "\n";
}

template<typename Iterator>
void print_iterator_category() {
    using category = typename std::iterator_traits<Iterator>::iterator_category;
    std::string name;
    if constexpr (std::is_same_v<category, std::input_iterator_tag>) name = "输入迭代器";
    else if constexpr (std::is_same_v<category, std::output_iterator_tag>) name = "输出迭代器";
    else if constexpr (std::is_same_v<category, std::forward_iterator_tag>) name = "前向迭代器";
    else if constexpr (std::is_same_v<category, std::bidirectional_iterator_tag>) name = "双向迭代器";
    else if constexpr (std::is_same_v<category, std::random_access_iterator_tag>) name = "随机访问迭代器";
    else name = "未知";
    std::cout << name;
}

void demo_iterator_traits() {
    std::cout << "=== iterator_traits ===\n";

    std::cout << "各容器的迭代器类型:\n";
    std::cout << "  vector<int>: "; print_iterator_category<std::vector<int>::iterator>(); std::cout << "\n";
    std::cout << "  deque<int>:  "; print_iterator_category<std::deque<int>::iterator>(); std::cout << "\n";
    std::cout << "  list<int>:   "; print_iterator_category<std::list<int>::iterator>(); std::cout << "\n";
    std::cout << "  forward_list<int>: "; print_iterator_category<std::forward_list<int>::iterator>(); std::cout << "\n";
    std::cout << "  set<int>:    "; print_iterator_category<std::set<int>::iterator>(); std::cout << "\n";
    std::cout << "  map<int,int>: "; print_iterator_category<std::map<int,int>::iterator>(); std::cout << "\n";
    std::cout << "  int*:        "; print_iterator_category<int*>(); std::cout << "\n";

    std::cout << "\niterator_traits 的成员:\n";
    std::cout << "  difference_type - 两个迭代器距离的类型\n";
    std::cout << "  value_type      - 迭代器指向的值类型\n";
    std::cout << "  pointer         - 指向值类型的指针\n";
    std::cout << "  reference       - 值类型的引用\n";
    std::cout << "  iterator_category - 迭代器分类标签\n";

    std::cout << "\n";
}

void demo_reverse_iterator() {
    std::cout << "=== 反向迭代器 ===\n";

    std::vector<int> v = {1, 2, 3, 4, 5};

    std::cout << "正向遍历: ";
    for (auto it = v.begin(); it != v.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    std::cout << "反向遍历: ";
    for (auto it = v.rbegin(); it != v.rend(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    std::sort(v.rbegin(), v.rend());
    std::cout << "sort(rbegin, rend) 降序: ";
    for (auto x : v) std::cout << x << " ";
    std::cout << "\n";

    auto rit = v.rbegin();
    auto fit = rit.base();
    std::cout << "rbegin().base() 指向: " << *fit << "\n";

    std::cout << "\n反向迭代器的要点:\n";
    std::cout << "  rbegin() 指向最后一个元素\n";
    std::cout << "  rend() 指向第一个元素之前\n";
    std::cout << "  base() 转换为正向迭代器 (偏移1位)\n";

    std::cout << "\n";
}

void demo_insert_iterators() {
    std::cout << "=== 插入迭代器 ===\n";

    std::vector<int> src = {1, 2, 3, 4, 5};
    std::vector<int> dst;

    std::cout << "--- back_insert_iterator ---\n";
    std::copy(src.begin(), src.end(), std::back_inserter(dst));
    std::cout << "  back_inserter: ";
    for (auto x : dst) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "\n--- front_insert_iterator ---\n";
    std::deque<int> dq;
    std::copy(src.begin(), src.end(), std::front_inserter(dq));
    std::cout << "  front_inserter: ";
    for (auto x : dq) std::cout << x << " ";
    std::cout << " (逆序插入)\n";

    std::cout << "\n--- insert_iterator ---\n";
    std::vector<int> dst2 = {10, 20, 30};
    std::copy(src.begin(), src.end(), std::inserter(dst2, dst2.begin() + 1));
    std::cout << "  inserter(位置1): ";
    for (auto x : dst2) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "\n插入迭代器的原理:\n";
    std::cout << "  back_inserter: 调用 push_back()\n";
    std::cout << "  front_inserter: 调用 push_front()\n";
    std::cout << "  inserter: 调用 insert()\n";
    std::cout << "  赋值操作 *it = value 转换为对应的插入操作\n";

    std::cout << "\n";
}

void demo_stream_iterators() {
    std::cout << "=== 流迭代器 ===\n";

    std::cout << "--- ostream_iterator ---\n";
    std::vector<int> v = {1, 2, 3, 4, 5};
    std::cout << "  输出: ";
    std::copy(v.begin(), v.end(), std::ostream_iterator<int>(std::cout, ", "));
    std::cout << "\n";

    std::cout << "\n--- istream_iterator ---\n";
    std::cout << "  用法: istream_iterator<int>(cin) 从cin读取\n";
    std::cout << "  istream_iterator<int>() 默认构造表示结束\n";
    std::cout << "  示例: vector<int> v(istream_iterator<int>(cin), istream_iterator<int>());\n";

    std::cout << "\n--- 结合使用 ---\n";
    std::vector<int> data = {10, 20, 30, 40, 50};
    std::cout << "  格式化输出: ";
    std::transform(data.begin(), data.end(), std::ostream_iterator<std::string>(std::cout, "\n  "),
        [](int x) { return "[" + std::to_string(x) + "]"; });
    std::cout << "\n";

    std::cout << "\n";
}

void demo_iterator_algorithms() {
    std::cout << "=== 迭代器辅助函数 ===\n";

    std::list<int> lst = {1, 2, 3, 4, 5};

    auto it = lst.begin();
    std::advance(it, 3);
    std::cout << "advance(begin, 3): *it = " << *it << "\n";

    auto dist = std::distance(lst.begin(), it);
    std::cout << "distance(begin, it): " << dist << "\n";

    auto it2 = std::next(lst.begin(), 2);
    std::cout << "next(begin, 2): *it2 = " << *it2 << "\n";

    auto it3 = std::prev(lst.end(), 2);
    std::cout << "prev(end, 2): *it3 = " << *it3 << "\n";

    std::cout << "\nadvance vs next/prev:\n";
    std::cout << "  advance: 原地移动, 返回 void\n";
    std::cout << "  next/prev: 返回新迭代器, 不修改原迭代器\n";
    std::cout << "  对随机访问迭代器: O(1)\n";
    std::cout << "  对其他迭代器: O(n)\n";

    std::cout << "\n";
}

int main() {
    demo_iterator_categories();
    demo_iterator_traits();
    demo_reverse_iterator();
    demo_insert_iterators();
    demo_stream_iterators();
    demo_iterator_algorithms();

    return 0;
}
