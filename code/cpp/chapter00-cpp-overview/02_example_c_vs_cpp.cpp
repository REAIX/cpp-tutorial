/** @file 02_example_c_vs_cpp.cpp
 *  @brief C 与 C++ 的关键差异对比
 *  @description 对应文档: 02-CPP/00-cpp-overview
 */

#include <iostream>
#include <string>
#include <cstring>
#include <memory>

// ===== 1. 函数重载 (C++ 独有) =====
int add(int a, int b) {
    return a + b;
}

double add(double a, double b) {
    return a + b;
}

std::string add(const std::string& a, const std::string& b) {
    return a + b;
}

void demo_function_overloading() {
    std::cout << "===== 函数重载 =====" << std::endl;
    // C 语言不支持函数重载, 同名函数只能有一个
    // C++ 根据参数类型/数量选择正确的重载版本
    std::cout << "add(3, 4) = " << add(3, 4) << std::endl;
    std::cout << "add(3.14, 2.72) = " << add(3.14, 2.72) << std::endl;
    std::cout << "add(\"Hello\", \" C++\") = " << add(std::string("Hello"), std::string(" C++")) << std::endl;
}

// ===== 2. 引用 (C++ 独有) =====
void swap_by_pointer(int* a, int* b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void swap_by_reference(int& a, int& b) {
    int temp = a;
    a = b;
    b = temp;
}

void demo_reference() {
    std::cout << "\n===== 引用 vs 指针 =====" << std::endl;
    int x = 10, y = 20;

    // C 风格: 指针传参
    swap_by_pointer(&x, &y);
    std::cout << "指针交换后: x=" << x << ", y=" << y << std::endl;

    // C++ 风格: 引用传参, 更简洁直观
    swap_by_reference(x, y);
    std::cout << "引用交换后: x=" << x << ", y=" << y << std::endl;

    // 引用必须初始化, 不能为空, 更安全
    int& ref = x;
    std::cout << "引用 ref 绑定到 x: ref=" << ref << std::endl;
    ref = 100;
    std::cout << "通过引用修改: x=" << x << std::endl;
}

// ===== 3. bool 类型 =====
void demo_bool_type() {
    std::cout << "\n===== bool 类型 =====" << std::endl;
    // C89 没有 bool, C99 通过 <stdbool.h> 引入
    // C++ 原生支持 bool, true, false 关键字
    bool is_valid = true;
    bool is_empty = false;

    std::cout << std::boolalpha;
    std::cout << "is_valid = " << is_valid << std::endl;
    std::cout << "is_empty = " << is_empty << std::endl;

    // bool 与整数的隐式转换
    int count = 5;
    bool has_items = count;
    std::cout << "count=5 转为 bool: " << has_items << std::endl;

    count = 0;
    has_items = count;
    std::cout << "count=0 转为 bool: " << has_items << std::endl;
}

// ===== 4. new/delete vs malloc/free =====
void demo_new_delete() {
    std::cout << "\n===== new/delete vs malloc/free =====" << std::endl;

    // C 风格: malloc/free, 不调用构造/析构函数
    int* c_ptr = (int*)malloc(sizeof(int));
    *c_ptr = 42;
    std::cout << "malloc 分配: *c_ptr = " << *c_ptr << std::endl;
    free(c_ptr);

    // C++ 风格: new/delete, 自动调用构造/析构函数
    int* cpp_ptr = new int(42);
    std::cout << "new 分配: *cpp_ptr = " << *cpp_ptr << std::endl;
    delete cpp_ptr;

    // 对象的 new/delete 会调用构造/析构
    std::string* str = new std::string("Hello C++");
    std::cout << "new 对象: *str = " << *str << std::endl;
    delete str;

    // 数组
    int* arr = new int[5]{1, 2, 3, 4, 5};
    std::cout << "new 数组: ";
    for (int i = 0; i < 5; ++i) {
        std::cout << arr[i] << " ";
    }
    std::cout << std::endl;
    delete[] arr;

    // 现代 C++ 推荐: 智能指针, 自动管理内存
    auto smart_ptr = std::make_unique<int>(99);
    std::cout << "智能指针: *smart_ptr = " << *smart_ptr << std::endl;
    // 无需 delete, 离开作用域自动释放
}

// ===== 5. 命名空间 =====
namespace Physics {
    const double G = 9.8;
    double freeFall(double t) { return 0.5 * G * t * t; }
}

namespace Math {
    const double PI = 3.14159265358979;
    double circleArea(double r) { return PI * r * r; }
}

void demo_namespace() {
    std::cout << "\n===== 命名空间 =====" << std::endl;
    // C 语言没有命名空间, 全局名称容易冲突
    // C++ 用命名空间组织代码, 避免名称冲突

    std::cout << "自由落体 2s 距离: " << Physics::freeFall(2.0) << " m" << std::endl;
    std::cout << "半径 3 的圆面积: " << Math::circleArea(3.0) << " m^2" << std::endl;
}

int main() {
    std::cout << "========== C vs C++ 差异对比 ==========\n" << std::endl;

    demo_function_overloading();
    demo_reference();
    demo_bool_type();
    demo_new_delete();
    demo_namespace();

    std::cout << "\n===== 总结 =====" << std::endl;
    std::cout << "C++ 相比 C 的核心增强:" << std::endl;
    std::cout << "  1. 函数重载 - 同名不同参" << std::endl;
    std::cout << "  2. 引用 - 更安全的间接访问" << std::endl;
    std::cout << "  3. bool 类型 - 原生布尔支持" << std::endl;
    std::cout << "  4. new/delete - 面向对象的内存管理" << std::endl;
    std::cout << "  5. 命名空间 - 避免名称冲突" << std::endl;

    return 0;
}
