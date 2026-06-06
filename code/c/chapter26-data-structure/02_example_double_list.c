/**
 * @file 02_example_double_list.c
 * @brief 双向链表示例: 插入、删除、双向遍历
 * @description 对应文档: 26-链表与数据结构
 */

#include <stdio.h>
#include <stdlib.h>

typedef struct DNode {
    int data;
    struct DNode *prev;
    struct DNode *next;
} DNode;

typedef struct {
    DNode *head;
    DNode *tail;
    int size;
} DList;

DNode *dnode_create(int data) {
    DNode *n = (DNode *)malloc(sizeof(DNode));
    if (n) {
        n->data = data;
        n->prev = NULL;
        n->next = NULL;
    }
    return n;
}

void dlist_init(DList *list) {
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void dlist_print_forward(const DList *list) {
    printf("正向: [");
    const DNode *cur = list->head;
    while (cur) {
        printf("%d", cur->data);
        if (cur->next) printf(" <-> ");
        cur = cur->next;
    }
    printf("] (size=%d)\n", list->size);
}

void dlist_print_backward(const DList *list) {
    printf("反向: [");
    const DNode *cur = list->tail;
    while (cur) {
        printf("%d", cur->data);
        if (cur->prev) printf(" <-> ");
        cur = cur->prev;
    }
    printf("]\n");
}

void dlist_push_back(DList *list, int data) {
    DNode *n = dnode_create(data);
    if (!n) return;
    if (!list->head) {
        list->head = n;
        list->tail = n;
    } else {
        n->prev = list->tail;
        list->tail->next = n;
        list->tail = n;
    }
    list->size++;
}

void dlist_push_front(DList *list, int data) {
    DNode *n = dnode_create(data);
    if (!n) return;
    if (!list->head) {
        list->head = n;
        list->tail = n;
    } else {
        n->next = list->head;
        list->head->prev = n;
        list->head = n;
    }
    list->size++;
}

void dlist_insert_at(DList *list, int index, int data) {
    if (index <= 0) { dlist_push_front(list, data); return; }
    if (index >= list->size) { dlist_push_back(list, data); return; }

    DNode *n = dnode_create(data);
    if (!n) return;

    DNode *cur = list->head;
    for (int i = 0; i < index; i++) cur = cur->next;

    n->prev = cur->prev;
    n->next = cur;
    cur->prev->next = n;
    cur->prev = n;
    list->size++;
}

void dlist_delete_by_value(DList *list, int value) {
    DNode *cur = list->head;
    while (cur && cur->data != value) cur = cur->next;
    if (!cur) return;

    if (cur->prev) cur->prev->next = cur->next;
    else list->head = cur->next;

    if (cur->next) cur->next->prev = cur->prev;
    else list->tail = cur->prev;

    free(cur);
    list->size--;
}

void dlist_destroy(DList *list) {
    DNode *cur = list->head;
    while (cur) {
        DNode *temp = cur;
        cur = cur->next;
        free(temp);
    }
    list->head = NULL;
    list->tail = NULL;
    list->size = 0;
}

void demo_dlist_basic(void) {
    printf("\n=== demo_dlist_basic ===\n");

    DList list;
    dlist_init(&list);

    dlist_push_back(&list, 10);
    dlist_push_back(&list, 20);
    dlist_push_back(&list, 30);
    printf("尾部插入 10,20,30:\n");
    dlist_print_forward(&list);
    dlist_print_backward(&list);

    dlist_push_front(&list, 5);
    printf("头部插入 5:\n");
    dlist_print_forward(&list);

    dlist_destroy(&list);
}

void demo_dlist_insert_delete(void) {
    printf("\n=== demo_dlist_insert_delete ===\n");

    DList list;
    dlist_init(&list);
    for (int i = 1; i <= 5; i++) dlist_push_back(&list, i * 10);
    printf("初始链表:\n");
    dlist_print_forward(&list);

    dlist_insert_at(&list, 2, 25);
    printf("在索引2插入25:\n");
    dlist_print_forward(&list);

    dlist_delete_by_value(&list, 30);
    printf("删除值30:\n");
    dlist_print_forward(&list);

    dlist_delete_by_value(&list, 10);
    printf("删除头部10:\n");
    dlist_print_forward(&list);
    dlist_print_backward(&list);

    dlist_destroy(&list);
}

void demo_dlist_vs_slist(void) {
    printf("\n=== demo_dlist_vs_slist ===\n");
    printf("单链表 vs 双向链表:\n\n");
    printf("操作         单链表        双向链表\n");
    printf("头部插入     O(1)         O(1)\n");
    printf("尾部插入     O(n)/O(1)*   O(1)\n");
    printf("头部删除     O(1)         O(1)\n");
    printf("尾部删除     O(n)         O(1)\n");
    printf("查找         O(n)         O(n)\n");
    printf("反向遍历     不支持        O(n)\n");
    printf("内存开销     1个指针       2个指针\n\n");
    printf("* 尾部插入O(1)需要维护tail指针\n\n");

    printf("选择建议:\n");
    printf("  - 只需正向遍历: 单链表, 内存开销小\n");
    printf("  - 需要双向遍历/尾部删除: 双向链表\n");
    printf("  - 频繁头尾操作: 双向链表 + head/tail指针\n");
}

int main(void) {
    printf("双向链表示例: 插入、删除、双向遍历\n");

    demo_dlist_basic();
    demo_dlist_insert_delete();
    demo_dlist_vs_slist();

    printf("\n所有演示完成!\n");
    return 0;
}
