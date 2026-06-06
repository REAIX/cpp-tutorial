/**
 * @file plugin_math.c
 * @brief dlopen示例 - 数学运算插件实现
 * @description 对应文档: 20-dynamic-loading
 *              实现一个数学运算插件，编译为动态库后由主程序通过 dlopen 加载
 *
 * 编译为动态库:
 *   gcc -fPIC -shared -o plugin_math.so plugin_math.c
 *
 * 注意: 插件必须导出 plugin_create 函数作为入口点，
 *       主程序通过 dlsym 查找此函数来获取插件接口
 */

#include <stdio.h>
#include "plugin.h"

static int math_init(void) {
    printf("  [plugin_math] 初始化完成\n");
    return 0;
}

static void math_cleanup(void) {
    printf("  [plugin_math] 清理完成\n");
}

static int math_execute(int a, int b) {
    printf("  [plugin_math] 执行运算: %d + %d = %d, %d * %d = %d\n",
           a, b, a + b, a, b, a * b);
    return a + b;
}

static plugin_interface_t math_plugin = {
    .name = "math",
    .version = "1.0.0",
    .init = math_init,
    .cleanup = math_cleanup,
    .execute = math_execute
};

plugin_interface_t *plugin_create(void) {
    return &math_plugin;
}
