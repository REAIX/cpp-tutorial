/**
 * @file 02_example_signal.c
 * @brief 信号处理基础
 * @description 对应文档: 13-错误处理与信号
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

static volatile sig_atomic_t g_interrupted = 0;
static int g_sigint_count = 0;

void sigint_handler(int signo) {
    (void)signo;
    const char msg[] = "\n  [SIGINT捕获] 收到中断信号\n";
    fwrite(msg, 1, sizeof(msg) - 1, stdout);
    g_interrupted = 1;
}

void sigterm_handler(int signo) {
    (void)signo;
    const char msg[] = "\n  [SIGTERM捕获] 收到终止信号, 正在清理退出...\n";
    fwrite(msg, 1, sizeof(msg) - 1, stdout);
    exit(0);
}

void sigint_count_handler(int signo) {
    (void)signo;
    g_sigint_count++;
}

void fpe_handler(int signo) {
    (void)signo;
    const char msg[] = "  [SIGFPE捕获] 浮点异常/除零错误!\n";
    fwrite(msg, 1, sizeof(msg) - 1, stdout);
    exit(1);
}

void demo_signal_basic(void) {
    printf("=== signal()基本用法 ===\n");
    signal(SIGINT, sigint_handler);
    signal(SIGTERM, sigterm_handler);

    printf("  已注册SIGINT和SIGTERM处理函数\n");
    printf("  g_interrupted = %d\n", (int)g_interrupted);
    printf("  (实际运行时可按Ctrl+C测试, 本演示仅展示注册)\n\n");

    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);
}

void demo_signal_ignore(void) {
    printf("=== 忽略信号 ===\n");
    signal(SIGINT, SIG_IGN);
    printf("  SIGINT已被忽略 (Ctrl+C不会中断)\n");
    printf("  恢复默认处理...\n");
    signal(SIGINT, SIG_DFL);
    printf("  SIGINT已恢复默认处理\n\n");
}

void demo_raise_signal(void) {
    printf("=== raise()发送信号 ===\n");

    g_sigint_count = 0;
    void (*old_handler)(int) = signal(SIGINT, sigint_count_handler);
    printf("  发送3次SIGINT(用自定义处理函数计数)...\n");
    for (int i = 0; i < 3; i++) {
        raise(SIGINT);
        printf("  第%d次SIGINT已处理, count=%d\n", i + 1, g_sigint_count);
    }
    signal(SIGINT, old_handler ? old_handler : SIG_DFL);
    printf("  注意: SIGUSR1在Windows上不可用, 此处用SIGINT演示raise\n\n");
}

void demo_signal_names(void) {
    printf("=== 标准信号一览 ===\n");
    struct { int signo; const char *name; const char *desc; } sigs[] = {
        {SIGABRT, "SIGABRT", "异常终止(abort)"},
        {SIGFPE,  "SIGFPE",  "浮点异常(如除零)"},
        {SIGILL,  "SIGILL",  "非法指令"},
        {SIGINT,  "SIGINT",  "中断(Ctrl+C)"},
        {SIGSEGV, "SIGSEGV", "段错误(非法内存访问)"},
        {SIGTERM, "SIGTERM", "终止信号"},
    };
    for (int i = 0; i < (int)(sizeof(sigs) / sizeof(sigs[0])); i++) {
        printf("  %-10s (%2d): %s\n", sigs[i].name, sigs[i].signo, sigs[i].desc);
    }
    printf("  注意: SIGUSR1/SIGUSR2等POSIX信号在Windows上不可用\n\n");
}

void demo_sigfpe_handler(void) {
    printf("=== SIGFPE处理(整数除零) ===\n");

    signal(SIGFPE, fpe_handler);
    printf("  已注册SIGFPE处理函数\n");
    printf("  注意: 整数除零是未定义行为, 不一定触发SIGFPE\n");
    printf("  (不同编译器和平台行为可能不同)\n\n");
    signal(SIGFPE, SIG_DFL);
}

int main(void) {
    printf("========== 信号处理基础示例 ==========\n\n");

    demo_signal_basic();
    demo_signal_ignore();
    demo_raise_signal();
    demo_signal_names();
    demo_sigfpe_handler();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
