/** @file 02_deep_dive_algorithm_patterns.cpp
 *  @brief 算法模式：算法+lambda、投影、并行算法、复杂度指南
 *  @description 对应文档: 15-STL算法与迭代器 | 举一反三：掌握STL算法的组合模式
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <functional>
#include <map>
#include <set>

void demo_algorithm_lambda_patterns() {
    std::cout << "=== 算法 + Lambda 模式 ===\n";

    std::vector<std::string> words = {"banana", "apple", "cherry", "date", "elderberry"};

    std::cout << "--- 按长度排序 ---\n";
    std::sort(words.begin(), words.end(),
        [](const std::string& a, const std::string& b) { return a.size() < b.size(); });
    for (const auto& w : words) std::cout << "  " << w << " (" << w.size() << ")\n";

    std::cout << "\n--- 过滤 + 变换 ---\n";
    std::vector<int> nums = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> even_squares;
    std::for_each(nums.begin(), nums.end(), [&even_squares](int x) {
        if (x % 2 == 0) even_squares.push_back(x * x);
    });
    std::cout << "偶数的平方: ";
    for (auto x : even_squares) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "\n--- 分组 ---\n";
    std::map<int, std::vector<int>> groups;
    std::for_each(nums.begin(), nums.end(), [&groups](int x) {
        groups[x % 3].push_back(x);
    });
    for (const auto& [key, vals] : groups) {
        std::cout << "  模3=" << key << ": ";
        for (auto v : vals) std::cout << v << " ";
        std::cout << "\n";
    }

    std::cout << "\n--- 频率统计 ---\n";
    std::vector<std::string> items = {"apple", "banana", "apple", "cherry", "banana", "apple"};
    std::map<std::string, int> freq;
    std::for_each(items.begin(), items.end(), [&freq](const std::string& s) {
        ++freq[s];
    });
    for (const auto& [item, count] : freq) {
        std::cout << "  " << item << ": " << count << "\n";
    }

    std::cout << "\n";
}

struct Employee {
    std::string name;
    std::string department;
    double salary;
    int age;
};

void demo_projection_pattern() {
    std::cout << "=== 投影模式 (Ranges 预览) ===\n";

    std::vector<Employee> employees = {
        {"Alice",   "工程", 15000, 28},
        {"Bob",     "市场", 12000, 35},
        {"Charlie", "工程", 18000, 32},
        {"Diana",   "市场", 13000, 26},
        {"Eve",     "工程", 16000, 30}
    };

    std::cout << "--- 按薪资排序 ---\n";
    std::sort(employees.begin(), employees.end(),
        [](const Employee& a, const Employee& b) { return a.salary > b.salary; });
    for (const auto& e : employees) {
        std::cout << "  " << e.name << ": " << e.salary << "\n";
    }

    std::cout << "\n--- 查找最高薪资 ---\n";
    auto max_it = std::max_element(employees.begin(), employees.end(),
        [](const Employee& a, const Employee& b) { return a.salary < b.salary; });
    std::cout << "  最高薪资: " << max_it->name << " (" << max_it->salary << ")\n";

    std::cout << "\n--- 按部门分组 ---\n";
    std::map<std::string, std::vector<Employee>> by_dept;
    std::for_each(employees.begin(), employees.end(), [&by_dept](const Employee& e) {
        by_dept[e.department].push_back(e);
    });
    for (auto& [dept, emps] : by_dept) {
        double total = std::accumulate(emps.begin(), emps.end(), 0.0,
            [](double sum, const Employee& e) { return sum + e.salary; });
        std::cout << "  " << dept << ": 人数=" << emps.size() << ", 平均薪资=" << total / emps.size() << "\n";
    }

    std::cout << "\n--- 提取字段 ---\n";
    std::vector<std::string> names;
    std::transform(employees.begin(), employees.end(), std::back_inserter(names),
        [](const Employee& e) { return e.name; });
    std::cout << "  名字: ";
    for (const auto& n : names) std::cout << n << " ";
    std::cout << "\n";

    std::vector<double> salaries;
    std::transform(employees.begin(), employees.end(), std::back_inserter(salaries),
        [](const Employee& e) { return e.salary; });
    double avg = std::accumulate(salaries.begin(), salaries.end(), 0.0) / salaries.size();
    std::cout << "  平均薪资: " << avg << "\n";

    std::cout << "\nC++20 Ranges 的投影:\n";
    std::cout << "  ranges::sort(employees, {}, &Employee::salary);\n";
    std::cout << "  不再需要手写 lambda, 直接传成员指针\n";

    std::cout << "\n";
}

void demo_parallel_algorithms() {
    std::cout << "=== 并行算法 (C++17) ===\n";

    std::cout << "C++17 引入并行版算法, 需要头文件 <execution>:\n\n";

    std::cout << "执行策略:\n";
    std::cout << "  std::execution::seq       - 顺序执行\n";
    std::cout << "  std::execution::par       - 并行执行\n";
    std::cout << "  std::execution::par_unseq - 并行+向量化\n\n";

    std::cout << "支持并行的算法:\n";
    std::cout << "  sort, stable_sort, nth_element\n";
    std::cout << "  for_each, for_each_n\n";
    std::cout << "  transform, replace, fill\n";
    std::cout << "  count, min_element, max_element\n";
    std::cout << "  reduce, transform_reduce\n";
    std::cout << "  inclusive_scan, exclusive_scan\n\n";

    std::cout << "使用示例:\n";
    std::cout << "  #include <execution>\n";
    std::cout << "  std::sort(std::execution::par, v.begin(), v.end());\n";
    std::cout << "  std::for_each(std::execution::par, v.begin(), v.end(), func);\n\n";

    std::cout << "注意事项:\n";
    std::cout << "  1. 并行算法中的操作必须线程安全\n";
    std::cout << "  2. 不能使用有数据竞争的 lambda\n";
    std::cout << "  3. 小数据集可能比顺序更慢 (线程开销)\n";
    std::cout << "  4. 需要编译器/库支持\n";

    std::cout << "\n";
}

void demo_algorithm_complexity_guide() {
    std::cout << "=== 算法复杂度指南 ===\n";

    std::cout << "O(1) 操作:\n";
    std::cout << "  advance (随机访问迭代器)\n";
    std::cout << "  distance (随机访问迭代器)\n\n";

    std::cout << "O(log n) 操作:\n";
    std::cout << "  binary_search, lower_bound, upper_bound\n";
    std::cout << "  equal_range\n";
    std::cout << "  set_intersection, set_union 等 (有序范围)\n\n";

    std::cout << "O(n) 操作:\n";
    std::cout << "  find, count, for_each, transform\n";
    std::cout << "  copy, fill, generate, replace\n";
    std::cout << "  accumulate, inner_product\n";
    std::cout << "  min_element, max_element\n";
    std::cout << "  remove, unique (单次遍历)\n\n";

    std::cout << "O(n log n) 操作:\n";
    std::cout << "  sort, stable_sort\n";
    std::cout << "  partial_sort\n";
    std::cout << "  inplace_merge\n\n";

    std::cout << "O(n²) 操作:\n";
    std::cout << "  find_end, search (最坏情况)\n";
    std::cout << "  next_permutation (单次调用 O(n), 全排列 O(n!))\n\n";

    std::cout << "选择算法的原则:\n";
    std::cout << "  1. 有序范围优先用二分查找\n";
    std::cout << "  2. 只需前N个用 partial_sort\n";
    std::cout << "  3. 只需第N大用 nth_element\n";
    std::cout << "  4. 去重先排序再 unique\n";
    std::cout << "  5. 能用 STL 算法就不用手写循环\n";

    std::cout << "\n";
}

void demo_algorithm_best_practices() {
    std::cout << "=== 算法最佳实践 ===\n";

    std::cout << "1. 优先使用算法而非手写循环:\n";
    std::cout << "   std::find 比 while 循环更清晰\n";
    std::cout << "   std::sort 比手写快排更可靠\n\n";

    std::cout << "2. 选择正确的算法:\n";
    std::cout << "   查找有序范围 => binary_search / lower_bound\n";
    std::cout << "   查找无序范围 => find / find_if\n";
    std::cout << "   删除元素 => erase-remove 惯用法\n\n";

    std::cout << "3. 注意迭代器要求:\n";
    std::cout << "   sort 需要随机访问迭代器\n";
    std::cout << "   list 用自己的 sort 成员函数\n";
    std::cout << "   关联容器用成员函数而非算法\n\n";

    std::cout << "4. 避免不必要的拷贝:\n";
    std::cout << "   用 const auto& 代替 auto\n";
    std::cout << "   用 emplace_back 代替 push_back\n";
    std::cout << "   用 std::move 转移所有权\n\n";

    std::cout << "5. 算法组合:\n";
    std::cout << "   sort + unique => 去重\n";
    std::cout << "   sort + equal_range => 等值范围\n";
    std::cout << "   partition + sort => 分组排序\n";
    std::cout << "   transform + accumulate => 自定义聚合\n";

    std::cout << "\n";
}

int main() {
    demo_algorithm_lambda_patterns();
    demo_projection_pattern();
    demo_parallel_algorithms();
    demo_algorithm_complexity_guide();
    demo_algorithm_best_practices();

    return 0;
}
