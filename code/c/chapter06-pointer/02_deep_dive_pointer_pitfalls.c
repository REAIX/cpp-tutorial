/** @file 02_deep_dive_pointer_pitfalls.c
 *  @brief 指针陷阱：悬垂指针、野指针、空指针解引用、指针别名、常见Bug
 *  @description 对应文档: 06-指针 | 举一反三：识别和避免指针的常见陷阱
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demo_dangling_pointer(void) {
    printf("=== 悬垂指针 (Dangling Pointer) ===\n");
    printf("指向已释放内存或已超出作用域的变量的指针\n\n");

    printf("场景1: 栈变量超出作用域\n");
    int *p = NULL;
    {
        int local = 42;
        p = &local;
        printf("  作用域内: *p = %d\n", *p);
    }
    printf("  作用域外: p 仍指向 local 的旧地址, 但 local 已不存在\n");
    printf("  此时解引用 *p 是未定义行为!\n");
    printf("  (不同编译器/运行结果可能不同, 这里不实际解引用)\n");

    printf("\n场景2: 堆内存已释放\n");
    int *heap_p = (int *)malloc(sizeof(int));
    if (heap_p) {
        *heap_p = 99;
        printf("  释放前: *heap_p = %d\n", *heap_p);
        free(heap_p);
        printf("  释放后: heap_p 仍保存旧地址, 但内存已归还\n");
        printf("  此时 *heap_p 是未定义行为 (use-after-free)\n");
    }
    heap_p = NULL;
    printf("  正确做法: free 后立即置 NULL\n");

    printf("\n场景3: 函数返回局部变量地址\n");
    printf("  int* bad_func() { int x = 10; return &x; }  // 错误!\n");
    printf("  局部变量在函数返回后销毁, 返回的地址无效\n");

    printf("\n");
}

void demo_wild_pointer(void) {
    printf("=== 野指针 (Wild Pointer) ===\n");
    printf("未初始化的指针, 包含随机地址\n\n");

    printf("危险示例 (不运行, 仅说明):\n");
    printf("  int *p;        // 未初始化, 包含垃圾值\n");
    printf("  *p = 42;       // 未定义行为! 写入随机地址\n\n");

    printf("正确做法: 声明时立即初始化\n");
    int *safe1 = NULL;
    int value = 10;
    int *safe2 = &value;

    printf("  int *safe1 = NULL;  // 初始化为 NULL\n");
    printf("  int *safe2 = &value;  // 初始化为有效地址\n");

    (void)safe1;
    (void)safe2;

    printf("\n");
}

void demo_null_pointer_dereference(void) {
    printf("=== 空指针解引用 ===\n");

    int *p = NULL;
    (void)p;

    printf("解引用 NULL 指针是严重的运行时错误\n");
    printf("通常导致段错误 (Segmentation Fault) 或访问违例\n\n");

    printf("防御性编程模式:\n");
    printf("  if (p != NULL) {\n");
    printf("      *p = 42;  // 安全\n");
    printf("  } else {\n");
    printf("      // 处理空指针情况\n");
    printf("  }\n\n");

    int *q = NULL;
    if (q != NULL) {
        *q = 42;
    } else {
        printf("检测到空指针, 跳过解引用\n");
    }

    printf("\n");
}

void demo_pointer_aliasing(void) {
    printf("=== 指针别名 (Pointer Aliasing) ===\n");
    printf("两个指针指向同一块内存, 修改一个会影响另一个\n\n");

    int x = 100;
    int *p1 = &x;
    int *p2 = &x;

    printf("x = %d, *p1 = %d, *p2 = %d\n", x, *p1, *p2);
    *p1 = 200;
    printf("*p1 = 200 后: x = %d, *p2 = %d\n", x, *p2);
    *p2 = 300;
    printf("*p2 = 300 后: x = %d, *p1 = %d\n", x, *p1);

    printf("\n别名问题的影响:\n");
    printf("1. 编译器难以优化 (不知道指针是否指向同一内存)\n");
    printf("2. 多线程下的数据竞争\n");
    printf("3. 函数参数间的隐式依赖\n");

    printf("\n");
}

void demo_uninitialized_pointer_bug(void) {
    printf("=== 常见Bug: 未初始化指针 ===\n");

    printf("Bug 模式:\n");
    printf("  char *str;           // 野指针\n");
    printf("  strcpy(str, \"hello\"); // 写入随机地址! 崩溃!\n\n");

    printf("修复方案:\n");
    char *str = (char *)malloc(32);
    if (str != NULL) {
        strcpy(str, "hello");
        printf("  str = \"%s\" (正确分配内存后使用)\n", str);
        free(str);
        str = NULL;
    }

    printf("\n");
}

void demo_buffer_overflow_via_pointer(void) {
    printf("=== 常见Bug: 指针越界访问 ===\n");

    int arr[5] = {1, 2, 3, 4, 5};
    int *p = arr;

    printf("数组 arr 有 5 个元素 (索引 0-4)\n");
    printf("*(p + 4) = %d  (合法, 最后一个元素)\n", *(p + 4));
    printf("*(p + 5) = ???  (越界! 未定义行为)\n\n");

    printf("常见越界场景:\n");
    printf("1. 循环条件写错: for(i=0; i<=5; i++) 对5元素数组越界\n");
    printf("2. 指针算术错误: p += n 但 n 超出数组范围\n");
    printf("3. 字符串操作: strcpy 目标缓冲区太小\n\n");

    printf("防御措施:\n");
    printf("1. 始终检查边界\n");
    printf("2. 使用 size_t 而非 int 表示大小\n");
    printf("3. 使用 strncpy/snprintf 等安全函数\n");

    printf("\n");
}

void demo_double_free(void) {
    printf("=== 常见Bug: 重复释放 (Double Free) ===\n");

    int *p = (int *)malloc(sizeof(int));
    if (p) {
        *p = 42;
        printf("分配并赋值: *p = %d\n", *p);
        free(p);
        printf("第一次 free(p): 正确\n");
        p = NULL;
        printf("p = NULL: 防止 double free\n");
    }

    printf("\n如果不置 NULL, 再次 free 会导致:\n");
    printf("  free(p);  // 第一次 OK\n");
    printf("  free(p);  // Double Free! 堆损坏!\n\n");

    printf("置 NULL 后 free(NULL) 是安全的 (什么都不做)\n");

    printf("\n");
}

void demo_memory_leak_pattern(void) {
    printf("=== 常见Bug: 内存泄漏 ===\n");

    printf("泄漏模式1: 忘记释放\n");
    printf("  void leak() { int *p = malloc(sizeof(int)); }  // 函数结束, p 丢失\n\n");

    printf("泄漏模式2: 覆盖指针\n");
    printf("  int *p = malloc(sizeof(int));\n");
    printf("  p = malloc(sizeof(int));  // 第一块内存泄漏!\n\n");

    printf("泄漏模式3: 提前返回\n");
    printf("  int *p = malloc(sizeof(int));\n");
    printf("  if (error) return;  // 忘记 free(p) 就返回\n\n");

    printf("正确模式:\n");
    int *p = (int *)malloc(sizeof(int));
    if (p == NULL) {
        printf("  分配失败, 提前返回\n");
        return;
    }
    *p = 42;
    printf("  使用: *p = %d\n", *p);
    free(p);
    p = NULL;
    printf("  使用完毕, 释放并置 NULL\n");

    printf("\n");
}

void demo_pointer_best_practices(void) {
    printf("=== 指针最佳实践总结 ===\n");
    printf("1. 声明指针时立即初始化 (至少初始化为 NULL)\n");
    printf("2. 使用指针前检查是否为 NULL\n");
    printf("3. free 之后立即将指针置为 NULL\n");
    printf("4. 不要返回局部变量的地址\n");
    printf("5. 注意指针的生命周期, 避免悬垂指针\n");
    printf("6. 使用 const 修饰不应修改的指针或数据\n");
    printf("7. 注意指针算术的边界, 避免越界访问\n");
    printf("8. 使用内存检测工具 (Valgrind, ASAN) 检查问题\n");
    printf("\n");
}

int main(void) {
    demo_dangling_pointer();
    demo_wild_pointer();
    demo_null_pointer_dereference();
    demo_pointer_aliasing();
    demo_uninitialized_pointer_bug();
    demo_buffer_overflow_via_pointer();
    demo_double_free();
    demo_memory_leak_pattern();
    demo_pointer_best_practices();

    return 0;
}
