# undefined reference 排查指南
> 📖 相关章节：[编译与链接](../../01-C语言/17-编译与链接.md)

### 1. 核心要义

**undefined reference** = 链接错误，编译通过了但链接器找不到函数/变量的定义。五大常见原因：未实现、未链接库、C/C++混用、重复定义、模板未实例化。

***

### 2. 错误信息解读

```bash
# 常见错误格式
main.cpp:(.text+0x1f): undefined reference to `foo(int)'
#                              ↑ 缺少的符号名（mangled）

# 使用 nm 查看改编后的符号
nm main.o | grep foo
# U _Z3fooi  ← U 表示 undefined（未定义）
# 如果看到 T _Z3fooi  ← T 表示已定义（text section）
```

链接错误 vs 编译错误：
| 错误类型 | 阶段 | 特征 | 常见原因 |
|----------|:----:|------|----------|
| 编译错误 | 编译期 | 语法/类型错误 | 拼写、类型不匹配 |
| 链接错误 | 链接期 | undefined reference | 缺少定义/库 |
| 运行时错误 | 运行期 | 崩溃/异常 | 逻辑/内存错误 |

### 3. 五大常见原因

| 原因 | 示例 |
|------|------|
| 声明了但没实现 | `.h` 声明了函数，`.cpp` 忘了写 |
| 未链接库 | 用了 pthread 但没加 `-lpthread` |
| C/C++ 混用 | C++ 调 C 函数没有 `extern "C"` |
| 重复定义 | 同一函数在多个 `.cpp` 中定义 |
| 模板未实例化 | 模板定义在 `.cpp` 中，其他文件看不到 |

### 4. 排查流程图

```
发现 undefined reference 错误
       │
       ▼
看错误信息中的符号名
       │
       ├── 符号名是 C++ mangled 格式？
       │       │
       │       ├── 是 → 检查是否声明但未实现
       │       │         nm -C 查看反改编后的符号
       │       │
       │       └── 否 → 检查 C/C++ 混用
       │                 是否有 extern "C"？
       │
       ▼
