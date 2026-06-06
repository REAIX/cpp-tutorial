/**
 * @file 01_example_vcpkg_conan.cpp
 * @brief 包管理器概念: vcpkg/Conan, 依赖声明, CMake集成
 * @description 对应文档: 02-CPP/37-包管理工具
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <functional>
#include <cmath>
#include <algorithm>
#include <numeric>
#include <sstream>

namespace simulated {
    namespace json_lib {
        struct JsonValue {
            std::map<std::string, std::string> data;

            void set(const std::string& key, const std::string& val) {
                data[key] = val;
            }

            void set(const std::string& key, int val) {
                data[key] = std::to_string(val);
            }

            void set(const std::string& key, double val) {
                data[key] = std::to_string(val);
            }

            std::string get(const std::string& key, const std::string& default_val = "") const {
                auto it = data.find(key);
                return (it != data.end()) ? it->second : default_val;
            }

            std::string dump() const {
                std::string result = "{";
                bool first = true;
                for (const auto& [k, v] : data) {
                    if (!first) result += ",";
                    result += "\"" + k + "\":\"" + v + "\"";
                    first = false;
                }
                result += "}";
                return result;
            }
        };
    }

    namespace http_lib {
        struct HttpResponse {
            int status_code;
            std::string body;
            std::map<std::string, std::string> headers;
        };

        class HttpClient {
        public:
            HttpResponse get(const std::string& url) {
                std::cout << "  [模拟HTTP] GET " << url << "\n";
                return {200, "{\"status\":\"ok\"}", {{"Content-Type", "application/json"}}};
            }

            HttpResponse post(const std::string& url, const std::string& body) {
                std::cout << "  [模拟HTTP] POST " << url << " body=" << body << "\n";
                return {201, "{\"id\":1}", {{"Content-Type", "application/json"}}};
            }
        };
    }

    namespace fmt_lib {
        template<typename... Args>
        std::string format(const std::string& fmt, Args... args) {
            std::ostringstream oss;
            ((oss << args << " "), ...);
            return fmt + " " + oss.str();
        }

        void print(const std::string& msg) {
            std::cout << msg;
        }
    }
}

void demo_vcpkg_concept() {
    std::cout << "\n=== demo_vcpkg_concept ===\n";
    std::cout << "vcpkg: 微软开源的C++包管理器\n\n";

    std::cout << "vcpkg基本使用:\n";
    std::cout << "  # 安装vcpkg\n";
    std::cout << "  git clone https://github.com/microsoft/vcpkg\n";
    std::cout << "  ./vcpkg/bootstrap-vcpkg.sh\n\n";

    std::cout << "  # 安装库\n";
    std::cout << "  ./vcpkg/vcpkg install nlohmann-json\n";
    std::cout << "  ./vcpkg/vcpkg install fmt\n";
    std::cout << "  ./vcpkg/vcpkg install cpp-httplib\n\n";

    std::cout << "  # CMake集成\n";
    std::cout << "  cmake -B build -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake\n\n";

    std::cout << "  # CMakeLists.txt中:\n";
    std::cout << "  find_package(nlohmann_json CONFIG REQUIRED)\n";
    std::cout << "  find_package(fmt CONFIG REQUIRED)\n";
    std::cout << "  target_link_libraries(myapp PRIVATE nlohmann_json::nlohmann_json fmt::fmt)\n\n";

    std::cout << "  # vcpkg.json (清单模式):\n";
    std::cout << "  {\n";
    std::cout << "    \"name\": \"myapp\",\n";
    std::cout << "    \"version\": \"1.0.0\",\n";
    std::cout << "    \"dependencies\": [\"nlohmann-json\", \"fmt\", \"cpp-httplib\"]\n";
    std::cout << "  }\n\n";

    std::cout << "vcpkg特点:\n";
    std::cout << "  1. 源码编译, 确保兼容性\n";
    std::cout << "  2. 清单模式(vcpkg.json)声明依赖\n";
    std::cout << "  3. 与CMake深度集成\n";
    std::cout << "  4. 支持多平台(Windows/Linux/macOS)\n";
    std::cout << "  5. 2000+可用包\n";
}

void demo_conan_concept() {
    std::cout << "\n=== demo_conan_concept ===\n";
    std::cout << "Conan: 去中心化的C++包管理器\n\n";

    std::cout << "Conan基本使用:\n";
    std::cout << "  # 安装Conan\n";
    std::cout << "  pip install conan\n\n";

    std::cout << "  # conanfile.txt:\n";
    std::cout << "  [requires]\n";
    std::cout << "  nlohmann_json/3.11.2\n";
    std::cout << "  fmt/10.1.1\n";
    std::cout << "  cpp-httplib/0.14.1\n\n";

    std::cout << "  [generators]\n";
    std::cout << "  CMakeDeps\n";
    std::cout << "  CMakeToolchain\n\n";

    std::cout << "  # 安装依赖\n";
    std::cout << "  conan install . --output-folder=build --build=missing\n\n";

    std::cout << "  # conanfile.py (更灵活):\n";
    std::cout << "  from conan import ConanFile\n";
    std::cout << "  class MyAppConan(ConanFile):\n";
    std::cout << "      settings = \"os\", \"compiler\", \"build_type\", \"arch\"\n";
    std::cout << "      requires = \"nlohmann_json/3.11.2\", \"fmt/10.1.1\"\n";
    std::cout << "      generators = \"CMakeDeps\", \"CMakeToolchain\"\n\n";

    std::cout << "Conan特点:\n";
    std::cout << "  1. 去中心化, 支持私有仓库\n";
    std::cout << "  2. 预编译二进制包 (更快)\n";
    std::cout << "  3. 支持多种构建系统\n";
    std::cout << "  4. 灵活的版本范围约束\n";
    std::cout << "  5. Python脚本定制构建流程\n\n";

    std::cout << "vcpkg vs Conan:\n";
    std::cout << "  ┌──────────┬──────────────┬──────────────┐\n";
    std::cout << "  │ 特性     │ vcpkg        │ Conan        │\n";
    std::cout << "  ├──────────┼──────────────┼──────────────┤\n";
    std::cout << "  │ 仓库     │ 集中         │ 去中心化     │\n";
    std::cout << "  │ 编译     │ 源码编译     │ 预编译+源码  │\n";
    std::cout << "  │ 构建系统 │ CMake为主    │ 多种支持     │\n";
    std::cout << "  │ 配置语言 │ JSON         │ Python       │\n";
    std::cout << "  │ 学习曲线 │ 低           │ 中           │\n";
    std::cout << "  │ 平台     │ 全平台       │ 全平台       │\n";
    std::cout << "  └──────────┴──────────────┴──────────────┘\n";
}

void demo_simulated_third_party_usage() {
    std::cout << "\n=== demo_simulated_third_party_usage ===\n";
    std::cout << "模拟使用第三方库 (实际项目中通过包管理器安装)\n\n";

    std::cout << "1. 模拟nlohmann/json:\n";
    simulated::json_lib::JsonValue config;
    config.set("host", "localhost");
    config.set("port", 8080);
    config.set("timeout", 30);
    config.set("debug", "true");
    std::cout << "  配置JSON: " << config.dump() << "\n";
    std::cout << "  读取host: " << config.get("host") << "\n";
    std::cout << "  读取port: " << config.get("port") << "\n\n";

    std::cout << "  实际使用nlohmann/json:\n";
    std::cout << "    #include <nlohmann/json.hpp>\n";
    std::cout << "    nlohmann::json config;\n";
    std::cout << "    config[\"host\"] = \"localhost\";\n";
    std::cout << "    config[\"port\"] = 8080;\n";
    std::cout << "    std::string s = config.dump();\n\n";

    std::cout << "2. 模拟cpp-httplib:\n";
    simulated::http_lib::HttpClient client;
    auto response = client.get("https://api.example.com/users");
    std::cout << "  响应码: " << response.status_code << "\n";
    std::cout << "  响应体: " << response.body << "\n\n";

    std::cout << "  实际使用cpp-httplib:\n";
    std::cout << "    #include <httplib.h>\n";
    std::cout << "    httplib::Client cli(\"https://api.example.com\");\n";
    std::cout << "    auto res = cli.Get(\"/users\");\n";
    std::cout << "    std::cout << res->status << std::endl;\n\n";

    std::cout << "3. 模拟fmt:\n";
    std::string msg = simulated::fmt_lib::format("Hello", "World", 42, 3.14);
    std::cout << "  格式化输出: " << msg << "\n\n";

    std::cout << "  实际使用fmt:\n";
    std::cout << "    #include <fmt/core.h>\n";
    std::cout << "    fmt::print(\"Hello {} {}\", name, age);\n";

    std::cout << "\nCMakeLists.txt完整示例:\n";
    std::cout << "  cmake_minimum_required(VERSION 3.15)\n";
    std::cout << "  project(myapp VERSION 1.0.0)\n\n";
    std::cout << "  find_package(nlohmann_json CONFIG REQUIRED)\n";
    std::cout << "  find_package(fmt CONFIG REQUIRED)\n";
    std::cout << "  find_package(httplib CONFIG REQUIRED)\n\n";
    std::cout << "  add_executable(myapp main.cpp)\n";
    std::cout << "  target_link_libraries(myapp PRIVATE\n";
    std::cout << "      nlohmann_json::nlohmann_json\n";
    std::cout << "      fmt::fmt\n";
    std::cout << "      httplib::httplib\n";
    std::cout << "  )\n";
}

int main() {
    std::cout << "C++包管理器概念演示\n";

    demo_vcpkg_concept();
    demo_conan_concept();
    demo_simulated_third_party_usage();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
