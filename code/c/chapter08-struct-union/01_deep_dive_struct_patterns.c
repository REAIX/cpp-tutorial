/** @file 01_deep_dive_struct_patterns.c
 *  @brief 结构体进阶模式：不透明类型、结构体数组、链表节点、函数指针模拟方法
 *  @description 对应文档: 08-结构体与联合体 | 举一反三：结构体的高级设计模式
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <math.h>

typedef struct {
    int x;
    int y;
} Vec2;

float vec2_length(const Vec2 *v) {
    return (float)sqrt(v->x * v->x + v->y * v->y);
}

Vec2 vec2_add(const Vec2 *a, const Vec2 *b) {
    Vec2 result = {a->x + b->x, a->y + b->y};
    return result;
}

void vec2_scale(Vec2 *v, float s) {
    v->x = (int)(v->x * s);
    v->y = (int)(v->y * s);
}

void demo_opaque_type(void) {
    printf("=== 不透明类型 (Opaque Type) ===\n");

    typedef struct {
        int width;
        int height;
        int *pixels;
    } Image;

    Image *image_create(int w, int h) {
        Image *img = (Image *)malloc(sizeof(Image));
        if (!img) return NULL;
        img->width = w;
        img->height = h;
        img->pixels = (int *)calloc((size_t)(w * h), sizeof(int));
        if (!img->pixels) { free(img); return NULL; }
        return img;
    }

    void image_set_pixel(Image *img, int x, int y, int color) {
        if (img && x >= 0 && x < img->width && y >= 0 && y < img->height) {
            img->pixels[y * img->width + x] = color;
        }
    }

    int image_get_pixel(const Image *img, int x, int y) {
        if (img && x >= 0 && x < img->width && y >= 0 && y < img->height) {
            return img->pixels[y * img->width + x];
        }
        return -1;
    }

    void image_destroy(Image *img) {
        if (img) {
            free(img->pixels);
            free(img);
        }
    }

    Image *img = image_create(10, 10);
    if (img) {
        image_set_pixel(img, 5, 5, 0xFF0000);
        printf("像素 (5,5) 颜色: 0x%06X\n", image_get_pixel(img, 5, 5));
        printf("图像大小: %d x %d\n", img->width, img->height);
        image_destroy(img);
    }

    printf("\n不透明类型: 头文件只声明 struct, 实现细节隐藏在 .c 文件中\n");
    printf("用户只能通过提供的函数操作, 无法直接访问成员\n");

    printf("\n");
}

void demo_struct_array(void) {
    printf("=== 结构体数组 ===\n");

    typedef struct {
        char name[20];
        int score;
    } Record;

    Record records[] = {
        {"Alice", 92},
        {"Bob", 85},
        {"Charlie", 78},
        {"Diana", 95},
        {"Eve", 88}
    };
    int count = sizeof(records) / sizeof(records[0]);

    printf("原始记录:\n");
    for (int i = 0; i < count; i++) {
        printf("  %s: %d\n", records[i].name, records[i].score);
    }

    int compare_scores(const void *a, const void *b) {
        const Record *ra = (const Record *)a;
        const Record *rb = (const Record *)b;
        return rb->score - ra->score;
    }

    qsort(records, (size_t)count, sizeof(Record), compare_scores);

    printf("\n按分数降序排序:\n");
    for (int i = 0; i < count; i++) {
        printf("  %s: %d\n", records[i].name, records[i].score);
    }

    printf("\n");
}

typedef struct Node {
    int data;
    struct Node *next;
} Node;

Node *node_create(int data) {
    Node *n = (Node *)malloc(sizeof(Node));
    if (n) {
        n->data = data;
        n->next = NULL;
    }
    return n;
}

void list_append(Node **head, int data) {
    Node *new_node = node_create(data);
    if (!new_node) return;

    if (*head == NULL) {
        *head = new_node;
        return;
    }

    Node *curr = *head;
    while (curr->next != NULL) {
        curr = curr->next;
    }
    curr->next = new_node;
}

void list_print(const Node *head) {
    const Node *curr = head;
    while (curr != NULL) {
        printf("%d -> ", curr->data);
        curr = curr->next;
    }
    printf("NULL\n");
}

void list_free(Node *head) {
    Node *curr = head;
    while (curr != NULL) {
        Node *next = curr->next;
        free(curr);
        curr = next;
    }
}

void demo_linked_list(void) {
    printf("=== 链表节点 ===\n");

    Node *head = NULL;
    list_append(&head, 10);
    list_append(&head, 20);
    list_append(&head, 30);
    list_append(&head, 40);

    printf("链表内容: ");
    list_print(head);

    printf("链表是结构体 + 指针的经典应用\n");
    printf("每个节点包含数据和指向下一个节点的指针\n");

    list_free(head);

    printf("\n");
}

typedef struct {
    float x;
    float y;
} Vec2f;

typedef struct {
    Vec2f (*add)(const Vec2f *, const Vec2f *);
    float (*length)(const Vec2f *);
    void (*scale)(Vec2f *, float);
    void (*print)(const Vec2f *);
} Vec2VTable;

Vec2f vec2f_add(const Vec2f *a, const Vec2f *b) {
    Vec2f result = {a->x + b->x, a->y + b->y};
    return result;
}

float vec2f_length(const Vec2f *v) {
    return sqrtf(v->x * v->x + v->y * v->y);
}

void vec2f_scale(Vec2f *v, float s) {
    v->x *= s;
    v->y *= s;
}

void vec2f_print(const Vec2f *v) {
    printf("(%.1f, %.1f)", v->x, v->y);
}

void demo_function_pointer_method(void) {
    printf("=== 函数指针模拟方法 (面向对象) ===\n");

    Vec2VTable vtable = {
        .add = vec2f_add,
        .length = vec2f_length,
        .scale = vec2f_scale,
        .print = vec2f_print
    };

    Vec2f a = {3.0f, 4.0f};
    Vec2f b = {1.0f, 2.0f};

    printf("a = ");
    vtable.print(&a);
    printf(", 长度 = %.2f\n", vtable.length(&a));

    printf("b = ");
    vtable.print(&b);
    printf("\n");

    Vec2f c = vtable.add(&a, &b);
    printf("a + b = ");
    vtable.print(&c);
    printf("\n");

    vtable.scale(&a, 2.0f);
    printf("a * 2 = ");
    vtable.print(&a);
    printf("\n");

    printf("\n函数指针表 (vtable) 是 C 实现多态的基础\n");
    printf("C++ 的虚函数表就是这种模式的编译器实现\n");

    printf("\n");
}

typedef struct {
    char name[32];
    void (*speak)(const char *name);
} Animal;

void dog_speak(const char *name) { printf("%s: 汪汪!\n", name); }
void cat_speak(const char *name) { printf("%s: 喵喵!\n", name); }
void cow_speak(const char *name) { printf("%s: 哞哞!\n", name); }

void demo_simple_polymorphism(void) {
    printf("=== 简单多态示例 ===\n");

    Animal animals[] = {
        {"小狗", dog_speak},
        {"小猫", cat_speak},
        {"小牛", cow_speak}
    };

    int count = sizeof(animals) / sizeof(animals[0]);
    for (int i = 0; i < count; i++) {
        animals[i].speak(animals[i].name);
    }

    printf("\n通过函数指针, 同一接口表现出不同行为 = 多态\n");

    printf("\n");
}

int main(void) {
    demo_opaque_type();
    demo_struct_array();
    demo_linked_list();
    demo_function_pointer_method();
    demo_simple_polymorphism();

    return 0;
}
