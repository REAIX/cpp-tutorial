#ifndef CU_UTILS_CONFIG_UTILS_H
#define CU_UTILS_CONFIG_UTILS_H

#include "cu_utils/export.h"
#include <string>
#include <map>

namespace cu {

class CXXU_API ConfigUtils {
public:
    ConfigUtils() = delete;
    ConfigUtils(const ConfigUtils&) = delete;
    ConfigUtils& operator=(const ConfigUtils&) = delete;

    static std::map<std::string, std::string> load(const std::string& path);
    static std::string get(const std::map<std::string, std::string>& config, const std::string& key, const std::string& defaultValue = "");
    static void set(std::map<std::string, std::string>& config, const std::string& key, const std::string& value);
    static bool save(const std::string& path, const std::map<std::string, std::string>& config);
};

}

#endif
