/**
 * @file 02_example_cpp23_advanced.cpp
 * @brief C++23高级用法模式示例
 * @description 对应文档: 02-CPP/28-cpp23, 演示C++23高级特性的实际应用模式
 * 编译命令: g++ -std=c++23 -o 02_example_cpp23_advanced 02_example_cpp23_advanced.cpp
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <optional>
#include <variant>
#include <stdexcept>
#include <functional>
#include <cstdint>
#include <array>
#include <algorithm>
#include <type_traits>

// std::print 需要单独头文件, GCC 尚未完整支持链接, 暂时禁用
#if defined(__cpp_lib_print) && __cpp_lib_print >= 202207L && !defined(__GNUC__)
#include <print>
#endif

// ============================================================
// 特性检测与回退实现
// ============================================================

// std::expected 回退实现
#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202211L
#include <expected>
using std::expected;
using std::unexpected;
#else
template<typename T, typename E>
class expected {
    std::variant<T, E> data_;
    bool has_value_;
public:
    expected(T val) : data_(std::move(val)), has_value_(true) {}
    expected(E err, bool) : data_(std::move(err)), has_value_(false) {}
    expected(const expected&) = default;
    expected(expected&&) = default;
    expected& operator=(const expected&) = default;
    expected& operator=(expected&&) = default;

    bool has_value() const { return has_value_; }
    T& value() & { if (!has_value_) throw std::runtime_error("bad expected access"); return std::get<0>(data_); }
    const T& value() const& { if (!has_value_) throw std::runtime_error("bad expected access"); return std::get<0>(data_); }
    E& error() & { if (has_value_) throw std::runtime_error("bad expected access"); return std::get<1>(data_); }
    const E& error() const& { if (has_value_) throw std::runtime_error("bad expected access"); return std::get<1>(data_); }
    T value_or(T default_val) const { return has_value_ ? value() : default_val; }
    T& operator*() { return value(); }
    const T& operator*() const { return value(); }
    T* operator->() { return &value(); }
    const T* operator->() const { return &value(); }
    explicit operator bool() const { return has_value_; }
};

template<typename E>
class unexpected {
    E error_;
public:
    explicit unexpected(E err) : error_(std::move(err)) {}
    const E& error() const { return error_; }
};
#endif

// std::unreachable 回退
#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
#include <utility>
// std::unreachable 可用
#else
[[noreturn]] inline void unreachable_impl() {
    std::terminate();
}
#endif

// Ranges 相关头文件
#if __has_include(<ranges>)
#include <ranges>
#endif

// ============================================================
// 1. std::expected - 无异常的错误处理高级模式
// ============================================================

struct ErrorCode {
    int code;
    std::string message;
};

// 模拟数据库查询
expected<std::string, ErrorCode> query_user(int user_id) {
    static std::map<int, std::string> users = {
        {1, "张三"}, {2, "李四"}, {3, "王五"}
    };
    auto it = users.find(user_id);
    if (it == users.end()) {
        return expected<std::string, ErrorCode>(ErrorCode{404, "用户不存在: ID=" + std::to_string(user_id)}, false);
    }
    return expected<std::string, ErrorCode>(it->second);
}

// 模拟权限检查
expected<bool, ErrorCode> check_permission(const std::string& user) {
    static std::vector<std::string> admins = {"张三", "王五"};
    bool has_perm = std::find(admins.begin(), admins.end(), user) != admins.end();
    if (!has_perm) {
        return expected<bool, ErrorCode>(ErrorCode{403, "权限不足: " + user}, false);
    }
    return expected<bool, ErrorCode>(true);
}

// 模拟数据获取
expected<std::string, ErrorCode> fetch_data(bool has_perm, const std::string& user) {
    if (!has_perm) {
        return expected<std::string, ErrorCode>(ErrorCode{403, "无法获取数据"}, false);
    }
    return expected<std::string, ErrorCode>(user + " 的机密数据");
}

void demo_expected_advanced() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::expected - 无异常错误处理高级模式\n";
    std::cout << "═══════════════════════════════════════\n";

    // 场景1: 成功路径
    std::cout << "\n[场景1] 查询存在的用户(ID=1):\n";
    auto user1 = query_user(1);
    if (user1.has_value()) {
        std::cout << "  用户名: " << user1.value() << "\n";
        auto perm1 = check_permission(user1.value());
        if (perm1.has_value() && perm1.value()) {
            auto data1 = fetch_data(perm1.value(), user1.value());
            if (data1.has_value()) {
                std::cout << "  数据: " << data1.value() << "\n";
            }
        }
    }

    // 场景2: 用户不存在
    std::cout << "\n[场景2] 查询不存在的用户(ID=99):\n";
    auto user2 = query_user(99);
    if (!user2.has_value()) {
        std::cout << "  错误: code=" << user2.error().code
                  << ", message=" << user2.error().message << "\n";
    }

    // 场景3: 用户存在但权限不足
    std::cout << "\n[场景3] 权限不足的用户(ID=2, 李四):\n";
    auto user3 = query_user(2);
    if (user3.has_value()) {
        std::cout << "  用户名: " << user3.value() << "\n";
        auto perm3 = check_permission(user3.value());
        if (!perm3.has_value()) {
            std::cout << "  错误: code=" << perm3.error().code
                      << ", message=" << perm3.error().message << "\n";
        }
    }

    std::cout << "\n举一反三 - expected高级用法:\n";
    std::cout << "  1. and_then: 链式操作, 成功时继续, 失败时短路\n";
    std::cout << "  2. or_else:  错误恢复, 失败时提供默认值\n";
    std::cout << "  3. transform: 值变换, 成功时转换值类型\n";
    std::cout << "  4. transform_error: 错误变换, 统一错误格式\n";
    std::cout << R"(
  // C++23 标准库写法(需要编译器支持):
  auto result = query_user(1)
      .and_then([](const std::string& name) { return check_permission(name); })
      .and_then([](bool perm) { return fetch_data(perm, "admin"); })
      .transform([](const std::string& data) { return "结果: " + data; })
      .or_else([](ErrorCode e) -> expected<std::string, ErrorCode> {
          std::cout << "恢复错误: " << e.message << "\n";
          return "默认数据";
      });
)" << "\n";
}

// ============================================================
// 2. std::print / std::println - 格式化输出
// ============================================================

void demo_std_print_advanced() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::print / std::println - 格式化输出\n";
    std::cout << "═══════════════════════════════════════\n";

#if defined(__cpp_lib_print) && __cpp_lib_print >= 202207L && !defined(__GNUC__)
    std::print("std::print 可用! 你好, {}!\n", "C++23");
    std::println("std::println 自动换行");
    std::println("格式化: 整数={:06d}, 浮点={:.2f}, 十六进制={:x}", 42, 3.14159, 255);
#else
    std::cout << "std::print 不可用(需要C++23编译器支持), 概念演示:\n\n";

    std::cout << "基本用法:\n";
    std::cout << "  std::print(\"你好, {}!\\n\", \"C++23\");\n";
    std::cout << "  std::println(\"自动换行\");\n";

    std::cout << "\n格式化输出:\n";
    std::cout << "  std::println(\"整数={:06d}\", 42);        // 000042\n";
    std::cout << "  std::println(\"浮点={:.2f}\", 3.14159);   // 3.14\n";
    std::cout << "  std::println(\"十六进制={:x}\", 255);      // ff\n";
    std::cout << "  std::println(\"二进制={:b}\", 10);         // 1010\n";

    std::cout << "\n对齐与宽度:\n";
    std::cout << "  std::println(\"{:>10}\", 42);    // 右对齐宽10:        42\n";
    std::cout << "  std::println(\"{:<10}\", 42);    // 左对齐宽10: 42       \n";
    std::cout << "  std::println(\"{:^10}\", 42);    // 居中宽10:     42   \n";
    std::cout << "  std::println(\"{:*^10}\", 42);   // 填充*: ****42****\n";

    std::cout << "\n输出到stderr:\n";
    std::cout << "  std::print(stderr, \"错误: {}\\n\", \"something\");\n";
    std::cout << "  std::println(stderr, \"警告: {}\", \"low memory\");\n";

    // 用现有方式模拟输出效果
    std::cout << "\n模拟输出效果:\n";
    std::cout << "  你好, C++23!\n";
    std::cout << "  整数=000042, 浮点=3.14, 十六进制=ff\n";
#endif

    std::cout << "\n举一反三 - std::print vs printf vs iostream:\n";
    std::cout << "  printf:   快但不类型安全, 无扩展性\n";
    std::cout << "  iostream: 类型安全但冗长, 性能差\n";
    std::cout << "  std::print: 类型安全 + 高性能 + 格式化\n";
    std::cout << "  std::print 基于 fmt 库, 是 fmt 的标准化\n";
}

// ============================================================
// 3. std::flat_map / std::flat_set - 扁平关联容器
// ============================================================

void demo_flat_containers() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::flat_map / std::flat_set - 扁平关联容器\n";
    std::cout << "═══════════════════════════════════════\n";

#if defined(__cpp_lib_flat_map) && __cpp_lib_flat_map >= 202207L
    std::cout << "std::flat_map 可用!\n";
    std::flat_map<std::string, int> scores;
    scores["语文"] = 92;
    scores["数学"] = 98;
    scores["英语"] = 85;
    for (const auto& [k, v] : scores) {
        std::println("  {}: {}", k, v);
    }
#else
    std::cout << "std::flat_map/std::flat_set 尚不可用, 概念演示:\n\n";

    std::cout << "设计理念:\n";
    std::cout << "  传统map/set: 基于红黑树, 节点分散在堆上\n";
    std::cout << "  flat_map/set: 基于连续数组, 数据紧凑排列\n";

    std::cout << "\n适用场景:\n";
    std::cout << "  1. 数据量小(< 100个元素): 缓存友好, 遍历快\n";
    std::cout << "  2. 只读场景: 构建后不修改, 查询为主\n";
    std::cout << "  3. 内存敏感: 无节点开销, 内存占用更少\n";

    std::cout << "\n不适用场景:\n";
    std::cout << "  1. 频繁插入/删除: 数组移动元素开销大\n";
    std::cout << "  2. 大数据集: 插入O(n), 而非树结构O(log n)\n";

    std::cout << "\n用法示例:\n";
    std::cout << R"(  #include <flat_map>
  #include <flat_set>

  std::flat_map<std::string, int> scores;
  scores["语文"] = 92;
  scores["数学"] = 98;
  scores["英语"] = 85;

  std::flat_set<int> ids = {3, 1, 4, 1, 5};  // {1, 3, 4, 5}
)" << "\n";

    // 用 vector + sort 模拟 flat_map 的行为
    std::cout << "用vector模拟flat_map行为:\n";
    std::vector<std::pair<std::string, int>> flat_scores = {
        {"数学", 98}, {"语文", 92}, {"英语", 85}
    };
    std::sort(flat_scores.begin(), flat_scores.end());
    for (const auto& [k, v] : flat_scores) {
        std::cout << "  " << k << ": " << v << "\n";
    }
    std::cout << "  (数据按key排序存储在连续内存中)\n";
#endif
}

// ============================================================
// 4. Deducing this - 显式对象参数
// ============================================================

void demo_deducing_this() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  Deducing this - 显式对象参数\n";
    std::cout << "═══════════════════════════════════════\n";

    std::cout << "C++23核心变化: 成员函数可以显式声明this参数\n\n";

    // 模式1: 消除const/non-const重复
    std::cout << "[模式1] 消除const/non-const重复:\n";
    std::cout << R"(  // C++20: 需要写两个版本
  class Config {
      std::string value_;
  public:
      std::string& get() { return value_; }             // 非const版本
      const std::string& get() const { return value_; } // const版本
  };

  // C++23: 只需一个版本
  class Config {
      std::string value_;
  public:
      auto&& get(this auto&& self) {
          return std::forward<decltype(self)>(self).value_;
      }
  };
)" << "\n";

    // 模式2: 递归Lambda
    std::cout << "[模式2] 递归Lambda(无需std::function):\n";
    std::cout << R"(  // C++20: 需要std::function或泛型lambda hack
  std::function<int(int)> fib = [&fib](int n) -> int {
      return n <= 1 ? n : fib(n-1) + fib(n-2);
  };

  // C++23: 显式this参数, 无额外开销
  auto fibonacci = [](this auto self, int n) -> int {
      return n <= 1 ? n : self(n-1) + self(n-2);
  };
  // fibonacci(10) = 55
)" << "\n";

    // 模式3: 完美转发成员函数
    std::cout << "[模式3] 完美转发成员函数:\n";
    std::cout << R"(  class Builder {
      std::string name_;
      int count_ = 0;
  public:
      template<typename Self, typename S>
      auto&& with_name(this Self&& self, S&& name) {
          std::forward<Self>(self).name_ = std::forward<S>(name);
          return std::forward<Self>(self);
      }
      template<typename Self>
      auto&& with_count(this Self&& self, int c) {
          std::forward<Self>(self).count_ = c;
          return std::forward<Self>(self);
      }
  };
  // Builder b;
  // b.with_name("test").with_count(42);
)" << "\n";

    // 模式4: CRTP简化
    std::cout << "[模式4] CRTP模式简化:\n";
    std::cout << R"(  // C++20 CRTP: 需要基类模板
  template<typename Derived>
  class Base {
  public:
      void interface() { static_cast<Derived*>(this)->impl(); }
  };
  class MyDerived : public Base<MyDerived> {
  public:
      void impl() { /* ... */ }
  };

  // C++23: 无需CRTP, 直接用deducing this
  class NewBase {
  public:
      template<typename Self>
      void interface(this Self&& self) {
          std::forward<Self>(self).impl();
      }
  };
)" << "\n";

    std::cout << "举一反三:\n";
    std::cout << "  deducing this 是C++23最重要的语言特性之一\n";
    std::cout << "  它统一了值类别, 消除了大量重复代码\n";
    std::cout << "  与模板结合时特别强大\n";
}

