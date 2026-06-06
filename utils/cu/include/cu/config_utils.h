#ifndef CU_CONFIG_UTILS_H
#define CU_CONFIG_UTILS_H

#include "cu/export.h"
#include "cu/collection_utils.h"

typedef struct {
    char** keys;
    char** values;
    size_t size;
    size_t capacity;
} ConfigMap;

CU_API ConfigMap* config_load(const char* path);
CU_API const char* config_get(const ConfigMap* config, const char* key, const char* default_value);
CU_API void config_set(ConfigMap* config, const char* key, const char* value);
CU_API int config_save(const ConfigMap* config, const char* path);
CU_API void config_destroy(ConfigMap* config);

#endif
