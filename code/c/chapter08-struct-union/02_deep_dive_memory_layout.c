/** @file 02_deep_dive_memory_layout.c
 *  @brief 内存布局深度分析：对齐规则、缓存友好设计、弹性数组成员
 *  @description 对应文档: 08-结构体与联合体 | 举一反三：理解结构体的内存布局
 */

#include <stdio.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

void demo_detailed_layout(void) {
    printf("=== 详细内存布局分析 ===\n");

    struct Example {
        char    a;
        int     b;
        short   c;
        double  d;
        char    e;
    };

    printf("struct Example 布局:\n");
    printf("  成员   类型    大小  偏移  填充\n");
    printf("  a      char    1     %zu    %zu\n", offsetof(struct Example, a), (size_t)0);
    printf("  [填充]         3           %zu\n", offsetof(struct Example, b) - 1);
    printf("  b      int     4     %zu    0\n", offsetof(struct Example, b));
    printf("  c      short   2     %zu    0\n", offsetof(struct Example, c));
    printf("  [填充]         4           %zu\n", offsetof(struct Example, d) - offsetof(struct Example, c) - 2);
    printf("  d      double  8     %zu    0\n", offsetof(struct Example, d));
    printf("  e      char    1     %zu    0\n", offsetof(struct Example, e));
    printf("  [尾部填充]     7\n");
    printf("  总大小: %zu 字节\n", sizeof(struct Example));

    printf("\n");
}

void demo_alignment_boundary(void) {
    printf("=== 对齐边界详解 ===\n");

    printf("各类型的默认对齐要求:\n");
    printf("  char    : %zu 字节对齐\n", _Alignof(char));
    printf("  short   : %zu 字节对齐\n", _Alignof(short));
    printf("  int     : %zu 字节对齐\n", _Alignof(int));
    printf("  long    : %zu 字节对齐\n", _Alignof(long));
    printf("  float   : %zu 字节对齐\n", _Alignof(float));
    printf("  double  : %zu 字节对齐\n", _Alignof(double));
    printf("  pointer : %zu 字节对齐\n", _Alignof(void *));

    printf("\n对齐的原因:\n");
    printf("1. 硬件效率: CPU 从对齐地址读取数据更快\n");
    printf("2. 某些架构: 未对齐访问会触发异常 (如 ARM, SPARC)\n");
    printf("3. x86 容忍未对齐访问, 但性能会下降\n");

    printf("\n");
}

typedef struct {
    int id;
    char name[32];
    double score;
} StudentSlow;

typedef struct {
    double score;
    int id;
    char name[32];
} StudentFast;

void demo_cache_friendly_design(void) {
    printf("=== 缓存友好的结构体设计 ===\n");

    printf("原则1: 热数据放前面, 冷数据放后面\n");
    printf("原则2: 按大小从大到小排列成员\n");
    printf("原则3: 减少结构体大小, 提高缓存命中率\n\n");

    printf("StudentSlow (char[32] 在中间, 打断对齐):\n");
    printf("  sizeof = %zu\n", sizeof(StudentSlow));

    printf("StudentFast (double 在前面, 更紧凑):\n");
    printf("  sizeof = %zu\n", sizeof(StudentFast));

    printf("\n缓存行通常是 64 字节\n");
    printf("结构体越小, 单个缓存行能装下越多实例\n");

    printf("\n");
}

void demo_structure_of_arrays(void) {
    printf("=== SoA vs AoS (数组结构 vs 结构数组) ===\n");

    printf("AoS (Array of Structures) - 传统方式:\n");
    printf("  struct Particle { float x, y, z, vx, vy, vz; };\n");
    printf("  Particle particles[N];\n");
    printf("  访问 x 时, 缓存行也加载了 vx,vy,vz (浪费)\n\n");

    printf("SoA (Structure of Arrays) - 缓存友好:\n");
    printf("  struct Particles {\n");
    printf("    float x[N], y[N], z[N];\n");
    printf("    float vx[N], vy[N], vz[N];\n");
    printf("  };\n");
    printf("  访问所有 x 时, 缓存行全是 x (高效)\n\n");

    printf("SoA 适合: 批量处理同一字段 (如物理模拟)\n");
    printf("AoS 适合: 频繁访问同一对象的所有字段\n");

    printf("\n");
}

void demo_flexible_array_member(void) {
    printf("=== 弹性数组成员 (Flexible Array Member) ===\n");

    typedef struct {
        size_t length;
        int data[];
    } IntArray;

    size_t n = 5;
    IntArray *arr = (IntArray *)malloc(sizeof(IntArray) + n * sizeof(int));
    if (arr) {
        arr->length = n;
        for (size_t i = 0; i < n; i++) {
            arr->data[i] = (int)(i * 10);
        }

        printf("弹性数组成员:\n");
        printf("  length = %zu\n", arr->length);
        printf("  data = [");
        for (size_t i = 0; i < arr->length; i++) {
            printf("%d", arr->data[i]);
            if (i < arr->length - 1) printf(", ");
        }
        printf("]\n");

        printf("\nsizeof(IntArray) = %zu (不包含弹性数组)\n", sizeof(IntArray));
        printf("分配大小 = %zu + %zu*%zu = %zu\n",
               sizeof(IntArray), n, sizeof(int),
               sizeof(IntArray) + n * sizeof(int));

        free(arr);
    }

    printf("\n弹性数组成员的规则:\n");
    printf("1. 必须是结构体的最后一个成员\n");
    printf("2. 不指定大小: int data[];\n");
    printf("3. sizeof 不包含弹性数组的大小\n");
    printf("4. 必须动态分配, 不能静态初始化\n");
    printf("5. C99 标准引入, 替代 C89 的 \"struct hack\"\n");

    printf("\n");
}

void demo_struct_hack(void) {
    printf("=== C89 的 struct hack (弹性数组的前身) ===\n");

    typedef struct {
        size_t length;
        int data[1];
    } OldArray;

    size_t n = 5;
    OldArray *arr = (OldArray *)malloc(sizeof(OldArray) + (n - 1) * sizeof(int));
    if (arr) {
        arr->length = n;
        for (size_t i = 0; i < n; i++) {
            arr->data[i] = (int)(i * 100);
        }

        printf("struct hack 结果: ");
        for (size_t i = 0; i < arr->length; i++) {
            printf("%d ", arr->data[i]);
        }
        printf("\n");

        printf("sizeof(OldArray) = %zu (包含1个int)\n", sizeof(OldArray));

        free(arr);
    }

    printf("\nstruct hack 是 C89 时代的变通方法, 现在应使用弹性数组成员\n");

    printf("\n");
}

void demo_memory_layout_best_practices(void) {
    printf("=== 内存布局最佳实践 ===\n");
    printf("1. 按对齐要求从大到小排列结构体成员\n");
    printf("2. 使用 offsetof 验证布局是否符合预期\n");
    printf("3. 跨平台代码避免依赖特定布局\n");
    printf("4. 网络协议/文件格式使用 packed 或手动序列化\n");
    printf("5. 性能关键代码考虑 SoA 布局\n");
    printf("6. 变长数据使用弹性数组成员\n");
    printf("7. 使用 _Alignof 和 alignas (C11) 控制对齐\n");
    printf("\n");
}

int main(void) {
    demo_detailed_layout();
    demo_alignment_boundary();
    demo_cache_friendly_design();
    demo_structure_of_arrays();
    demo_flexible_array_member();
    demo_struct_hack();
    demo_memory_layout_best_practices();

    return 0;
}