// ============================================================
// 5. 多维下标运算符
// ============================================================

void demo_multidimensional_subscript() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  多维下标运算符 operator[](size_t, size_t)\n";
    std::cout << "═══════════════════════════════════════\n";

    std::cout << "C++23允许operator[]接受多个参数:\n\n";

    // 使用传统 operator() 方式实现(兼容所有标准)
    class Matrix3x3 {
        std::array<std::array<int, 3>, 3> data_{};
    public:
        // C++20写法: operator()
        int& operator()(size_t i, size_t j) { return data_[i][j]; }
        const int& operator()(size_t i, size_t j) const { return data_[i][j]; }

#if __cplusplus >= 202302L
        // C++23写法: operator[] 多参数
        int& operator[](size_t i, size_t j) { return data_[i][j]; }
        const int& operator[](size_t i, size_t j) const { return data_[i][j]; }
#endif

        void fill_diagonal(int val) {
            for (size_t i = 0; i < 3; ++i) data_[i][i] = val;
        }

        void print() const {
            for (size_t i = 0; i < 3; ++i) {
                std::cout << "  [";
                for (size_t j = 0; j < 3; ++j) {
                    std::cout << (*this)(i, j);
                    if (j < 2) std::cout << ", ";
                }
                std::cout << "]\n";
            }
        }
    };

    Matrix3x3 mat;
    mat(0, 0) = 1; mat(1, 1) = 5; mat(2, 2) = 9;
    mat(0, 1) = 2; mat(1, 2) = 6;

    std::cout << "矩阵内容(operator()语法):\n";
    mat.print();

