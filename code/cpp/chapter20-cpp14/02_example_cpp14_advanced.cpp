/** @file 02_example_cpp14_advanced.cpp
 *  @brief C++14高级用法：初始化捕获、泛型lambda与算法、decltype(auto)完美返回、constexpr编译期计算、integer_sequence元编程、[[deprecated]]属性、std::exchange
 *  @description 对应文档: 02-CPP/21-cpp14 | 深入演示C++14高级特性在实际项目中的应用模式
 *  编译命令: g++ -std=c++20 02_example_cpp14_advanced.cpp -o 02_example_cpp14_advanced
 */

#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <array>
#include <algorithm>
#include <utility>
#include <type_traits>
#include <tuple>
#include <numeric>
#include <cstdint>

// ============================================================
// 6. [[deprecated]] 属性演示
// ============================================================

[[deprecated("请使用 compute_v2() 代替")]]
int compute_old(int x) {
    return x * x;
}

int compute_v2(int x) {
    return x * x + 1;
}

[[deprecated("此API已弃用，请迁移至 process_new()")]]
void process_old() {}

void process_new() {}

// ============================================================
// 4. constexpr 编译期计算
// ============================================================

// 编译期字符串哈希 (FNV-1a 算法)
constexpr std::uint32_t fnv1a_hash(const char* str, std::size_t len) {
    std::uint32_t hash = 0x811c9dc5;
    for (std::size_t i = 0; i < len; ++i) {
        hash ^= static_cast<std::uint32_t>(str[i]);
        hash *= 0x01000193;
    }
    return hash;
}

// 辅助：编译期计算C字符串长度
constexpr std::size_t const_strlen(const char* str) {
    std::size_t len = 0;
    while (str[len] != '\0') ++len;
    return len;
}

// 编译期生成查找表
constexpr std::array<int, 10> make_square_table() {
    std::array<int, 10> table{};
    for (int i = 0; i < 10; ++i) {
        table[i] = i * i;
    }
    return table;
}

// 编译期生成斐波那契表
constexpr std::array<int, 12> make_fib_table() {
    std::array<int, 12> table{};
    table[0] = 0;
    table[1] = 1;
    for (int i = 2; i < 12; ++i) {
        table[i] = table[i - 1] + table[i - 2];
    }
    return table;
}

// ============================================================
// 5. std::integer_sequence 元编程
// ============================================================

// 利用 integer_sequence 展开并调用函数，将数组转为tuple
template<typename T, std::size_t... Is>
auto array_to_tuple_impl(const std::array<T, sizeof...(Is)>& arr,
                         std::index_sequence<Is...>) {
    return std::make_tuple(arr[Is]...);
}

template<typename T, std::size_t N>
auto array_to_tuple(const std::array<T, N>& arr) {
    return array_to_tuple_impl(arr, std::make_index_sequence<N>{});
}

// 利用 integer_sequence 生成运行时打印
template<std::size_t... Is>
void print_index_sequence(std::index_sequence<Is...>) {
    std::cout << "  序列内容: ";
    ((std::cout << Is << " "), ...);
    std::cout << "\n";
}

// 利用 integer_sequence 在编译期填充数组
template<typename T, std::size_t... Is>
constexpr std::array<T, sizeof...(Is)> fill_array_impl(T value, std::index_sequence<Is...>) {
    return {{(static_cast<void>(Is), value)...}};
}

template<std::size_t N, typename T>
constexpr std::array<T, N> fill_array(T value) {
    return fill_array_impl(value, std::make_index_sequence<N>{});
}

// ============================================================
// 3. decltype(auto) 完美返回
// ============================================================

// decltype(auto) 保留引用语义 —— 完美转发容器元素访问
template<typename Container>
decltype(auto) safe_at(Container& c, std::size_t i) {
    return c.at(i);
}

