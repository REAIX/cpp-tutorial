/** @file 03_example_explicit.cpp
 *  @brief explicit构造函数、explicit转换运算符、delete函数、default函数
 *  @description 对应文档: 02-CPP/05-core-mechanism
 */

#include <iostream>
#include <string>
#include <vector>

// ===== 1. explicit 构造函数 =====
class Temperature {
public:
    // 不加 explicit: 允许隐式转换
    // Temperature t = 100.0;  // 隐式转换: double -> Temperature
    explicit Temperature(double celsius) : celsius_(celsius) {
        std::cout << "  Temperature(" << celsius_ << "°C)" << std::endl;
    }

    double celsius() const { return celsius_; }
    double fahrenheit() const { return celsius_ * 9.0 / 5.0 + 32.0; }

private:
    double celsius_;
};

void print_temp(const Temperature& t) {
    std::cout << "  温度: " << t.celsius() << "°C / " << t.fahrenheit() << "°F" << std::endl;
}

class Width {
public:
    // 不加 explicit 的构造函数
    Width(int w) : value_(w) {
        std::cout << "  Width(" << value_ << ")" << std::endl;
    }

    int value() const { return value_; }

private:
    int value_;
};

void demo_explicit_constructor() {
    std::cout << "===== explicit 构造函数 =====" << std::endl;

    // explicit: 阻止隐式转换
    Temperature t1(36.5);       // OK: 直接初始化
    // Temperature t2 = 36.5;   // 错误: explicit 阻止隐式转换
    Temperature t3 = Temperature(36.5);  // OK: 显式转换
    print_temp(t1);

    // 不加 explicit: 允许隐式转换 (可能意外)
    Width w1(100);       // OK: 直接初始化
    Width w2 = 200;      // OK: 隐式转换 int -> Width
    print_temp(Temperature(w1.value()));  // 需要显式

    std::cout << "\n隐式转换的陷阱:" << std::endl;
    std::cout << "  void func(Width w);" << std::endl;
    std::cout << "  func(42);  // 隐式转换! 可能不是预期行为" << std::endl;

    std::cout << "\nexplicit 的建议:" << std::endl;
    std::cout << "  - 单参数构造函数几乎总是应该加 explicit" << std::endl;
    std::cout << "  - 多参数构造函数也可以加 explicit (C++11)" << std::endl;
    std::cout << "  - 防止意外的类型转换" << std::endl;
}

// ===== 2. explicit 转换运算符 =====
class BoolWrapper {
public:
    // 不加 explicit: 允许隐式转为 bool
    // 问题: 可以参与算术运算 (bool -> int)
    // operator bool() const { return value_; }

    // explicit: 阻止隐式转换, 但允许条件上下文
    explicit operator bool() const {
        return value_;
    }

    BoolWrapper(bool v) : value_(v) {}

private:
    bool value_;
};

class SafeBool {
public:
    explicit operator bool() const { return valid_; }

    SafeBool(bool v) : valid_(v) {}

private:
    bool valid_;
};

void demo_explicit_conversion() {
    std::cout << "\n===== explicit 转换运算符 =====" << std::endl;

    BoolWrapper bw(true);

    if (bw) {  // OK: 条件上下文中 explicit operator bool 可用
        std::cout << "  bw 为 true" << std::endl;
    }

    // int n = bw;  // 错误: explicit 阻止隐式转换
    // int n = bw + 1;  // 错误: 不会转为 int 再运算

    SafeBool sb(false);
    if (!sb) {
        std::cout << "  sb 为 false" << std::endl;
    }

    std::cout << "\nexplicit operator bool 的好处:" << std::endl;
    std::cout << "  - 可以在 if/while/for 条件中使用" << std::endl;
    std::cout << "  - 不能参与算术运算 (更安全)" << std::endl;
    std::cout << "  - 替代旧的 safe bool idiom" << std::endl;
    std::cout << "  - 标准库的 unique_ptr, shared_ptr 都用此方式" << std::endl;
}

// ===== 3. delete 函数 =====
class NonCopyable {
public:
    NonCopyable() = default;

    // 删除拷贝操作
    NonCopyable(const NonCopyable&) = delete;
    NonCopyable& operator=(const NonCopyable&) = delete;

