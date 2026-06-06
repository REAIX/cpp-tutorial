/** @file 02_example_class_template.cpp
 *  @brief 类模板基础：定义、成员函数、非类型参数、默认模板参数
 *  @description 对应文档: 10-模板基础
 */

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>

template<typename T>
class Stack {
public:
    void push(const T& value) {
        data_.push_back(value);
    }

    void push(T&& value) {
        data_.push_back(std::move(value));
    }

    template<typename... Args>
    void emplace(Args&&... args) {
        data_.emplace_back(std::forward<Args>(args)...);
    }

    void pop() {
        if (data_.empty()) throw std::runtime_error("栈为空");
        data_.pop_back();
    }

    T& top() {
        if (data_.empty()) throw std::runtime_error("栈为空");
        return data_.back();
    }

    const T& top() const {
        if (data_.empty()) throw std::runtime_error("栈为空");
        return data_.back();
    }

    bool empty() const { return data_.empty(); }
    size_t size() const { return data_.size(); }

private:
    std::vector<T> data_;
};

void demo_class_template_basics() {
    std::cout << "=== 类模板基础 ===\n";

    Stack<int> int_stack;
    int_stack.push(10);
    int_stack.push(20);
    int_stack.push(30);
    std::cout << "int 栈顶: " << int_stack.top() << "\n";
    int_stack.pop();
    std::cout << "pop 后栈顶: " << int_stack.top() << "\n";

    Stack<std::string> str_stack;
    str_stack.push("Hello");
    str_stack.push(std::string("World"));
    str_stack.emplace("C++17");
    std::cout << "string 栈顶: " << str_stack.top() << "\n";

    std::cout << "\n类模板的定义:\n";
    std::cout << "  template<typename T>\n";
    std::cout << "  class Stack { ... };\n";
    std::cout << "  使用时必须指定类型: Stack<int>, Stack<string>\n";

    std::cout << "\n";
}

template<typename T>
class Pair {
public:
    Pair() : first_(), second_() {}
    Pair(const T& first, const T& second) : first_(first), second_(second) {}

    T& first() { return first_; }
    const T& first() const { return first_; }
    T& second() { return second_; }
    const T& second() const { return second_; }

    void swap(Pair& other) {
        using std::swap;
        swap(first_, other.first_);
        swap(second_, other.second_);
    }

    void print() const {
        std::cout << "(" << first_ << ", " << second_ << ")\n";
    }

private:
    T first_;
    T second_;
};

void demo_member_functions() {
    std::cout << "=== 类模板的成员函数 ===\n";

    Pair<int> p1(10, 20);
    std::cout << "p1 = ";
    p1.print();

    Pair<std::string> p2("Hello", "World");
    std::cout << "p2 = ";
    p2.print();

    Pair<int> p3(30, 40);
    p1.swap(p3);
    std::cout << "swap 后 p1 = ";
    p1.print();

    std::cout << "\n成员函数的注意事项:\n";
    std::cout << "  1. 类模板的成员函数本身就是函数模板\n";
    std::cout << "  2. 在类内定义时, 可以省略 template 前缀\n";
    std::cout << "  3. 在类外定义时, 需要带模板参数列表\n";

    std::cout << "\n";
}

template<typename T, size_t N>
class FixedArray {
public:
    T& operator[](size_t index) {
        if (index >= N) throw std::out_of_range("索引越界");
        return data_[index];
    }

    const T& operator[](size_t index) const {
        if (index >= N) throw std::out_of_range("索引越界");
        return data_[index];
    }

    constexpr size_t size() const { return N; }

    T* begin() { return data_; }
    T* end() { return data_ + N; }
    const T* begin() const { return data_; }
    const T* end() const { return data_ + N; }

    void fill(const T& value) {
        for (size_t i = 0; i < N; ++i) {
            data_[i] = value;
        }
    }

private:
    T data_[N];
};

void demo_non_type_parameters() {
    std::cout << "=== 非类型模板参数 ===\n";

    FixedArray<int, 5> arr;
    arr.fill(0);
    arr[0] = 10;
    arr[2] = 30;
    arr[4] = 50;

    std::cout << "FixedArray<int, 5> 内容: ";
    for (const auto& v : arr) {
        std::cout << v << " ";
    }
    std::cout << "\n大小: " << arr.size() << "\n";

    FixedArray<double, 3> darr;
    darr[0] = 1.1;
    darr[1] = 2.2;
    darr[2] = 3.3;

    std::cout << "FixedArray<double, 3> 内容: ";
    for (const auto& v : darr) {
        std::cout << v << " ";
    }
    std::cout << "\n";

    std::cout << "\n非类型模板参数的限制:\n";
    std::cout << "  1. 必须是编译期常量\n";
    std::cout << "  2. 可以是: 整型、枚举、指针、引用\n";
    std::cout << "  3. C++17 起: 可以是 auto\n";

    std::cout << "\n";
}

template<typename T, typename Container = std::vector<T>>
class FlexibleStack {
public:
    void push(const T& value) {
        data_.push_back(value);
    }

    void pop() {
        if (data_.empty()) throw std::runtime_error("栈为空");
        data_.pop_back();
    }

    T& top() {
        if (data_.empty()) throw std::runtime_error("栈为空");
        return data_.back();
    }

    bool empty() const { return data_.empty(); }
    size_t size() const { return data_.size(); }

private:
    Container data_;
};

void demo_default_template_args() {
    std::cout << "=== 默认模板参数 ===\n";

    FlexibleStack<int> default_stack;
    default_stack.push(1);
    default_stack.push(2);
    std::cout << "默认容器栈顶: " << default_stack.top() << "\n";

    std::cout << "\n默认模板参数的语法:\n";
    std::cout << "  template<typename T, typename Container = vector<T>>\n";
    std::cout << "  class FlexibleStack { ... };\n";
    std::cout << "  FlexibleStack<int>  // Container 默认为 vector<int>\n";

    std::cout << "\n";
}

template<typename T>
class Wrapper {
public:
    Wrapper(T value) : value_(std::move(value)) {}

    T& get() { return value_; }
    const T& get() const { return value_; }

    void print() const {
        std::cout << "Wrapper 值: " << value_ << "\n";
    }

private:
    T value_;
};

template<typename T>
class Wrapper<T*> {
public:
    Wrapper(T* ptr) : ptr_(ptr) {}

    T& operator*() { return *ptr_; }
    T* operator->() { return ptr_; }

    void print() const {
        if (ptr_) {
            std::cout << "Wrapper<T*> 值: " << *ptr_ << "\n";
        } else {
            std::cout << "Wrapper<T*> 空指针\n";
        }
    }

private:
    T* ptr_;
};

void demo_partial_specialization_intro() {
    std::cout << "=== 偏特化简介 ===\n";

    Wrapper<int> w1(42);
    w1.print();

    int x = 100;
    Wrapper<int*> w2(&x);
    w2.print();

    std::cout << "\n偏特化: 为模板的某些参数组合提供专门实现\n";
    std::cout << "  template<typename T> class Wrapper<T*> { ... }\n";
    std::cout << "  当 T 为指针类型时, 使用偏特化版本\n";

    std::cout << "\n";
}

int main() {
    demo_class_template_basics();
    demo_member_functions();
    demo_non_type_parameters();
    demo_default_template_args();
    demo_partial_specialization_intro();

    return 0;
}