// decltype(auto) 在条件返回中保留精确类型
template<typename T>
decltype(auto) forward_value(T&& val) {
    return std::forward<T>(val);
}

// ============================================================
// 7. std::exchange 实现移动操作
// ============================================================

class ResourceHolder {
    std::unique_ptr<int[]> data_;
    std::size_t size_;
    std::string name_;
public:
    ResourceHolder() : data_(nullptr), size_(0), name_("empty") {}

    ResourceHolder(const std::string& name, std::size_t n)
        : data_(std::make_unique<int[]>(n)), size_(n), name_(name) {
        for (std::size_t i = 0; i < n; ++i) data_[i] = static_cast<int>(i);
    }

    // 使用 std::exchange 实现移动构造
    ResourceHolder(ResourceHolder&& other) noexcept
        : data_(std::exchange(other.data_, nullptr))
        , size_(std::exchange(other.size_, 0))
        , name_(std::exchange(other.name_, "(已移出)")) {}

    // 使用 std::exchange 实现移动赋值
    ResourceHolder& operator=(ResourceHolder&& other) noexcept {
        if (this != &other) {
            data_ = std::exchange(other.data_, nullptr);
            size_ = std::exchange(other.size_, 0);
            name_ = std::exchange(other.name_, "(已移出)");
        }
        return *this;
    }

    const std::string& name() const { return name_; }
    std::size_t size() const { return size_; }
    int operator[](std::size_t i) const { return data_[i]; }
};

// ============================================================
// 演示函数
// ============================================================

void demo_init_capture() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  初始化捕获 (广义lambda捕获)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // 移动 unique_ptr 到 lambda 中
    auto ptr = std::make_unique<int>(42);
    std::cout << "移动前 ptr 指向: " << (ptr ? std::to_string(*ptr) : "空") << "\n";

    auto take_ownership = [p = std::move(ptr)]() {
        return p ? *p : -1;
    };
    std::cout << "移动后 ptr 指向: " << (ptr ? "非空" : "空") << "\n";
    std::cout << "lambda 内部值: " << take_ownership() << "\n\n";

    // 初始化捕获用于捕获移动语义的容器
    std::vector<int> big_data = {10, 20, 30, 40, 50};
    auto processor = [data = std::move(big_data)]() {
        int sum = 0;
        for (const auto& v : data) sum += v;
        return sum;
    };
    std::cout << "移动vector到lambda中:\n";
    std::cout << "  原vector大小: " << big_data.size() << " (移动后为0)\n";
    std::cout << "  lambda内求和: " << processor() << "\n\n";

    // 初始化捕获用于表达式求值
    auto widget = std::make_shared<std::string>("HelloC++14");
    auto lambda = [wp = std::weak_ptr<std::string>(widget)]() {
        if (auto sp = wp.lock()) {
            return "弱引用有效: " + *sp;
        }
        return std::string("弱引用已失效");
    };
    std::cout << "初始化捕获创建weak_ptr:\n";
    std::cout << "  " << lambda() << "\n";
    widget.reset();
    std::cout << "  释放后: " << lambda() << "\n\n";

    // 初始化捕获用于捕获当前时间戳
    auto timestamp = [t = std::string("2024-01-01")]() {
        return "捕获时间戳: " + t;
    };
    std::cout << "表达式初始化捕获:\n";
    std::cout << "  " << timestamp() << "\n";
}

