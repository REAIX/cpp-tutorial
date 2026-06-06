/** @file 02_deep_dive_memory_debug.c
 *  @brief 内存调试：Valgrind概念、ASAN、常见内存Bug、防御性编程
 *  @description 对应文档: 09-内存管理 | 举一反三：识别和调试内存问题
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demo_use_after_free(void) {
    printf("=== Use-After-Free (释放后使用) ===\n");

    int *p = (int *)malloc(sizeof(int));
    if (!p) return;
    *p = 42;
    printf("分配: *p = %d\n", *p);

    free(p);
    p = NULL;

    printf("释放后, p 已置 NULL\n");
    printf("如果未置 NULL, *p 就是 use-after-free:\n");
    printf("  int *p = malloc(sizeof(int));\n");
    printf("  *p = 42;\n");
    printf("  free(p);\n");
    printf("  printf(\"%%d\", *p);  // 未定义行为! 可能读到旧值或垃圾值\n");

    printf("\nASAN 检测: 报告 heap-use-after-free\n");
    printf("Valgrind 检测: 报告 Invalid read of size 4\n");

    printf("\n");
}

void demo_double_free(void) {
    printf("=== Double Free (重复释放) ===\n");

    int *p = (int *)malloc(sizeof(int));
    if (!p) return;
    *p = 42;
    free(p);
    p = NULL;

    printf("正确: free(p); p = NULL;\n");
    printf("错误: free(p); free(p);  // 第二次 free 导致堆损坏\n");

    printf("\n防御: free 后立即置 NULL, free(NULL) 是安全的\n");

    printf("\nASAN 检测: 报告 double-free\n");
    printf("Valgrind 检测: 报告 Invalid free() / delete\n");

    printf("\n");
}

void demo_buffer_overflow_heap(void) {
    printf("=== 堆缓冲区溢出 ===\n");

    int *arr = (int *)malloc(5 * sizeof(int));
    if (!arr) return;

    printf("分配了 5 个 int 的数组\n");
    printf("合法访问: arr[0] 到 arr[4]\n");
    printf("越界写入: arr[5] = 99;  // 堆缓冲区溢出!\n\n");

    printf("后果:\n");
    printf("1. 覆盖堆元数据, 导致 malloc/free 崩溃\n");
    printf("2. 覆盖相邻的堆对象\n");
    printf("3. 安全漏洞: 可能被利用执行任意代码\n");

    for (int i = 0; i < 5; i++) arr[i] = i;
    free(arr);

    printf("\nASAN 检测: 报告 heap-buffer-overflow\n");
    printf("Valgrind 检测: 报告 Invalid write of size 4\n");

    printf("\n");
}

void demo_stack_buffer_overflow(void) {
    printf("=== 栈缓冲区溢出 ===\n");

    char buf[8];
    printf("char buf[8]; 分配了8字节栈空间\n");
    printf("strcpy(buf, \"Hello\");  // 安全, 6字节(含\\0)\n");
    strcpy(buf, "Hello");
    printf("  buf = \"%s\"\n", buf);

    printf("\nstrcpy(buf, \"Hello, World!\");  // 溢出! 14字节到8字节缓冲区\n");
    printf("后果: 覆盖栈上的其他变量、返回地址\n");
    printf("这是经典的栈溢出攻击方式\n");

    printf("\nASAN 检测: 报告 stack-buffer-overflow\n");

    printf("\n");
}

void demo_memory_leak_debug(void) {
    printf("=== 内存泄漏调试 ===\n");

    printf("泄漏类型:\n\n");

    printf("1. 直接泄漏: malloc 后没有 free\n");
    printf("   void func() { void *p = malloc(100); }  // p 丢失\n\n");

    printf("2. 间接泄漏: 结构体释放了, 但成员未释放\n");
    printf("   struct { char *name; } *s = malloc(sizeof(*s));\n");
    printf("   s->name = malloc(50);\n");
    printf("   free(s);  // s->name 泄漏!\n\n");

    printf("3. 可能泄漏: 仍存在指向内存的指针, 但不再使用\n");
    printf("   全局变量或长生命周期对象中不再访问的内存\n\n");

    printf("Valgrind 泄漏报告级别:\n");
    printf("  definitely lost: 确定泄漏\n");
    printf("  indirectly lost: 间接泄漏\n");
    printf("  possibly lost: 可能泄漏\n");
    printf("  still reachable: 程序退出时仍可达 (通常可忽略)\n");

    printf("\n");
}

void demo_asan_usage(void) {
    printf("=== AddressSanitizer (ASAN) 使用 ===\n");

    printf("编译命令:\n");
    printf("  gcc -fsanitize=address -g -O1 program.c -o program\n\n");

    printf("ASAN 检测的问题:\n");
    printf("  1. heap-buffer-overflow  堆缓冲区溢出\n");
    printf("  2. stack-buffer-overflow 栈缓冲区溢出\n");
    printf("  3. heap-use-after-free   释放后使用\n");
    printf("  4. double-free           重复释放\n");
    printf("  5. memory-leaks          内存泄漏 (需加 ASAN_OPTIONS=detect_leaks=1)\n\n");

    printf("ASAN 优势:\n");
    printf("  - 开销小 (约2x速度, 2x内存)\n");
    printf("  - 报告精确 (含调用栈)\n");
    printf("  - 可在开发/测试阶段启用\n");

    printf("\n");
}

void demo_valgrind_usage(void) {
    printf("=== Valgrind 使用 ===\n");

    printf("运行命令:\n");
    printf("  valgrind --leak-check=full --show-leak-kinds=all ./program\n\n");

    printf("Valgrind 检测的问题:\n");
    printf("  1. 内存泄漏\n");
    printf("  2. 非法读写 (越界、use-after-free)\n");
    printf("  3. 未初始化值的使用\n");
    printf("  4. 重复释放\n");
    printf("  5. 不匹配的 malloc/free/new/delete\n\n");

    printf("Valgrind 优势:\n");
    printf("  - 无需重新编译\n");
    printf("  - 检测范围广\n");
    printf("Valgrind 劣势:\n");
    printf("  - 开销大 (10-20x 速度降低)\n");
    printf("  - 仅支持 Linux/macOS\n");

    printf("\n");
}

static int process_file(void) {
    FILE *f = NULL;
    char *buf = NULL;
    int result = -1;

    f = fopen("test.txt", "r");
    if (!f) goto cleanup;

    buf = (char *)malloc(1024);
    if (!buf) goto cleanup;

    if (!fgets(buf, 1024, f)) goto cleanup;

    result = 0;

cleanup:
    if (buf) free(buf);
    if (f) fclose(f);
    return result;
}

void demo_defensive_programming(void) {
    printf("=== 防御性编程 ===\n");

    printf("1. 始终检查 malloc 返回值\n");
    printf("   int *p = malloc(sizeof(int));\n");
    printf("   if (!p) { /* 处理错误 */ }\n\n");

    printf("2. free 后立即置 NULL\n");
    printf("   free(p); p = NULL;\n\n");

    printf("3. 使用安全的字符串函数\n");
    printf("   snprintf 代替 sprintf\n");
    printf("   strncpy 代替 strcpy\n\n");

    printf("4. 初始化所有分配的内存\n");
    printf("   calloc 代替 malloc (需要零初始化时)\n");
    printf("   或 malloc 后立即 memset\n\n");

    printf("5. 遵循 RAII 原则 (C 中用 goto cleanup 模拟)\n");

    (void)process_file;

    printf("6. 使用静态分析工具\n");
    printf("   gcc -Wall -Wextra -Wpedantic\n");
    printf("   cppcheck, clang-tidy\n\n");

    printf("7. 开发时启用 ASAN, 测试时运行 Valgrind\n");

    printf("\n");
}

int main(void) {
    demo_use_after_free();
    demo_double_free();
    demo_buffer_overflow_heap();
    demo_stack_buffer_overflow();
    demo_memory_leak_debug();
    demo_asan_usage();
    demo_valgrind_usage();
    demo_defensive_programming();

    return 0;
}
