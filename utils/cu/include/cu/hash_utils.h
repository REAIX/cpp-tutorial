#ifndef CU_HASH_UTILS_H
#define CU_HASH_UTILS_H

#include "cu/export.h"
#include <stddef.h>

CU_API char* md5_hash(const char* data, size_t data_len);
CU_API char* sha256_hash(const char* data, size_t data_len);
CU_API char* file_md5(const char* path);
CU_API unsigned int crc32_hash(const char* data, size_t data_len);

#endif
