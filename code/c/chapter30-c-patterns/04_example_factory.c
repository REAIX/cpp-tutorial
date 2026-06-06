/**
 * @file 04_example_factory.c
 * @brief 工厂模式: 函数指针表实现对象创建
 * @description 对应文档: 30-C语言设计模式实现
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    void (*draw)(const void *self);
    double (*area)(const void *self);
    void (*destroy)(void *self);
    const char *type_name;
} ShapeVTable;

typedef struct {
    const ShapeVTable *vtable;
} Shape;

typedef struct {
    Shape base;
    double radius;
} FactoryCircle;

typedef struct {
    Shape base;
    double width;
    double height;
} FactoryRect;

static void fc_draw(const void *s) {
    const FactoryCircle *c = (const FactoryCircle *)s;
    printf("  圆: r=%.2f\n", c->radius);
}

static double fc_area(const void *s) {
    const FactoryCircle *c = (const FactoryCircle *)s;
    return 3.14159265 * c->radius * c->radius;
}

static void fc_destroy(void *s) { free(s); }

static const ShapeVTable circle_vtable = { fc_draw, fc_area, fc_destroy, "Circle" };

static void fr_draw(const void *s) {
    const FactoryRect *r = (const FactoryRect *)s;
    printf("  矩形: %.2fx%.2f\n", r->width, r->height);
}

static double fr_area(const void *s) {
    const FactoryRect *r = (const FactoryRect *)s;
    return r->width * r->height;
}

static void fr_destroy(void *s) { free(s); }

static const ShapeVTable rect_vtable = { fr_draw, fr_area, fr_destroy, "Rectangle" };

typedef Shape *(*ShapeFactory)(const char *params);

static Shape *create_circle(const char *params) {
    double radius = 1.0;
    if (params) sscanf(params, "%lf", &radius);
    FactoryCircle *c = (FactoryCircle *)calloc(1, sizeof(FactoryCircle));
    if (c) {
        c->base.vtable = &circle_vtable;
        c->radius = radius;
    }
    return (Shape *)c;
}

static Shape *create_rectangle(const char *params) {
    double w = 1.0, h = 1.0;
    if (params) sscanf(params, "%lf,%lf", &w, &h);
    FactoryRect *r = (FactoryRect *)calloc(1, sizeof(FactoryRect));
    if (r) {
        r->base.vtable = &rect_vtable;
        r->width = w;
        r->height = h;
    }
    return (Shape *)r;
}

typedef struct {
    const char *name;
    ShapeFactory factory;
} FactoryEntry;

static FactoryEntry factory_registry[] = {
    {"circle", create_circle},
    {"rectangle", create_rectangle},
    {NULL, NULL}
};

Shape *shape_create(const char *type, const char *params) {
    for (int i = 0; factory_registry[i].name != NULL; i++) {
        if (strcmp(factory_registry[i].name, type) == 0) {
            return factory_registry[i].factory(params);
        }
    }
    printf("  未知类型: %s\n", type);
    return NULL;
}

void demo_simple_factory(void) {
    printf("\n=== demo_simple_factory ===\n");
    printf("简单工厂: 根据类型字符串创建对象\n\n");

    Shape *shapes[] = {
        shape_create("circle", "5.0"),
        shape_create("rectangle", "4.0,6.0"),
        shape_create("circle", "3.0"),
        shape_create("triangle", "1,2,3")
    };

    for (int i = 0; i < 4; i++) {
        if (shapes[i]) {
            printf("[%s]\n", shapes[i]->vtable->type_name);
            shapes[i]->vtable->draw(shapes[i]);
            printf("  面积=%.2f\n", shapes[i]->vtable->area(shapes[i]));
            shapes[i]->vtable->destroy(shapes[i]);
        }
    }
}

typedef struct {
    char name[32];
    ShapeFactory factory;
} RegistryEntry;

#define MAX_REGISTRY 32

static RegistryEntry dynamic_registry[MAX_REGISTRY];
static int registry_count = 0;

void factory_register(const char *name, ShapeFactory factory) {
    if (registry_count >= MAX_REGISTRY) return;
    strncpy(dynamic_registry[registry_count].name, name, sizeof(dynamic_registry[0].name) - 1);
    dynamic_registry[registry_count].factory = factory;
    registry_count++;
}

Shape *factory_create(const char *type, const char *params) {
    for (int i = 0; i < registry_count; i++) {
        if (strcmp(dynamic_registry[i].name, type) == 0) {
            return dynamic_registry[i].factory(params);
        }
    }
    return NULL;
}

typedef struct {
    Shape base;
    double side;
} FactorySquare;

static void fsq_draw(const void *s) {
    const FactorySquare *sq = (const FactorySquare *)s;
    printf("  正方形: 边长=%.2f\n", sq->side);
}

static double fsq_area(const void *s) {
    const FactorySquare *sq = (const FactorySquare *)s;
    return sq->side * sq->side;
}

static void fsq_destroy(void *s) { free(s); }

static const ShapeVTable square_vtable = { fsq_draw, fsq_area, fsq_destroy, "Square" };

static Shape *create_square(const char *params) {
    double side = 1.0;
    if (params) sscanf(params, "%lf", &side);
    FactorySquare *sq = (FactorySquare *)calloc(1, sizeof(FactorySquare));
    if (sq) {
        sq->base.vtable = &square_vtable;
        sq->side = side;
    }
    return (Shape *)sq;
}

void demo_dynamic_factory(void) {
    printf("\n=== demo_dynamic_factory ===\n");
    printf("动态工厂: 运行时注册新的创建函数\n\n");

    factory_register("circle", create_circle);
    factory_register("rectangle", create_rectangle);
    printf("注册 circle, rectangle\n");

    Shape *c = factory_create("circle", "7.0");
    if (c) {
        c->vtable->draw(c);
        c->vtable->destroy(c);
    }

    printf("\n动态注册 square:\n");
    factory_register("square", create_square);

    Shape *sq = factory_create("square", "4.0");
    if (sq) {
        sq->vtable->draw(sq);
        printf("  面积=%.2f\n", sq->vtable->area(sq));
        sq->vtable->destroy(sq);
    }

    printf("\n动态工厂优势:\n");
    printf("  1. 运行时扩展: 新增类型无需修改工厂代码\n");
    printf("  2. 插件友好: 动态库可以注册新类型\n");
    printf("  3. 解耦: 工厂不需要知道所有具体类型\n");
}

void demo_factory_comparison(void) {
    printf("\n=== demo_factory_comparison ===\n");
    printf("工厂模式变体:\n\n");

    printf("1. 简单工厂:\n");
    printf("   一个工厂函数, switch/if选择创建\n");
    printf("   优点: 简单  缺点: 新增类型需修改工厂\n\n");

    printf("2. 工厂方法:\n");
    printf("   每个类型有自己的创建函数\n");
    printf("   通过函数指针表注册\n");
    printf("   优点: 开放扩展  缺点: 稍复杂\n\n");

    printf("3. 抽象工厂:\n");
    printf("   创建一系列相关对象\n");
    printf("   如: LightTheme工厂, DarkTheme工厂\n");
    printf("   每个工厂创建一组风格一致的对象\n\n");

    printf("C语言工厂模式核心:\n");
    printf("  函数指针表 = 类型名 -> 创建函数的映射\n");
    printf("  注册机制 = 运行时动态添加映射\n");
    printf("  创建接口 = 统一的创建函数签名\n");
}

int main(void) {
    printf("工厂模式: 函数指针表实现对象创建\n");

    demo_simple_factory();
    demo_dynamic_factory();
    demo_factory_comparison();

    printf("\n所有演示完成!\n");
    return 0;
}
