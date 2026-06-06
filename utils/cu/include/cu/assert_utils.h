#ifndef CU_ASSERT_UTILS_H
#define CU_ASSERT_UTILS_H

#include "cu/export.h"

/*
 * 注意：CU_API 不是 C 语言关键字，而是一个宏定义。
 * 它由 cu/export.h 通过 #define 定义，用于跨平台控制函数的可见性：
 *   - Windows 下展开为 __declspec(dllexport) 或 __declspec(dllimport)
 *   - Linux/GCC 下展开为 __attribute__((visibility("default")))
 *   - 静态链接时展开为空
 * 使用者无需手动定义，只需包含头文件即可。
 */
CU_API void cu_assert_fail(const char* expr, const char* file, int line, const char* msg);
CU_API int cu_check(int cond, const char* msg);

#define CU_ASSERT(expr, msg) do { if (!(expr)) cu_assert_fail(#expr, __FILE__, __LINE__, msg); } while(0)
#define CU_CHECK(cond, msg) cu_check(cond, msg)

#endif
