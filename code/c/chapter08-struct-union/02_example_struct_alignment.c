/** @file 02_example_struct_alignment.c
 *  @brief 结构体内存对齐：对齐规则、填充、sizeof、__attribute__((packed))
 *  @description 对应文档: 08-结构体与联合体
 */

#include <stdio.h>
#include <stddef.h>
#include <stdint.h>

void demo_basic_alignment(void) {
    printf("=== 基本对齐规则 ===\n");

    struct A {
        char a;
        int b;
        char c;
    };

    struct B {
        char a;
        char c;
        int b;
    };

    printf("struct A { char; int; char }:\n");
    printf("  sizeof = %zu\n", sizeof(struct A));
    printf("  offsetof(a) = %zu\n", offsetof(struct A, a));
    printf("  offsetof(b) = %zu\n", offsetof(struct A, b));
    printf("  offsetof(c) = %zu\n", offsetof(struct A, c));

    printf("\nstruct B { char; char; int }:\n");
    printf("  sizeof = %zu\n", sizeof(struct B));
    printf("  offsetof(a) = %zu\n", offsetof(struct B, a));
    printf("  offsetof(c) = %zu\n", offsetof(struct B, c));
    printf("  offsetof(b) = %zu\n", offsetof(struct B, b));

    printf("\n同样的成员, 不同顺序, 大小不同!\n");
    printf("A 有更多填充, B 更紧凑\n");

    printf("\n");
}

void demo_alignment_rules(void) {
    printf("=== 对齐规则详解 ===\n");

    printf("规则1: 每个成员的偏移量必须是其对齐数的整数倍\n");
    printf("  对齐数 = min(成员自身大小, 默认对齐数)\n");
    printf("  默认对齐数通常为编译器设置 (一般 #pragma pack 默认值)\n\n");

    printf("规则2: 结构体总大小必须是最大对齐数的整数倍\n\n");

    printf("规则3: 嵌套结构体对齐到其最大成员的对齐数\n\n");

    struct Example {
        char a;
        double b;
        int c;
        short d;
    };

    printf("struct Example { char; double; int; short }:\n");
    printf("  sizeof = %zu\n", sizeof(struct Example));
    printf("  offsetof(a) = %zu (char 对齐到 1)\n", offsetof(struct Example, a));
    printf("  offsetof(b) = %zu (double 对齐到 8)\n", offsetof(struct Example, b));
    printf("  offsetof(c) = %zu (int 对齐到 4)\n", offsetof(struct Example, c));
    printf("  offsetof(d) = %zu (short 对齐到 2)\n", offsetof(struct Example, d));

    printf("\n");
}

void demo_packed_attribute(void) {
    printf("=== __attribute__((packed)) 取消对齐 ===\n");

    struct Normal {
        char a;
        int b;
        char c;
    };

    struct __attribute__((packed)) Packed {
        char a;
        int b;
        char c;
    };

    printf("struct Normal: sizeof = %zu\n", sizeof(struct Normal));
    printf("struct Packed: sizeof = %zu\n", sizeof(struct Packed));

    printf("\nNormal 布局: a(1) + 填充(3) + b(4) + c(1) + 填充(3) = %zu\n", sizeof(struct Normal));
    printf("Packed 布局: a(1) + b(4) + c(1) = %zu (无填充)\n", sizeof(struct Packed));

    printf("\npacked 的代价:\n");
    printf("1. 某些平台上未对齐的访问会导致性能下降或硬件异常\n");
    printf("2. 适用于网络协议、文件格式等需要精确控制内存布局的场景\n");

    printf("\n");
}

void demo_pragma_pack(void) {
    printf("=== #pragma pack 控制对齐 ===\n");

#pragma pack(push, 1)
    struct Pack1 {
        char a;
        int b;
        char c;
    };
#pragma pack(pop)

#pragma pack(push, 2)
    struct Pack2 {
        char a;
        int b;
        char c;
    };
#pragma pack(pop)

    printf("默认对齐: sizeof = %zu\n", sizeof(struct { char a; int b; char c; }));
    printf("#pragma pack(1): sizeof = %zu\n", sizeof(struct Pack1));
    printf("#pragma pack(2): sizeof = %zu\n", sizeof(struct Pack2));

    printf("\n#pragma pack(push, n) 保存当前对齐设置并设为 n\n");
    printf("#pragma pack(pop) 恢复之前的设置\n");

    printf("\n");
}

void demo_sizeof_various(void) {
    printf("=== 各种结构体的 sizeof ===\n");

    struct S1 { char c; };
    struct S2 { int i; };
    struct S3 { double d; };
    struct S4 { char c; int i; double d; };
    struct S5 { double d; int i; char c; };

    printf("struct { char }         : %zu\n", sizeof(struct S1));
    printf("struct { int }          : %zu\n", sizeof(struct S2));
    printf("struct { double }       : %zu\n", sizeof(struct S3));
    printf("struct { char;int;dbl } : %zu\n", sizeof(struct S4));
    printf("struct { dbl;int;char } : %zu\n", sizeof(struct S5));

    printf("\nS4 和 S5 成员相同但顺序不同, 大小可能不同\n");
    printf("优化技巧: 按对齐要求从大到小排列成员\n");

    printf("\n");
}

void demo_alignment_optimization(void) {
    printf("=== 对齐优化实践 ===\n");

    struct Before {
        char a;
        double b;
        char c;
        int d;
        short e;
    };

    struct After {
        double b;
        int d;
        short e;
        char a;
        char c;
    };

    printf("优化前 struct Before: sizeof = %zu\n", sizeof(struct Before));
    printf("优化后 struct After:  sizeof = %zu\n", sizeof(struct After));

    printf("\n优化原则: 按成员大小从大到小排列, 减少填充浪费\n");

    printf("\n");
}

int main(void) {
    demo_basic_alignment();
    demo_alignment_rules();
    demo_packed_attribute();
    demo_pragma_pack();
    demo_sizeof_various();
    demo_alignment_optimization();

    return 0;
}
