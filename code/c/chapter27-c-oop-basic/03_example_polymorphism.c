/**
 * @file 03_example_polymorphism.c
 * @brief 多态: 函数指针、虚函数表实现运行时多态
 * @description 对应文档: 28-C语言面向对象实现-基础
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct Shape Shape;

struct Shape {
    void (*draw)(const Shape *self);
    double (*area)(const Shape *self);
    double (*perimeter)(const Shape *self);
    void (*destroy)(Shape *self);
    const char *type_name;
};

void shape_draw(const Shape *s) { if (s && s->draw) s->draw(s); }
double shape_area(const Shape *s) { return s && s->area ? s->area(s) : 0; }
double shape_perimeter(const Shape *s) { return s && s->perimeter ? s->perimeter(s) : 0; }
void shape_destroy(Shape *s) { if (s && s->destroy) s->destroy(s); }

typedef struct {
    Shape base;
    double radius;
} Circle;

static void circle_draw(const Shape *self) {
    const Circle *c = (const Circle *)self;
    printf("  画圆: 半径=%.2f\n", c->radius);
}

static double circle_area(const Shape *self) {
    const Circle *c = (const Circle *)self;
    return 3.14159265 * c->radius * c->radius;
}

static double circle_perimeter(const Shape *self) {
    const Circle *c = (const Circle *)self;
    return 2 * 3.14159265 * c->radius;
}

static void circle_destroy(Shape *self) {
    free(self);
}

Shape *circle_create(double radius) {
    Circle *c = (Circle *)calloc(1, sizeof(Circle));
    if (!c) return NULL;
    c->base.draw = circle_draw;
    c->base.area = circle_area;
    c->base.perimeter = circle_perimeter;
    c->base.destroy = circle_destroy;
    c->base.type_name = "Circle";
    c->radius = radius;
    return (Shape *)c;
}

typedef struct {
    Shape base;
    double width;
    double height;
} Rectangle;

static void rect_draw(const Shape *self) {
    const Rectangle *r = (const Rectangle *)self;
    printf("  画矩形: 宽=%.2f, 高=%.2f\n", r->width, r->height);
}

static double rect_area(const Shape *self) {
    const Rectangle *r = (const Rectangle *)self;
    return r->width * r->height;
}

static double rect_perimeter(const Shape *self) {
    const Rectangle *r = (const Rectangle *)self;
    return 2 * (r->width + r->height);
}

static void rect_destroy(Shape *self) {
    free(self);
}

Shape *rectangle_create(double width, double height) {
    Rectangle *r = (Rectangle *)calloc(1, sizeof(Rectangle));
    if (!r) return NULL;
    r->base.draw = rect_draw;
    r->base.area = rect_area;
    r->base.perimeter = rect_perimeter;
    r->base.destroy = rect_destroy;
    r->base.type_name = "Rectangle";
    r->width = width;
    r->height = height;
    return (Shape *)r;
}

typedef struct {
    Shape base;
    double side_a;
    double side_b;
    double side_c;
} Triangle;

static int valid_triangle(double a, double b, double c) {
    return (a + b > c) && (a + c > b) && (b + c > a);
}

static void tri_draw(const Shape *self) {
    const Triangle *t = (const Triangle *)self;
    printf("  画三角形: 边长=%.2f, %.2f, %.2f\n", t->side_a, t->side_b, t->side_c);
}

static double tri_area(const Shape *self) {
    const Triangle *t = (const Triangle *)self;
    double s = (t->side_a + t->side_b + t->side_c) / 2;
    return sqrt(s * (s - t->side_a) * (s - t->side_b) * (s - t->side_c));
}

static double tri_perimeter(const Shape *self) {
    const Triangle *t = (const Triangle *)self;
    return t->side_a + t->side_b + t->side_c;
}

static void tri_destroy(Shape *self) {
    free(self);
}

Shape *triangle_create(double a, double b, double c) {
    if (!valid_triangle(a, b, c)) return NULL;
    Triangle *t = (Triangle *)calloc(1, sizeof(Triangle));
    if (!t) return NULL;
    t->base.draw = tri_draw;
    t->base.area = tri_area;
    t->base.perimeter = tri_perimeter;
    t->base.destroy = tri_destroy;
    t->base.type_name = "Triangle";
    t->side_a = a;
    t->side_b = b;
    t->side_c = c;
    return (Shape *)t;
}

void demo_shape_polymorphism(void) {
    printf("\n=== demo_shape_polymorphism ===\n");
    printf("多态: 同一接口, 不同实现\n\n");

    Shape *shapes[] = {
        circle_create(5.0),
        rectangle_create(4.0, 6.0),
        triangle_create(3.0, 4.0, 5.0)
    };
    int count = sizeof(shapes) / sizeof(shapes[0]);

    for (int i = 0; i < count; i++) {
        printf("[%s]\n", shapes[i]->type_name);
        shape_draw(shapes[i]);
        printf("  面积=%.2f, 周长=%.2f\n\n",
               shape_area(shapes[i]), shape_perimeter(shapes[i]));
    }

    for (int i = 0; i < count; i++) {
        shape_destroy(shapes[i]);
    }
}

typedef struct {
    int (*compare)(const void *a, const void *b);
} Comparator;

int int_compare(const void *a, const void *b) {
    int ia = *(const int *)a, ib = *(const int *)b;
    return (ia > ib) - (ia < ib);
}

int double_compare(const void *a, const void *b) {
    double da = *(const double *)a;
    double db = *(const double *)b;
    if (da < db) return -1;
    if (da > db) return 1;
    return 0;
}

int str_compare(const void *a, const void *b) {
    return strcmp(*(const char **)a, *(const char **)b);
}

void generic_sort(void *base, size_t nmemb, size_t size,
                  int (*compare)(const void *, const void *)) {
    char *arr = (char *)base;
    char *temp = (char *)malloc(size);

    for (size_t i = 1; i < nmemb; i++) {
        memcpy(temp, arr + i * size, size);
        size_t j = i;
        while (j > 0 && compare(arr + (j - 1) * size, temp) > 0) {
            memcpy(arr + j * size, arr + (j - 1) * size, size);
            j--;
        }
        memcpy(arr + j * size, temp, size);
    }
    free(temp);
}

void demo_comparator_polymorphism(void) {
    printf("\n=== demo_comparator_polymorphism ===\n");
    printf("比较器多态: 通过函数指针实现不同类型的比较\n\n");

    int ints[] = {5, 2, 8, 1, 9, 3};
    int n_ints = sizeof(ints) / sizeof(ints[0]);
    printf("排序前: ");
    for (int i = 0; i < n_ints; i++) printf("%d ", ints[i]);
    generic_sort(ints, n_ints, sizeof(int), int_compare);
    printf("\n排序后: ");
    for (int i = 0; i < n_ints; i++) printf("%d ", ints[i]);
    printf("\n\n");

    double doubles[] = {3.14, 1.41, 2.72, 0.58};
    int n_doubles = sizeof(doubles) / sizeof(doubles[0]);
    printf("排序前: ");
    for (int i = 0; i < n_doubles; i++) printf("%.2f ", doubles[i]);
    generic_sort(doubles, n_doubles, sizeof(double), double_compare);
    printf("\n排序后: ");
    for (int i = 0; i < n_doubles; i++) printf("%.2f ", doubles[i]);
    printf("\n\n");

    const char *strings[] = {"banana", "apple", "cherry", "date"};
    int n_strings = sizeof(strings) / sizeof(strings[0]);
    printf("排序前: ");
    for (int i = 0; i < n_strings; i++) printf("%s ", strings[i]);
    generic_sort(strings, n_strings, sizeof(const char *), str_compare);
    printf("\n排序后: ");
    for (int i = 0; i < n_strings; i++) printf("%s ", strings[i]);
    printf("\n");
}

void demo_polymorphism_principles(void) {
    printf("\n=== demo_polymorphism_principles ===\n");
    printf("C语言实现多态的三种方式:\n\n");

    printf("1. 函数指针(最常用):\n");
    printf("   - 对象内嵌函数指针\n");
    printf("   - 运行时决定调用哪个函数\n");
    printf("   - 类似C++虚函数\n\n");

    printf("2. 回调函数:\n");
    printf("   - 将函数作为参数传递\n");
    printf("   - qsort()就是典型例子\n");
    printf("   - 算法与具体类型解耦\n\n");

    printf("3. 联合体+类型标签:\n");
    printf("   - 用tag标识当前类型\n");
    printf("   - switch分发到不同处理\n");
    printf("   - 简单但不易扩展\n\n");

    printf("多态的本质:\n");
    printf("  同一接口, 不同行为\n");
    printf("  调用方不需要知道具体类型\n");
    printf("  新增类型不需要修改调用方代码\n");
}

int main(void) {
    printf("多态: 函数指针、虚函数表实现运行时多态\n");

    demo_shape_polymorphism();
    demo_comparator_polymorphism();
    demo_polymorphism_principles();

    printf("\n所有演示完成!\n");
    return 0;
}
