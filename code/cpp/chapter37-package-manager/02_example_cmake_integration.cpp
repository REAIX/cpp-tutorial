/**
 * @file 02_example_cmake_integration.cpp
 * @brief CMake与包管理器集成: find_package, FetchContent, vcpkg/Conan/CPM集成, 现代CMake模式
 * @description 对应文档: 02-CPP/37-包管理工具
 * 编译命令: g++ -std=c++20 -o 02_example_cmake_integration 02_example_cmake_integration.cpp
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <functional>
#include <sstream>
#include <algorithm>

// ============================================================================
// 模拟的CMake目标属性结构 (用于演示target-based概念)
// ============================================================================
struct CMakeTarget {
    std::string name;
    std::string type; // executable, static_library, shared_library, interface
    std::vector<std::string> sources;
    std::vector<std::string> include_dirs;
    std::vector<std::string> link_libraries;
    std::vector<std::string> compile_features;
    std::vector<std::string> compile_definitions;
    std::vector<std::string> alias_names;

    std::string summary() const {
        std::ostringstream oss;
        oss << "  目标: " << name << " (" << type << ")\n";
        if (!sources.empty()) {
            oss << "    源文件: ";
            for (size_t i = 0; i < sources.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << sources[i];
            }
            oss << "\n";
        }
        if (!include_dirs.empty()) {
            oss << "    包含目录: ";
            for (size_t i = 0; i < include_dirs.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << include_dirs[i];
            }
            oss << "\n";
        }
        if (!link_libraries.empty()) {
            oss << "    链接库: ";
            for (size_t i = 0; i < link_libraries.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << link_libraries[i];
            }
            oss << "\n";
        }
        if (!compile_features.empty()) {
            oss << "    编译特性: ";
            for (size_t i = 0; i < compile_features.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << compile_features[i];
            }
            oss << "\n";
        }
        if (!compile_definitions.empty()) {
            oss << "    编译定义: ";
            for (size_t i = 0; i < compile_definitions.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << compile_definitions[i];
            }
            oss << "\n";
        }
        if (!alias_names.empty()) {
            oss << "    别名: ";
            for (size_t i = 0; i < alias_names.size(); ++i) {
                if (i > 0) oss << ", ";
                oss << alias_names[i];
            }
            oss << "\n";
        }
        return oss.str();
    }
};

// ============================================================================
// Demo 1: find_package 模式
// ============================================================================
void demo_find_package_pattern() {
    std::cout << "\n═══════════════════════════════════════════════════════════\n";
    std::cout << "=== demo_find_package_pattern ===\n";
    std::cout << "═══════════════════════════════════════════════════════════\n";
    std::cout << "CMake find_package: 查找并加载外部依赖\n\n";

    std::cout << "find_package两种模式:\n\n";

    std::cout << "1. Module模式 (FindXXX.cmake):\n";
    std::cout << "   CMake搜索CMAKE_MODULE_PATH中的FindXXX.cmake模块\n";
    std::cout << "   适用于: 没有提供CMake配置文件的第三方库\n\n";

    std::cout << "   find_package(OpenSSL REQUIRED)\n";
    std::cout << "   # CMake查找: cmake/FindOpenSSL.cmake\n\n";

    std::cout << "2. Config模式 (XXXConfig.cmake):\n";
    std::cout << "   CMake搜索包自带的XXXConfig.cmake或xxx-config.cmake\n";
    std::cout << "   适用于: 现代CMake项目, 包管理器安装的库\n\n";

    std::cout << "   find_package(fmt CONFIG REQUIRED)\n";
    std::cout << "   # CMake查找: fmt-config.cmake 或 fmtConfig.cmake\n\n";

    std::cout << "find_package常用参数:\n";
    std::cout << "  find_package(<包名> [版本] [EXACT] [REQUIRED]\n";
    std::cout << "              [COMPONENTS <组件...>]\n";
    std::cout << "              [OPTIONAL_COMPONENTS <组件...>])\n\n";

    std::cout << "示例:\n";
    std::cout << "  find_package(fmt 10.1 REQUIRED)           # 版本>=10.1\n";
    std::cout << "  find_package(Boost 1.82 REQUIRED\n";
    std::cout << "      COMPONENTS filesystem system)          # 指定组件\n";
    std::cout << "  find_package(Qt6 REQUIRED\n";
    std::cout << "      COMPONENTS Core Widgets)               # Qt组件\n";
    std::cout << "  find_package(OpenSSL OPTIONAL_COMPONENTS)  # 可选依赖\n\n";

    std::cout << "查找路径搜索顺序:\n";
    std::cout << "  1. CMAKE_PREFIX_PATH (包管理器通常设置此变量)\n";
    std::cout << "  2. 标准系统路径 (/usr/local, /usr)\n";
    std::cout << "  3. <包名>_ROOT 环境变量\n";
    std::cout << "  4. CMAKE_MODULE_PATH (Module模式)\n";
    std::cout << "  5. vcpkg/Conan安装路径 (通过工具链文件注入)\n\n";

    std::cout << "find_package成功后导入的目标命名规范:\n";
    std::cout << "  包名::目标名  (双冒号命名空间)\n";
    std::cout << "  例: fmt::fmt, nlohmann_json::nlohmann_json\n";
    std::cout << "  例: Boost::filesystem, Boost::system\n";
    std::cout << "  例: Qt6::Core, Qt6::Widgets\n";
}

// ============================================================================
// Demo 2: FetchContent
// ============================================================================
void demo_fetch_content() {
    std::cout << "\n═══════════════════════════════════════════════════════════\n";
    std::cout << "=== demo_fetch_content ===\n";
    std::cout << "═══════════════════════════════════════════════════════════\n";
    std::cout << "FetchContent: CMake内置的依赖下载机制 (CMake 3.11+)\n\n";

    std::cout << "基本用法:\n";
    std::cout << "  include(FetchContent)\n\n";

    std::cout << "  # 声明要获取的内容\n";
    std::cout << "  FetchContent_Declare(\n";
    std::cout << "    googletest\n";
    std::cout << "    GIT_REPOSITORY https://github.com/google/googletest.git\n";
    std::cout << "    GIT_TAG        v1.14.0\n";
    std::cout << "  )\n\n";

    std::cout << "  # 获取并使其可用\n";
    std::cout << "  FetchContent_MakeAvailable(googletest)\n\n";

    std::cout << "  # 然后就可以像普通目标一样使用\n";
    std::cout << "  target_link_libraries(myapp PRIVATE GTest::gtest_main)\n\n";

    std::cout << "不同来源的声明方式:\n\n";

    std::cout << "  # Git仓库\n";
    std::cout << "  FetchContent_Declare(\n";
    std::cout << "    json_lib\n";
    std::cout << "    GIT_REPOSITORY https://github.com/nlohmann/json.git\n";
    std::cout << "    GIT_TAG        v3.11.2\n";
    std::cout << "  )\n\n";

    std::cout << "  # URL下载\n";
    std::cout << "  FetchContent_Declare(\n";
    std::cout << "    json_lib\n";
    std::cout << "    URL      https://github.com/nlohmann/json/releases/download/v3.11.2/json.tar.xz\n";
    std::cout << "    URL_HASH SHA256=...  # 推荐校验哈希\n";
    std::cout << "  )\n\n";

    std::cout << "FetchContent特点:\n";
    std::cout << "  1. CMake内置, 无需额外工具\n";
    std::cout << "  2. 在配置时下载, 下载后缓存\n";
    std::cout << "  3. 依赖作为子项目参与构建\n";
    std::cout << "  4. 适合小型项目和快速原型\n\n";

    std::cout << "FetchContent注意事项:\n";
    std::cout << "  1. 每次配置都会检查远程 (可设置FETCHCONTENT_UPDATES_DISCONNECTED)\n";
    std::cout << "  2. 没有锁文件, 构建可重现性依赖GIT_TAG\n";
    std::cout << "  3. 依赖的CMakeLists.txt在你的项目中执行\n";
    std::cout << "  4. 可能与你项目的设置冲突 (如CMAKE_CXX_STANDARD)\n";
    std::cout << "  5. 大型项目推荐使用vcpkg/Conan代替\n\n";

    std::cout << "FetchContent vs 包管理器:\n";
    std::cout << "  ┌────────────┬──────────────┬──────────────┐\n";
    std::cout << "  │ 特性       │ FetchContent │ 包管理器     │\n";
    std::cout << "  ├────────────┼──────────────┼──────────────┤\n";
    std::cout << "  │ 依赖缓存   │ 本地缓存     │ 全局缓存     │\n";
    std::cout << "  │ 二进制缓存 │ 无           │ 有(vcpkg/Conan)│\n";
    std::cout << "  │ 锁文件     │ 无           │ 有           │\n";
    std::cout << "  │ 传递依赖   │ 自动         │ 自动         │\n";
    std::cout << "  │ 版本范围   │ 精确标签     │ 语义版本范围 │\n";
    std::cout << "  │ 适用规模   │ 小型项目     │ 中大型项目   │\n";
    std::cout << "  └────────────┴──────────────┴──────────────┘\n";
}

// ============================================================================
// Demo 3: vcpkg集成CMake
// ============================================================================
void demo_vcpkg_cmake_integration() {
    std::cout << "\n═══════════════════════════════════════════════════════════\n";
    std::cout << "=== demo_vcpkg_cmake_integration ===\n";
    std::cout << "═══════════════════════════════════════════════════════════\n";
    std::cout << "vcpkg与CMake集成: CMAKE_TOOLCHAIN_FILE方式\n\n";

    std::cout << "方式1: 命令行传递工具链文件\n";
    std::cout << "  cmake -B build \\\n";
    std::cout << "    -DCMAKE_TOOLCHAIN_FILE=<vcpkg_root>/scripts/buildsystems/vcpkg.cmake\n\n";

    std::cout << "方式2: CMakePresets.json (推荐)\n";
    std::cout << "  {\n";
    std::cout << "    \"version\": 3,\n";
    std::cout << "    \"configurePresets\": [\n";
    std::cout << "      {\n";
    std::cout << "        \"name\": \"vcpkg\",\n";
    std::cout << "        \"cacheVariables\": {\n";
    std::cout << "          \"CMAKE_TOOLCHAIN_FILE\": \"$env{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake\"\n";
    std::cout << "        }\n";
    std::cout << "      }\n";
    std::cout << "    ]\n";
    std::cout << "  }\n\n";

    std::cout << "方式3: 环境变量\n";
    std::cout << "  set VCPKG_ROOT=C:/vcpkg\n";
    std::cout << "  cmake -B build -DCMAKE_TOOLCHAIN_FILE=%VCPKG_ROOT%/scripts/buildsystems/vcpkg.cmake\n\n";

    std::cout << "清单模式 (vcpkg.json) - 推荐:\n";
    std::cout << "  项目根目录放置vcpkg.json:\n";
    std::cout << "  {\n";
    std::cout << "    \"name\": \"myapp\",\n";
    std::cout << "    \"version\": \"1.0.0\",\n";
    std::cout << "    \"dependencies\": [\n";
    std::cout << "      \"nlohmann-json\",\n";
    std::cout << "      \"fmt\",\n";
    std::cout << "      {\"name\": \"cpp-httplib\", \"features\": [\"openssl\"]}\n";
    std::cout << "    ],\n";
    std::cout << "    \"overrides\": [],\n";
    std::cout << "    \"builtin-baseline\": \"<git-commit-hash>\"\n";
    std::cout << "  }\n\n";

    std::cout << "vcpkg.json中的版本约束:\n";
    std::cout << "  \"dependencies\": [\n";
    std::cout << "    {\"name\": \"fmt\", \"version>=\": \"10.1.0\"},\n";
    std::cout << "    {\"name\": \"nlohmann-json\", \"version>=\": \"3.11.0\"}\n";
    std::cout << "  ]\n\n";

    std::cout << "CMakeLists.txt中使用vcpkg安装的库:\n";
    std::cout << "  cmake_minimum_required(VERSION 3.15)\n";
    std::cout << "  project(myapp VERSION 1.0.0 LANGUAGES CXX)\n\n";
    std::cout << "  # vcpkg通过工具链文件自动设置搜索路径\n";
    std::cout << "  find_package(fmt CONFIG REQUIRED)\n";
    std::cout << "  find_package(nlohmann_json CONFIG REQUIRED)\n";
    std::cout << "  find_package(httplib CONFIG REQUIRED)\n\n";
    std::cout << "  add_executable(myapp main.cpp)\n\n";
    std::cout << "  target_link_libraries(myapp PRIVATE\n";
    std::cout << "      fmt::fmt\n";
    std::cout << "      nlohmann_json::nlohmann_json\n";
    std::cout << "      httplib::httplib\n";
    std::cout << "  )\n\n";

    std::cout << "vcpkg工具链文件做了什么:\n";
    std::cout << "  1. 设置CMAKE_PREFIX_PATH指向vcpkg安装目录\n";
    std::cout << "  2. 自动处理清单模式(vcpkg.json)的依赖安装\n";
    std::cout << "  3. 为find_package提供正确的搜索路径\n";
    std::cout << "  4. 处理triplet (x64-windows, x64-linux等)\n";
    std::cout << "  5. 管理动态库的DLL复制(Windows)\n";
}

// ============================================================================
// Demo 4: Conan集成CMake
// ============================================================================
void demo_conan_cmake_integration() {
    std::cout << "\n═══════════════════════════════════════════════════════════\n";
    std::cout << "=== demo_conan_cmake_integration ===\n";
    std::cout << "═══════════════════════════════════════════════════════════\n";
    std::cout << "Conan与CMake集成: CMakeDeps + CMakeToolchain生成器\n\n";

    std::cout << "步骤1: 编写conanfile.txt (或conanfile.py)\n";
    std::cout << "  [requires]\n";
    std::cout << "  fmt/10.1.1\n";
    std::cout << "  nlohmann_json/3.11.2\n";
    std::cout << "  cpp-httplib/0.14.1\n\n";

    std::cout << "  [generators]\n";
    std::cout << "  CMakeDeps      # 生成XXXConfig.cmake文件\n";
    std::cout << "  CMakeToolchain  # 生成工具链文件\n\n";

    std::cout << "步骤2: 安装依赖\n";
    std::cout << "  conan install . \\\n";
    std::cout << "    --output-folder=build \\\n";
    std::cout << "    --build=missing\n\n";

    std::cout << "步骤3: 使用Conan生成的工具链配置CMake\n";
    std::cout << "  cmake -B build -S . \\\n";
    std::cout << "    -DCMAKE_TOOLCHAIN_FILE=build/conan_toolchain.cmake \\\n";
    std::cout << "    -DCMAKE_BUILD_TYPE=Release\n\n";

    std::cout << "步骤4: CMakeLists.txt照常使用find_package\n";
    std::cout << "  find_package(fmt REQUIRED)\n";
    std::cout << "  find_package(nlohmann_json REQUIRED)\n\n";
    std::cout << "  target_link_libraries(myapp PRIVATE fmt::fmt nlohmann_json::nlohmann_json)\n\n";

    std::cout << "使用conanfile.py (更灵活):\n";
    std::cout << "  from conan import ConanFile\n\n";
    std::cout << "  class MyAppConan(ConanFile):\n";
    std::cout << "      settings = \"os\", \"compiler\", \"build_type\", \"arch\"\n";
    std::cout << "      requires = \"fmt/10.1.1\", \"nlohmann_json/3.11.2\"\n";
    std::cout << "      generators = \"CMakeDeps\", \"CMakeToolchain\"\n\n";
    std::cout << "      def requirements(self):\n";
    std::cout << "          # 根据条件添加依赖\n";
    std::cout << "          if self.settings.os == \"Windows\":\n";
    std::cout << "              self.requires(\"winsock/1.0\")\n\n";

    std::cout << "Conan 2.x的新生成器 (推荐):\n";
    std::cout << "  CMakeDeps: 生成CMake配置文件, 替代旧版cmake_find_package\n";
    std::cout << "  CMakeToolchain: 生成工具链文件, 替代旧版cmake_paths\n\n";

    std::cout << "Conan vs vcpkg CMake集成对比:\n";
    std::cout << "  ┌──────────────┬──────────────────┬──────────────────┐\n";
    std::cout << "  │ 方面         │ vcpkg            │ Conan            │\n";
    std::cout << "  ├──────────────┼──────────────────┼──────────────────┤\n";
    std::cout << "  │ 集成方式     │ CMAKE_TOOLCHAIN_FILE │ CMAKE_TOOLCHAIN_FILE │\n";
    std::cout << "  │ 配置文件     │ vcpkg.json       │ conanfile.txt/py │\n";
    std::cout << "  │ 依赖安装时机 │ CMake配置时自动  │ 需先conan install│\n";
    std::cout << "  │ find_package │ CONFIG模式       │ CONFIG模式       │\n";
    std::cout << "  │ 二进制缓存   │ 本地+远程        │ 本地+远程+私有   │\n";
    std::cout << "  └──────────────┴──────────────────┴──────────────────┘\n";
}

// ============================================================================
// Demo 5: CPM.cmake
// ============================================================================
void demo_cpm_cmake() {
    std::cout << "\n═══════════════════════════════════════════════════════════\n";
    std::cout << "=== demo_cpm_cmake ===\n";
    std::cout << "═══════════════════════════════════════════════════════════\n";
    std::cout << "CPM.cmake: 轻量级CMake依赖管理 (FetchContent的增强封装)\n\n";

    std::cout << "安装CPM.cmake (只需一个文件):\n";
    std::cout << "  # 在CMakeLists.txt开头\n";
    std::cout << "  include(cmake/CPM.cmake)\n\n";

    std::cout << "  # 或自动下载\n";
    std::cout << "  set(CPM_DOWNLOAD_VERSION 0.38.5)\n";
    std::cout << "  file(DOWNLOAD\n";
    std::cout << "    https://github.com/cpm-cmake/CPM.cmake/releases/download/v${CPM_DOWNLOAD_VERSION}/CPM.cmake\n";
    std::cout << "    ${CMAKE_BINARY_DIR}/cmake/CPM.cmake\n";
    std::cout << "  )\n";
    std::cout << "  include(${CMAKE_BINARY_DIR}/cmake/CPM.cmake)\n\n";

    std::cout << "使用CPM添加依赖:\n";
    std::cout << "  # Git仓库\n";
    std::cout << "  CPMAddPackage(\n";
    std::cout << "    NAME nlohmann_json\n";
    std::cout << "    GITHUB_REPOSITORY nlohmann/json\n";
    std::cout << "    VERSION 3.11.2\n";
    std::cout << "  )\n\n";

    std::cout << "  # 另一种简写\n";
    std::cout << "  CPMAddPackage(\"gh:nlohmann/json@3.11.2\")\n\n";

    std::cout << "  # URL下载\n";
    std::cout << "  CPMAddPackage(\n";
    std::cout << "    NAME fmt\n";
    std::cout << "    URL https://github.com/fmtlib/fmt/releases/download/10.1.1/fmt-10.1.1.zip\n";
    std::cout << "    VERSION 10.1.1\n";
    std::cout << "  )\n\n";

    std::cout << "  # 带选项\n";
    std::cout << "  CPMAddPackage(\n";
    std::cout << "    NAME googletest\n";
    std::cout << "    GITHUB_REPOSITORY google/googletest\n";
    std::cout << "    VERSION 1.14.0\n";
    std::cout << "    OPTIONS\n";
    std::cout << "      \"BUILD_GMOCK OFF\"\n";
    std::cout << "      \"INSTALL_GTEST OFF\"\n";
    std::cout << "  )\n\n";

    std::cout << "CPM.cmake优势:\n";
    std::cout << "  1. 零安装: 只需一个.cmake文件\n";
    std::cout << "  2. FetchContent增强: 添加版本控制和缓存\n";
    std::cout << "  3. 不会重复下载: 多项目共享已下载的包\n";
    std::cout << "  4. 支持包已通过find_package找到时跳过下载\n";
    std::cout << "  5. 简洁的语法\n\n";

    std::cout << "CPM与find_package协同:\n";
    std::cout << "  CPMAddPackage(\n";
    std::cout << "    NAME fmt\n";
    std::cout << "    GITHUB_REPOSITORY fmtlib/fmt\n";
    std::cout << "    VERSION 10.1.1\n";
    std::cout << "    FIND_PACKAGE_ARGUMENTS REQUIRED  # 如果已安装则跳过下载\n";
    std::cout << "  )\n\n";

    std::cout << "适用场景:\n";
    std::cout << "  1. 小型项目, 不想引入完整包管理器\n";
    std::cout << "  2. CI/CD中快速获取依赖\n";
    std::cout << "  3. 原型开发阶段\n";
    std::cout << "  4. header-only库的快速集成\n";
}

// ============================================================================
// Demo 6: 自定义FindXXX.cmake模块
// ============================================================================
void demo_custom_find_module() {
    std::cout << "\n═══════════════════════════════════════════════════════════\n";
    std::cout << "=== demo_custom_find_module ===\n";
    std::cout << "═══════════════════════════════════════════════════════════\n";
    std::cout << "创建自定义FindXXX.cmake模块\n\n";

    std::cout << "何时需要自定义Find模块:\n";
    std::cout << "  1. 库没有提供CMake配置文件 (XXXConfig.cmake)\n";
    std::cout << "  2. 库使用pkg-config而非CMake\n";
    std::cout << "  3. 系统安装的库需要自定义搜索逻辑\n";
    std::cout << "  4. 需要兼容旧版CMake\n\n";

    std::cout << "项目结构:\n";
    std::cout << "  myproject/\n";
    std::cout << "  ├── cmake/\n";
    std::cout << "  │   └── FindMyLib.cmake\n";
    std::cout << "  ├── CMakeLists.txt\n";
    std::cout << "  └── src/\n\n";

    std::cout << "CMakeLists.txt中指定模块路径:\n";
    std::cout << "  list(APPEND CMAKE_MODULE_PATH \"${CMAKE_SOURCE_DIR}/cmake\")\n";
    std::cout << "  find_package(MyLib REQUIRED)\n\n";

    std::cout << "FindMyLib.cmake模板:\n";
    std::cout << "  # 查找头文件\n";
    std::cout << "  find_path(MYLIB_INCLUDE_DIR\n";
    std::cout << "    NAMES mylib/core.h\n";
    std::cout << "    PATHS /usr/local/include /usr/include\n";
    std::cout << "    PATH_SUFFIXES mylib\n";
    std::cout << "  )\n\n";

    std::cout << "  # 查找库文件\n";
    std::cout << "  find_library(MYLIB_LIBRARY\n";
    std::cout << "    NAMES mylib mylibd  # release和debug名称\n";
    std::cout << "    PATHS /usr/local/lib /usr/lib\n";
    std::cout << "  )\n\n";

    std::cout << "  # 处理标准参数 (REQUIRED, QUIET, 版本)\n";
    std::cout << "  include(FindPackageHandleStandardArgs)\n";
    std::cout << "  find_package_handle_standard_args(MyLib\n";
    std::cout << "    REQUIRED_VARS MYLIB_LIBRARY MYLIB_INCLUDE_DIR\n";
    std::cout << "    VERSION_VAR MYLIB_VERSION\n";
    std::cout << "  )\n\n";

    std::cout << "  # 创建导入目标 (推荐方式)\n";
    std::cout << "  if(MyLib_FOUND AND NOT TARGET MyLib::MyLib)\n";
    std::cout << "    add_library(MyLib::MyLib UNKNOWN IMPORTED)\n";
    std::cout << "    set_target_properties(MyLib::MyLib PROPERTIES\n";
    std::cout << "      IMPORTED_LOCATION \"${MYLIB_LIBRARY}\"\n";
    std::cout << "      INTERFACE_INCLUDE_DIRECTORIES \"${MYLIB_INCLUDE_DIR}\"\n";
    std::cout << "    )\n";
    std::cout << "  endif()\n\n";

    std::cout << "  # 标记高级变量\n";
    std::cout << "  mark_as_advanced(MYLIB_INCLUDE_DIR MYLIB_LIBRARY)\n\n";

    std::cout << "创建导入目标的类型选择:\n";
    std::cout << "  UNKNOWN     - 不确定是静态还是动态\n";
    std::cout << "  STATIC      - 静态库\n";
    std::cout << "  SHARED      - 动态库\n";
    std::cout << "  INTERFACE   - 纯头文件库\n\n";

    std::cout << "最佳实践:\n";
    std::cout << "  1. 总是创建导入目标 (MyLib::MyLib), 而非只设置变量\n";
    std::cout << "  2. 使用FindPackageHandleStandardArgs处理标准参数\n";
    std::cout << "  3. 支持Debug/Release不同库路径\n";
    std::cout << "  4. 检查目标是否已存在 (NOT TARGET)\n";
    std::cout << "  5. 标记高级变量, 避免污染cmake-gui\n";
}

// ============================================================================
// Demo 7: CMake target-based方法
// ============================================================================
void demo_target_based_approach() {
    std::cout << "\n═══════════════════════════════════════════════════════════\n";
    std::cout << "=== demo_target_based_approach ===\n";
    std::cout << "═══════════════════════════════════════════════════════════\n";
    std::cout << "现代CMake: 基于目标的属性管理\n\n";

    std::cout << "核心原则: 属性附加到目标(target)上, 而非全局变量\n\n";

    // 用模拟的CMakeTarget结构演示
    CMakeTarget myapp;
    myapp.name = "myapp";
    myapp.type = "executable";
    myapp.sources = {"main.cpp", "app.cpp", "utils.cpp"};
    myapp.include_dirs = {"include/"};
    myapp.link_libraries = {"fmt::fmt", "nlohmann_json::nlohmann_json"};
    myapp.compile_features = {"cxx_std_17"};
    myapp.compile_definitions = {"SPDLOG_ACTIVE_LEVEL=SPDLOG_LEVEL_DEBUG"};

    std::cout << "1. target_link_libraries - 链接库并传递使用要求\n\n";
    std::cout << "  add_executable(myapp main.cpp)\n";
    std::cout << "  target_link_libraries(myapp PRIVATE\n";
    std::cout << "      fmt::fmt\n";
    std::cout << "      nlohmann_json::nlohmann_json\n";
    std::cout << "  )\n\n";

    std::cout << "  可见性关键字:\n";
    std::cout << "    PRIVATE   - 仅当前目标使用 (不传递)\n";
    std::cout << "    PUBLIC    - 当前目标使用, 且传递给依赖此目标的目标\n";
    std::cout << "    INTERFACE - 仅传递给依赖此目标的目标, 当前目标不使用\n\n";

    std::cout << "  模拟目标属性:\n";
    std::cout << myapp.summary() << "\n";

    std::cout << "2. target_include_directories - 指定头文件搜索路径\n\n";
    std::cout << "  target_include_directories(myapp\n";
    std::cout << "      PRIVATE\n";
    std::cout << "          ${CMAKE_CURRENT_SOURCE_DIR}/src    # 私有头文件\n";
    std::cout << "      PUBLIC\n";
    std::cout << "          ${CMAKE_CURRENT_SOURCE_DIR}/include  # 公共头文件\n";
    std::cout << "          $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>\n";
    std::cout << "          $<INSTALL_INTERFACE:include>         # 安装时路径\n";
    std::cout << "  )\n\n";

    std::cout << "3. target_compile_features - 指定编译特性\n\n";
    std::cout << "  target_compile_features(myapp PRIVATE cxx_std_17)\n";
    std::cout << "  # 或\n";
    std::cout << "  set_target_properties(myapp PROPERTIES\n";
    std::cout << "      CXX_STANDARD 17\n";
    std::cout << "      CXX_STANDARD_REQUIRED ON\n";
    std::cout << "      CXX_EXTENSIONS OFF\n";
    std::cout << "  )\n\n";

    std::cout << "4. target_compile_definitions - 添加编译定义\n\n";
    std::cout << "  target_compile_definitions(myapp\n";
    std::cout << "      PRIVATE\n";
    std::cout << "          DEBUG_MODE\n";
    std::cout << "          VERSION=\\\"1.0.0\\\"\n";
    std::cout << "  )\n\n";

    std::cout << "5. target_compile_options - 添加编译选项\n\n";
    std::cout << "  target_compile_options(myapp\n";
    std::cout << "      PRIVATE\n";
    std::cout << "          -Wall -Wextra -Wpedantic\n";
    std::cout << "          $<$<CONFIG:Debug>:-g -O0>    # Debug配置\n";
    std::cout << "          $<$<CONFIG:Release>:-O3>      # Release配置\n";
    std::cout << "  )\n\n";

    std::cout << "传统方式 vs 现代方式:\n";
    std::cout << "  ┌────────────────────┬──────────────────────┐\n";
    std::cout << "  │ 传统 (全局变量)    │ 现代 (目标属性)      │\n";
    std::cout << "  ├────────────────────┼──────────────────────┤\n";
    std::cout << "  │ include_directories│ target_include_directories │\n";
    std::cout << "  │ add_definitions    │ target_compile_definitions │\n";
    std::cout << "  │ link_libraries     │ target_link_libraries │\n";
    std::cout << "  │ set(CMAKE_CXX_FLAGS)│ target_compile_options│\n";
    std::cout << "  └────────────────────┴──────────────────────┘\n\n";

    std::cout << "现代方式优势:\n";
    std::cout << "  1. 属性隔离: 不同目标可以有不同设置\n";
    std::cout << "  2. 传递性: PUBLIC属性自动传递给依赖方\n";
    std::cout << "  3. 可组合: 多个目标的属性正确合并\n";
    std::cout << "  4. 可导出: 目标可以导出供其他项目使用\n";
}

// ============================================================================
// Demo 8: 现代CMake模式
// ============================================================================
void demo_modern_cmake_patterns() {
    std::cout << "\n═══════════════════════════════════════════════════════════\n";
    std::cout << "=== demo_modern_cmake_patterns ===\n";
    std::cout << "═══════════════════════════════════════════════════════════\n";
    std::cout << "现代CMake高级模式: 接口库, 别名目标, 生成器表达式\n\n";

    // ---- 接口库 ----
    std::cout << "1. 接口库 (INTERFACE library)\n";
    std::cout << "   纯头文件库或仅传递编译属性的目标\n\n";

    std::cout << "   # 纯头文件库\n";
    std::cout << "   add_library(mylib_headers INTERFACE)\n";
    std::cout << "   target_include_directories(mylib_headers INTERFACE\n";
    std::cout << "       $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>\n";
    std::cout << "       $<INSTALL_INTERFACE:include>\n";
    std::cout << "   )\n\n";

    std::cout << "   # 编译警告接口库 (统一项目警告设置)\n";
    std::cout << "   add_library(project_warnings INTERFACE)\n";
    std::cout << "   target_compile_options(project_warnings INTERFACE\n";
    std::cout << "       -Wall -Wextra -Wpedantic -Werror\n";
    std::cout << "       -Wno-unused-parameter\n";
    std::cout << "   )\n";
    std::cout << "   # 使用时:\n";
    std::cout << "   target_link_libraries(myapp PRIVATE project_warnings)\n\n";

    std::cout << "   # 项目通用设置接口库\n";
    std::cout << "   add_library(project_common INTERFACE)\n";
    std::cout << "   target_compile_features(project_common INTERFACE cxx_std_17)\n";
    std::cout << "   target_compile_definitions(project_common INTERFACE\n";
    std::cout << "       $<$<CONFIG:Debug>:DEBUG_MODE>\n";
    std::cout << "   )\n\n";

    // ---- 别名目标 ----
    std::cout << "2. 别名目标 (ALIAS targets)\n";
    std::cout << "   为目标创建可读名称, 常用于库的导出\n\n";

    std::cout << "   # 在库的CMakeLists.txt中\n";
    std::cout << "   add_library(mylib mylib.cpp)\n";
    std::cout << "   add_library(mylib::mylib ALIAS mylib)  # 创建别名\n\n";

    std::cout << "   # 使用者可以直接用别名\n";
    std::cout << "   target_link_libraries(myapp PRIVATE mylib::mylib)\n\n";

    std::cout << "   别名的作用:\n";
    std::cout << "     1. 统一命名: 项目内和导出后使用相同名称\n";
    std::cout << "     2. 命名空间: mylib::mylib 避免名称冲突\n";
    std::cout << "     3. 可读性: 清楚表明目标来源\n";
    std::cout << "     4. 不可修改: 别名是只读的, 不能修改属性\n\n";

    // 模拟别名目标
    CMakeTarget mylib;
    mylib.name = "mylib";
    mylib.type = "static_library";
    mylib.sources = {"mylib.cpp"};
    mylib.include_dirs = {"include/"};
    mylib.alias_names = {"mylib::mylib"};

    std::cout << "   模拟别名目标:\n";
    std::cout << mylib.summary() << "\n";

    // ---- 生成器表达式 ----
    std::cout << "3. 生成器表达式 (Generator Expressions)\n";
    std::cout << "   在生成阶段求值的表达式, 实现条件化配置\n\n";

    std::cout << "   常用生成器表达式:\n\n";

    std::cout << "   # 条件表达式: $<condition:true_string>\n";
    std::cout << "   # $<$<CONFIG:Debug>:-g>  => Debug模式时添加-g\n\n";

    std::cout << "   # 逻辑表达式:\n";
    std::cout << "   $<BOOL:value>              # 布尔转换\n";
    std::cout << "   $<AND:a,b>                 # 逻辑与\n";
    std::cout << "   $<OR:a,b>                  # 逻辑或\n";
    std::cout << "   $<NOT:a>                   # 逻辑非\n\n";

    std::cout << "   # 配置查询:\n";
    std::cout << "   $<CONFIG:Debug>            # 当前是否Debug配置\n";
    std::cout << "   $<CONFIG:Release>          # 当前是否Release配置\n\n";

    std::cout << "   # 目标属性查询:\n";
    std::cout << "   $<TARGET_FILE:myapp>       # 目标的输出文件路径\n";
    std::cout << "   $<TARGET_FILE_NAME:myapp>  # 目标的输出文件名\n\n";

    std::cout << "   # 编译器ID查询:\n";
    std::cout << "   $<CXX_COMPILER_ID:GNU>     # 是否GCC\n";
    std::cout << "   $<CXX_COMPILER_ID:MSVC>    # 是否MSVC\n";
    std::cout << "   $<CXX_COMPILER_ID:Clang>   # 是否Clang\n\n";

    std::cout << "   实际使用示例:\n\n";

    std::cout << "   # 按配置设置编译选项\n";
    std::cout << "   target_compile_options(myapp PRIVATE\n";
    std::cout << "       $<$<CONFIG:Debug>:-g -O0>\n";
    std::cout << "       $<$<CONFIG:Release>:-O3 -DNDEBUG>\n";
    std::cout << "   )\n\n";

    std::cout << "   # 按编译器设置选项\n";
    std::cout << "   target_compile_options(myapp PRIVATE\n";
    std::cout << "       $<$<CXX_COMPILER_ID:GNU>:-fno-rtti>\n";
    std::cout << "       $<$<CXX_COMPILER_ID:MSVC>:/GR->\n";
    std::cout << "   )\n\n";

    std::cout << "   # 安装时路径处理\n";
    std::cout << "   target_include_directories(mylib PUBLIC\n";
    std::cout << "       $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>\n";
    std::cout << "       $<INSTALL_INTERFACE:include>\n";
    std::cout << "   )\n\n";

    std::cout << "   # 链接时条件依赖\n";
    std::cout << "   target_link_libraries(myapp PRIVATE\n";
    std::cout << "       $<$<PLATFORM_ID:Windows>:ws2_32>\n";
    std::cout << "       $<$<PLATFORM_ID:Linux>:pthread>\n";
    std::cout << "   )\n\n";

    std::cout << "4. 完整的现代CMake项目示例:\n\n";
    std::cout << "  cmake_minimum_required(VERSION 3.20)\n";
    std::cout << "  project(myapp VERSION 1.0.0 LANGUAGES CXX)\n\n";

    std::cout << "  # 接口库: 项目通用设置\n";
    std::cout << "  add_library(project_common INTERFACE)\n";
    std::cout << "  target_compile_features(project_common INTERFACE cxx_std_17)\n\n";

    std::cout << "  # 接口库: 警告设置\n";
    std::cout << "  add_library(project_warnings INTERFACE)\n";
    std::cout << "  target_compile_options(project_warnings INTERFACE\n";
    std::cout << "      -Wall -Wextra -Wpedantic\n";
    std::cout << "      $<$<CXX_COMPILER_ID:GNU>:-Wconversion>\n";
    std::cout << "      $<$<CXX_COMPILER_ID:MSVC>:/W4>\n";
    std::cout << "  )\n\n";

    std::cout << "  # 主目标\n";
    std::cout << "  add_executable(myapp src/main.cpp)\n";
    std::cout << "  target_link_libraries(myapp PRIVATE\n";
    std::cout << "      project_common\n";
    std::cout << "      project_warnings\n";
    std::cout << "      fmt::fmt\n";
    std::cout << "      nlohmann_json::nlohmann_json\n";
    std::cout << "  )\n";
}

// ============================================================================
// main
// ============================================================================
int main() {
    std::cout << "CMake与包管理器集成模式演示\n";

    demo_find_package_pattern();
    demo_fetch_content();
    demo_vcpkg_cmake_integration();
    demo_conan_cmake_integration();
    demo_cpm_cmake();
    demo_custom_find_module();
    demo_target_based_approach();
    demo_modern_cmake_patterns();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
