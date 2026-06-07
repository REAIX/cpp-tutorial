# C 与 C++ 基础数据类型差异
> 📖 相关章节：[运算符与表达式](../../01-C语言/02-运算符与表达式.md)、[控制结构](../../01-C语言/03-控制结构.md)、[函数](../../01-C语言/04-函数.md)、[基础特性](../../02-CPP/01-基础特性.md)、[命名空间](../../02-CPP/02-命名空间与编码规范.md)

## 1. 共同的基础类型

C 和 C++ 共享这些基本类型：

| 类型 | C | C++ | 说明 |
|------|:---:|:---:|------|
| char | ✓ | ✓ | 字符型 |
| short | ✓ | ✓ | 短整型 |
| int | ✓ | ✓ | 整型 |
| long | ✓ | ✓ | 长整型 |
| float | ✓ | ✓ | 单精度浮点 |
| double | ✓ | ✓ | 双精度浮点 |
| void | ✓ | ✓ | 无类型 |

**这些类型在 C 和 C++ 中几乎完全相同，没有本质差异。**

***

## 2. C++ 新增的类型

| 类型 | C | C++ | 说明 |
|------|:---:|:---:|------|
| bool | ✗ | ✓ | 布尔类型 |
| wchar_t | ✗ | ✓ | 宽字符类型 |
| long long | C99 | C++11 | 64位整型 |
| char16_t | ✗ | C++11 | UTF-16字符 |
| char32_t | ✗ | C++11 | UTF-32字符 |
| nullptr_t | ✗ | C++11 | 空指针类型 |
| char8_t | ✗ | C++20 | UTF-8字符 |

### 1. bool 类型

```c
// C 语言：没有 bool，用 int 代替
int flag = 1;   // 0 = 假，非0 = 真

// C99 引入了 _Bool，需要 <stdbool.h>
#include <stdbool.h>
bool flag = true;
```

```cpp
// C++：原生 bool
bool flag = true;   // true/false 是关键字
```

### 2. 字符类型

```c
// C：只有 char
char c = 'A';
// 宽字符需要 wchar_t，但它是 typedef
typedef ... wchar_t;  // 不是独立关键字
```

```cpp
// C++：多种字符类型
char c = 'A';           // 窄字符
wchar_t wc = L'A';      // 宽字符（独立关键字）
char16_t c16 = u'A';    // C++11 UTF-16
char32_t c32 = U'A';    // C++11 UTF-32
char8_t c8 = u8'A';     // C++20 UTF-8
```

### 3. 宽字符类型的用途

**问：宽字符类型是干嘛的？**

**答：** 宽字符类型（`wchar_t`）主要用于表示**Unicode 字符**，解决普通 `char` 只能表示 ASCII 字符的局限性。

**核心用途：**

1. **支持多语言字符**：普通 `char` 通常是 1 字节，只能表示 ASCII 字符（0-127）。而 `wchar_t` 通常是 2 或 4 字节，可以表示全球各种语言的字符（如中文、日文、阿拉伯文等）。

2. **处理 Unicode 文本**：用于存储和操作 Unicode 编码的字符串。

**使用示例：**

```cpp
// 窄字符（只能表示ASCII）
char c = 'A';

// 宽字符（可以表示Unicode）
wchar_t wc = L'中';  // L前缀表示宽字符字面量

// 宽字符串
const wchar_t* wstr = L"你好世界";

// 宽字符串类
std::wstring wstr_obj = L"Hello World";
```

**Unicode 字符类型对比：**

| 类型 | 编码 | 字节数 | 说明 |
|------|------|--------|------|
| `char` | 执行字符集相关 | 1 | 窄字符 |
| `wchar_t` | 平台相关 | 2或4 | 宽字符（Windows为UTF-16，Linux为UTF-32） |
| `char16_t` | UTF-16 | 2 | C++11新增，固定2字节 |
| `char32_t` | UTF-32 | 4 | C++11新增，固定4字节 |
| `char8_t` | UTF-8 | 1 | C++20新增，UTF-8字符 |

**注意事项：**
- `wchar_t` 在 C++ 中是独立关键字，而在 C 语言中只是 `typedef`
- 使用宽字符字面量时需要加 `L` 前缀
- 标准库提供了对应的宽字符串类 `std::wstring`

***

## 3. C++ 新增的复合类型

| 类型 | C | C++ | 说明 |
|------|:---:|:---:|------|
| class | ✗ | ✓ | 类 |
| reference (&) | ✗ | ✓ | 引用 |
| string | ✗ | ✓ | 标准字符串（不是基本类型，但极常用） |

### 1. 引用

```c
// C：只有指针
void swap(int* a, int* b) {
    int tmp = *a; *a = *b; *b = tmp;
}
```

```cpp
// C++：有引用
void swap(int& a, int& b) {
    int tmp = a; a = b; b = tmp;
}
```

### 2. 字符串

