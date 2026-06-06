/**
 * @file 01_deep_dive_module_migration.cpp
 * @brief 头文件到模块的迁移策略
 * @description 对应文档: 02-CPP/27-modules
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <functional>

/*
 * ============================================================
 * 头文件到模块的迁移策略
 * ============================================================
 *
 * 阶段1: 准备工作
 *   - 消除头文件中的宏依赖
 *   - 确保头文件有include guards
 *   - 减少头文件间的耦合
 *
 * 阶段2: 渐进迁移
 *   - 使用全局模块片段包含旧头文件
 *   - 逐个将头文件转为模块
 *   - 混合使用#include和import
 *
 * 阶段3: 完全迁移
 *   - 所有头文件转为模块
 *   - 移除全局模块片段中的#include
 *   - 更新构建系统
 *
 * ============================================================
 */

namespace old_style {

class Logger {
public:
    enum class Level { Debug, Info, Warning, Error };

    void log(Level level, const std::string& msg) {
        const char* level_str[] = {"DEBUG", "INFO", "WARN", "ERROR"};
        std::cout << "[" << level_str[static_cast<int>(level)] << "] " << msg << "\n";
    }

    void debug(const std::string& msg) { log(Level::Debug, msg); }
    void info(const std::string& msg) { log(Level::Info, msg); }
    void warn(const std::string& msg) { log(Level::Warning, msg); }
    void error(const std::string& msg) { log(Level::Error, msg); }
};

class Config {
    std::map<std::string, std::string> data_;
public:
    void set(const std::string& key, const std::string& value) {
        data_[key] = value;
    }

    std::string get(const std::string& key, const std::string& default_val = "") const {
        auto it = data_.find(key);
        return it != data_.end() ? it->second : default_val;
    }

    bool has(const std::string& key) const {
        return data_.count(key) > 0;
    }

    void list() const {
        for (const auto& [k, v] : data_) {
            std::cout << "  " << k << " = " << v << "\n";
        }
    }
};

}

namespace new_style {

class Logger {
public:
    enum class Level { Debug, Info, Warning, Error };

    void log(Level level, const std::string& msg) {
        const char* level_str[] = {"DEBUG", "INFO", "WARN", "ERROR"};
        std::cout << "[" << level_str[static_cast<int>(level)] << "] " << msg << "\n";
    }

    void debug(const std::string& msg) { log(Level::Debug, msg); }
    void info(const std::string& msg) { log(Level::Info, msg); }
    void warn(const std::string& msg) { log(Level::Warning, msg); }
    void error(const std::string& msg) { log(Level::Error, msg); }
};

class Config {
    std::map<std::string, std::string> data_;
public:
    void set(const std::string& key, const std::string& value) {
        data_[key] = value;
    }

    std::string get(const std::string& key, const std::string& default_val = "") const {
        auto it = data_.find(key);
        return it != data_.end() ? it->second : default_val;
    }

    bool has(const std::string& key) const {
        return data_.count(key) > 0;
    }

    void list() const {
        for (const auto& [k, v] : data_) {
            std::cout << "  " << k << " = " << v << "\n";
        }
    }
};

}

void demo_migration_steps() {
    std::cout << "\n=== 迁移步骤详解 ===\n";

    std::cout << "步骤1: 识别模块边界\n";
    std::cout << "  - 将相关的类和函数归为一个模块\n";
    std::cout << "  - 例如: logger模块, config模块, math模块\n";

    std::cout << "\n步骤2: 创建模块接口文件\n";
    std::cout << "  - 将.h文件转为.cppm文件\n";
    std::cout << "  - 添加export关键字\n";
    std::cout << "  - 保留声明, 移除实现\n";

    std::cout << "\n步骤3: 创建模块实现文件\n";
    std::cout << "  - 将.cpp文件转为模块实现单元\n";
    std::cout << "  - 将#include替换为import(逐步)\n";

    std::cout << "\n步骤4: 更新构建系统\n";
    std::cout << "  - CMake: 使用target_sources添加模块文件\n";
    std::cout << "  - Makefile: 添加模块编译规则\n";
    std::cout << "  - 需要编译器支持模块的构建顺序\n";
}

