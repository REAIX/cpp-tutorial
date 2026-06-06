/**
 * @file 02_deep_dive_ds_applications.c
 * @brief 数据结构实战应用: LRU缓存、表达式求值、BFS/DFS
 * @description 对应文档: 26-链表与数据结构
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define LRU_CAPACITY 4

typedef struct LRUNode {
    int key;
    int value;
    struct LRUNode *prev;
    struct LRUNode *next;
} LRUNode;

typedef struct {
    LRUNode *head;
    LRUNode *tail;
    LRUNode *entries[256];
    int size;
    int capacity;
} LRUCache;

void lru_remove_node(LRUCache *cache, LRUNode *node) {
    if (node->prev) node->prev->next = node->next;
    else cache->head = node->next;
    if (node->next) node->next->prev = node->prev;
    else cache->tail = node->prev;
    node->prev = NULL;
    node->next = NULL;
}

void lru_add_to_front(LRUCache *cache, LRUNode *node) {
    node->next = cache->head;
    node->prev = NULL;
    if (cache->head) cache->head->prev = node;
    cache->head = node;
    if (!cache->tail) cache->tail = node;
}

void lru_move_to_front(LRUCache *cache, LRUNode *node) {
    lru_remove_node(cache, node);
    lru_add_to_front(cache, node);
}

LRUCache *lru_create(int capacity) {
    LRUCache *cache = (LRUCache *)calloc(1, sizeof(LRUCache));
    cache->capacity = capacity;
    return cache;
}

int lru_get(LRUCache *cache, int key) {
    if (key < 0 || key >= 256 || !cache->entries[key]) return -1;
    LRUNode *node = cache->entries[key];
    lru_move_to_front(cache, node);
    return node->value;
}

void lru_put(LRUCache *cache, int key, int value) {
    if (key < 0 || key >= 256) return;

    if (cache->entries[key]) {
        cache->entries[key]->value = value;
        lru_move_to_front(cache, cache->entries[key]);
        return;
    }

    LRUNode *node = (LRUNode *)malloc(sizeof(LRUNode));
    node->key = key;
    node->value = value;
    node->prev = NULL;
    node->next = NULL;

    if (cache->size >= cache->capacity) {
        LRUNode *lru = cache->tail;
        lru_remove_node(cache, lru);
        cache->entries[lru->key] = NULL;
        free(lru);
        cache->size--;
    }

    lru_add_to_front(cache, node);
    cache->entries[key] = node;
    cache->size++;
}

void lru_print(const LRUCache *cache) {
    printf("LRU(最近->最久): [");
    LRUNode *cur = cache->head;
    while (cur) {
        printf("%d:%d", cur->key, cur->value);
        if (cur->next) printf(" -> ");
        cur = cur->next;
    }
    printf("]\n");
}

void lru_destroy(LRUCache *cache) {
    LRUNode *cur = cache->head;
    while (cur) {
        LRUNode *temp = cur;
        cur = cur->next;
        free(temp);
    }
    free(cache);
}

void demo_lru_cache(void) {
    printf("\n=== demo_lru_cache ===\n");
    printf("LRU缓存: 哈希表 + 双向链表, O(1)读写\n\n");

    LRUCache *cache = lru_create(LRU_CAPACITY);

    lru_put(cache, 1, 100);
    lru_put(cache, 2, 200);
    lru_put(cache, 3, 300);
    lru_put(cache, 4, 400);
    lru_print(cache);

    printf("\n访问key=2: %d\n", lru_get(cache, 2));
    lru_print(cache);

    printf("\n插入key=5 (淘汰最久未用):\n");
    lru_put(cache, 5, 500);
    lru_print(cache);

    printf("\n访问key=1: %d (已被淘汰)\n", lru_get(cache, 1));
    lru_print(cache);

    lru_destroy(cache);

    printf("\nLRU缓存举一反三:\n");
    printf("  1. 哈希表: O(1)查找, 双向链表: O(1)调整顺序\n");
    printf("  2. 变体: LFU(最不经常使用), FIFO, ARC\n");
    printf("  3. 应用: CPU缓存, 页面置换, 浏览器缓存\n");
}

#define EXPR_MAX 256

typedef struct {
    double data[EXPR_MAX];
    int top;
} NumStack;

typedef struct {
    char data[EXPR_MAX];
    int top;
} OpStack;

void num_push(NumStack *s, double val) { s->data[++s->top] = val; }
double num_pop(NumStack *s) { return s->data[s->top--]; }

void op_push(OpStack *s, char val) { s->data[++s->top] = val; }
char op_pop(OpStack *s) { return s->data[s->top--]; }
char op_peek(OpStack *s) { return s->data[s->top]; }

int precedence(char op) {
    if (op == '+' || op == '-') return 1;
    if (op == '*' || op == '/') return 2;
    return 0;
}

double apply_op(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/': return b != 0 ? a / b : 0;
    }
    return 0;
}

double evaluate_expression(const char *expr) {
    NumStack nums = {.top = -1};
    OpStack ops = {.top = -1};

    for (int i = 0; expr[i]; i++) {
        if (isspace((unsigned char)expr[i])) continue;

        if (isdigit((unsigned char)expr[i]) || expr[i] == '.') {
            double val = 0;
            int decimal = 0;
            double frac = 0.1;
            while (expr[i] && (isdigit((unsigned char)expr[i]) || expr[i] == '.')) {
                if (expr[i] == '.') {
                    decimal = 1;
                    i++;
                    continue;
                }
                if (decimal) {
                    val += (expr[i] - '0') * frac;
                    frac *= 0.1;
                } else {
                    val = val * 10 + (expr[i] - '0');
                }
                i++;
            }
            i--;
            num_push(&nums, val);
        } else if (expr[i] == '(') {
            op_push(&ops, expr[i]);
        } else if (expr[i] == ')') {
            while (ops.top >= 0 && op_peek(&ops) != '(') {
                double b = num_pop(&nums);
                double a = num_pop(&nums);
                char op = op_pop(&ops);
                num_push(&nums, apply_op(a, b, op));
            }
            if (ops.top >= 0) op_pop(&ops);
        } else {
            while (ops.top >= 0 && precedence(op_peek(&ops)) >= precedence(expr[i])) {
                double b = num_pop(&nums);
                double a = num_pop(&nums);
                char op = op_pop(&ops);
                num_push(&nums, apply_op(a, b, op));
            }
            op_push(&ops, expr[i]);
        }
    }

    while (ops.top >= 0) {
        double b = num_pop(&nums);
        double a = num_pop(&nums);
        char op = op_pop(&ops);
        num_push(&nums, apply_op(a, b, op));
    }

    return num_pop(&nums);
}

void demo_expression_eval(void) {
    printf("\n=== demo_expression_eval ===\n");
    printf("表达式求值: 双栈法(Dijkstra的Shunting-yard算法)\n\n");

    const char *exprs[] = {
        "3 + 4 * 2",
        "(3 + 4) * 2",
        "10 + 20 / 5 - 3",
        "2 * (3 + 4) - 10 / 2"
    };
    double expected[] = {11, 14, 11, 9};

    for (int i = 0; i < 4; i++) {
        double result = evaluate_expression(exprs[i]);
        printf("\"%s\" = %.2f (期望: %.2f) %s\n",
               exprs[i], result, expected[i],
               (result - expected[i] < 0.001 && result - expected[i] > -0.001) ? "✓" : "✗");
    }

    printf("\n双栈法原理:\n");
    printf("  1. 数字栈: 存储操作数\n");
    printf("  2. 运算符栈: 按优先级处理\n");
    printf("  3. 遇到低优先级运算符: 先计算栈顶高优先级运算\n");
    printf("  4. 括号: 递归处理子表达式\n");
}

#define TREE_SIZE 7

typedef struct {
    int data[TREE_SIZE];
    int left[TREE_SIZE];
    int right[TREE_SIZE];
} StaticTree;

void demo_bfs_dfs(void) {
    printf("\n=== demo_bfs_dfs ===\n");
    printf("BFS(广度优先)用队列, DFS(深度优先)用栈\n\n");

    StaticTree tree = {
        .data  = {1, 2, 3, 4, 5, 6, 7},
        .left  = {1, 3, 5, -1, -1, -1, -1},
        .right = {2, 4, 6, -1, -1, -1, -1}
    };

    printf("    1\n");
    printf("   / \\\n");
    printf("  2   3\n");
    printf(" / \\ / \\\n");
    printf("4  5 6  7\n\n");

    printf("BFS(层序遍历): ");
    int queue[TREE_SIZE];
    int qfront = 0, qrear = 0;
    queue[qrear++] = 0;
    while (qfront < qrear) {
        int node = queue[qfront++];
        printf("%d ", tree.data[node]);
        if (tree.left[node] >= 0) queue[qrear++] = tree.left[node];
        if (tree.right[node] >= 0) queue[qrear++] = tree.right[node];
    }
    printf("\n");

    printf("DFS(前序,迭代): ");
    int stack[TREE_SIZE];
    int stop = -1;
    stack[++stop] = 0;
    while (stop >= 0) {
        int node = stack[stop--];
        printf("%d ", tree.data[node]);
        if (tree.right[node] >= 0) stack[++stop] = tree.right[node];
        if (tree.left[node] >= 0) stack[++stop] = tree.left[node];
    }
    printf("\n");

    printf("\nBFS vs DFS:\n");
    printf("  BFS: 队列, 层序遍历, 最短路径(无权图)\n");
    printf("  DFS: 栈/递归, 深度探索, 拓扑排序, 连通分量\n");
    printf("  空间: BFS最坏O(n), DFS最坏O(h) h=树高\n");
}

int main(void) {
    printf("数据结构实战应用: LRU缓存、表达式求值、BFS/DFS\n");

    demo_lru_cache();
    demo_expression_eval();
    demo_bfs_dfs();

    printf("\n所有演示完成!\n");
    return 0;
}
