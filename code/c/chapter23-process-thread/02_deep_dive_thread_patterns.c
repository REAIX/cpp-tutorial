/**
 * @file 02_deep_dive_thread_patterns.c
 * @brief 线程模式深入: 线程池、生产者消费者、读写者问题
 * @description 对应文档: 24-进程与线程
 *  @note C++ 中可使用 std::thread / std::mutex 等更高级的抽象, 参见 C++ 章节 29-34
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#define QUEUE_SIZE 16

typedef struct {
    int data[QUEUE_SIZE];
    int head;
    int tail;
    int count;
} CircularQueue;

void queue_init(CircularQueue *q) {
    q->head = 0;
    q->tail = 0;
    q->count = 0;
}

int queue_is_empty(CircularQueue *q) {
    return q->count == 0;
}

int queue_is_full(CircularQueue *q) {
    return q->count == QUEUE_SIZE;
}

void queue_push(CircularQueue *q, int value) {
    q->data[q->tail] = value;
    q->tail = (q->tail + 1) % QUEUE_SIZE;
    q->count++;
}

int queue_pop(CircularQueue *q) {
    int value = q->data[q->head];
    q->head = (q->head + 1) % QUEUE_SIZE;
    q->count--;
    return value;
}

typedef struct {
    CircularQueue queue;
#ifdef _WIN32
    CRITICAL_SECTION mutex;
    CONDITION_VARIABLE not_empty;
    CONDITION_VARIABLE not_full;
#else
    pthread_mutex_t mutex;
    pthread_cond_t not_empty;
    pthread_cond_t not_full;
#endif
    int done;
} ProducerConsumerCtx;

static ProducerConsumerCtx pc_ctx;

void pc_init(ProducerConsumerCtx *ctx) {
    queue_init(&ctx->queue);
    ctx->done = 0;
#ifdef _WIN32
    InitializeCriticalSection(&ctx->mutex);
    InitializeConditionVariable(&ctx->not_empty);
    InitializeConditionVariable(&ctx->not_full);
#else
    pthread_mutex_init(&ctx->mutex, NULL);
    pthread_cond_init(&ctx->not_empty, NULL);
    pthread_cond_init(&ctx->not_full, NULL);
#endif
}

void pc_destroy(ProducerConsumerCtx *ctx) {
#ifdef _WIN32
    DeleteCriticalSection(&ctx->mutex);
#else
    pthread_mutex_destroy(&ctx->mutex);
    pthread_cond_destroy(&ctx->not_empty);
    pthread_cond_destroy(&ctx->not_full);
#endif
}

#ifdef _WIN32
DWORD WINAPI producer_func(LPVOID arg) {
#else
void *producer_func(void *arg) {
#endif
    int id = *(int *)arg;
    for (int i = 0; i < 20; i++) {
#ifdef _WIN32
        EnterCriticalSection(&pc_ctx.mutex);
        while (queue_is_full(&pc_ctx.queue)) {
            SleepConditionVariableCS(&pc_ctx.not_full, &pc_ctx.mutex, INFINITE);
        }
        int item = id * 100 + i;
        queue_push(&pc_ctx.queue, item);
        printf("[生产者 %d] 生产: %d (队列大小: %d)\n", id, item, pc_ctx.queue.count);
        WakeConditionVariable(&pc_ctx.not_empty);
        LeaveCriticalSection(&pc_ctx.mutex);
#else
        pthread_mutex_lock(&pc_ctx.mutex);
        while (queue_is_full(&pc_ctx.queue)) {
            pthread_cond_wait(&pc_ctx.not_full, &pc_ctx.mutex);
        }
        int item = id * 100 + i;
        queue_push(&pc_ctx.queue, item);
        printf("[生产者 %d] 生产: %d (队列大小: %d)\n", id, item, pc_ctx.queue.count);
        pthread_cond_signal(&pc_ctx.not_empty);
        pthread_mutex_unlock(&pc_ctx.mutex);
#endif

#ifdef _WIN32
        Sleep(10);
#else
        usleep(10000);
#endif
    }
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

#ifdef _WIN32
DWORD WINAPI consumer_func(LPVOID arg) {
#else
void *consumer_func(void *arg) {
#endif
    int id = *(int *)arg;
    int consumed = 0;
    while (consumed < 20) {
#ifdef _WIN32
        EnterCriticalSection(&pc_ctx.mutex);
        while (queue_is_empty(&pc_ctx.queue) && !pc_ctx.done) {
            SleepConditionVariableCS(&pc_ctx.not_empty, &pc_ctx.mutex, 1000);
        }
        if (queue_is_empty(&pc_ctx.queue) && pc_ctx.done) {
            LeaveCriticalSection(&pc_ctx.mutex);
            break;
        }
        int item = queue_pop(&pc_ctx.queue);
        printf("[消费者 %d] 消费: %d (队列大小: %d)\n", id, item, pc_ctx.queue.count);
        WakeConditionVariable(&pc_ctx.not_full);
        LeaveCriticalSection(&pc_ctx.mutex);
#else
        pthread_mutex_lock(&pc_ctx.mutex);
        while (queue_is_empty(&pc_ctx.queue) && !pc_ctx.done) {
            pthread_cond_wait(&pc_ctx.not_empty, &pc_ctx.mutex);
        }
        if (queue_is_empty(&pc_ctx.queue) && pc_ctx.done) {
            pthread_mutex_unlock(&pc_ctx.mutex);
            break;
        }
        int item = queue_pop(&pc_ctx.queue);
        printf("[消费者 %d] 消费: %d (队列大小: %d)\n", id, item, pc_ctx.queue.count);
        pthread_cond_signal(&pc_ctx.not_full);
        pthread_mutex_unlock(&pc_ctx.mutex);
#endif
        consumed++;
    }
    printf("[消费者 %d] 共消费 %d 个项目\n", id, consumed);
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

void demo_producer_consumer(void) {
    printf("\n=== demo_producer_consumer ===\n");
    printf("生产者-消费者模式: 互斥锁 + 条件变量实现线程安全的有界缓冲区\n\n");

    pc_init(&pc_ctx);

    int p1_id = 1, p2_id = 2, c1_id = 1, c2_id = 2;

#ifdef _WIN32
    HANDLE threads[4];
    threads[0] = CreateThread(NULL, 0, producer_func, &p1_id, 0, NULL);
    threads[1] = CreateThread(NULL, 0, producer_func, &p2_id, 0, NULL);
    threads[2] = CreateThread(NULL, 0, consumer_func, &c1_id, 0, NULL);
    threads[3] = CreateThread(NULL, 0, consumer_func, &c2_id, 0, NULL);

    WaitForMultipleObjects(2, threads, TRUE, INFINITE);
    pc_ctx.done = 1;
    WakeAllConditionVariable(&pc_ctx.not_empty);
    WaitForMultipleObjects(2, threads + 2, TRUE, INFINITE);
    for (int i = 0; i < 4; i++) CloseHandle(threads[i]);
#else
    pthread_t threads[4];
    pthread_create(&threads[0], NULL, producer_func, &p1_id);
    pthread_create(&threads[1], NULL, producer_func, &p2_id);
    pthread_create(&threads[2], NULL, consumer_func, &c1_id);
    pthread_create(&threads[3], NULL, consumer_func, &c2_id);

    pthread_join(threads[0], NULL);
    pthread_join(threads[1], NULL);
    pc_ctx.done = 1;
    pthread_cond_broadcast(&pc_ctx.not_empty);
    pthread_join(threads[2], NULL);
    pthread_join(threads[3], NULL);
#endif

    pc_destroy(&pc_ctx);

    printf("\n举一反三:\n");
    printf("  1. 条件变量必须配合互斥锁使用\n");
    printf("  2. while循环检查条件(非if), 防止虚假唤醒\n");
    printf("  3. signal唤醒一个, broadcast唤醒全部\n");
}

typedef struct {
    int data;
    int readers_active;
    int writer_active;
    int writers_waiting;
#ifdef _WIN32
    CRITICAL_SECTION mutex;
    CONDITION_VARIABLE can_read;
    CONDITION_VARIABLE can_write;
#else
    pthread_mutex_t mutex;
    pthread_cond_t can_read;
    pthread_cond_t can_write;
#endif
} RWLock;

static RWLock rwlock;

void rwlock_init(RWLock *rw) {
    rw->data = 0;
    rw->readers_active = 0;
    rw->writer_active = 0;
    rw->writers_waiting = 0;
#ifdef _WIN32
    InitializeCriticalSection(&rw->mutex);
    InitializeConditionVariable(&rw->can_read);
    InitializeConditionVariable(&rw->can_write);
#else
    pthread_mutex_init(&rw->mutex, NULL);
    pthread_cond_init(&rw->can_read, NULL);
    pthread_cond_init(&rw->can_write, NULL);
#endif
}

void rwlock_destroy(RWLock *rw) {
#ifdef _WIN32
    DeleteCriticalSection(&rw->mutex);
#else
    pthread_mutex_destroy(&rw->mutex);
    pthread_cond_destroy(&rw->can_read);
    pthread_cond_destroy(&rw->can_write);
#endif
}

void rwlock_read_lock(RWLock *rw) {
#ifdef _WIN32
    EnterCriticalSection(&rw->mutex);
    while (rw->writer_active || rw->writers_waiting > 0) {
        SleepConditionVariableCS(&rw->can_read, &rw->mutex, INFINITE);
    }
    rw->readers_active++;
    LeaveCriticalSection(&rw->mutex);
#else
    pthread_mutex_lock(&rw->mutex);
    while (rw->writer_active || rw->writers_waiting > 0) {
        pthread_cond_wait(&rw->can_read, &rw->mutex);
    }
    rw->readers_active++;
    pthread_mutex_unlock(&rw->mutex);
#endif
}

void rwlock_read_unlock(RWLock *rw) {
#ifdef _WIN32
    EnterCriticalSection(&rw->mutex);
    rw->readers_active--;
    if (rw->readers_active == 0) {
        WakeConditionVariable(&rw->can_write);
    }
    LeaveCriticalSection(&rw->mutex);
#else
    pthread_mutex_lock(&rw->mutex);
    rw->readers_active--;
    if (rw->readers_active == 0) {
        pthread_cond_signal(&rw->can_write);
    }
    pthread_mutex_unlock(&rw->mutex);
#endif
}

void rwlock_write_lock(RWLock *rw) {
#ifdef _WIN32
    EnterCriticalSection(&rw->mutex);
    rw->writers_waiting++;
    while (rw->writer_active || rw->readers_active > 0) {
        SleepConditionVariableCS(&rw->can_write, &rw->mutex, INFINITE);
    }
    rw->writers_waiting--;
    rw->writer_active = 1;
    LeaveCriticalSection(&rw->mutex);
#else
    pthread_mutex_lock(&rw->mutex);
    rw->writers_waiting++;
    while (rw->writer_active || rw->readers_active > 0) {
        pthread_cond_wait(&rw->can_write, &rw->mutex);
    }
    rw->writers_waiting--;
    rw->writer_active = 1;
    pthread_mutex_unlock(&rw->mutex);
#endif
}

void rwlock_write_unlock(RWLock *rw) {
#ifdef _WIN32
    EnterCriticalSection(&rw->mutex);
    rw->writer_active = 0;
    WakeAllConditionVariable(&rw->can_read);
    WakeConditionVariable(&rw->can_write);
    LeaveCriticalSection(&rw->mutex);
#else
    pthread_mutex_lock(&rw->mutex);
    rw->writer_active = 0;
    pthread_cond_broadcast(&rw->can_read);
    pthread_cond_signal(&rw->can_write);
    pthread_mutex_unlock(&rw->mutex);
#endif
}

#ifdef _WIN32
DWORD WINAPI reader_func(LPVOID arg) {
#else
void *reader_func(void *arg) {
#endif
    int id = *(int *)arg;
    for (int i = 0; i < 5; i++) {
        rwlock_read_lock(&rwlock);
        printf("[读者 %d] 读取数据: %d (活跃读者: %d)\n", id, rwlock.data, rwlock.readers_active);
        rwlock_read_unlock(&rwlock);
#ifdef _WIN32
        Sleep(50);
#else
        usleep(50000);
#endif
    }
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

#ifdef _WIN32
DWORD WINAPI writer_func(LPVOID arg) {
#else
void *writer_func(void *arg) {
#endif
    int id = *(int *)arg;
    for (int i = 0; i < 3; i++) {
        rwlock_write_lock(&rwlock);
        rwlock.data += 10;
        printf("[写者 %d] 写入数据: %d\n", id, rwlock.data);
        rwlock_write_unlock(&rwlock);
#ifdef _WIN32
        Sleep(100);
#else
        usleep(100000);
#endif
    }
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

void demo_reader_writer(void) {
    printf("\n=== demo_reader_writer ===\n");
    printf("读写者问题: 允许多个读者同时读, 写者独占访问\n");
    printf("本实现采用写者优先策略, 防止写者饥饿\n\n");

    rwlock_init(&rwlock);

    int r1 = 1, r2 = 2, r3 = 3, w1 = 1, w2 = 2;

#ifdef _WIN32
    HANDLE threads[5];
    threads[0] = CreateThread(NULL, 0, reader_func, &r1, 0, NULL);
    threads[1] = CreateThread(NULL, 0, reader_func, &r2, 0, NULL);
    threads[2] = CreateThread(NULL, 0, reader_func, &r3, 0, NULL);
    threads[3] = CreateThread(NULL, 0, writer_func, &w1, 0, NULL);
    threads[4] = CreateThread(NULL, 0, writer_func, &w2, 0, NULL);
    WaitForMultipleObjects(5, threads, TRUE, INFINITE);
    for (int i = 0; i < 5; i++) CloseHandle(threads[i]);
#else
    pthread_t threads[5];
    pthread_create(&threads[0], NULL, reader_func, &r1);
    pthread_create(&threads[1], NULL, reader_func, &r2);
    pthread_create(&threads[2], NULL, reader_func, &r3);
    pthread_create(&threads[3], NULL, writer_func, &w1);
    pthread_create(&threads[4], NULL, writer_func, &w2);
    for (int i = 0; i < 5; i++) pthread_join(threads[i], NULL);
#endif

    rwlock_destroy(&rwlock);

    printf("\n读写锁陷阱:\n");
    printf("  1. 读者优先: 写者可能饥饿\n");
    printf("  2. 写者优先: 读者可能饥饿\n");
    printf("  3. 公平策略: 按请求顺序服务, 但实现更复杂\n");
}

typedef struct {
    void (*task_func)(void *);
    void *arg;
} Task;

#define MAX_TASKS 64
#define NUM_WORKERS 4

typedef struct {
    Task tasks[MAX_TASKS];
    int head;
    int tail;
    int count;
    int shutdown;
#ifdef _WIN32
    CRITICAL_SECTION mutex;
    CONDITION_VARIABLE has_task;
#else
    pthread_mutex_t mutex;
    pthread_cond_t has_task;
#endif
} ThreadPool;

static ThreadPool pool;

void pool_init(ThreadPool *p) {
    p->head = 0;
    p->tail = 0;
    p->count = 0;
    p->shutdown = 0;
#ifdef _WIN32
    InitializeCriticalSection(&p->mutex);
    InitializeConditionVariable(&p->has_task);
#else
    pthread_mutex_init(&p->mutex, NULL);
    pthread_cond_init(&p->has_task, NULL);
#endif
}

void pool_destroy(ThreadPool *p) {
#ifdef _WIN32
    DeleteCriticalSection(&p->mutex);
#else
    pthread_mutex_destroy(&p->mutex);
    pthread_cond_destroy(&p->has_task);
#endif
}

int pool_submit(ThreadPool *p, void (*func)(void *), void *arg) {
#ifdef _WIN32
    EnterCriticalSection(&p->mutex);
    if (p->count >= MAX_TASKS) {
        LeaveCriticalSection(&p->mutex);
        return -1;
    }
    p->tasks[p->tail].task_func = func;
    p->tasks[p->tail].arg = arg;
    p->tail = (p->tail + 1) % MAX_TASKS;
    p->count++;
    WakeConditionVariable(&p->has_task);
    LeaveCriticalSection(&p->mutex);
#else
    pthread_mutex_lock(&p->mutex);
    if (p->count >= MAX_TASKS) {
        pthread_mutex_unlock(&p->mutex);
        return -1;
    }
    p->tasks[p->tail].task_func = func;
    p->tasks[p->tail].arg = arg;
    p->tail = (p->tail + 1) % MAX_TASKS;
    p->count++;
    pthread_cond_signal(&p->has_task);
    pthread_mutex_unlock(&p->mutex);
#endif
    return 0;
}

#ifdef _WIN32
DWORD WINAPI worker_func(LPVOID arg) {
#else
void *worker_func(void *arg) {
#endif
    int id = *(int *)arg;
    while (1) {
#ifdef _WIN32
        EnterCriticalSection(&pool.mutex);
        while (pool.count == 0 && !pool.shutdown) {
            SleepConditionVariableCS(&pool.has_task, &pool.mutex, INFINITE);
        }
        if (pool.shutdown && pool.count == 0) {
            LeaveCriticalSection(&pool.mutex);
            break;
        }
#else
        pthread_mutex_lock(&pool.mutex);
        while (pool.count == 0 && !pool.shutdown) {
            pthread_cond_wait(&pool.has_task, &pool.mutex);
        }
        if (pool.shutdown && pool.count == 0) {
            pthread_mutex_unlock(&pool.mutex);
            break;
        }
#endif
        Task task = pool.tasks[pool.head];
        pool.head = (pool.head + 1) % MAX_TASKS;
        pool.count--;

#ifdef _WIN32
        WakeConditionVariable(&pool.has_task);
        LeaveCriticalSection(&pool.mutex);
#else
        pthread_cond_signal(&pool.has_task);
        pthread_mutex_unlock(&pool.mutex);
#endif

        task.task_func(task.arg);
        printf("[工作线程 %d] 完成任务\n", id);
    }
    printf("[工作线程 %d] 退出\n", id);
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

static int task_counter __attribute__((used)) = 0;

void compute_task(void *arg) {
    int n = *(int *)arg;
    long long sum = 0;
    for (int i = 1; i <= n; i++) {
        sum += i;
    }
    printf("  计算 1+2+...+%d = %lld\n", n, sum);
}

void demo_thread_pool(void) {
    printf("\n=== demo_thread_pool ===\n");
    printf("线程池: 预创建固定数量工作线程, 从任务队列取任务执行\n");
    printf("优点: 避免频繁创建销毁线程的开销, 控制并发数量\n\n");

    pool_init(&pool);

    int worker_ids[NUM_WORKERS];
#ifdef _WIN32
    HANDLE workers[NUM_WORKERS];
    for (int i = 0; i < NUM_WORKERS; i++) {
        worker_ids[i] = i + 1;
        workers[i] = CreateThread(NULL, 0, worker_func, &worker_ids[i], 0, NULL);
    }
#else
    pthread_t workers[NUM_WORKERS];
    for (int i = 0; i < NUM_WORKERS; i++) {
        worker_ids[i] = i + 1;
        pthread_create(&workers[i], NULL, worker_func, &worker_ids[i]);
    }
#endif

    int task_data[10] = {100, 200, 300, 400, 500, 600, 700, 800, 900, 1000};
    for (int i = 0; i < 10; i++) {
        pool_submit(&pool, compute_task, &task_data[i]);
    }

#ifdef _WIN32
    Sleep(2000);
#else
    sleep(2);
#endif

#ifdef _WIN32
    EnterCriticalSection(&pool.mutex);
    pool.shutdown = 1;
    WakeAllConditionVariable(&pool.has_task);
    LeaveCriticalSection(&pool.mutex);
    WaitForMultipleObjects(NUM_WORKERS, workers, TRUE, INFINITE);
    for (int i = 0; i < NUM_WORKERS; i++) CloseHandle(workers[i]);
#else
    pthread_mutex_lock(&pool.mutex);
    pool.shutdown = 1;
    pthread_cond_broadcast(&pool.has_task);
    pthread_mutex_unlock(&pool.mutex);
    for (int i = 0; i < NUM_WORKERS; i++) pthread_join(workers[i], NULL);
#endif

    pool_destroy(&pool);

    printf("\n线程池最佳实践:\n");
    printf("  1. 线程数 = CPU核心数 * (1 + I/O等待时间/CPU时间)\n");
    printf("  2. 任务队列需要有界, 防止内存溢出\n");
    printf("  3. 优雅关闭: 先停止接受新任务, 等待现有任务完成\n");
}

int main(void) {
    printf("线程模式深入: 线程池、生产者消费者、读写者问题\n");

    demo_producer_consumer();
    demo_reader_writer();
    demo_thread_pool();

    printf("\n所有演示完成!\n");
    return 0;
}