void demo_generic_lambda_with_algorithms() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  泛型lambda与标准算法\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // sort 使用泛型lambda
    std::vector<int> nums = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    std::cout << "排序前: ";
    for (auto v : nums) std::cout << v << " ";
    std::cout << "\n";

    auto descending = [](const auto& a, const auto& b) { return a > b; };
    std::sort(nums.begin(), nums.end(), descending);
    std::cout << "降序排序: ";
    for (auto v : nums) std::cout << v << " ";
    std::cout << "\n\n";

    // transform 使用泛型lambda
    std::vector<int> src = {1, 2, 3, 4, 5};
    std::vector<double> dst(src.size());
    auto square = [](const auto& x) { return x * x * 0.5; };
    std::transform(src.begin(), src.end(), dst.begin(), square);
    std::cout << "transform (x²×0.5):\n  输入: ";
    for (auto v : src) std::cout << v << " ";
    std::cout << "\n  输出: ";
    for (auto v : dst) std::cout << v << " ";
    std::cout << "\n\n";

    // find_if 使用泛型lambda
    std::vector<std::string> words = {"apple", "banana", "cherry", "date", "elderberry"};
    auto longer_than_5 = [](const auto& s) { return s.size() > 5; };
    auto it = std::find_if(words.begin(), words.end(), longer_than_5);
    std::cout << "find_if (长度>5):\n";
    if (it != words.end()) {
        std::cout << "  找到: \"" << *it << "\"\n";
    }
    auto all_long = std::find_if(words.begin(), words.end(),
        [](const auto& s) { return s.size() > 10; });
    std::cout << "  长度>10: " << (all_long != words.end() ? *all_long : "未找到") << "\n\n";

    // 泛型lambda + remove_if + erase
    std::vector<int> data = {1, -2, 3, -4, 5, -6, 7};
    auto is_negative = [](const auto& x) { return x < 0; };
    data.erase(std::remove_if(data.begin(), data.end(), is_negative), data.end());
    std::cout << "remove_if (移除负数):\n  结果: ";
    for (auto v : data) std::cout << v << " ";
    std::cout << "\n\n";

    // 泛型lambda 作为投影 (C++20风格的前置用法)
    std::vector<std::pair<std::string, int>> scores = {
        {"Alice", 85}, {"Bob", 92}, {"Charlie", 78}, {"Diana", 95}
    };
    std::sort(scores.begin(), scores.end(),
        [](const auto& a, const auto& b) { return a.second > b.second; });
    std::cout << "按分数降序排序:\n";
    for (const auto& [name, score] : scores) {
        std::cout << "  " << name << ": " << score << "\n";
    }
}

void demo_decltype_auto() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  decltype(auto) 完美返回\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // decltype(auto) 保留引用
    std::vector<int> vec = {10, 20, 30, 40, 50};
    decltype(auto) elem = safe_at(vec, 2);
    std::cout << "safe_at(vec, 2) = " << elem << "\n";
    elem = 99;  // 修改的是原容器中的元素
    std::cout << "修改后 vec[2] = " << vec[2] << " (引用语义生效)\n\n";

    // 对比 auto 与 decltype(auto)
    std::cout << "auto vs decltype(auto) 对比:\n";
    int x = 42;
    auto r1 = x;              // int
    decltype(auto) r2 = x;    // int
    decltype(auto) r3 = (x);  // int& —— 保留引用!
    r3 = 100;
    std::cout << "  auto r1 = x;       → 类型为int, r1=" << r1 << "\n";
    std::cout << "  decltype(auto) r2 = x;   → 类型为int, r2=" << r2 << "\n";
    std::cout << "  decltype(auto) r3 = (x); → 类型为int&, 修改后x=" << x << "\n\n";

    // decltype(auto) 在函数返回中保留值类别
    std::cout << "decltype(auto) 在模板中的价值:\n";
    auto val1 = forward_value(42);
    auto val2 = forward_value(std::string("hello"));
    std::cout << "  forward_value(42) → int: " << val1 << "\n";
    std::cout << "  forward_value(string) → string: " << val2 << "\n\n";

    std::cout << "使用场景:\n";
    std::cout << "  1. 函数需要返回引用时(auto会剥离引用)\n";
    std::cout << "  2. 完美转发返回值\n";
    std::cout << "  3. 容器访问器(operator[], at等)\n";
    std::cout << "  ⚠ 注意: 避免返回局部变量的引用!\n";
}