#if __cplusplus >= 202302L
    std::cout << "\nC++23语法: mat[0,1] = " << mat[0, 1] << "\n";
#else
    std::cout << "\nC++23语法(当前编译器不支持): mat[0,1]\n";
    std::cout << "当前使用C++20语法: mat(0,1) = " << mat(0, 1) << "\n";
#endif

    std::cout << "\n语法对比:\n";
    std::cout << "  C++20: mat(i, j)    // 函数调用语法\n";
    std::cout << "  C++20: mat[(i,j)]   // 逗号运算符hack(危险!)\n";
    std::cout << "  C++23: mat[i, j]    // 多维下标运算符(正确)\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  1. 数学库: 矩阵/张量访问更直观\n";
    std::cout << "  2. 图像处理: pixel[y, x] 或 pixel[y, x, c]\n";
    std::cout << "  3. 多维数组: arr[i, j, k] 替代 arr[i][j][k]\n";
    std::cout << "  4. 注意: 逗号运算符hack mat[(i,j)] 等价于 mat[j], 这是bug!\n";
}

// ============================================================
// 6. std::unreachable() - 标记不可达代码
// ============================================================

enum class Color { Red, Green, Blue };
enum class Direction { Up, Down, Left, Right };

void demo_std_unreachable() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::unreachable() - 标记不可达代码\n";
    std::cout << "═══════════════════════════════════════\n";

    auto color_name = [](Color c) -> std::string {
        switch (c) {
            case Color::Red:   return "红色";
            case Color::Green: return "绿色";
            case Color::Blue:  return "蓝色";
        }
#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
        std::unreachable(); // 告诉编译器: 这里永远不会执行到
#else
        // 回退: 使用terminate或__builtin_unreachable
        std::terminate();
#endif
    };

    auto direction_name = [](Direction d) -> std::string {
        switch (d) {
            case Direction::Up:    return "上";
            case Direction::Down:  return "下";
            case Direction::Left:  return "左";
            case Direction::Right: return "右";
        }
#if defined(__cpp_lib_unreachable) && __cpp_lib_unreachable >= 202202L
        std::unreachable();
#else
        std::terminate();
#endif
    };

    std::cout << "Color::Red = " << color_name(Color::Red) << "\n";
    std::cout << "Color::Blue = " << color_name(Color::Blue) << "\n";
    std::cout << "Direction::Up = " << direction_name(Direction::Up) << "\n";
    std::cout << "Direction::Right = " << direction_name(Direction::Right) << "\n";

    std::cout << "\nstd::unreachable() 的作用:\n";
    std::cout << "  1. 编译器优化提示: 不需要生成处理不可达路径的代码\n";
    std::cout << "  2. 消除警告: 抑制'并非所有路径返回值'的警告\n";
    std::cout << "  3. 文档意义: 明确表达'这里不应该到达'\n";
    std::cout << "  4. 如果真的执行到: 未定义行为(UB)\n";

    std::cout << "\n与替代方案对比:\n";
    std::cout << "  std::unreachable():  标准化, 可移植, 自文档\n";
    std::cout << "  __builtin_unreachable(): GCC/Clang扩展, 不可移植\n";
    std::cout << "  std::terminate():    安全但无优化, 有运行时开销\n";
    std::cout << "  assert(false):       Debug检查, Release中被移除\n";

    std::cout << "\n典型使用场景:\n";
    std::cout << "  1. switch的default分支(已穷举所有枚举值)\n";
    std::cout << "  2. 不可能执行到的else分支\n";
    std::cout << "  3. 类型擦除后的已知类型恢复\n";
}

