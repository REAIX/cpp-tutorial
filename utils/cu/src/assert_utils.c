#include "cu/assert_utils.h"
#include <stdio.h>
#include <stdlib.h>

void cu_assert_fail(const char* expr, const char* file, int line, const char* msg) {
    fprintf(stderr, "ASSERTION FAILED: %s at %s:%d: %s\n", expr, file, line, msg ? msg : "");
    abort();
}

int cu_check(int cond, const char* msg) {
    if (!cond) {
        fprintf(stderr, "CHECK FAILED: %s\n", msg ? msg : "");
        return 0;
    }
    return 1;
}
