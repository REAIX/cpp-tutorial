/**
 * @file 01_deep_dive_makefile_patterns.c
 * @brief Makefile深入剖析 - 递归Make、自动依赖、并行构建、跨平台
 * @description 对应文档: 22-makefile
 *              本文件以独立可运行代码演示Makefile的高级模式，
 *              包括递归make、自动依赖生成、并行构建、跨平台Makefile
 *
 * 编译: gcc -Wall -Wextra -std=c11 01_deep_dive_makefile_patterns.c -o deep_dive_makefile
 * 运行: ./deep_dive_makefile
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * 第一部分: 递归 Make (Recursive Make)
 * ======================================================================== */

void demo_recursive_make(void) {
    printf("===== 递归 Make =====\n\n");

    printf("大型项目通常将代码组织到多个子目录中，\n");
    printf("每个子目录有自己的 Makefile，顶层 Makefile 递归调用子目录的 Makefile。\n\n");

    printf("项目目录结构示例:\n");
    printf("  project/\n");
    printf("  ├── Makefile              # 顶层 Makefile\n");
    printf("  ├── src/\n");
    printf("  │   ├── Makefile          # src 子目录 Makefile\n");
    printf("  │   ├── main.c\n");
    printf("  │   └── utils.c\n");
    printf("  ├── lib/\n");
    printf("  │   ├── Makefile          # lib 子目录 Makefile\n");
    printf("  │   └── mylib.c\n");
    printf("  └── tests/\n");
    printf("      ├── Makefile          # tests 子目录 Makefile\n");
    printf("      └── test_main.c\n\n");

    printf("顶层 Makefile 示例:\n");
    printf("  SUBDIRS = src lib tests\n\n");
    printf("  .PHONY: all clean $(SUBDIRS)\n\n");
    printf("  all: $(SUBDIRS)\n\n");
    printf("  src: lib\n");
    printf("      $(MAKE) -C $@\n\n");
    printf("  lib:\n");
    printf("      $(MAKE) -C $@\n\n");
    printf("  tests: src\n");
    printf("      $(MAKE) -C $@\n\n");
    printf("  clean:\n");
    printf("      for dir in $(SUBDIRS); do \\\n");
    printf("          $(MAKE) -C $$dir clean; \\\n");
    printf("      done\n\n");

    printf("递归 Make 的问题 (Peter Miller 的论文 'Recursive Make Considered Harmful'):\n");
    printf("  1. 依赖关系跨目录时难以正确追踪\n");
    printf("  2. 可能导致不必要的重新编译\n");
    printf("  3. 并行构建时依赖顺序难以保证\n\n");

    printf("替代方案: 非递归 Make (Single Makefile)\n");
    printf("  使用一个顶层 Makefile 管理所有源文件\n");
    printf("  通过 include 引入子目录的 Makefile 片段\n\n");

    printf("非递归 Make 示例:\n");
    printf("  # 顶层 Makefile\n");
    printf("  include src/Makefile.inc\n");
    printf("  include lib/Makefile.inc\n\n");
    printf("  # src/Makefile.inc\n");
    printf("  SRC_SRCS := src/main.c src/utils.c\n");
    printf("  SRC_OBJS := $(SRC_SRCS:.c=.o)\n\n");
}

/* ========================================================================
 * 第二部分: 自动依赖生成
 * ======================================================================== */

void demo_auto_dependencies(void) {
    printf("===== 自动依赖生成 =====\n\n");

    printf("问题: 修改头文件后，依赖该头文件的 .c 文件需要重新编译。\n");
    printf("手动维护依赖关系容易遗漏，需要自动生成。\n\n");

    printf("方法1: GCC -M 选项生成依赖关系\n");
    printf("  gcc -M file.c             # 输出完整依赖 (包括系统头文件)\n");
    printf("  gcc -MM file.c            # 只输出用户头文件的依赖\n");
    printf("  gcc -MD file.c            # 编译同时生成 .d 文件\n");
    printf("  gcc -MMD file.c           # 编译同时生成 .d (不含系统头文件)\n\n");

    printf("生成的 .d 文件内容示例:\n");
    printf("  main.o: main.c utils.h config.h types.h\n\n");

    printf("方法2: 在 Makefile 中自动包含 .d 文件\n\n");

    printf("  # 编译时自动生成依赖文件\n");
    printf("  CFLAGS += -MMD -MP\n\n");
    printf("  # -MP: 为每个头文件生成伪目标，避免头文件删除后报错\n\n");

    printf("  # 收集所有 .d 文件\n");
    printf("  DEPS := $(OBJ:.o=.d)\n\n");

    printf("  # 包含 .d 文件 (如果存在)\n");
    printf("  -include $(DEPS)\n\n");

    printf("方法3: 使用 GCC -MF 指定依赖文件名\n");
    printf("  CFLAGS += -MMD -MP -MF $(@:.o=.d)\n\n");

    printf("完整示例:\n");
    printf("  SRCS := main.c utils.c\n");
    printf("  OBJS := $(SRCS:.c=.o)\n");
    printf("  DEPS := $(OBJS:.o=.d)\n\n");
    printf("  CFLAGS := -Wall -MMD -MP\n\n");
    printf("  target: $(OBJS)\n");
    printf("      $(CC) -o $@ $^\n\n");
    printf("  %%o: %%.c\n");
    printf("      $(CC) $(CFLAGS) -c $<\n\n");
    printf("  -include $(DEPS)\n\n");

    printf("举一反三:\n");
    printf("  - 增量编译的核心就是正确的依赖追踪\n");
    printf("  - CMake 自动处理依赖，无需手动配置\n");
    printf("  - 大型项目中 .d 文件可能有数千个\n");
    printf("\n");
}

