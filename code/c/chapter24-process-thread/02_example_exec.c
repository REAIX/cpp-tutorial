/**
 * @file 02_example_exec.c
 * @brief exec族函数与管道fork模式
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

void demo_exec_family(void) {
    printf("\n=== demo_exec_family ===\n");

#ifdef _WIN32
    printf("[Windows] 使用 _spawnl 系列函数替代 exec\n");
    printf("exec 族函数在 Windows 上不可用, 使用 spawn 系列替代\n");

    int result = _spawnl(_P_WAIT, "cmd.exe", "cmd.exe", "/c", "echo", "Hello from spawn", NULL);
    printf("spawn 返回值: %d\n", result);
#else
    printf("演示 execlp: 让子进程执行 ls 命令\n");

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork 失败");
        return;
    }

    if (pid == 0) {
        printf("[子进程] 执行 execlp(\"ls\", \"ls\", \"-la\", NULL)\n");
        execlp("ls", "ls", "-la", NULL);
        perror("execlp 失败");
        exit(1);
    } else {
        int status;
        waitpid(pid, &status, 0);
        printf("[父进程] 子进程执行完毕, 状态=%d\n", WEXITSTATUS(status));
    }
#endif
}

void demo_pipe_fork(void) {
    printf("\n=== demo_pipe_fork ===\n");

#ifdef _WIN32
    printf("[Windows] 使用 CreatePipe + CreateProcess 实现管道通信\n");

    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        printf("CreatePipe 失败\n");
        return;
    }

    STARTUPINFO si = {0};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {0};
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdOutput = hWritePipe;
    si.hStdError = hWritePipe;
    si.hStdInput = GetStdHandle(STD_INPUT_HANDLE);

    if (CreateProcess(NULL, "cmd /c echo Hello from pipe", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        CloseHandle(hWritePipe);

        char buffer[256] = {0};
        DWORD bytesRead;
        ReadFile(hReadPipe, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        printf("从管道读取: %s", buffer);

        WaitForSingleObject(pi.hProcess, 5000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    CloseHandle(hReadPipe);
#else
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe 失败");
        return;
    }

    printf("创建管道: 读端=%d, 写端=%d\n", pipefd[0], pipefd[1]);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork 失败");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid == 0) {
        close(pipefd[0]);
        const char *msg = "Hello from child via pipe!";
        write(pipefd[1], msg, strlen(msg));
        printf("[子进程] 已写入管道: \"%s\"\n", msg);
        close(pipefd[1]);
        exit(0);
    } else {
        close(pipefd[1]);
        char buffer[256] = {0};
        ssize_t n = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("[父进程] 从管道读取: \"%s\"\n", buffer);
        }
        close(pipefd[0]);
        waitpid(pid, NULL, 0);
    }
#endif
}

void demo_pipe_fork_filter(void) {
    printf("\n=== demo_pipe_fork_filter ===\n");

#ifdef _WIN32
    printf("[Windows] 管道过滤模式: 父进程写入 -> 子进程读取处理\n");

    HANDLE hReadPipe1, hWritePipe1;
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };
    CreatePipe(&hReadPipe1, &hWritePipe1, &sa, 0);

    HANDLE hReadPipe2, hWritePipe2;
    CreatePipe(&hReadPipe2, &hWritePipe2, &sa, 0);

    STARTUPINFO si = {0};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {0};
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hReadPipe1;
    si.hStdOutput = hWritePipe2;
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);

    if (CreateProcess(NULL, "cmd /c findstr Hello", NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi)) {
        const char *data = "Hello World\nGoodbye\nHello C\n";
        DWORD written;
        WriteFile(hWritePipe1, data, (DWORD)strlen(data), &written, NULL);
        CloseHandle(hWritePipe1);
        CloseHandle(hWritePipe2);

        char buffer[256] = {0};
        DWORD bytesRead;
        ReadFile(hReadPipe2, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        printf("过滤结果:\n%s", buffer);

        WaitForSingleObject(pi.hProcess, 5000);
        CloseHandle(pi.hProcess);
        CloseHandle(pi.hThread);
    }
    CloseHandle(hReadPipe1);
    CloseHandle(hReadPipe2);
#else
    int pipe_in[2], pipe_out[2];
    pipe(pipe_in);
    pipe(pipe_out);

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork 失败");
        return;
    }

    if (pid == 0) {
        close(pipe_in[1]);
        close(pipe_out[0]);
        dup2(pipe_in[0], STDIN_FILENO);
        dup2(pipe_out[1], STDOUT_FILENO);
        close(pipe_in[0]);
        close(pipe_out[1]);
        execlp("grep", "grep", "Hello", NULL);
        perror("execlp grep 失败");
        exit(1);
    } else {
        close(pipe_in[0]);
        close(pipe_out[1]);

        const char *data = "Hello World\nGoodbye\nHello C\n";
        write(pipe_in[1], data, strlen(data));
        close(pipe_in[1]);

        char buffer[256] = {0};
        ssize_t n = read(pipe_out[0], buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("过滤结果:\n%s", buffer);
        }
        close(pipe_out[0]);
        waitpid(pid, NULL, 0);
    }
#endif
}

int main(void) {
    printf("exec族函数与管道fork模式示例\n");

    demo_exec_family();
    demo_pipe_fork();
    demo_pipe_fork_filter();

    printf("\n所有演示完成!\n");
    return 0;
}
