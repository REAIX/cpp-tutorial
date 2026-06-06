/**
 * @file 01_deep_dive_dynamic_lib_internals.c
 * @brief 动态库深入剖析 - GOT、PLT、延迟绑定、符号介入
 * @description 对应文档: 19-dynamic-library
 *              本文件以独立可运行代码演示动态链接的内部工作原理，
 *              包括 GOT/PLT 机制、延迟绑定、符号介入、版本控制等
 *
 * 编译: gcc 01_deep_dive_dynamic_lib_internals.c -o deep_dive_dynamic_lib
 * 运行: ./deep_dive_dynamic_lib
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* ========================================================================
 * 第一部分: 位置无关代码 (PIC) 详解
 * ======================================================================== */

/*
 * 位置无关代码 (Position Independent Code, PIC) 是动态库的核心要求。
 *
 * 为什么需要 PIC?
 *   - 动态库在进程地址空间中的加载位置在编译时未知
 *   - 多个进程可能将同一个 .so 映射到不同的虚拟地址
 *   - PIC 使得代码不依赖绝对地址，可以在任意位置执行
 *
 * PIC 的实现方式:
 *   1. 函数调用: 使用相对跳转 (x86 的 call 指令本身就是相对的)
 *   2. 全局变量访问: 通过 GOT (全局偏移表) 间接访问
 *   3. 全局函数调用: 通过 PLT (过程链接表) 间接调用
 *
 * -fPIC vs -fpic:
 *   -fPIC: 生成更大的 GOT，但兼容性更好（推荐）
 *   -fpic: 生成更小的 GOT，但在某些平台上有限制
 */

static int local_data = 42;

void demo_pic_concept(void) {
    printf("===== 位置无关代码 (PIC) 详解 =====\n");
    printf("普通代码 vs PIC 代码的区别:\n\n");

    printf("普通代码访问全局变量:\n");
    printf("  mov eax, [0x400800]    # 直接使用绝对地址\n");
    printf("  问题: 如果库被加载到不同地址，0x400800 可能不正确\n\n");

    printf("PIC 代码访问全局变量:\n");
    printf("  mov ebx, [GOT + offset]  # 通过 GOT 间接访问\n");
    printf("  GOT 在加载时由动态链接器填充正确地址\n\n");

    printf("本地变量 (local_data = %d) 的访问:\n", local_data);
    printf("  static 变量可以用 RIP 相对寻址，不需要 GOT\n");
    printf("  非 static 全局变量必须通过 GOT 访问\n\n");

    printf("-fPIC vs -fpic:\n");
    printf("  -fPIC: 始终安全，GOT 可无限大小，推荐使用\n");
    printf("  -fpic: GOT 大小有限(如 x86 上 64K)，某些情况不够用\n");
    printf("\n");
}

/* ========================================================================
 * 第二部分: GOT (全局偏移表) 详解
 * ======================================================================== */

/*
 * GOT (Global Offset Table) 是动态链接的核心数据结构。
 *
 * GOT 的位置:
 *   - 位于 .got 段（已初始化数据）或 .got.plt 段
 *   - 在数据段中，运行时可写
 *
 * GOT 的内容:
 *   - 每个全局变量/函数占一个条目 (8字节 on x86-64)
 *   - 条目存储全局符号的实际地址
 *   - 初始时，条目可能为0或指向 PLT 的延迟绑定桩
 *
 * 工作流程:
 *   1. 编译时: 编译器生成通过 GOT 间接访问的代码
 *   2. 加载时: 动态链接器填充 GOT 中的地址
 *   3. 运行时: 代码通过 GOT 间接访问全局变量
 *
 * 安全考虑:
 *   - GOT 可写是一个安全风险（GOT 覆盖攻击）
 *   - RELRO (Relocation Read-Only) 技术可以在绑定后将 GOT 设为只读
 *   - -Wl,-z,relro: 延迟绑定后标记为只读
 *   - -Wl,-z,now: 立即绑定（完全 RELRO）
 */

