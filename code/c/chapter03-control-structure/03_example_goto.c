/** @file 03_example_goto.c
 *  @brief goto语句：用法、何时可接受、替代方案
 *  @description 对应文档: 03-control-structure | 演示goto的用法、合理使用场景和替代方案
 *  编译命令: gcc -std=c17 03_example_goto.c -o 03_example_goto
 */

#include <stdio.h>
#include <stdlib.h>

void demo_goto_basics(void) {
    printf("═══════════════════════════════════════\n");
    printf("  goto 基础语法\n");
    printf("═══════════════════════════════════════\n\n");

    printf("goto 语法: goto 标签名;\n");
    printf("标签定义: 标签名: 语句\n\n");

    int count = 0;
start:
    printf("  执行第 %d 次\n", ++count);
    if (count < 3) {
        goto start;
    }

    printf("\ngoto的限制:\n");
    printf("  ✓ 可以在同一个函数内跳转\n");
    printf("  ✗ 不能跨函数跳转\n");
    printf("  ✗ 不能跳过变长数组(VLA)的声明\n");
    printf("  ✗ 不能跳过带有初始化的变量声明(C99+)\n");
}

void demo_goto_error_handling(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  goto的合理使用: 错误处理与资源清理\n");
    printf("═══════════════════════════════════════\n\n");

    printf("Linux内核中goto最常见的模式 —— 集中错误处理:\n\n");

    printf("  int func(void) {\n");
    printf("      void *a = malloc(100);\n");
    printf("      if (!a) goto fail_a;\n");
    printf("      void *b = malloc(200);\n");
    printf("      if (!b) goto fail_b;\n");
    printf("      void *c = malloc(300);\n");
    printf("      if (!c) goto fail_c;\n");
    printf("      // 使用a, b, c...\n");
    printf("      free(c);\n");
    printf("  fail_c:\n");
    printf("      free(b);\n");
    printf("  fail_b:\n");
    printf("      free(a);\n");
    printf("  fail_a:\n");
    printf("      return -1;\n");
    printf("  }\n\n");

    printf("模拟执行:\n");
    int step1_ok = 1, step2_ok = 0, step3_ok = 1;
    int a_allocated = 0, b_allocated = 0, c_allocated = 0;

    a_allocated = 1;
    printf("  步骤1: 分配资源A → 成功\n");
    if (!step1_ok) goto cleanup_a;

    b_allocated = 1;
    printf("  步骤2: 分配资源B → 成功\n");
    if (!step2_ok) goto cleanup_b;

    c_allocated = 1;
    printf("  步骤3: 分配资源C → 成功\n");
    if (!step3_ok) goto cleanup_c;

    printf("  所有步骤完成!\n");

cleanup_c:
    if (c_allocated) printf("  清理资源C\n");
cleanup_b:
    if (b_allocated) printf("  清理资源B\n");
cleanup_a:
    if (a_allocated) printf("  清理资源A\n");

    printf("\n这种模式的优点:\n");
    printf("  ✓ 避免深层嵌套的if-else\n");
    printf("  ✓ 清理代码集中，不会遗漏\n");
    printf("  ✓ 每个错误路径都有对应的清理\n");
}

void demo_goto_nested_loop_exit(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  goto的合理使用: 退出多层嵌套循环\n");
    printf("═══════════════════════════════════════\n\n");

    printf("在二维数组中查找目标值:\n");
    int matrix[3][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    };
    int target = 7;
    int found_row = -1, found_col = -1;

    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            if (matrix[i][j] == target) {
                found_row = i;
                found_col = j;
                goto found;
            }
        }
    }

found:
    if (found_row >= 0) {
        printf("  找到 %d 在 [%d][%d]\n", target, found_row, found_col);
    } else {
        printf("  未找到 %d\n", target);
    }

    printf("\n不用goto的替代方案:\n");

    printf("  方案1: 使用标志变量\n");
    int found2 = 0;
    for (int i = 0; i < 3 && !found2; i++) {
        for (int j = 0; j < 4 && !found2; j++) {
            if (matrix[i][j] == target) {
                found_row = i;
                found_col = j;
                found2 = 1;
            }
        }
    }
    printf("  找到 %d 在 [%d][%d]\n", target, found_row, found_col);

    printf("\n  方案2: 封装为函数，用return退出\n");
    printf("  int find(int *result_row, int *result_col) {\n");
    printf("      for (...) for (...) if (匹配) { *r=i; *c=j; return 1; }\n");
    printf("      return 0;\n");
    printf("  }\n");
}

void demo_goto_antipatterns(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  goto的反模式(应避免)\n");
    printf("═══════════════════════════════════════\n\n");

    printf("❌ 反模式1: 向后跳转(制造意大利面条代码)\n");
    printf("   goto back;\n");
    printf("   ...\n");
    printf("   back:\n");
    printf("   → 代码流程混乱，难以理解\n\n");

    printf("❌ 反模式2: 用goto代替循环\n");
    printf("   int i = 0;\n");
    printf("   loop: if (i < 10) { ... i++; goto loop; }\n");
    printf("   → 应该用 for/while 循环\n\n");

    printf("❌ 反模式3: 跳过变量初始化\n");
    printf("   goto skip;\n");
    printf("   int x = compute();  // 跳过初始化!\n");
    printf("   skip: use(x);       // x的值未定义\n\n");

    printf("❌ 反模式4: 过多的goto标签\n");
    printf("   → 代码变成跳来跳去的意大利面条\n\n");

    printf("Dijkstra的经典论文: \"Goto Considered Harmful\"\n");
    printf("  核心观点: 程序的正确性依赖于清晰的控制流\n");
    printf("  goto破坏了结构化编程的顺序、选择、循环三种基本结构\n");
}

void demo_goto_guidelines(void) {
    printf("\n═══════════════════════════════════════\n");
    printf("  goto使用指南\n");
    printf("═══════════════════════════════════════\n\n");

    printf("✅ 可以接受的goto使用:\n");
    printf("  1. 跳转到函数末尾的清理代码(最常见)\n");
    printf("  2. 退出多层嵌套循环\n");
    printf("  3. Linux内核、系统编程中的标准做法\n\n");

    printf("❌ 应该避免的goto使用:\n");
    printf("  1. 向后跳转\n");
    printf("  2. 替代正常的循环结构\n");
    printf("  3. 替代函数调用\n");
    printf("  4. 跳过变量初始化\n\n");

    printf("替代方案优先级:\n");
    printf("  1. 优先用结构化方式(循环、函数、标志变量)\n");
    printf("  2. 错误处理: 考虑 do { ... } while(0) + break 模式\n");
    printf("  3. 嵌套循环退出: 考虑封装为函数\n");
    printf("  4. 只有在上述方案使代码更复杂时，才考虑goto\n\n");

    printf("do-while(0) + break 替代goto的错误处理:\n");
    printf("  do {\n");
    printf("      if (error1) break;\n");
    printf("      if (error2) break;\n");
    printf("      // 正常逻辑\n");
    printf("  } while(0);\n");
    printf("  // 清理代码\n");
}

int main(void) {
    demo_goto_basics();
    demo_goto_error_handling();
    demo_goto_nested_loop_exit();
    demo_goto_antipatterns();
    demo_goto_guidelines();

    return 0;
}
