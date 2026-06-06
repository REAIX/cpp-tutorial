/** @file 01_deep_dive_lambda_advanced.cpp
 *  @brief Lambda进阶：移动捕获、STL算法中的lambda、回调、类型擦除
 *  @description 对应文档: 13-Lambda与函数对象 | 举一反三：掌握Lambda的高级用法
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <functional>
#include <memory>
#include <map>

void demo_move_capture() {
    std::cout << "=== 移动捕获 (C++14/20) ===\n";

    std::cout << "--- C++14: 初始化捕获实现移动 ---\n";
    {
        auto ptr = std::make_unique<int>(42);
        auto lambda = [p = std::move(ptr)]() {
            std::cout << "移动捕获 unique_ptr: *p = " << *p << "\n";
        };
        lambda();
        std::cout << "ptr 移动后: " << (ptr ? "非空" : "空") << "\n";
    }

    std::cout << "\n--- 移动捕获大型对象 ---\n";
    {
        std::vector<int> big_data(1000, 42);
        auto processor = [data = std::move(big_data)]() {
            std::cout << "处理数据, 大小=" << data.size() << "\n";
        };
        processor();
        std::cout << "big_data 移动后大小: " << big_data.size() << "\n";
    }

    std::cout << "\n--- C++20: 模板 lambda ---\n";
    {
        auto print = []<typename T>(const T& value) {
            std::cout << "模板 lambda: " << value << "\n";
        };
        print(42);
        print(3.14);
        print(std::string("hello"));
    }

    std::cout << "\n";
}

void demo_lambda_in_stl() {
    std::cout << "=== STL 算法中的 Lambda ===\n";

    std::vector<int> nums = {5, 2, 8, 1, 9, 3, 7, 4, 6};

    std::cout << "--- 排序 ---\n";
    std::sort(nums.begin(), nums.end(), [](int a, int b) { return a > b; });
    std::cout << "降序: ";
    for (int n : nums) std::cout << n << " ";
    std::cout << "\n\n";

    std::cout << "--- 查找 ---\n";
    auto it = std::find_if(nums.begin(), nums.end(), [](int n) { return n > 7; });
    if (it != nums.end()) {
        std::cout << "第一个大于7的: " << *it << "\n";
    }

    std::cout << "\n--- 计数 ---\n";
    int count = std::count_if(nums.begin(), nums.end(), [](int n) { return n % 2 == 0; });
    std::cout << "偶数个数: " << count << "\n";

    std::cout << "\n--- 删除 ---\n";
    nums.erase(std::remove_if(nums.begin(), nums.end(), [](int n) { return n < 5; }), nums.end());
    std::cout << "删除小于5的: ";
    for (int n : nums) std::cout << n << " ";
    std::cout << "\n";

    std::cout << "\n--- 变换 ---\n";
    std::vector<int> doubled(nums.size());
    std::transform(nums.begin(), nums.end(), doubled.begin(), [](int n) { return n * 2; });
    std::cout << "翻倍: ";
    for (int n : doubled) std::cout << n << " ";
    std::cout << "\n";

    std::cout << "\n--- 遍历 ---\n";
    std::for_each(nums.begin(), nums.end(), [](int n) {
        std::cout << n << (n % 2 == 0 ? "(偶) " : "(奇) ");
    });
    std::cout << "\n";

    std::cout << "\n";
}

class Button {
public:
    using ClickHandler = std::function<void()>;

    void on_click(ClickHandler handler) {
        handler_ = std::move(handler);
    }

    void click() {
        if (handler_) handler_();
    }
private:
    ClickHandler handler_;
};

void demo_lambda_as_callback() {
    std::cout << "=== Lambda 作为回调 ===\n";

    Button btn;
    int click_count = 0;

    btn.on_click([&click_count]() {
        ++click_count;
        std::cout << "按钮被点击! 次数: " << click_count << "\n";
    });

    btn.click();
    btn.click();
    btn.click();

    std::cout << "\n--- 异步回调 ---\n";
    {
        using AsyncCallback = std::function<void(int result)>;

        auto async_compute = [](int a, int b, AsyncCallback callback) {
            int result = a + b;
            callback(result);
        };

        async_compute(10, 20, [](int result) {
            std::cout << "异步计算结果: " << result << "\n";
        });
    }

    std::cout << "\n";
}

void demo_lambda_type_erasure() {
    std::cout << "=== Lambda 与类型擦除 ===\n";

    std::cout << "每个 lambda 有唯一的匿名类型:\n";
    {
        auto f1 = []() { return 1; };
        auto f2 = []() { return 1; };

        std::cout << "f1 和 f2 类型相同: " << (std::is_same_v<decltype(f1), decltype(f2)> ? "是" : "否") << "\n";
        std::cout << "即使签名相同, 每个 lambda 的类型也是唯一的\n\n";
    }

    std::cout << "std::function 统一存储:\n";
    {
        std::vector<std::function<int(int)>> operations;

        operations.push_back([](int x) { return x * 2; });
        operations.push_back([](int x) { return x + 10; });
        operations.push_back([](int x) { return x * x; });

        for (const auto& op : operations) {
            std::cout << "op(5) = " << op(5) << "\n";
        }
    }

    std::cout << "\n类型擦除的代价:\n";
    std::cout << "  1. 堆分配 (可能)\n";
    std::cout << "  2. 虚函数调用开销\n";
    std::cout << "  3. 失去内联优化\n";
    std::cout << "  4. 适合: 需要统一类型存储可调用对象时\n";

    std::cout << "\n";
}

void demo_lambda_size_and_performance() {
    std::cout << "=== Lambda 大小与性能 ===\n";

    int a = 1, b = 2, c = 3;
    double d = 4.0;

    auto no_capture = []() { return 42; };
    auto capture_one = [a]() { return a; };
    auto capture_two = [a, b]() { return a + b; };
    auto capture_ref = [&a, &b]() { return a + b; };
    auto capture_all = [a, b, c, d]() { return a + b + c + d; };

    std::cout << "无捕获 lambda 大小: " << sizeof(no_capture) << "\n";
    std::cout << "捕获1个int: " << sizeof(capture_one) << "\n";
    std::cout << "捕获2个int: " << sizeof(capture_two) << "\n";
    std::cout << "捕获2个引用: " << sizeof(capture_ref) << "\n";
    std::cout << "捕获3个int+1个double: " << sizeof(capture_all) << "\n";
    std::cout << "std::function 大小: " << sizeof(std::function<int()>) << "\n";

    std::cout << "\n性能要点:\n";
    std::cout << "  1. 无捕获 lambda 可转为函数指针 (零开销)\n";
    std::cout << "  2. 捕获 lambda 大小 = 捕获变量大小之和\n";
    std::cout << "  3. 引用捕获大小 = 指针大小\n";
    std::cout << "  4. std::function 有额外开销\n";
    std::cout << "  5. 模板参数接受 lambda 优于 std::function\n";

    std::cout << "\n";
}

int main() {
    demo_move_capture();
    demo_lambda_in_stl();
    demo_lambda_as_callback();
    demo_lambda_type_erasure();
    demo_lambda_size_and_performance();

    return 0;
}
