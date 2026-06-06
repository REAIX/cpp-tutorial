/**
 * @file 01_deep_dive_pattern_catalog.c
 * @brief 设计模式目录: 状态模式、命令模式、装饰器模式
 * @description 对应文档: 30-C语言设计模式实现
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct StateMachine StateMachine;

typedef struct {
    const char *name;
    void (*on_enter)(StateMachine *sm);
    void (*on_exit)(StateMachine *sm);
    void (*handle_event)(StateMachine *sm, const char *event);
} State;

struct StateMachine {
    State *current;
    State *idle_state;
    State *running_state;
    State *paused_state;
    int counter;
};

static StateMachine g_sm;

static void sm_transition(StateMachine *sm, State *new_state) {
    if (sm->current == new_state) return;
    if (sm->current && sm->current->on_exit) sm->current->on_exit(sm);
    printf("  状态转换: %s -> %s\n",
           sm->current ? sm->current->name : "NULL",
           new_state ? new_state->name : "NULL");
    sm->current = new_state;
    if (sm->current && sm->current->on_enter) sm->current->on_enter(sm);
}

static void idle_on_enter(StateMachine *sm) { (void)sm; printf("  [空闲] 等待开始...\n"); }
static void idle_handle(StateMachine *sm, const char *event) {
    if (strcmp(event, "START") == 0) sm_transition(sm, sm->running_state);
    else printf("  [空闲] 忽略事件: %s\n", event);
}

static void running_on_enter(StateMachine *sm) { printf("  [运行] 开始计数\n"); sm->counter = 0; }
static void running_handle(StateMachine *sm, const char *event) {
    if (strcmp(event, "PAUSE") == 0) sm_transition(sm, sm->paused_state);
    else if (strcmp(event, "STOP") == 0) sm_transition(sm, sm->idle_state);
    else if (strcmp(event, "TICK") == 0) { sm->counter++; printf("  [运行] tick: %d\n", sm->counter); }
    else printf("  [运行] 忽略事件: %s\n", event);
}

static void paused_on_enter(StateMachine *sm) { printf("  [暂停] 计数=%d\n", sm->counter); }
static void paused_on_exit(StateMachine *sm) { (void)sm; printf("  [暂停] 离开暂停\n"); }
static void paused_handle(StateMachine *sm, const char *event) {
    if (strcmp(event, "RESUME") == 0) sm_transition(sm, sm->running_state);
    else if (strcmp(event, "STOP") == 0) sm_transition(sm, sm->idle_state);
    else printf("  [暂停] 忽略事件: %s\n", event);
}

static State idle_state = {"空闲", idle_on_enter, NULL, idle_handle};
static State running_state = {"运行", running_on_enter, NULL, running_handle};
static State paused_state = {"暂停", paused_on_enter, paused_on_exit, paused_handle};

void demo_state_pattern(void) {
    printf("\n=== demo_state_pattern ===\n");
    printf("状态模式: 对象行为随状态变化, 状态转换封装\n\n");

    g_sm.idle_state = &idle_state;
    g_sm.running_state = &running_state;
    g_sm.paused_state = &paused_state;
    g_sm.counter = 0;

    sm_transition(&g_sm, &idle_state);

    const char *events[] = {"START", "TICK", "TICK", "TICK", "PAUSE", "TICK", "RESUME", "TICK", "STOP"};
    for (int i = 0; i < 9; i++) {
        printf("\n事件: %s\n", events[i]);
        g_sm.current->handle_event(&g_sm, events[i]);
    }

    printf("\n状态模式优势:\n");
    printf("  1. 状态逻辑分散到各状态类, 而非巨大switch\n");
    printf("  2. 新增状态只需添加State结构体\n");
    printf("  3. 状态转换逻辑集中, 易于维护\n");
}

typedef struct {
    const char *name;
    void (*execute)(void *self);
    void (*undo)(void *self);
} Command;

typedef struct {
    int *target;
    int value;
    int old_value;
    Command base;
} AddCommand;

static void add_execute(void *self) {
    AddCommand *cmd = (AddCommand *)self;
    cmd->old_value = *cmd->target;
    *cmd->target += cmd->value;
    printf("  执行: %d + %d = %d\n", cmd->old_value, cmd->value, *cmd->target);
}

static void add_undo(void *self) {
    AddCommand *cmd = (AddCommand *)self;
    *cmd->target = cmd->old_value;
    printf("  撤销: 恢复为 %d\n", *cmd->target);
}

AddCommand *add_command_create(int *target, int value) {
    AddCommand *cmd = (AddCommand *)calloc(1, sizeof(AddCommand));
    if (cmd) {
        cmd->target = target;
        cmd->value = value;
        cmd->base.name = "Add";
        cmd->base.execute = add_execute;
        cmd->base.undo = add_undo;
    }
    return cmd;
}

#define MAX_HISTORY 16

typedef struct {
    Command *history[MAX_HISTORY];
    int count;
} CommandHistory;

void history_push(CommandHistory *h, Command *cmd) {
    if (h->count >= MAX_HISTORY) {
        free(h->history[0]);
        for (int i = 1; i < h->count; i++) h->history[i - 1] = h->history[i];
        h->count--;
    }
    h->history[h->count++] = cmd;
}

Command *history_pop(CommandHistory *h) {
    if (h->count <= 0) return NULL;
    return h->history[--h->count];
}

void demo_command_pattern(void) {
    printf("\n=== demo_command_pattern ===\n");
    printf("命令模式: 将操作封装为对象, 支持撤销/重做\n\n");

    int value = 100;
    CommandHistory history = {.count = 0};

    printf("初始值: %d\n\n", value);

    AddCommand *cmd1 = add_command_create(&value, 10);
    cmd1->base.execute(cmd1);
    history_push(&history, (Command *)cmd1);

    AddCommand *cmd2 = add_command_create(&value, 25);
    cmd2->base.execute(cmd2);
    history_push(&history, (Command *)cmd2);

    AddCommand *cmd3 = add_command_create(&value, 50);
    cmd3->base.execute(cmd3);
    history_push(&history, (Command *)cmd3);

    printf("\n撤销操作:\n");
    Command *last = history_pop(&history);
    if (last) last->undo(last);

    last = history_pop(&history);
    if (last) last->undo(last);

    printf("\n当前值: %d\n", value);

    for (int i = 0; i < history.count; i++) free(history.history[i]);

    printf("\n命令模式优势:\n");
    printf("  1. 操作对象化: 可存储、传递、排队\n");
    printf("  2. 撤销/重做: 保存历史命令\n");
    printf("  3. 宏命令: 组合多个命令\n");
    printf("  4. 延迟执行: 创建命令后稍后执行\n");
}

typedef struct {
    void (*print_info)(const void *self);
    double (*cost)(const void *self);
    void (*destroy)(void *self);
} CoffeeVTable;

typedef struct {
    const CoffeeVTable *vtable;
} Coffee;

typedef struct {
    Coffee base;
} SimpleCoffee;

static void sc_info(const void *self) {
    (void)self;
    printf("  简单咖啡");
}

static double sc_cost(const void *self) { (void)self; return 10.0; }
static void sc_destroy(void *self) { free(self); }

static const CoffeeVTable simple_vtable = { sc_info, sc_cost, sc_destroy };

Coffee *simple_coffee_create(void) {
    SimpleCoffee *c = (SimpleCoffee *)calloc(1, sizeof(SimpleCoffee));
    if (c) c->base.vtable = &simple_vtable;
    return (Coffee *)c;
}

typedef struct {
    Coffee base;
    Coffee *wrapped;
    const char *decorator_name;
    double extra_cost;
} CoffeeDecorator;

static void dec_info(const void *self) {
    const CoffeeDecorator *d = (const CoffeeDecorator *)self;
    d->wrapped->vtable->print_info(d->wrapped);
    printf(" + %s", d->decorator_name);
}

static double dec_cost(const void *self) {
    const CoffeeDecorator *d = (const CoffeeDecorator *)self;
    return d->wrapped->vtable->cost(d->wrapped) + d->extra_cost;
}

static void dec_destroy(void *self) {
    CoffeeDecorator *d = (CoffeeDecorator *)self;
    d->wrapped->vtable->destroy(d->wrapped);
    free(self);
}

static const CoffeeVTable decorator_vtable = { dec_info, dec_cost, dec_destroy };

Coffee *coffee_add_milk(Coffee *base) {
    CoffeeDecorator *d = (CoffeeDecorator *)calloc(1, sizeof(CoffeeDecorator));
    if (d) {
        d->base.vtable = &decorator_vtable;
        d->wrapped = base;
        d->decorator_name = "牛奶";
        d->extra_cost = 3.0;
    }
    return (Coffee *)d;
}

Coffee *coffee_add_mocha(Coffee *base) {
    CoffeeDecorator *d = (CoffeeDecorator *)calloc(1, sizeof(CoffeeDecorator));
    if (d) {
        d->base.vtable = &decorator_vtable;
        d->wrapped = base;
        d->decorator_name = "摩卡";
        d->extra_cost = 5.0;
    }
    return (Coffee *)d;
}

Coffee *coffee_add_whip(Coffee *base) {
    CoffeeDecorator *d = (CoffeeDecorator *)calloc(1, sizeof(CoffeeDecorator));
    if (d) {
        d->base.vtable = &decorator_vtable;
        d->wrapped = base;
        d->decorator_name = "奶泡";
        d->extra_cost = 2.0;
    }
    return (Coffee *)d;
}

void demo_decorator_pattern(void) {
    printf("\n=== demo_decorator_pattern ===\n");
    printf("装饰器模式: 动态添加功能, 不修改原始对象\n\n");

    Coffee *c1 = simple_coffee_create();
    c1->vtable->print_info(c1);
    printf(" = %.1f元\n", c1->vtable->cost(c1));
    c1->vtable->destroy(c1);

    Coffee *c2 = simple_coffee_create();
    c2 = coffee_add_milk(c2);
    c2->vtable->print_info(c2);
    printf(" = %.1f元\n", c2->vtable->cost(c2));
    c2->vtable->destroy(c2);

    Coffee *c3 = simple_coffee_create();
    c3 = coffee_add_milk(c3);
    c3 = coffee_add_mocha(c3);
    c3 = coffee_add_whip(c3);
    c3->vtable->print_info(c3);
    printf(" = %.1f元\n", c3->vtable->cost(c3));
    c3->vtable->destroy(c3);

    printf("\n装饰器模式优势:\n");
    printf("  1. 动态组合: 运行时添加任意装饰\n");
    printf("  2. 开放扩展: 新增装饰器不影响已有代码\n");
    printf("  3. 单一职责: 每个装饰器只负责一个功能\n");
    printf("  4. 嵌套装饰: 装饰器可以叠加\n");
}

int main(void) {
    printf("设计模式目录: 状态模式、命令模式、装饰器模式\n");

    demo_state_pattern();
    demo_command_pattern();
    demo_decorator_pattern();

    printf("\n所有演示完成!\n");
    return 0;
}
