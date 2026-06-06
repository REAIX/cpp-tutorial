/** @file 01_example_const.cpp
 *  @brief const变量、const引用、const指针、const成员函数、mutable
 *  @description 对应文档: 02-CPP/05-core-mechanism
 */

#include <iostream>
#include <string>
#include <vector>

// ===== 1. const 变量 =====
void demo_const_variables() {
    std::cout << "===== const 变量 =====" << std::endl;

    const int max_size = 100;
    // max_size = 200;  // 编译错误: const 变量不能修改

    // const 替代 #define
    // #define MAX_SIZE 100  // C 风格, 无类型检查
    const int kMaxSize = 100;  // C++ 风格, 有类型检查, 有作用域

    std::cout << "const 变量: max_size = " << max_size << std::endl;
    std::cout << "const 比 #define 更安全: 有类型, 有作用域, 可调试" << std::endl;

    // constexpr: 编译期常量 (C++11)
    constexpr int compile_time_const = 42;
    std::cout << "constexpr: 编译期常量 = " << compile_time_const << std::endl;

    // const vs constexpr
    // const: 运行期或编译期常量
    // constexpr: 必须是编译期常量
    int runtime_value = 10;
    const int runtime_const = runtime_value;  // OK: 运行期 const
    // constexpr int runtime_constexpr = runtime_value;  // 错误: 需要编译期值
    (void)runtime_const;
}

// ===== 2. const 引用 =====
void demo_const_reference() {
    std::cout << "\n===== const 引用 =====" << std::endl;

    int value = 42;
    const int& cref = value;  // const 引用: 只读

    std::cout << "const 引用读取: " << cref << std::endl;
    // cref = 100;  // 编译错误: 不能通过 const 引用修改

    value = 100;  // 可以通过原变量修改
    std::cout << "原变量修改后: " << cref << std::endl;

    // const 引用绑定到右值 (延长临时对象生命周期)
    const int& rref = 42;
    std::cout << "const 引用绑定右值: " << rref << std::endl;

    // const 引用作为函数参数 (避免拷贝, 保证不修改)
    auto print_string = [](const std::string& s) {
        std::cout << "  const& 参数: " << s << std::endl;
    };
    print_string("Hello");  // 可以接受右值
    std::string str = "World";
    print_string(str);      // 可以接受左值
}

// ===== 3. const 指针 =====
void demo_const_pointer() {
    std::cout << "\n===== const 指针 =====" << std::endl;

    int a = 10, b = 20;

    // 指向 const 的指针 (pointer to const): 不能通过指针修改值
    const int* ptr1 = &a;
    // *ptr1 = 100;  // 错误: 不能通过 ptr1 修改
    ptr1 = &b;       // OK: 可以改变指向
    std::cout << "const int* ptr1: *ptr1 = " << *ptr1 << std::endl;

    // const 指针 (const pointer): 不能改变指向
    int* const ptr2 = &a;
    *ptr2 = 100;     // OK: 可以通过 ptr2 修改值
    // ptr2 = &b;    // 错误: 不能改变指向
    std::cout << "int* const ptr2: *ptr2 = " << *ptr2 << std::endl;

    // 指向 const 的 const 指针
    const int* const ptr3 = &a;
    // *ptr3 = 200;  // 错误
    // ptr3 = &b;    // 错误
    std::cout << "const int* const ptr3: *ptr3 = " << *ptr3 << std::endl;

    std::cout << "\n记忆技巧: 从右往左读" << std::endl;
    std::cout << "  const int* p    -> p is a pointer to int const" << std::endl;
    std::cout << "  int* const p    -> p is a const pointer to int" << std::endl;
    std::cout << "  const int* const p -> p is a const pointer to int const" << std::endl;
}

// ===== 4. const 成员函数 =====
class BankAccount {
public:
    BankAccount(const std::string& owner, double balance)
        : owner_(owner), balance_(balance), access_count_(0) {}

    // const 成员函数: 承诺不修改对象状态
    double get_balance() const {
        ++access_count_;  // OK: mutable 成员可以在 const 函数中修改
        return balance_;
    }

    const std::string& get_owner() const {
        return owner_;
    }

    void display() const {
        std::cout << "  " << owner_ << ": 余额 " << balance_
                  << " (查询" << access_count_ << "次)" << std::endl;
    }

    // 非 const 成员函数: 可以修改对象状态
    void deposit(double amount) {
        balance_ += amount;
    }

    void withdraw(double amount) {
        if (amount <= balance_) {
            balance_ -= amount;
        }
    }

private:
    std::string owner_;
    double balance_;
    mutable int access_count_;  // mutable: 即使在 const 函数中也可修改
};

void demo_const_member_function() {
    std::cout << "\n===== const 成员函数 =====" << std::endl;

    BankAccount acc("张三", 1000.0);
    acc.deposit(500.0);
    acc.display();

    const BankAccount& const_acc = acc;
    const_acc.display();       // OK: const 对象可以调用 const 成员函数
    std::cout << "  余额: " << const_acc.get_balance() << std::endl;
    // const_acc.deposit(100);  // 错误: const 对象不能调用非 const 成员函数

    std::cout << "\nconst 成员函数规则:" << std::endl;
    std::cout << "  - const 对象只能调用 const 成员函数" << std::endl;
    std::cout << "  - 不修改状态的函数都应标记 const" << std::endl;
    std::cout << "  - mutable: 逻辑上不影响对象状态的成员" << std::endl;
}

// ===== 5. mutable 关键字 =====
class Cache {
public:
    int compute(int x) const {
        ++call_count_;  // mutable: 缓存统计不影响逻辑 const 性
        if (x == cached_input_) {
            ++cache_hits_;
            return cached_result_;
        }
        cached_input_ = x;
        cached_result_ = x * x;  // 模拟计算
        return cached_result_;
    }

    void stats() const {
        std::cout << "  调用 " << call_count_ << " 次, 缓存命中 " << cache_hits_ << " 次" << std::endl;
    }

private:
    mutable int cached_input_ = 0;
    mutable int cached_result_ = 0;
    mutable int call_count_ = 0;
    mutable int cache_hits_ = 0;
};

void demo_mutable() {
    std::cout << "\n===== mutable 关键字 =====" << std::endl;

    Cache cache;
    std::cout << "  compute(5) = " << cache.compute(5) << std::endl;
    std::cout << "  compute(5) = " << cache.compute(5) << std::endl;
    std::cout << "  compute(3) = " << cache.compute(3) << std::endl;
    cache.stats();

    std::cout << "\nmutable 的合理用途:" << std::endl;
    std::cout << "  1. 缓存计算结果" << std::endl;
    std::cout << "  2. 访问计数/统计" << std::endl;
    std::cout << "  3. 互斥锁 (mutex 必须可修改)" << std::endl;
    std::cout << "  注意: 不要滥用 mutable 规避 const 约束" << std::endl;
}

int main() {
    std::cout << "========== const 详解 ==========\n" << std::endl;

    demo_const_variables();
    demo_const_reference();
    demo_const_pointer();
    demo_const_member_function();
    demo_mutable();

    return 0;
}
