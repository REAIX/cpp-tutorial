/** @file 02_deep_dive_c_philosophy.c
 *  @brief 深入理解C语言设计哲学
 *  @description 对应文档: 00-c-overview | C的设计理念、为什么C依然重要、C与其他语言对比、何时选择C
 *  编译命令: gcc -std=c17 02_deep_dive_c_philosophy.c -o 02_deep_dive_c_philosophy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demo_c_design_principles(void) {
    printf("═══════════════════════════════════════\n");
    printf("  C语言的核心设计原则\n");
    printf("═══════════════════════════════════════\n\n");

    printf("1. 信任程序员 (Trust the programmer)\n");
    printf("   C假设程序员知道自己在做什么\n");
    printf("   允许直接操作内存，不做过多安全检查\n");
    printf("   例: 指针算术、手动内存管理、类型强转\n\n");

    printf("2. 不要阻止程序员做需要做的事\n");
    printf("   (Don't prevent the programmer from doing what needs to be done)\n");
    printf("   void* 通用指针可以指向任何类型\n");
    printf("   可以通过强制转换绕过类型系统\n\n");

    printf("3. 保持语言简洁 (Keep the language small)\n");
    printf("   C89只有32个关键字\n");
    printf("   没有内置的字符串类型、动态数组、哈希表\n");
    printf("   通过标准库提供功能，而非语言特性\n\n");

    printf("4. 只提供一种方法做一件事\n");
    printf("   (Provide only one way to do an operation)\n");
    printf("   减少歧义，提高代码一致性\n\n");

    printf("5. 让它快到不值得用汇编重写\n");
    printf("   (Make it fast, even if it is not guaranteed to be portable)\n");
    printf("   C的抽象开销极低，接近硬件性能\n");
}

void demo_c_history(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  C语言发展简史\n");
    printf("═══════════════════════════════════════\n\n");

    printf("  1969  BCPL语言 (Martin Richards)\n");
    printf("    ↓\n");
    printf("  1970  B语言 (Ken Thompson, 贝尔实验室)\n");
    printf("    ↓\n");
    printf("  1972  C语言诞生 (Dennis Ritchie, 贝尔实验室)\n");
    printf("        最初用于重写UNIX操作系统\n");
    printf("    ↓\n");
    printf("  1978  K&R C (《The C Programming Language》出版)\n");
    printf("    ↓\n");
    printf("  1989  C89/C90 (ANSI C / ISO C 第一个标准)\n");
    printf("    ↓\n");
    printf("  1999  C99 (变长数组、//注释、bool类型、snprintf等)\n");
    printf("    ↓\n");
    printf("  2011  C11 (多线程、泛型宏 _Generic、匿名结构体等)\n");
    printf("    ↓\n");
    printf("  2018  C17 (缺陷修复版本)\n");
    printf("    ↓\n");
    printf("  2024  C23 (最新标准，大量现代化改进)\n\n");

    printf("关键洞察: C诞生于操作系统开发，这决定了它:\n");
    printf("  - 贴近硬件，可直接操作内存和端口\n");
    printf("  - 极小的运行时依赖\n");
    printf("  - 可移植的汇编级控制\n");
}

void demo_c_vs_others(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  C语言与其他语言对比\n");
    printf("═══════════════════════════════════════\n\n");

    printf("特性            C        C++       Java      Python    Rust\n");
    printf("────────────────────────────────────────────────────────────\n");
    printf("内存管理        手动      手动/RAII  GC        GC        所有权\n");
    printf("运行速度        极快      极快       快        慢        极快\n");
    printf("抽象级别        低        高        高        极高      中高\n");
    printf("类型安全        弱        强        强        动态      极强\n");
    printf("指针            有        有        无        无        受限\n");
    printf("面向对象        无        有        有        有        trait\n");
    printf("泛型            宏/void*  模板      泛型      鸭子      泛型\n");
    printf("运行时依赖      极小      较小      JVM       解释器    极小\n");
    printf("学习曲线        中等      陡峭      中等      平缓      陡峭\n\n");

    printf("C的独特优势:\n");
    printf("  ✓ 最接近硬件的高级语言，可直接内嵌汇编\n");
    printf("  ✓ 极小的运行时，适合嵌入式和操作系统开发\n");
    printf("  ✓ 50年历史，生态极其成熟\n");
    printf("  ✓ 几乎所有平台都有C编译器\n");
    printf("  ✓ 是理解计算机系统的最佳入口\n\n");

    printf("C的不足:\n");
    printf("  ✗ 内存安全依赖程序员，容易出错\n");
    printf("  ✗ 缺乏现代抽象(无异常、无泛型、无模块)\n");
    printf("  ✗ 标准库功能有限(无网络、无线程直到C11)\n");
    printf("  ✗ 字符串处理繁琐且易出错\n");
}

void demo_when_to_use_c(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  何时选择C语言\n");
    printf("═══════════════════════════════════════\n\n");

    printf("适合使用C的场景:\n\n");

    printf("1. 操作系统开发\n");
    printf("   Linux内核、Windows内核、macOS XNU都是C写的\n");
    printf("   原因: 需要直接操作硬件，极小的运行时依赖\n\n");

    printf("2. 嵌入式系统\n");
    printf("   单片机、IoT设备、汽车ECU\n");
    printf("   原因: 资源受限，需要精确控制内存和性能\n\n");

    printf("3. 系统级编程\n");
    printf("   驱动程序、编译器、虚拟机、数据库引擎\n");
    printf("   原因: 需要底层控制和高性能\n\n");

    printf("4. 高性能计算\n");
    printf("   图像处理、信号处理、游戏引擎核心\n");
    printf("   原因: 零开销抽象，可精确优化\n\n");

    printf("5. 安全关键系统\n");
    printf("   航空航天、医疗设备、核电站控制\n");
    printf("   原因: 行业认证(MISRA C)，行为可预测\n\n");

    printf("6. 理解计算机系统\n");
    printf("   学习C是理解内存、指针、编译原理的最佳途径\n");
    printf("   原因: C的抽象层最薄，能看清底层机制\n\n");

    printf("不适合使用C的场景:\n");
    printf("  ✗ 快速开发的Web应用 → 用 Python/JS/Ruby\n");
    printf("  ✗ 复杂业务逻辑 → 用 Java/C#/Go\n");
    printf("  ✗ 需要内存安全保证 → 用 Rust\n");
    printf("  ✗ 跨平台GUI应用 → 用 Electron/Qt(C++)\n");
}

void demo_c_in_real_world(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  C语言在现实世界中的应用\n");
    printf("═══════════════════════════════════════\n\n");

    printf("著名C语言项目:\n\n");

    printf("  项目              代码量(约)    领域\n");
    printf("  ─────────────────────────────────────\n");
    printf("  Linux内核          3000万行     操作系统\n");
    printf("  SQLite             15万行       数据库\n");
    printf("  Git                30万行       版本控制\n");
    printf("  Python解释器       50万行       编程语言\n");
    printf("  PostgreSQL         100万行      数据库\n");
    printf("  Nginx              15万行       Web服务器\n");
    printf("  Redis              10万行       缓存/数据库\n");
    printf("  curl               10万行       网络工具\n\n");

    printf("TIOBE编程语言排行榜中，C常年位居前3。\n");
    printf("几乎所有编程语言的底层实现都依赖C:\n");
    printf("  Python → CPython(C实现)\n");
    printf("  Ruby → MRI(C实现)\n");
    printf("  PHP → Zend引擎(C实现)\n");
    printf("  Java → HotSpot JVM(C++实现，但JVM底层依赖C)\n");
}

void demo_learning_path(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  C语言学习路径建议\n");
    printf("═══════════════════════════════════════\n\n");

    printf("入门阶段 (1-2个月):\n");
    printf("  → 基本语法、数据类型、控制结构\n");
    printf("  → 函数、数组、指针基础\n");
    printf("  → 能写100行以内的小程序\n\n");

    printf("进阶阶段 (2-4个月):\n");
    printf("  → 指针深入、内存管理、结构体\n");
    printf("  → 文件I/O、预处理器、多文件编译\n");
    printf("  → 能写500行以内的项目\n\n");

    printf("深入阶段 (4-8个月):\n");
    printf("  → 数据结构(链表、树、哈希表)的C实现\n");
    printf("  → 系统编程(进程、线程、网络)\n");
    printf("  → 能写数千行的完整项目\n\n");

    printf("精通阶段 (持续):\n");
    printf("  → 阅读开源项目源码(Linux内核、SQLite等)\n");
    printf("  → 理解编译原理和计算机体系结构\n");
    printf("  → 能写出高质量、可维护的C代码\n\n");

    printf("推荐书籍:\n");
    printf("  入门: 《C Primer Plus》\n");
    printf("  进阶: 《The C Programming Language》(K&R)\n");
    printf("  深入: 《C Expert Programming》《C陷阱与缺陷》\n");
    printf("  系统: 《深入理解计算机系统》(CSAPP)\n");
}

int main(void) {
    demo_c_design_principles();
    demo_c_history();
    demo_c_vs_others();
    demo_when_to_use_c();
    demo_c_in_real_world();
    demo_learning_path();

    return 0;
}
