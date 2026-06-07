/**
 * @file 02_example_range_algorithms.cpp
 * @brief Ranges算法示例
 * @description 对应文档: 02-CPP/24-ranges
 */

#include <iostream>
#include <vector>
#include <algorithm>
#include <ranges>
#include <numeric>
#include <string>
#include <iterator>

void demo_ranges_vs_classic() {
    std::cout << "\n=== Ranges算法 vs 经典算法 ===\n";

    std::vector<int> nums = {5, 3, 8, 1, 9, 2, 7, 4, 6};

    std::vector<int> copy1 = nums;
    std::sort(copy1.begin(), copy1.end());
    std::cout << "经典sort: ";
    for (int n : copy1) std::cout << n << " ";
    std::cout << "\n";

    std::vector<int> copy2 = nums;
    std::ranges::sort(copy2);
    std::cout << "Ranges sort: ";
    for (int n : copy2) std::cout << n << " ";
    std::cout << "\n";

    auto it = std::ranges::find(nums, 8);
    if (it != nums.end()) {
        std::cout << "找到: " << *it << "\n";
    }

    auto has_big = std::ranges::any_of(nums, [](int n) { return n > 8; });
    std::cout << "是否有>8的: " << std::boolalpha << has_big << "\n";

    auto all_positive = std::ranges::all_of(nums, [](int n) { return n > 0; });
    std::cout << "是否全部>0: " << std::boolalpha << all_positive << "\n";

    auto none_neg = std::ranges::none_of(nums, [](int n) { return n < 0; });
    std::cout << "是否没有<0的: " << std::boolalpha << none_neg << "\n";
}

void demo_projections() {
    std::cout << "\n=== 投影(Projections) ===\n";

    struct Student {
        std::string name;
        int score;
    };

    std::vector<Student> students = {
        {"alice", 90}, {"bob", 85}, {"charlie", 95}, {"diana", 88}
    };

    std::ranges::sort(students, {}, &Student::score);
    std::cout << "按分数排序: ";
    for (const auto& s : students) std::cout << s.name << "(" << s.score << ") ";
    std::cout << "\n";

    std::ranges::sort(students, {}, &Student::name);
    std::cout << "按姓名排序: ";
    for (const auto& s : students) std::cout << s.name << "(" << s.score << ") ";
    std::cout << "\n";

    std::ranges::sort(students, std::greater{}, &Student::score);
    std::cout << "按分数降序: ";
    for (const auto& s : students) std::cout << s.name << "(" << s.score << ") ";
    std::cout << "\n";

    auto it = std::ranges::find(students, "bob", &Student::name);
    if (it != students.end()) {
        std::cout << "找到学生: " << it->name << " 分数: " << it->score << "\n";
    }

    auto high = std::ranges::count_if(students, [](int s) { return s >= 90; }, &Student::score);
    std::cout << "90分以上人数: " << high << "\n";
}

void demo_constrained_algorithms() {
    std::cout << "\n=== 受约束算法 ===\n";

    std::vector<int> nums = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};

    auto [min_val, max_val] = std::ranges::minmax(nums);
    std::cout << "最小值: " << min_val << " 最大值: " << max_val << "\n";

    std::cout << "min: " << std::ranges::min(nums) << "\n";
    std::cout << "max: " << std::ranges::max(nums) << "\n";

    std::vector<int> to_find = {1, 5, 9};
    auto [first, second] = std::ranges::mismatch(nums, to_find);
    if (first != nums.end() && second != to_find.end()) {
        std::cout << "mismatch: " << *first << " vs " << *second << "\n";
    }

    std::vector<int> src = {10, 20, 30};
    std::vector<int> dst(3);
    std::ranges::copy(src, dst.begin());
    std::cout << "copy结果: ";
    for (int n : dst) std::cout << n << " ";
    std::cout << "\n";

    std::vector<int> transformed(5);
    std::ranges::transform(nums | std::views::take(5), transformed.begin(),
                           [](int n) { return n * 10; });
    std::cout << "transform结果: ";
    for (int n : transformed) std::cout << n << " ";
    std::cout << "\n";
}

void demo_ranges_with_views() {
    std::cout << "\n=== Ranges算法与视图结合 ===\n";

    std::vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    auto even_squares = nums
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * n; });

    auto count = std::ranges::count_if(even_squares, [](int n) { return n > 20; });
    std::cout << "偶数平方中>20的个数: " << count << "\n";

    auto total = std::accumulate(even_squares.begin(), even_squares.end(), 0);
    std::cout << "偶数平方之和: " << total << "\n";

    std::vector<int> vec = {5, 3, 1, 4, 2};
    auto top3 = vec | std::views::take(3);
    std::ranges::sort(vec);
    auto sorted_top3 = vec | std::views::take(3);
    std::cout << "排序后前3: ";
    for (int n : sorted_top3) std::cout << n << " ";
    std::cout << "\n";
}

void demo_fold_and_more() {
    std::cout << "\n=== 折叠与更多算法 ===\n";

    std::vector<int> nums = {1, 2, 3, 4, 5};

    int sum = std::accumulate(nums.begin(), nums.end(), 0);
    std::cout << "累加和: " << sum << "\n";

    int product = std::accumulate(nums.begin(), nums.end(), 1, std::multiplies<int>());
    std::cout << "累乘积: " << product << "\n";

    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {1, 2, 3};
    bool equal = std::ranges::equal(a, b);
    std::cout << "a==b: " << std::boolalpha << equal << "\n";

    std::vector<int> c = {1, 2, 3, 4, 5};
    bool starts = std::ranges::equal(a, std::views::take(c, 3));
    std::cout << "c以a开头: " << std::boolalpha << starts << "\n";

    bool ends = std::ranges::equal(std::vector{4, 5}, std::views::drop(c, 3));
    std::cout << "c以{4,5}结尾: " << std::boolalpha << ends << "\n";
}

int main() {
    std::cout << "========== C++20 Ranges 算法示例 ==========\n";

    demo_ranges_vs_classic();
    demo_projections();
    demo_constrained_algorithms();
    demo_ranges_with_views();
    demo_fold_and_more();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
