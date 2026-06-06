/** @file 02_deep_dive_lambda_patterns.cpp
 *  @brief Lambda模式：延迟执行、定制点、RAII、立即调用、递归lambda
 *  @description 对应文档: 13-Lambda与函数对象 | 举一反三：掌握Lambda的设计模式
 */

#include <iostream>
#include <string>
#include <vector>
#include <functional>
#include <memory>
#include <algorithm>

template<typename T, typename Comparator = std::function<bool(const T&, const T&)>>
class SortedVector {
public:
    SortedVector(Comparator comp = std::less<T>{}) : comp_(std::move(comp)) {}

    void insert(const T& value) {
        auto pos = std::lower_bound(data_.begin(), data_.end(), value, comp_);
        data_.insert(pos, value);
    }

    void print() const {
        for (const auto& v : data_) std::cout << v << " ";
        std::cout << "\n";
    }
private:
    std::vector<T> data_;
    Comparator comp_;
};

void demo_deferred_execution() {
    std::cout << "=== Lambda 延迟执行 ===\n";

    class LazyValue {
    public:
        using Factory = std::function<int()>;

        LazyValue(Factory factory) : factory_(std::move(factory)), computed_(false) {}

        int value() {
            if (!computed_) {
                cached_ = factory_();
                computed_ = true;
            }
            return cached_;
        }
    private:
        Factory factory_;
        int cached_ = 0;
        bool computed_;
    };

    int call_count = 0;
    LazyValue lazy([&call_count]() {
        ++call_count;
        std::cout << "  计算中... (第" << call_count << "次调用)\n";
        return 42;
    });

    std::cout << "创建 LazyValue, 尚未计算\n";
    std::cout << "第一次获取: " << lazy.value() << "\n";
    std::cout << "第二次获取: " << lazy.value() << " (使用缓存)\n";

    std::cout << "\n延迟执行的应用:\n";
    std::cout << "  1. 懒加载/懒计算\n";
    std::cout << "  2. 条件执行 (只在需要时计算)\n";
    std::cout << "  3. 日志: 只在日志级别启用时格式化消息\n";

    std::cout << "\n";
}

void demo_customization_points() {
    std::cout << "=== Lambda 作为定制点 ===\n";

    SortedVector<int> asc;
    asc.insert(5); asc.insert(1); asc.insert(3); asc.insert(2);
    std::cout << "升序: "; asc.print();

    SortedVector<int, std::function<bool(const int&, const int&)>> desc(
        [](int a, int b) { return a > b; }
    );
    desc.insert(5); desc.insert(1); desc.insert(3); desc.insert(2);
    std::cout << "降序: "; desc.print();

    std::cout << "\n定制点模式:\n";
    std::cout << "  1. 通过 lambda 定制算法行为\n";
    std::cout << "  2. 比继承更灵活\n";
    std::cout << "  3. 比策略模式更简洁\n";

    std::cout << "\n";
}

void demo_lambda_and_raii() {
    std::cout << "=== Lambda 与 RAII ===\n";

    class ScopeGuard {
    public:
        ScopeGuard(std::function<void()> cleanup) : cleanup_(std::move(cleanup)), active_(true) {}
        ~ScopeGuard() { if (active_) cleanup_(); }
        void dismiss() { active_ = false; }
        ScopeGuard(ScopeGuard&& other) noexcept : cleanup_(std::move(other.cleanup_)), active_(other.active_) {
            other.active_ = false;
        }
        ScopeGuard(const ScopeGuard&) = delete;
        ScopeGuard& operator=(const ScopeGuard&) = delete;
    private:
        std::function<void()> cleanup_;
        bool active_;
    };

    {
        std::cout << "进入作用域\n";
        ScopeGuard guard([]() {
            std::cout << "作用域退出时自动清理!\n";
        });
        std::cout << "执行一些操作...\n";
    }

    std::cout << "\n--- 条件性清理 ---\n";
    {
        bool success = true;
        ScopeGuard rollback([&success]() {
            if (!success) {
                std::cout << "操作失败, 执行回滚\n";
            } else {
                std::cout << "操作成功, 无需回滚\n";
            }
        });
        std::cout << "执行操作...\n";
        success = true;
    }

    std::cout << "\n--- 多个清理动作 ---\n";
    {
        std::vector<ScopeGuard> guards;
        guards.emplace_back([]() { std::cout << "清理1: 释放资源A\n"; });
        guards.emplace_back([]() { std::cout << "清理2: 释放资源B\n"; });
        guards.emplace_back([]() { std::cout << "清理3: 释放资源C\n"; });
        std::cout << "执行操作...\n";
    }

    std::cout << "\n";
}

