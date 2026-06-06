#ifndef CU_ASSERT_UTILS_H
#define CU_ASSERT_UTILS_H

#include "cu/export.h"

CU_API void cu_assert_fail(const char* expr, const char* file, int line, const char* msg);
CU_API int cu_check(int cond, const char* msg);

#define CU_ASSERT(expr, msg) do { if (!(expr)) cu_assert_fail(#expr, __FILE__, __LINE__, msg); } while(0)
#define CU_CHECK(cond, msg) cu_check(cond, msg)

#endif
