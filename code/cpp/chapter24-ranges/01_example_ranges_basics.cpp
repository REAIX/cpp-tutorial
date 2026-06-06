/**
 * @file 01_example_ranges_basics.cpp
 * @brief Ranges基础示例
 * @description 对应文档: 02-CPP/24-ranges
 */

#include <iostream>
#include <vector>
#include <ranges>
#include <algorithm>
#include <string>
#include <map>

void demo_views_and_adapters() {
    std::cout << "\n=== Views与适配器 ===\n";

    std::vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto filtered = nums | std::views::filter([](int n) { return n % 2 == 0; });
    std::cout << "过滤偶数: ";
    for (int n : filtered) std::cout << n << " ";
    std::cout << "\n";

    auto transformed = nums | std::views::transform([](int n) { return n * n; });
    std::cout << "平方变换: ";
    for (int n : transformed) std::cout << n << " ";
    std::cout << "\n";

    auto first_five = nums | std::views::take(5);
    std::cout << "取前5个: ";
    for (int n : first_five) std::cout << n << " ";
    std::cout << "\n";

    auto dropped = nums | std::views::drop(7);
    std::cout << "跳过前7个: ";
    for (int n : dropped) std::cout << n << " ";
    std::cout << "\n";
}

void demo_pipe_composition() {
    std::cout << "\n=== 管道组合 ===\n";

    std::vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto result = nums
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * n; })
        | std::views::take(3);
    std::cout << "偶数->平方->取前3: ";
    for (int n : result) std::cout << n << " ";
    std::cout << "\n";

    auto reversed = nums | std::views::reverse;
    std::cout << "反转: ";
    for (int n : reversed) std::cout << n << " ";
    std::cout << "\n";

    auto rev_filtered = nums
        | std::views::filter([](int n) { return n > 5; })
        | std::views::reverse
        | std::views::take(3);
    std::cout << "大于5->反转->取前3: ";
    for (int n : rev_filtered) std::cout << n << " ";
    std::cout << "\n";
}

void demo_more_adapters() {
    std::cout << "\n=== 更多适配器 ===\n";

    std::vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto evens = std::views::filter(nums, [](int n) { return n % 2 == 0; });
    auto squares = std::views::transform(evens, [](int n) { return n * n; });
    std::cout << "非管道方式组合: ";
    for (int n : squares) std::cout << n << " ";
    std::cout << "\n";

    std::vector<std::string> words = {"hello", "world", "cpp", "ranges", "are", "cool"};
    auto long_words = words | std::views::filter([](const std::string& s) { return s.size() > 3; });
    std::cout << "长度>3的单词: ";
    for (const auto& w : long_words) std::cout << w << " ";
    std::cout << "\n";

    auto keys_view = std::views::keys;
    std::map<std::string, int> scores = {{"alice", 90}, {"bob", 85}, {"charlie", 95}};
    std::cout << "map的键: ";
    for (const auto& k : scores | std::views::keys) std::cout << k << " ";
    std::cout << "\n";
    std::cout << "map的值: ";
    for (const auto& v : scores | std::views::values) std::cout << v << " ";
    std::cout << "\n";
}

void demo_view_factories() {
    std::cout << "\n=== 视图工厂 ===\n";

    auto iota_view = std::views::iota(1, 10);
    std::cout << "iota(1,10): ";
    for (int n : iota_view) std::cout << n << " ";
    std::cout << "\n";

    auto infinite = std::views::iota(1) | std::views::take(5);
    std::cout << "无限序列取前5: ";
    for (int n : infinite) std::cout << n << " ";
    std::cout << "\n";

    std::cout << "重复42取前5: ";
    for (int i = 0; i < 5; ++i) std::cout << 42 << " ";
    std::cout << "\n";

    auto single = std::views::single(100);
    std::cout << "单元素: ";
    for (int n : single) std::cout << n << " ";
    std::cout << "\n";

    auto empty = std::views::empty<int>;
    std::cout << "空视图大小: " << std::ranges::size(empty) << "\n";
}

void demo_common_view() {
    std::cout << "\n=== Common视图 ===\n";

    auto uncommon = std::views::iota(1) | std::views::take(5);
    auto common = uncommon | std::views::common;
    std::vector<int> vec(common.begin(), common.end());
    std::cout << "从视图构造vector: ";
    for (int n : vec) std::cout << n << " ";
    std::cout << "\n";

    std::vector<int> direct = std::vector<int>(
        (std::views::iota(1, 6)).begin(),
        (std::views::iota(1, 6)).end()
    );
    std::cout << "直接构造: ";
    for (int n : direct) std::cout << n << " ";
    std::cout << "\n";
}

void demo_counted_view() {
    std::cout << "\n=== Counted视图 ===\n";

    int arr[] = {10, 20, 30, 40, 50, 60, 70, 80};
    auto counted = std::views::counted(arr, 4);
    std::cout << "counted(arr, 4): ";
    for (int n : counted) std::cout << n << " ";
    std::cout << "\n";
}

int main() {
    std::cout << "========== C++20 Ranges 基础示例 ==========\n";

    demo_views_and_adapters();
    demo_pipe_composition();
    demo_more_adapters();
    demo_view_factories();
    demo_common_view();
    demo_counted_view();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