// ============================================================
// 7. std::views::zip / std::views::enumerate - Ranges新增
// ============================================================

void demo_ranges_additions() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  Ranges新增: zip_view / enumerate\n";
    std::cout << "═══════════════════════════════════════\n";

    std::vector<std::string> names = {"张三", "李四", "王五"};
    std::vector<int> scores = {92, 85, 98};
    std::vector<char> grades = {'A', 'B', 'A'};

    std::cout << "数据准备:\n";
    std::cout << "  names  = {\"张三\", \"李四\", \"王五\"}\n";
    std::cout << "  scores = {92, 85, 98}\n";
    std::cout << "  grades = {'A', 'B', 'A'}\n";

#if defined(__cpp_lib_ranges_zip) && __cpp_lib_ranges_zip >= 202110L
    std::cout << "\nstd::views::zip 可用!\n";
    std::cout << "zip结果:\n";
    for (auto&& [name, score, grade] : std::views::zip(names, scores, grades)) {
        std::cout << "  " << name << ": " << score << "分 (" << grade << ")\n";
    }
#else
    std::cout << "\nstd::views::zip 不可用, 手动模拟:\n";
    for (size_t i = 0; i < names.size(); ++i) {
        std::cout << "  " << names[i] << ": " << scores[i] << "分 (" << grades[i] << ")\n";
    }
