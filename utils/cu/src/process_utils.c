/**
 * @file process_utils.c
 * @brief 进程工具实现 (C 版本)
 *
 * 实现延时、命令执行、进程信息获取等功能，支持跨平台。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#include "cu/process_utils.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

/**
 * @brief 阻塞式延时
 *
 * 使当前线程阻塞指定的毫秒数。
 *
 * @param milliseconds 延时的毫秒数
 */
void cu_sleep(unsigned int milliseconds) {
#ifdef _WIN32
    Sleep(milliseconds);
#else
    struct timespec ts;
    ts.tv_sec = milliseconds / 1000;
    ts.tv_nsec = (long)(milliseconds % 1000) * 1000000L;
    nanosleep(&ts, NULL);
#endif
}

/**
 * @brief 执行系统命令
 *
 * 执行指定的系统命令并捕获标准输出。
 *
 * @param command 要执行的命令字符串
 * @param output 输出缓冲区
 * @param output_size 输出缓冲区大小
 * @return 命令退出码，失败返回-1
 */
int execute_command(const char* command, char* output, size_t output_size) {
    if (!command || !output || output_size == 0) {
        return -1;
    }

    output[0] = '\0';

#ifdef _WIN32
    FILE* pipe = _popen(command, "r");
#else
    FILE* pipe = popen(command, "r");
#endif

    if (!pipe) {
        return -1;
    }

    /* 读取命令输出 */
    size_t total = 0;
    char buffer[256];
    while (fgets(buffer, sizeof(buffer), pipe) && total < output_size - 1) {
        size_t len = strlen(buffer);
        if (total + len >= output_size) {
            len = output_size - total - 1;
        }
        memcpy(output + total, buffer, len);
        total += len;
    }
    output[total] = '\0';

#ifdef _WIN32
    int result = _pclose(pipe);
    return result;
#else
    int result = pclose(pipe);
    return WIFEXITED(result) ? WEXITSTATUS(result) : -1;
#endif
}

/**
 * @brief 获取当前进程 ID
 *
 * @return 当前进程的 ID
 */
int get_process_id(void) {
#ifdef _WIN32
    return _getpid();
#else
    return getpid();
#endif
}

/**
 * @brief 获取环境变量值
 *
 * @param name 环境变量名
 * @param buffer 输出缓冲区
 * @param buffer_size 缓冲区大小
 * @return 指向 buffer 的指针，失败返回 NULL
 */
char* get_environment_variable(const char* name, char* buffer, size_t buffer_size) {
    if (!name || !buffer || buffer_size == 0) {
        return NULL;
    }

#ifdef _WIN32
    DWORD len = GetEnvironmentVariableA(name, buffer, (DWORD)buffer_size);
    if (len == 0 || len >= buffer_size) {
        buffer[0] = '\0';
        return NULL;
    }
#else
    char* value = getenv(name);
    if (!value) {
        buffer[0] = '\0';
        return NULL;
    }
    strncpy(buffer, value, buffer_size - 1);
    buffer[buffer_size - 1] = '\0';
#endif

    return buffer;
}

/**
 * @brief 设置环境变量
 *
 * @param name 环境变量名
 * @param value 环境变量值（为 NULL 则删除）
 * @return 成功返回1，失败返回0
 */
int set_environment_variable(const char* name, const char* value) {
    if (!name) {
        return 0;
    }

#ifdef _WIN32
    return SetEnvironmentVariableA(name, value) != 0;
#else
    if (value) {
        return setenv(name, value, 1) == 0;
    } else {
        return unsetenv(name) == 0;
    }
#endif
}