/* ========================================================================
 * 第三部分: 并行构建
 * ======================================================================== */

void demo_parallel_builds(void) {
    printf("===== 并行构建 =====\n\n");

    printf("make -j: 并行执行多个任务，显著加快编译速度。\n\n");

    printf("使用方法:\n");
    printf("  make -j4          # 使用4个并行任务\n");
    printf("  make -j$(nproc)   # 使用所有CPU核心\n");
    printf("  make -j           # 不限制并行数 (可能耗尽内存)\n\n");

    printf("并行构建的注意事项:\n");
    printf("  1. 依赖关系必须正确声明\n");
    printf("     错误的依赖会导致竞态条件\n\n");

    printf("  2. 伪目标之间的依赖\n");
    printf("     all: lib src tests    # make知道这三个可以并行\n\n");

    printf("  3. 输出交错\n");
    printf("     并行构建时多个命令的输出会交错\n");
    printf("     使用 -O 选项使输出有序: make -j4 -O\n\n");

    printf("  4. 递归 Make 的并行问题\n");
    printf("     使用 MAKELEVEL 变量追踪递归深度\n");
    printf("     使用 .WAIT 伪目标强制顺序: all: lib .WAIT src\n\n");

    printf("  5. jobserver 机制\n");
    printf("     递归 make 通过 jobserver 共享 job slot\n");
    printf("     确保总并行数不超过 -j 指定的值\n\n");

    printf("性能对比 (大型项目):\n");
    printf("  make           120秒 (单线程)\n");
    printf("  make -j4        35秒\n");
    printf("  make -j8        20秒\n");
    printf("  make -j$(nproc) 15秒 (假设16核)\n\n");

    printf("最佳实践:\n");
    printf("  - 开发时: make -j$(nproc)\n");
    printf("  - CI/CD: make -j$(nproc) 或 make -j4\n");
    printf("  - 内存不足时: 减少 -j 的值\n");
    printf("\n");
}

/* ========================================================================
 * 第四部分: 跨平台 Makefile
 * ======================================================================== */

void demo_cross_platform_makefile(void) {
    printf("===== 跨平台 Makefile =====\n\n");

    printf("检测操作系统:\n");
    printf("  UNAME_S := $(shell uname -s)\n\n");

    printf("  ifeq ($(UNAME_S),Linux)\n");
    printf("      PLATFORM := linux\n");
    printf("      EXE_SUFFIX :=\n");
    printf("      RM := rm -f\n");
    printf("  endif\n");
    printf("  ifeq ($(UNAME_S),Darwin)\n");
    printf("      PLATFORM := macos\n");
    printf("      EXE_SUFFIX :=\n");
    printf("      RM := rm -f\n");
    printf("  endif\n");
    printf("  ifeq ($(OS),Windows_NT)\n");
    printf("      PLATFORM := windows\n");
    printf("      EXE_SUFFIX := .exe\n");
    printf("      RM := del /q\n");
    printf("  endif\n\n");

    printf("跨平台注意事项:\n");
    printf("  1. 路径分隔符: Linux用 /, Windows用 \\\\\n");
    printf("  2. 可执行文件后缀: Windows需要 .exe\n");
    printf("  3. 删除命令: Linux用 rm, Windows用 del\n");
    printf("  4. 动态库后缀: .so (Linux), .dylib (macOS), .dll (Windows)\n");
    printf("  5. 编译器差异: GCC vs MSVC vs Clang\n\n");

    printf("推荐: 使用 CMake 替代手写跨平台 Makefile\n");
    printf("  CMake 自动处理平台差异，生成对应的构建文件\n\n");

    printf("GNU Make vs BSD Make vs NMAKE:\n");
    printf("  GNU Make:  Linux默认，功能最丰富\n");
    printf("  BSD Make:  macOS默认，语法有差异\n");
    printf("  NMAKE:     MSVC配套，完全不同的语法\n");
    printf("  建议: 统一使用 GNU Make (gmake)\n\n");
}

/* ========================================================================
 * 第五部分: Makefile 调试与技巧
 * ======================================================================== */