void demo_constexpr_advanced() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  constexpr 高级用法: 编译期计算\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // 编译期字符串哈希
    constexpr auto hash_hello = fnv1a_hash("hello", const_strlen("hello"));
    constexpr auto hash_world = fnv1a_hash("world", const_strlen("world"));
    std::cout << "编译期FNV-1a字符串哈希:\n";
    std::cout << "  hash(\"hello\") = 0x" << std::hex << hash_hello << std::dec << "\n";
    std::cout << "  hash(\"world\") = 0x" << std::hex << hash_world << std::dec << "\n";
    std::cout << "  可用于switch-case字符串匹配\n\n";

    // 编译期查找表
    constexpr auto squares = make_square_table();
    constexpr auto fibs = make_fib_table();
    std::cout << "编译期查找表:\n";
    std::cout << "  平方表: ";
    for (auto v : squares) std::cout << v << " ";
    std::cout << "\n  斐波那契表: ";
    for (auto v : fibs) std::cout << v << " ";
    std::cout << "\n\n";

    // 运行时使用编译期表
    std::cout << "运行时查表 (零开销):\n";
    std::cout << "  squares[7] = " << squares[7] << "\n";
    std::cout << "  fibs[10] = " << fibs[10] << "\n\n";

    std::cout << "C++14 constexpr 改进要点:\n";
    std::cout << "  - 局部变量: 可以在constexpr函数中声明\n";
    std::cout << "  - 循环: for/while 替代递归\n";
    std::cout << "  - 条件: if/else 替代三元运算符\n";
    std::cout << "  - 编译期表生成: 消除运行时初始化开销\n";
}

void demo_integer_sequence() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::integer_sequence 模板元编程\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // 基本用法: 生成索引序列
    std::cout << "1. 生成索引序列:\n";
    print_index_sequence(std::make_index_sequence<8>{});
    std::cout << "\n";

    // array 转 tuple
    std::cout << "2. std::array 转 std::tuple:\n";
    constexpr std::array<int, 5> arr = {10, 20, 30, 40, 50};
    auto tup = array_to_tuple(arr);
    std::cout << "  array: ";
    for (auto v : arr) std::cout << v << " ";
    std::cout << "\n  tuple: ";
    std::cout << std::get<0>(tup) << ", "
              << std::get<1>(tup) << ", "
              << std::get<2>(tup) << ", "
              << std::get<3>(tup) << ", "
              << std::get<4>(tup) << "\n\n";

    // 编译期填充数组
    std::cout << "3. 编译期填充数组:\n";
    constexpr auto filled = fill_array<6>(42);
    std::cout << "  fill_array<6>(42): ";
    for (auto v : filled) std::cout << v << " ";
    std::cout << "\n\n";

    // 利用 integer_sequence 解包 tuple 调用函数
    std::cout << "4. 解包tuple调用函数:\n";
    auto add_three = [](int a, int b, int c) { return a + b + c; };
    auto args = std::make_tuple(1, 2, 3);
    auto call_with_tuple = [&add_three](const auto& t) {
        return std::apply(add_three, t);
    };
    std::cout << "  add_three(1,2,3) via tuple = " << call_with_tuple(args) << "\n\n";

    std::cout << "integer_sequence 核心用途:\n";
    std::cout << "  - 编译期生成整数序列\n";
    std::cout << "  - 参数包展开的辅助工具\n";
    std::cout << "  - tuple/array 之间的类型转换\n";
    std::cout << "  - std::make_index_sequence<N> 最常用\n";
}

