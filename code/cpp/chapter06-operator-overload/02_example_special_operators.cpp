/** @file 02_example_special_operators.cpp
 *  @brief 重载 [], (), ->, *, ++, --, 类型转换运算符
 *  @description 对应文档: 02-CPP/06-operator-overload
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>

// ===== 1. 下标运算符 [] =====
class IntArray {
public:
    IntArray(size_t size) : data_(size, 0) {}

    // 非 const 版本: 可读写
    int& operator[](size_t index) {
        return data_[index];
    }

    // const 版本: 只读
    const int& operator[](size_t index) const {
        return data_[index];
    }

    size_t size() const { return data_.size(); }

private:
    std::vector<int> data_;
};

void demo_subscript_operator() {
    std::cout << "===== 下标运算符 [] =====" << std::endl;

    IntArray arr(5);
    for (size_t i = 0; i < arr.size(); ++i) {
        arr[i] = static_cast<int>(i * 10);
    }

    std::cout << "  arr 内容: ";
    for (size_t i = 0; i < arr.size(); ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;

    const IntArray& carr = arr;
    std::cout << "  const 访问: carr[2] = " << carr[2] << std::endl;

    std::cout << "\n[] 重载要点:" << std::endl;
    std::cout << "  - 提供 const 和非 const 版本" << std::endl;
    std::cout << "  - 返回引用以支持 arr[i] = value" << std::endl;
    std::cout << "  - 考虑边界检查 (或 at() 方法)" << std::endl;
}

// ===== 2. 函数调用运算符 () =====
class Adder {
public:
    Adder(int base) : base_(base) {}

    // 重载 (): 使对象可像函数一样调用 (仿函数/functor)
    int operator()(int x, int y) const {
        return x + y + base_;
    }

    // 可以有多个重载
    int operator()(int x) const {
        return x + base_;
    }

private:
    int base_;
};

class Multiplier {
public:
    double operator()(double a, double b) const {
        return a * b;
    }
};

void demo_call_operator() {
    std::cout << "\n===== 函数调用运算符 () =====" << std::endl;

    Adder add10(10);
    std::cout << "  add10(3, 4) = " << add10(3, 4) << std::endl;
    std::cout << "  add10(5) = " << add10(5) << std::endl;

    Multiplier mul;
    std::cout << "  mul(3.0, 4.0) = " << mul(3.0, 4.0) << std::endl;

    // Lambda 本质上是重载了 () 的匿名类
    auto lambda = [](int x) { return x * x; };
    std::cout << "  lambda(5) = " << lambda(5) << std::endl;

    std::cout << "\n() 重载要点:" << std::endl;
    std::cout << "  - 使对象可像函数一样调用 (仿函数)" << std::endl;
    std::cout << "  - 可以有状态 (成员变量)" << std::endl;
    std::cout << "  - 可以有多个重载" << std::endl;
    std::cout << "  - STL 算法大量使用仿函数" << std::endl;
    std::cout << "  - Lambda 是仿函数的语法糖" << std::endl;
}

// ===== 3. 箭头运算符 -> 和解引用 * =====
template<typename T>
class SmartPtr {
public:
    explicit SmartPtr(T* ptr = nullptr) : ptr_(ptr) {}
    ~SmartPtr() { delete ptr_; }

    SmartPtr(const SmartPtr&) = delete;
    SmartPtr& operator=(const SmartPtr&) = delete;

    // 解引用
    T& operator*() const {
        return *ptr_;
    }

    // 箭头运算符: 返回指针
    T* operator->() const {
        return ptr_;
    }

    // bool 转换: 检查是否为空
    explicit operator bool() const {
        return ptr_ != nullptr;
    }

private:
    T* ptr_;
};

struct Person {
    std::string name;
    int age;
    void greet() const {
        std::cout << "  你好, 我是 " << name << ", " << age << " 岁" << std::endl;
    }
};

void demo_arrow_and_deref() {
    std::cout << "\n===== 箭头 -> 和解引用 * =====" << std::endl;

    SmartPtr<Person> sp(new Person{"张三", 25});

    // -> 运算符
    sp->greet();
    std::cout << "  sp->name = " << sp->name << std::endl;

    // * 运算符
    std::cout << "  (*sp).age = " << (*sp).age << std::endl;

    // bool 转换
    if (sp) {
        std::cout << "  智能指针非空" << std::endl;
    }

    SmartPtr<Person> empty;
    if (!empty) {
        std::cout << "  空智能指针" << std::endl;
    }

    std::cout << "\n-> 重载要点:" << std::endl;
    std::cout << "  - 返回指针, 编译器自动继续 -> 访问成员" << std::endl;
    std::cout << "  - 可以返回另一个重载了 -> 的对象 (链式调用)" << std::endl;
    std::cout << "  - * 返回引用" << std::endl;
}

// ===== 4. 自增/自减运算符 ++/-- =====
class Counter {
public:
    Counter(int value = 0) : value_(value) {}

    // 前置 ++: 返回引用
    Counter& operator++() {
        ++value_;
        return *this;
    }

    // 后置 ++: 返回值 (参数 int 是区分标记, 不使用)
    Counter operator++(int) {
        Counter temp = *this;
        ++value_;
        return temp;
    }

    // 前置 --
    Counter& operator--() {
        --value_;
        return *this;
    }

    // 后置 --
    Counter operator--(int) {
        Counter temp = *this;
        --value_;
        return temp;
    }

    int value() const { return value_; }

    friend std::ostream& operator<<(std::ostream& os, const Counter& c) {
        os << c.value_;
        return os;
    }

private:
    int value_;
};

void demo_increment_decrement() {
    std::cout << "\n===== 自增/自减运算符 ++/-- =====" << std::endl;

    Counter c(5);
    std::cout << "  初始: " << c << std::endl;

    ++c;
    std::cout << "  ++c: " << c << std::endl;

    c++;
    std::cout << "  c++: " << c << std::endl;

    Counter c2 = c++;
    std::cout << "  c2 = c++: c=" << c << ", c2=" << c2 << std::endl;

    Counter c3 = ++c;
    std::cout << "  c3 = ++c: c=" << c << ", c3=" << c3 << std::endl;

    std::cout << "\n前置 vs 后置:" << std::endl;
    std::cout << "  前置 ++x: 返回引用, 无拷贝, 更高效" << std::endl;
    std::cout << "  后置 x++: 返回旧值拷贝, 有额外开销" << std::endl;
    std::cout << "  建议: 默认使用前置 ++" << std::endl;
}

// ===== 5. 类型转换运算符 =====
class Celsius {
public:
    explicit Celsius(double c) : value_(c) {}

    // 类型转换运算符
    operator double() const {
        return value_;
    }

    // explicit 类型转换 (C++11)
    explicit operator int() const {
        return static_cast<int>(value_);
    }

private:
    double value_;
};

void demo_conversion_operators() {
    std::cout << "\n===== 类型转换运算符 =====" << std::endl;

    Celsius c(36.5);

    // 隐式转换: operator double()
    double temp = c;
    std::cout << "  隐式转 double: " << temp << std::endl;

    // 显式转换: explicit operator int()
    int itemp = static_cast<int>(c);
    std::cout << "  显式转 int: " << itemp << std::endl;

    std::cout << "\n类型转换运算符要点:" << std::endl;
    std::cout << "  - 避免隐式转换 (加 explicit)" << std::endl;
    std::cout << "  - operator bool() 应该总是 explicit" << std::endl;
    std::cout << "  - 过多隐式转换导致难以调试的问题" << std::endl;
}

int main() {
    std::cout << "========== 特殊运算符重载 ==========\n" << std::endl;

    demo_subscript_operator();
    demo_call_operator();
    demo_arrow_and_deref();
    demo_increment_decrement();
    demo_conversion_operators();

    return 0;
}
