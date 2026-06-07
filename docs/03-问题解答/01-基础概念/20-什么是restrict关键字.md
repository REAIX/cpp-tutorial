# 什么是restrict关键字
> 📖 相关章节：[运算符与表达式](../../01-C语言/02-运算符与表达式.md)、[控制结构](../../01-C语言/03-控制结构.md)、[函数](../../01-C语言/04-函数.md)、[基础特性](../../02-CPP/01-基础特性.md)、[命名空间](../../02-CPP/02-命名空间与编码规范.md)

> "restrict=向编译器发誓：这个指针是访问数据的唯一途径。"——C99关键字，告诉编译器没有别名，允许更激进的优化。

***

### 1. 通俗理解

- **restrict** = 程序员向编译器做出的承诺：通过这个指针访问的内存，不会通过其他指针来访问
- 就像你向快递员保证"这个信箱只有我有钥匙"，快递员就可以放心地把信直接塞进去，不用每次检查信箱里有没有别人的信
- restrict不改变程序的行为，只是给编译器更多优化空间
- 如果你违背承诺（通过其他指针也访问同一内存），结果是未定义行为

| 概念 | 类比 | 说明 |
|------|------|------|
| restrict | "唯一钥匙"承诺 | 向编译器保证指针是访问数据的唯一方式 |
| 别名问题 | 多把钥匙开同一把锁 | 两个指针指向同一内存，编译器必须保守优化 |
| 优化 | 快递员不用反复检查 | 编译器知道没有别名，可以更激进地优化 |
| 违背承诺 | 偷配钥匙 | 用其他指针访问同一内存，未定义行为 |

***

### 2. 技术说明

#### 1. 为什么需要restrict——别名问题

编译器在优化时，必须考虑两个指针是否指向同一块内存（别名问题）。

```c
void add_arrays(int* a, int* b, int* c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}
```

编译器不能确定`c`和`a`是否重叠。如果`c`和`a`指向同一数组，那么写入`c[i]`可能影响`a[i+1]`的值，编译器必须保守地每次重新从内存读取`a[i]`。

加上restrict后：

```c
void add_arrays(int* restrict a, int* restrict b, int* restrict c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}
```

编译器知道`a`、`b`、`c`不会重叠，可以：
- 把`a`和`b`的值缓存到寄存器中
- 进行循环展开
- 使用SIMD指令并行处理

#### 2. restrict的用法

**函数参数声明**（最常见用法）：

```c
void copy(int* restrict dst, const int* restrict src, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = src[i];
    }
}
```

**局部变量声明**：

```c
void process(void) {
    int* restrict p1 = (int*)malloc(100 * sizeof(int));
    int* restrict p2 = (int*)malloc(100 * sizeof(int));
    /* 编译器知道p1和p2指向不同内存 */
}
```

**结构体成员**：

```c
struct buffer {
    int* restrict data;
    int size;
};
```

#### 3. restrict不保证运行时唯一性

restrict是一个**编译期承诺**，不是运行时检查：

| 维度 | 说明 |
|------|------|
| 编译器 | 信任程序员的承诺，基于此进行优化 |
| 运行时 | 不会检查指针是否真的唯一 |
| 违背承诺 | 未定义行为——可能得到错误结果，也可能碰巧正确 |
| 调试 | 某些工具（如GCC的`-fstrict-aliasing`）可帮助检测 |

```c
#include <stdio.h>

void add(int* restrict a, int* restrict b, int* restrict c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

int main(void) {
    int data[] = {1, 2, 3, 4};
    /* 危险！a和c指向同一数组，违背了restrict承诺 */
    add(data, data, data, 4);
    /* 未定义行为，结果不可预测 */
    printf("%d %d %d %d\n", data[0], data[1], data[2], data[3]);
    return 0;
}
```

#### 4. C++中不支持restrict

| 维度 | C | C++ |
|------|---|-----|
| restrict | C99标准关键字 | 标准不支持 |
| __restrict | — | GCC/Clang扩展，效果相同 |
| __restrict__ | — | GCC扩展的另一种写法 |
| MSVC | — | 支持`__restrict` |

**C++中使用restrict的方式**：

```cpp
void add(int* __restrict a, int* __restrict b, int* __restrict c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}
```

#### 5. 标准库中的restrict

C99标准库大量使用了restrict：

| 函数 | restrict参数 | 原因 |
|------|-------------|------|
| `memcpy(void* restrict s1, const void* restrict s2, size_t n)` | s1, s2 | 源和目标不能重叠 |
| `strcpy(char* restrict s1, const char* restrict s2)` | s1, s2 | 源和目标不能重叠 |
| `strcat(char* restrict s1, const char* restrict s2)` | s1, s2 | 源和目标不能重叠 |
| `fprintf(FILE* restrict stream, const char* restrict format, ...)` | stream, format | 格式串和流不相关 |

注意`memmove`没有restrict，因为它允许源和目标重叠。

#### 6. 性能影响示例