void demo_got_internals(void) {
    printf("===== GOT (全局偏移表) 详解 =====\n");
    printf("GOT 的内存布局:\n\n");
    printf("  高地址\n");
    printf("  ┌──────────────────┐\n");
    printf("  │ .got.plt[0]      │ ← 动态链接器地址 (_dl_runtime_resolve)\n");
    printf("  │ .got.plt[1]      │ ← link_map 地址\n");
    printf("  │ .got.plt[2]      │ ← _dl_runtime_resolve 参数\n");
    printf("  │ .got.plt[3]      │ ← printf 的真实地址 (延迟绑定后填入)\n");
    printf("  │ .got.plt[4]      │ ← malloc 的真实地址\n");
    printf("  │ ...              │\n");
    printf("  └──────────────────┘\n");
    printf("  低地址\n\n");

    printf("GOT 访问过程:\n");
    printf("  1. 代码需要访问全局变量 var\n");
    printf("  2. 执行: mov rax, [GOT + offset_of_var]\n");
    printf("  3. rax 中得到 var 的实际地址\n");
    printf("  4. 通过 rax 间接访问 var\n\n");

    printf("查看 GOT 的命令:\n");
    printf("  objdump -d -j .got.plt ./program   # 反汇编 GOT.PLT 段\n");
    printf("  readelf -r ./program                # 查看重定位条目\n");
    printf("  readelf -S ./program                # 查看所有段\n\n");

    printf("安全: RELRO 机制\n");
    printf("  Partial RELRO: -Wl,-z,relro\n");
    printf("    .got 设为只读, .got.plt 仍可写(延迟绑定需要)\n");
    printf("  Full RELRO: -Wl,-z,relro,-z,now\n");
    printf("    立即绑定所有符号, 整个 GOT 设为只读\n");
    printf("    防止 GOT 覆盖攻击, 但启动稍慢\n\n");
}

/* ========================================================================
 * 第三部分: PLT (过程链接表) 与延迟绑定
 * ======================================================================== */

/*
 * PLT (Procedure Linkage Table) 实现了函数调用的延迟绑定。
 *
 * 延迟绑定 (Lazy Binding):
 *   - 函数地址在第一次被调用时才解析，而非程序启动时
 *   - 优点: 加快程序启动速度（不解析未使用的函数）
 *   - 缺点: 首次调用有额外开销；不利于安全（GOT 可写）
 *
 * PLT 的工作流程 (以调用 printf 为例):
 *
 *   1. 代码调用 printf@plt:
 *      call printf@plt
 *
 *   2. PLT 条目执行:
 *      printf@plt:
 *        jmp *GOT[printf]        # 第一次: GOT[printf] 指向下一行
 *        push $reloc_offset      # 压入重定位偏移
 *        jmp PLT[0]              # 跳转到动态链接器
 *
 *   3. 动态链接器解析:
 *      _dl_runtime_resolve:
 *        查找 printf 的真实地址
 *        将真实地址写入 GOT[printf]
 *        跳转到 printf 执行
 *
 *   4. 后续调用:
 *      GOT[printf] 已有真实地址
 *      jmp *GOT[printf] 直接跳转，无需再次解析
 *
 * 禁用延迟绑定:
 *   - 环境变量: LD_BIND_NOW=1 ./program
 *   - 编译选项: -Wl,-z,now
 */

void demo_plt_lazy_binding(void) {
    printf("===== PLT 与延迟绑定详解 =====\n");
    printf("PLT 条目的汇编结构:\n\n");
    printf("  printf@plt:\n");
    printf("    jmp *GOT[printf_offset]     # 间接跳转到 GOT 中的地址\n");
    printf("    push $reloc_index           # 压入重定位索引\n");
    printf("    jmp resolver_stub           # 跳转到解析器\n\n");

    printf("延迟绑定的完整流程:\n");
    printf("  ┌─────────────────────────────────────────────────────┐\n");
    printf("  │ 第一次调用 printf:                                    │\n");
    printf("  │   call printf@plt                                    │\n");
    printf("  │   → jmp *GOT[printf]  (GOT中存的是下一条指令地址)     │\n");
    printf("  │   → push reloc_index                                 │\n");
    printf("  │   → jmp _dl_runtime_resolve                          │\n");
    printf("  │   → 解析 printf 真实地址，写入 GOT[printf]            │\n");
    printf("  │   → 跳转到 printf 执行                               │\n");
    printf("  ├─────────────────────────────────────────────────────┤\n");
    printf("  │ 后续调用 printf:                                      │\n");
    printf("  │   call printf@plt                                    │\n");
    printf("  │   → jmp *GOT[printf]  (GOT中已是真实地址)             │\n");
    printf("  │   → 直接跳转到 printf 执行，无额外开销                │\n");
    printf("  └─────────────────────────────────────────────────────┘\n\n");

    printf("延迟绑定的利弊:\n");
    printf("  优点:\n");
    printf("    - 加快程序启动（只解析实际用到的函数）\n");
    printf("    - 减少动态链接器工作量\n");
    printf("  缺点:\n");
    printf("    - 首次调用有微秒级延迟\n");
    printf("    - GOT.PLT 可写，存在安全风险\n");
    printf("    - 运行时可能触发链接错误（而非启动时）\n\n");

    printf("查看 PLT 的命令:\n");
    printf("  objdump -d -j .plt ./program        # 反汇编 PLT 段\n");
    printf("  objdump -d -j .plt.sec ./program     # 安全 PLT (如果存在)\n\n");

    printf("禁用延迟绑定:\n");
    printf("  LD_BIND_NOW=1 ./program              # 运行时禁用\n");
    printf("  gcc -Wl,-z,now ...                   # 编译时禁用\n");
    printf("\n");
}