void demo_immediately_invoked_lambda() {
    std::cout << "=== 立即调用的 Lambda (IIFE) ===\n";

    const auto config = []() {
        struct Config {
            std::string host = "localhost";
            int port = 8080;
            bool verbose = false;
        };
        Config c;
        c.host = "192.168.1.1";
        c.port = 9090;
        c.verbose = true;
        return c;
    }();

    std::cout << "配置: host=" << config.host << ", port=" << config.port
              << ", verbose=" << (config.verbose ? "true" : "false") << "\n\n";

    std::cout << "--- 复杂初始化 ---\n";
    const auto fib10 = [n = 10]() {
        std::vector<long long> fib(n);
        fib[0] = 0;
        fib[1] = 1;
        for (int i = 2; i < n; ++i) {
            fib[i] = fib[i-1] + fib[i-2];
        }
        return fib;
    }();

    std::cout << "斐波那契前10项: ";
    for (auto v : fib10) std::cout << v << " ";
    std::cout << "\n\n";

    std::cout << "--- const 变量复杂初始化 ---\n";
    const auto result = [](int x) {
        int temp = x * x;
        temp += 10;
        temp *= 2;
        return temp;
    }(5);
    std::cout << "计算结果: " << result << "\n\n";

    std::cout << "IIFE 的优势:\n";
    std::cout << "  1. const 变量的复杂初始化\n";
    std::cout << "  2. 避免变量先声明再赋值\n";
    std::cout << "  3. 局部作用域, 不污染外部\n";
    std::cout << "  4. 替代静态初始化块\n";

    std::cout << "\n";
}

void demo_recursive_lambda() {
    std::cout << "=== 递归 Lambda ===\n";

    std::cout << "--- 方式1: std::function ---\n";
    {
        std::function<int(int)> factorial = [&factorial](int n) -> int {
            if (n <= 1) return 1;
            return n * factorial(n - 1);
        };
        std::cout << "5! = " << factorial(5) << "\n";
        std::cout << "10! = " << factorial(10) << "\n";
    }

    std::cout << "\n--- 方式2: 泛型 lambda + auto (C++14) ---\n";
    {
        auto fibonacci = [](auto& self, int n) -> long long {
            if (n <= 0) return 0;
            if (n == 1) return 1;
            return self(self, n - 1) + self(self, n - 2);
        };
        std::cout << "fib(10) = " << fibonacci(fibonacci, 10) << "\n";
        std::cout << "fib(20) = " << fibonacci(fibonacci, 20) << "\n";
    }

    std::cout << "\n--- 方式3: Y 组合子 ---\n";
    {
        auto Y = [](auto f) {
            return [f](auto&&... args) {
                return f(f, std::forward<decltype(args)>(args)...);
            };
        };

        auto fact = Y([](auto self, int n) -> int {
            if (n <= 1) return 1;
            return n * self(self, n - 1);
        });

        std::cout << "Y组合子 5! = " << fact(5) << "\n";
    }

    std::cout << "\n递归 lambda 的方式对比:\n";
    std::cout << "  std::function: 最简单, 但有开销\n";
    std::cout << "  显式传递 self: 零开销, 但调用不自然\n";
    std::cout << "  Y 组合子: 函数式风格, 零开销\n";

    std::cout << "\n";
}

int main() {
    demo_deferred_execution();
    demo_customization_points();
    demo_lambda_and_raii();
    demo_immediately_invoked_lambda();
    demo_recursive_lambda();

    return 0;
}
