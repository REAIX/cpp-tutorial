# GCC/G++ 编译器深度使用指南

> **前置阅读**：如果你还不熟悉GCC/G++的基本用法，请先阅读 [GCC编译器基础](./07-GCC编译器基础.md)。本文档假设你已经掌握了GCC的基本编译命令，需要深入学习优化和链接参数。

## 1. 常用编译参数完全指南

### 1. 优化参数

GCC提供多个优化级别，每个级别启用不同的优化选项集合：

```bash
# -O0：无优化（默认级别，调试用）
g++ -O0 main.cpp -o main
# 代码与源码几乎一一对应，便于调试

# -O1：基本优化
g++ -O1 main.cpp -o main
# 启用的优化：-fdefer-pop, -foptimize-sibling-calls, -finline-functions-called-once 等

# -O2：推荐发布优化
g++ -O2 main.cpp -o main
# 在-O1基础上增加：-floop-optimize, -finline-small-functions, -fregmove 等
# 不会增加代码大小，不会改变严格合规行为

# -O3：激进优化
g++ -O3 main.cpp -o main
# 在-O2基础上增加：-finline-functions, -funswitch-loops, -fgcse-after-reload 等
# 可能增大代码体积，可能改变浮点精度行为

# -Os：优化代码大小
g++ -Os main.cpp -o main
# 在-O2基础上禁用增加代码大小的优化
# 适用于嵌入式系统、磁盘/缓存敏感场景

# -Og：调试优化（GCC 4.8+）
g++ -Og main.cpp -o main
# 在保持快速编译和良好调试体验的前提下进行优化
# 推荐替代-O0用于调试

# -Ofast：最快优化（可能违反标准）
g++ -Ofast main.cpp -o main
# 在-O3基础上增加：-ffast-math
# -ffast-math允许：不遵循IEEE 754、假设无NaN/Inf、允许重排浮点运算
```

各级别具体优化项差异查看：

```bash
# 查看-O2启用了哪些优化选项
g++ -O2 -Q --help=optimizers

# 对比-O2和-O3的优化差异
diff <(g++ -O2 -Q --help=optimizers) <(g++ -O3 -Q --help=optimizers)

# 查看某个优化级别是否启用了特定选项
g++ -O2 -Q --help=optimizers | grep inline
```

优化级别选择建议：

| 场景 | 推荐级别 | 原因 |
|------|----------|------|
| 开发调试 | -O0 或 -Og | 调试信息准确，变量可观察 |
| 性能测试 | -O2 | 平衡性能与正确性 |
| 最终发布 | -O2 或 -O3 | O3需验证正确性 |
| 嵌入式/资源受限 | -Os | 减小代码体积 |
| 科学计算（不关心精度） | -Ofast | 最快速度，但浮点精度不保证 |

### 2. 链接参数

```bash
# -l：链接指定库（去掉前缀lib和后缀）
g++ main.cpp -lpthread -o main     # 链接libpthread.so/libpthread.a
g++ main.cpp -lm -o main           # 链接libm.so（数学库）
g++ main.cpp -lstdc++ -o main      # 链接libstdc++.so（C++标准库）
g++ main.cpp -lcurl -o main        # 链接libcurl.so/libcurl.a

# -L：添加库文件搜索路径
g++ main.cpp -L/usr/local/lib -lmylib -o main
# 编译器会在/usr/local/lib目录下搜索库文件

# -I：添加头文件搜索路径
g++ main.cpp -I/usr/local/include -o main
# 编译器会在/usr/local/include目录下搜索头文件

# -static：完全静态链接
g++ -static main.cpp -o main
# 所有库都静态链接，生成的可执行文件不依赖动态库
# 优点：可移植性好；缺点：文件体积大

# -shared：生成动态共享库
g++ -shared -fPIC mylib.cpp -o libmylib.so
# -fPIC：生成位置无关代码（Position Independent Code），动态库必需

# -Wl,：传递选项给链接器ld
g++ main.cpp -Wl,--verbose -o main
# --verbose让链接器输出详细信息

# -Wl,-rpath：设置运行时库搜索路径
g++ main.cpp -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lmylib -o main
# 编译时在-L路径找库，运行时在-rpath路径找库
# 这两个通常需要同时设置

# -Wl,-rpath,$ORIGIN：相对于可执行文件位置的库路径
g++ main.cpp -Wl,-rpath,'$ORIGIN/lib' -L./lib -lmylib -o main
# $ORIGIN在运行时被替换为可执行文件所在目录

# 多个链接器选项
g++ main.cpp -Wl,-rpath,/usr/local/lib,--as-needed -lmylib -o main
```

