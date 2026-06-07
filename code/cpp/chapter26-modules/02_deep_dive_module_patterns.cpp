/**
 * @file 02_deep_dive_module_patterns.cpp
 * @brief 模块设计模式与最佳实践
 * @description 对应文档: 02-CPP/27-modules
 */

#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <map>
#include <functional>
#include <sstream>

/*
 * ============================================================
 * 模块设计模式与最佳实践
 * ============================================================
 * 本文件展示模块的设计模式、命名空间配合、ABI考虑等
 * 模块语法以注释形式展示, 实际代码使用传统方式编译
 * ============================================================
 */

namespace module_patterns {

namespace detail {

int internal_counter = 0;

int next_id() {
    return ++internal_counter;
}

}

class Entity {
    int id_;
    std::string name_;
public:
    Entity(std::string name) : id_(detail::next_id()), name_(std::move(name)) {}

    int id() const { return id_; }
    const std::string& name() const { return name_; }

    void describe() const {
        std::cout << "Entity#" << id_ << ": " << name_ << "\n";
    }
};

class EntityManager {
    std::map<int, std::unique_ptr<Entity>> entities_;
public:
    Entity& create(const std::string& name) {
        auto entity = std::make_unique<Entity>(name);
        int id = entity->id();
        entities_[id] = std::move(entity);
        return *entities_[id];
    }

    Entity* find(int id) {
        auto it = entities_.find(id);
        return it != entities_.end() ? it->second.get() : nullptr;
    }

    void list_all() const {
        for (const auto& [id, entity] : entities_) {
            entity->describe();
        }
    }

    size_t count() const { return entities_.size(); }
};

}

void demo_layered_module() {
    std::cout << "\n=== 分层模块模式 ===\n";

    /*
     * 分层模块设计:
     *
     * // core.cppm - 核心层
     * export module core;
     * export class Entity { /* ... *\/ };
     *
     * // data.cppm - 数据层
     * export module data;
     * import core;
     * export class EntityManager { /* ... *\/ };
     *
     * // service.cppm - 服务层
     * export module service;
     * import data;
     * export class EntityService { /* ... *\/ };
     *
     * // app.cppm - 应用层
     * export module app;
     * import service;
     * export class Application { /* ... *\/ };
     */

    module_patterns::EntityManager mgr;
    mgr.create("Player");
    mgr.create("Enemy");
    mgr.create("NPC");
    std::cout << "实体列表:\n";
    mgr.list_all();
    std::cout << "总数: " << mgr.count() << "\n";

    std::cout << "\n举一反三:\n";
    std::cout << "  核心层: 基础类型和工具\n";
    std::cout << "  数据层: 数据访问和存储\n";
    std::cout << "  服务层: 业务逻辑\n";
    std::cout << "  应用层: 应用入口和协调\n";
    std::cout << "  每层一个模块, 依赖方向单向\n";
}

void demo_facade_module() {
    std::cout << "\n=== 外观模块模式 ===\n";

    /*
     * 外观模块:
     *
     * // internal_a.cppm
     * export module internal_a;
     * export void func_a();
     *
     * // internal_b.cppm
     * export module internal_b;
     * export void func_b();
     *
     * // facade.cppm - 对外统一接口
     * export module mylib;
     * export import :internal_a;
     * export import :internal_b;
     *
     * // 使用者只需 import mylib;
     */

    std::cout << "外观模块将多个子模块统一导出:\n";
    std::cout << "  export module mylib;\n";
    std::cout << "  export import :sub_module_a;\n";
    std::cout << "  export import :sub_module_b;\n";
    std::cout << "\n优势:\n";
    std::cout << "  使用者只需import一个模块\n";
    std::cout << "  内部可以自由重组\n";
    std::cout << "  版本升级时保持接口稳定\n";
}

void demo_module_and_namespace() {
    std::cout << "\n=== 模块与命名空间 ===\n";

    /*
     * 模块与命名空间的配合:
     *
     * // math.cppm
     * export module math;
     *
     * export namespace math {
     *     int add(int a, int b);
     *     int multiply(int a, int b);
     * }
     *
     * // 或者
     * export module math;
     * namespace math {
     *     int add(int a, int b);       // 不导出
     *     int multiply(int a, int b);  // 不导出
     * }
     *
     * 最佳实践:
     *   1. 模块名和命名空间可以不同
     *   2. 一个模块可以包含多个命名空间
     *   3. export作用于声明, 不作用于命名空间
     */

    std::cout << "模块与命名空间的关系:\n";
    std::cout << "  模块: 编译单元级别的组织\n";
    std::cout << "  命名空间: 逻辑级别的组织\n";
    std::cout << "\n";
    std::cout << "推荐做法:\n";
    std::cout << "  模块名: mylib.core (用点分隔)\n";
    std::cout << "  命名空间: mylib::core (用::分隔)\n";
    std::cout << "  保持模块名和命名空间名的一致性\n";
    std::cout << "\n";
    std::cout << "注意: export不传播到命名空间内部\n";
    std::cout << "  export namespace X { int a; } -> a被导出\n";
    std::cout << "  namespace X { int a; } -> a不被导出\n";
}