void demo_module_and_build_systems() {
    std::cout << "\n=== 模块与构建系统 ===\n";

    std::cout << "CMake支持(CMake 3.28+):\n";
    std::cout << R"(
  target_sources(my_target
    PUBLIC
      FILE_SET cxx_modules TYPE CXX_MODULES FILES
        src/my_module.cppm
        src/my_module_impl.cpp
  )
)" << "\n";

    std::cout << "构建顺序问题:\n";
    std::cout << "  模块接口必须在导入者之前编译\n";
    std::cout << "  传统Makefile难以自动推导依赖\n";
    std::cout << "  CMake 3.28+可以自动处理模块依赖\n";

    std::cout << "\n编译器特定的构建方式:\n";
    std::cout << "  GCC: 编译.cppm生成.gcm文件, 链接时使用\n";
    std::cout << "  Clang: 编译.cppm生成.pcm文件\n";
    std::cout << "  MSVC: 编译.ixx/.cppm生成.ifc文件\n";
}

void demo_module_and_templates() {
    std::cout << "\n=== 模块与模板 ===\n";

    std::cout << "模板在模块中的特殊处理:\n";
    std::cout << "  1. 模板定义必须在接口单元中(或内联分区中)\n";
    std::cout << "  2. 模板实例化可以跨模块\n";
    std::cout << "  3. 显式实例化可以在实现单元中\n";

    std::cout << "\n模块语法示例:\n";
    std::cout << R"(
  // container_module.cppm
  export module container_module;

  export template<typename T>
  class Stack {
      std::vector<T> data_;
  public:
      void push(const T& val) { data_.push_back(val); }
      T pop() { T val = data_.back(); data_.pop_back(); return val; }
      bool empty() const { return data_.empty(); }
  };

  // 显式实例化(可选, 减少编译时间)
  extern template class Stack<int>;
)" << "\n";

    std::cout << "举一反三:\n";
    std::cout << "  模板头文件 -> 模块接口单元(直接迁移)\n";
    std::cout << "  显式实例化 -> 模块实现单元\n";
    std::cout << "  模板特化 -> 可以在导入模块后定义\n";
}

void demo_global_module_fragment_migration() {
    std::cout << "\n=== 全局模块片段迁移 ===\n";

    std::cout << "渐进迁移模式:\n\n";

    std::cout << R"(  // 阶段1: 全局模块片段包含旧头文件
  module;
  #include "old_header.h"      // 旧代码
  #include <vector>            // 标准库

  export module my_module;

  export using old_type = OldType;  // 重新导出旧类型
  export int new_func(int x);       // 新接口
)" << "\n";

    std::cout << R"(  // 阶段2: 逐步替换#include为import
  module;
  #include "old_header.h"      // 还未迁移的旧代码

  export module my_module;
  import <vector>;             // 已迁移为头文件单元

  export using old_type = OldType;
  export int new_func(int x);
)" << "\n";

    std::cout << R"(  // 阶段3: 完全迁移
  export module my_module;
  import <vector>;

  export class NewType { /* ... */ };  // 替代OldType
  export int new_func(int x);
)" << "\n";
}

void demo_practical_example() {
    std::cout << "\n=== 实际迁移示例 ===\n";

    new_style::Logger logger;
    logger.info("模块迁移示例");

    new_style::Config config;
    config.set("host", "localhost");
    config.set("port", "8080");
    config.set("debug", "true");

    std::cout << "配置项:\n";
    config.list();

    std::cout << "\n迁移前后对比:\n";
    std::cout << "  旧: #include \"logger.h\" #include \"config.h\"\n";
    std::cout << "  新: import logger; import config;\n";
    std::cout << "  优势: 编译更快, 无宏污染, 隔离性更好\n";
}

int main() {
    std::cout << "========== 头文件到模块的迁移策略 ==========\n";
    std::cout << "注意: 完整模块支持需要较新编译器, 本文件使用传统方式\n";

    demo_migration_steps();
    demo_module_and_build_systems();
    demo_module_and_templates();
    demo_global_module_fragment_migration();
    demo_practical_example();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
