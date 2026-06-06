/**
 * @file 02_deep_dive_file_advanced.c
 * @brief 文件操作高级主题
 * @description 对应文档: 15-文件操作
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

static const char *TEST_FILE = "advanced_test.dat";

static void create_test_file(const char *path, const char *content) {
    FILE *f = fopen(path, "w");
    if (f) { fputs(content, f); fclose(f); }
}

void demo_buffered_io(void) {
    printf("=== 缓冲I/O vs 无缓冲I/O ===\n");
    printf("  标准库(fopen等): 缓冲I/O, 数据先写入缓冲区, 满了再刷到内核\n");
    printf("  系统调用(open等): 无缓冲I/O, 每次直接进入内核\n\n");

    printf("  三种缓冲模式:\n");
    printf("    _IOFBF: 全缓冲 - 缓冲区满时刷新(默认用于文件)\n");
    printf("    _IOLBF: 行缓冲 - 遇到换行符时刷新(默认用于终端)\n");
    printf("    _IONBF: 无缓冲 - 每次操作直接刷新(默认用于stderr)\n\n");
}

void demo_setvbuf(void) {
    printf("=== setvbuf 设置缓冲模式 ===\n");
    create_test_file(TEST_FILE, "");

    FILE *f = fopen(TEST_FILE, "w");
    if (!f) return;

    char buf[4096];
    if (setvbuf(f, buf, _IOFBF, sizeof(buf)) == 0) {
        printf("  设置全缓冲成功, 缓冲区大小: %zu 字节\n", sizeof(buf));
    }

    fprintf(f, "测试数据");
    printf("  数据已写入, 但可能还在缓冲区中\n");

    fflush(f);
    printf("  fflush后, 数据确保写入文件\n");

    fclose(f);
    printf("  fclose也会自动fflush\n\n");
}

void demo_fflush(void) {
    printf("=== fflush 详解 ===\n");
    printf("  fflush(fp): 将fp的输出缓冲区数据写入文件\n");
    printf("  fflush(NULL): 刷新所有打开的输出流\n\n");
    printf("  陷阱:\n");
    printf("    - fflush对输入流的行为是未定义的!\n");
    printf("    - fflush不保证数据写入磁盘(可能还在OS缓存)\n");
    printf("    - 要确保写入磁盘, 需要fsync()(POSIX)\n\n");
}

void demo_file_locking(void) {
    printf("=== 文件锁定 ===\n");
    printf("  为什么需要文件锁?\n");
    printf("    多个进程同时写同一文件可能导致数据混乱\n\n");
    printf("  POSIX文件锁:\n");
    printf("    flock():       BSD风格, 整文件锁\n");
    printf("    fcntl():       POSIX风格, 可锁文件区域\n");
    printf("    lockf():       fcntl的简化接口\n\n");
    printf("  Windows文件锁:\n");
    printf("    LockFile()/UnlockFile()\n");
    printf("    LockFileEx()/UnlockFileEx()\n\n");
    printf("  建议:\n");
    printf("    - 短时间操作用排他锁\n");
    printf("    - 长时间操作考虑记录级锁\n");
    printf("    - 注意死锁: 按相同顺序加锁\n\n");
}

void demo_mmap_concept(void) {
    printf("=== 内存映射文件(概念) ===\n");
    printf("  mmap(POSIX): 将文件映射到内存地址空间\n");
    printf("  优点:\n");
    printf("    - 避免read/write系统调用开销\n");
    printf("    - 利用操作系统的页面缓存\n");
    printf("    - 适合大文件随机访问\n");
    printf("    - 可实现进程间共享内存\n\n");
    printf("  缺点:\n");
    printf("    - 地址空间占用(32位系统受限)\n");
    printf("    - 错误处理复杂(SIGSEGV)\n");
    printf("    - 不适合顺序小量读写\n\n");
    printf("  Windows等价: CreateFileMapping/MapViewOfFile\n\n");
}

void demo_file_error_handling(void) {
    printf("=== 文件操作错误处理最佳实践 ===\n");

    printf("  1. 始终检查fopen返回值\n");
    FILE *f = fopen("__no_file__", "r");
    if (!f) {
        printf("    fopen失败: %s\n", strerror(errno));
    }

    printf("  2. 检查fwrite/fread的返回值\n");
    create_test_file(TEST_FILE, "test");
    f = fopen(TEST_FILE, "r");
    if (f) {
        char buf[10];
        size_t n = fread(buf, 1, sizeof(buf), f);
        if (n < sizeof(buf)) {
            if (feof(f)) printf("    到达文件末尾, 读取%zu字节\n", n);
            if (ferror(f)) printf("    读取错误!\n");
        }
        fclose(f);
    }

    printf("  3. 使用goto模式统一清理资源\n");
    printf("  4. 写入后检查fclose返回值(可能刷新失败)\n");
    printf("  5. 临时文件使用后确保删除\n\n");
}

void demo_file_io_performance(void) {
    printf("=== 文件I/O性能建议 ===\n");
    printf("  1. 使用合适的缓冲区大小(4KB-64KB)\n");
    printf("  2. 批量读写代替逐字节操作\n");
    printf("  3. 减少fseek次数(顺序访问优于随机访问)\n");
    printf("  4. 大文件考虑内存映射(mmap)\n");
    printf("  5. 避免频繁open/close\n");
    printf("  6. 二进制模式比文本模式快(无换行转换)\n");
    printf("  7. setvbuf增大缓冲区可提升小量写入性能\n\n");
}

int main(void) {
    printf("========== 文件操作高级主题 ==========\n\n");

    demo_buffered_io();
    demo_setvbuf();
    demo_fflush();
    demo_file_locking();
    demo_mmap_concept();
    demo_file_error_handling();
    demo_file_io_performance();

    remove(TEST_FILE);
    printf("========== 所有演示完成 ==========\n");
    return 0;
}