#endif

#if defined(__cpp_lib_ranges_enumerate) && __cpp_lib_ranges_enumerate >= 202302L
    std::cout << "\nstd::views::enumerate 可用!\n";
    std::cout << "enumerate结果:\n";
    for (auto&& [idx, name] : std::views::enumerate(names)) {
        std::cout << "  [" << idx << "] " << name << "\n";
    }
#else
    std::cout << "\nstd::views::enumerate 不可用, 手动模拟:\n";
    for (size_t i = 0; i < names.size(); ++i) {
        std::cout << "  [" << i << "] " << names[i] << "\n";
    }
#endif

    std::cout << "\nC++23 Ranges新增视图一览:\n";
    std::cout << "  1. views::zip:       并行遍历多个范围\n";
    std::cout << "  2. views::enumerate: 带索引遍历(类似Python的enumerate)\n";
    std::cout << "  3. views::chunk:     按固定大小分块\n";
    std::cout << "  4. views::slide:     滑动窗口\n";
    std::cout << "  5. views::join_with: 用分隔符连接\n";
    std::cout << "  6. views::chunk_by:  按条件分块\n";
    std::cout << "  7. views::repeat:    重复元素\n";
    std::cout << "  8. views::stride:    按步长跳跃\n";

    std::cout << "\n用法示例:\n";
    std::cout << R"(  // zip: 并行遍历
  for (auto&& [name, score] : std::views::zip(names, scores)) { ... }

  // enumerate: 带索引遍历
  for (auto&& [i, name] : std::views::enumerate(names)) { ... }

  // chunk: 分块
  for (auto chunk : std::views::chunk(data, 3)) { ... }

  // slide: 滑动窗口
  for (auto window : std::views::slide(data, 2)) { ... }
)" << "\n";
}

