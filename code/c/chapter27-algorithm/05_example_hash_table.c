/**
 * @file 05_example_hash_table.c
 * @brief 简单哈希表: 链地址法解决冲突
 * @description 对应文档: 27-排序与查找算法
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_TABLE_SIZE 16

typedef struct HashNode {
    char *key;
    int value;
    struct HashNode *next;
} HashNode;

typedef struct {
    HashNode *buckets[HASH_TABLE_SIZE];
    int size;
    int capacity;
} HashTable;

unsigned int hash_function(const char *key) {
    unsigned int hash = 5381;
    while (*key) {
        hash = ((hash << 5) + hash) + (unsigned char)(*key);
        key++;
    }
    return hash % HASH_TABLE_SIZE;
}

HashNode *hash_node_create(const char *key, int value) {
    HashNode *n = (HashNode *)malloc(sizeof(HashNode));
    if (!n) return NULL;
    n->key = strdup(key);
    if (!n->key) {
        free(n);
        return NULL;
    }
    n->value = value;
    n->next = NULL;
    return n;
}

void hash_node_destroy(HashNode *n) {
    free(n->key);
    free(n);
}

void ht_init(HashTable *ht) {
    memset(ht->buckets, 0, sizeof(ht->buckets));
    ht->size = 0;
    ht->capacity = HASH_TABLE_SIZE;
}

int ht_insert(HashTable *ht, const char *key, int value) {
    unsigned int idx = hash_function(key);
    HashNode *cur = ht->buckets[idx];

    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            cur->value = value;
            return 0;
        }
        cur = cur->next;
    }

    HashNode *n = hash_node_create(key, value);
    if (!n) return -1;

    n->next = ht->buckets[idx];
    ht->buckets[idx] = n;
    ht->size++;
    return 0;
}

int ht_get(const HashTable *ht, const char *key, int *value) {
    unsigned int idx = hash_function(key);
    HashNode *cur = ht->buckets[idx];

    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            *value = cur->value;
            return 0;
        }
        cur = cur->next;
    }
    return -1;
}

int ht_remove(HashTable *ht, const char *key) {
    unsigned int idx = hash_function(key);
    HashNode *cur = ht->buckets[idx];
    HashNode *prev = NULL;

    while (cur) {
        if (strcmp(cur->key, key) == 0) {
            if (prev) prev->next = cur->next;
            else ht->buckets[idx] = cur->next;
            hash_node_destroy(cur);
            ht->size--;
            return 0;
        }
        prev = cur;
        cur = cur->next;
    }
    return -1;
}

void ht_destroy(HashTable *ht) {
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        HashNode *cur = ht->buckets[i];
        while (cur) {
            HashNode *temp = cur;
            cur = cur->next;
            hash_node_destroy(temp);
        }
        ht->buckets[i] = NULL;
    }
    ht->size = 0;
}

void ht_print(const HashTable *ht) {
    printf("哈希表(大小=%d, 容量=%d):\n", ht->size, ht->capacity);
    for (int i = 0; i < HASH_TABLE_SIZE; i++) {
        if (ht->buckets[i]) {
            printf("  [%2d]: ", i);
            HashNode *cur = ht->buckets[i];
            while (cur) {
                printf("(%s:%d)", cur->key, cur->value);
                if (cur->next) printf(" -> ");
                cur = cur->next;
            }
            printf("\n");
        }
    }
}

void demo_hash_table_basic(void) {
    printf("\n=== demo_hash_table_basic ===\n");

    HashTable ht;
    ht_init(&ht);

    ht_insert(&ht, "apple", 5);
    ht_insert(&ht, "banana", 3);
    ht_insert(&ht, "cherry", 8);
    ht_insert(&ht, "date", 2);
    ht_insert(&ht, "elderberry", 7);

    ht_print(&ht);

    int val;
    if (ht_get(&ht, "cherry", &val) == 0) {
        printf("\n查找 cherry: %d\n", val);
    }
    if (ht_get(&ht, "fig", &val) != 0) {
        printf("查找 fig: 未找到\n");
    }
}

void demo_hash_table_update_delete(void) {
    printf("\n=== demo_hash_table_update_delete ===\n");

    HashTable ht;
    ht_init(&ht);

    ht_insert(&ht, "x", 10);
    ht_insert(&ht, "y", 20);
    ht_insert(&ht, "z", 30);
    printf("初始:\n");
    ht_print(&ht);

    ht_insert(&ht, "y", 99);
    printf("\n更新 y=99:\n");
    ht_print(&ht);

    ht_remove(&ht, "x");
    printf("\n删除 x:\n");
    ht_print(&ht);

    ht_destroy(&ht);
}

void demo_hash_function(void) {
    printf("\n=== demo_hash_function ===\n");
    printf("DJB2哈希函数演示:\n\n");

    const char *keys[] = {"apple", "banana", "cherry", "date", "fig", "grape", "kiwi", "lemon"};
    for (int i = 0; i < 8; i++) {
        printf("  hash(\"%s\") = %u -> 桶[%u]\n",
               keys[i], hash_function(keys[i]), hash_function(keys[i]));
    }

    printf("\n哈希函数要求:\n");
    printf("  1. 确定性: 相同输入总是相同输出\n");
    printf("  2. 均匀性: 尽量均匀分布到各桶\n");
    printf("  3. 高效性: 计算速度快\n");
    printf("  4. 雪崩效应: 输入微小变化, 输出大不同\n");

    printf("\n常见哈希函数:\n");
    printf("  DJB2: 简单高效, 适合字符串\n");
    printf("  FNV-1a: 分布均匀, 速度快\n");
    printf("  MurmurHash: 非加密, 性能极佳\n");
    printf("  SHA-256: 加密哈希, 安全但慢\n");
}

int main(void) {
    printf("简单哈希表: 链地址法解决冲突\n");

    demo_hash_table_basic();
    demo_hash_table_update_delete();
    demo_hash_function();

    printf("\n所有演示完成!\n");
    return 0;
}
