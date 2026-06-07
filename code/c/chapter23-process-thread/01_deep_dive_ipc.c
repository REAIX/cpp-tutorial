/**
 * @file 01_deep_dive_ipc.c
 * @brief 进程间通信深入: 管道、FIFO、共享内存、消息队列
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
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <sys/stat.h>
#endif

void demo_pipe_ipc(void) {
    printf("\n=== demo_pipe_ipc ===\n");
    printf("管道(Pipe): 半双工通信, 只能在有亲缘关系的进程间使用\n");
    printf("特点: 数据单向流动, 先进先出, 内核缓冲区\n\n");

#ifdef _WIN32
    HANDLE hRead, hWrite;
    SECURITY_ATTRIBUTES sa = { sizeof(sa), NULL, TRUE };

    if (!CreatePipe(&hRead, &hWrite, &sa, 4096)) {
        printf("CreatePipe 失败\n");
        return;
    }

    const char *msg = "Hello from parent via pipe!";
    DWORD written;
    WriteFile(hWrite, msg, (DWORD)strlen(msg), &written, NULL);
    printf("[父进程] 写入管道: \"%s\" (%lu 字节)\n", msg, written);
    CloseHandle(hWrite);

    char buffer[256] = {0};
    DWORD bytesRead;
    ReadFile(hRead, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
    printf("[模拟子进程] 从管道读取: \"%s\" (%lu 字节)\n", buffer, bytesRead);
    // 注意: Windows 版本未创建子进程, 仅在同一进程中演示管道读写
    // 实际 IPC 应使用 CreateProcess 创建子进程, 并通过管道通信
    CloseHandle(hRead);
#else
    int pipefd[2];
    if (pipe(pipefd) < 0) {
        perror("pipe");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        close(pipefd[0]);
        close(pipefd[1]);
        return;
    }

    if (pid == 0) {
        close(pipefd[1]);
        char buffer[256] = {0};
        ssize_t n = read(pipefd[0], buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("[子进程] 从管道读取: \"%s\" (%zd 字节)\n", buffer, n);
        }
        close(pipefd[0]);
        exit(0);
    } else {
        close(pipefd[0]);
        const char *msg = "Hello from parent via pipe!";
        ssize_t n = write(pipefd[1], msg, strlen(msg));
        printf("[父进程] 写入管道: \"%s\" (%zd 字节)\n", msg, n);
        close(pipefd[1]);
        waitpid(pid, NULL, 0);
    }
#endif

    printf("\n管道陷阱:\n");
    printf("  1. 管道容量有限(通常64KB), 写满后写端阻塞\n");
    printf("  2. 读端关闭后写端会收到SIGPIPE信号\n");
    printf("  3. 只能用于有亲缘关系的进程间通信\n");
}

void demo_fifo_ipc(void) {
    printf("\n=== demo_fifo_ipc ===\n");
    printf("FIFO(命名管道): 任意进程间通信, 文件系统中存在路径名\n");
    printf("特点: 半双工, 通过文件路径标识, 无亲缘关系进程可用\n\n");

#ifdef _WIN32
    printf("[Windows] 使用 CreateNamedPipe 模拟 FIFO\n");

    const char *pipe_name = "\\\\.\\pipe\\my_fifo_pipe";
    HANDLE hServer = CreateNamedPipeA(pipe_name, PIPE_ACCESS_DUPLEX,
        PIPE_TYPE_BYTE | PIPE_READMODE_BYTE | PIPE_WAIT, 1, 4096, 4096, 0, NULL);
    if (hServer == INVALID_HANDLE_VALUE) {
        printf("CreateNamedPipe 失败: %lu\n", GetLastError());
        return;
    }

    printf("[服务端] 等待客户端连接...\n");
    if (ConnectNamedPipe(hServer, NULL) || GetLastError() == ERROR_PIPE_CONNECTED) {
        const char *msg = "Hello from FIFO server!";
        DWORD written;
        WriteFile(hServer, msg, (DWORD)strlen(msg), &written, NULL);
        printf("[服务端] 发送: \"%s\"\n", msg);

        char buffer[256] = {0};
        DWORD bytesRead;
        ReadFile(hServer, buffer, sizeof(buffer) - 1, &bytesRead, NULL);
        printf("[服务端] 收到响应: \"%s\"\n", buffer);
    }
    CloseHandle(hServer);
#else
    const char *fifo_path = "/tmp/my_fifo_demo";
    unlink(fifo_path);

    if (mkfifo(fifo_path, 0666) < 0) {
        perror("mkfifo");
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        unlink(fifo_path);
        return;
    }

    if (pid == 0) {
        int fd = open(fifo_path, O_WRONLY);
        const char *msg = "Hello from child via FIFO!";
        write(fd, msg, strlen(msg));
        printf("[子进程] 写入FIFO: \"%s\"\n", msg);
        close(fd);
        exit(0);
    } else {
        int fd = open(fifo_path, O_RDONLY);
        char buffer[256] = {0};
        ssize_t n = read(fd, buffer, sizeof(buffer) - 1);
        if (n > 0) {
            buffer[n] = '\0';
            printf("[父进程] 从FIFO读取: \"%s\"\n", buffer);
        }
        close(fd);
        waitpid(pid, NULL, 0);
        unlink(fifo_path);
    }
#endif

    printf("\nFIFO vs Pipe:\n");
    printf("  FIFO: 文件系统中有路径名, 无亲缘关系进程可用\n");
    printf("  Pipe: 匿名, 只能父子进程间使用\n");
    printf("  共同: 半双工, 字节流, 先进先出\n");
}

void demo_shared_memory(void) {
    printf("\n=== demo_shared_memory ===\n");
    printf("共享内存: 最快的IPC方式, 进程直接读写同一块内存\n");
    printf("特点: 零拷贝, 需要同步机制, 生命周期随内核\n\n");

#ifdef _WIN32
    printf("[Windows] 使用 CreateFileMapping 实现共享内存\n");

    HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, 4096, "MySharedMemory");
    if (!hMap) {
        printf("CreateFileMapping 失败\n");
        return;
    }

    int *shared_data = (int *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, 4096);
    if (!shared_data) {
        printf("MapViewOfFile 失败\n");
        CloseHandle(hMap);
        return;
    }

    shared_data[0] = 42;
    shared_data[1] = 100;
    printf("[进程1] 写入共享内存: data[0]=%d, data[1]=%d\n", shared_data[0], shared_data[1]);

    printf("[模拟进程2] 读取共享内存: data[0]=%d, data[1]=%d\n", shared_data[0], shared_data[1]);

    UnmapViewOfFile(shared_data);
    CloseHandle(hMap);
#else
    key_t key = ftok("/tmp", 'S');
    int shmid = shmget(key, 4096, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        return;
    }

    int *shared_data = (int *)shmat(shmid, NULL, 0);
    if (shared_data == (int *)-1) {
        perror("shmat");
        shmctl(shmid, IPC_RMID, NULL);
        return;
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        shmdt(shared_data);
        shmctl(shmid, IPC_RMID, NULL);
        return;
    }

    if (pid == 0) {
        shared_data[0] = 42;
        shared_data[1] = 100;
        printf("[子进程] 写入共享内存: data[0]=%d, data[1]=%d\n", shared_data[0], shared_data[1]);
        shmdt(shared_data);
        exit(0);
    } else {
        waitpid(pid, NULL, 0);
        printf("[父进程] 读取共享内存: data[0]=%d, data[1]=%d\n", shared_data[0], shared_data[1]);
        shmdt(shared_data);
        shmctl(shmid, IPC_RMID, NULL);
    }
#endif

    printf("\n共享内存陷阱:\n");
    printf("  1. 必须配合信号量/互斥锁使用, 否则数据竞争\n");
    printf("  2. 生命周期随内核, 需要显式删除(shmctl IPC_RMID)\n");
    printf("  3. 指针不能跨进程传递(地址空间不同)\n");
}

void demo_message_queue_concept(void) {
    printf("\n=== demo_message_queue_concept ===\n");
    printf("消息队列: 消息格式的IPC, 按类型读取\n");
    printf("特点: 有格式消息, 可按类型选择性读取, 内核持久化\n\n");

#ifdef _WIN32
    printf("[Windows] 消息队列概念演示\n");
    printf("Windows 下可使用 Mailslot 或 COM 消息队列\n");

    typedef struct {
        long mtype;
        char mtext[128];
    } Message;

    Message msgs[3] = {
        {1, "紧急消息: 系统告警"},
        {2, "普通消息: 数据更新"},
        {1, "紧急消息: 连接断开"}
    };

    printf("消息队列内容:\n");
    for (int i = 0; i < 3; i++) {
        printf("  [类型=%ld] %s\n", msgs[i].mtype, msgs[i].mtext);
    }

    printf("\n按类型1(紧急)读取:\n");
    for (int i = 0; i < 3; i++) {
        if (msgs[i].mtype == 1) {
            printf("  -> %s\n", msgs[i].mtext);
        }
    }
#else
    typedef struct {
        long mtype;
        char mtext[128];
    } Message;

    key_t key = ftok("/tmp", 'Q');
    int msqid = msgget(key, IPC_CREAT | 0666);
    if (msqid < 0) {
        perror("msgget");
        return;
    }

    Message msg1 = {1, "紧急消息: 系统告警"};
    Message msg2 = {2, "普通消息: 数据更新"};
    Message msg3 = {1, "紧急消息: 连接断开"};

    msgsnd(msqid, &msg1, sizeof(msg1.mtext), 0);
    msgsnd(msqid, &msg2, sizeof(msg2.mtext), 0);
    msgsnd(msqid, &msg3, sizeof(msg3.mtext), 0);
    printf("已发送3条消息\n");

    printf("\n按类型1(紧急)读取:\n");
    Message recv;
    while (msgrcv(msqid, &recv, sizeof(recv.mtext), 1, IPC_NOWAIT) > 0) {
        printf("  -> [类型=%ld] %s\n", recv.mtype, recv.mtext);
    }

    printf("\n按类型2(普通)读取:\n");
    while (msgrcv(msqid, &recv, sizeof(recv.mtext), 2, IPC_NOWAIT) > 0) {
        printf("  -> [类型=%ld] %s\n", recv.mtype, recv.mtext);
    }

    msgctl(msqid, IPC_RMID, NULL);
#endif

    printf("\nIPC方式对比:\n");
    printf("  管道:   简单, 半双工, 字节流, 亲缘进程\n");
    printf("  FIFO:   简单, 半双工, 字节流, 任意进程\n");
    printf("  共享内存: 最快, 需同步, 随机访问, 任意进程\n");
    printf("  消息队列: 有格式, 可选读, 内核持久, 任意进程\n");
}

int main(void) {
    printf("进程间通信深入: 管道、FIFO、共享内存、消息队列\n");

    demo_pipe_ipc();
    demo_fifo_ipc();
    demo_shared_memory();
    demo_message_queue_concept();

    printf("\n所有演示完成!\n");
    return 0;
}
