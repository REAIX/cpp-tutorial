/** @file 02_deep_dive_cpp_ecosystem.cpp
 *  @brief C++ 生态系统与学习路径
 *  @description 对应文档: 02-CPP/00-cpp-overview
 */

#include <iostream>
#include <string>
#include <map>
#include <vector>

// ===== 1. C++ 标准委员会与标准流程 =====
void demo_standards_committee() {
    std::cout << "===== C++ 标准委员会 (WG21) =====" << std::endl;

    std::cout << "ISO/IEC JTC1/SC22/WG21 - C++ 标准委员会" << std::endl;
    std::cout << "  - 由各国国家标准机构派代表参加" << std::endl;
    std::cout << "  - 下设多个研究组 (SG):" << std::endl;

    std::map<std::string, std::string> groups = {
        {"SG1",  "并发与并行"},
        {"SG5",  "反射"},
        {"SG6",  "数值计算"},
        {"SG7",  "编译期编程"},
        {"SG9",  "范围(Ranges)"},
        {"SG13", "方向组(战略方向)"},
        {"SG14", "低延迟/游戏/金融"},
        {"SG15", "工具(模块等)"},
        {"SG16", "Unicode"},
        {"SG19", "教育"},
        {"SG20", "库基础"},
        {"SG21", "契约"},
        {"SG22", "C 与 C++ 互操作"}
    };

    for (const auto& [id, desc] : groups) {
        std::cout << "    " << id << ": " << desc << std::endl;
    }

    std::cout << "\n标准制定流程:" << std::endl;
    std::cout << "  1. 提案 (Proposal) -> 2. 工作草案 (WD)" << std::endl;
    std::cout << "  3. 委员会草案 (CD) -> 4. 国际标准草案 (DIS)" << std::endl;
    std::cout << "  5. 最终国际标准草案 (FDIS) -> 6. 国际标准 (IS)" << std::endl;
    std::cout << "  通常每 3 年发布一个新标准" << std::endl;
}

// ===== 2. 主流编译器 =====
void demo_compilers() {
    std::cout << "\n===== 主流 C++ 编译器 =====" << std::endl;

    struct CompilerInfo {
        std::string name;
        std::string org;
        std::string platforms;
        std::string notes;
    };

    std::vector<CompilerInfo> compilers = {
        {"GCC",       "GNU/FSF",     "Linux, macOS, Windows(MinGW)", "最广泛使用的开源编译器"},
        {"Clang",     "LLVM 项目",    "全平台",                       "诊断信息优秀, 模块化设计"},
        {"MSVC",      "微软",         "Windows",                      "Visual Studio 内置"},
        {"Intel C++", "Intel",        "Linux, Windows",               "高性能计算优化"},
        {"EDG",       "Edison Design","前端, 被其他编译器使用",        "最符合标准的C++前端"}
    };

    for (const auto& c : compilers) {
        std::cout << "  " << c.name << " (" << c.org << ")" << std::endl;
        std::cout << "    平台: " << c.platforms << std::endl;
        std::cout << "    特点: " << c.notes << std::endl;
    }

    // 编译器对标准的支持程度不同
    std::cout << "\n编译器标准支持:" << std::endl;
    std::cout << "  GCC 13+: C++20 大部分, C++23 部分" << std::endl;
    std::cout << "  Clang 17+: C++20 大部分, C++23 部分" << std::endl;
    std::cout << "  MSVC 19.35+: C++20 大部分, C++23 部分" << std::endl;
    std::cout << "  查阅: https://en.cppreference.com/w/cpp/compiler_support" << std::endl;
}

// ===== 3. 构建系统与包管理器 =====
void demo_build_systems() {
    std::cout << "\n===== 构建系统与包管理器 =====" << std::endl;

    std::cout << "构建系统:" << std::endl;
    std::cout << "  Make       - 经典构建工具, Makefile 驱动" << std::endl;
    std::cout << "  CMake      - 事实标准, 跨平台元构建系统" << std::endl;
    std::cout << "  Ninja      - 快速构建后端, 常与 CMake 配合" << std::endl;
    std::cout << "  Bazel      - Google 出品, 大规模项目" << std::endl;
    std::cout << "  Meson      - 现代, 快速, 用户友好" << std::endl;
    std::cout << "  xmake      - 国产, 轻量, Lua 配置" << std::endl;

    std::cout << "\n包管理器:" << std::endl;
    std::cout << "  vcpkg      - 微软出品, CMake 集成好" << std::endl;
    std::cout << "  Conan      - 去中心化, Python 配置" << std::endl;
    std::cout << "  CPM.cmake  - CMake 原生包管理" << std::endl;
    std::cout << "  build2     - 一体化构建+包管理" << std::endl;
    std::cout << "  C++20 模块  - 未来可能改变依赖管理方式" << std::endl;
}

