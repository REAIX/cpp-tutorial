/**
 * @file 03_example_stack.c
 * @brief 栈的实现: 数组栈和链表栈
 * @description 对应文档: 26-链表与数据结构
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define ARRAY_STACK_SIZE 64

typedef struct {
    int data[ARRAY_STACK_SIZE];
    int top;
} ArrayStack;

void astack_init(ArrayStack *s) {
    s->top = -1;
}

int astack_is_empty(const ArrayStack *s) {
    return s->top == -1;
}

int astack_is_full(const ArrayStack *s) {
    return s->top == ARRAY_STACK_SIZE - 1;
}

int astack_push(ArrayStack *s, int value) {
    if (astack_is_full(s)) return -1;
    s->data[++s->top] = value;
    return 0;
}

int astack_pop(ArrayStack *s, int *value) {
    if (astack_is_empty(s)) return -1;
    *value = s->data[s->top--];
    return 0;
}

int astack_peek(const ArrayStack *s, int *value) {
    if (astack_is_empty(s)) return -1;
    *value = s->data[s->top];
    return 0;
}

void astack_print(const ArrayStack *s) {
    printf("栈(底->顶): [");
    for (int i = 0; i <= s->top; i++) {
        printf("%d", s->data[i]);
        if (i < s->top) printf(", ");
    }
    printf("]\n");
}

typedef struct SNode {
    int data;
    struct SNode *next;
} SNode;

typedef struct {
    SNode *top;
    int size;
} LinkedStack;

void lstack_init(LinkedStack *s) {
    s->top = NULL;
    s->size = 0;
}

int lstack_is_empty(const LinkedStack *s) {
    return s->top == NULL;
}

int lstack_push(LinkedStack *s, int value) {
    SNode *n = (SNode *)malloc(sizeof(SNode));
    if (!n) return -1;
    n->data = value;
    n->next = s->top;
    s->top = n;
    s->size++;
    return 0;
}

int lstack_pop(LinkedStack *s, int *value) {
    if (lstack_is_empty(s)) return -1;
    SNode *temp = s->top;
    *value = temp->data;
    s->top = temp->next;
    free(temp);
    s->size--;
    return 0;
}

int lstack_peek(const LinkedStack *s, int *value) {
    if (lstack_is_empty(s)) return -1;
    *value = s->top->data;
    return 0;
}

void lstack_destroy(LinkedStack *s) {
    while (!lstack_is_empty(s)) {
        int val;
        lstack_pop(s, &val);
    }
}

void lstack_print(const LinkedStack *s) {
    printf("栈(底->顶): [");
    int *arr = (int *)malloc(s->size * sizeof(int));
    SNode *cur = s->top;
    int i = s->size - 1;
    while (cur) {
        arr[i--] = cur->data;
        cur = cur->next;
    }
    for (int j = 0; j < s->size; j++) {
        printf("%d", arr[j]);
        if (j < s->size - 1) printf(", ");
    }
    printf("]\n");
    free(arr);
}

void demo_array_stack(void) {
    printf("\n=== demo_array_stack ===\n");

    ArrayStack s;
    astack_init(&s);

    printf("入栈: 10, 20, 30\n");
    astack_push(&s, 10);
    astack_push(&s, 20);
    astack_push(&s, 30);
    astack_print(&s);

    int val;
    astack_peek(&s, &val);
    printf("栈顶: %d\n", val);

    astack_pop(&s, &val);
    printf("出栈: %d\n", val);
    astack_print(&s);

    printf("栈大小: %d\n", s.top + 1);
}

void demo_linked_stack(void) {
    printf("\n=== demo_linked_stack ===\n");

    LinkedStack s;
    lstack_init(&s);

    printf("入栈: 100, 200, 300\n");
    lstack_push(&s, 100);
    lstack_push(&s, 200);
    lstack_push(&s, 300);
    lstack_print(&s);

    int val;
    lstack_pop(&s, &val);
    printf("出栈: %d\n", val);
    lstack_print(&s);

    lstack_peek(&s, &val);
    printf("栈顶: %d\n", val);

    lstack_destroy(&s);
}

void demo_stack_bracket_check(void) {
    printf("\n=== demo_stack_bracket_check ===\n");
    printf("括号匹配检查 - 栈的经典应用\n\n");

    const char *tests[] = {
        "({[]})",
        "({[})",
        "((()))",
        "([)]",
        "{[()]}"
    };

    for (int t = 0; t < 5; t++) {
        const char *str = tests[t];
        ArrayStack s;
        astack_init(&s);
        int valid = 1;

        for (int i = 0; str[i] && valid; i++) {
            char c = str[i];
            if (c == '(' || c == '[' || c == '{') {
                astack_push(&s, c);
            } else if (c == ')' || c == ']' || c == '}') {
                int top;
                if (astack_pop(&s, &top) < 0) {
                    valid = 0;
                } else {
                    if ((c == ')' && top != '(') ||
                        (c == ']' && top != '[') ||
                        (c == '}' && top != '{')) {
                        valid = 0;
                    }
                }
            }
        }

        if (valid && astack_is_empty(&s)) {
            printf("\"%s\" -> 匹配\n", str);
        } else {
            printf("\"%s\" -> 不匹配\n", str);
        }
    }
}

void demo_stack_comparison(void) {
    printf("\n=== demo_stack_comparison ===\n");
    printf("数组栈 vs 链表栈:\n\n");
    printf("特性       数组栈          链表栈\n");
    printf("容量       固定            动态\n");
    printf("内存       连续            分散\n");
    printf("缓存友好   好              差\n");
    printf("扩容       需要realloc     无需\n");
    printf("额外开销   无              指针\n\n");
    printf("选择建议:\n");
    printf("  - 已知最大容量: 数组栈, 性能更好\n");
    printf("  - 容量不确定: 链表栈, 更灵活\n");
}

int main(void) {
    printf("栈的实现: 数组栈和链表栈\n");

    demo_array_stack();
    demo_linked_stack();
    demo_stack_bracket_check();
    demo_stack_comparison();

    printf("\n所有演示完成!\n");
    return 0;
}
