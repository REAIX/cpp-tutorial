/**
 * @file point.c
 * @brief 不透明指针模式 - 点的实现
 * @description 对应文档: 16-多文件编程 - 实现细节对调用者隐藏
 */
#include "point.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

struct point {
    double x;
    double y;
};

point_t *point_create(double x, double y) {
    point_t *p = malloc(sizeof(point_t));
    if (p) {
        p->x = x;
        p->y = y;
    }
    return p;
}

void point_destroy(point_t *p) {
    free(p);
}

double point_get_x(const point_t *p) {
    return p ? p->x : 0.0;
}

double point_get_y(const point_t *p) {
    return p ? p->y : 0.0;
}

void point_set_x(point_t *p, double x) {
    if (p) p->x = x;
}

void point_set_y(point_t *p, double y) {
    if (p) p->y = y;
}

double point_distance(const point_t *a, const point_t *b) {
    if (!a || !b) return -1.0;
    double dx = a->x - b->x;
    double dy = a->y - b->y;
    return sqrt(dx * dx + dy * dy);
}

void point_translate(point_t *p, double dx, double dy) {
    if (p) {
        p->x += dx;
        p->y += dy;
    }
}

void point_print(const point_t *p) {
    if (p) {
        printf("Point(%.2f, %.2f)", p->x, p->y);
    } else {
        printf("Point(NULL)");
    }
}
