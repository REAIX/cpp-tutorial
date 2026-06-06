/**
 * @file main.c
 * @brief 不透明指针模式演示
 * @description 对应文档: 16-多文件编程 - 项目组织、接口与实现分离、封装、API设计
 */
#include <stdio.h>
#include <stdlib.h>
#include "point.h"

void demo_opaque_pointer(void) {
    printf("=== 不透明指针(Opaque Pointer)模式 ===\n");
    printf("  头文件只声明: typedef struct point point_t;\n");
    printf("  结构体定义在.c文件中, 调用者无法访问内部字段\n\n");

    point_t *p1 = point_create(3.0, 4.0);
    point_t *p2 = point_create(6.0, 8.0);

    if (!p1 || !p2) {
        fprintf(stderr, "  创建点失败\n");
        if (p1) point_destroy(p1);
        if (p2) point_destroy(p2);
        return;
    }

    printf("  p1 = "); point_print(p1); printf("\n");
    printf("  p2 = "); point_print(p2); printf("\n");

    printf("  距离 = %.2f\n", point_distance(p1, p2));

    point_translate(p1, 1.0, 1.0);
    printf("  p1平移(1,1)后 = "); point_print(p1); printf("\n");

    point_set_x(p2, 10.0);
    printf("  p2设置x=10后 = "); point_print(p2); printf("\n\n");

    point_destroy(p1);
    point_destroy(p2);
}

void demo_encapsulation_benefits(void) {
    printf("=== 封装的好处 ===\n");
    printf("  1. 隐藏实现细节: 调用者只能通过API操作\n");
    printf("  2. 可自由修改实现: 改结构体字段不影响调用者\n");
    printf("  3. ABI稳定: 只要不改头文件, 二进制兼容\n");
    printf("  4. 防止误用: 无法直接访问内部字段\n\n");

    printf("  对比: 如果头文件暴露了struct定义\n");
    printf("    - 调用者可能直接访问字段, 绕过校验逻辑\n");
    printf("    - 修改字段名/类型会破坏所有调用者\n");
    printf("    - sizeof在调用者端确定, 无法动态调整\n\n");
}

void demo_api_design(void) {
    printf("=== C语言API设计原则 ===\n");
    printf("  1. 创建/销毁函数对: create/destroy, init/cleanup\n");
    printf("  2. getter/setter函数: 封装字段访问\n");
    printf("  3. 一致的命名约定: 模块名_操作名\n");
    printf("  4. const正确性: 不修改的参数用const\n");
    printf("  5. 错误处理: 返回错误码或NULL\n");
    printf("  6. 前向声明: 头文件只放声明, 不放定义\n\n");
}

void demo_project_structure(void) {
    printf("=== 项目组织模式 ===\n");
    printf("  推荐目录结构:\n");
    printf("    myproject/\n");
    printf("    ├── include/        # 公共头文件\n");
    printf("    │   └── mymodule.h\n");
    printf("    ├── src/            # 源文件\n");
    printf("    │   └── mymodule.c\n");
    printf("    ├── tests/          # 测试文件\n");
    printf("    ├── CMakeLists.txt\n");
    printf("    └── README.md\n\n");

    printf("  头文件分类:\n");
    printf("    - 公共头文件: 对外API, 放include/\n");
    printf("    - 私有头文件: 内部实现, 放src/\n");
    printf("    - 配置头文件: 由构建系统生成\n\n");
}

void demo_interface_vs_implementation(void) {
    printf("=== 接口与实现分离 ===\n");
    printf("  头文件(.h) = 接口 = 契约\n");
    printf("    - 类型声明(typedef struct point point_t;)\n");
    printf("    - 函数声明(point_t *point_create(...);)\n");
    printf("    - 宏定义和常量\n");
    printf("    - 文档注释\n\n");
    printf("  源文件(.c) = 实现 = 细节\n");
    printf("    - 结构体完整定义(struct point { double x, y; };)\n");
    printf("    - 函数实现\n");
    printf("    - 内部辅助函数(static)\n");
    printf("    - 内部状态变量\n\n");
}

int main(void) {
    printf("========== 不透明指针模式与项目组织 ==========\n\n");

    demo_opaque_pointer();
    demo_encapsulation_benefits();
    demo_api_design();
    demo_project_structure();
    demo_interface_vs_implementation();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
