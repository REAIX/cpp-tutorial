/**
 * @file 04_example_mutex.c
 * @brief 互斥锁与竞态条件演示
 * @description 对应文档: 24-进程与线程
 *  @note C++ 中可使用 std::thread / std::mutex 等更高级的抽象, 参见 C++ 章节 29-34
 */

#include <stdio.h>
#include <stdlib.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#endif

static int shared_counter = 0;

#ifdef _WIN32
static CRITICAL_SECTION cs;
#else
static pthread_mutex_t mutex;
#endif

#ifdef _WIN32
DWORD WINAPI increment_without_lock(LPVOID arg) {
#else
void *increment_without_lock(void *arg) {
#endif
    int iterations = *(int *)arg;
    for (int i = 0; i < iterations; i++) {
        shared_counter++;
    }
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

void demo_race_condition(void) {
    printf("\n=== demo_race_condition ===\n");

    int iterations = 1000000;
    shared_counter = 0;

#ifdef _WIN32
    HANDLE t1 = CreateThread(NULL, 0, increment_without_lock, &iterations, 0, NULL);
    HANDLE t2 = CreateThread(NULL, 0, increment_without_lock, &iterations, 0, NULL);
    WaitForMultipleObjects(2, (HANDLE[]){t1, t2}, TRUE, INFINITE);
    CloseHandle(t1);
    CloseHandle(t2);
#else
    pthread_t t1, t2;
    pthread_create(&t1, NULL, increment_without_lock, &iterations);
    pthread_create(&t2, NULL, increment_without_lock, &iterations);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
#endif

    printf("无锁保护: 期望值=%d, 实际值=%d (竞态条件导致数据丢失!)\n",
           iterations * 2, shared_counter);
}

#ifdef _WIN32
DWORD WINAPI increment_with_lock(LPVOID arg) {
#else
void *increment_with_lock(void *arg) {
#endif
    int iterations = *(int *)arg;
    for (int i = 0; i < iterations; i++) {
#ifdef _WIN32
        EnterCriticalSection(&cs);
        shared_counter++;
        LeaveCriticalSection(&cs);
#else
        pthread_mutex_lock(&mutex);
        shared_counter++;
        pthread_mutex_unlock(&mutex);
#endif
    }
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

void demo_mutex_protection(void) {
    printf("\n=== demo_mutex_protection ===\n");

    int iterations = 1000000;
    shared_counter = 0;

#ifdef _WIN32
    InitializeCriticalSection(&cs);
    HANDLE t1 = CreateThread(NULL, 0, increment_with_lock, &iterations, 0, NULL);
    HANDLE t2 = CreateThread(NULL, 0, increment_with_lock, &iterations, 0, NULL);
    WaitForMultipleObjects(2, (HANDLE[]){t1, t2}, TRUE, INFINITE);
    CloseHandle(t1);
    CloseHandle(t2);
    DeleteCriticalSection(&cs);
#else
    pthread_mutex_init(&mutex, NULL);
    pthread_t t1, t2;
    pthread_create(&t1, NULL, increment_with_lock, &iterations);
    pthread_create(&t2, NULL, increment_with_lock, &iterations);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_mutex_destroy(&mutex);
#endif

    printf("有锁保护: 期望值=%d, 实际值=%d (互斥锁保证正确性!)\n",
           iterations * 2, shared_counter);
}

typedef struct {
    int balance;
#ifdef _WIN32
    CRITICAL_SECTION lock;
#else
    pthread_mutex_t lock;
#endif
} BankAccount;

void bank_init(BankAccount *acc, int initial) {
    acc->balance = initial;
#ifdef _WIN32
    InitializeCriticalSection(&acc->lock);
#else
    pthread_mutex_init(&acc->lock, NULL);
#endif
}

void bank_destroy(BankAccount *acc) {
#ifdef _WIN32
    DeleteCriticalSection(&acc->lock);
#else
    pthread_mutex_destroy(&acc->lock);
#endif
}

void bank_deposit(BankAccount *acc, int amount) {
#ifdef _WIN32
    EnterCriticalSection(&acc->lock);
    int old = acc->balance;
    acc->balance = old + amount;
    LeaveCriticalSection(&acc->lock);
#else
    pthread_mutex_lock(&acc->lock);
    int old = acc->balance;
    acc->balance = old + amount;
    pthread_mutex_unlock(&acc->lock);
#endif
}

void bank_withdraw(BankAccount *acc, int amount) {
#ifdef _WIN32
    EnterCriticalSection(&acc->lock);
    if (acc->balance >= amount) {
        acc->balance -= amount;
    }
    LeaveCriticalSection(&acc->lock);
#else
    pthread_mutex_lock(&acc->lock);
    if (acc->balance >= amount) {
        acc->balance -= amount;
    }
    pthread_mutex_unlock(&acc->lock);
#endif
}

static BankAccount g_account;

#ifdef _WIN32
DWORD WINAPI deposit_thread(LPVOID arg) {
#else
void *deposit_thread(void *arg) {
#endif
    (void)arg;
    for (int i = 0; i < 10000; i++) {
        bank_deposit(&g_account, 1);
    }
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

#ifdef _WIN32
DWORD WINAPI withdraw_thread(LPVOID arg) {
#else
void *withdraw_thread(void *arg) {
#endif
    (void)arg;
    for (int i = 0; i < 10000; i++) {
        bank_withdraw(&g_account, 1);
    }
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

void demo_bank_account(void) {
    printf("\n=== demo_bank_account ===\n");

    bank_init(&g_account, 10000);
    printf("初始余额: %d\n", g_account.balance);

#ifdef _WIN32
    HANDLE t1 = CreateThread(NULL, 0, deposit_thread, NULL, 0, NULL);
    HANDLE t2 = CreateThread(NULL, 0, withdraw_thread, NULL, 0, NULL);
    WaitForMultipleObjects(2, (HANDLE[]){t1, t2}, TRUE, INFINITE);
    CloseHandle(t1);
    CloseHandle(t2);
#else
    pthread_t t1, t2;
    pthread_create(&t1, NULL, deposit_thread, NULL);
    pthread_create(&t2, NULL, withdraw_thread, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
#endif

    printf("最终余额: %d (存取各10000次1元, 余额应不变)\n", g_account.balance);
    bank_destroy(&g_account);
}

int main(void) {
    printf("互斥锁与竞态条件演示\n");

    demo_race_condition();
    demo_mutex_protection();
    demo_bank_account();

    printf("\n所有演示完成!\n");
    return 0;
}