静态链接 vs 动态链接对比：

```bash
# 动态链接（默认）
g++ main.cpp -o main_dynamic
ls -lh main_dynamic
# -rwxr-xr-x 1 user user 16K  main_dynamic
ldd main_dynamic
#     libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6
#     libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6

# 静态链接
g++ -static main.cpp -o main_static
ls -lh main_static
# -rwxr-xr-x 1 user user 2.0M  main_static
ldd main_static
#     not a dynamic executable
```

---

## 2. 外部库使用

### 1. 基本使用流程

```bash
# 三个关键参数
g++ main.cpp \
    -I/path/to/include \    # 头文件搜索路径
    -L/path/to/lib \        # 库文件搜索路径
    -lmylib \               # 链接libmylib.so或libmylib.a
    -o main
```

### 2. 静态库 vs 动态库的链接区别

```bash
# 创建静态库
g++ -c mylib.cpp -o mylib.o
ar rcs libmylib.a mylib.o

# 创建动态库
g++ -c -fPIC mylib.cpp -o mylib.o
g++ -shared -o libmylib.so mylib.o

# 链接静态库
g++ main.cpp -L. -lmylib -o main_static
# 静态库的代码被复制到可执行文件中

# 链接动态库
g++ main.cpp -L. -lmylib -o main_dynamic
# 动态库在运行时加载，可执行文件只包含引用

# 强制链接静态库（即使动态库存在）
g++ main.cpp -L. -static -lmylib -o main_static

# 指定链接静态或动态版本
g++ main.cpp -L. -l:libmylib.a -o main_static   # 直接指定文件名
g++ main.cpp -L. -l:libmylib.so -o main_dynamic
```

### 3. 运行时库路径

```bash
# 问题：编译成功但运行时找不到动态库
g++ main.cpp -L/usr/local/lib -lmylib -o main
./main
# 报错：error while loading shared libraries: libmylib.so: cannot open shared object file

# 解决方法1：设置LD_LIBRARY_PATH环境变量
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
./main

# 解决方法2：编译时设置-rpath（推荐）
g++ main.cpp -L/usr/local/lib -Wl,-rpath,/usr/local/lib -lmylib -o main
./main  # 正常运行

# 解决方法3：使用$ORIGIN实现相对路径
g++ main.cpp -L./lib -Wl,-rpath,'$ORIGIN/lib' -lmylib -o main
# 可执行文件和lib目录一起部署时很有用

# 解决方法4：将库路径添加到ldconfig
echo "/usr/local/lib" | sudo tee /etc/ld.so.conf.d/mylib.conf
sudo ldconfig
```

### 4. pkg-config的使用

pkg-config是一个管理库编译和链接参数的工具：

```bash
# 查看已安装的库
pkg-config --list-all

# 查看某个库的编译参数
pkg-config --cflags libcurl
# 输出：-I/usr/include/x86_64-linux-gnu

# 查看某个库的链接参数
pkg-config --libs libcurl
# 输出：-lcurl

# 同时获取编译和链接参数
pkg-config --cflags --libs libcurl
# 输出：-I/usr/include/x86_64-linux-gnu -lcurl

# 在编译命令中使用pkg-config
g++ main.cpp $(pkg-config --cflags --libs libcurl) -o main

# 查看库的版本
pkg-config --modversion libcurl
# 输出：7.88.1

# 查看库的所有变量
pkg-config --print-variables libcurl
```

