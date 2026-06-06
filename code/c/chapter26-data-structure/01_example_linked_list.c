/**
 * @file 01_example_linked_list.c
 * @brief 单链表示例: 创建、插入、删除、遍历、查找
 * @description 对应文档: 26-链表与数据结构
 */

#include <stdio.h>
#include <stdlib.h>

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

void list_print(const Node *head) {
    const Node *cur = head;
    printf("[");
    while (cur) {
        printf("%d", cur->data);
        if (cur->next) printf(" -> ");
        cur = cur->next;
    }
    printf("]\n");
}

int list_length(const Node *head) {
    int len = 0;
    const Node *cur = head;
    while (cur) {
        len++;
        cur = cur->next;
    }
    return len;
}

Node *list_append(Node *head, int data) {
    Node *n = node_create(data);
    if (!n) return head;
    if (!head) return n;
    Node *cur = head;
    while (cur->next) cur = cur->next;
    cur->next = n;
    return head;
}

Node *list_prepend(Node *head, int data) {
    Node *n = node_create(data);
    if (!n) return head;
    n->next = head;
    return n;
}

Node *list_insert_at(Node *head, int index, int data) {
    if (index <= 0) return list_prepend(head, data);
    if (index >= list_length(head)) return list_append(head, data);

    Node *n = node_create(data);
    if (!n) return head;

    Node *cur = head;
    for (int i = 0; i < index - 1; i++) cur = cur->next;
    n->next = cur->next;
    cur->next = n;
    return head;
}

Node *list_delete_by_value(Node *head, int value) {
    if (!head) return NULL;

    if (head->data == value) {
        Node *temp = head;
        head = head->next;
        free(temp);
        return head;
    }

    Node *cur = head;
    while (cur->next && cur->next->data != value) cur = cur->next;

    if (cur->next) {
        Node *temp = cur->next;
        cur->next = temp->next;
        free(temp);
    }
    return head;
}

Node *list_search(const Node *head, int value) {
    const Node *cur = head;
    while (cur) {
        if (cur->data == value) return (Node *)cur;
        cur = cur->next;
    }
    return NULL;
}

void list_destroy(Node *head) {
    Node *cur = head;
    while (cur) {
        Node *temp = cur;
        cur = cur->next;
        free(temp);
    }
}

Node *list_reverse(Node *head) {
    Node *prev = NULL;
    Node *cur = head;
    while (cur) {
        Node *next = cur->next;
        cur->next = prev;
        prev = cur;
        cur = next;
    }
    return prev;
}

void demo_list_basic(void) {
    printf("\n=== demo_list_basic ===\n");

    Node *head = NULL;
    printf("创建空链表: ");
    list_print(head);

    head = list_append(head, 10);
    head = list_append(head, 20);
    head = list_append(head, 30);
    printf("追加 10,20,30: ");
    list_print(head);

    head = list_prepend(head, 5);
    printf("头部插入 5: ");
    list_print(head);

    printf("链表长度: %d\n", list_length(head));

    list_destroy(head);
}

void demo_list_insert_delete(void) {
    printf("\n=== demo_list_insert_delete ===\n");

    Node *head = NULL;
    for (int i = 1; i <= 5; i++) head = list_append(head, i * 10);
    printf("初始链表: ");
    list_print(head);

    head = list_insert_at(head, 2, 25);
    printf("在索引2插入25: ");
    list_print(head);

    head = list_delete_by_value(head, 30);
    printf("删除值30: ");
    list_print(head);

    head = list_delete_by_value(head, 10);
    printf("删除头部10: ");
    list_print(head);

    list_destroy(head);
}

void demo_list_search_reverse(void) {
    printf("\n=== demo_list_search_reverse ===\n");

    Node *head = NULL;
    for (int i = 1; i <= 5; i++) head = list_append(head, i * 10);
    printf("链表: ");
    list_print(head);

    Node *found = list_search(head, 30);
    printf("查找30: %s\n", found ? "找到" : "未找到");

    found = list_search(head, 99);
    printf("查找99: %s\n", found ? "找到" : "未找到");

    head = list_reverse(head);
    printf("反转后: ");
    list_print(head);

    list_destroy(head);
}

int main(void) {
    printf("单链表示例: 创建、插入、删除、遍历、查找\n");

    demo_list_basic();
    demo_list_insert_delete();
    demo_list_search_reverse();

    printf("\n所有演示完成!\n");
    return 0;
}