void demo_module_and_abi() {
    std::cout << "\n=== 模块与ABI ===\n";

    std::cout << "模块对ABI的影响:\n\n";

    std::cout << "1. 符号可见性:\n";
    std::cout << "   export的声明 -> 对外可见(类似__declspec(dllexport))\n";
    std::cout << "   非export的声明 -> 模块内部可见(类似static)\n";

    std::cout << "\n2. 内联函数:\n";
    std::cout << "   模块中的内联函数不会在每个翻译单元生成副本\n";
    std::cout << "   编译器可以更好地优化内联决策\n";

    std::cout << "\n3. 模板实例化:\n";
    std::cout << "   模块中的模板只实例化一次\n";
    std::cout << "   减少编译时间和目标文件大小\n";

    std::cout << "\n4. ODR(单定义规则):\n";
    std::cout << "   模块加强了ODR检查\n";
    std::cout << "   同一实体在不同翻译单元中必须一致\n";

    std::cout << "\n5. 版本兼容性:\n";
    std::cout << "   修改模块接口 -> 所有依赖者需重新编译\n";
    std::cout << "   修改模块实现 -> 只需重新编译实现单元\n";
    std::cout << "   这与头文件的行为一致, 但更明确\n";
}

void demo_module_best_practices() {
    std::cout << "\n=== 模块最佳实践 ===\n";

    std::cout << "1. 模块命名:\n";
    std::cout << "   使用分层命名: mylib.core, mylib.io\n";
    std::cout << "   避免与标准库冲突: 不要用std.xxx\n";
    std::cout << "   保持简短但有意义\n";

    std::cout << "\n2. 接口设计:\n";
    std::cout << "   只导出必要的声明(最小接口原则)\n";
    std::cout << "   实现细节放在实现单元或私有片段中\n";
    std::cout << "   使用前向声明减少编译依赖\n";

    std::cout << "\n3. 依赖管理:\n";
    std::cout << "   避免循环依赖(A import B, B import A)\n";
    std::cout << "   使用分层架构管理依赖方向\n";
    std::cout << "   考虑使用接口模块解耦\n";

    std::cout << "\n4. 渐进迁移:\n";
    std::cout << "   从叶子模块开始(无依赖或依赖少)\n";
    std::cout << "   使用全局模块片段过渡\n";
    std::cout << "   保持旧接口兼容, 逐步废弃\n";

    std::cout << "\n5. 构建系统:\n";
    std::cout << "   使用CMake 3.28+的模块支持\n";
    std::cout << "   确保模块接口先于使用者编译\n";
    std::cout << "   使用编译器提供的依赖扫描工具\n";

    std::cout << "\n6. 测试:\n";
    std::cout << "   模块接口单元可以被单独测试\n";
    std::cout << "   使用import而非#include引入被测模块\n";
    std::cout << "   测试代码可以是传统翻译单元\n";
}

void demo_pitfalls() {
    std::cout << "\n=== 模块常见陷阱 ===\n";

    std::cout << "1. 宏不穿透模块:\n";
    std::cout << "   模块中定义的宏不会传播到import者\n";
    std::cout << "   这是特性不是bug, 但需要注意\n";

    std::cout << "\n2. include顺序敏感:\n";
    std::cout << "   全局模块片段中的#include顺序仍然重要\n";
    std::cout << "   但模块间的import顺序无关\n";

    std::cout << "\n3. 模块编译顺序:\n";
    std::cout << "   import的模块必须先编译\n";
    std::cout << "   需要构建系统支持依赖分析\n";

    std::cout << "\n4. 模板和模块:\n";
    std::cout << "   模板定义通常需要在接口单元中\n";
    std::cout << "   不能像普通函数那样分离声明和定义\n";

    std::cout << "\n5. 兼容性:\n";
    std::cout << "   不同编译器的模块ABI不兼容\n";
    std::cout << "   GCC的.gcm, Clang的.pcm, MSVC的.ifc互不通用\n";
    std::cout << "   混合编译器环境需要特别处理\n";
}

int main() {
    std::cout << "========== 模块设计模式与最佳实践 ==========\n";
    std::cout << "注意: 完整模块支持需要较新编译器, 本文件使用传统方式\n";

    demo_layered_module();
    demo_facade_module();
    demo_module_and_namespace();
    demo_module_and_abi();
    demo_module_best_practices();
    demo_pitfalls();

    std::cout << "\n========== 所有演示完成 ==========\n";
    return 0;
}