### 5. 常见库的编译命令示例

```bash
# OpenSSL
g++ main.cpp $(pkg-config --cflags --libs openssl) -o main

# SQLite3
g++ main.cpp -lsqlite3 -o main

# pthread（POSIX线程）
g++ main.cpp -lpthread -o main
# 或者使用C++11的<thread>（g++自动链接）

# Boost（以filesystem为例）
g++ main.cpp -lboost_filesystem -lboost_system -o main

# SDL2
g++ main.cpp $(pkg-config --cflags --libs sdl2) -o main

# OpenGL + GLFW
g++ main.cpp -lGL -lGLEW -lglfw -o main

# protobuf
g++ main.cpp $(pkg-config --cflags --libs protobuf) -o main

# fmt库
g++ main.cpp $(pkg-config --cflags --libs fmt) -o main
```

---

## 3. 常用技巧

### 1. 查看预处理器输出

```bash
# 查看预处理结果（输出很多，用less分页）
gcc -E main.c | less

# 只看预处理后的代码（去掉系统头文件展开）
gcc -E main.c | grep -v '^#' | less

# 查看宏展开结果
gcc -E -dM main.c | grep MAX_SIZE
```

### 2. 查看汇编输出

```bash
# 生成带注释的汇编代码
gcc -S -fverbose-asm main.c -o main.s

# 生成AT&T语法（默认）
gcc -S main.c -o main_att.s

# 生成Intel语法
gcc -S -masm=intel main.c -o main_intel.s

# 同时生成汇编和C源码对照
gcc -S -fverbose-asm -g main.c -o main.s
objdump -S main.o  # 反汇编时混入源码
```

### 3. 查看所有宏定义

```bash
# 查看编译器预定义的所有宏
gcc -dM -E - < /dev/null
# 输出大量宏定义，如：
# #define __linux__ 1
# #define __x86_64__ 1
# #define __GNUC__ 13
# #define __cplusplus 201703L

# 查看特定宏的值
gcc -dM -E - < /dev/null | grep __cplusplus
# #define __cplusplus 201703L

# 查看不同标准下的宏差异
diff <(gcc -std=c++17 -dM -E - < /dev/null) <(gcc -std=c++20 -dM -E - < /dev/null)

# 查看包含头文件后新增的宏
gcc -dM -E -include <iostream> - < /dev/null | grep __cplusplus
```

### 4. 查看include搜索路径

```bash
# 查看编译器搜索头文件的路径
gcc -print-search-dirs
# 输出：
# install: /usr/lib/gcc/x86_64-linux-gnu/13/
# programs: /usr/lib/gcc/x86_64-linux-gnu/13/:...
# libraries: /usr/lib/gcc/x86_64-linux-gnu/13/:...

# 更直接地查看include路径
gcc -E -Wp,-v - < /dev/null
# 输出：
# #include <...> search starts here:
#  /usr/include/c++/13
#  /usr/include/x86_64-linux-gnu/c++/13
#  /usr/include/c++/13/backward
#  /usr/lib/gcc/x86_64-linux-gnu/13/include
#  /usr/local/include
#  /usr/include/x86_64-linux-gnu
#  /usr/include
```

### 5. 查看链接了哪些库

```bash
# 查看动态库依赖
ldd ./main
# 输出：
#     linux-vdso.so.1 (0x00007ffd8f3fe000)
#     libstdc++.so.6 => /lib/x86_64-linux-gnu/libstdc++.so.6 (0x00007f8c2a000000)
#     libm.so.6 => /lib/x86_64-linux-gnu/libm.so.6 (0x00007f8c29b1f000)
#     libgcc_s.so.1 => /lib/x86_64-linux-gnu/libgcc_s.so.1 (0x00007f8c29ae0000)
#     libc.so.6 => /lib/x86_64-linux-gnu/libc.so.6 (0x00007f8c29800000)
#     /lib64/ld-linux-x86-64.so.2 (0x00007f8c2a2a0000)

# 查看直接依赖（不含间接依赖）
ldd ./main | grep "=>"

# 查看某个库的路径
ldconfig -p | grep libstdc++
```

