/**
 * @file 01_deep_dive_gcc_advanced.c
 * @brief GCC高级特性深入 - Sanitizer、LTO、PGO、交叉编译
 * @description 对应文档: 21-gcc-g++
 *              本文件以独立可运行代码演示GCC的高级特性，
 *              包括Address Sanitizer、Undefined Behavior Sanitizer、
 *              LTO、PGO、-march=native、交叉编译基础
 *
 * 编译: gcc -Wall -Wextra -std=c11 01_deep_dive_gcc_advanced.c -o deep_dive_gcc
 * 运行: ./deep_dive_gcc
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * 第一部分: Sanitizer (内存与行为检测器)
 * ======================================================================== */

/*
 * Sanitizer 是 GCC/Clang 内置的运行时检测工具，能在运行时发现
 * 内存错误、未定义行为等难以调试的问题。
 *
 * 主要类型:
 *   -fsanitize=address     AddressSanitizer (ASan)
 *   -fsanitize=undefined   UndefinedBehaviorSanitizer (UBSan)
 *   -fsanitize=thread      ThreadSanitizer (TSan)
 *   -fsanitize=memory      MemorySanitizer (MSan, 仅Clang)
 *
 * 使用方式:
 *   gcc -fsanitize=address -g program.c -o program
 *   gcc -fsanitize=undefined -g program.c -o program
 *   gcc -fsanitize=address,undefined -g program.c -o program
 */

void demo_address_sanitizer(void) {
    printf("===== AddressSanitizer (ASan) =====\n\n");

    printf("ASan 能检测的错误类型:\n");
    printf("  1. 堆缓冲区溢出 (Heap buffer overflow)\n");
    printf("  2. 栈缓冲区溢出 (Stack buffer overflow)\n");
    printf("  3. 全局缓冲区溢出 (Global buffer overflow)\n");
    printf("  4. 使用已释放内存 (Use after free)\n");
    printf("  5. 双重释放 (Double free)\n");
    printf("  6. 内存泄漏 (Memory leak, 需加 -fsanitize=leak)\n\n");

    printf("ASan 使用方法:\n");
    printf("  编译: gcc -fsanitize=address -g -fno-omit-frame-pointer \\\n");
    printf("            program.c -o program\n");
    printf("  运行: ./program\n");
    printf("  出错时会打印详细的错误信息和调用栈\n\n");

    printf("ASan 原理:\n");
    printf("  - 在每个内存分配周围插入\"红区\"(red zone)\n");
    printf("  - 访问红区时触发错误报告\n");
    printf("  - 使用影子内存(shadow memory)跟踪内存状态\n");
    printf("  - 性能开销约 2x，内存开销约 3x\n\n");

    printf("ASan 环境变量:\n");
    printf("  ASAN_OPTIONS=detect_leaks=1     启用泄漏检测\n");
    printf("  ASAN_OPTIONS=halt_on_error=0    不在错误时终止\n");
    printf("  ASAN_OPTIONS=verbosity=2        详细输出\n\n");

    printf("示例 - 堆溢出检测:\n");
    printf("  int *p = malloc(10 * sizeof(int));\n");
    printf("  p[10] = 42;    // ASan 会报告 heap-buffer-overflow\n\n");

    printf("示例 - Use-after-free:\n");
    printf("  int *p = malloc(sizeof(int));\n");
    printf("  free(p);\n");
    printf("  *p = 42;       // ASan 会报告 heap-use-after-free\n\n");
}

