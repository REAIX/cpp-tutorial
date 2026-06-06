/**
 * @file 01_example_fork.c
 * @brief 进程创建示例
 * @description 对应文档: 24-进程与线程
 *  @note C++ 中可使用 std::thread / std::mutex 等更高级的抽象, 参见 C++ 章节 29-34
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <process.h>
#include <windows.h>
#else
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#endif

void demo_fork_basic(void) {
    printf("\n=== demo_fork_basic ===\n");

#ifdef _WIN32
    printf("[Windows] fork() 在 Windows 上不可用, 使用 _beginthreadex 模拟\n");
    printf("当前进程 PID: %d\n", _getpid());

    HANDLE hThread = (HANDLE)_beginthreadex(NULL, 0,
        (unsigned (__stdcall *)(void *))(void (*)(void))Sleep, (void *)1000, 0, NULL);
    if (hThread) {
        WaitForSingleObject(hThread, 2000);
        CloseHandle(hThread);
        printf("子线程执行完毕\n");
    }
#else
    printf("父进程 PID: %d, 父进程的父进程 PID: %d\n", getpid(), getppid());

    pid_t pid = fork();

    if (pid < 0) {
        perror("fork 失败");
        return;
    }

    if (pid == 0) {
        printf("[子进程] 我是子进程! PID=%d, 父进程 PID=%d\n", getpid(), getppid());
        exit(0);
    } else {
        printf("[父进程] 创建了子进程, 子进程 PID=%d\n", pid);
        int status;
        waitpid(pid, &status, 0);
        printf("[父进程] 子进程已退出, 状态=%d\n", WEXITSTATUS(status));
    }
#endif
}

void demo_fork_multiple(void) {
    printf("\n=== demo_fork_multiple ===\n");

#ifdef _WIN32
    printf("[Windows] 模拟创建多个子进程\n");
    for (int i = 0; i < 3; i++) {
        STARTUPINFO si = {0};
        si.cb = sizeof(si);
        PROCESS_INFORMATION pi;
        char cmd[256];
        snprintf(cmd, sizeof(cmd), "cmd /c echo 子进程 %d PID=%d", i + 1, _getpid());
        if (CreateProcess(NULL, cmd, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
            WaitForSingleObject(pi.hProcess, 5000);
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);
        }
    }
#else
    printf("父进程 PID: %d\n", getpid());

    for (int i = 0; i < 3; i++) {
        pid_t pid = fork();
        if (pid < 0) {
            perror("fork 失败");
            return;
        }
        if (pid == 0) {
            printf("[子进程 %d] PID=%d, 父进程 PID=%d\n", i + 1, getpid(), getppid());
            exit(i + 1);
        }
    }

    for (int i = 0; i < 3; i++) {
        int status;
        pid_t pid = wait(&status);
        printf("[父进程] 子进程 PID=%d 退出, 状态=%d\n", pid, WEXITSTATUS(status));
    }
#endif
}

void demo_fork_shared_data(void) {
    printf("\n=== demo_fork_shared_data ===\n");

#ifdef _WIN32
    printf("[Windows] 进程间不共享内存, 各自独立\n");
    int var = 42;
    printf("变量初始值: %d\n", var);
    printf("注意: Windows 下进程间需要通过共享内存(如 CreateFileMapping)通信\n");
#else
    int var = 42;
    printf("fork 前: var = %d (地址: %p)\n", var, (void *)&var);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork 失败");
        return;
    }

    if (pid == 0) {
        var = 100;
        printf("[子进程] 修改 var = %d (地址: %p) - 写时复制!\n", var, (void *)&var);
        exit(0);
    } else {
        waitpid(pid, NULL, 0);
        printf("[父进程] var 仍为 %d (地址: %p) - 进程间内存隔离\n", var, (void *)&var);
    }
#endif
}

int main(void) {
    printf("进程创建示例 - fork(), getpid(), getppid()\n");

    demo_fork_basic();
    demo_fork_multiple();
    demo_fork_shared_data();

    printf("\n所有演示完成!\n");
    return 0;
}
