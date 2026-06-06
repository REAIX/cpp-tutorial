/** @file 03_example_union_bitfield.c
 *  @brief 联合体与位域：联合体基础、类型双关、位域使用
 *  @description 对应文档: 08-结构体与联合体
 */

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <math.h>

void demo_union_basics(void) {
    printf("=== 联合体基础 ===\n");

    union Data {
        int i;
        float f;
        char str[20];
    };

    printf("union Data 大小: %zu (等于最大成员的大小)\n", sizeof(union Data));
    printf("  sizeof(int)    = %zu\n", sizeof(int));
    printf("  sizeof(float)  = %zu\n", sizeof(float));
    printf("  sizeof(char[20]) = %zu\n", sizeof(char[20]));

    union Data d;
    d.i = 42;
    printf("\n赋值 d.i = 42:\n");
    printf("  d.i = %d\n", d.i);
    printf("  d.f = %f (无意义, 共享内存被重新解释)\n", d.f);

    d.f = 3.14f;
    printf("\n赋值 d.f = 3.14:\n");
    printf("  d.f = %f\n", d.f);
    printf("  d.i = %d (之前的值被覆盖)\n", d.i);

    printf("\n联合体所有成员共享同一块内存, 同一时刻只能使用一个成员\n");

    printf("\n");
}

void demo_type_punning(void) {
    printf("=== 类型双关 (Type Punning) ===\n");

    union FloatInt {
        float f;
        uint32_t i;
    };

    union FloatInt fi;
    fi.f = 3.14f;
    printf("float 3.14f 的二进制表示: 0x%08X\n", fi.i);

    fi.i = 0x40490FDB;
    printf("0x40490FDB 解释为 float: %f\n", fi.f);

    printf("\n通过联合体可以安全地查看浮点数的位模式\n");
    printf("这在 C 中是合法的 (C99 及以后), 但在 C++ 中是未定义行为\n");

    printf("\n实际应用: 查看浮点数的符号位、指数、尾数\n");
    union FloatInt pi;
    pi.f = -3.14f;
    uint32_t bits = pi.i;
    int sign = (bits >> 31) & 1;
    int exponent = (bits >> 23) & 0xFF;
    uint32_t mantissa = bits & 0x7FFFFF;
    printf("  符号位: %d, 指数: %d, 尾数: 0x%06X\n", sign, exponent, mantissa);

    printf("\n");
}

void demo_tagged_union(void) {
    printf("=== 标记联合体 (Tagged Union) ===\n");

    enum ShapeType { SHAPE_CIRCLE, SHAPE_RECT, SHAPE_TRIANGLE };

    struct Shape {
        enum ShapeType type;
        union {
            struct { double radius; } circle;
            struct { double width, height; } rect;
            struct { double a, b, c; } triangle;
        } data;
    };

    struct Shape s1;
    s1.type = SHAPE_CIRCLE;
    s1.data.circle.radius = 5.0;

    struct Shape s2;
    s2.type = SHAPE_RECT;
    s2.data.rect.width = 3.0;
    s2.data.rect.height = 4.0;

    double area(const struct Shape *s) {
        switch (s->type) {
            case SHAPE_CIRCLE:
                return 3.14159265 * s->data.circle.radius * s->data.circle.radius;
            case SHAPE_RECT:
                return s->data.rect.width * s->data.rect.height;
            case SHAPE_TRIANGLE: {
                double a = s->data.triangle.a, b = s->data.triangle.b, c = s->data.triangle.c;
                double sp = (a + b + c) / 2;
                return sqrt(sp * (sp - a) * (sp - b) * (sp - c));
            }
        }
        return 0;
    }

    printf("圆面积: %.2f\n", area(&s1));
    printf("矩形面积: %.2f\n", area(&s2));

    printf("\n标记联合体 = 类型标签 + 联合体, 是 C 中实现多态的经典方式\n");

    printf("\n");
}

void demo_bitfield_basics(void) {
    printf("=== 位域基础 ===\n");

    struct Flags {
        unsigned int is_active  : 1;
        unsigned int is_visible : 1;
        unsigned int is_enabled : 1;
        unsigned int priority   : 3;
        unsigned int reserved   : 2;
    };

    printf("struct Flags 大小: %zu 字节\n", sizeof(struct Flags));
    printf("  1+1+1+3+2 = 8 位 = 1 字节 (实际可能因对齐而更大)\n");

    struct Flags f = {0};
    f.is_active = 1;
    f.is_visible = 1;
    f.priority = 5;

    printf("\n设置标志: is_active=%u, is_visible=%u, is_enabled=%u, priority=%u\n",
           f.is_active, f.is_visible, f.is_enabled, f.priority);

    printf("\n位域的优势: 节省内存, 语法清晰\n");
    printf("位域的限制: 不能取地址, 跨平台位序不确定\n");

    printf("\n");
}

void demo_bitfield_protocol(void) {
    printf("=== 位域用于协议解析 ===\n");

    struct TCPHeader {
        uint16_t src_port;
        uint16_t dst_port;
        uint32_t seq_num;
        uint32_t ack_num;
        unsigned int data_offset : 4;
        unsigned int reserved    : 3;
        unsigned int ns          : 1;
        unsigned int cwr         : 1;
        unsigned int ece         : 1;
        unsigned int urg         : 1;
        unsigned int ack_flag    : 1;
        unsigned int psh         : 1;
        unsigned int rst         : 1;
        unsigned int syn         : 1;
        unsigned int fin         : 1;
        uint16_t window;
        uint16_t checksum;
        uint16_t urgent_ptr;
    };

    printf("TCP头部结构体大小: %zu 字节\n", sizeof(struct TCPHeader));
    printf("标准TCP头部: 20 字节\n");

    printf("\n注意: 位域的内存布局依赖编译器实现\n");
    printf("跨平台场景建议使用位操作代替位域\n");

    printf("\n");
}

int main(void) {
    demo_union_basics();
    demo_type_punning();
    demo_tagged_union();
    demo_bitfield_basics();
    demo_bitfield_protocol();

    return 0;
}
