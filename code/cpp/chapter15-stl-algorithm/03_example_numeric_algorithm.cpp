/** @file 03_example_numeric_algorithm.cpp
 *  @brief 数值算法：accumulate、inner_product、partial_sum、iota、reduce
 *  @description 对应文档: 15-STL算法与迭代器
 */

#include <iostream>
#include <vector>
#include <string>
#include <numeric>
#include <algorithm>
#include <functional>
#include <cmath>

void print_vec(const std::vector<int>& v, const std::string& label = "") {
    if (!label.empty()) std::cout << label << ": ";
    for (auto x : v) std::cout << x << " ";
    std::cout << "\n";
}

void demo_accumulate() {
    std::cout << "=== std::accumulate ===\n";

    std::vector<int> v = {1, 2, 3, 4, 5};

    int sum = std::accumulate(v.begin(), v.end(), 0);
    std::cout << "求和: " << sum << "\n";

    int product = std::accumulate(v.begin(), v.end(), 1, std::multiplies<int>());
    std::cout << "求积: " << product << "\n";

    std::vector<std::string> words = {"Hello", " ", "World", "!"};
    std::string concat = std::accumulate(words.begin(), words.end(), std::string(""));
    std::cout << "字符串拼接: " << concat << "\n";

    std::string joined = std::accumulate(next(words.begin()), words.end(), words[0],
        [](const std::string& a, const std::string& b) { return a + ", " + b; });
    std::cout << "逗号连接: " << joined << "\n";

    double mean = static_cast<double>(std::accumulate(v.begin(), v.end(), 0)) / v.size();
    std::cout << "平均值: " << mean << "\n";

    std::cout << "\naccumulate 的要点:\n";
    std::cout << "  初始值决定返回类型!\n";
    std::cout << "  accumulate(begin, end, 0) => int 求和\n";
    std::cout << "  accumulate(begin, end, 0.0) => double 求和\n";

    std::cout << "\n";
}

void demo_inner_product() {
    std::cout << "=== std::inner_product ===\n";

    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {4, 5, 6};

    int dot = std::inner_product(a.begin(), a.end(), b.begin(), 0);
    std::cout << "点积: " << dot << " (1*4 + 2*5 + 3*6)\n";

    int sum_of_products = std::inner_product(a.begin(), a.end(), b.begin(), 0,
        std::plus<int>(), std::minus<int>());
    std::cout << "差之和: " << sum_of_products << " ((1-4) + (2-5) + (3-6))\n";

    std::cout << "\n";
}

void demo_partial_sum() {
    std::cout << "=== std::partial_sum ===\n";

    std::vector<int> v = {1, 2, 3, 4, 5};
    std::vector<int> result(v.size());

    std::partial_sum(v.begin(), v.end(), result.begin());
    print_vec(result, "前缀和");

    std::vector<int> result2(v.size());
    std::partial_sum(v.begin(), v.end(), result2.begin(), std::multiplies<int>());
    print_vec(result2, "前缀积");

    std::cout << "\n";
}

void demo_adjacent_difference() {
    std::cout << "=== std::adjacent_difference ===\n";

    std::vector<int> v = {1, 3, 6, 10, 15};
    std::vector<int> result(v.size());

    std::adjacent_difference(v.begin(), v.end(), result.begin());
    print_vec(result, "相邻差");

    std::vector<int> v2 = {1, 2, 6, 24, 120};
    std::vector<int> result2(v2.size());
    std::adjacent_difference(v2.begin(), v2.end(), result2.begin(), std::divides<int>());
    print_vec(result2, "相邻商");

    std::cout << "\n";
}

void demo_iota() {
    std::cout << "=== std::iota ===\n";

    std::vector<int> v(10);
    std::iota(v.begin(), v.end(), 0);
    print_vec(v, "iota(0起)");

    std::vector<int> v2(5);
    std::iota(v2.begin(), v2.end(), 100);
    print_vec(v2, "iota(100起)");

    std::cout << "\n";
}

void demo_reduce() {
    std::cout << "=== std::reduce (C++17) ===\n";

    std::vector<int> v = {1, 2, 3, 4, 5};

    int sum = std::reduce(v.begin(), v.end(), 0);
    std::cout << "reduce 求和: " << sum << "\n";

    int product = std::reduce(v.begin(), v.end(), 1, std::multiplies<int>());
    std::cout << "reduce 求积: " << product << "\n";

    std::cout << "\nreduce vs accumulate:\n";
    std::cout << "  accumulate: 从左到右顺序计算\n";
    std::cout << "  reduce: 可以并行计算 (不确定顺序)\n";
    std::cout << "  reduce 要求操作满足结合律和交换律\n";
    std::cout << "  对于加法/乘法, 两者结果相同\n";
    std::cout << "  对于减法/除法, 结果可能不同!\n";

    std::cout << "\n";
}

void demo_numeric_patterns() {
    std::cout << "=== 数值算法实战模式 ===\n";

    std::vector<double> data = {2.5, 3.7, 1.8, 4.2, 2.9, 3.1, 5.0, 1.5};

    double mean = std::accumulate(data.begin(), data.end(), 0.0) / data.size();
    std::cout << "均值: " << mean << "\n";

    double variance = std::accumulate(data.begin(), data.end(), 0.0,
        [mean](double acc, double x) {
            return acc + (x - mean) * (x - mean);
        }) / data.size();
    std::cout << "方差: " << variance << "\n";
    std::cout << "标准差: " << std::sqrt(variance) << "\n";

    auto [min_it, max_it] = std::minmax_element(data.begin(), data.end());
    std::cout << "范围: [" << *min_it << ", " << *max_it << "]\n";

    std::cout << "\n";
}

int main() {
    demo_accumulate();
    demo_inner_product();
    demo_partial_sum();
    demo_adjacent_difference();
    demo_iota();
    demo_reduce();
    demo_numeric_patterns();

    return 0;
}
