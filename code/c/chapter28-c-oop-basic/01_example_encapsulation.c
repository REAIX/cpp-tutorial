/**
 * @file 01_example_encapsulation.c
 * @brief 封装: 不透明指针、getter/setter
 * @description 对应文档: 28-C语言面向对象实现-基础
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct BankAccount BankAccount;

BankAccount *bank_account_create(const char *owner, double initial_balance);
void bank_account_destroy(BankAccount *acc);
const char *bank_account_get_owner(const BankAccount *acc);
double bank_account_get_balance(const BankAccount *acc);
int bank_account_deposit(BankAccount *acc, double amount);
int bank_account_withdraw(BankAccount *acc, double amount);
void bank_account_print(const BankAccount *acc);

struct BankAccount {
    char *owner;
    double balance;
    int is_closed;
};

BankAccount *bank_account_create(const char *owner, double initial_balance) {
    BankAccount *acc = (BankAccount *)malloc(sizeof(BankAccount));
    if (!acc) return NULL;

    acc->owner = strdup(owner);
    if (!acc->owner) { free(acc); return NULL; }

    acc->balance = initial_balance;
    acc->is_closed = 0;
    return acc;
}

void bank_account_destroy(BankAccount *acc) {
    if (!acc) return;
    free(acc->owner);
    acc->owner = NULL;
    acc->is_closed = 1;
    free(acc);
}

const char *bank_account_get_owner(const BankAccount *acc) {
    return acc ? acc->owner : NULL;
}

double bank_account_get_balance(const BankAccount *acc) {
    return acc ? acc->balance : 0.0;
}

int bank_account_deposit(BankAccount *acc, double amount) {
    if (!acc || acc->is_closed || amount <= 0) return -1;
    acc->balance += amount;
    return 0;
}

int bank_account_withdraw(BankAccount *acc, double amount) {
    if (!acc || acc->is_closed || amount <= 0) return -1;
    if (amount > acc->balance) return -2;
    acc->balance -= amount;
    return 0;
}

void bank_account_print(const BankAccount *acc) {
    if (!acc) return;
    printf("  账户[%s] 余额: %.2f\n", acc->owner, acc->balance);
}

void demo_opaque_pointer(void) {
    printf("\n=== demo_opaque_pointer ===\n");
    printf("不透明指针(Opaque Pointer): 隐藏实现细节\n");
    printf("头文件只声明 typedef struct X X; 和函数原型\n");
    printf("结构体定义在.c文件中, 用户无法直接访问成员\n\n");

    BankAccount *acc = bank_account_create("张三", 1000.0);
    bank_account_print(acc);

    bank_account_deposit(acc, 500.0);
    printf("存入500: ");
    bank_account_print(acc);

    int result = bank_account_withdraw(acc, 200.0);
    printf("取出200 (结果=%d): ", result);
    bank_account_print(acc);

    result = bank_account_withdraw(acc, 2000.0);
    printf("取出2000 (余额不足, 结果=%d): ", result);
    bank_account_print(acc);

    printf("通过getter获取: owner=%s, balance=%.2f\n",
           bank_account_get_owner(acc), bank_account_get_balance(acc));

    bank_account_destroy(acc);

    printf("\n封装优势:\n");
    printf("  1. 隐藏实现: 用户无法直接操作内部数据\n");
    printf("  2. 接口稳定: 修改结构体不影响调用代码\n");
    printf("  3. 不变量维护: 通过函数保证数据一致性\n");
    printf("  4. 编译隔离: 修改实现无需重新编译调用方\n");
}

typedef struct {
    int x;
    int y;
} Point;

typedef struct {
    Point center;
    double radius;
    int _ref_count;
} Circle;

Circle *circle_create(int x, int y, double radius) {
    Circle *c = (Circle *)malloc(sizeof(Circle));
    if (c) {
        c->center.x = x;
        c->center.y = y;
        c->radius = radius > 0 ? radius : 0;
        c->_ref_count = 1;
    }
    return c;
}

int circle_get_x(const Circle *c) { return c ? c->center.x : 0; }
int circle_get_y(const Circle *c) { return c ? c->center.y : 0; }
double circle_get_radius(const Circle *c) { return c ? c->radius : 0; }
double circle_get_area(const Circle *c) { return c ? 3.14159265 * c->radius * c->radius : 0; }

void circle_set_center(Circle *c, int x, int y) {
    if (c) { c->center.x = x; c->center.y = y; }
}

int circle_set_radius(Circle *c, double radius) {
    if (!c || radius < 0) return -1;
    c->radius = radius;
    return 0;
}

void circle_print(const Circle *c) {
    if (c) printf("  圆心(%d,%d) 半径=%.2f 面积=%.2f\n",
                  c->center.x, c->center.y, c->radius, circle_get_area(c));
}

void demo_getter_setter(void) {
    printf("\n=== demo_getter_setter ===\n");
    printf("getter/setter: 控制对内部数据的访问\n\n");

    Circle *c = circle_create(0, 0, 5.0);
    circle_print(c);

    circle_set_center(c, 10, 20);
    printf("移动圆心: ");
    circle_print(c);

    circle_set_radius(c, 10.0);
    printf("设置半径: ");
    circle_print(c);

    int r = circle_set_radius(c, -1.0);
    printf("设置负半径(结果=%d): ", r);
    circle_print(c);

    printf("\ngetter/setter设计原则:\n");
    printf("  1. 只暴露必要的访问接口\n");
    printf("  2. setter中验证数据有效性\n");
    printf("  3. 计算属性用函数实现(如area)\n");
    printf("  4. 只读属性只提供getter\n");
}

typedef struct {
    void *impl;
    int (*get_value)(const void *);
    void (*set_value)(void *, int);
    void (*destroy)(void *);
} EncapsulatedInt;

typedef struct {
    int value;
    int min;
    int max;
} BoundedInt;

int bounded_int_get(const void *impl) {
    return ((BoundedInt *)impl)->value;
}

void bounded_int_set(void *impl, int val) {
    BoundedInt *bi = (BoundedInt *)impl;
    if (val < bi->min) val = bi->min;
    if (val > bi->max) val = bi->max;
    bi->value = val;
}

void bounded_int_destroy(void *impl) {
    free(impl);
}

EncapsulatedInt *bounded_int_create(int initial, int min, int max) {
    BoundedInt *bi = (BoundedInt *)malloc(sizeof(BoundedInt));
    if (!bi) return NULL;
    bi->min = min;
    bi->max = max;
    bi->value = initial;
    if (bi->value < min) bi->value = min;
    if (bi->value > max) bi->value = max;

    EncapsulatedInt *ei = (EncapsulatedInt *)malloc(sizeof(EncapsulatedInt));
    if (!ei) { free(bi); return NULL; }
    ei->impl = bi;
    ei->get_value = bounded_int_get;
    ei->set_value = bounded_int_set;
    ei->destroy = bounded_int_destroy;
    return ei;
}

void demo_encapsulation_validation(void) {
    printf("\n=== demo_encapsulation_validation ===\n");
    printf("封装实现数据验证: 有界整数\n\n");

    EncapsulatedInt *val = bounded_int_create(50, 0, 100);
    printf("创建有界整数(50, 范围0-100): %d\n", val->get_value(val->impl));

    val->set_value(val->impl, 75);
    printf("设置75: %d\n", val->get_value(val->impl));

    val->set_value(val->impl, 150);
    printf("设置150(超出上限): %d (自动截断)\n", val->get_value(val->impl));

    val->set_value(val->impl, -50);
    printf("设置-50(超出下限): %d (自动截断)\n", val->get_value(val->impl));

    val->destroy(val->impl);
    free(val);

    printf("\n封装的核心思想:\n");
    printf("  数据 + 操作 = 对象\n");
    printf("  隐藏内部状态, 通过接口访问\n");
    printf("  保护不变量(invariant)\n");
}

int main(void) {
    printf("封装: 不透明指针、getter/setter\n");

    demo_opaque_pointer();
    demo_getter_setter();
    demo_encapsulation_validation();

    printf("\n所有演示完成!\n");
    return 0;
}