### 6. 查看符号表

```bash
# 查看可执行文件/目标文件的符号表
nm ./main
# 输出：
# 0000000000001149 T main
#                  U printf
# 0000000000003d40 D __data_start

# 只看已定义的符号
nm --defined-only ./main

# 只看未定义的符号（需要链接的）
nm --undefined-only ./main

# 查看C++符号（demangled，可读形式）
nm -C ./main
# 0000000000001149 T main
#                  U std::cout

# 查看动态符号
nm -D ./main
```

### 7. 查看依赖关系

```bash
# 查看ELF文件的动态段信息
objdump -p ./main
# 输出包含 NEEDED 条目，即运行时需要的动态库

# 只看需要的动态库
objdump -p ./main | grep NEEDED
# 输出：
#   NEEDED               libstdc++.so.6
#   NEEDED               libm.so.6
#   NEEDED               libgcc_s.so.1
#   NEEDED               libc.so.6

# 查看RPATH/RUNPATH
objdump -p ./main | grep -E 'RPATH|RUNPATH'
```

### 8. 生成依赖关系

```bash
# 生成源文件的头文件依赖关系（Makefile用）
gcc -MM main.c
# 输出：main.o: main.c math_utils.h config.h

# 包含系统头文件
gcc -M main.c
# 输出：main.o: main.c /usr/include/stdio.h /usr/include/math_utils.h ...

# 生成依赖文件（用于Makefile自动依赖）
gcc -MM main.c -MF main.d
# 生成main.d文件，可以被Makefile包含
```

### 9. 查看优化了什么

```bash
# 查看某个优化级别启用了哪些优化选项
gcc -O2 -Q --help=optimizers
# 输出：
#   -fdefer-pop                         [enabled]
#   -fforward-propagate                 [enabled]
#   -fguess-branch-probability          [enabled]
#   -finline-functions-called-once      [enabled]
#   ...

# 对比两个优化级别的差异
diff <(gcc -O2 -Q --help=optimizers 2>&1) <(gcc -O3 -Q --help=optimizers 2>&1)
```

### 10. 查看编译器的默认选项

```bash
# 查看编译器的specs文件（默认行为配置）
gcc -dumpspecs

# 查看默认搜索路径
gcc -print-search-dirs

# 查看默认包含的文件和宏
gcc -dM -E - < /dev/null
```

### 11. 交叉编译

```bash
# 交叉编译ARM架构
arm-linux-gnueabihf-gcc main.c -o main_arm

# 指定目标架构（如果安装了多架构支持）
gcc --target=arm-linux-gnueabihf main.c -o main_arm

# 交叉编译时指定sysroot
gcc --sysroot=/path/to/arm/sysroot main.c -o main_arm

# 查看已安装的交叉编译工具链
ls /usr/bin/*-gcc
# arm-linux-gnueabihf-gcc
# aarch64-linux-gnu-gcc
# riscv64-linux-gnu-gcc
```

---

## 4. Sanitizer 使用

Sanitizer是GCC/Clang内置的运行时错误检测工具，能在程序运行时检测各种内存和并发错误。

### 1. AddressSanitizer（ASan）—— 内存错误检测

```bash
# 启用AddressSanitizer
g++ -fsanitize=address -g main.cpp -o main
```