/* ========================================================================
 * 第四部分: 符号介入 (Symbol Interposition)
 * ======================================================================== */

/*
 * 符号介入是动态链接中一个强大但容易出错的特性。
 *
 * 当多个共享库和可执行文件定义了同名符号时，动态链接器
 * 会选择"最前面"的那个（基于加载顺序）。
 *
 * 符号查找顺序:
 *   1. 可执行文件中的全局符号
 *   2. 按加载顺序排列的共享库中的全局符号
 *   3. LD_PRELOAD 指定的库中的符号（优先级最高）
 *
 * 这意味着:
 *   - 可执行文件中的符号会覆盖共享库中的同名符号
 *   - LD_PRELOAD 可以覆盖任何符号
 *   - 共享库之间的符号可能互相覆盖
 *
 * 应用场景:
 *   - malloc/free 的调试版本 (如 libtcmalloc, libjemalloc)
 *   - 函数拦截和追踪
 *   - 兼容性补丁
 *
 * 风险:
 *   - 意外的符号覆盖导致难以调试的bug
 *   - 破坏库的内部假设
 */

static int my_malloc_count = 0;

void *my_malloc_wrapper(size_t size) {
    my_malloc_count++;
    printf("  [interpose] malloc(%zu) called (#%d)\n", size, my_malloc_count);
    return malloc(size);
}

void demo_symbol_interposition(void) {
    printf("===== 符号介入 (Symbol Interposition) 详解 =====\n");
    printf("符号查找顺序:\n");
    printf("  1. LD_PRELOAD 指定的库 (最高优先级)\n");
    printf("  2. 可执行文件中的全局符号\n");
    printf("  3. 按加载顺序的共享库中的全局符号\n\n");

    printf("LD_PRELOAD 用法:\n");
    printf("  # 预加载自定义 malloc 实现\n");
    printf("  LD_PRELOAD=libtcmalloc.so ./my_program\n\n");

    printf("  # 预加载调试库\n");
    printf("  LD_PRELOAD=libefence.so ./my_program\n\n");

    printf("符号介入的模拟演示:\n");
    printf("  (本文件中 my_malloc_wrapper 模拟了介入行为)\n");
    void *p = my_malloc_wrapper(128);
    printf("  分配结果: %p\n", p);
    free(p);
    printf("\n");

    printf("符号介入的风险:\n");
    printf("  - 意外覆盖导致隐蔽的 bug\n");
    printf("  - 库内部调用也可能被介入（递归风险）\n");
    printf("  - 破坏 ABI 兼容性\n\n");

    printf("避免意外介入:\n");
    printf("  - 使用 static 限制符号可见性\n");
    printf("  - 使用 -fvisibility=hidden 编译选项\n");
    printf("  - 使用 __attribute__((visibility(\"hidden\")))\n");
    printf("  - 库内部调用使用 -Bsymbolic 或 -Bsymbolic-functions\n\n");

    printf("举一反三 - 符号介入的实际应用:\n");
    printf("  1. 内存调试: LD_PRELOAD=libefence.so 检测越界\n");
    printf("  2. 性能分析: 介入函数记录调用次数和耗时\n");
    printf("  3. 沙箱机制: 介入系统调用实现安全隔离\n");
    printf("  4. 兼容层: 介入旧 API 转发到新实现\n");
    printf("\n");
}

/* ========================================================================
 * 第五部分: 动态库版本控制 (SONAME)
 * ======================================================================== */