```c
// C：char* + <string.h>
char s1[20] = "hello";
char s2[20];
strcpy(s2, s1);    // 手动管理
strcat(s2, " world");
printf("%s\n", s2);
```

```cpp
// C++：std::string
#include <string>
std::string s1 = "hello";
std::string s2 = s1 + " world";  // 自动管理
std::cout << s2 << "\n";
```

***

## 4. 类型检查的差异

### 1. C：弱类型检查

```c
// C 允许隐式转换，很多不安全操作不警告
void* p = malloc(10);
int* ip = p;           // OK：void* 隐式转 int*
int x = 3.14;          // OK：double 隐式截断为 int
```

### 2. C++：强类型检查

```cpp
// C++ 更严格
void* p = malloc(10);
int* ip = (int*)p;     // 需要显式转换！void* 不能隐式转 int*
int x = 3.14;          // 警告：窄化转换
int y = {3.14};        // 错误！列表初始化禁止窄化
```

### 3. 枚举类型

```c
// C：enum 本质是 int
enum Color { RED, GREEN, BLUE };
enum Color c = 1;       // OK：int 隐式转 enum
int x = RED;            // OK：enum 隐式转 int
```

```cpp
// C++：enum 是独立类型
enum Color { RED, GREEN, BLUE };
Color c = 1;            // 错误！int 不能隐式转 enum
int x = RED;            // OK：enum 可隐式转 int

// C++11：enum class 更严格
enum class Color2 { RED, GREEN, BLUE };
int y = Color2::RED;    // 错误！不能隐式转 int
Color2 c2 = 2;          // 错误！int 不能隐式转 enum class
```

***

## 5. 总结

```
C 和 C++ 基础数据类型几乎相同（int/char/float/double等）
C++ 新增了 bool、wchar_t、char16_t、char32_t、char8_t、nullptr_t
C++ 新增了引用(&)和 class 类型
C++ 类型检查更严格，禁止很多 C 的隐式转换
C++ 的 enum class 比 C 的 enum 更安全
```

### 4. C++ 各版本新增类型汇总

| C++版本 | 新增类型 | 说明 |
|---------|----------|------|
| C++98 | `bool`, `wchar_t` | 布尔类型、宽字符类型 |
| C++11 | `long long`, `char16_t`, `char32_t`, `nullptr_t` | 64位整型、UTF-16/UTF-32字符、空指针类型 |
| C++20 | `char8_t` | UTF-8字符类型 |

### 5. 其他重要的类型相关特性

1. **enum class（C++11）**：强类型枚举，提供更好的类型安全
   ```cpp
   enum class Color { RED, GREEN, BLUE };
   Color c = Color::RED;  // 必须使用作用域解析
   ```

2. **nullptr（C++11）**：类型安全的空指针常量
   ```cpp
   int* p = nullptr;  // 比 NULL 更安全
   ```

   **问：`nullptr_t` 和 `nullptr` 有什么区别？**

   **答：** 简单来说：
   - `nullptr` 是**值**，是空指针常量
   - `nullptr_t` 是**类型**，是 `nullptr` 的类型

   **详细解释：**

   | 名称 | 类型 | 说明 |
|------|------|------|
   | `nullptr` | `std::nullptr_t` | 空指针常量值 |
   | `nullptr_t` | 类型 | 空指针类型（定义在 `<cstddef>` 中） |

   **使用示例：**

   ```cpp
   #include <cstddef>  // 包含 nullptr_t 的定义
   
   // nullptr 是空指针常量
   int* p = nullptr;
   char* q = nullptr;
   
   // nullptr_t 是空指针类型
   std::nullptr_t null_val = nullptr;
   
   // nullptr 可以隐式转换为任何指针类型
   void* vp = nullptr;
   int** pp = nullptr;
   
   // nullptr_t 类型的变量也可以转换为指针
   std::nullptr_t np = nullptr;
   int* r = np;  // OK
   ```

   **与 NULL 的对比：**

   ```cpp
   // C 语言的 NULL（通常是 (void*)0 或 0）
   int* p1 = NULL;  // 在 C++ 中可能有类型问题
   
   // C++11 的 nullptr（类型安全）
   int* p2 = nullptr;  // 类型安全，推荐使用
   
   // nullptr 可以区分指针和整数
   void func(int);
   void func(int*);
   
   func(NULL);   // 可能调用 func(int)，取决于 NULL 的定义
   func(nullptr); // 明确调用 func(int*)
   ```

   **总结：**
   - `nullptr` 是具体的空指针值，用于给指针变量赋值
   - `nullptr_t` 是这个值的类型，主要用于模板编程和类型萃取

   **问：`std::nullptr_t` 和 `void*` 有区别吗？**

   **答：** 有区别，而且区别很重要！

   | 特性 | `std::nullptr_t` | `void*` |
