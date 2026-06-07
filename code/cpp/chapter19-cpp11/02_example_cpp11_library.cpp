/** @file 02_example_cpp11_library.cpp
 *  @brief C++11库特性：array, tuple, unordered_map, chrono, regex, thread
 *  @description 对应文档: 02-CPP/20-cpp11 | 演示C++11标准库的重要新增
 *  编译命令: g++ -std=c++20 02_example_cpp11_library.cpp -o 02_example_cpp11_library
 */

#include <iostream>
#include <array>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <regex>
#include <thread>
#include <mutex>
#include <string>
#include <vector>
#include <algorithm>
#include <numeric>

void demo_std_array() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  std::array —— 固定大小数组\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::array<int, 5> arr = {1, 2, 3, 4, 5};

    std::cout << "vs C数组:\n";
    std::cout << "  std::array 知道自身大小，支持拷贝赋值\n";
    std::cout << "  不会退化为指针，更安全\n\n";

    std::cout << "基本操作:\n";
    std::cout << "  size(): " << arr.size() << "\n";
    std::cout << "  front(): " << arr.front() << "\n";
    std::cout << "  back(): " << arr.back() << "\n";
    std::cout << "  [2]: " << arr[2] << "\n";
    std::cout << "  at(2): " << arr.at(2) << " (带边界检查)\n";
    std::cout << "  data(): " << arr.data() << " (原始指针)\n\n";

    std::cout << "遍历:\n  ";
    for (const auto& x : arr) std::cout << x << " ";
    std::cout << "\n\n";

    std::cout << "排序:\n  ";
    std::array<int, 5> unsorted = {5, 3, 1, 4, 2};
    std::sort(unsorted.begin(), unsorted.end());
    for (const auto& x : unsorted) std::cout << x << " ";
    std::cout << "\n\n";

    std::cout << "填充:\n  ";
    std::array<int, 5> filled;
    filled.fill(42);
    for (const auto& x : filled) std::cout << x << " ";
    std::cout << "\n";
}

void demo_std_tuple() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::tuple —— 元组\n";
    std::cout << "═══════════════════════════════════════\n\n";

    auto person = std::make_tuple("张三", 25, 95.5);

    std::cout << "创建: make_tuple(\"张三\", 25, 95.5)\n";
    std::cout << "  get<0>: " << std::get<0>(person) << "\n";
    std::cout << "  get<1>: " << std::get<1>(person) << "\n";
    std::cout << "  get<2>: " << std::get<2>(person) << "\n";
    std::cout << "  size: " << std::tuple_size<decltype(person)>::value << "\n\n";

    std::string name;
    int age;
    double score;
    std::tie(name, age, score) = person;
    std::cout << "tie 解包: name=" << name << ", age=" << age << ", score=" << score << "\n\n";

    auto [n, a, s] = person;
    std::cout << "结构化绑定(C++17): name=" << n << ", age=" << a << ", score=" << s << "\n\n";

    std::cout << "tuple 用于多返回值:\n";
    auto divide = [](int a, int b) -> std::tuple<int, int> {
        return {a / b, a % b};
    };
    auto [quotient, remainder] = divide(17, 5);
    std::cout << "  17 / 5 = " << quotient << " 余 " << remainder << "\n\n";

    std::cout << "tuple 拼接:\n";
    auto t1 = std::make_tuple(1, 2);
    auto t2 = std::make_tuple("hello", 3.14);
    auto combined = std::tuple_cat(t1, t2);
    std::cout << "  tuple_cat 大小: " << std::tuple_size<decltype(combined)>::value << "\n";
}

void demo_unordered_map() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  unordered_map —— 哈希表\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::unordered_map<std::string, int> word_count;

    std::vector<std::string> words = {"apple", "banana", "apple", "cherry", "banana", "apple"};
    for (const auto& w : words) {
        word_count[w]++;
    }

    std::cout << "词频统计:\n";
    for (const auto& [word, count] : word_count) {
        std::cout << "  " << word << ": " << count << "\n";
    }

    std::cout << "\nvs map:\n";
    std::cout << "  map:            基于红黑树，有序，O(log n)\n";
    std::cout << "  unordered_map:  基于哈希表，无序，平均O(1)\n\n";

    std::cout << "查找:\n";
    auto it = word_count.find("apple");
    if (it != word_count.end()) {
        std::cout << "  find(\"apple\"): " << it->second << "\n";
    }

    std::cout << "  count(\"apple\"): " << word_count.count("apple") << "\n\n";

    std::cout << "桶信息:\n";
    std::cout << "  bucket_count: " << word_count.bucket_count() << "\n";
    std::cout << "  load_factor: " << word_count.load_factor() << "\n";
    std::cout << "  max_load_factor: " << word_count.max_load_factor() << "\n";

    std::cout << "\nunordered_set:\n";
    std::unordered_set<int> seen;
    for (int x : {1, 2, 3, 2, 1, 4, 3}) {
        auto [iter, inserted] = seen.insert(x);
        std::cout << "  插入 " << x << ": " << (inserted ? "新" : "重复") << "\n";
    }
}

void demo_chrono_and_regex() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  chrono 与 regex 快速演示\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "chrono 计时:\n";
    auto start = std::chrono::steady_clock::now();

    volatile long long sum = 0;
    for (int i = 0; i < 1000000; i++) sum += i;

    auto end = std::chrono::steady_clock::now();
    auto ms = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "  100万次加法: " << ms.count() << " 微秒\n\n";

    std::cout << "regex 匹配:\n";
    std::regex email_re(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
    std::vector<std::string> test = {"user@mail.com", "invalid", "test@domain.org"};
    for (const auto& s : test) {
        std::cout << "  \"" << s << "\": " << std::regex_match(s, email_re) << "\n";
    }
}

void demo_thread_basics() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  thread —— 线程基础\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::mutex mtx;
    int counter = 0;

    auto worker = [&counter, &mtx](int id, int iterations) {
        for (int i = 0; i < iterations; i++) {
            std::lock_guard<std::mutex> lock(mtx);
            counter++;
        }
    };

    std::thread t1(worker, 1, 10000);
    std::thread t2(worker, 2, 10000);

    t1.join();
    t2.join();

    std::cout << "两个线程各递增10000次:\n";
    std::cout << "  最终值: " << counter << " (期望20000)\n";
    std::cout << "  使用 lock_guard 保证线程安全\n\n";

    std::cout << "thread 基本操作:\n";
    std::cout << "  std::thread t(func, args...)  —— 创建线程\n";
    std::cout << "  t.join()    —— 等待线程完成\n";
    std::cout << "  t.detach()  —— 分离线程(后台运行)\n";
    std::cout << "  t.joinable() —— 线程是否可join\n";
    std::cout << "  std::this_thread::get_id() —— 当前线程ID\n";
    std::cout << "  std::this_thread::sleep_for() —— 休眠\n";
    std::cout << "  std::this_thread::yield() —— 让出时间片\n";
}

int main() {
    demo_std_array();
    demo_std_tuple();
    demo_unordered_map();
    demo_chrono_and_regex();
    demo_thread_basics();
    return 0;
}
