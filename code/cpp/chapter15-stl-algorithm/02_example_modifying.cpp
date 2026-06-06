/** @file 02_example_modifying.cpp
 *  @brief 修改算法：copy、transform、remove、replace、fill、generate、unique、sort
 *  @description 对应文档: 15-STL算法与迭代器
 */

#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <numeric>
#include <iterator>

void print_vec(const std::vector<int>& v, const std::string& label = "") {
    if (!label.empty()) std::cout << label << ": ";
    for (auto x : v) std::cout << x << " ";
    std::cout << "\n";
}

void demo_copy_algorithms() {
    std::cout << "=== 拷贝算法 ===\n";

    std::vector<int> src = {1, 2, 3, 4, 5};
    std::vector<int> dst(5);

    std::copy(src.begin(), src.end(), dst.begin());
    print_vec(dst, "copy");

    std::vector<int> dst2;
    std::copy_if(src.begin(), src.end(), std::back_inserter(dst2),
        [](int x) { return x % 2 == 0; });
    print_vec(dst2, "copy_if(偶数)");

    std::vector<int> dst3(5);
    std::copy_n(src.begin(), 3, dst3.begin());
    print_vec(dst3, "copy_n(3个)");

    std::vector<int> dst4(5);
    std::reverse_copy(src.begin(), src.end(), dst4.begin());
    print_vec(dst4, "reverse_copy");

    std::cout << "\n";
}

void demo_transform() {
    std::cout << "=== transform ===\n";

    std::vector<int> v = {1, 2, 3, 4, 5};
    std::vector<int> result(v.size());

    std::transform(v.begin(), v.end(), result.begin(), [](int x) { return x * x; });
    print_vec(result, "平方");

    std::vector<int> v2 = {10, 20, 30, 40, 50};
    std::vector<int> sum(v.size());
    std::transform(v.begin(), v.end(), v2.begin(), sum.begin(),
        [](int a, int b) { return a + b; });
    print_vec(sum, "两向量相加");

    std::cout << "\n";
}

void demo_remove_algorithms() {
    std::cout << "=== 删除算法 (erase-remove 惯用法) ===\n";

    std::vector<int> v = {1, 2, 3, 2, 4, 2, 5};

    auto new_end = std::remove(v.begin(), v.end(), 2);
    print_vec(v, "remove(2)后 (逻辑删除)");
    std::cout << "  逻辑大小: " << (new_end - v.begin()) << ", 物理大小: " << v.size() << "\n";

    v.erase(new_end, v.end());
    print_vec(v, "erase后 (物理删除)");

    std::vector<int> v2 = {1, 2, 3, 4, 5, 6, 7, 8};
    v2.erase(std::remove_if(v2.begin(), v2.end(), [](int x) { return x % 2 == 0; }), v2.end());
    print_vec(v2, "erase-remove_if(偶数)");

    std::cout << "\nerase-remove 惯用法:\n";
    std::cout << "  v.erase(std::remove(v.begin(), v.end(), value), v.end());\n";
    std::cout << "  第一步: remove 把要删除的元素移到末尾\n";
    std::cout << "  第二步: erase 真正删除末尾元素\n";

    std::cout << "\n";
}

void demo_replace_algorithms() {
    std::cout << "=== 替换算法 ===\n";

    std::vector<int> v = {1, 2, 3, 2, 4, 2, 5};

    std::replace(v.begin(), v.end(), 2, 99);
    print_vec(v, "replace(2, 99)");

    std::replace_if(v.begin(), v.end(), [](int x) { return x > 50; }, 0);
    print_vec(v, "replace_if(>50, 0)");

    std::cout << "\n";
}

void demo_fill_generate() {
    std::cout << "=== fill/generate ===\n";

    std::vector<int> v1(10);
    std::fill(v1.begin(), v1.end(), 42);
    print_vec(v1, "fill(42)");

    std::vector<int> v2(5);
    std::fill_n(v2.begin(), 3, 7);
    print_vec(v2, "fill_n(3个7)");

    int counter = 0;
    std::vector<int> v3(5);
    std::generate(v3.begin(), v3.end(), [&counter]() { return counter++; });
    print_vec(v3, "generate(递增)");

    std::vector<int> v4(5);
    std::iota(v4.begin(), v4.end(), 10);
    print_vec(v4, "iota(10起)");

    std::cout << "\n";
}

void demo_unique() {
    std::cout << "=== unique ===\n";

    std::vector<int> v = {1, 1, 2, 2, 2, 3, 3, 4, 5, 5};

    auto new_end = std::unique(v.begin(), v.end());
    v.erase(new_end, v.end());
    print_vec(v, "unique (相邻去重)");

    std::vector<int> v2 = {3, 1, 4, 1, 5, 9, 2, 6, 5, 3};
    std::sort(v2.begin(), v2.end());
    v2.erase(std::unique(v2.begin(), v2.end()), v2.end());
    print_vec(v2, "sort + unique (完全去重)");

    std::cout << "\nunique 只去除相邻重复, 需要先排序\n";

    std::cout << "\n";
}

void demo_sort() {
    std::cout << "=== sort / stable_sort ===\n";

    std::vector<int> v = {5, 2, 8, 1, 9, 3, 7, 4, 6};

    std::sort(v.begin(), v.end());
    print_vec(v, "sort (升序)");

    std::sort(v.begin(), v.end(), std::greater<int>());
    print_vec(v, "sort (降序)");

    struct Student {
        std::string name;
        int score;
    };

    std::vector<Student> students = {
        {"Alice", 95}, {"Bob", 87}, {"Charlie", 95}, {"David", 78}
    };

    std::stable_sort(students.begin(), students.end(),
        [](const Student& a, const Student& b) { return a.score > b.score; });

    std::cout << "stable_sort 按分数降序:\n";
    for (const auto& s : students) {
        std::cout << "  " << s.name << ": " << s.score << "\n";
    }
    std::cout << "  同分数的 Alice 和 Charlie 保持原始顺序\n";

    std::cout << "\npartial_sort:\n";
    std::vector<int> v2 = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    std::partial_sort(v2.begin(), v2.begin() + 3, v2.end());
    print_vec(v2, "partial_sort(前3个最小)");

    std::cout << "\nnth_element:\n";
    std::vector<int> v3 = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    std::nth_element(v3.begin(), v3.begin() + 4, v3.end());
    std::cout << "nth_element(第5小): v3[4]=" << v3[4] << "\n";
    std::cout << "  前4个 <= v3[4], 后4个 >= v3[4], 但不保证有序\n";

    std::cout << "\n";
}

int main() {
    demo_copy_algorithms();
    demo_transform();
    demo_remove_algorithms();
    demo_replace_algorithms();
    demo_fill_generate();
    demo_unique();
    demo_sort();

    return 0;
}
