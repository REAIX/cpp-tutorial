/** @file 01_example_struct_basics.c
 *  @brief 结构体基础：声明、初始化、成员访问、嵌套结构体
 *  @description 对应文档: 08-结构体与联合体
 */

#include <stdio.h>
#include <string.h>

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    char name[32];
    int age;
    float score;
} Student;

typedef struct {
    Point center;
    double radius;
} Circle;

typedef struct {
    char street[64];
    char city[32];
    int zip;
} Address;

typedef struct {
    char name[32];
    Address home;
    Address work;
} Person;

void demo_struct_declaration(void) {
    printf("=== 结构体声明 ===\n");

    struct { int a; int b; } anonymous = {1, 2};
    printf("匿名结构体: a=%d, b=%d\n", anonymous.a, anonymous.b);

    Point p = {3, 4};
    printf("Point (typedef): x=%d, y=%d\n", p.x, p.y);

    printf("\n结构体将多个相关变量组合成一个逻辑单元\n");
    printf("\n");
}

void demo_struct_initialization(void) {
    printf("=== 结构体初始化 ===\n");

    Student s1 = {"Alice", 20, 92.5f};
    printf("顺序初始化: name=%s, age=%d, score=%.1f\n", s1.name, s1.age, s1.score);

    Student s2 = {.score = 88.0f, .name = "Bob", .age = 21};
    printf("指定初始化器: name=%s, age=%d, score=%.1f\n", s2.name, s2.age, s2.score);

    Student s3 = {0};
    printf("零初始化: name=\"%s\", age=%d, score=%.1f\n", s3.name, s3.age, s3.score);

    Student s4;
    memset(&s4, 0, sizeof(s4));
    printf("memset清零: name=\"%s\", age=%d, score=%.1f\n", s4.name, s4.age, s4.score);

    printf("\n推荐: 使用指定初始化器, 代码更清晰, 不依赖成员顺序\n");
    printf("\n");
}

void demo_member_access(void) {
    printf("=== 成员访问 ===\n");

    Student s = {"Charlie", 22, 85.5f};
    Student *ps = &s;

    printf("点运算符 (.): s.name = %s\n", s.name);
    printf("箭头运算符 (->): ps->name = %s\n", ps->name);
    printf("解引用+点: (*ps).name = %s\n", (*ps).name);

    printf("\nps->name 等价于 (*ps).name, 箭头运算符更简洁\n");

    strcpy(ps->name, "David");
    ps->age = 23;
    ps->score = 90.0f;
    printf("通过指针修改后: name=%s, age=%d, score=%.1f\n", s.name, s.age, s.score);

    printf("\n");
}

void demo_nested_struct(void) {
    printf("=== 嵌套结构体 ===\n");

    Circle c = {{0, 0}, 5.0};
    printf("圆心: (%d, %d), 半径: %.1f\n", c.center.x, c.center.y, c.radius);

    Person p;
    strcpy(p.name, "张三");
    strcpy(p.home.street, "中山路100号");
    strcpy(p.home.city, "北京");
    p.home.zip = 100000;
    strcpy(p.work.street, "科技园A座");
    strcpy(p.work.city, "北京");
    p.work.zip = 100080;

    printf("\n姓名: %s\n", p.name);
    printf("家庭住址: %s, %s, %d\n", p.home.street, p.home.city, p.home.zip);
    printf("工作地址: %s, %s, %d\n", p.work.street, p.work.city, p.work.zip);

    printf("\n嵌套结构体通过多层点号访问: p.home.zip\n");
    printf("\n");
}

void demo_struct_assignment(void) {
    printf("=== 结构体赋值 ===\n");

    Student s1 = {"Eve", 20, 95.0f};
    Student s2;

    s2 = s1;
    printf("s2 = s1 后: name=%s, age=%d, score=%.1f\n", s2.name, s2.age, s2.score);

    strcpy(s2.name, "Frank");
    s2.score = 78.0f;
    printf("修改 s2 后: s1.name=%s, s2.name=%s\n", s1.name, s2.name);
    printf("结构体赋值是值拷贝, 修改副本不影响原值\n");

    printf("\n注意: 结构体包含指针成员时, 浅拷贝会导致共享指针!\n");
    printf("\n");
}

static void print_student_value(Student s) {
    printf("值传递: name=%s, age=%d, score=%.1f\n", s.name, s.age, s.score);
    s.score = 0;
}

static void print_student_pointer(const Student *s) {
    printf("指针传递: name=%s, age=%d, score=%.1f\n", s->name, s->age, s->score);
}

void demo_struct_as_function_param(void) {
    printf("=== 结构体作为函数参数 ===\n");

    Student s = {"Grace", 21, 88.0f};
    print_student_value(s);
    print_student_pointer(&s);
    printf("值传递后原值不变: score=%.1f\n", s.score);

    printf("\n推荐: 用 const 指针传递, 避免拷贝开销\n");
    printf("\n");
}

int main(void) {
    demo_struct_declaration();
    demo_struct_initialization();
    demo_member_access();
    demo_nested_struct();
    demo_struct_assignment();
    demo_struct_as_function_param();

    return 0;
}