|------|------------------|---------|
   | **类型本质** | 空指针类型 | 无类型指针 |
   | **隐式转换** | 可转换为**任何指针类型** | 可转换为**非 const 指针类型** |
   | **指向对象** | 不指向任何对象 | 指向未知类型的对象 |
   | **解引用** | 不能解引用 | 不能直接解引用（需转换） |
   | **比较运算** | 可以比较 | 可以比较 |
   | **函数重载** | 匹配指针参数 | 匹配 `void*` 参数 |

   **关键区别示例：**

   ```cpp
   #include <cstddef>
   
   void func(int*);
   void func(void*);
   void func(std::nullptr_t);
   
   int main() {
       int* p = nullptr;
       void* vp = nullptr;
       
       func(p);         // 调用 func(int*)
       func(vp);        // 调用 func(void*)
       func(nullptr);   // 调用 func(std::nullptr_t)（如果存在）
       
       // void* 可以指向任意类型的对象
       int x = 42;
       void* pv = &x;  // OK
       
       // nullptr_t 不能指向对象
       std::nullptr_t np = nullptr;
       // std::nullptr_t np2 = &x;  // 错误！
       
       return 0;
   }
   ```

   **类型安全对比：**

   ```cpp
   // void* 的问题：可以转换为任意指针，但不安全
   void* void_ptr = malloc(10);
   int* int_ptr = (int*)void_ptr;  // 需要显式转换
   
   // nullptr_t 的优势：类型安全的空指针
   int* safe_ptr = nullptr;  // 隐式转换，类型安全
   
   // nullptr_t 不能转换为非指针类型
   // int x = nullptr;  // 错误！
   
   // void* 可以转换为 bool（在条件判断中）
   if (void_ptr) { /* ... */ }  // OK
   ```

   **总结：**
   - `void*` 是"指向未知类型的指针"，可以指向任何对象
   - `nullptr_t` 是"空指针类型"，表示不指向任何对象
   - 在现代 C++ 中，推荐使用 `nullptr`（`nullptr_t` 类型）而不是 `NULL` 或 `void*` 来表示空指针

   **问：不指向任何对象，那设计 `nullptr_t` 来干嘛？**

   **答：** 空指针的设计是为了表示"指针变量当前没有指向有效对象"的状态，这在编程中非常有用：

   **主要用途：**

   1. **初始化指针**：声明指针时初始化为空，避免野指针
      ```cpp
      int* p = nullptr;  // 明确表示指针不指向任何对象
      ```

   2. **标记特殊状态**：表示操作失败或无结果
      ```cpp
      int* find(int value) {
          // 如果没找到，返回空指针
          if (not_found) return nullptr;
          return &result;
      }
      ```

   3. **条件判断**：检查指针是否有效
      ```cpp
      int* p = get_pointer();
      if (p != nullptr) {  // 检查指针是否指向有效对象
          *p = 42;
      }
      ```

   4. **重置指针**：释放资源后将指针置空
      ```cpp
      delete p;
      p = nullptr;  // 防止悬垂指针
      ```

   5. **函数重载区分**：区分指针和非指针参数
      ```cpp
      void process(int* p);     // 处理指针
      void process(int value);   // 处理整数值
      
      process(nullptr);  // 明确调用 process(int*)，不会混淆
      ```

   6. **模板编程**：类型安全的空指针处理
      ```cpp
      template<typename T>
      T* safe_cast(void* ptr) {
          return ptr != nullptr ? static_cast<T*>(ptr) : nullptr;
      }
      ```

   **为什么需要专门的 `nullptr_t` 类型？**

   - **类型安全**：`nullptr` 只能转换为指针类型，不能转换为整数
   - **重载区分**：可以为 `nullptr_t` 提供专门的重载版本
   - **语义清晰**：明确表达"空指针"的语义，提高代码可读性

   **对比 C 语言的 NULL：**

   ```cpp
   // C 语言的 NULL 可能是 0 或 (void*)0
   int* p1 = NULL;  // 在某些情况下可能被解释为整数 0
   
   // C++ 的 nullptr 是类型安全的
   int* p2 = nullptr;  // 明确是空指针
   ```

   **总结：** `nullptr_t` 的设计是为了提供一种**类型安全、语义清晰**的方式来表示"空指针"状态，避免 C 语言中 NULL 的歧义问题。

3. **auto（C++11）**：类型自动推导
   ```cpp
   auto x = 42;      // x 是 int
   auto y = 3.14;    // y 是 double
   ```

4. **decltype（C++11）**：获取表达式类型
   ```cpp
   int x = 10;
   decltype(x) y = 20;  // y 的类型是 int
   ```

***

## 6. 极简口诀

```
基础类型两相同，int char float double
C++新增bool和引用，字符类型更丰富
C弱类型随便转，C++严格要显式
enum class更安全，列表初始化防窄化
```

***

### 相关阅读

- [浮点数精度陷阱](29-浮点数精度陷阱.md)
- [sizeof运算符常见误区](26-sizeof运算符常见误区.md)
- [原码反码与补码](06-原码反码与补码.md)