void demo_makefile_debug_tips(void) {
    printf("===== Makefile 调试技巧 =====\n\n");

    printf("1. 干运行 (Dry Run):\n");
    printf("   make -n          # 显示要执行的命令但不执行\n");
    printf("   make -n -j4      # 查看并行构建的命令\n\n");

    printf("2. 调试变量:\n");
    printf("   make print-VAR   # 打印变量值\n");
    printf("   # 在 Makefile 中添加:\n");
    printf("   print-%%:\n");
    printf("       @echo $* = $($*)\n\n");

    printf("3. 跟踪执行:\n");
    printf("   make -d          # 打印所有调试信息(非常详细)\n");
    printf("   make --debug=v   # 只打印基本调试信息\n\n");

    printf("4. 忽略错误:\n");
    printf("   make -i          # 忽略所有命令错误\n");
    printf("   make -k          # 遇到错误继续构建其他目标\n\n");

    printf("5. 显示命令:\n");
    printf("   make -B          # 无条件重新构建所有目标\n");
    printf("   make -t          # 只更新文件时间戳(不编译)\n\n");

    printf("6. 常见错误排查:\n");
    printf("   'missing separator':\n");
    printf("     → 命令行没有用 Tab 缩进，用了空格\n\n");
    printf("   'No rule to make target':\n");
    printf("     → 依赖文件不存在或路径错误\n\n");
    printf("   'circular dependency':\n");
    printf("     → 目标和依赖形成循环依赖\n\n");

    printf("7. 性能优化:\n");
    printf("   使用 := 替代 = (简单展开 vs 递归展开)\n");
    printf("   减少 $(shell) 调用\n");
    printf("   使用 order-only 依赖: target: normal | order-only\n");
    printf("\n");
}

/* ========================================================================
 * 第六部分: 完整项目 Makefile 模板
 * ======================================================================== */

void demo_makefile_template(void) {
    printf("===== 完整项目 Makefile 模板 =====\n\n");

    printf("# ===== 项目配置 =====\n");
    printf("PROJECT  := myproject\n");
    printf("VERSION  := 1.0.0\n\n");

    printf("# ===== 编译器与选项 =====\n");
    printf("CC       := gcc\n");
    printf("CFLAGS   := -Wall -Wextra -Wpedantic -std=c11\n");
    printf("LDFLAGS  :=\n");
    printf("LDLIBS   :=\n\n");

    printf("# ===== 构建类型 =====\n");
    printf("DEBUG    ?= 0\n");
    printf("ifeq ($(DEBUG),1)\n");
    printf("    CFLAGS  += -g -O0 -DDEBUG\n");
    printf("else\n");
    printf("    CFLAGS  += -O2 -DNDEBUG\n");
    printf("endif\n\n");

    printf("# ===== 源文件 =====\n");
    printf("SRCDIR   := src\n");
    printf("BUILDDIR := build\n");
    printf("SRCS     := $(wildcard $(SRCDIR)/*.c)\n");
    printf("OBJS     := $(patsubst $(SRCDIR)/%%.c,$(BUILDDIR)/%%.o,$(SRCS))\n");
    printf("DEPS     := $(OBJS:.o=.d)\n\n");

    printf("# ===== 自动依赖 =====\n");
    printf("CFLAGS   += -MMD -MP\n\n");

    printf("# ===== 目标 =====\n");
    printf("TARGET   := $(BUILDDIR)/$(PROJECT)\n\n");

    printf(".PHONY: all clean debug release\n\n");

    printf("all: $(TARGET)\n\n");

    printf("$(TARGET): $(OBJS)\n");
    printf("	$(CC) $(LDFLAGS) -o $@ $^ $(LDLIBS)\n\n");

    printf("$(BUILDDIR)/%%.o: $(SRCDIR)/%%.c | $(BUILDDIR)\n");
    printf("	$(CC) $(CFLAGS) -c -o $@ $<\n\n");

    printf("$(BUILDDIR):\n");
    printf("	mkdir -p $@\n\n");

    printf("clean:\n");
    printf("	$(RM) -r $(BUILDDIR)\n\n");

    printf("debug:\n");
    printf("	$(MAKE) DEBUG=1\n\n");

    printf("release:\n");
    printf("	$(MAKE) DEBUG=0\n\n");

    printf("-include $(DEPS)\n\n");

    printf("举一反三 - Makefile 演进路线:\n");
    printf("  小项目: 简单 Makefile (本文件第一版)\n");
    printf("  中项目: 模式规则 + 自动依赖 + 构建目录\n");
    printf("  大项目: 非递归Make 或 迁移到 CMake\n");
    printf("  超大项目: CMake + Ninja + 分布式编译\n");
    printf("\n");
}

int main(void) {
    printf("================================================\n");
    printf("  Makefile深入剖析 - 递归Make/自动依赖/并行构建\n");
    printf("================================================\n\n");

    demo_recursive_make();
    demo_auto_dependencies();
    demo_parallel_builds();
    demo_cross_platform_makefile();
    demo_makefile_debug_tips();
    demo_makefile_template();

    printf("================================================\n");
    printf("  演示结束\n");
    printf("================================================\n");
    return 0;
}