```c
#include <stdio.h>
#include <time.h>
#include <stdlib.h>

#define SIZE 10000000

void add_no_restrict(int* a, int* b, int* c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

void add_with_restrict(int* restrict a, int* restrict b, int* restrict c, int n) {
    for (int i = 0; i < n; i++) {
        c[i] = a[i] + b[i];
    }
}

int main(void) {
    int* a = (int*)malloc(SIZE * sizeof(int));
    int* b = (int*)malloc(SIZE * sizeof(int));
    int* c = (int*)malloc(SIZE * sizeof(int));

    for (int i = 0; i < SIZE; i++) {
        a[i] = i;
        b[i] = i * 2;
    }

    clock_t start, end;

    start = clock();
    add_no_restrict(a, b, c, SIZE);
    end = clock();
    printf("无restrict: %.3f ms\n", (double)(end - start) * 1000 / CLOCKS_PER_SEC);

    start = clock();
    add_with_restrict(a, b, c, SIZE);
    end = clock();
    printf("有restrict: %.3f ms\n", (double)(end - start) * 1000 / CLOCKS_PER_SEC);

    free(a);
    free(b);
    free(c);
    return 0;
}
```

**编译**：

```bash
gcc -O2 restrict_demo.c -o restrict_demo
```

在高优化级别下，restrict版本可能更快，因为编译器可以进行向量化优化。

#### 7. restrict vs memcpy vs memmove

| 函数 | restrict | 重叠行为 | 适用场景 |
|------|----------|---------|---------|
| memcpy | 有 | 未定义行为（源和目标不重叠） | 确定不重叠时使用，更快 |
| memmove | 无 | 正确处理重叠 | 可能重叠时使用，稍慢 |

***

### 3. 代码示例：矩阵运算中的restrict优化

```c
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 500

void matrix_add_no_restrict(double* a, double* b, double* c, int n) {
    for (int i = 0; i < n * n; i++) {
        c[i] = a[i] + b[i];
    }
}

void matrix_add_with_restrict(double* restrict a, double* restrict b,
                               double* restrict c, int n) {
    for (int i = 0; i < n * n; i++) {
        c[i] = a[i] + b[i];
    }
}

void matrix_scale_with_restrict(double* restrict a, double* restrict b,
                                 double scale, int n) {
    for (int i = 0; i < n * n; i++) {
        b[i] = a[i] * scale;
    }
}

int main(void) {
    int size = N * N;
    double* a = (double*)malloc(size * sizeof(double));
    double* b = (double*)malloc(size * sizeof(double));
    double* c = (double*)malloc(size * sizeof(double));

    for (int i = 0; i < size; i++) {
        a[i] = (double)i * 0.001;
        b[i] = (double)i * 0.002;
    }

    clock_t start, end;

    start = clock();
    for (int i = 0; i < 100; i++) {
        matrix_add_no_restrict(a, b, c, N);
    }
    end = clock();
    printf("无restrict矩阵加法: %.3f ms\n", (double)(end - start) * 1000 / CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < 100; i++) {
        matrix_add_with_restrict(a, b, c, N);
    }
    end = clock();
    printf("有restrict矩阵加法: %.3f ms\n", (double)(end - start) * 1000 / CLOCKS_PER_SEC);

    start = clock();
    for (int i = 0; i < 100; i++) {
        matrix_scale_with_restrict(a, b, 2.5, N);
    }
    end = clock();
    printf("有restrict矩阵缩放: %.3f ms\n", (double)(end - start) * 1000 / CLOCKS_PER_SEC);

    free(a);
    free(b);
    free(c);
    return 0;
}
```

**编译**：

```bash
gcc -O3 restrict_matrix.c -o restrict_matrix
./restrict_matrix
```

***

### 4. 常见问题

#### 1. 问题1：restrict和const有什么区别

const表示"不能通过这个指针修改数据"，restrict表示"只有这个指针能访问这块数据"。const是类型系统的约束（编译器会检查），restrict是程序员的承诺（编译器信任但不检查）。

#### 2. 问题2：违背restrict承诺一定会出问题吗

不一定。未定义行为意味着"编译器可以假设承诺成立并据此优化"。如果编译器确实据此做了优化（如向量化、重排序），就会产生错误结果；如果编译器没有做相关优化，可能碰巧正确。但绝不能依赖这种碰巧。

#### 3. 问题3：C++为什么不支持restrict

C++有引用语义、类、模板等复杂特性，restrict的语义在C++中难以精确定义。C++标准委员会认为引入restrict的复杂度不值得。但主流编译器通过`__restrict`扩展提供了类似功能。

#### 4. 问题4：什么时候应该使用restrict

在性能关键的函数参数中使用，特别是：数组处理函数、内存拷贝函数、数学运算函数。确保传入的指针确实不会指向重叠内存。

***

### 5. 极简总结

**restrict是C99关键字，告诉编译器"这个指针是访问对应内存的唯一方式"，帮助编译器消除别名顾虑、进行更激进的优化。restrict是程序员承诺而非运行时检查，违背承诺是未定义行为。C++标准不支持restrict，但GCC/Clang/MSVC通过__restrict扩展支持。标准库中memcpy使用restrict，memmove不使用。**

| 要点 | 一句话 |
|------|--------|
| restrict | 向编译器承诺"指针是唯一访问途径"——帮助优化 |
| 别名问题 | 两个指针可能指向同一内存——编译器必须保守 |
| 违背承诺 | 未定义行为——可能错误，也可能碰巧正确 |
| C++ | 标准不支持，用`__restrict`扩展代替 |
| memcpy vs memmove | memcpy有restrict（不重叠），memmove无（允许重叠） |
| 使用场景 | 性能关键的数组/内存处理函数参数 |

***

### 相关阅读

- [volatile关键字](./12-volatile关键字.md)
- [什么是缓存命中率](./04-什么是缓存命中率.md)
- [什么是开销Overhead](./03-什么是开销Overhead.md)