```cpp
// asan_demo.cpp
#include <iostream>

void use_after_free() {
    int *p = new int(42);
    delete p;
    std::cout << *p << std::endl;  // 使用已释放的内存
}

void buffer_overflow() {
    int arr[5] = {1, 2, 3, 4, 5};
    std::cout << arr[10] << std::endl;  // 数组越界访问
}

void memory_leak() {
    int *p = new int[100];  // 未释放，内存泄漏
}

int main() {
    // use_after_free();   // 取消注释测试
    // buffer_overflow();  // 取消注释测试
    memory_leak();
    return 0;
}
```

```bash
# 编译并运行
g++ -fsanitize=address -g -O1 asan_demo.cpp -o asan_demo
./asan_demo
# ASan输出示例（内存泄漏）：
# =================================================================
# ==12345==ERROR: LeakSanitizer: detected memory leaks
#
# Direct leak of 400 byte(s) in 1 object(s) allocated from:
#     #0 0x7f... in operator new[](unsigned long) (...)
#     #1 0x401234 in memory_leak() .../asan_demo.cpp:14
#     #2 0x401256 in main .../asan_demo.cpp:19
#     #3 0x7f... in __libc_start_main (...)
#
# SUMMARY: AddressSanitizer: 400 byte(s) leaked in 1 allocation(s).
```

ASan能检测的错误类型：
- 堆/栈/全局变量的越界访问
- 使用已释放的内存（use-after-free）
- 重复释放（double-free）
- 内存泄漏（需配合LeakSanitizer）

### 2. ThreadSanitizer（TSan）—— 数据竞争检测

```bash
# 启用ThreadSanitizer
g++ -fsanitize=thread -g main.cpp -o main -lpthread
```

```cpp
// tsan_demo.cpp
#include <iostream>
#include <thread>

int shared_data = 0;  // 无保护的共享变量

void increment(int times) {
    for (int i = 0; i < times; i++) {
        shared_data++;  // 数据竞争！
    }
}

int main() {
    std::thread t1(increment, 100000);
    std::thread t2(increment, 100000);

    t1.join();
    t2.join();

    std::cout << "结果: " << shared_data << std::endl;
    // 期望200000，但实际结果不确定
    return 0;
}
```

```bash
g++ -fsanitize=thread -g -O1 tsan_demo.cpp -o tsan_demo -lpthread
./tsan_demo
# TSan输出示例：
# ==================
# WARNING: ThreadSanitizer: data race (pid=12345)
#   Write of size 4 at 0x... by thread T2:
#     #0 increment() .../tsan_demo.cpp:8
#
#   Previous write of size 4 at 0x... by thread T1:
#     #0 increment() .../tsan_demo.cpp:8
#
#   Location is global 'shared_data' .../tsan_demo.cpp:5
```

### 3. UndefinedBehaviorSanitizer（UBSan）—— 未定义行为检测

```bash
# 启用UndefinedBehaviorSanitizer
g++ -fsanitize=undefined -g main.cpp -o main
```

```cpp
// ubsan_demo.cpp
#include <iostream>
#include <climits>

void signed_overflow() {
    int x = INT_MAX;
    int y = x + 1;  // 有符号整数溢出——未定义行为
    std::cout << y << std::endl;
}

void null_dereference() {
    int *p = nullptr;
    // int val = *p;  // 空指针解引用——未定义行为
}

void shift_overflow() {
    int x = 1 << 31;  // 对有符号int左移进入符号位——未定义行为
    std::cout << x << std::endl;
}

int main() {
    signed_overflow();
    shift_overflow();
    return 0;
}
```

```bash
g++ -fsanitize=undefined -g ubsan_demo.cpp -o ubsan_demo
./ubsan_demo
# UBSan输出示例：
# ubsan_demo.cpp:6:14: runtime error: signed integer overflow: 2147483647 + 1 cannot be represented in type 'int'
# ubsan_demo.cpp:15:15: runtime error: left shift of 1 by 31 places cannot be represented in type 'int'
```

