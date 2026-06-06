/**
 * @file 02_example_inheritance.c
 * @brief 继承: 结构体嵌入、vtable概念
 * @description 对应文档: 28-C语言面向对象实现-基础
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[64];
    int age;
} Person;

void person_init(Person *p, const char *name, int age) {
    strncpy(p->name, name, sizeof(p->name) - 1);
    p->name[sizeof(p->name) - 1] = '\0';
    p->age = age;
}

void person_greet(const Person *p) {
    printf("  你好, 我是%s, 今年%d岁\n", p->name, p->age);
}

typedef struct {
    Person base;
    char school[64];
} Student;

void student_init(Student *s, const char *name, int age, const char *school) {
    person_init(&s->base, name, age);
    strncpy(s->school, school, sizeof(s->school) - 1);
    s->school[sizeof(s->school) - 1] = '\0';
}

void student_study(const Student *s) {
    printf("  %s在%s学习\n", s->base.name, s->school);
}

typedef struct {
    Person base;
    char company[64];
    double salary;
} Employee;

void employee_init(Employee *e, const char *name, int age, const char *company, double salary) {
    person_init(&e->base, name, age);
    strncpy(e->company, company, sizeof(e->company) - 1);
    e->company[sizeof(e->company) - 1] = '\0';
    e->salary = salary;
}

void employee_work(const Employee *e) {
    printf("  %s在%s工作, 薪水%.2f\n", e->base.name, e->company, e->salary);
}

void demo_struct_embedding(void) {
    printf("\n=== demo_struct_embedding ===\n");
    printf("结构体嵌入实现继承: 子结构体包含父结构体作为第一个成员\n\n");

    Person p;
    person_init(&p, "张三", 30);
    printf("Person:\n");
    person_greet(&p);

    Student s;
    student_init(&s, "李四", 20, "清华大学");
    printf("Student:\n");
    person_greet(&s.base);
    student_study(&s);

    Employee e;
    employee_init(&e, "王五", 35, "阿里巴巴", 25000.0);
    printf("Employee:\n");
    person_greet(&e.base);
    employee_work(&e);

    printf("\n关键: Student和Employee的第一个成员是Person\n");
    printf("可以安全地将 Student* 转换为 Person*\n");
}

typedef struct {
    void (*greet)(void *self);
    void (*describe)(void *self);
    size_t base_offset;
} VTable;

typedef struct {
    const VTable *vtable;
    char name[64];
    int age;
} PersonV;

typedef struct {
    PersonV base;
    char school[64];
} StudentV;

typedef struct {
    PersonV base;
    char company[64];
    double salary;
} EmployeeV;

void person_v_greet(void *self) {
    PersonV *p = (PersonV *)self;
    printf("  你好, 我是%s\n", p->name);
}

void person_v_describe(void *self) {
    PersonV *p = (PersonV *)self;
    printf("  %s, %d岁\n", p->name, p->age);
}

void student_v_greet(void *self) {
    StudentV *s = (StudentV *)self;
    printf("  同学你好, 我是%s, 来自%s\n", s->base.name, s->school);
}

void student_v_describe(void *self) {
    StudentV *s = (StudentV *)self;
    printf("  %s, %d岁, 学生, %s\n", s->base.name, s->base.age, s->school);
}

void employee_v_greet(void *self) {
    EmployeeV *e = (EmployeeV *)self;
    printf("  您好, 我是%s, 在%s工作\n", e->base.name, e->company);
}

void employee_v_describe(void *self) {
    EmployeeV *e = (EmployeeV *)self;
    printf("  %s, %d岁, 员工, %s, 薪水%.0f\n",
           e->base.name, e->base.age, e->company, e->salary);
}

static const VTable person_vtable = {
    .greet = person_v_greet,
    .describe = person_v_describe,
    .base_offset = 0
};

static const VTable student_vtable = {
    .greet = student_v_greet,
    .describe = student_v_describe,
    .base_offset = 0
};

static const VTable employee_vtable = {
    .greet = employee_v_greet,
    .describe = employee_v_describe,
    .base_offset = 0
};

void demo_vtable_concept(void) {
    printf("\n=== demo_vtable_concept ===\n");
    printf("虚函数表(vtable): 实现运行时多态的基础\n\n");

    PersonV pv;
    pv.vtable = &person_vtable;
    strncpy(pv.name, "张三", sizeof(pv.name));
    pv.age = 30;

    StudentV sv;
    sv.base.vtable = &student_vtable;
    strncpy(sv.base.name, "李四", sizeof(sv.base.name));
    sv.base.age = 20;
    strncpy(sv.school, "清华大学", sizeof(sv.school));

    EmployeeV ev;
    ev.base.vtable = &employee_vtable;
    strncpy(ev.base.name, "王五", sizeof(ev.base.name));
    ev.base.age = 35;
    strncpy(ev.company, "阿里巴巴", sizeof(ev.company));
    ev.salary = 25000.0;

    PersonV *people[] = {&pv, (PersonV *)&sv, (PersonV *)&ev};

    for (int i = 0; i < 3; i++) {
        people[i]->vtable->greet(people[i]);
        people[i]->vtable->describe(people[i]);
    }

    printf("\nvtable原理:\n");
    printf("  1. 每个类有一个虚函数表(函数指针数组)\n");
    printf("  2. 每个对象包含指向vtable的指针\n");
    printf("  3. 调用虚函数时通过vtable间接调用\n");
    printf("  4. 子类覆盖父类函数时, vtable中替换对应指针\n");
}

void demo_inheritance_upcast(void) {
    printf("\n=== demo_inheritance_upcast ===\n");
    printf("向上转型: 子类指针安全转换为父类指针\n\n");

    Student s;
    student_init(&s, "赵六", 22, "北京大学");

    Person *pp = &s.base;
    printf("Student* -> Person* (向上转型):\n");
    person_greet(pp);

    printf("\n向上转型规则:\n");
    printf("  1. 子类指针可以安全转为父类指针\n");
    printf("  2. 因为父结构体是子结构体的第一个成员\n");
    printf("  3. 内存布局保证地址相同\n");
    printf("  4. 向下转型(父->子)不安全, 需要确认实际类型\n");
}

int main(void) {
    printf("继承: 结构体嵌入、vtable概念\n");

    demo_struct_embedding();
    demo_vtable_concept();
    demo_inheritance_upcast();

    printf("\n所有演示完成!\n");
    return 0;
}