符号是否在项目源码中？
       │
       ├── 是 → 检查：
       │       1. 函数体是否完整实现
       │       2. .cpp 文件是否参与编译
       │       3. 条件编译 (#ifdef) 是否排除
       │       4. 模板是否在头文件中
       │
       └── 否 → 检查外部库：
               1. 是否链接了正确的库？
               2. 库的顺序是否正确？
               3. 库的架构是否匹配（32/64位）？
               4. ldd 检查动态库是否找到？
```

### 5. 常见场景详解

#### 1. 场景1：声明未实现

```cpp
// math_utils.h
int add(int a, int b);  // 只有声明

// math_utils.cpp
// 忘了写实现！→ undefined reference to `add(int, int)'

// main.cpp
#include "math_utils.h"
int main() {
    return add(1, 2);  // 链接错误
}
```

#### 2. 场景2：未链接库

```bash
# 使用了 pthread 但没有链接
g++ main.cpp -o app
# undefined reference to `pthread_create'

# 正确：加 -lpthread
g++ main.cpp -lpthread -o app

# 常见需要显式链接的库
# -lpthread   → POSIX 线程
# -lm         → 数学库（sqrt, sin 等）
# -ldl        → 动态加载（dlopen, dlsym）
# -lrt        → 实时扩展（shm_open 等）
# -lz         → zlib 压缩
# -lssl -lcrypto → OpenSSL
```

#### 3. 场景3：C/C++ 混用

```cpp
// C 头文件（未保护）
// c_utils.h
void c_function();

// C++ 中引用
#include "c_utils.h"
int main() {
    c_function();  // 编译通过，链接报 undefined reference
}

// 原因：C++ 编译器改编了函数名（name mangling）
// C 编译器：c_function → c_function
// C++ 编译器：c_function → _Z11c_functionv

// 解决方案：加 extern "C"
#ifdef __cplusplus
extern "C" {
#endif
void c_function();
#ifdef __cplusplus
}
#endif
```

#### 4. 场景4：重复定义

```cpp
// 错误：在头文件中定义函数
// utils.h
int max(int a, int b) {  // 定义在头文件中！
    return a > b ? a : b;
}

// a.cpp
#include "utils.h"  // 生成 max 的定义

// b.cpp
#include "utils.h"  // 又生成了 max 的定义
// 链接时报 duplicate symbol 或 undefined reference

// 解决方案1：inline
inline int max(int a, int b) { return a > b ? a : b; }

// 解决方案2：声明放头文件，定义放一个 .cpp
// utils.h: int max(int a, int b);
// utils.cpp: int max(int a, int b) { ... }
```

#### 5. 场景5：模板未实例化

```cpp
// 错误：模板实现放在 .cpp 中
// template.h
template <typename T> T max(T a, T b);

// template.cpp
template <typename T> T max(T a, T b) { return a > b ? a : b; }
// 其他文件看不到实现 → undefined reference

// 正确：模板实现放在头文件中
// template.h
template <typename T> T max(T a, T b) { return a > b ? a : b; }

// 或显式实例化
// template.cpp
template int max<int>(int, int);  // 显式实例化 int 版本
```

### 6. 链接顺序问题

```bash
# 链接器处理库的顺序很重要！
# 错误：库的顺序导致 undefined reference
g++ main.cpp -lA -lB -o app
# 如果 libA 依赖 libB，但 libA 先出现，链接器已经处理完 libA

# 正确：被依赖的库放在后面
g++ main.cpp -lB -lA -o app  # libA 依赖 libB
# 或使用 --start-group / --end-group
g++ main.cpp -Wl,--start-group -lA -lB -Wl,--end-group -o app

# 链接器处理规则：
# 1. 从左到右处理目标文件和库
# 2. 如果库中的符号未被前面的文件引用，该库会被忽略
# 3. 推荐：依赖库放在引用它的库后面
```

### 7. 诊断工具

```bash
# nm：查看目标文件中的符号
nm main.o                    # 查看所有符号
nm -C main.o                 # C++ 符号反改编（demangle）
nm -u main.o                 # 只显示未定义符号
nm -D libfoo.so              # 查看动态库导出的符号

# ldd：查看动态库依赖
ldd ./app                    # 列出所有依赖的动态库
ldd -r ./app                 # 检查未解析的符号

# objdump：更详细的符号信息
objdump -t main.o            # 查看符号表
objdump -T libfoo.so         # 查看动态符号表

# readelf（Linux）
readelf -s main.o            # 查看符号表
readelf -d libfoo.so         # 查看动态段信息

# strings：搜索符号
strings libfoo.so | grep foo # 在库中搜索符号名

# 综合使用示例
g++ -c main.cpp -o main.o
nm -C main.o | grep foo      # 查看 foo 相关的符号
objdump -t main.o | grep foo # 同上，更详细
```

### 8. 静态库 vs 动态库的链接问题

```bash
# 静态库（.a）：
# 编译时直接嵌入目标文件
g++ -c math.cpp -o math.o
ar rcs libmath.a math.o       # 创建静态库

# 动态库（.so / .dll）：
# 运行时动态加载
g++ -fPIC -c math.cpp -o math.o
g++ -shared -o libmath.so math.o  # 创建动态库

# 链接静态库
g++ main.cpp -L. -lmath -o app    # -L 指定库路径
# 链接动态库
g++ main.cpp -L. -lmath -o app    # 默认优先链接动态库
# 强制静态链接
g++ main.cpp -static -L. -lmath -o app

# 常见问题：混合链接顺序
# libA.a 依赖 libB.so
g++ main.cpp -lA -lB -o app       # 可能失败
g++ main.cpp -lB -lA -o app       # 正确顺序
```

### 9. 平台特定问题

| 平台 | 常见问题 | 解决方案 |
|------|----------|----------|
| Linux | 未链接数学库 | `-lm` |
| Linux | pthread 未链接 | `-lpthread` |
| Windows | MSVC 需要特定库 | `#pragma comment(lib, "ws2_32.lib")` |
| Windows | DLL 导出符号 | `__declspec(dllexport)` / `.def` 文件 |
| macOS | 动态库路径 | `@rpath` / `install_name_tool` |
| 交叉编译 | 库架构不匹配 | 检查 `-m32`/`-m64` 是否一致 |

### 10. 极简总结

**undefined reference = 链接器找不到定义 → 检查实现/库/C混用/模板/链接顺序 → nm 看符号 → ldd 查依赖 → 注意库顺序（被依赖的放后面）**

***

### 相关阅读

- [什么是ODR单定义规则](./01-什么是ODR单定义规则.md)
- [什么是符号表Symbol-Table](./11-什么是符号表Symbol-Table.md)
- [什么是名称修饰Name-Mangling](./09-什么是名称修饰Name-Mangling.md)
