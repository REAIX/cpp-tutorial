/**
 * @file 02_deep_dive_search_patterns.c
 * @brief 查找模式深入: 插值查找、哈希函数设计、冲突解决、负载因子
 * @description 对应文档: 27-排序与查找算法
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define HASH_SIZE 16

int interpolation_search(const int *arr, int n, int target) {
    int low = 0, high = n - 1;

    while (low <= high && target >= arr[low] && target <= arr[high]) {
        if (arr[high] == arr[low]) {
            if (arr[low] == target) return low;
            return -1;
        }

        int pos = low + ((double)(target - arr[low]) / (arr[high] - arr[low])) * (high - low);

        if (pos < low) pos = low;
        if (pos > high) pos = high;

        if (arr[pos] == target) return pos;
        if (arr[pos] < target) low = pos + 1;
        else high = pos - 1;
    }
    return -1;
}

void demo_interpolation_search(void) {
    printf("\n=== demo_interpolation_search ===\n");
    printf("插值查找: 根据数据分布估计位置, 均匀分布时O(log log n)\n\n");

    int arr[] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int n = sizeof(arr) / sizeof(arr[0]);

    printf("均匀分布数组: ");
    for (int i = 0; i < n; i++) printf("%d ", arr[i]);
    printf("\n\n");

    int targets[] = {30, 70, 10, 100, 55};
    for (int i = 0; i < 5; i++) {
        int idx = interpolation_search(arr, n, targets[i]);
        printf("查找 %d: %s\n", targets[i],
               idx >= 0 ? "找到" : "未找到");
    }

    printf("\n插值查找 vs 二分查找:\n");
    printf("  二分查找: 始终从中间开始, O(log n)\n");
    printf("  插值查找: 按比例估计位置, 均匀分布O(log log n)\n");
    printf("  插值查找最坏: O(n), 如高度不均匀数据\n");
    printf("  适用: 数据均匀分布, 如电话号码、学号\n");
}

unsigned int hash_djb2(const char *key) {
    unsigned int hash = 5381;
    while (*key) {
        hash = ((hash << 5) + hash) + (unsigned char)(*key);
        key++;
    }
    return hash;
}

unsigned int hash_fnv1a(const char *key) {
    unsigned int hash = 2166136261u;
    while (*key) {
        hash ^= (unsigned char)(*key);
        hash *= 16777619u;
        key++;
    }
    return hash;
}

unsigned int hash_simple(const char *key) {
    unsigned int hash = 0;
    while (*key) {
        hash += (unsigned char)(*key);
        key++;
    }
    return hash;
}

void demo_hash_function_design(void) {
    printf("\n=== demo_hash_function_design ===\n");
    printf("哈希函数设计原则:\n");
    printf("  1. 确定性: 相同输入->相同输出\n");
    printf("  2. 均匀性: 输出均匀分布在值域\n");
    printf("  3. 雪崩效应: 输入微变->输出大变\n");
    printf("  4. 高效性: 计算速度快\n\n");

    const char *keys[] = {"abc", "abd", "hello", "world", "test"};
    printf("不同哈希函数对比:\n");
    printf("%-10s %-12s %-12s %-12s\n", "Key", "Simple", "DJB2", "FNV-1a");
    for (int i = 0; i < 5; i++) {
        printf("%-10s %-12u %-12u %-12u\n",
               keys[i],
               hash_simple(keys[i]) % HASH_SIZE,
               hash_djb2(keys[i]) % HASH_SIZE,
               hash_fnv1a(keys[i]) % HASH_SIZE);
    }

    printf("\nSimple哈希: 只累加ASCII值, 分布差, 'abc'和'cba'相同\n");
    printf("DJB2: 乘法+加法, 分布好, 简单高效\n");
    printf("FNV-1a: 异或+乘法, 分布极佳, 推荐使用\n");
}

typedef struct ChainNode {
    int key;
    int value;
    struct ChainNode *next;
} ChainNode;

typedef struct {
    ChainNode *buckets[HASH_SIZE];
    int size;
} ChainHashTable;

void chain_ht_init(ChainHashTable *ht) {
    memset(ht->buckets, 0, sizeof(ht->buckets));
    ht->size = 0;
}

void chain_ht_insert(ChainHashTable *ht, int key, int value) {
    unsigned int idx = (unsigned int)key % HASH_SIZE;
    ChainNode *cur = ht->buckets[idx];
    while (cur) {
        if (cur->key == key) { cur->value = value; return; }
        cur = cur->next;
    }
    ChainNode *n = (ChainNode *)malloc(sizeof(ChainNode));
    n->key = key;
    n->value = value;
    n->next = ht->buckets[idx];
    ht->buckets[idx] = n;
    ht->size++;
}

void chain_ht_destroy(ChainHashTable *ht) {
    for (int i = 0; i < HASH_SIZE; i++) {
        ChainNode *cur = ht->buckets[i];
        while (cur) {
            ChainNode *temp = cur;
            cur = cur->next;
            free(temp);
        }
    }
    ht->size = 0;
}

typedef struct {
    int key;
    int value;
    int occupied;
} OpenAddrEntry;

typedef struct {
    OpenAddrEntry entries[HASH_SIZE];
    int size;
} OpenAddrHashTable;

void open_ht_init(OpenAddrHashTable *ht) {
    for (int i = 0; i < HASH_SIZE; i++) {
        ht->entries[i].occupied = 0;
    }
    ht->size = 0;
}

int open_ht_insert(OpenAddrHashTable *ht, int key, int value) {
    if (ht->size >= HASH_SIZE) return -1;
    unsigned int idx = (unsigned int)key % HASH_SIZE;
    for (int i = 0; i < HASH_SIZE; i++) {
        unsigned int pos = (idx + i) % HASH_SIZE;
        if (!ht->entries[pos].occupied) {
            ht->entries[pos].key = key;
            ht->entries[pos].value = value;
            ht->entries[pos].occupied = 1;
            ht->size++;
            return 0;
        }
        if (ht->entries[pos].key == key) {
            ht->entries[pos].value = value;
            return 0;
        }
    }
    return -1;
}

int open_ht_get(OpenAddrHashTable *ht, int key, int *value) {
    unsigned int idx = (unsigned int)key % HASH_SIZE;
    for (int i = 0; i < HASH_SIZE; i++) {
        unsigned int pos = (idx + i) % HASH_SIZE;
        if (!ht->entries[pos].occupied) return -1;
        if (ht->entries[pos].key == key) {
            *value = ht->entries[pos].value;
            return 0;
        }
    }
    return -1;
}

void demo_collision_resolution(void) {
    printf("\n=== demo_collision_resolution ===\n");
    printf("冲突解决方法:\n\n");

    printf("--- 链地址法(Separate Chaining) ---\n");
    ChainHashTable cht;
    chain_ht_init(&cht);
    for (int i = 0; i < 20; i++) chain_ht_insert(&cht, i * 7, i * 100);

    printf("插入20个元素(键=i*7):\n");
    int chain_lens[HASH_SIZE] = {0};
    for (int i = 0; i < HASH_SIZE; i++) {
        ChainNode *cur = cht.buckets[i];
        while (cur) { chain_lens[i]++; cur = cur->next; }
    }
    int max_chain = 0;
    for (int i = 0; i < HASH_SIZE; i++) {
        if (chain_lens[i] > max_chain) max_chain = chain_lens[i];
    }
    printf("最大链长: %d\n", max_chain);
    chain_ht_destroy(&cht);

    printf("\n--- 开放地址法(Open Addressing, 线性探测) ---\n");
    OpenAddrHashTable oht;
    open_ht_init(&oht);
    for (int i = 0; i < 12; i++) open_ht_insert(&oht, i * 7, i * 100);

    printf("插入12个元素(键=i*7):\n");
    for (int i = 0; i < HASH_SIZE; i++) {
        if (oht.entries[i].occupied) {
            printf("  [%2d]: key=%d, value=%d\n", i, oht.entries[i].key, oht.entries[i].value);
        }
    }

    printf("\n链地址法 vs 开放地址法:\n");
    printf("特性         链地址法          开放地址法\n");
    printf("额外空间     每节点指针        无额外指针\n");
    printf("删除         简单              需要标记(墓碑)\n");
    printf("缓存         差(链表分散)      好(数组连续)\n");
    printf("负载因子     可>1              必须<1\n");
    printf("聚集         无                一次/二次聚集\n");
}

void demo_load_factor(void) {
    printf("\n=== demo_load_factor ===\n");
    printf("负载因子 = 已存储元素数 / 哈希表容量\n\n");

    printf("负载因子对性能的影响:\n");
    printf("  链地址法:\n");
    printf("    负载因子<0.75: 查找O(1), 性能好\n");
    printf("    负载因子>1:   链过长, 退化为O(n)\n");
    printf("    建议: 负载因子>0.75时扩容\n\n");

    printf("  开放地址法:\n");
    printf("    负载因子<0.5: 查找O(1), 性能好\n");
    printf("    负载因子>0.7: 聚集严重, 性能急剧下降\n");
    printf("    建议: 负载因子>0.7时扩容\n\n");

    printf("扩容策略(Rehash):\n");
    printf("  1. 分配2倍大小的新数组\n");
    printf("  2. 重新计算所有元素的哈希位置\n");
    printf("  3. 释放旧数组\n");
    printf("  4. 扩容代价O(n), 但均摊后每次插入O(1)\n\n");

    printf("举一反三:\n");
    printf("  - Java HashMap: 链地址法, 负载因子0.75, 链长>8转红黑树\n");
    printf("  - Python dict: 开放地址法(二次探测), 负载因子2/3时扩容\n");
    printf("  - C++ unordered_map: 链地址法, 负载因子1.0时扩容\n");
}

int main(void) {
    printf("查找模式深入: 插值查找、哈希函数设计、冲突解决、负载因子\n");

    demo_interpolation_search();
    demo_hash_function_design();
    demo_collision_resolution();
    demo_load_factor();

    printf("\n所有演示完成!\n");
    return 0;
}
