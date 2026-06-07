/**
 * @file 02_deep_dive_vtable.c
 * @brief vtable深入: 详细实现、多重继承模拟、接口概念
 * @description 对应文档: 28-C语言面向对象实现-基础
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void (*draw)(void *self);
    void (*resize)(void *self, double factor);
    double (*area)(const void *self);
    const char *(*type_name)(const void *self);
    void (*destroy)(void *self);
} DrawableVTable;

typedef struct {
    const DrawableVTable *vtable;
} Drawable;

void drawable_draw(const Drawable *d) { d->vtable->draw((void *)d); }
void drawable_resize(Drawable *d, double f) { d->vtable->resize((void *)d, f); }
double drawable_area(const Drawable *d) { return d->vtable->area(d); }
const char *drawable_type(const Drawable *d) { return d->vtable->type_name(d); }
void drawable_destroy(Drawable *d) { d->vtable->destroy((void *)d); }

typedef struct {
    Drawable base;
    double radius;
} VCircle;

static void vcircle_draw(void *self) {
    VCircle *c = (VCircle *)self;
    printf("  圆: 半径=%.2f\n", c->radius);
}

static void vcircle_resize(void *self, double factor) {
    VCircle *c = (VCircle *)self;
    c->radius *= factor;
}

static double vcircle_area(const void *self) {
    const VCircle *c = (const VCircle *)self;
    return 3.14159265 * c->radius * c->radius;
}

static const char *vcircle_type(const void *self) {
    (void)self;
    return "Circle";
}

static void vcircle_destroy(void *self) {
    free(self);
}

static const DrawableVTable circle_vtable = {
    .draw = vcircle_draw,
    .resize = vcircle_resize,
    .area = vcircle_area,
    .type_name = vcircle_type,
    .destroy = vcircle_destroy
};

Drawable *vcircle_create(double radius) {
    VCircle *c = (VCircle *)calloc(1, sizeof(VCircle));
    if (!c) return NULL;
    c->base.vtable = &circle_vtable;
    c->radius = radius;
    return (Drawable *)c;
}

typedef struct {
    Drawable base;
    double width;
    double height;
} VRectangle;

static void vrect_draw(void *self) {
    VRectangle *r = (VRectangle *)self;
    printf("  矩形: %.2f x %.2f\n", r->width, r->height);
}

static void vrect_resize(void *self, double factor) {
    VRectangle *r = (VRectangle *)self;
    r->width *= factor;
    r->height *= factor;
}

static double vrect_area(const void *self) {
    const VRectangle *r = (const VRectangle *)self;
    return r->width * r->height;
}

static const char *vrect_type(const void *self) {
    (void)self;
    return "Rectangle";
}

static void vrect_destroy(void *self) {
    free(self);
}

static const DrawableVTable rect_vtable = {
    .draw = vrect_draw,
    .resize = vrect_resize,
    .area = vrect_area,
    .type_name = vrect_type,
    .destroy = vrect_destroy
};

Drawable *vrectangle_create(double w, double h) {
    VRectangle *r = (VRectangle *)calloc(1, sizeof(VRectangle));
    if (!r) return NULL;
    r->base.vtable = &rect_vtable;
    r->width = w;
    r->height = h;
    return (Drawable *)r;
}

void demo_vtable_detailed(void) {
    printf("\n=== demo_vtable_detailed ===\n");
    printf("vtable详细实现: 每个类型有自己的虚函数表\n\n");

    Drawable *shapes[] = {
        vcircle_create(5.0),
        vrectangle_create(4.0, 6.0)
    };

    for (int i = 0; i < 2; i++) {
        printf("[%s]\n", drawable_type(shapes[i]));
        drawable_draw(shapes[i]);
        printf("  面积=%.2f\n", drawable_area(shapes[i]));
        drawable_resize(shapes[i], 2.0);
        printf("  放大2倍后: ");
        drawable_draw(shapes[i]);
        drawable_destroy(shapes[i]);
        printf("\n");
    }

    printf("vtable内存布局:\n");
    printf("  Drawable对象: [vtable指针] [子类数据...]\n");
    printf("  vtable:       [draw][resize][area][type_name][destroy]\n");
    printf("  每个类型的vtable是const静态变量, 所有实例共享\n");
}

typedef struct {
    void (*serialize)(const void *self, char *buf, size_t size);
    int (*deserialize)(void *self, const char *buf);
} SerializableVTable;

typedef struct {
    const SerializableVTable *vtable;
} Serializable;

typedef struct {
    Drawable base;
    Serializable serializable;
    double side;
} VSquare;

static void vsquare_draw(void *self) {
    VSquare *s = (VSquare *)self;
    printf("  正方形: 边长=%.2f\n", s->side);
}

static void vsquare_resize(void *self, double factor) {
    VSquare *s = (VSquare *)self;
    s->side *= factor;
}

static double vsquare_area(const void *self) {
    const VSquare *s = (const VSquare *)self;
    return s->side * s->side;
}

static const char *vsquare_type(const void *self) {
    (void)self;
    return "Square";
}

static void vsquare_destroy(void *self) {
    free(self);
}

static const DrawableVTable square_vtable = {
    .draw = vsquare_draw,
    .resize = vsquare_resize,
    .area = vsquare_area,
    .type_name = vsquare_type,
    .destroy = vsquare_destroy
};

static void vsquare_serialize(const void *self, char *buf, size_t size) {
    const VSquare *s = (const VSquare *)self;
    snprintf(buf, size, "Square:%.4f", s->side);
}

static int vsquare_deserialize(void *self, const char *buf) {
    VSquare *s = (VSquare *)self;
    if (sscanf(buf, "Square:%lf", &s->side) == 1) return 0;
    return -1;
}

static const SerializableVTable square_serial_vtable = {
    .serialize = vsquare_serialize,
    .deserialize = vsquare_deserialize
};

VSquare *vsquare_create(double side) {
    VSquare *s = (VSquare *)calloc(1, sizeof(VSquare));
    if (!s) return NULL;
    s->base.vtable = &square_vtable;
    s->serializable.vtable = &square_serial_vtable;
    s->side = side;
    return s;
}

void demo_multiple_inheritance(void) {
    printf("\n=== demo_multiple_inheritance ===\n");
    printf("多重继承模拟: 结构体包含多个接口基类\n\n");

    VSquare *sq = vsquare_create(5.0);

    printf("通过Drawable接口使用:\n");
    drawable_draw((Drawable *)sq);
    printf("  面积=%.2f\n", drawable_area((Drawable *)sq));

    printf("\n通过Serializable接口使用:\n");
    char buf[128];
    sq->serializable.vtable->serialize(sq, buf, sizeof(buf));
    printf("  序列化: %s\n", buf);

    VSquare sq2 = *sq;
    sq2.serializable.vtable->deserialize(&sq2, "Square:10.0");
    printf("  反序列化后: ");
    drawable_draw((Drawable *)&sq2);

    free(sq);

    printf("\n多重继承内存布局:\n");
    printf("  VSquare: [Drawable{vtable}] [Serializable{vtable}] [side]\n");
    printf("  注意: Serializable的vtable不是在偏移0处!\n");
    printf("  转换时需要调整指针: (Serializable*)((char*)sq + offset)\n");
}

typedef struct {
    int (*compare)(const void *self, const void *other);
    int (*equals)(const void *self, const void *other);
    unsigned long (*hash)(const void *self);
    const char *(*to_string)(const void *self, char *buf, size_t size);
} ComparableVTable;

typedef struct {
    const ComparableVTable *vtable;
} Comparable;

typedef struct {
    Comparable base;
    int value;
} ComparableInt;

static int comp_int_compare(const void *a, const void *b) {
    const ComparableInt *ia = (const ComparableInt *)a;
    const ComparableInt *ib = (const ComparableInt *)b;
    return ia->value - ib->value;
}

static int comp_int_equals(const void *a, const void *b) {
    return comp_int_compare(a, b) == 0;
}

static unsigned long comp_int_hash(const void *self) {
    const ComparableInt *ci = (const ComparableInt *)self;
    unsigned long h = (unsigned long)ci->value;
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = ((h >> 16) ^ h) * 0x45d9f3b;
    h = (h >> 16) ^ h;
    return h;
}

static const char *comp_int_to_string(const void *self, char *buf, size_t size) {
    const ComparableInt *ci = (const ComparableInt *)self;
    snprintf(buf, size, "%d", ci->value);
    return buf;
}

static const ComparableVTable comp_int_vtable = {
    .compare = comp_int_compare,
    .equals = comp_int_equals,
    .hash = comp_int_hash,
    .to_string = comp_int_to_string
};

ComparableInt *comp_int_create(int value) {
    ComparableInt *ci = (ComparableInt *)calloc(1, sizeof(ComparableInt));
    if (!ci) return NULL;
    ci->base.vtable = &comp_int_vtable;
    ci->value = value;
    return ci;
}

void demo_interface_concept(void) {
    printf("\n=== demo_interface_concept ===\n");
    printf("接口概念: 只包含虚函数的结构体, 定义行为契约\n\n");

    ComparableInt *a = comp_int_create(42);
    ComparableInt *b = comp_int_create(42);
    ComparableInt *c = comp_int_create(99);

    char buf[64];
    printf("a = %s\n", a->base.vtable->to_string(a, buf, sizeof(buf)));
    printf("b = %s\n", b->base.vtable->to_string(b, buf, sizeof(buf)));
    printf("c = %s\n", c->base.vtable->to_string(c, buf, sizeof(buf)));

    printf("\na == b ? %s\n", a->base.vtable->equals(a, b) ? "是" : "否");
    printf("a == c ? %s\n", a->base.vtable->equals(a, c) ? "是" : "否");
    printf("a < c  ? %s\n", a->base.vtable->compare(a, c) < 0 ? "是" : "否");
    printf("hash(a) = %lu\n", a->base.vtable->hash(a));

    free(a);
    free(b);
    free(c);

    printf("\nC语言接口设计原则:\n");
    printf("  1. 接口 = 只含函数指针的结构体\n");
    printf("  2. 实现接口 = 包含接口结构体 + 提供函数实现\n");
    printf("  3. 一个类型可以实现多个接口\n");
    printf("  4. 接口是契约, 不是继承\n\n");

    printf("C++对比:\n");
    printf("  C++ class Drawable { virtual draw()=0; };\n");
    printf("  C   struct DrawableVTable { void (*draw)(void*); };\n");
    printf("  C++ 编译器自动生成vtable, C需要手动维护\n");
}

int main(void) {
    printf("vtable深入: 详细实现、多重继承模拟、接口概念\n");

    demo_vtable_detailed();
    demo_multiple_inheritance();
    demo_interface_concept();

    printf("\n所有演示完成!\n");
    return 0;
}
