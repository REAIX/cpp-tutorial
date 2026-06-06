/**
 * @file 02_deep_dive_oop_best_practices.c
 * @brief OOP最佳实践: 何时用OOP、常见错误、与C++对比
 * @description 对应文档: 29-C语言面向对象实现-进阶
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    char name[64];
    int health;
    int attack;
    int defense;
} GameCharacter;

void character_init(GameCharacter *c, const char *name, int hp, int atk, int def) {
    strncpy(c->name, name, sizeof(c->name) - 1);
    c->name[sizeof(c->name) - 1] = '\0';
    c->health = hp;
    c->attack = atk;
    c->defense = def;
}

void character_attack(const GameCharacter *attacker, GameCharacter *target) {
    int damage = attacker->attack - target->defense;
    if (damage < 1) damage = 1;
    target->health -= damage;
    printf("  %s 攻击 %s, 造成 %d 伤害 (HP: %d)\n",
           attacker->name, target->name, damage, target->health);
}

int character_is_alive(const GameCharacter *c) {
    return c->health > 0;
}

void demo_when_to_use_oop(void) {
    printf("\n=== demo_when_to_use_oop ===\n");
    printf("何时使用OOP in C?\n\n");

    printf("适合OOP的场景:\n");
    printf("  1. 复杂系统: 多种相关类型, 需要多态\n");
    printf("  2. 插件架构: 需要运行时扩展\n");
    printf("  3. GUI系统: 大量不同控件, 统一接口\n");
    printf("  4. 游戏开发: 大量实体, 行为各异\n\n");

    printf("不适合OOP的场景:\n");
    printf("  1. 简单工具: 过度设计, 增加复杂度\n");
    printf("  2. 性能关键: 函数指针间接调用有开销\n");
    printf("  3. 嵌入式开发: 内存和性能受限\n");
    printf("  4. 纯数据处理: 算法为主, 不需要封装\n\n");

    GameCharacter hero, monster;
    character_init(&hero, "勇者", 100, 20, 10);
    character_init(&monster, "史莱姆", 50, 8, 3);

    printf("简单游戏角色(不需要OOP):\n");
    while (character_is_alive(&hero) && character_is_alive(&monster)) {
        character_attack(&hero, &monster);
        if (character_is_alive(&monster)) {
            character_attack(&monster, &hero);
        }
    }
    printf("  %s 获胜!\n", character_is_alive(&hero) ? hero.name : monster.name);
}

typedef struct {
    char *data;
    size_t length;
} BadString;

BadString bad_string_create(const char *init) {
    BadString s;
    s.length = strlen(init);
    s.data = (char *)malloc(s.length + 1);
    strcpy(s.data, init);
    return s;
}

void demo_common_mistakes(void) {
    printf("\n=== demo_common_mistakes ===\n");
    printf("C语言OOP常见错误:\n\n");

    printf("错误1: 返回栈上对象(悬空指针)\n");
    printf("  BadString s = bad_string_create(\"hello\"); // data在堆上, 但s在栈上\n");
    printf("  返回结构体时data指针可能失效(如果返回局部变量)\n");
    printf("  修正: 返回堆分配的对象指针\n\n");

    printf("错误2: 忘记调用destroy(内存泄漏)\n");
    printf("  String *s = string_create(\"hello\");\n");
    printf("  // 忘记 string_destroy(s) -> 内存泄漏!\n");
    printf("  修正: 建立所有权规则, 谁创建谁销毁\n\n");

    printf("错误3: 错误的类型转换(未定义行为)\n");
    printf("  Circle *c = (Circle *)rectangle_create(4, 6); // 强制转换错误!\n");
    printf("  修正: 使用类型标签或安全的类型检查\n\n");

    printf("错误4: vtable指针未初始化\n");
    printf("  Shape s; s.vtable->draw(&s); // vtable是NULL -> 崩溃!\n");
    printf("  修正: 始终使用create函数创建对象\n\n");

    printf("错误5: 浅拷贝导致双重释放\n");
    printf("  String *a = string_create(\"hello\");\n");
    printf("  String *b = a; // 浅拷贝, 共享data指针\n");
    printf("  string_destroy(a); // 释放data\n");
    printf("  string_destroy(b); // 双重释放! 崩溃!\n");
    printf("  修正: 实现深拷贝函数或使用引用计数\n\n");

    printf("错误6: 不透明指针不完整\n");
    printf("  头文件暴露了结构体定义 -> 用户直接访问成员\n");
    printf("  修正: 头文件只放前向声明 typedef struct X X;\n");
}

void demo_oop_vs_cpp(void) {
    printf("\n=== demo_oop_vs_cpp ===\n");
    printf("C语言OOP vs C++ OOP:\n\n");

    printf("特性             C语言OOP           C++ OOP\n");
    printf("封装             不透明指针          class+访问控制\n");
    printf("继承             结构体嵌入          class : public\n");
    printf("多态             函数指针/vtable     virtual函数\n");
    printf("构造/析构        手动create/destroy  自动构造/析构\n");
    printf("内存管理         手动malloc/free     RAII智能指针\n");
    printf("异常             setjmp/longjmp      try/catch\n");
    printf("泛型             宏/void*            模板\n");
    printf("类型安全         无(运行时)          强(编译时)\n");
    printf("运行时开销       手动控制            虚函数表\n");
    printf("代码量           多(手动实现)        少(编译器生成)\n\n");

    printf("C语言OOP优势:\n");
    printf("  1. 完全控制内存布局和开销\n");
    printf("  2. 无隐式行为(无构造/析构调用)\n");
    printf("  3. 可移植性更好\n");
    printf("  4. 适合底层开发(内核、驱动、嵌入式)\n\n");

    printf("C++ OOP优势:\n");
    printf("  1. 编译器自动生成大量代码\n");
    printf("  2. 类型安全, 编译时检查\n");
    printf("  3. RAII自动资源管理\n");
    printf("  4. 标准库丰富, 生态完善\n\n");

    printf("选择建议:\n");
    printf("  - 系统编程/嵌入式: C语言OOP\n");
    printf("  - 应用开发/游戏引擎: C++ OOP\n");
    printf("  - 混合: C实现核心, C++包装接口\n");
}

typedef struct {
    const char *type;
    void (*action)(void *self);
} TypeInfo;

typedef struct {
    TypeInfo *type_info;
    int x;
    int y;
} SafeObject;

void safe_object_check_type(SafeObject *obj, const char *expected) {
    if (!obj || !obj->type_info || strcmp(obj->type_info->type, expected) != 0) {
        printf("  [类型错误] 期望 %s, 实际 %s\n",
               expected, obj && obj->type_info ? obj->type_info->type : "NULL");
    }
}

void demo_defensive_programming(void) {
    printf("\n=== demo_defensive_programming ===\n");
    printf("防御性编程: 在OOP中增加安全检查\n\n");

    printf("1. 类型标签: 每个对象附带类型信息\n");
    printf("   防止错误的类型转换\n\n");

    printf("2. 魔数验证: 对象创建时设置, destroy时清除\n");
    printf("   检测use-after-free\n\n");

    printf("3. 空指针检查: 所有公有函数入口检查参数\n");
    printf("   防止空指针解引用\n\n");

    printf("4. 状态检查: 操作前验证对象状态\n");
    printf("   防止对已关闭/已销毁对象操作\n\n");

    printf("5. 边界检查: 数组访问前验证索引\n");
    printf("   防止缓冲区溢出\n\n");

    printf("C语言OOP最佳实践总结:\n");
    printf("  1. 一致性: create/destroy, init/cleanup 命名统一\n");
    printf("  2. 所有权: 明确每个对象的拥有者\n");
    printf("  3. 不透明: 头文件只暴露接口, 隐藏实现\n");
    printf("  4. 不可变: 尽量提供只读接口, 减少状态变更\n");
    printf("  5. 防御: 所有入口检查参数, 所有出口检查结果\n");
    printf("  6. 文档: 注释说明前置条件、后置条件、线程安全性\n");
}

int main(void) {
    printf("OOP最佳实践: 何时用OOP、常见错误、与C++对比\n");

    demo_when_to_use_oop();
    demo_common_mistakes();
    demo_oop_vs_cpp();
    demo_defensive_programming();

    printf("\n所有演示完成!\n");
    return 0;
}
