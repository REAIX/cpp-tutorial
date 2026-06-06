/** @file 01_example_reference.cpp
 *  @brief C++ 引用基础
 *  @description 对应文档: 02-CPP/01-basics
 */

#include <iostream>
#include <string>

// ===== 1. 引用声明与基本用法 =====
void demo_reference_basics() {
    std::cout << "===== 引用声明与基本用法 =====" << std::endl;

    int value = 42;
    int& ref = value;  // ref 是 value 的引用(别名)

    std::cout << "value = " << value << std::endl;
    std::cout << "ref = " << ref << std::endl;
    std::cout << "&value = " << &value << std::endl;
    std::cout << "&ref = " << &ref << std::endl;
    std::cout << "引用和原变量地址相同, 是同一个对象" << std::endl;

    // 通过引用修改原变量
    ref = 100;
    std::cout << "修改 ref 后, value = " << value << std::endl;
}

// ===== 2. 引用 vs 指针 =====
void demo_reference_vs_pointer() {
    std::cout << "\n===== 引用 vs 指针 =====" << std::endl;

    int a = 10, b = 20;

    // 引用: 必须初始化, 不能重绑定
    int& ref = a;
    ref = b;  // 这不是重绑定! 这是把 b 的值赋给 a
    std::cout << "ref = b 后, a = " << a << " (a 被修改为 b 的值)" << std::endl;

    // 指针: 可以不初始化, 可以重指向
    int* ptr = &a;
    ptr = &b;  // 指针重新指向 b
    std::cout << "ptr = &b 后, *ptr = " << *ptr << " (ptr 指向 b)" << std::endl;

    // 关键区别
    std::cout << "\n引用 vs 指针的关键区别:" << std::endl;
    std::cout << "  1. 引用必须初始化, 指针可以为 nullptr" << std::endl;
    std::cout << "  2. 引用不能重绑定, 指针可以重新指向" << std::endl;
    std::cout << "  3. 引用不需要解引用(*), 使用更自然" << std::endl;
    std::cout << "  4. 引用不可能为空, 更安全" << std::endl;
    std::cout << "  5. 引用没有 '引用的引用', 指针可以多级间接" << std::endl;
}

// ===== 3. const 引用 =====
void demo_const_reference() {
    std::cout << "\n===== const 引用 =====" << std::endl;

    int value = 42;
    const int& cref = value;  // const 引用: 只读

    std::cout << "const 引用读取: cref = " << cref << std::endl;
    // cref = 100;  // 编译错误: const 引用不能修改

    value = 100;  // 可以通过原变量修改
    std::cout << "原变量修改后: cref = " << cref << std::endl;

    // const 引用可以绑定到右值(临时对象)
    const int& rref = 42;  // OK: const 引用延长临时对象生命周期
    std::cout << "const 引用绑定右值: rref = " << rref << std::endl;

    // int& ref = 42;  // 编译错误: 非const引用不能绑定右值

    // const 引用绑定不同类型(会创建临时对象)
    double d = 3.14;
    const int& iref = d;  // OK: 实际绑定到临时 int 对象
    std::cout << "const int& 绑定 double: iref = " << iref << std::endl;
    std::cout << "注意: iref 不是 d 的别名, 而是临时 int 对象的引用" << std::endl;
}

// ===== 4. 引用作为函数参数 =====
void increment_by_value(int x) {
    x++;
}

void increment_by_pointer(int* x) {
    (*x)++;
}

void increment_by_reference(int& x) {
    x++;
}

void print_string_by_value(std::string s) {
    std::cout << s << " (值传递, 发生拷贝)" << std::endl;
}

void print_string_by_reference(const std::string& s) {
    std::cout << s << " (const引用传递, 无拷贝)" << std::endl;
}

void demo_reference_as_parameter() {
    std::cout << "\n===== 引用作为函数参数 =====" << std::endl;

    int n = 10;

    increment_by_value(n);
    std::cout << "值传递后: n = " << n << " (未改变)" << std::endl;

    increment_by_pointer(&n);
    std::cout << "指针传递后: n = " << n << " (已改变)" << std::endl;

    increment_by_reference(n);
    std::cout << "引用传递后: n = " << n << " (已改变)" << std::endl;

    std::string text = "Hello C++";
    print_string_by_value(text);
    print_string_by_reference(text);

    std::cout << "\n参数传递建议:" << std::endl;
    std::cout << "  - 内置类型(int等): 值传递" << std::endl;
    std::cout << "  - 大对象(string等): const 引用传递" << std::endl;
    std::cout << "  - 需要修改实参: 非const 引用传递" << std::endl;
}

// ===== 5. 引用作为返回值 =====
int global_array[5] = {10, 20, 30, 40, 50};

int& get_element(int index) {
    return global_array[index];  // 返回元素的引用
}

int bad_dangling_reference() {
    int local = 42;
    // return local;  // 警告: 返回局部变量的引用! 悬垂引用!
    return local;  // 返回值的拷贝, 安全
}

void demo_reference_as_return() {
    std::cout << "\n===== 引用作为返回值 =====" << std::endl;

    std::cout << "get_element(2) = " << get_element(2) << std::endl;

    // 返回引用可以用在赋值左侧
    get_element(2) = 99;
    std::cout << "修改后 global_array[2] = " << global_array[2] << std::endl;

    std::cout << "\n注意: 永远不要返回局部变量的引用!" << std::endl;
    std::cout << "  - 局部变量在函数返回后销毁" << std::endl;
    std::cout << "  - 引用变成悬垂引用(dangling reference)" << std::endl;
    std::cout << "  - 使用悬垂引用是未定义行为(UB)" << std::endl;
}

int main() {
    std::cout << "========== C++ 引用基础 ==========\n" << std::endl;

    demo_reference_basics();
    demo_reference_vs_pointer();
    demo_const_reference();
    demo_reference_as_parameter();
    demo_reference_as_return();

    return 0;
}
