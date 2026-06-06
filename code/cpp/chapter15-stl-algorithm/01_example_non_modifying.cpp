/** @file 01_example_non_modifying.cpp
 *  @brief 非修改算法：find、count、search、all_of、any_of、none_of、for_each、min/max
 *  @description 对应文档: 15-STL算法与迭代器
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>

void demo_find_algorithms() {
    std::cout << "=== 查找算法 ===\n";

    std::vector<int> v = {5, 3, 8, 1, 9, 3, 7, 4, 6, 2};

    auto it = std::find(v.begin(), v.end(), 9);
    if (it != v.end()) {
        std::cout << "find(9): 位置=" << (it - v.begin()) << "\n";
    }

    auto it2 = std::find_if(v.begin(), v.end(), [](int x) { return x > 7; });
    if (it2 != v.end()) {
        std::cout << "find_if(>7): " << *it2 << " 位置=" << (it2 - v.begin()) << "\n";
    }

    auto it3 = std::find_if_not(v.begin(), v.end(), [](int x) { return x > 0; });
    std::cout << "find_if_not(>0): " << (it3 == v.end() ? "未找到(全部>0)" : std::to_string(*it3)) << "\n";

    std::vector<int> sub = {3, 8, 1};
    auto it4 = std::search(v.begin(), v.end(), sub.begin(), sub.end());
    if (it4 != v.end()) {
        std::cout << "search({3,8,1}): 位置=" << (it4 - v.begin()) << "\n";
    }

    auto it5 = std::find_end(v.begin(), v.end(), sub.begin(), sub.end());
    std::cout << "find_end: 查找子序列最后一次出现\n";

    auto it6 = std::adjacent_find(v.begin(), v.end());
    if (it6 != v.end()) {
        std::cout << "adjacent_find: 位置=" << (it6 - v.begin()) << " 值=" << *it6 << "\n";
    } else {
        std::cout << "adjacent_find: 无相邻重复\n";
    }

    std::cout << "\n";
}

void demo_count_algorithms() {
    std::cout << "=== 计数算法 ===\n";

    std::vector<int> v = {1, 2, 3, 2, 4, 2, 5, 2};

    std::cout << "count(2): " << std::count(v.begin(), v.end(), 2) << "\n";
    std::cout << "count_if(偶数): " << std::count_if(v.begin(), v.end(), [](int x) { return x % 2 == 0; }) << "\n";
    std::cout << "count_if(>3): " << std::count_if(v.begin(), v.end(), [](int x) { return x > 3; }) << "\n";

    std::cout << "\n";
}

void demo_predicate_algorithms() {
    std::cout << "=== 谓词算法 ===\n";

    std::vector<int> v = {2, 4, 6, 8, 10};

    std::cout << "all_of(偶数): " << (std::all_of(v.begin(), v.end(), [](int x) { return x % 2 == 0; }) ? "是" : "否") << "\n";
    std::cout << "any_of(>8): " << (std::any_of(v.begin(), v.end(), [](int x) { return x > 8; }) ? "是" : "否") << "\n";
    std::cout << "none_of(<0): " << (std::none_of(v.begin(), v.end(), [](int x) { return x < 0; }) ? "是" : "否") << "\n";

    std::vector<int> v2 = {1, 2, 3, 4, 5};
    std::cout << "all_of(偶数): " << (std::all_of(v2.begin(), v2.end(), [](int x) { return x % 2 == 0; }) ? "是" : "否") << "\n";

    std::cout << "\n";
}

void demo_for_each() {
    std::cout << "=== for_each ===\n";

    std::vector<int> v = {1, 2, 3, 4, 5};

    std::cout << "基本用法: ";
    std::for_each(v.begin(), v.end(), [](int x) {
        std::cout << x << " ";
    });
    std::cout << "\n";

    int sum = 0;
    std::for_each(v.begin(), v.end(), [&sum](int x) {
        sum += x;
    });
    std::cout << "求和: " << sum << "\n";

    std::cout << "\nfor_each vs 范围 for:\n";
    std::cout << "  for_each: 可以修改元素, 可带状态\n";
    std::cout << "  范围 for: 更简洁, 更常用\n";
    std::cout << "  建议: 优先使用范围 for\n";

    std::cout << "\n";
}

void demo_min_max() {
    std::cout << "=== min/max 算法 ===\n";

    std::cout << "min(3, 7) = " << std::min(3, 7) << "\n";
    std::cout << "max(3, 7) = " << std::max(3, 7) << "\n";

    auto [mn, mx] = std::minmax(7, 3);
    std::cout << "minmax(7, 3) = (" << mn << ", " << mx << ")\n";

    std::vector<int> v = {5, 2, 8, 1, 9, 3};
    auto [vmin, vmax] = std::minmax_element(v.begin(), v.end());
    std::cout << "minmax_element: min=" << *vmin << " max=" << *vmax << "\n";

    std::cout << "min({3,1,4,1,5}): " << std::min({3, 1, 4, 1, 5}) << "\n";
    std::cout << "max({3,1,4,1,5}): " << std::max({3, 1, 4, 1, 5}) << "\n";

    std::cout << "\n自定义比较:\n";
    std::vector<std::string> names = {"Alice", "Bob", "Charlie", "Dave"};
    auto longest = std::max_element(names.begin(), names.end(),
        [](const std::string& a, const std::string& b) { return a.size() < b.size(); });
    std::cout << "最长名字: " << *longest << "\n";

    std::cout << "\nclamp (C++17):\n";
    std::cout << "  clamp(15, 0, 10) = " << std::clamp(15, 0, 10) << "\n";
    std::cout << "  clamp(5, 0, 10) = " << std::clamp(5, 0, 10) << "\n";
    std::cout << "  clamp(-3, 0, 10) = " << std::clamp(-3, 0, 10) << "\n";

    std::cout << "\n";
}

void demo_comparison_algorithms() {
    std::cout << "=== 比较算法 ===\n";

    std::vector<int> v1 = {1, 2, 3, 4, 5};
    std::vector<int> v2 = {1, 2, 3, 4, 5};
    std::vector<int> v3 = {1, 2, 3};

    std::cout << "equal(v1, v2): " << (std::equal(v1.begin(), v1.end(), v2.begin()) ? "是" : "否") << "\n";
    std::cout << "equal(v1, v3): " << (std::equal(v1.begin(), v1.begin() + 3, v3.begin()) ? "是(前3个)" : "否") << "\n";

    auto [mismatch_it1, mismatch_it2] = std::mismatch(v1.begin(), v1.end(), v2.begin());
    std::cout << "mismatch: " << (mismatch_it1 == v1.end() ? "完全匹配" : "不匹配") << "\n";

    std::vector<int> v4 = {1, 2, 0, 4, 5};
    auto [m1, m2] = std::mismatch(v1.begin(), v1.end(), v4.begin());
    if (m1 != v1.end()) {
        std::cout << "mismatch 位置: v1[" << (m1 - v1.begin()) << "]=" << *m1
                  << ", v4[" << (m2 - v4.begin()) << "]=" << *m2 << "\n";
    }

    std::cout << "lexicographical_compare:\n";
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {1, 2, 4};
    std::cout << "  {1,2,3} < {1,2,4}: " << (std::lexicographical_compare(a.begin(), a.end(), b.begin(), b.end()) ? "是" : "否") << "\n";

    std::cout << "\n";
}

int main() {
    demo_find_algorithms();
    demo_count_algorithms();
    demo_predicate_algorithms();
    demo_for_each();
    demo_min_max();
    demo_comparison_algorithms();

    return 0;
}
