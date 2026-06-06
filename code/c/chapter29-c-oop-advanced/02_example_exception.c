/**
 * @file 02_example_exception.c
 * @brief 异常处理: setjmp/longjmp实现异常机制
 * @description 对应文档: 29-C语言面向对象实现-进阶
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

typedef struct {
    int code;
    char message[256];
} Exception;

static Exception g_exception;
static jmp_buf g_exception_buf;

#define TRY do { int _ex_code = setjmp(g_exception_buf); if (_ex_code == 0) {
#define CATCH(e) } else { (e) = g_exception; (e).code = _ex_code;
#define ENDTRY } } while(0)

#define THROW(err_code, msg) do { \
    g_exception.code = (err_code); \
    strncpy(g_exception.message, (msg), sizeof(g_exception.message) - 1); \
    g_exception.message[sizeof(g_exception.message) - 1] = '\0'; \
    longjmp(g_exception_buf, (err_code)); \
} while(0)

double safe_divide(double a, double b) {
    if (b == 0.0) THROW(1, "除零错误");
    return a / b;
}

int safe_array_get(const int *arr, int size, int index) {
    if (index < 0 || index >= size) THROW(2, "数组越界");
    return arr[index];
}

void *safe_malloc(size_t size) {
    void *ptr = malloc(size);
    if (!ptr && size > 0) THROW(3, "内存分配失败");
    return ptr;
}

void demo_basic_exception(void) {
    printf("\n=== demo_basic_exception ===\n");
    printf("使用setjmp/longjmp模拟try-catch异常机制\n\n");

    Exception e = {0, ""};

    printf("正常除法:\n");
    TRY {
        double result = safe_divide(10.0, 3.0);
        printf("  10 / 3 = %.2f\n", result);
    }
    CATCH(e) {
        printf("  捕获异常[%d]: %s\n", e.code, e.message);
    }
    ENDTRY;

    printf("\n除零异常:\n");
    TRY {
        double result = safe_divide(10.0, 0.0);
        printf("  10 / 0 = %.2f\n", result);
    }
    CATCH(e) {
        printf("  捕获异常[%d]: %s\n", e.code, e.message);
    }
    ENDTRY;

    printf("\n数组越界异常:\n");
    int arr[] = {10, 20, 30};
    TRY {
        int val = safe_array_get(arr, 3, 5);
        printf("  arr[5] = %d\n", val);
    }
    CATCH(e) {
        printf("  捕获异常[%d]: %s\n", e.code, e.message);
    }
    ENDTRY;
}

typedef struct {
    jmp_buf buf;
    int active;
    Exception last_exception;
} ExceptionStack;

#define MAX_NESTED_EXCEPTIONS 8

static ExceptionStack ex_stack[MAX_NESTED_EXCEPTIONS];
static int ex_stack_top = 0;

#define TRY_NESTED \
    do { \
        int _stack_idx = ex_stack_top++; \
        if (_stack_idx >= MAX_NESTED_EXCEPTIONS) { \
            fprintf(stderr, "异常嵌套过深\n"); \
            exit(1); \
        } \
        ex_stack[_stack_idx].active = 1; \
        int _ex_code = setjmp(ex_stack[_stack_idx].buf); \
        if (_ex_code == 0) {

#define CATCH_NESTED(e) \
        } else { \
            ex_stack[_stack_idx].active = 0; \
            ex_stack_top--; \
            (e) = ex_stack[_stack_idx].last_exception; \
            (e).code = _ex_code;

#define ENDTRY_NESTED \
            ex_stack[_stack_idx].active = 0; \
            ex_stack_top--; \
        } \
    } while(0)

#define THROW_NESTED(err_code, msg) do { \
    if (ex_stack_top > 0) { \
        int _idx = ex_stack_top - 1; \
        ex_stack[_idx].last_exception.code = (err_code); \
        strncpy(ex_stack[_idx].last_exception.message, (msg), \
                sizeof(ex_stack[_idx].last_exception.message) - 1); \
        longjmp(ex_stack[_idx].buf, (err_code)); \
    } else { \
        fprintf(stderr, "未捕获异常[%d]: %s\n", (err_code), (msg)); \
        exit(1); \
    } \
} while(0)

double nested_operation(int mode) {
    if (mode == 1) THROW_NESTED(10, "嵌套异常: 模式1错误");
    if (mode == 2) THROW_NESTED(20, "嵌套异常: 模式2错误");
    return 42.0;
}

void demo_nested_exception(void) {
    printf("\n=== demo_nested_exception ===\n");
    printf("嵌套异常: 支持多层try-catch\n\n");

    Exception e1 = {0, ""}, e2 = {0, ""};

    printf("外层正常, 内层异常:\n");
    TRY_NESTED {
        TRY_NESTED {
            nested_operation(1);
        }
        CATCH_NESTED(e2) {
            printf("  内层捕获异常[%d]: %s\n", e2.code, e2.message);
        }
        ENDTRY_NESTED;
        printf("  外层继续执行\n");
    }
    CATCH_NESTED(e1) {
        printf("  外层捕获异常[%d]: %s\n", e1.code, e1.message);
    }
    ENDTRY_NESTED;

    printf("\n内层未捕获, 传播到外层:\n");
    TRY_NESTED {
        TRY_NESTED {
            nested_operation(2);
        }
        ENDTRY_NESTED;
    }
    CATCH_NESTED(e1) {
        printf("  外层捕获传播的异常[%d]: %s\n", e1.code, e1.message);
    }
    ENDTRY_NESTED;
}

void demo_exception_pitfalls(void) {
    printf("\n=== demo_exception_pitfalls ===\n");
    printf("setjmp/longjmp陷阱:\n\n");

    printf("1. 资源泄漏:\n");
    printf("   longjmp跳过后续代码, 栈上局部变量的清理被跳过\n");
    printf("   malloc的内存、打开的文件等不会自动释放\n");
    printf("   解决: 使用cleanup属性或手动管理\n\n");

    printf("2. 局部变量优化:\n");
    printf("   setjmp后的局部变量可能被优化, 值不确定\n");
    printf("   解决: 用volatile修饰setjmp后使用的局部变量\n\n");

    printf("3. 只能跳回, 不能跳前:\n");
    printf("   longjmp只能跳回之前setjmp的位置\n");
    printf("   函数返回后不能再longjmp到该函数的setjmp\n\n");

    printf("4. 不如C++异常安全:\n");
    printf("   C++异常会自动调用析构函数\n");
    printf("   C的longjmp不会执行任何清理代码\n\n");

    printf("最佳实践:\n");
    printf("  1. 尽量用返回值代替异常(错误码模式)\n");
    printf("  2. 必须使用时, 确保资源在THROW前释放\n");
    printf("  3. 避免跨函数longjmp到已返回的函数\n");
    printf("  4. 考虑使用错误码+goto cleanup模式替代\n");
}

void demo_error_code_pattern(void) {
    printf("\n=== demo_error_code_pattern ===\n");
    printf("错误码+goto cleanup: C语言更安全的错误处理模式\n\n");

    typedef struct {
        int *data;
        int size;
    } Result;

    Result process_data(int count) {
        Result r = {NULL, 0};
        int *temp = NULL;
        int *output = NULL;

        temp = (int *)malloc(count * sizeof(int));
        if (!temp) goto cleanup;

        output = (int *)malloc(count * sizeof(int));
        if (!output) goto cleanup;

        for (int i = 0; i < count; i++) {
            temp[i] = i * 2;
            output[i] = temp[i] + 1;
        }

        r.data = output;
        r.size = count;
        output = NULL;

    cleanup:
        free(temp);
        free(output);
        return r;
    }

    Result r = process_data(5);
    if (r.data) {
        printf("处理结果: ");
        for (int i = 0; i < r.size; i++) printf("%d ", r.data[i]);
        printf("\n");
        free(r.data);
    } else {
        printf("处理失败\n");
    }

    printf("\ngoto cleanup优势:\n");
    printf("  1. 资源始终被正确释放\n");
    printf("  2. 不会跳过清理代码\n");
    printf("  3. 代码流清晰可读\n");
    printf("  4. 是Linux内核的标准错误处理模式\n");
}

int main(void) {
    printf("异常处理: setjmp/longjmp实现异常机制\n");

    demo_basic_exception();
    demo_nested_exception();
    demo_exception_pitfalls();
    demo_error_code_pattern();

    printf("\n所有演示完成!\n");
    return 0;
}