void demo_undefined_behavior_sanitizer(void) {
    printf("===== UndefinedBehaviorSanitizer (UBSan) =====\n\n");

    printf("UBSan 能检测的未定义行为:\n");
    printf("  1. 整数溢出 (Signed integer overflow)\n");
    printf("  2. 空指针解引用 (Null pointer dereference)\n");
    printf("  3. 除以零 (Division by zero)\n");
    printf("  4. 数组越界 (Out of bounds array access)\n");
    printf("  5. 类型不匹配的类型转换\n");
    printf("  6. 移位操作超出范围\n");
    printf("  7. 对齐违规 (Alignment violation)\n\n");

    printf("UBSan 使用方法:\n");
    printf("  gcc -fsanitize=undefined -g program.c -o program\n\n");

    printf("UBSan 子选项:\n");
    printf("  -fsanitize=signed-integer-overflow  只检测有符号整数溢出\n");
    printf("  -fsanitize=null                     只检测空指针\n");
    printf("  -fsanitize=bounds                   只检测数组越界\n");
    printf("  -fsanitize=alignment                只检测对齐问题\n");
    printf("  -fsanitize=shift                    只检测移位越界\n\n");

    printf("UBSan vs ASan:\n");
    printf("  UBSan: 检测未定义行为，性能开销小 (~5%%)\n");
    printf("  ASan:  检测内存错误，性能开销大 (~2x)\n");
    printf("  可以同时使用: -fsanitize=address,undefined\n\n");

    printf("C语言中的常见未定义行为:\n");
    printf("  int a = INT_MAX + 1;       // 有符号整数溢出\n");
    printf("  int *p = NULL; *p = 42;    // 空指针解引用\n");
    printf("  int b = 1 << 31;           // 移位超出int宽度\n");
    printf("  int c = 10 / 0;            // 除以零\n\n");
}

void demo_thread_sanitizer(void) {
    printf("===== ThreadSanitizer (TSan) =====\n\n");

    printf("TSan 检测数据竞争 (Data Race):\n");
    printf("  当多个线程同时访问同一内存，且至少一个是写操作，\n");
    printf("  且没有同步机制时，就是数据竞争。\n\n");

    printf("TSan 使用方法:\n");
    printf("  gcc -fsanitize=thread -g program.c -o program -lpthread\n\n");

    printf("TSan 性能开销:\n");
    printf("  约 5-15x 慢，10x 内存开销\n");
    printf("  只在测试环境使用\n\n");

    printf("数据竞争示例:\n");
    printf("  // 线程1:              // 线程2:\n");
    printf("  global_var = 42;       printf(\"%%d\\n\", global_var);\n");
    printf("  // 没有锁保护 → 数据竞争!\n\n");
}

/* ========================================================================
 * 第二部分: 链接时优化 (LTO)
 * ======================================================================== */

void demo_lto(void) {
    printf("===== 链接时优化 (Link-Time Optimization) =====\n\n");

    printf("LTO 允许编译器在链接阶段进行跨编译单元的优化。\n\n");

    printf("普通编译流程:\n");
    printf("  file1.c → file1.o ─┐\n");
    printf("                      ├→ 链接器 → 可执行文件\n");
    printf("  file2.c → file2.o ─┘\n");
    printf("  每个文件独立优化，链接器不做优化\n\n");

    printf("LTO 编译流程:\n");
    printf("  file1.c → file1.o (含GIMPLE中间表示) ─┐\n");
    printf("                                         ├→ LTO → 链接器 → 可执行文件\n");
    printf("  file2.c → file2.o (含GIMPLE中间表示) ─┘\n");
    printf("  链接时可以看到所有代码，进行全局优化\n\n");

    printf("LTO 使用方法:\n");
    printf("  编译: gcc -flto -c file1.c\n");
    printf("        gcc -flto -c file2.c\n");
    printf("  链接: gcc -flto file1.o file2.o -o program\n");
    printf("  或一步: gcc -flto file1.c file2.c -o program\n\n");

    printf("LTO 的优化能力:\n");
    printf("  1. 跨文件内联: file1.c 中的函数内联到 file2.c\n");
    printf("  2. 跨文件死代码消除\n");
    printf("  3. 更精确的类型分析\n");
    printf("  4. 更好的虚函数去虚拟化 (C++)\n\n");

    printf("LTO 的代价:\n");
    printf("  - 链接时间显著增加\n");
    printf("  - 链接时需要更多内存\n");
    printf("  - 增量编译效果减弱\n\n");

    printf("LTO 级别:\n");
    printf("  -flto           自动选择(通常等于 -flto=auto)\n");
    printf("  -flto=1         单线程LTO\n");
    printf("  -flto=auto      多线程LTO (并行加速)\n");
    printf("  -flto=thin      ThinLTO (更快的增量LTO, LLVM)\n\n");
}