// ============================================================
// 8. if consteval - 编译期条件判断
// ============================================================

constexpr int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

constexpr int smart_fibonacci(int n) {
#if __cplusplus >= 202302L
    // C++23: if consteval
    if consteval {
        // 编译期: 使用递归(编译期无性能问题)
        return n <= 1 ? n : smart_fibonacci(n - 1) + smart_fibonacci(n - 2);
    } else {
        // 运行时: 使用迭代(避免栈溢出)
#endif
        if (n <= 1) return n;
        int a = 0, b = 1;
        for (int i = 2; i <= n; ++i) {
            int tmp = a + b;
            a = b;
            b = tmp;
        }
        return b;
#if __cplusplus >= 202302L
    }
#endif
}

void demo_if_consteval() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  if consteval - 编译期条件判断\n";
    std::cout << "═══════════════════════════════════════\n";

    // 编译期计算
    constexpr int fib10 = fibonacci(10);
    std::cout << "编译期 fibonacci(10) = " << fib10 << "\n";

    // 运行时计算
    int n = 15;
    int fib_n = smart_fibonacci(n);
    std::cout << "运行时 smart_fibonacci(15) = " << fib_n << "\n";

    std::cout << "\nif consteval vs if constexpr(is_constant_evaluated()):\n";
    std::cout << R"(  // C++20写法(容易误用):
  constexpr int compute(int n) {
      if constexpr (std::is_constant_evaluated()) {  // 注意: 总是true!
          return n * n;  // 这行总是会编译, 即使在运行时
      }
      return n + n;
  }

  // C++23写法(清晰正确):
  constexpr int compute(int n) {
      if consteval {
          return n * n;   // 仅在编译期执行
      } else {
          return n + n;   // 仅在运行时执行
      }
  }
)" << "\n";

    std::cout << "关键区别:\n";
    std::cout << "  if constexpr(std::is_constant_evaluated()):\n";
    std::cout << "    - is_constant_evaluated()是运行时函数\n";
    std::cout << "    - if constexpr在编译期求值, 结果总是true\n";
    std::cout << "    - 正确写法: if (std::is_constant_evaluated())\n";
    std::cout << "  if consteval:\n";
    std::cout << "    - 语法清晰, 不可能误用\n";
    std::cout << "    - 直接判断当前是否在编译期求值\n";
    std::cout << "    - 是C++23推荐的方式\n";

    std::cout << "\n典型应用场景:\n";
    std::cout << "  1. 编译期用递归, 运行时用迭代(如上例)\n";
    std::cout << "  2. 编译期用简单算法, 运行时用优化算法\n";
    std::cout << "  3. 编译期避免使用运行时设施(如malloc)\n";
}

