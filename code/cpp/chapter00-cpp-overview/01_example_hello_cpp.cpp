/** @file 01_example_hello_cpp.cpp
 *  @brief C++ Hello World 与程序结构基础
 *  @description 对应文档: 02-CPP/00-cpp-overview
 */

#include <iostream>
#include <string>
#include <cstdio>

namespace MyMath {
    const double pi = 3.14159265358979;
    int add(int a, int b) { return a + b; }
}

void demo_iostream_vs_stdio() {
    std::cout << "===== iostream vs stdio =====" << std::endl;

    // C++ 风格: iostream, 类型安全, 自动推导格式
    int value = 42;
    std::string name = "C++";
    std::cout << "iostream 输出: value = " << value
              << ", name = " << name << std::endl;

    // C 风格: stdio, 需要手动指定格式符, 不类型安全
    std::printf("stdio 输出: value = %d, name = %s\n", value, name.c_str());

    // iostream 的优势: 编译期类型检查, 不容易出错
    // std::printf("%s", value);  // 错误: int 当字符串, 但编译可能不报错, 运行时崩溃
    std::cout << "iostream 类型安全, 不会出现格式不匹配问题" << std::endl;
}

void demo_namespace_basics() {
    std::cout << "\n===== 命名空间基础 =====" << std::endl;

    // 使用完全限定名
    std::cout << "1. 完全限定名: std::cout" << std::endl;

    // using 声明: 引入单个名称
    using std::string;
    string msg = "2. using 声明引入 string, 无需 std:: 前缀";
    std::cout << msg << std::endl;

    // using 指令: 引入整个命名空间(不推荐在头文件中使用)
    using namespace std;
    cout << "3. using namespace std, 所有 std 名称可见" << endl;

    // 自定义命名空间 (在函数外定义, 此处仅演示使用)
    cout << "4. 自定义命名空间 MyMath::pi = " << MyMath::pi << endl;
    cout << "   MyMath::add(3, 4) = " << MyMath::add(3, 4) << endl;
}

void demo_cpp_program_structure() {
    std::cout << "\n===== C++ 程序结构 =====" << std::endl;

    // C++ 程序的基本组成:
    // 1. 预处理指令 (#include, #define 等)
    // 2. 命名空间声明
    // 3. 全局变量/常量
    // 4. 函数声明与定义
    // 5. 类/结构体定义
    // 6. main() 函数 - 程序入口

    std::cout << "C++ 程序结构:" << std::endl;
    std::cout << "  1. 预处理指令 (#include)" << std::endl;
    std::cout << "  2. 命名空间 (namespace)" << std::endl;
    std::cout << "  3. 全局声明" << std::endl;
    std::cout << "  4. 函数和类定义" << std::endl;
    std::cout << "  5. main() 入口函数" << std::endl;

    // main 函数的两种标准形式:
    // int main()              - 不带参数
    // int main(int argc, char* argv[])  - 带命令行参数
    // 返回 0 表示成功, 非 0 表示错误
    std::cout << "\nmain() 返回 0 表示程序正常结束" << std::endl;
}

int main() {
    std::cout << "========== C++ Hello World ==========" << std::endl;
    std::cout << "Hello, C++ World!" << std::endl;
    std::cout << "=====================================\n" << std::endl;

    demo_iostream_vs_stdio();
    demo_namespace_basics();
    demo_cpp_program_structure();

    std::cout << "\n===== 编译命令 =====" << std::endl;
    std::cout << "g++ -std=c++20 01_example_hello_cpp.cpp -o hello_cpp" << std::endl;

    return 0;
}
