/**
 * @file main.c
 * @brief 动态库示例 - 使用动态库的主程序
 * @description 对应文档: 19-dynamic-library
 *              演示如何链接并使用动态库 libgreetlib.so
 *
 * 编译与运行步骤:
 *   1. 先编译动态库:
 *      gcc -fPIC -shared -o libgreetlib.so greetlib.c
 *
 *   2. 编译主程序并链接动态库:
 *      gcc main.c -L. -lgreetlib -o main
 *
 *   3. 运行时需要设置库搜索路径:
 *      export LD_LIBRARY_PATH=.:$LD_LIBRARY_PATH
 *      ./main
 *
 *   或使用 -rpath 在编译时嵌入搜索路径:
 *      gcc main.c -L. -lgreetlib -Wl,-rpath,. -o main
 *
 * LD_LIBRARY_PATH 说明:
 *   运行时动态链接器搜索共享库的路径列表
 *   类似于 PATH 环境变量，但用于 .so 文件
 *
 * 其他查找动态库的方式:
 *   1. -Wl,-rpath,<path>  编译时嵌入运行时搜索路径
 *   2. /etc/ld.so.conf     系统级库搜索配置
 *   3. ldconfig            更新动态链接器缓存
 *   4. DT_RPATH / DT_RUNPATH  ELF 文件中的嵌入路径
 */

#include <stdio.h>
#include "greetlib.h"

void demo_basic_greeting(void) {
    printf("===== 基本问候演示 =====\n");
    greet_hello("Alice");
    greet_hello("Bob");
    greet_hello(NULL);
    printf("\n");
}

void demo_goodbye(void) {
    printf("===== 告别演示 =====\n");
    greet_goodbye("Alice");
    greet_goodbye(NULL);
    printf("\n");
}

void demo_time_greeting(void) {
    printf("===== 时段问候演示 =====\n");
    int hours[] = {3, 8, 14, 20, 25};
    for (int i = 0; i < 5; i++) {
        greet_time_of_day(hours[i]);
    }
    printf("\n");
}

void demo_version_info(void) {
    printf("===== 版本信息 =====\n");
    printf("greetlib 版本: %s\n", greet_get_version());
    printf("\n");
}

void demo_dynamic_lib_concept(void) {
    printf("===== 动态库概念说明 =====\n");
    printf("动态库 (.so / .dll) 的特点:\n");
    printf("  1. 运行时链接: 程序启动时由动态链接器加载\n");
    printf("  2. 代码共享: 多个进程共享同一份库的内存映射\n");
    printf("  3. 体积较小: 可执行文件不包含库代码\n");
    printf("  4. 便于更新: 替换 .so 文件即可更新，无需重新编译\n");
    printf("  5. 启动稍慢: 需要运行时符号解析和重定位\n");
    printf("\n");

    printf("创建动态库的命令:\n");
    printf("  gcc -fPIC -c greetlib.c -o greetlib.o\n");
    printf("  gcc -shared -o libgreetlib.so greetlib.o\n");
    printf("  或一步完成:\n");
    printf("  gcc -fPIC -shared -o libgreetlib.so greetlib.c\n");
    printf("\n");

    printf("运行时查找动态库的顺序:\n");
    printf("  1. DT_RPATH (编译时 -Wl,-rpath 指定，已弃用)\n");
    printf("  2. LD_LIBRARY_PATH 环境变量\n");
    printf("  3. DT_RUNPATH (编译时 -Wl,-rpath 指定，新版)\n");
    printf("  4. /etc/ld.so.cache (ldconfig 缓存)\n");
    printf("  5. 默认路径 /lib, /usr/lib 等\n");
    printf("\n");

    printf("查看动态库依赖:\n");
    printf("  ldd ./main              # 查看程序依赖的动态库\n");
    printf("  ldd libgreetlib.so      # 查看库本身的依赖\n");
    printf("\n");
}

int main(void) {
    printf("========== 动态库使用示例 ==========\n\n");

    demo_basic_greeting();
    demo_goodbye();
    demo_time_greeting();
    demo_version_info();
    demo_dynamic_lib_concept();

    printf("========== 程序结束 ==========\n");
    return 0;
}