void demo_soname_versioning(void) {
    printf("===== 动态库版本控制 (SONAME) =====\n");
    printf("SONAME 机制解决了动态库的兼容性问题:\n\n");

    printf("版本命名约定:\n");
    printf("  libfoo.so        → 符号链接 → libfoo.so.3\n");
    printf("  libfoo.so.3      → 符号链接 → libfoo.so.3.1.2\n");
    printf("  libfoo.so.3.1.2  → 实际文件 (主版本.次版本.补丁)\n\n");

    printf("SONAME 的含义:\n");
    printf("  主版本号变化 → 不兼容的 API 变更\n");
    printf("  次版本号变化 → 向后兼容的新增功能\n");
    printf("  补丁号变化   → bug 修复，完全兼容\n\n");

    printf("设置 SONAME:\n");
    printf("  gcc -shared -Wl,-soname,libfoo.so.3 -o libfoo.so.3.1.2 foo.c\n\n");

    printf("查看 SONAME:\n");
    printf("  readelf -d libfoo.so.3.1.2 | grep SONAME\n");
    printf("  objdump -p libfoo.so.3.1.2 | grep SONAME\n\n");

    printf("安装动态库的标准流程:\n");
    printf("  gcc -fPIC -shared -Wl,-soname,libfoo.so.3 \\\n");
    printf("      -o libfoo.so.3.1.2 foo.c\n");
    printf("  ln -sf libfoo.so.3.1.2 libfoo.so.3\n");
    printf("  ln -sf libfoo.so.3 libfoo.so\n");
    printf("  cp libfoo.so.3.1.2 /usr/local/lib/\n");
    printf("  ldconfig\n\n");
}

/* ========================================================================
 * 第六部分: 实用技巧与常见陷阱
 * ======================================================================== */

void demo_tips_and_pitfalls(void) {
    printf("===== 实用技巧与常见陷阱 =====\n\n");

    printf("陷阱1: 运行时找不到动态库\n");
    printf("  错误: error while loading shared libraries: libfoo.so: cannot open\n");
    printf("  解决:\n");
    printf("    - 临时: export LD_LIBRARY_PATH=/path/to/lib:$LD_LIBRARY_PATH\n");
    printf("    - 永久: 将路径加入 /etc/ld.so.conf 并运行 ldconfig\n");
    printf("    - 编译: gcc -Wl,-rpath,/path/to/lib ...\n\n");

    printf("陷阱2: 编译时找不到动态库\n");
    printf("  错误: cannot find -lfoo\n");
    printf("  解决:\n");
    printf("    - 使用 -L 指定库搜索路径: gcc -L/path/to/lib -lfoo\n");
    printf("    - 设置 LIBRARY_PATH 环境变量\n\n");

    printf("陷阱3: -fPIC 遗漏\n");
    printf("  错误: relocation R_X86_64_32 against `.rodata' can not be used\n");
    printf("  解决: 编译 .o 时必须加 -fPIC\n\n");

    printf("陷阱4: 混用静态库和动态库\n");
    printf("  问题: 同一个库既有 .a 又有 .so，链接器默认选 .so\n");
    printf("  强制静态链接: gcc -static ... 或 gcc -L. -l:libfoo.a ...\n\n");

    printf("技巧1: 查看程序运行时加载了哪些库\n");
    printf("  ldd ./program\n");
    printf("  LD_DEBUG=libs ./program    # 详细加载过程\n\n");

    printf("技巧2: 查看动态库导出了哪些符号\n");
    printf("  nm -D libfoo.so\n");
    printf("  readelf -s libfoo.so | grep FUNC\n\n");

    printf("技巧3: 检查动态库是否为 PIC\n");
    printf("  readelf -d libfoo.so | grep TEXTREL\n");
    printf("  如果有 TEXTREL 条目，说明不是 PIC（有问题）\n\n");

    printf("技巧4: LD_DEBUG 调试动态链接\n");
    printf("  LD_DEBUG=help ./program    # 查看可用选项\n");
    printf("  LD_DEBUG=bindings ./program # 查看符号绑定\n");
    printf("  LD_DEBUG=versions ./program # 查看版本解析\n");
    printf("\n");
}

int main(void) {
    printf("================================================\n");
    printf("  动态库深入剖析 - GOT/PLT/延迟绑定/符号介入\n");
    printf("================================================\n\n");

    demo_pic_concept();
    demo_got_internals();
    demo_plt_lazy_binding();
    demo_symbol_interposition();
    demo_soname_versioning();
    demo_tips_and_pitfalls();

    printf("================================================\n");
    printf("  演示结束\n");
    printf("================================================\n");
    return 0;
}