UBSan能检测的未定义行为：
- 有符号整数溢出
- 空指针解引用
- 除零
- 移位溢出
- 对齐违规
- 类型不匹配的reinterpret_cast
- 无效的bool值
- 数组越界（需配合-fsanitize=bounds）

### 4. MemorySanitizer（MSan）—— 未初始化内存检测

```bash
# 启用MemorySanitizer（仅Clang支持，GCC不支持）
clang++ -fsanitize=memory -g main.cpp -o main
```

```cpp
// msan_demo.cpp
#include <iostream>

void use_uninitialized() {
    int x;  // 未初始化
    if (x > 0) {  // 使用未初始化的值
        std::cout << "正数" << std::endl;
    }
}

int main() {
    use_uninitialized();
    return 0;
}
```

注意：MSan目前只有Clang支持，GCC没有实现。如果需要使用MSan，请安装Clang。

### 5. LeakSanitizer（LSan）—— 内存泄漏检测

```bash
# LeakSanitizer通常集成在AddressSanitizer中
g++ -fsanitize=address -g main.cpp -o main

# 单独使用LeakSanitizer（仅Clang支持）
clang++ -fsanitize=leak -g main.cpp -o main

# 禁用ASan中的LSan（如果只想检测内存错误不检测泄漏）
g++ -fsanitize=address -g main.cpp -o main
ASAN_OPTIONS=detect_leaks=0 ./main
```

### 6. 组合使用规则

```bash
# ASan + UBSan（可以组合使用，推荐）
g++ -fsanitize=address,undefined -g -O1 main.cpp -o main

# TSan不能与ASan组合使用（两者冲突）
# 错误：g++ -fsanitize=address,thread main.cpp -o main

# UBSan可以与TSan组合
g++ -fsanitize=thread,undefined -g -O1 main.cpp -o main -lpthread

# MSan不能与ASan组合使用
# MSan不能与TSan组合使用

# 推荐的组合策略
# 1. 开发阶段：ASan + UBSan
g++ -fsanitize=address,undefined -g -O1 -fno-omit-frame-pointer main.cpp -o main

# 2. 多线程测试：TSan + UBSan
g++ -fsanitize=thread,undefined -g -O1 -fno-omit-frame-pointer main.cpp -o main -lpthread

# 3. 内存初始化检查：MSan（需Clang）
clang++ -fsanitize=memory -g -O1 -fno-omit-frame-pointer main.cpp -o main
```

### 7. 与CMake集成

```cmake
# CMakeLists.txt中集成Sanitizer

# 选项：启用ASan
option(ENABLE_ASAN "Enable AddressSanitizer" OFF)
if(ENABLE_ASAN)
    target_compile_options(main PRIVATE -fsanitize=address -fno-omit-frame-pointer)
    target_link_options(main PRIVATE -fsanitize=address)
endif()

# 选项：启用TSan
option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
if(ENABLE_TSAN)
    target_compile_options(main PRIVATE -fsanitize=thread -fno-omit-frame-pointer)
    target_link_options(main PRIVATE -fsanitize=thread)
endif()

# 选项：启用UBSan
option(ENABLE_UBSAN "Enable UndefinedBehaviorSanitizer" OFF)
if(ENABLE_UBSAN)
    target_compile_options(main PRIVATE -fsanitize=undefined -fno-omit-frame-pointer)
    target_link_options(main PRIVATE -fsanitize=undefined)
endif()

# 使用方式
# cmake -DENABLE_ASAN=ON ..
# make
```

更完善的CMake Sanitizer模块：

