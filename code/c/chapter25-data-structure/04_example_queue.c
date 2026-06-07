/**
 * @file 04_example_queue.c
 * @brief 队列实现: 循环数组队列和链表队列
 * @description 对应文档: 26-链表与数据结构
 */

#include <stdio.h>
#include <stdlib.h>

#define CIRCULAR_QUEUE_SIZE 8

typedef struct {
    int data[CIRCULAR_QUEUE_SIZE];
    int front;
    int rear;
    int size;
} CircularQueue;

void cqueue_init(CircularQueue *q) {
    q->front = 0;
    q->rear = 0;
    q->size = 0;
}

int cqueue_is_empty(const CircularQueue *q) {
    return q->size == 0;
}

int cqueue_is_full(const CircularQueue *q) {
    return q->size == CIRCULAR_QUEUE_SIZE;
}

int cqueue_enqueue(CircularQueue *q, int value) {
    if (cqueue_is_full(q)) return -1;
    q->data[q->rear] = value;
    q->rear = (q->rear + 1) % CIRCULAR_QUEUE_SIZE;
    q->size++;
    return 0;
}

int cqueue_dequeue(CircularQueue *q, int *value) {
    if (cqueue_is_empty(q)) return -1;
    *value = q->data[q->front];
    q->front = (q->front + 1) % CIRCULAR_QUEUE_SIZE;
    q->size--;
    return 0;
}

int cqueue_peek(const CircularQueue *q, int *value) {
    if (cqueue_is_empty(q)) return -1;
    *value = q->data[q->front];
    return 0;
}

void cqueue_print(const CircularQueue *q) {
    printf("队列(前->后): [");
    for (int i = 0; i < q->size; i++) {
        int idx = (q->front + i) % CIRCULAR_QUEUE_SIZE;
        printf("%d", q->data[idx]);
        if (i < q->size - 1) printf(", ");
    }
    printf("] (front=%d, rear=%d, size=%d)\n", q->front, q->rear, q->size);
}

typedef struct QNode {
    int data;
    struct QNode *next;
} QNode;

typedef struct {
    QNode *front;
    QNode *rear;
    int size;
} LinkedQueue;

void lqueue_init(LinkedQueue *q) {
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}

int lqueue_is_empty(const LinkedQueue *q) {
    return q->front == NULL;
}

int lqueue_enqueue(LinkedQueue *q, int value) {
    QNode *n = (QNode *)malloc(sizeof(QNode));
    if (!n) return -1;
    n->data = value;
    n->next = NULL;
    if (!q->rear) {
        q->front = n;
        q->rear = n;
    } else {
        q->rear->next = n;
        q->rear = n;
    }
    q->size++;
    return 0;
}

int lqueue_dequeue(LinkedQueue *q, int *value) {
    if (lqueue_is_empty(q)) return -1;
    QNode *temp = q->front;
    *value = temp->data;
    q->front = temp->next;
    if (!q->front) q->rear = NULL;
    free(temp);
    q->size--;
    return 0;
}

int lqueue_peek(const LinkedQueue *q, int *value) {
    if (lqueue_is_empty(q)) return -1;
    *value = q->front->data;
    return 0;
}

void lqueue_destroy(LinkedQueue *q) {
    while (!lqueue_is_empty(q)) {
        int val;
        lqueue_dequeue(q, &val);
    }
}

void lqueue_print(const LinkedQueue *q) {
    printf("队列(前->后): [");
    QNode *cur = q->front;
    while (cur) {
        printf("%d", cur->data);
        if (cur->next) printf(", ");
        cur = cur->next;
    }
    printf("] (size=%d)\n", q->size);
}

void demo_circular_queue(void) {
    printf("\n=== demo_circular_queue ===\n");

    CircularQueue q;
    cqueue_init(&q);

    printf("入队: 1,2,3,4,5\n");
    for (int i = 1; i <= 5; i++) cqueue_enqueue(&q, i);
    cqueue_print(&q);

    int val;
    cqueue_dequeue(&q, &val);
    printf("出队: %d\n", val);
    cqueue_dequeue(&q, &val);
    printf("出队: %d\n", val);
    cqueue_print(&q);

    printf("继续入队: 6,7,8,9\n");
    for (int i = 6; i <= 9; i++) cqueue_enqueue(&q, i);
    cqueue_print(&q);

    printf("尝试入队10(队列已满): %s\n",
           cqueue_enqueue(&q, 10) == 0 ? "成功" : "失败");
}

void demo_linked_queue(void) {
    printf("\n=== demo_linked_queue ===\n");

    LinkedQueue q;
    lqueue_init(&q);

    printf("入队: 100,200,300\n");
    lqueue_enqueue(&q, 100);
    lqueue_enqueue(&q, 200);
    lqueue_enqueue(&q, 300);
    lqueue_print(&q);

    int val;
    lqueue_dequeue(&q, &val);
    printf("出队: %d\n", val);
    lqueue_print(&q);

    lqueue_peek(&q, &val);
    printf("队首: %d\n", val);

    lqueue_destroy(&q);
}

void demo_queue_comparison(void) {
    printf("\n=== demo_queue_comparison ===\n");
    printf("循环数组队列 vs 链表队列:\n\n");
    printf("特性         循环数组队列      链表队列\n");
    printf("容量         固定              动态\n");
    printf("内存         连续              分散\n");
    printf("缓存友好     好                差\n");
    printf("入队出队     O(1)              O(1)\n");
    printf("空间浪费     可能预留空间      每节点额外指针\n\n");

    printf("循环队列关键:\n");
    printf("  - 用size字段区分空和满, 避免浪费一个位置\n");
    printf("  - 取模运算实现循环: (index+1) %% CAPACITY\n");
    printf("  - 适合已知最大容量的场景(如缓冲区)\n");
}

int main(void) {
    printf("队列实现: 循环数组队列和链表队列\n");

    demo_circular_queue();
    demo_linked_queue();
    demo_queue_comparison();

    printf("\n所有演示完成!\n");
    return 0;
}