/* ========================================================================
 * 第三部分: -march=native 与目标平台优化
 * ======================================================================== */

void demo_march_native(void) {
    printf("===== -march=native 与平台优化 =====\n\n");

    printf("-march=native: 生成针对当前CPU优化的代码\n");
    printf("  自动检测CPU支持的指令集(SSE, AVX, AVX2等)\n");
    printf("  生成使用这些指令集的代码\n\n");

    printf("示例:\n");
    printf("  gcc -march=native -O2 program.c -o program\n\n");

    printf("查看 -march=native 实际启用的选项:\n");
    printf("  gcc -march=native -Q --help=target\n\n");

    printf("指定特定架构:\n");
    printf("  -march=x86-64       通用x86-64 (兼容性最好)\n");
    printf("  -march=x86-64-v2    SSE4.2 + POPCNT\n");
    printf("  -march=x86-64-v3    AVX2 + BMI + FMA\n");
    printf("  -march=x86-64-v4    AVX-512\n");
    printf("  -march=haswell      Intel Haswell\n");
    printf("  -march=znver3       AMD Zen 3\n\n");

    printf("-mtune vs -march:\n");
    printf("  -march: 生成使用特定指令集的代码 (可能不兼容旧CPU)\n");
    printf("  -mtune: 优化指令调度，但不使用新指令 (保持兼容)\n\n");

    printf("注意:\n");
    printf("  -march=native 生成的二进制不能在不支持相同指令集的CPU上运行\n");
    printf("  发布软件时应使用 -march=x86-64 保证兼容性\n\n");
}

/* ========================================================================
 * 第四部分: Profile-Guided Optimization (PGO)
 * ======================================================================== */

void demo_pgo(void) {
    printf("===== Profile-Guided Optimization (PGO) =====\n\n");

    printf("PGO 利用程序的实际运行数据指导优化决策。\n\n");

    printf("PGO 三步流程:\n\n");

    printf("步骤1: 编译插桩版本\n");
    printf("  gcc -fprofile-generate=./profdata \\\n");
    printf("      -O2 program.c -o program_instrumented\n\n");

    printf("步骤2: 运行插桩版本收集性能数据\n");
    printf("  ./program_instrumented < typical_input.txt\n");
    printf("  # 在 ./profdata/ 目录下生成 .gcda 文件\n\n");

    printf("步骤3: 使用性能数据重新编译\n");
    printf("  gcc -fprofile-use=./profdata \\\n");
    printf("      -O2 program.c -o program_optimized\n\n");

    printf("PGO 的优化效果:\n");
    printf("  1. 分支预测优化: 根据实际分支概率排列代码\n");
    printf("  2. 函数布局优化: 热路径函数放在一起\n");
    printf("  3. 内联决策优化: 根据实际调用频率决定内联\n");
    printf("  4. 代码段排序: 减少指令缓存缺失\n\n");

    printf("PGO 注意事项:\n");
    printf("  - 测试数据必须具有代表性\n");
    printf("  - 不同使用模式可能需要不同的profile\n");
    printf("  - 编译流程更复杂，CI/CD需要额外步骤\n\n");

    printf("AutoFDO (Auto-Feedback Directed Optimization):\n");
    printf("  使用 perf 采集的性能数据，无需插桩版本\n");
    printf("  gcc -fauto-profile=profile.data -O2 program.c\n\n");
}

/* ========================================================================
 * 第五部分: 交叉编译基础
 * ======================================================================== */