```cmake
# Sanitize.cmake模块
function(enable_sanitizers target)
    set(SANITIZER_FLAGS "")

    if(ENABLE_ASAN)
        string(APPEND SANITIZER_FLAGS " -fsanitize=address")
    endif()

    if(ENABLE_TSAN)
        if(ENABLE_ASAN)
            message(FATAL_ERROR "ASan和TSan不能同时启用")
        endif()
        string(APPEND SANITIZER_FLAGS " -fsanitize=thread")
    endif()

    if(ENABLE_UBSAN)
        string(APPEND SANITIZER_FLAGS " -fsanitize=undefined")
    endif()

    if(SANITIZER_FLAGS)
        string(STRIP "${SANITIZER_FLAGS}" SANITIZER_FLAGS)
        target_compile_options(${target} PRIVATE ${SANITIZER_FLAGS} -fno-omit-frame-pointer -O1)
        target_link_options(${target} PRIVATE ${SANITIZER_FLAGS})
    endif()
endfunction()
```

---

## 5. 编译器分析工具

### 1. -fdiagnostics-show-option：显示警告选项名

```bash
# 默认警告输出
g++ -Wall main.cpp
# main.cpp:5:10: warning: unused variable 'x' [-Wunused-variable]

# 启用后会在警告信息末尾显示对应的选项名
g++ -Wall -fdiagnostics-show-option main.cpp
# main.cpp:5:10: warning: unused variable 'x' [-Wunused-variable]
# 选项名清晰可见，方便用-Wno-选项名来关闭
```

### 2. -fdiagnostics-color=always：彩色输出

```bash
# 彩色编译输出（错误红色、警告黄色等）
g++ -Wall -fdiagnostics-color=always main.cpp -o main

# 自动检测终端是否支持颜色
g++ -Wall -fdiagnostics-color=auto main.cpp -o main

# 永不使用颜色
g++ -Wall -fdiagnostics-color=never main.cpp -o main

# 在CI/CD管道中推荐always
# 在脚本中推荐never
```

### 3. -ftime-report：编译时间报告

```bash
# 显示各编译阶段的时间消耗
g++ -ftime-report main.cpp -o main
# 输出示例：
# Time variable                                   usr           sys          wall           GGC
# phase setup and parsing                        0.01          0.00          0.01          30 kB
# phase lang. deferred                           0.00          0.00          0.00           0 kB
# phase opt and generate                         0.02          0.00          0.02          20 kB
# phase finalize                                 0.00          0.00          0.00           0 kB
# dump files                                     0.00          0.00          0.00           0 kB
# callgraph construction                         0.00          0.00          0.00           2 kB
# ipa inlining heuristics                        0.00          0.00          0.00           0 kB
# CFG construction                               0.00          0.00          0.00           0 kB
# CFG cleanup                                    0.00          0.00          0.00           0 kB
# alias analysis                                 0.00          0.00          0.00           0 kB
# reload                                         0.01          0.00          0.01          10 kB
# TOTAL                                          0.04          0.00          0.04          62 kB

# 更详细的时间报告
g++ -ftime-report-details main.cpp -o main
```

### 4. -fstack-usage：栈使用分析

```bash
# 生成每个函数的栈使用信息
g++ -fstack-usage main.cpp -o main
# 会生成 main.su 文件

# 查看栈使用信息
cat main.su
# 输出示例：
# main.cpp:5:6:main    32  static
# main.cpp:10:6:process_data    1024  dynamic,bounded
# main.cpp:20:6:recursive_func    64  dynamic
#
# 格式：文件:行:列:函数名  栈大小(字节)  类型
# static：固定大小
# dynamic,bounded：动态但有上界（如VLA）
# dynamic：动态无上界（如alloca）
```

栈使用分析对于嵌入式开发和深度递归程序特别重要，可以预防栈溢出。

### 5. -Wp,-v：预处理器详细输出

```bash
# 查看预处理器的搜索路径和详细过程
g++ -Wp,-v -E main.cpp > /dev/null
# 输出：
# ignoring duplicate directory "/usr/include/x86_64-linux-gnu/c++/13"
# #include <...> search starts here:
#  /usr/include/c++/13
#  /usr/include/x86_64-linux-gnu/c++/13
#  /usr/include/c++/13/backward
#  /usr/lib/gcc/x86_64-linux-gnu/13/include
#  /usr/local/include
#  /usr/include/x86_64-linux-gnu
#  /usr/include
# End of search list.
```

