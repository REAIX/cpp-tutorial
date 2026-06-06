/**
 * @file 03_example_assert.c
 * @brief 断言与防御性编程
 * @description 对应文档: 13-错误处理与信号
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

void demo_assert_basic(void) {
    printf("=== assert()基本用法 ===\n");
    int x = 10;
    assert(x > 0);
    printf("  assert(x > 0) 通过, x=%d\n", x);

    int *ptr = malloc(sizeof(int));
    assert(ptr != NULL);
    printf("  assert(ptr != NULL) 通过, 内存分配成功\n");
    free(ptr);

    printf("  注意: 定义NDEBUG宏后assert变为空操作\n");
    printf("  编译时加 -DNDEBUG 可禁用所有断言: gcc -DNDEBUG ...\n\n");
}

void demo_static_assert_c11(void) {
    printf("=== C11 static_assert/_Static_assert ===\n");

    static_assert(sizeof(int) >= 4, "int必须至少4字节");
    printf("  static_assert(sizeof(int) >= 4) 通过\n");

    static_assert(CHAR_BIT == 8, "char必须为8位");
    printf("  static_assert(CHAR_BIT == 8) 通过\n");

    typedef struct {
        int id;
        double value;
    } record_t;

    static_assert(sizeof(record_t) >= sizeof(int) + sizeof(double),
                  "record_t内存布局不符合预期");
    printf("  static_assert(sizeof(record_t) >= %zu) 通过\n\n",
           sizeof(int) + sizeof(double));
}

static int safe_divide(int a, int b) {
    if (b == 0) {
        fprintf(stderr, "  错误: 除数不能为零\n");
        return -1;
    }
    return a / b;
}

void demo_defensive_programming(void) {
    printf("=== 防御性编程模式 ===\n");

    printf("  模式1: 参数校验\n");
    printf("    safe_divide(10, 3) = %d\n", safe_divide(10, 3));
    printf("    safe_divide(10, 0) = %d (安全处理)\n\n", safe_divide(10, 0));

    printf("  模式2: 返回值检查\n");
    FILE *f = fopen("__nonexistent__", "r");
    if (!f) {
        printf("    fopen失败, 已安全处理\n");
    } else {
        fclose(f);
    }

    printf("\n  模式3: 边界检查\n");
    int arr[] = {10, 20, 30, 40, 50};
    int idx = 10;
    if (idx >= 0 && idx < 5) {
        printf("    arr[%d] = %d\n", idx, arr[idx]);
    } else {
        printf("    索引%d越界, 已安全处理\n", idx);
    }
    printf("\n");
}

void demo_assert_vs_error_check(void) {
    printf("=== assert vs 错误检查 对比 ===\n");
    printf("  assert适用场景:\n");
    printf("    - 检查程序内部逻辑不变量(开发者错误)\n");
    printf("    - 前置条件/后置条件验证\n");
    printf("    - 开发阶段调试辅助\n\n");
    printf("  错误检查适用场景:\n");
    printf("    - 用户输入验证\n");
    printf("    - 外部资源访问(文件/网络)\n");
    printf("    - 运行时可能失败的操作\n");
    printf("    - 生产环境必须保留的检查\n\n");
    printf("  原则: assert用于\"不可能发生\"的情况,\n");
    printf("        错误检查用于\"可能发生\"的情况\n\n");
}

int main(void) {
    printf("========== 断言与防御性编程示例 ==========\n\n");

    demo_assert_basic();
    demo_static_assert_c11();
    demo_defensive_programming();
    demo_assert_vs_error_check();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