// ============================================================
// 9. std::stacktrace - 堆栈跟踪
// ============================================================

void demo_std_stacktrace() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::stacktrace - 堆栈跟踪\n";
    std::cout << "═══════════════════════════════════════\n";

#if defined(__cpp_lib_stacktrace) && __cpp_lib_stacktrace >= 202011L
    #include <stacktrace>
    std::cout << "std::stacktrace 可用!\n\n";

    auto trace = std::stacktrace::current();
    for (const auto& entry : trace) {
        std::cout << "  " << entry << "\n";
    }
#else
    std::cout << "std::stacktrace 尚不可用, 概念演示:\n\n";

    std::cout << "基本用法:\n";
    std::cout << R"(  #include <stacktrace>

  void func_c() {
      auto trace = std::stacktrace::current();
      for (const auto& entry : trace) {
          std::cout << entry << "\n";
      }
  }

  void func_b() { func_c(); }
  void func_a() { func_b(); }
  // 输出:
  //   func_c() at main.cpp:5
  //   func_b() at main.cpp:10
  //   func_a() at main.cpp:11
  //   main()   at main.cpp:15
)" << "\n";

    std::cout << "关键API:\n";
    std::cout << "  std::stacktrace::current()       // 获取当前调用栈\n";
    std::cout << "  std::basic_stacktrace::current()  // 同上(完整类型)\n";
    std::cout << "  entry.description()               // 函数名+地址\n";
    std::cout << "  entry.source_file()               // 源文件路径\n";
    std::cout << "  entry.source_line()               // 源代码行号\n";

    std::cout << "\n典型应用场景:\n";
    std::cout << "  1. 错误日志: 记录异常发生时的调用栈\n";
    std::cout << "  2. 断言增强: assert失败时打印调用栈\n";
    std::cout << "  3. 调试辅助: 无需GDB即可查看调用关系\n";
    std::cout << "  4. 性能分析: 标记热点函数的调用路径\n";

    std::cout << "\n注意事项:\n";
    std::cout << "  1. 需要编译器生成调试信息(-g)\n";
    std::cout << "  2. 部分平台需要链接额外库(-lstdc++_exp)\n";
    std::cout << "  3. 获取堆栈有一定运行时开销\n";
    std::cout << "  4. Release构建可能丢失函数名信息\n";
#endif
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "========== C++23 高级用法模式示例 ==========\n";
    std::cout << "注意: 部分特性可能需要较新编译器支持\n";
    std::cout << "编译标准: C++23 (g++ -std=c++23)\n";

    demo_expected_advanced();
    demo_std_print_advanced();
    demo_flat_containers();
    demo_deducing_this();
    demo_multidimensional_subscript();
    demo_std_unreachable();
    demo_ranges_additions();
    demo_if_consteval();
    demo_std_stacktrace();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