    // 允许移动操作
    NonCopyable(NonCopyable&&) = default;
    NonCopyable& operator=(NonCopyable&&) = default;
};

class OnlyInt {
public:
    void set(int value) {
        value_ = value;
        std::cout << "  set(int): " << value_ << std::endl;
    }

    // 删除不需要的重载, 防止隐式转换
    void set(double) = delete;
    void set(bool) = delete;

private:
    int value_ = 0;
};

void demo_delete_functions() {
    std::cout << "\n===== delete 函数 =====" << std::endl;

    NonCopyable nc1;
    // NonCopyable nc2 = nc1;  // 错误: 拷贝构造被删除
    NonCopyable nc3 = std::move(nc1);  // OK: 移动构造

    OnlyInt oi;
    oi.set(42);       // OK
    // oi.set(3.14);   // 错误: double 版本被删除
    // oi.set(true);   // 错误: bool 版本被删除

    std::cout << "\n= delete 的用途:" << std::endl;
    std::cout << "  1. 禁止拷贝 (NonCopyable)" << std::endl;
    std::cout << "  2. 禁止隐式转换 (只接受 int, 不接受 double/bool)" << std::endl;
    std::cout << "  3. 禁止堆分配: void* operator new(size_t) = delete;" << std::endl;
    std::cout << "  4. 禁止默认构造: MyClass() = delete;" << std::endl;

    std::cout << "\n= delete vs private (C++98 方式):" << std::endl;
    std::cout << "  - private: 只声明不定义, 链接错误, 友元可绕过" << std::endl;
    std::cout << "  - delete:  编译错误, 更清晰, 更安全" << std::endl;
}

// ===== 4. default 函数 =====
class SimpleType {
public:
    SimpleType() = default;
    SimpleType(int x) : x_(x) {}

    // 显式要求编译器生成默认的拷贝/移动/析构
    SimpleType(const SimpleType&) = default;
    SimpleType& operator=(const SimpleType&) = default;
    SimpleType(SimpleType&&) = default;
    SimpleType& operator=(SimpleType&&) = default;
    ~SimpleType() = default;

    int x() const { return x_; }

private:
    int x_ = 0;
};

class WithDestructor {
public:
    WithDestructor() = default;
    ~WithDestructor() {
        std::cout << "  自定义析构函数" << std::endl;
    }

    // 一旦自定义了析构函数, 拷贝/移动操作不再自动生成
    // 需要显式 = default 或自定义
    WithDestructor(const WithDestructor&) = default;
    WithDestructor& operator=(const WithDestructor&) = default;
    WithDestructor(WithDestructor&&) = default;
    WithDestructor& operator=(WithDestructor&&) = default;
};

void demo_default_functions() {
    std::cout << "\n===== default 函数 =====" << std::endl;

    SimpleType s1;
    SimpleType s2 = s1;       // 拷贝构造 (default)
    SimpleType s3 = std::move(s1);  // 移动构造 (default)

    std::cout << "s2.x = " << s2.x() << std::endl;
    std::cout << "s3.x = " << s3.x() << std::endl;

    std::cout << "\n= default 的用途:" << std::endl;
    std::cout << "  1. 显式使用编译器生成的默认实现" << std::endl;
    std::cout << "  2. 自定义析构后恢复拷贝/移动操作" << std::endl;
    std::cout << "  3. 使代码意图更清晰" << std::endl;
    std::cout << "  4. 与 = delete 配合, 精确控制特殊成员函数" << std::endl;

    std::cout << "\n特殊成员函数的生成规则 (C++11):" << std::endl;
    std::cout << "  自定义析构函数 -> 不自动生成移动操作" << std::endl;
    std::cout << "  自定义拷贝操作 -> 不自动生成移动操作" << std::endl;
    std::cout << "  自定义移动操作 -> 不自动生成拷贝操作" << std::endl;
    std::cout << "  建议: 需要任何一个就显式声明全部五个 (五之法则)" << std::endl;
}

int main() {
    std::cout << "========== explicit, delete, default ==========\n" << std::endl;

    demo_explicit_constructor();
    demo_explicit_conversion();
    demo_delete_functions();
    demo_default_functions();

    return 0;
}