void demo_deprecated_attribute() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  [[deprecated]] 属性\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "[[deprecated]] 标记弃用API:\n\n";

    // 调用弃用函数 (编译器会产生警告)
    std::cout << "compute_old(5) = " << compute_old(5) << " (已弃用)\n";
    std::cout << "compute_v2(5) = " << compute_v2(5) << " (推荐使用)\n\n";

    // 弃用类
    struct [[deprecated("请使用 NewHandler")]] OldHandler {
        static void handle() {}
    };

    struct NewHandler {
        static void handle() {}
    };

    std::cout << "[[deprecated]] 用法:\n";
    std::cout << "  [[deprecated]]                    —— 无原因\n";
    std::cout << "  [[deprecated(\"请使用新API\")]]       —— 带原因\n";
    std::cout << "  可用于: 函数、类、变量、typedef、枚举等\n\n";

    std::cout << "弃用变量示例:\n";
    [[deprecated]] const int OLD_BUFFER_SIZE = 1024;
    constexpr int NEW_BUFFER_SIZE = 4096;
    std::cout << "  OLD_BUFFER_SIZE = " << OLD_BUFFER_SIZE << " (已弃用)\n";
    std::cout << "  NEW_BUFFER_SIZE = " << NEW_BUFFER_SIZE << " (推荐)\n\n";

    std::cout << "最佳实践:\n";
    std::cout << "  1. 始终提供弃用原因，方便迁移\n";
    std::cout << "  2. 保留弃用API至少一个大版本\n";
    std::cout << "  3. 弃用 ≠ 删除，只是发出警告\n";
    std::cout << "  4. 配合文档说明替代方案\n";
}

void demo_std_exchange() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::exchange 实现移动操作\n";
    std::cout << "═══════════════════════════════════════\n\n";

    // 基本用法
    std::cout << "1. 基本用法 —— 替换并返回旧值:\n";
    int value = 42;
    int old_value = std::exchange(value, 100);
    std::cout << "  旧值: " << old_value << ", 新值: " << value << "\n\n";

    // 与 std::swap 对比
    std::cout << "2. std::exchange vs std::swap:\n";
    int a = 10, b = 20;
    std::cout << "  交换前: a=" << a << ", b=" << b << "\n";
    a = std::exchange(b, a);  // 等价于 swap
    std::cout << "  exchange后: a=" << a << ", b=" << b << "\n\n";

    // 实现移动语义
    std::cout << "3. 用 std::exchange 实现移动操作:\n";
    ResourceHolder h1("资源A", 5);
    std::cout << "  原始: name=\"" << h1.name() << "\", size=" << h1.size() << "\n";
    std::cout << "  数据: ";
    for (std::size_t i = 0; i < h1.size(); ++i) std::cout << h1[i] << " ";
    std::cout << "\n";

    ResourceHolder h2 = std::move(h1);
    std::cout << "  移动后 h1: name=\"" << h1.name() << "\", size=" << h1.size() << "\n";
    std::cout << "  移动后 h2: name=\"" << h2.name() << "\", size=" << h2.size() << "\n";
    std::cout << "  h2数据: ";
    for (std::size_t i = 0; i < h2.size(); ++i) std::cout << h2[i] << " ";
    std::cout << "\n\n";

    // exchange 用于原子操作模式
    std::cout << "4. std::exchange 的优势:\n";
    std::cout << "  - 原子性: 一步完成替换和获取旧值\n";
    std::cout << "  - 简洁: 移动构造/赋值只需一行\n";
    std::cout << "  - 安全: 旧值被新值替换，源对象处于有效状态\n\n";

    std::cout << "移动构造中使用 exchange 的模式:\n";
    std::cout << "  ResourceHolder(ResourceHolder&& o) noexcept\n";
    std::cout << "    : data_(std::exchange(o.data_, nullptr))\n";
    std::cout << "    , size_(std::exchange(o.size_, 0))\n";
    std::cout << "    , name_(std::exchange(o.name_, \"(已移出)\")) {}\n";
}

int main() {
    demo_init_capture();
    demo_generic_lambda_with_algorithms();
    demo_decltype_auto();
    demo_constexpr_advanced();
    demo_integer_sequence();
    demo_deprecated_attribute();
    demo_std_exchange();
    return 0;
}
