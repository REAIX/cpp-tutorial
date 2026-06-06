#ifndef CU_UTILS_HASH_UTILS_H
#define CU_UTILS_HASH_UTILS_H

#include "cu_utils/export.h"
#include <string>
#include <cstdint>

namespace cu {

class CXXU_API HashUtils {
public:
    HashUtils() = delete;
    HashUtils(const HashUtils&) = delete;
    HashUtils& operator=(const HashUtils&) = delete;

    static std::string md5(const std::string& data);
    static std::string sha256(const std::string& data);
    static std::string fileMd5(const std::string& path);
    static uint32_t crc32(const std::string& data);
};

}

#endif
