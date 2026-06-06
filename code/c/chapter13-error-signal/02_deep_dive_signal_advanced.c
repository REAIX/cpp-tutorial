/**
 * @file 02_deep_dive_signal_advanced.c
 * @brief 信号高级主题
 * @description 对应文档: 13-错误处理与信号
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>

static volatile sig_atomic_t g_signal_received = 0;
static volatile sig_atomic_t g_signal_number = 0;

void safe_handler(int signo) {
    g_signal_received = 1;
    g_signal_number = signo;
}

void demo_signal_safety(void) {
    printf("=== 信号安全性 ===\n");
    printf("  信号处理函数中只能安全调用\"异步信号安全\"函数\n");
    printf("  POSIX规定的异步信号安全函数(部分):\n");
    printf("    _exit, write, read, signal, kill, getpid, alarm\n");
    printf("    sigaction, sigprocmask, waitpid, wait\n\n");
    printf("  不安全的函数(在信号处理函数中禁止调用):\n");
    printf("    printf, malloc, free, fopen, fclose\n");
    printf("    strerror, atoi, strtol, exit\n\n");
    printf("  原因: 这些函数可能持有内部锁或操作全局状态,\n");
    printf("  如果信号打断了正在执行这些函数的代码, 可能导致死锁或数据损坏\n\n");
}

void demo_reentrant_functions(void) {
    printf("=== 可重入函数 ===\n");
    printf("  可重入函数: 可以被多个执行流安全调用(包括信号中断)\n");
    printf("  特征:\n");
    printf("    - 不使用全局/静态变量(或只读)\n");
    printf("    - 不调用malloc/free\n");
    printf("    - 不调用非可重入函数\n");
    printf("    - 不修改自身代码\n\n");
    printf("  不可重入 vs 可重入 示例:\n");
    printf("    strtok()     -> strtok_r()  (使用外部状态 vs 调用者提供状态)\n");
    printf("    rand()       -> rand_r()    (内部种子 vs 调用者提供种子)\n");
    printf("    asctime()    -> asctime_r() (静态缓冲区 vs 调用者缓冲区)\n");
    printf("    strerror()   -> strerror_r()(静态缓冲区 vs 调用者缓冲区)\n\n");
}

void demo_sigaction_basic(void) {
    printf("=== sigaction()高级信号注册 ===\n");
    printf("  sigaction比signal更强大、更可移植:\n\n");

    g_signal_received = 0;
    g_signal_number = 0;
    signal(SIGINT, safe_handler);
    raise(SIGINT);
    printf("  信号已处理: received=%d, number=%d\n", (int)g_signal_received, (int)g_signal_number);

    printf("\n  sigaction vs signal 对比:\n");
    printf("    sigaction: 可移植, 功能更丰富, 可设SA_RESTART等标志\n");
    printf("    signal:    简单但行为不一致(不同系统语义不同)\n\n");

    printf("  sigaction用法(POSIX, Linux/macOS可用):\n");
    printf("    struct sigaction sa;\n");
    printf("    sa.sa_handler = handler;\n");
    printf("    sigemptyset(&sa.sa_mask);\n");
    printf("    sa.sa_flags = SA_RESTART;\n");
    printf("    sigaction(SIGINT, &sa, NULL);\n\n");

    printf("  注意: sigaction在Windows上不可用, 需用signal()替代\n\n");
    signal(SIGINT, SIG_DFL);
}

void demo_sigaction_flags(void) {
    printf("=== sigaction标志详解 ===\n");
    printf("  SA_RESTART:  被信号中断的系统调用自动重启\n");
    printf("  SA_SIGINFO:  使用sa_sigaction(三参数)而非sa_handler\n");
    printf("  SA_NOCLDSTOP: 子进程停止时不通知父进程\n");
    printf("  SA_NOCLDWAIT: 子进程终止时不产生僵尸进程\n");
    printf("  SA_NODEFER:   处理信号时不自动屏蔽该信号(慎用!)\n\n");
}

void demo_sigprocmask(void) {
    printf("=== 信号屏蔽(sigprocmask) ===\n");
    printf("  POSIX用法:\n");
    printf("    sigset_t block_set, old_set;\n");
    printf("    sigemptyset(&block_set);\n");
    printf("    sigaddset(&block_set, SIGINT);\n");
    printf("    sigprocmask(SIG_BLOCK, &block_set, &old_set);\n");
    printf("    // ... 临界区 ...\n");
    printf("    sigprocmask(SIG_SETMASK, &old_set, NULL);\n\n");
    printf("  注意: sigprocmask在Windows上不可用\n\n");
}

void demo_signal_in_multithread(void) {
    printf("=== 多线程中的信号 ===\n");
    printf("  POSIX线程信号规则:\n");
    printf("    1. 信号处理函数是进程级别的(所有线程共享)\n");
    printf("    2. 信号屏蔽是线程级别的(每个线程独立)\n");
    printf("    3. 同步信号(SIGSEGV等)发给导致它的线程\n");
    printf("    4. 异步信号(SIGTERM等)发给任意未屏蔽的线程\n\n");
    printf("  推荐模式:\n");
    printf("    - 主线程屏蔽所有异步信号\n");
    printf("    - 创建专用信号处理线程, 用sigwait()同步等待\n");
    printf("    - 避免在多线程中使用signal()/sigaction()\n\n");
}

void demo_realtime_signals(void) {
    printf("=== 实时信号(POSIX) ===\n");
    printf("  标准信号: SIGRTMIN到SIGRTMAX\n");
    printf("  与标准信号的区别:\n");
    printf("    1. 实时信号可以排队(不丢失), 标准信号只记一次\n");
    printf("    2. 实时信号可携带附加数据(siginfo_t)\n");
    printf("    3. 同类型实时信号按发送顺序投递\n");
    printf("    4. 编号越低的实时信号优先级越高\n\n");
    printf("  注意: Windows不完全支持POSIX实时信号\n\n");
}

void demo_sigsegv_recovery(void) {
    printf("=== SIGSEGV恢复(谨慎使用!) ===\n");
    printf("  原则: SIGSEGV通常表示严重bug, 不应\"恢复\"继续执行\n");
    printf("  唯一合理用法: 记录日志后优雅退出\n\n");
    printf("  错误做法:\n");
    printf("    - longjmp跳出SIGSEGV处理函数(可能再次触发)\n");
    printf("    - 忽略SIGSEGV (程序状态已损坏)\n\n");
    printf("  正确做法:\n");
    printf("    - 记录崩溃信息(寄存器/栈/内存映射)\n");
    printf("    - 生成core dump\n");
    printf("    - 优雅退出或重启\n\n");
}

int main(void) {
    printf("========== 信号高级主题 ==========\n\n");

    demo_signal_safety();
    demo_reentrant_functions();
    demo_sigaction_basic();
    demo_sigaction_flags();
    demo_sigprocmask();
    demo_signal_in_multithread();
    demo_realtime_signals();
    demo_sigsegv_recovery();

    printf("========== 所有演示完成 ==========\n");
    return 0;
}
