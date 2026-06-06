/**
 * @file 02_deep_dive_pattern_selection.c
 * @brief 模式选择: 何时用哪种模式、模式组合、反模式
 * @description 对应文档: 30-C语言设计模式实现
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void demo_pattern_selection_guide(void) {
    printf("\n=== demo_pattern_selection_guide ===\n");
    printf("何时选择哪种设计模式?\n\n");

    printf("创建型模式:\n");
    printf("  单例:   需要全局唯一实例(配置、日志、连接池)\n");
    printf("  工厂:   创建逻辑复杂, 或需要运行时选择类型\n");
    printf("  建造者: 构建步骤多, 需要分步创建复杂对象\n\n");

    printf("结构型模式:\n");
    printf("  装饰器: 动态添加功能, 不修改原始对象\n");
    printf("  适配器: 接口不兼容, 需要转换\n");
    printf("  外观:   简化复杂子系统的接口\n");
    printf("  代理:   控制访问, 延迟加载, 远程代理\n\n");

    printf("行为型模式:\n");
    printf("  策略:   算法族, 运行时切换\n");
    printf("  观察者: 一对多依赖, 状态变化通知\n");
    printf("  命令:   操作对象化, 撤销/重做\n");
    printf("  状态:   行为随状态变化\n");
    printf("  模板方法: 固定算法骨架, 子类实现步骤\n\n");

    printf("选择原则:\n");
    printf("  1. 不要为了用模式而用模式\n");
    printf("  2. 先有问题, 再找模式\n");
    printf("  3. 简单方案优先\n");
    printf("  4. 模式是工具, 不是目标\n");
}

typedef struct {
    const char *name;
    void (*process)(const void *data, int size);
} Handler;

typedef struct {
    Handler *handler;
    void (*filter)(void *data, int size);
} FilterHandler;

static void __attribute__((used)) base_process(const void *data, int size) {
    const int *arr = (const int *)data;
    printf("  处理数据: ");
    for (int i = 0; i < size; i++) printf("%d ", arr[i]);
    printf("\n");
}

static void __attribute__((used)) validation_filter(void *data, int size) {
    int *arr = (int *)data;
    for (int i = 0; i < size; i++) {
        if (arr[i] < 0) arr[i] = 0;
    }
    printf("  [验证] 负值置零\n");
}

static void __attribute__((used)) logging_filter(void *data, int size) {
    (void)data;
    printf("  [日志] 处理 %d 个元素\n", size);
}

typedef struct {
    Handler base;
    Handler *wrapped;
    void (*filter)(void *data, int size);
} DecoratedHandler;

void demo_pattern_combination(void) {
    printf("\n=== demo_pattern_combination ===\n");
    printf("模式组合: 多个模式协同工作\n\n");

    printf("组合1: 工厂 + 策略\n");
    printf("  工厂根据配置创建不同的策略对象\n");
    printf("  如: CompressionFactory -> RLE/LZW/None策略\n\n");

    printf("组合2: 观察者 + 状态\n");
    printf("  状态变化时通知观察者\n");
    printf("  如: 网络连接状态变化 -> 通知UI更新\n\n");

    printf("组合3: 命令 + 策略\n");
    printf("  命令中使用策略选择执行方式\n");
    printf("  如: 排序命令 -> 选择排序策略\n\n");

    printf("组合4: 单例 + 工厂\n");
    printf("  工厂本身是单例, 管理对象创建\n");
    printf("  如: 全局唯一的连接池工厂\n\n");

    printf("组合5: 装饰器 + 策略\n");
    printf("  装饰器动态添加功能, 策略选择核心算法\n");
    printf("  如: 数据处理管道 -> 过滤装饰器 + 处理策略\n");
}

void demo_anti_patterns(void) {
    printf("\n=== demo_anti_patterns ===\n");
    printf("C语言常见反模式(Anti-Pattern):\n\n");

    printf("1. 上帝对象(God Object):\n");
    printf("   一个结构体/模块承担所有职责\n");
    printf("   反例: struct System { 所有数据; 所有函数指针; };\n");
    printf("   正解: 拆分为多个职责单一的对象\n\n");

    printf("2. 魔数(Magic Numbers):\n");
    printf("   代码中直接使用未命名的常量\n");
    printf("   反例: if (status == 3) ...\n");
    printf("   正解: enum { STATUS_ERROR = 3 }; if (status == STATUS_ERROR)\n\n");

    printf("3. 全局状态(Global State):\n");
    printf("   过多全局变量, 隐式依赖\n");
    printf("   反例: 10个全局变量在5个函数间传递状态\n");
    printf("   正解: 封装为结构体, 显式传递\n\n");

    printf("4. 过度回调(Callback Hell):\n");
    printf("   嵌套回调导致代码难以阅读\n");
    printf("   反例: async_op1(cb1 -> async_op2(cb2 -> async_op3(cb3)))\n");
    printf("   正解: 状态机, 事件循环, 协程\n\n");

    printf("5. 不检查返回值:\n");
    printf("   忽略malloc/fopen等函数的失败\n");
    printf("   反例: ptr = malloc(size); ptr[0] = 1; // 可能NULL!\n");
    printf("   正解: if (!ptr) { 错误处理 }\n\n");

    printf("6. 内存泄漏:\n");
    printf("   malloc后忘记free, 或逻辑分支跳过free\n");
    printf("   反例: 多个return路径, 部分路径未free\n");
    printf("   正解: goto cleanup模式, 或RAII模拟\n\n");

    printf("7. 过度抽象:\n");
    printf("   简单问题复杂化, 过早抽象\n");
    printf("   反例: 3行代码用5层间接调用\n");
    printf("   正解: 三次重复后再抽象\n");
}

void demo_c_pattern_principles(void) {
    printf("\n=== demo_c_pattern_principles ===\n");
    printf("C语言设计模式原则:\n\n");

    printf("SOLID原则的C语言解读:\n\n");

    printf("S - 单一职责:\n");
    printf("   每个结构体/模块只负责一件事\n");
    printf("   struct Logger 只负责日志, 不管网络\n\n");

    printf("O - 开放封闭:\n");
    printf("   对扩展开放, 对修改封闭\n");
    printf("   新增类型通过注册, 不修改已有代码\n\n");

    printf("L - 里氏替换:\n");
    printf("   子类型可以替换父类型\n");
    printf("   任何Shape*可以传给接受Shape*的函数\n\n");

    printf("I - 接口隔离:\n");
    printf("   不要强迫依赖不需要的接口\n");
    printf("   分离Drawable和Serializable接口\n\n");

    printf("D - 依赖倒置:\n");
    printf("   依赖抽象, 不依赖具体\n");
    printf("   依赖函数指针(接口), 不依赖具体函数\n\n");

    printf("C语言特有的模式技巧:\n");
    printf("  1. 不透明指针: 实现封装和信息隐藏\n");
    printf("  2. 函数指针: 实现多态和策略模式\n");
    printf("  3. 结构体嵌入: 实现继承和组合\n");
    printf("  4. 宏生成代码: 实现泛型(谨慎使用)\n");
    printf("  5. goto cleanup: 实现资源安全释放\n");
    printf("  6. 位域+联合: 实现内存高效布局\n");
}

void demo_pattern_summary(void) {
    printf("\n=== demo_pattern_summary ===\n");
    printf("设计模式总结:\n\n");

    printf("本教程涉及的C语言设计模式:\n\n");

    printf("创建型:\n");
    printf("  单例(Singleton)     - 全局唯一实例\n");
    printf("  工厂(Factory)       - 按类型创建对象\n");
    printf("  抽象工厂(Abstract)  - 创建一系列相关对象\n\n");

    printf("结构型:\n");
    printf("  装饰器(Decorator)   - 动态添加功能\n");
    printf("  适配器(Adapter)     - 接口转换\n");
    printf("  外观(Facade)        - 简化接口\n\n");

    printf("行为型:\n");
    printf("  策略(Strategy)      - 算法族切换\n");
    printf("  观察者(Observer)    - 一对多通知\n");
    printf("  命令(Command)       - 操作对象化\n");
    printf("  状态(State)         - 行为随状态变\n\n");

    printf("C语言实现核心:\n");
    printf("  函数指针 = 多态\n");
    printf("  结构体嵌入 = 继承\n");
    printf("  不透明指针 = 封装\n");
    printf("  回调函数 = 观察者\n");
    printf("  函数指针表 = 策略/工厂\n\n");

    printf("最终建议:\n");
    printf("  模式是经验的总结, 不是教条\n");
    printf("  理解问题比记住模式更重要\n");
    printf("  简单的代码 > 巧妙的模式\n");
    printf("  好的代码自己会说话, 不需要模式名称来解释\n");
}

int main(void) {
    printf("模式选择: 何时用哪种模式、模式组合、反模式\n");

    demo_pattern_selection_guide();
    demo_pattern_combination();
    demo_anti_patterns();
    demo_c_pattern_principles();
    demo_pattern_summary();

    printf("\n所有演示完成!\n");
    return 0;
}
