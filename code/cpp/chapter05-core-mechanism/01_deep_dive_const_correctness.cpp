/** @file 01_deep_dive_const_correctness.cpp
 *  @brief const正确性、const_cast危险、逻辑const与物理const
 *  @description 对应文档: 02-CPP/05-core-mechanism
 */

#include <iostream>
#include <string>
#include <vector>

// ===== 1. const 正确性贯穿代码库 =====

// 原则: 能用 const 就用 const
// - const 变量: 不会被修改的值
// - const 引用参数: 不会修改参数
// - const 指针参数: 不会通过指针修改
// - const 成员函数: 不会修改对象状态
// - const 返回值: 返回的引用/指针不应被修改

class Student {
public:
    Student(const std::string& name, int age)
        : name_(name), age_(age) {}

    // const 成员函数: 不修改对象
    const std::string& name() const { return name_; }
    int age() const { return age_; }

    // 非 const 成员函数: 修改对象
    void set_age(int age) { age_ = age; }

    // 迭代器: 提供 const 和非 const 版本
    using ScoreList = std::vector<int>;

    ScoreList::iterator begin() { return scores_.begin(); }
    ScoreList::const_iterator begin() const { return scores_.begin(); }

    void add_score(int score) { scores_.push_back(score); }

private:
    std::string name_;
    int age_;
    ScoreList scores_;
};

void print_student(const Student& s) {
    // const 引用参数: 保证不修改, 且可以接受临时对象
    std::cout << "  " << s.name() << ", " << s.age() << " 岁" << std::endl;
}

void demo_const_correctness() {
    std::cout << "===== const 正确性 =====" << std::endl;

    Student s("张三", 20);
    s.add_score(95);
    s.add_score(87);

    print_student(s);  // OK: 非 const 对象可以传给 const 引用

    const Student& cs = s;
    // cs.set_age(21);  // 错误: const 对象不能调用非 const 函数
    std::cout << "  const 引用访问: " << cs.name() << std::endl;

    std::cout << "\nconst 正确性的好处:" << std::endl;
    std::cout << "  1. 编译器帮你检查: 意外修改会被捕获" << std::endl;
    std::cout << "  2. 代码意图更清晰: const = 不修改" << std::endl;
    std::cout << "  3. 更好的接口契约: 函数签名传达信息" << std::endl;
    std::cout << "  4. 编译器优化: const 数据可能被放入只读段" << std::endl;
}

// ===== 2. const_cast 的危险 =====
void dangerous_const_cast() {
    std::cout << "\n===== const_cast 的危险 =====" << std::endl;

    // 场景1: 去掉 const 修改原始变量 (可能合法)
    int value = 42;
    const int& cref = value;
    int& ref = const_cast<int&>(cref);
    ref = 100;  // OK: value 本身不是 const, 修改合法
    std::cout << "  修改非 const 原变量: value = " << value << std::endl;

    // 场景2: 去掉 const 修改真正的 const 变量 (未定义行为!)
    const int const_value = 42;
    const int& cref2 = const_value;
    int& ref2 = const_cast<int&>(cref2);
    // ref2 = 100;  // 未定义行为! const_value 可能存储在只读内存
    std::cout << "  警告: 修改真正的 const 变量是未定义行为!" << std::endl;

    // 场景3: const_cast 在函数中的误用
    auto bad_function = [](const int& x) -> int& {
        // return const_cast<int&>(x);  // 危险! 调用者可能修改
        // 如果原对象是 const, 修改是 UB
        throw std::runtime_error("不应使用 const_cast 去掉 const");
    };
    (void)bad_function;

    std::cout << "\nconst_cast 的唯一合理用途:" << std::endl;
    std::cout << "  - 调用遗留 C API (不合理的 const 缺失)" << std::endl;
    std::cout << "  - 实现 const 和非 const 版本的函数 (避免代码重复)" << std::endl;
}

// ===== 3. const 和非 const 版本函数的代码复用 =====
class TextBlock {
public:
    TextBlock(const std::string& text) : text_(text) {}

    // 非 const 版本调用 const 版本, 避免代码重复
    const char& operator[](size_t pos) const {
        // ... 复杂的边界检查和日志 ...
        return text_[pos];
    }

    char& operator[](size_t pos) {
        // 调用 const 版本, 然后用 const_cast 去掉返回值的 const
        // 这是 const_cast 的合理使用!
        return const_cast<char&>(
            static_cast<const TextBlock&>(*this)[pos]
        );
    }

private:
    std::string text_;
};

void demo_const_overload_reuse() {
    std::cout << "\n===== const/非const 版本复用 =====" << std::endl;

    TextBlock tb("Hello");
    tb[0] = 'h';  // 非 const 版本
    std::cout << "  修改后: " << tb[0] << std::endl;

    const TextBlock ctb("World");
    std::cout << "  const 版本: " << ctb[0] << std::endl;

    std::cout << "\n复用模式:" << std::endl;
    std::cout << "  非 const 版本调用 const 版本" << std::endl;
    std::cout << "  步骤:" << std::endl;
    std::cout << "    1. static_cast<const T&>(*this) 添加 const" << std::endl;
    std::cout << "    2. 调用 const 版本函数" << std::endl;
    std::cout << "    3. const_cast 去掉返回值的 const" << std::endl;
    std::cout << "  这是 const_cast 的合理使用!" << std::endl;
}

