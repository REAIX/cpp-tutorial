# extern "C" 与动态库导出
> 📖 相关章节：[动态库与共享库](../../01-C语言/19-动态库与共享库.md)、[编译与链接](../../01-C语言/17-编译与链接.md)

### 1. 核心结论

1. **C++ 动态库想给 C / Python / C# / Java 调用** → 必须 `extern "C"`
2. **C++ 动态库只给 C++ 程序自己用** → **不用** **`extern "C"`**

### 2. 为什么需要 extern "C"

#### 1. C++ 的名字改编问题

C++ 支持：

- 函数重载 `add(int)` / `add(double)`
- 类成员函数
- 命名空间

编译器必须把函数名**改成唯一字符串**，才能区分，这叫 **Name Mangling（名字改编）**。

**示例**：

```cpp
int add(int a, int b);
```

编译后可能变成：`_Z3addii`

| 编译器 | add(int, int) 的改编结果 |
|:---|:---|
| GCC/Clang (Linux) | `_Z3addii` |
| MSVC (Windows) | `?add@@YAHHH@Z` |
| GCC (MinGW) | `_Z3addii` |

**后果**：你写 `GetProcAddress(dll, "add")` 会找不到函数，因为库里只有 `?_Z3addii` 或 `?add@@YAHHH@Z`。

#### 2. extern "C" 的作用

```cpp
extern "C" int add(int a, int b);
```

强制告诉 C++ 编译器：**按 C 规则编译，不要名字改编**。

**效果**：

- 导出函数名就是 **add**
- `GetProcAddress / dlsym` 直接按字符串 `"add"` 就能找到

**代价**：加了 `extern "C"` 的函数：

- 不能函数重载
- 不能用类成员、引用等 C++ 特有语法
- 只能写 **C 风格普通全局函数**

### 3. 两种动态库用法对比

#### 1. 跨语言调用（给 C/Python/C#/Java 用）

**必须使用** **`extern "C"`**：

```cpp
extern "C"
{
    __declspec(dllexport) int add(int a, int b);
}
```

- 禁止重载
- 函数名固定不变
- 可以用 `LoadLibrary + GetProcAddress` 按名字加载

#### 2. 纯 C++ 内部使用（只给 C++ 项目调用）

**不用** **`extern "C"`**：

- 随便重载、类、模板、命名空间
- 用头文件+库链接的方式调用
- 不走 `GetProcAddress` 字符串查找

### 4. 进阶：兼顾 C++ 类与跨语言调用

**标准做法**：内部写完整 C++ 类，外层包一层 `extern "C"` 接口。

**示例**：

```cpp
// 内部纯 C++ 类
class Calc
{
public:
    int add(int a, int b) { return a + b; }
    int sub(int a, int b) { return a - b; }
};

// 对外导出 C 接口
extern "C"
{
    static Calc calc;
    int calc_add(int a, int b)
    {
        return calc.add(a, b);
    }
    int calc_sub(int a, int b)
    {
        return calc.sub(a, b);
    }
}
```

- 外部语言调 `calc_add` / `calc_sub`
- 内部随便用 C++ 所有特性

### 5. 跨平台注意事项

#### 1. Windows (MSVC) 动态库导出

```cpp
// 宏定义统一导出
#ifdef _MSC_VER
    #define EXPORT __declspec(dllexport)
#else
    #define EXPORT __attribute__((visibility("default")))
#endif

extern "C"
{
    EXPORT int add(int a, int b);
    EXPORT void hello(const char* name);
}
```

#### 2. Linux (GCC/Clang) 动态库导出

```cpp
// Linux 下默认符号不可见，需要显式导出
// 编译选项：-fvisibility=hidden 后，只有标记的符号可见
extern "C"
{
    __attribute__((visibility("default"))) int add(int a, int b);
}
```

#### 3. 完整跨平台导出示例

```cpp
#ifdef __cplusplus
extern "C" {
#endif

#if defined(_WIN32) || defined(_WIN64)
    #ifdef BUILDING_DLL
        #define API __declspec(dllexport)
    #else
        #define API __declspec(dllimport)
    #endif
#else
    #define API __attribute__((visibility("default")))
#endif

API int add(int a, int b);
API int multiply(int a, int b);

#ifdef __cplusplus
}
#endif
```

#### 4. 调用约定差异

| 平台 | 默认调用约定 | 备注 |
|:---|:---|:---|
| Windows (x86) | `__cdecl` | C/C++编译器默认；WinAPI 使用 `__stdcall` |
| Windows (x64) | 一种调用约定 | 统一约定，不再区分 |
| Linux (x86/x64) | `__cdecl` / System V | 参数从右到左入栈，调用者清理栈 |

### 总结

| 场景 | 是否需要 extern "C" | 原因 |
| -------------- | --------------- | ----------------------- |
| C++ 库给其他语言调用 | ✅ 必须 | 防止名字改编，其他语言能按名字找到函数 |
| C++ 库只给 C++ 使用 | ❌ 不用 | 用头文件+链接方式，不怕名字改编 |
| 既要 C++ 类又要跨语言 | ✅ 外层封装 | 内部C++，外部用 extern "C" 接口 |
| 跨平台动态库 | ✅ 配合宏定义 | 统一 Windows/Linux 导出方式 |

***

### 相关阅读

- [extern关键字](14-extern关键字.md)
- [头文件守卫与pragmaonce](21-头文件守卫与pragmaonce.md)
- [inline关键字的真实含义](19-inline关键字的真实含义.md)