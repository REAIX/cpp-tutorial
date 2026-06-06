#include "cu/config_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static char* trim(char* str) {
    while (*str && isspace((unsigned char)*str)) str++;
    if (*str == '\0') return str;
    char* end = str + strlen(str) - 1;
    while (end > str && isspace((unsigned char)*end)) {
        *end = '\0';
        end--;
    }
    return str;
}

static void config_map_grow(ConfigMap* config) {
    if (config->size >= config->capacity) {
        size_t new_cap = config->capacity == 0 ? 16 : config->capacity * 2;
        char** new_keys = (char**)realloc(config->keys, new_cap * sizeof(char*));
        char** new_vals = (char**)realloc(config->values, new_cap * sizeof(char*));
        if (!new_keys || !new_vals) return;
        config->keys = new_keys;
        config->values = new_vals;
        config->capacity = new_cap;
    }
}

ConfigMap* config_load(const char* path) {
    FILE* fp = fopen(path, "r");
    if (!fp) return NULL;

    ConfigMap* config = (ConfigMap*)calloc(1, sizeof(ConfigMap));
    if (!config) {
        fclose(fp);
        return NULL;
    }

    char line[1024];
    while (fgets(line, sizeof(line), fp)) {
        char* trimmed = trim(line);
        if (*trimmed == '\0' || *trimmed == '#') continue;

        char* eq = strchr(trimmed, '=');
        if (!eq) continue;

        *eq = '\0';
        char* key = trim(trimmed);
        char* val = trim(eq + 1);

        config_set(config, key, val);
    }

    fclose(fp);
    return config;
}

const char* config_get(const ConfigMap* config, const char* key, const char* default_value) {
    if (!config || !key) return default_value;

    for (size_t i = 0; i < config->size; i++) {
        if (strcmp(config->keys[i], key) == 0) {
            return config->values[i];
        }
    }
    return default_value;
}

void config_set(ConfigMap* config, const char* key, const char* value) {
    if (!config || !key || !value) return;

    for (size_t i = 0; i < config->size; i++) {
        if (strcmp(config->keys[i], key) == 0) {
            char* new_val = strdup(value);
            if (new_val) {
                free(config->values[i]);
                config->values[i] = new_val;
            }
            return;
        }
    }

    config_map_grow(config);
    if (config->size >= config->capacity) return;

    config->keys[config->size] = strdup(key);
    config->values[config->size] = strdup(value);
    if (config->keys[config->size] && config->values[config->size]) {
        config->size++;
    }
}

int config_save(const ConfigMap* config, const char* path) {
    if (!config || !path) return -1;

    FILE* fp = fopen(path, "w");
    if (!fp) return -1;

    for (size_t i = 0; i < config->size; i++) {
        fprintf(fp, "%s=%s\n", config->keys[i], config->values[i]);
    }

    fclose(fp);
    return 0;
}

void config_destroy(ConfigMap* config) {
    if (!config) return;

    for (size_t i = 0; i < config->size; i++) {
        free(config->keys[i]);
        free(config->values[i]);
    }
    free(config->keys);
    free(config->values);
    free(config);
}
