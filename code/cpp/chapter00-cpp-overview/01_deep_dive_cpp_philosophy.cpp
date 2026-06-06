/** @file 01_deep_dive_cpp_philosophy.cpp
 *  @brief C++ 设计哲学与演进历程
 *  @description 对应文档: 02-CPP/00-cpp-overview
 */

#include <iostream>
#include <string>
#include <vector>
#include <array>
#include <chrono>
#include <type_traits>

// ===== 1. 零开销原则 (Zero-Overhead Principle) =====
void demo_zero_overhead() {
    std::cout << "===== 零开销原则 =====" << std::endl;

    // C++ 核心设计哲学: 你不为没有使用的东西付出代价
    // 抽象的运行时开销应该为零或接近零

    // 示例: std::array vs 原生数组 - 零开销抽象
    std::array<int, 5> cpp_arr = {1, 2, 3, 4, 5};
    int c_arr[5] = {1, 2, 3, 4, 5};

    // std::array 和原生数组在内存布局上完全一致
    std::cout << "std::array 大小: " << sizeof(cpp_arr) << " 字节" << std::endl;
    std::cout << "原生数组大小: " << sizeof(c_arr) << " 字节" << std::endl;
    std::cout << "两者大小相同, std::array 没有额外开销" << std::endl;

    // 内联函数 - 编译期优化, 无函数调用开销
    // 模板 - 编译期实例化, 无运行时多态开销
    std::cout << "\n零开销原则的体现:" << std::endl;
    std::cout << "  - 内联函数: 无调用开销" << std::endl;
    std::cout << "  - 模板: 编译期多态, 无虚函数开销" << std::endl;
    std::cout << "  - RAII: 无额外内存管理开销" << std::endl;
    std::cout << "  - std::array: 与原生数组零开销" << std::endl;
}

// ===== 2. C++ 标准演进 =====
void demo_cpp_evolution() {
    std::cout << "\n===== C++ 标准演进 =====" << std::endl;

    // C++98: 第一个国际标准
    std::string s98 = "C++98: STL, 异常, 命名空间, RTTI, 模板基础";
    std::cout << s98 << std::endl;

    // C++11: 革命性更新, "现代 C++" 的起点
    auto x = 42;                    // auto 类型推导
    decltype(x) y = 100;           // decltype
    std::vector<int> v = {1, 2, 3}; // 初始化列表
    for (auto& elem : v) {          // 范围 for 循环
        elem *= 2;
    }
    std::cout << "C++11: auto, range-for, 初始化列表, 移动语义, Lambda, 智能指针" << std::endl;

    // C++14: 小幅改进
    auto lambda = [](auto a, auto b) { return a + b; }; // 泛型 Lambda
    std::cout << "C++14: 泛型Lambda, 返回类型推导, make_unique" << std::endl;

    // C++17: 结构化绑定, if constexpr, std::optional
    std::pair<int, std::string> p{42, "hello"};
    auto [num, str] = p; // 结构化绑定
    (void)num; (void)str;
    std::cout << "C++17: 结构化绑定, if constexpr, optional, variant, filesystem" << std::endl;

    // C++20: 概念, 协程, 模块, 范围
    std::cout << "C++20: Concepts, Ranges, Coroutines, Modules, <=> 运算符" << std::endl;

    // C++23: std::expected, std::print, 显式 this 参数(deducing this)
    std::cout << "C++23: std::expected, std::print, deducing this, std::flat_map" << std::endl;
}

// ===== 3. 编译模型 =====
void demo_compilation_model() {
    std::cout << "\n===== C++ 编译模型 =====" << std::endl;

    // C++ 编译的四个阶段:
    std::cout << "1. 预处理 (Preprocessing)" << std::endl;
    std::cout << "   - 处理 #include, #define, #ifdef 等指令" << std::endl;
    std::cout << "   - 宏展开, 条件编译" << std::endl;
    std::cout << "   - 产出翻译单元 (Translation Unit)" << std::endl;

    std::cout << "\n2. 编译 (Compilation)" << std::endl;
    std::cout << "   - 语法分析, 语义分析" << std::endl;
    std::cout << "   - 模板实例化" << std::endl;
    std::cout << "   - 生成汇编代码" << std::endl;

    std::cout << "\n3. 汇编 (Assembly)" << std::endl;
    std::cout << "   - 汇编代码转目标文件 (.o / .obj)" << std::endl;

    std::cout << "\n4. 链接 (Linking)" << std::endl;
    std::cout << "   - 合并目标文件" << std::endl;
    std::cout << "   - 解析外部符号引用" << std::endl;
    std::cout << "   - 生成可执行文件" << std::endl;

    // ODR (One Definition Rule) - 单一定义规则
    std::cout << "\n重要规则: ODR (One Definition Rule)" << std::endl;
    std::cout << "  - 每个变量/函数在整个程序中只能有一个定义" << std::endl;
    std::cout << "  - 内联函数和模板可以在多个翻译单元中定义, 但必须相同" << std::endl;
    std::cout << "  - 违反 ODR 导致未定义行为(UB), 编译器可能不报错!" << std::endl;
}

// ===== 4. 举一反三: 零开销原则的实际影响 =====
void demo_zero_overhead_implications() {
    std::cout << "\n===== 举一反三: 零开销原则的影响 =====" << std::endl;

    // 陷阱1: 虚函数不是零开销
    std::cout << "注意: 虚函数有运行时开销!" << std::endl;
    std::cout << "  - 每个对象多一个 vptr 指针 (通常 8 字节)" << std::endl;
    std::cout << "  - 虚函数调用需要间接跳转 (vtable 查找)" << std::endl;
    std::cout << "  - 阻碍编译器内联优化" << std::endl;
    std::cout << "  解决方案: CRTP (编译期多态), if constexpr" << std::endl;

    // 陷阱2: 异常不是零开销
    std::cout << "\n注意: 异常有潜在开销!" << std::endl;
    std::cout << "  - 需要额外的展开表 (DWARF/.eh_frame)" << std::endl;
    std::cout << "  - 可能增加二进制大小" << std::endl;
    std::cout << "  - 某些场景下禁用异常 (-fno-exceptions)" << std::endl;

    // 最佳实践: 选择合适的抽象层次
    std::cout << "\n最佳实践:" << std::endl;
    std::cout << "  - 性能关键路径: 编译期多态 (模板, CRTP)" << std::endl;
    std::cout << "  - 灵活扩展路径: 运行时多态 (虚函数)" << std::endl;
    std::cout << "  - 编译期能做的事, 不要留到运行期" << std::endl;
    std::cout << "  - 编译期能检查的, 不要留到运行期" << std::endl;
}

int main() {
    std::cout << "========== C++ 设计哲学与演进 ==========\n" << std::endl;

    demo_zero_overhead();
    demo_cpp_evolution();
    demo_compilation_model();
    demo_zero_overhead_implications();

    return 0;
}