// ===== 4. 逻辑 const vs 物理 const =====
class LogCache {
public:
    double compute(int input) const {
        // 逻辑 const: 对象的"可观察状态"不变
        // 物理 const: 对象的任何成员都不变

        // 缓存是"逻辑上不影响可观察状态"的内部实现细节
        // 用 mutable 允许在 const 函数中修改缓存
        if (input == cached_input_) {
            ++cache_hits_;
            std::cout << "  缓存命中" << std::endl;
            return cached_result_;
        }

        // 模拟昂贵计算
        double result = input * input * 3.14159;
        cached_input_ = input;
        cached_result_ = result;
        return result;
    }

    void stats() const {
        std::cout << "  缓存命中: " << cache_hits_ << " 次" << std::endl;
    }

private:
    // 可观察状态 (逻辑 const 必须保护)
    // (这里没有可变状态, 纯计算类)

    // 内部缓存 (mutable: 逻辑 const 允许修改)
    mutable int cached_input_ = -1;
    mutable double cached_result_ = 0.0;
    mutable int cache_hits_ = 0;
};

class MutableCounter {  // 注意: 不是线程安全的! 仅用于演示 mutable 用法
public:
    int increment() const {
        // 逻辑 const: 计数不影响对象的"值"
        // 但 mutex 必须可修改 (lock/unlock)
        // 简化版: 用 bool 模拟锁
        locked_ = true;
        ++count_;
        locked_ = false;
        return count_;
    }

    int get() const {
        return count_;
    }

private:
    mutable int count_ = 0;
    mutable bool locked_ = false;
};

void demo_logical_vs_physical_const() {
    std::cout << "\n===== 逻辑 const vs 物理 const =====" << std::endl;

    const LogCache cache;
    std::cout << "  compute(5) = " << cache.compute(5) << std::endl;
    std::cout << "  compute(5) = " << cache.compute(5) << std::endl;
    std::cout << "  compute(3) = " << cache.compute(3) << std::endl;
    cache.stats();

    std::cout << "\n逻辑 const vs 物理 const:" << std::endl;
    std::cout << "  物理 const: 对象的任何字节都不变" << std::endl;
    std::cout << "  逻辑 const: 对象的可观察行为不变" << std::endl;
    std::cout << "  C++ 的 const 是物理 const" << std::endl;
    std::cout << "  mutable 用于实现逻辑 const" << std::endl;

    std::cout << "\nmutable 的合理场景:" << std::endl;
    std::cout << "  1. 缓存计算结果" << std::endl;
    std::cout << "  2. 访问计数/统计" << std::endl;
    std::cout << "  3. 互斥锁 (mutex 必须可修改)" << std::endl;
    std::cout << "  4. 延迟初始化" << std::endl;

    std::cout << "\nmutable 的滥用:" << std::endl;
    std::cout << "  - 不要用 mutable 绕过 const 约束" << std::endl;
    std::cout << "  - 如果 mutable 成员影响可观察行为, 设计有问题" << std::endl;
}

// ===== 5. 举一反三: const 最佳实践 =====
void demo_const_best_practices() {
    std::cout << "\n===== 举一反三: const 最佳实践 =====" << std::endl;

    std::cout << "1. 函数参数:" << std::endl;
    std::cout << "   - 内置类型: 值传递 (int x)" << std::endl;
    std::cout << "   - 大对象: const 引用 (const string& s)" << std::endl;
    std::cout << "   - 需要修改: 非const 引用/指针" << std::endl;

    std::cout << "\n2. 成员函数:" << std::endl;
    std::cout << "   - 不修改状态: 标记 const" << std::endl;
    std::cout << "   - 总是提供 const 和非 const 版本" << std::endl;

    std::cout << "\n3. 返回值:" << std::endl;
    std::cout << "   - 返回内部引用: const& (只读)" << std::endl;
    std::cout << "   - 需要修改: 非 const& (谨慎)" << std::endl;

    std::cout << "\n4. 局部变量:" << std::endl;
    std::cout << "   - 不修改: const (编译器优化 + 代码意图)" << std::endl;
    std::cout << "   - const auto& 绑定临时对象" << std::endl;

    std::cout << "\n5. 陷阱:" << std::endl;
    std::cout << "   - const 容器返回 const 迭代器" << std::endl;
    std::cout << "   - const 成员函数返回内部指针: 指针指向的数据仍可修改!" << std::endl;
    std::cout << "   - 这违反了逻辑 const, 但编译器无法检测" << std::endl;
}

int main() {
    std::cout << "========== const 正确性深入 ==========\n" << std::endl;

    demo_const_correctness();
    dangerous_const_cast();
    demo_const_overload_reuse();
    demo_logical_vs_physical_const();
    demo_const_best_practices();

    return 0;
}
