/** @file 03_example_unordered_container.cpp
 *  @brief 无序容器：unordered_map、unordered_set、哈希函数、桶、性能对比
 *  @description 对应文档: 14-STL容器
 */

#include <iostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <chrono>
#include <algorithm>
#include <functional>
#include <random>
#include <map>

void demo_unordered_map() {
    std::cout << "=== std::unordered_map ===\n";

    std::unordered_map<std::string, int> ages;

    ages["Alice"] = 25;
    ages["Bob"] = 30;
    ages.insert({"Charlie", 28});
    ages.emplace("David", 35);

    std::cout << "内容:\n";
    for (const auto& [name, age] : ages) {
        std::cout << "  " << name << ": " << age << "\n";
    }

    std::cout << "\n查找:\n";
    auto it = ages.find("Bob");
    if (it != ages.end()) {
        std::cout << "  find(\"Bob\"): " << it->second << "\n";
    }
    std::cout << "  count(\"Eve\") = " << ages.count("Eve") << "\n";

    std::cout << "\n桶信息:\n";
    std::cout << "  bucket_count() = " << ages.bucket_count() << "\n";
    std::cout << "  max_bucket_count() = " << ages.max_bucket_count() << "\n";
    std::cout << "  load_factor() = " << ages.load_factor() << "\n";
    std::cout << "  max_load_factor() = " << ages.max_load_factor() << "\n";

    ages.rehash(20);
    std::cout << "  rehash(20)后 bucket_count = " << ages.bucket_count() << "\n";

    std::cout << "\nunordered_map 特点:\n";
    std::cout << "  - 哈希表实现\n";
    std::cout << "  - 平均 O(1) 查找/插入/删除\n";
    std::cout << "  - 最坏 O(n) (哈希冲突严重时)\n";
    std::cout << "  - 元素无序\n";
    std::cout << "  - 键唯一\n";

    std::cout << "\n";
}

void demo_unordered_set() {
    std::cout << "=== std::unordered_set ===\n";

    std::unordered_set<int> us = {5, 3, 1, 4, 2};

    std::cout << "内容: ";
    for (auto x : us) std::cout << x << " ";
    std::cout << "\n";

    us.insert(6);
    us.insert(3);
    std::cout << "insert(6), insert(3): ";
    for (auto x : us) std::cout << x << " ";
    std::cout << "\n";

    std::cout << "find(4): " << (us.find(4) != us.end() ? "存在" : "不存在") << "\n";
    std::cout << "count(3): " << us.count(3) << "\n";

    std::cout << "\nunordered_set 特点:\n";
    std::cout << "  - 哈希集合\n";
    std::cout << "  - 平均 O(1) 操作\n";
    std::cout << "  - 适合快速查找/去重\n";

    std::cout << "\n";
}

struct Point {
    int x, y;
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
};

struct PointHash {
    size_t operator()(const Point& p) const {
        auto h1 = std::hash<int>{}(p.x);
        auto h2 = std::hash<int>{}(p.y);
        return h1 ^ (h2 << 1);
    }
};

void demo_custom_hash() {
    std::cout << "=== 自定义哈希函数 ===\n";

    std::unordered_set<Point, PointHash> points;
    points.insert({1, 2});
    points.insert({3, 4});
    points.insert({1, 2});

    std::cout << "自定义类型 unordered_set 大小: " << points.size() << "\n";
    std::cout << "find({1,2}): " << (points.find({1, 2}) != points.end() ? "存在" : "不存在") << "\n";

    std::cout << "\n自定义哈希的要求:\n";
    std::cout << "  1. 提供 operator()(const T&) const\n";
    std::cout << "  2. 相同输入必须返回相同哈希值\n";
    std::cout << "  3. 不同输入应尽量返回不同哈希值\n";
    std::cout << "  4. 类型必须提供 operator==\n";

    std::cout << "\n";
}

void demo_bucket_details() {
    std::cout << "=== 桶机制详解 ===\n";

    std::unordered_map<int, std::string> m;
    for (int i = 0; i < 20; ++i) {
        m[i] = "value_" + std::to_string(i);
    }

    std::cout << "插入20个元素后:\n";
    std::cout << "  bucket_count = " << m.bucket_count() << "\n";
    std::cout << "  size = " << m.size() << "\n";
    std::cout << "  load_factor = " << m.load_factor() << "\n\n";

    std::cout << "各桶的元素数量:\n";
    size_t max_bucket_size = 0;
    size_t empty_buckets = 0;
    for (size_t i = 0; i < m.bucket_count(); ++i) {
        size_t sz = m.bucket_size(i);
        if (sz > max_bucket_size) max_bucket_size = sz;
        if (sz == 0) ++empty_buckets;
    }
    std::cout << "  最大桶大小: " << max_bucket_size << "\n";
    std::cout << "  空桶数量: " << empty_buckets << "\n";

    std::cout << "\n桶机制:\n";
    std::cout << "  1. 哈希值 % 桶数量 => 桶索引\n";
    std::cout << "  2. 同一桶内用链表存储冲突元素\n";
    std::cout << "  3. load_factor = 元素数 / 桶数\n";
    std::cout << "  4. 超过 max_load_factor 时自动 rehash\n";

    std::cout << "\n";
}

void demo_ordered_vs_unordered_performance() {
    std::cout << "=== 有序 vs 无序容器性能对比 ===\n";

    const int N = 100000;
    std::vector<int> keys(N);
    for (int i = 0; i < N; ++i) keys[i] = i;
    std::shuffle(keys.begin(), keys.end(), std::mt19937(std::random_device{}()));

    auto test_insert = [&](auto& container, const std::string& name) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int k : keys) container[k] = k;
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };

    auto test_find = [&](auto& container, const std::string& name) {
        auto start = std::chrono::high_resolution_clock::now();
        for (int i = 0; i < N; i += 10) {
            volatile auto it = container.find(i);  // volatile 防止优化消除
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    };

    std::map<int, int> omap;
    std::unordered_map<int, int> umap;

    auto omap_insert = test_insert(omap, "map");
    auto umap_insert = test_insert(umap, "unordered_map");

    auto omap_find = test_find(omap, "map");
    auto umap_find = test_find(umap, "unordered_map");

    std::cout << "插入 " << N << " 个元素:\n";
    std::cout << "  map:            " << omap_insert << " us\n";
    std::cout << "  unordered_map:  " << umap_insert << " us\n\n";

    std::cout << "查找 " << N / 10 << " 次:\n";
    std::cout << "  map:            " << omap_find << " us\n";
    std::cout << "  unordered_map:  " << umap_find << " us\n";

    std::cout << "\n选择指南:\n";
    std::cout << "  需要有序遍历 => map/set\n";
    std::cout << "  只需快速查找 => unordered_map/set\n";
    std::cout << "  需要范围查询 => map/set\n";
    std::cout << "  键类型无哈希函数 => map/set\n";

    std::cout << "\n";
}

int main() {
    demo_unordered_map();
    demo_unordered_set();
    demo_custom_hash();
    demo_bucket_details();
    demo_ordered_vs_unordered_performance();

    return 0;
}
