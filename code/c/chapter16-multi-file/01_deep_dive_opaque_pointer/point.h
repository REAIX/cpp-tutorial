/**
 * @file point.h
 * @brief 不透明指针模式 - 点的接口
 * @description 对应文档: 16-多文件编程 - 接口与实现分离, 不透明指针模式实现封装
 */
#ifndef POINT_H
#define POINT_H

typedef struct point point_t;

point_t *point_create(double x, double y);
void point_destroy(point_t *p);

double point_get_x(const point_t *p);
double point_get_y(const point_t *p);
void point_set_x(point_t *p, double x);
void point_set_y(point_t *p, double y);

double point_distance(const point_t *a, const point_t *b);
void point_translate(point_t *p, double dx, double dy);
void point_print(const point_t *p);

#endif
