/**
 * @file 01_deep_dive_dependency_management.cpp
 * @brief 依赖管理最佳实践: 语义版本, 传递依赖, 依赖地狱, 锁文件
 * @description 对应文档: 02-CPP/37-包管理工具
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <set>
#include <algorithm>
#include <sstream>
#include <cassert>

struct Version {
    int major = 0;
    int minor = 0;
    int patch = 0;

    static Version parse(const std::string& s) {
        Version v;
        char dot1, dot2;
        std::istringstream iss(s);
        iss >> v.major >> dot1 >> v.minor >> dot2 >> v.patch;
        return v;
    }

    std::string to_string() const {
        return std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);
    }

    bool operator<(const Version& o) const {
        if (major != o.major) return major < o.major;
        if (minor != o.minor) return minor < o.minor;
        return patch < o.patch;
    }

    bool operator==(const Version& o) const {
        return major == o.major && minor == o.minor && patch == o.patch;
    }

    bool is_compatible_with(const Version& required) const {
        if (major != required.major) return false;
        if (minor < required.minor) return false;
        if (minor == required.minor && patch < required.patch) return false;
        return true;
    }
};

void demo_semantic_versioning() {
    std::cout << "\n=== demo_semantic_versioning ===\n";
    std::cout << "语义版本控制 (SemVer)\n\n";

    std::cout << "版本格式: MAJOR.MINOR.PATCH\n";
    std::cout << "  MAJOR: 不兼容的API变更\n";
    std::cout << "  MINOR: 向后兼容的功能新增\n";
    std::cout << "  PATCH: 向后兼容的Bug修复\n\n";

    Version v1 = Version::parse("1.2.3");
    Version v2 = Version::parse("1.3.0");
    Version v3 = Version::parse("2.0.0");

    std::cout << "版本比较:\n";
    std::cout << "  " << v1.to_string() << " < " << v2.to_string() << ": " << std::boolalpha << (v1 < v2) << "\n";
    std::cout << "  " << v2.to_string() << " < " << v3.to_string() << ": " << (v2 < v3) << "\n";
    std::cout << "  " << v1.to_string() << " == " << v1.to_string() << ": " << (v1 == v1) << "\n\n";

    std::cout << "兼容性检查 (同MAJOR版本兼容):\n";
    std::cout << "  " << v2.to_string() << " 兼容 " << v1.to_string() << ": "
              << v2.is_compatible_with(v1) << "\n";
    std::cout << "  " << v1.to_string() << " 兼容 " << v2.to_string() << ": "
              << v1.is_compatible_with(v2) << "\n";
    std::cout << "  " << v3.to_string() << " 兼容 " << v1.to_string() << ": "
              << v3.is_compatible_with(v1) << " (MAJOR不同)\n\n";

    std::cout << "语义版本规则:\n";
    std::cout << "  1. MAJOR=0: 初始开发, API可能随时变化\n";
    std::cout << "  2. MAJOR变更: 必须有不兼容的API修改\n";
    std::cout << "  3. MINOR变更: 必须向后兼容\n";
    std::cout << "  4. PATCH变更: 只能是Bug修复\n\n";

    std::cout << "版本范围表示:\n";
    std::cout << "  精确版本: 1.2.3\n";
    std::cout << "  兼容版本: ^1.2.3 (>=1.2.3, <2.0.0)\n";
    std::cout << "  补丁兼容: ~1.2.3 (>=1.2.3, <1.3.0)\n";
    std::cout << "  范围: >=1.2.3 <2.0.0\n";
}

void demo_transitive_dependencies() {
    std::cout << "\n=== demo_transitive_dependencies ===\n";
    std::cout << "传递依赖\n\n";

    struct Package {
        std::string name;
        Version version;
        std::vector<std::pair<std::string, Version>> dependencies;
    };

    std::map<std::string, Package> registry;
    registry["myapp"] = Package{"myapp", Version::parse("1.0.0"), {
        {"web-framework", Version::parse("2.0.0")},
        {"json-lib", Version::parse("3.0.0")}
    }};
    registry["web-framework"] = Package{"web-framework", Version::parse("2.0.0"), {
        {"http-lib", Version::parse("1.5.0")},
        {"logging", Version::parse("1.0.0")}
    }};
    registry["json-lib"] = Package{"json-lib", Version::parse("3.0.0"), {
        {"unicode", Version::parse("2.0.0")}
    }};
    registry["http-lib"] = Package{"http-lib", Version::parse("1.5.0"), {
        {"ssl-lib", Version::parse("1.0.0")}
    }};
    registry["logging"] = Package{"logging", Version::parse("1.0.0"), {}};
    registry["unicode"] = Package{"unicode", Version::parse("2.0.0"), {}};
    registry["ssl-lib"] = Package{"ssl-lib", Version::parse("1.0.0"), {}};

    std::cout << "依赖树:\n";
    std::cout << "  myapp@1.0.0\n";
    std::cout << "  ├── web-framework@2.0.0\n";
    std::cout << "  │   ├── http-lib@1.5.0\n";
    std::cout << "  │   │   └── ssl-lib@1.0.0\n";
    std::cout << "  │   └── logging@1.0.0\n";
    std::cout << "  └── json-lib@3.0.0\n";
    std::cout << "      └── unicode@2.0.0\n\n";

    std::cout << "传递依赖的问题:\n";
    std::cout << "  1. 依赖数量爆炸: 直接依赖2个, 传递依赖5个\n";
    std::cout << "  2. 版本冲突: 不同直接依赖要求同一库的不同版本\n";
    std::cout << "  3. 安全漏洞: 传递依赖可能有漏洞\n";
    std::cout << "  4. 许可证问题: 传递依赖的许可证可能不兼容\n\n";

    std::cout << "管理传递依赖:\n";
    std::cout << "  1. 最小化直接依赖数量\n";
    std::cout << "  2. 定期审计依赖树\n";
    std::cout << "  3. 使用锁文件固定版本\n";
    std::cout << "  4. 关注依赖的安全公告\n";
}

void demo_dependency_hell() {
    std::cout << "\n=== demo_dependency_hell ===\n";
    std::cout << "依赖地狱 (Dependency Hell)\n\n";

    std::cout << "场景1: 版本冲突 (Diamond Problem)\n\n";
    std::cout << "  myapp\n";
    std::cout << "  ├── libA (需要 json@1.x)\n";
    std::cout << "  └── libB (需要 json@2.x)\n";
    std::cout << "  json@1.x 和 json@2.x 不兼容!\n\n";

    std::cout << "解决方案:\n";
    std::cout << "  1. 升级libA使其兼容json@2.x\n";
    std::cout << "  2. 降级libB使其兼容json@1.x\n";
    std::cout << "  3. 寻找同时兼容两者的json版本\n";
    std::cout << "  4. 使用隔离机制 (不同模块用不同版本)\n\n";

    std::cout << "场景2: 版本膨胀\n\n";
    std::cout << "  libA -> libC@1.0\n";
    std::cout << "  libB -> libC@1.1\n";
    std::cout << "  libD -> libC@1.2\n";
    std::cout << "  只能选择一个版本, 需要最高版本兼容低版本\n\n";

    std::cout << "场景3: 循环依赖\n\n";
    std::cout << "  libA -> libB -> libA (循环!)\n";
    std::cout << "  解决: 提取公共部分到libC\n\n";

    std::cout << "避免依赖地狱的最佳实践:\n";
    std::cout << "  1. 最小化依赖: 只引入真正需要的库\n";
    std::cout << "  2. 优先使用标准库和header-only库\n";
    std::cout << "  3. 选择维护活跃的库\n";
    std::cout << "  4. 遵循SemVer, 明确声明版本范围\n";
    std::cout << "  5. 定期更新依赖, 不要长期不更新\n";
    std::cout << "  6. 使用依赖分析工具\n";
}

void demo_lock_files() {
    std::cout << "\n=== demo_lock_files ===\n";
    std::cout << "锁文件 (Lock File)\n\n";

    std::cout << "锁文件的作用:\n";
    std::cout << "  1. 记录精确的依赖版本 (包括传递依赖)\n";
    std::cout << "  2. 确保团队成员使用相同的依赖版本\n";
    std::cout << "  3. 确保CI/CD使用相同的依赖版本\n";
    std::cout << "  4. 确保部署环境与开发环境一致\n\n";

    std::cout << "vcpkg锁文件 (vcpkg-lock.json):\n";
    std::cout << "  {\n";
    std::cout << "    \"version\": 1,\n";
    std::cout << "    \"packages\": {\n";
    std::cout << "      \"nlohmann-json\": \"3.11.2\",\n";
    std::cout << "      \"fmt\": \"10.1.1\",\n";
    std::cout << "      \"cpp-httplib\": \"0.14.1\"\n";
    std::cout << "    }\n";
    std::cout << "  }\n\n";

    std::cout << "Conan锁文件 (conan.lock):\n";
    std::cout << "  自动生成, 记录完整的依赖图\n\n";

    std::cout << "锁文件最佳实践:\n";
    std::cout << "  1. 将锁文件提交到版本控制\n";
    std::cout << "  2. 应用项目: 使用锁文件确保可重现构建\n";
    std::cout << "  3. 库项目: 不提交锁文件 (让使用者解析)\n";
    std::cout << "  4. 定期更新锁文件 (安全更新)\n";
    std::cout << "  5. CI中验证锁文件一致性\n";

    std::cout << "\nC++依赖管理工具对比:\n";
    std::cout << "  ┌──────────┬─────────┬──────────┬──────────┬──────────┐\n";
    std::cout << "  │ 工具     │ 锁文件  │ 传递依赖 │ 版本范围 │ 二进制包 │\n";
    std::cout << "  ├──────────┼─────────┼──────────┼──────────┼──────────┤\n";
    std::cout << "  │ vcpkg    │ 支持    │ 自动     │ 基础     │ 源码编译 │\n";
    std::cout << "  │ Conan    │ 支持    │ 自动     │ 丰富     │ 预编译   │\n";
    std::cout << "  │ CPM.cmake│ 无      │ 自动     │ 基础     │ 源码     │\n";
    std::cout << "  │ FetchContent│ 无   │ 手动     │ 无       │ 源码     │\n";
    std::cout << "  └──────────┴─────────┴──────────┴──────────┴──────────┘\n\n";

    std::cout << "选择建议:\n";
    std::cout << "  新项目: vcpkg (最简单, 微软维护)\n";
    std::cout << "  企业项目: Conan (更灵活, 私有仓库)\n";
    std::cout << "  小项目: CPM.cmake 或 FetchContent\n";
    std::cout << "  跨平台项目: vcpkg + CMake\n";
}

int main() {
    std::cout << "依赖管理最佳实践\n";

    demo_semantic_versioning();
    demo_transitive_dependencies();
    demo_dependency_hell();
    demo_lock_files();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
