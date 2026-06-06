/** @file 02_example_associative_container.cpp
 *  @brief 关联容器：map、set、multimap、multiset
 *  @description 对应文档: 14-STL容器
 */

#include <iostream>
#include <string>
#include <map>
#include <set>
#include <unordered_map>
#include <algorithm>

void demo_map() {
    std::cout << "=== std::map ===\n";

    std::map<std::string, int> scores;

    scores["Alice"] = 95;
    scores["Bob"] = 87;
    scores.insert({"Charlie", 92});
    scores.emplace("David", 88);

    std::cout << "插入元素:\n";
    for (const auto& [name, score] : scores) {
        std::cout << "  " << name << ": " << score << "\n";
    }

    std::cout << "\n查找:\n";
    auto it = scores.find("Bob");
    if (it != scores.end()) {
        std::cout << "  找到 Bob: " << it->second << "\n";
    }

    std::cout << "  count(\"Eve\") = " << scores.count("Eve") << "\n";

    std::cout << "\n访问不存在的键:\n";
    std::cout << "  scores[\"Eve\"] = " << scores["Eve"] << " (自动插入默认值0)\n";

    scores["Eve"] = 78;
    std::cout << "  修改后 scores[\"Eve\"] = " << scores["Eve"] << "\n";

    scores.erase("Eve");
    std::cout << "  erase(\"Eve\") 后 count = " << scores.count("Eve") << "\n";

    std::cout << "\nmap 特点:\n";
    std::cout << "  - 有序键值对 (红黑树)\n";
    std::cout << "  - 键唯一, 自动排序\n";
    std::cout << "  - 插入/查找/删除 O(log n)\n";
    std::cout << "  - operator[] 会插入不存在的键\n";

    std::cout << "\n";
}

void demo_set() {
    std::cout << "=== std::set ===\n";

    std::set<int> s = {5, 3, 1, 4, 2, 3, 1};

    std::cout << "初始化 {5,3,1,4,2,3,1}: ";
    for (auto x : s) std::cout << x << " ";
    std::cout << "(自动排序, 去重)\n";

    s.insert(6);
    auto [iter, success] = s.insert(3);
    std::cout << "insert(3) 成功: " << (success ? "是" : "否") << " (已存在)\n";

    std::cout << "\n范围查找:\n";
    auto lower = s.lower_bound(3);
    auto upper = s.upper_bound(5);
    std::cout << "  [3, 5) 范围: ";
    for (auto it = lower; it != upper; ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    std::cout << "\nset 特点:\n";
    std::cout << "  - 有序集合 (红黑树)\n";
    std::cout << "  - 元素唯一, 自动排序\n";
    std::cout << "  - 插入/查找/删除 O(log n)\n";
    std::cout << "  - 支持范围查询\n";

    std::cout << "\n";
}

void demo_multimap() {
    std::cout << "=== std::multimap ===\n";

    std::multimap<std::string, std::string> courses;

    courses.insert({"Alice", "数学"});
    courses.insert({"Alice", "物理"});
    courses.insert({"Bob", "化学"});
    courses.insert({"Alice", "化学"});
    courses.insert({"Bob", "数学"});

    std::cout << "所有课程:\n";
    for (const auto& [name, course] : courses) {
        std::cout << "  " << name << " -> " << course << "\n";
    }

    std::cout << "\nAlice 的课程:\n";
    auto range = courses.equal_range("Alice");
    for (auto it = range.first; it != range.second; ++it) {
        std::cout << "  " << it->second << "\n";
    }

    std::cout << "Alice 选课数: " << courses.count("Alice") << "\n";

    std::cout << "\nmultimap 特点:\n";
    std::cout << "  - 允许重复键\n";
    std::cout << "  - 没有 operator[]\n";
    std::cout << "  - 使用 equal_range 访问同一键的所有值\n";

    std::cout << "\n";
}

void demo_multiset() {
    std::cout << "=== std::multiset ===\n";

    std::multiset<int> ms = {3, 1, 4, 1, 5, 9, 2, 6, 5};

    std::cout << "内容: ";
    for (auto x : ms) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "count(1) = " << ms.count(1) << "\n";
    std::cout << "count(5) = " << ms.count(5) << "\n";

    ms.erase(ms.find(5));
    std::cout << "erase一个5后 count(5) = " << ms.count(5) << "\n";

    std::cout << "\nmultiset 特点:\n";
    std::cout << "  - 允许重复元素\n";
    std::cout << "  - 自动排序\n";
    std::cout << "  - erase(find(x)) 只删一个, erase(x) 删除所有\n";

    std::cout << "\n";
}

void demo_custom_comparator() {
    std::cout << "=== 自定义比较器 ===\n";

    std::map<std::string, int, std::greater<std::string>> desc_map;
    desc_map["Alice"] = 95;
    desc_map["Bob"] = 87;
    desc_map["Charlie"] = 92;

    std::cout << "降序 map:\n";
    for (const auto& [name, score] : desc_map) {
        std::cout << "  " << name << ": " << score << "\n";
    }

    struct CaseInsensitiveCompare {
        bool operator()(const std::string& a, const std::string& b) const {
            return std::lexicographical_compare(
                a.begin(), a.end(), b.begin(), b.end(),
                [](char c1, char c2) { return tolower(c1) < tolower(c2); }
            );
        }
    };

    std::set<std::string, CaseInsensitiveCompare> ci_set;
    ci_set.insert("Hello");
    ci_set.insert("hello");
    ci_set.insert("HELLO");
    std::cout << "\n大小写不敏感 set 大小: " << ci_set.size() << " (只保留一个)\n";

    std::cout << "\n";
}

int main() {
    demo_map();
    demo_set();
    demo_multimap();
    demo_multiset();
    demo_custom_comparator();

    return 0;
}