void demo_cross_compilation(void) {
    printf("===== 交叉编译基础 =====\n\n");

    printf("交叉编译: 在一个平台上编译，在另一个平台上运行\n\n");

    printf("常见场景:\n");
    printf("  - x86 主机上编译 ARM 程序 (嵌入式开发)\n");
    printf("  - Linux 上编译 Windows 程序 (MinGW)\n");
    printf("  - 64位主机上编译32位程序\n\n");

    printf("交叉编译工具链:\n");
    printf("  ARM Linux:    arm-linux-gnueabihf-gcc\n");
    printf("  AArch64:      aarch64-linux-gnu-gcc\n");
    printf("  Windows (64): x86_64-w64-mingw32-gcc\n");
    printf("  Windows (32): i686-w64-mingw32-gcc\n");
    printf("  RISC-V:       riscv64-linux-gnu-gcc\n\n");

    printf("使用示例:\n");
    printf("  arm-linux-gnueabihf-gcc -o program program.c\n\n");

    printf("Sysroot:\n");
    printf("  交叉编译需要目标平台的头文件和库\n");
    printf("  --sysroot=/path/to/target/root\n\n");

    printf("CMake 交叉编译:\n");
    printf("  创建 toolchain.cmake 文件:\n");
    printf("    set(CMAKE_SYSTEM_NAME Linux)\n");
    printf("    set(CMAKE_SYSTEM_PROCESSOR arm)\n");
    printf("    set(CMAKE_C_COMPILER arm-linux-gnueabihf-gcc)\n");
    printf("  cmake -DCMAKE_TOOLCHAIN_FILE=toolchain.cmake ..\n\n");
}

/* ========================================================================
 * 第六部分: 综合最佳实践
 * ======================================================================== */

void demo_best_practices(void) {
    printf("===== GCC 使用最佳实践 =====\n\n");

    printf("开发阶段编译选项:\n");
    printf("  gcc -Wall -Wextra -Wpedantic -Wshadow -std=c11 \\\n");
    printf("      -g -Og -fsanitize=address,undefined \\\n");
    printf("      -fno-omit-frame-pointer program.c -o program\n\n");

    printf("测试阶段编译选项:\n");
    printf("  gcc -Wall -Wextra -Wpedantic -std=c11 \\\n");
    printf("      -g -O2 -fsanitize=address,undefined \\\n");
    printf("      program.c -o program\n\n");

    printf("生产发布编译选项:\n");
    printf("  gcc -Wall -Wextra -Werror -std=c11 \\\n");
    printf("      -O2 -DNDEBUG -flto \\\n");
    printf("      -Wl,-z,relro,-z,now \\\n");
    printf("      -fPIE -pie \\\n");
    printf("      program.c -o program\n\n");

    printf("性能关键路径编译选项:\n");
    printf("  gcc -O3 -march=native -flto \\\n");
    printf("      -fprofile-use=./profdata \\\n");
    printf("      program.c -o program\n\n");

    printf("安全加固选项:\n");
    printf("  -fPIE -pie              位置无关可执行文件 (ASLR)\n");
    printf("  -fstack-protector-strong  栈保护 (防缓冲区溢出)\n");
    printf("  -D_FORTIFY_SOURCE=2     运行时缓冲区检查\n");
    printf("  -Wl,-z,relro,-z,now     完全RELRO\n");
    printf("  -Wl,-z,noexecstack      禁止栈执行\n\n");

    printf("举一反三 - 编译选项选择决策树:\n");
    printf("  开发? → -Og -g -fsanitize=address,undefined\n");
    printf("  测试? → -O2 -g -fsanitize=address,undefined\n");
    printf("  发布? → -O2 -DNDEBUG -flto -fPIE -pie\n");
    printf("  性能? → -O3 -march=native -flto -fprofile-use\n");
    printf("  嵌入? → -Os -ffunction-sections -fdata-sections\n");
    printf("         -Wl,--gc-sections\n");
    printf("\n");
}

int main(void) {
    printf("================================================\n");
    printf("  GCC高级特性深入 - Sanitizer/LTO/PGO/交叉编译\n");
    printf("================================================\n\n");

    demo_address_sanitizer();
    demo_undefined_behavior_sanitizer();
    demo_thread_sanitizer();
    demo_lto();
    demo_march_native();
    demo_pgo();
    demo_cross_compilation();
    demo_best_practices();

    printf("================================================\n");
    printf("  演示结束\n");
    printf("================================================\n");
    return 0;
}
