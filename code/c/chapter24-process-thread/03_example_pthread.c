/**
 * @file 03_example_pthread.c
 * @brief POSIX线程创建与参数传递
 * @description 对应文档: 24-进程与线程
 *  @note C++ 中可使用 std::thread / std::mutex 等更高级的抽象, 参见 C++ 章节 29-34
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <pthread.h>
#include <unistd.h>
#endif

#ifdef _WIN32
DWORD WINAPI thread_basic_func(LPVOID arg) {
    int id = *(int *)arg;
    printf("[线程 %d] 正在执行, 线程ID=%lu\n", id, GetCurrentThreadId());
    return 0;
}
#else
void *thread_basic_func(void *arg) {
    int id = *(int *)arg;
    printf("[线程 %d] 正在执行, 线程ID=%lu\n", id, (unsigned long)pthread_self());
    return NULL;
}
#endif

void demo_thread_basic(void) {
    printf("\n=== demo_thread_basic ===\n");

#ifdef _WIN32
    printf("[Windows] 使用 CreateThread 创建线程\n");

    int args[3] = {1, 2, 3};
    HANDLE threads[3];
    for (int i = 0; i < 3; i++) {
        threads[i] = CreateThread(NULL, 0, thread_basic_func, &args[i], 0, NULL);
    }
    WaitForMultipleObjects(3, threads, TRUE, INFINITE);
    for (int i = 0; i < 3; i++) {
        CloseHandle(threads[i]);
    }
#else
    printf("[POSIX] 使用 pthread_create 创建线程\n");

    int args[3] = {1, 2, 3};
    pthread_t threads[3];
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_basic_func, &args[i]);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
#endif
    printf("所有线程执行完毕\n");
}

typedef struct {
    int id;
    int iterations;
    int result;
} ThreadArg;

#ifdef _WIN32
DWORD WINAPI thread_compute_func(LPVOID arg) {
#else
void *thread_compute_func(void *arg) {
#endif
    ThreadArg *targ = (ThreadArg *)arg;
    int sum = 0;
    for (int i = 0; i < targ->iterations; i++) {
        sum += i;
    }
    targ->result = sum;
    printf("[线程 %d] 计算 %d 次累加, 结果=%d\n", targ->id, targ->iterations, sum);
#ifdef _WIN32
    return 0;
#else
    return NULL;
#endif
}

void demo_thread_args_return(void) {
    printf("\n=== demo_thread_args_return ===\n");

    ThreadArg args[3] = {
        {1, 100, 0},
        {2, 200, 0},
        {3, 500, 0}
    };

#ifdef _WIN32
    HANDLE threads[3];
    for (int i = 0; i < 3; i++) {
        threads[i] = CreateThread(NULL, 0, thread_compute_func, &args[i], 0, NULL);
    }
    WaitForMultipleObjects(3, threads, TRUE, INFINITE);
    for (int i = 0; i < 3; i++) {
        CloseHandle(threads[i]);
    }
#else
    pthread_t threads[3];
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_compute_func, &args[i]);
    }
    for (int i = 0; i < 3; i++) {
        pthread_join(threads[i], NULL);
    }
#endif

    int total = 0;
    for (int i = 0; i < 3; i++) {
        printf("线程 %d 返回结果: %d\n", args[i].id, args[i].result);
        total += args[i].result;
    }
    printf("所有线程结果总和: %d\n", total);
}

#ifdef _WIN32
DWORD WINAPI thread_return_ptr_func(LPVOID arg) {
    int *result = (int *)malloc(sizeof(int));
    *result = *(int *)arg * 10;
    return (DWORD)(uintptr_t)result;
}
#else
void *thread_return_ptr_func(void *arg) {
    int *result = (int *)malloc(sizeof(int));
    *result = *(int *)arg * 10;
    return result;
}
#endif

void demo_thread_return_value(void) {
    printf("\n=== demo_thread_return_value ===\n");

    int input_values[3] = {5, 10, 15};

#ifdef _WIN32
    HANDLE threads[3];
    for (int i = 0; i < 3; i++) {
        threads[i] = CreateThread(NULL, 0, thread_return_ptr_func, &input_values[i], 0, NULL);
    }
    WaitForMultipleObjects(3, threads, TRUE, INFINITE);
    for (int i = 0; i < 3; i++) {
        DWORD exit_code;
        GetExitCodeThread(threads[i], &exit_code);
        int *result = (int *)(uintptr_t)exit_code;
        printf("输入 %d -> 线程返回值: %d\n", input_values[i], *result);
        free(result);
        CloseHandle(threads[i]);
    }
#else
    pthread_t threads[3];
    for (int i = 0; i < 3; i++) {
        pthread_create(&threads[i], NULL, thread_return_ptr_func, &input_values[i]);
    }
    for (int i = 0; i < 3; i++) {
        void *retval;
        pthread_join(threads[i], &retval);
        int *result = (int *)retval;
        printf("输入 %d -> 线程返回值: %d\n", input_values[i], *result);
        free(result);
    }
#endif
}

int main(void) {
    printf("POSIX线程创建与参数传递示例\n");

    demo_thread_basic();
    demo_thread_args_return();
    demo_thread_return_value();

    printf("\n所有演示完成!\n");
    return 0;
}