### 6. 更多有用的分析选项

```bash
# -fstack-protector：栈保护（防止缓冲区溢出攻击）
g++ -fstack-protector-all main.cpp -o main
# 在函数中插入canary值，检测栈溢出

# -D_FORTIFY_SOURCE=2：运行时缓冲区溢出检测
g++ -O2 -D_FORTIFY_SOURCE=2 main.cpp -o main
# 需要配合-O1及以上优化级别使用
# 会将strcpy等不安全函数替换为安全版本

# -flto：链接时优化（Link Time Optimization）
g++ -O2 -flto main.cpp math_utils.cpp -o main
# 跨编译单元优化，可以内联其他文件中的函数
# 编译时间更长，但运行性能可能更好

# -fprofile-generate / -fprofile-use：基于反馈的优化（FDO）
# 步骤1：编译带插桩的程序
g++ -O2 -fprofile-generate main.cpp -o main_instrumented

# 步骤2：运行程序生成profile数据
./main_instrumented
# 生成 *.gcda 文件

# 步骤3：使用profile数据重新编译
g++ -O2 -fprofile-use main.cpp -o main_optimized

# -fopt-info：查看优化决策
g++ -O2 -fopt-info-vec main.cpp -o main
# 输出向量化信息：
# main.cpp:10:3: note: loop vectorized

g++ -O2 -fopt-info-inline main.cpp -o main
# 输出内联信息：
# main.cpp:5:3: note: inlined 'add' into 'main'
```

### 7. 完整的推荐编译选项集

```bash
# 开发调试版本
g++ -std=c++17 \
    -g -Og \
    -Wall -Wextra -Wshadow -Wconversion -Wold-style-cast \
    -fsanitize=address,undefined \
    -fno-omit-frame-pointer \
    -DDEBUG \
    main.cpp -o main_debug

# 发布优化版本
g++ -std=c++17 \
    -O2 \
    -Wall -Wextra -Wshadow -Wconversion \
    -DNDEBUG \
    -D_FORTIFY_SOURCE=2 \
    -fstack-protector-strong \
    -flto \
    main.cpp -o main_release

# 最严格检查版本
g++ -std=c++17 \
    -O0 -g3 \
    -Wall -Wextra -Wpedantic -Wshadow -Wconversion \
    -Wold-style-cast -Werror \
    -fsanitize=address,undefined \
    -fno-omit-frame-pointer \
    main.cpp -o main_strict
```

---

## 6. 附录：常见问题速查

### 1. 编译错误速查

| 错误信息 | 原因 | 解决方法 |
|----------|------|----------|
| `undefined reference to 'std::cout'` | 用gcc编译C++程序 | 改用g++或加-lstdc++ |
| `No such file or directory` for header | 头文件路径不对 | 加-I/path/to/include |
| `cannot find -lmylib` | 找不到库文件 | 加-L/path/to/lib |
| `error while loading shared libraries` | 运行时找不到动态库 | 加-Wl,-rpath或设LD_LIBRARY_PATH |
| `multiple definition of 'xxx'` | 重复定义 | 检查头文件中是否有定义（非声明） |
| `undefined reference to 'xxx'` | 链接时找不到符号 | 检查是否链接了对应的库 |

### 2. 常用命令速查

```bash
# 查看编译器版本
gcc --version

# 查看预定义宏
gcc -dM -E - < /dev/null

# 查看头文件搜索路径
gcc -E -Wp,-v - < /dev/null

# 查看动态库依赖
ldd ./main

# 查看符号表
nm -C ./main

# 查看需要的动态库
objdump -p ./main | grep NEEDED

# 反汇编
objdump -d -S ./main

# 生成依赖关系
gcc -MM main.c

# 查看优化选项差异
diff <(gcc -O2 -Q --help=optimizers) <(gcc -O3 -Q --help=optimizers)
```
