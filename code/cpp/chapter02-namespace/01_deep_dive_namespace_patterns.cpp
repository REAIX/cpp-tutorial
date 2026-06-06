/** @file 01_deep_dive_namespace_patterns.cpp
 *  @brief 命名空间设计模式、ADL、命名空间版本化、inline namespace
 *  @description 对应文档: 02-CPP/02-namespace
 */

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>

// ===== 1. 命名空间设计模式 =====

// 模式1: 按功能模块划分
namespace network {
    void connect(const std::string& host) {
        std::cout << "连接到 " << host << std::endl;
    }
    void disconnect() {
        std::cout << "断开连接" << std::endl;
    }
}

namespace database {
    void query(const std::string& sql) {
        std::cout << "执行查询: " << sql << std::endl;
    }
    void commit() {
        std::cout << "提交事务" << std::endl;
    }
}

// 模式2: 按层级划分 (避免过深嵌套)
namespace app::models {
    struct User {
        std::string name;
        int id;
    };
}

namespace app::services {
    void process_user(const app::models::User& user) {
        std::cout << "处理用户: " << user.name << " (id=" << user.id << ")" << std::endl;
    }
}

// 模式3: 命名空间 + 前向声明
namespace external {
    class Renderer;  // 前向声明
    void render(Renderer& r);
}

void demo_namespace_patterns() {
    std::cout << "===== 命名空间设计模式 =====" << std::endl;

    network::connect("localhost");
    network::disconnect();

    database::query("SELECT * FROM users");
    database::commit();

    app::models::User user{"Alice", 1};
    app::services::process_user(user);

    std::cout << "\n命名空间设计原则:" << std::endl;
    std::cout << "  1. 按功能/模块划分, 不是按文件类型" << std::endl;
    std::cout << "  2. 嵌套不超过3层" << std::endl;
    std::cout << "  3. 公共 API 放在顶层命名空间" << std::endl;
    std::cout << "  4. 实现细节放在 detail 或 internal 子命名空间" << std::endl;
}

// ===== 2. ADL (Argument-Dependent Lookup, 参数依赖查找) =====
namespace mylib {
    struct Point {
        double x, y;
    };

    void print(const Point& p) {
        std::cout << "(" << p.x << ", " << p.y << ")";
    }

    void swap(Point& a, Point& b) {
        std::cout << "mylib::swap 被调用" << std::endl;
        Point temp = a;
        a = b;
        b = temp;
    }
}

void demo_adl() {
    std::cout << "\n===== ADL (参数依赖查找) =====" << std::endl;

    mylib::Point p1{1.0, 2.0};
    mylib::Point p2{3.0, 4.0};

    // ADL: 编译器在参数的命名空间中查找函数
    print(p1);  // 不需要 mylib:: 前缀! ADL 在 mylib 中找到 print
    std::cout << std::endl;

    // ADL 与 std::swap
    using std::swap;
    swap(p1, p2);  // ADL 找到 mylib::swap, 优先于 std::swap

    std::cout << "\nADL 规则:" << std::endl;
    std::cout << "  - 在函数参数的命名空间中查找函数" << std::endl;
    std::cout << "  - 无需完全限定名即可调用" << std::endl;
    std::cout << "  - 运算符重载依赖 ADL (如 operator<<)" << std::endl;

    std::cout << "\nADL 的陷阱:" << std::endl;
    std::cout << "  - 意外找到不相关的函数" << std::endl;
    std::cout << "  - using namespace 可能导致意外 ADL" << std::endl;
    std::cout << "  - 在命名空间中重载运算符时要特别小心" << std::endl;

    std::cout << "\nADL 最佳实践:" << std::endl;
    std::cout << "  - swap 惯用法: using std::swap; swap(a, b);" << std::endl;
    std::cout << "  - 自定义类型在自身命名空间中提供 swap" << std::endl;
    std::cout << "  - 运算符重载放在参数的命名空间中" << std::endl;
}

// ===== 3. inline namespace (内联命名空间) =====
namespace library {
    // 版本1 (旧版本, 仍可用)
    inline namespace v1 {
        void process() {
            std::cout << "library::process() v1 实现" << std::endl;
        }
    }

    // 版本2 (新版本)
    namespace v2 {
        void process() {
            std::cout << "library::process() v2 实现" << std::endl;
        }
    }
}

namespace library2 {
    // 切换默认版本: 把 v2 标记为 inline
    namespace v1 {
        void compute() {
            std::cout << "library2::compute() v1 实现" << std::endl;
        }
    }

    inline namespace v2 {
        void compute() {
            std::cout << "library2::compute() v2 实现" << std::endl;
        }
    }
}

void demo_inline_namespace() {
    std::cout << "\n===== inline namespace =====" << std::endl;

    // inline namespace 的成员可以直接通过外层命名空间访问
    library::process();       // 调用 v1 (v1 是 inline)
    library::v1::process();   // 显式调用 v1
    library::v2::process();   // 显式调用 v2

    library2::compute();      // 调用 v2 (v2 是 inline)
    library2::v1::compute();  // 显式调用 v1

    std::cout << "\ninline namespace 的用途:" << std::endl;
    std::cout << "  1. 版本控制: 默认使用 inline 版本, 可显式选择旧版" << std::endl;
    std::cout << "  2. ABI 兼容: inline namespace 的符号名包含版本信息" << std::endl;
    std::cout << "  3. 特化: 为特定版本提供不同的实现" << std::endl;

    std::cout << "\n注意: inline namespace 不等于匿名命名空间!" << std::endl;
    std::cout << "  - inline: 外层可直接访问, 符号仍导出" << std::endl;
    std::cout << "  - 匿名: 限制在翻译单元内, 符号不导出" << std::endl;
}

// ===== 4. 命名空间版本化模式 =====
namespace api {
    inline namespace current {
        struct Config {
            int version = 2;
            bool verbose = true;
        };

        void initialize(const Config& cfg) {
            std::cout << "API v" << cfg.version << " 初始化"
                      << (cfg.verbose ? " (详细模式)" : "") << std::endl;
        }
    }

    namespace deprecated {
        struct Config {
            int version = 1;
        };

        void initialize(const Config& cfg) {
            std::cout << "API v" << cfg.version << " 初始化 (已弃用)" << std::endl;
        }
    }
}

void demo_namespace_versioning() {
    std::cout << "\n===== 命名空间版本化 =====" << std::endl;

    api::Config cfg;  // 默认使用 current (inline)
    api::initialize(cfg);

    api::deprecated::Config old_cfg;
    api::deprecated::initialize(old_cfg);

    std::cout << "\n版本化最佳实践:" << std::endl;
    std::cout << "  - 当前版本标记为 inline" << std::endl;
    std::cout << "  - 旧版本保留在非 inline 命名空间" << std::endl;
    std::cout << "  - 通过 [[deprecated]] 属性标记旧接口" << std::endl;
}

int main() {
    std::cout << "========== 命名空间高级模式 ==========\n" << std::endl;

    demo_namespace_patterns();
    demo_adl();
    demo_inline_namespace();
    demo_namespace_versioning();

    return 0;
}
