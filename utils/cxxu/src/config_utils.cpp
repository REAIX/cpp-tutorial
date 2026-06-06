#include "cu_utils/config_utils.h"
#include <fstream>
#include <algorithm>

namespace cu {

static std::string trim(const std::string& s) {
    size_t start = s.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) return "";
    size_t end = s.find_last_not_of(" \t\r\n");
    return s.substr(start, end - start + 1);
}

std::map<std::string, std::string> ConfigUtils::load(const std::string& path) {
    std::map<std::string, std::string> config;
    std::ifstream file(path);
    if (!file.is_open()) return config;

    std::string line;
    while (std::getline(file, line)) {
        line = trim(line);
        if (line.empty() || line[0] == '#') continue;

        size_t pos = line.find('=');
        if (pos == std::string::npos) continue;

        std::string key = trim(line.substr(0, pos));
        std::string value = trim(line.substr(pos + 1));
        config[key] = value;
    }

    return config;
}

std::string ConfigUtils::get(const std::map<std::string, std::string>& config, const std::string& key, const std::string& defaultValue) {
    auto it = config.find(key);
    if (it != config.end()) return it->second;
    return defaultValue;
}

void ConfigUtils::set(std::map<std::string, std::string>& config, const std::string& key, const std::string& value) {
    config[key] = value;
}

bool ConfigUtils::save(const std::string& path, const std::map<std::string, std::string>& config) {
    std::ofstream file(path);
    if (!file.is_open()) return false;

    for (const auto& [key, value] : config) {
        file << key << "=" << value << "\n";
    }

    return true;
}

}