// ===== 4. 学习路径 =====
void demo_learning_path() {
    std::cout << "\n===== C++ 学习路径 =====" << std::endl;

    std::cout << "第一阶段: 基础 (1-3个月)" << std::endl;
    std::cout << "  - 基本语法, 类型系统, 控制流" << std::endl;
    std::cout << "  - 函数, 引用, 指针" << std::endl;
    std::cout << "  - 类与对象, 构造/析构" << std::endl;
    std::cout << "  - 标准库容器 (vector, string, map)" << std::endl;

    std::cout << "\n第二阶段: 进阶 (3-6个月)" << std::endl;
    std::cout << "  - 继承与多态, 虚函数机制" << std::endl;
    std::cout << "  - 模板基础, STL 算法" << std::endl;
    std::cout << "  - 智能指针, RAII, 异常安全" << std::endl;
    std::cout << "  - 移动语义, 右值引用" << std::endl;

    std::cout << "\n第三阶段: 高级 (6-12个月)" << std::endl;
    std::cout << "  - 模板元编程, SFINAE, Concepts" << std::endl;
    std::cout << "  - 多线程, 内存模型, 并发模式" << std::endl;
    std::cout << "  - 设计模式在 C++ 中的应用" << std::endl;
    std::cout << "  - 性能优化, 内存管理" << std::endl;

    std::cout << "\n推荐书籍:" << std::endl;
    std::cout << "  入门: 《C++ Primer》" << std::endl;
    std::cout << "  进阶: 《Effective C++》《Effective Modern C++》" << std::endl;
    std::cout << "  高级: 《C++ Concurrency in Action》" << std::endl;
    std::cout << "  参考: 《The C++ Programming Language》" << std::endl;

    std::cout << "\n在线资源:" << std::endl;
    std::cout << "  - cppreference.com: 最权威的在线参考" << std::endl;
    std::cout << "  - godbolt.org: 在线编译器, 查看汇编" << std::endl;
    std::cout << "  - compiler-explorer.org: 同上" << std::endl;
    std::cout << "  - isocpp.org: 官方博客和FAQ" << std::endl;
    std::cout << "  - CppCon YouTube: 年度大会演讲" << std::endl;
}

// ===== 5. 举一反三: 工程实践中的选择 =====
void demo_practical_choices() {
    std::cout << "\n===== 举一反三: 工程实践中的选择 =====" << std::endl;

    // 陷阱: 盲目追求最新标准
    std::cout << "常见误区:" << std::endl;
    std::cout << "  1. 追求最新特性而忽视项目约束" << std::endl;
    std::cout << "     - 需要考虑: 目标平台编译器支持, 团队熟悉度" << std::endl;
    std::cout << "  2. 过度使用模板导致编译时间爆炸" << std::endl;
    std::cout << "     - 平衡: 编译期抽象 vs 编译时间" << std::endl;
    std::cout << "  3. 忽视 ABI 兼容性" << std::endl;
    std::cout << "     - 不同编译器/版本可能 ABI 不兼容" << std::endl;

    std::cout << "\n最佳实践:" << std::endl;
    std::cout << "  - 选择 C++17 作为生产项目的基线标准" << std::endl;
    std::cout << "  - 使用 CMake 作为构建系统" << std::endl;
    std::cout << "  - 使用 vcpkg/Conan 管理第三方依赖" << std::endl;
    std::cout << "  - 开启 -Wall -Wextra -Werror 编译警告" << std::endl;
    std::cout << "  - 使用 clang-format 统一代码风格" << std::endl;
    std::cout << "  - 使用 clang-tidy 做静态分析" << std::endl;
    std::cout << "  - CI 中使用 AddressSanitizer 检测内存问题" << std::endl;
}

int main() {
    std::cout << "========== C++ 生态系统与学习路径 ==========\n" << std::endl;

    demo_standards_committee();
    demo_compilers();
    demo_build_systems();
    demo_learning_path();
    demo_practical_choices();

    return 0;
}
