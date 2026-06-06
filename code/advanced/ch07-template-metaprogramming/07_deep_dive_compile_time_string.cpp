/** @file 07_deep_dive_compile_time_string.cpp
 *  @brief 编译期字符串处理与哈希：编译期字符串操作、哈希、字符串匹配
 *  @description 对应文档: 07-模板元编程与编译期计算 / 模板元编程实战(深入)
 */

#include <iostream>
#include <string>
#include <string_view>
#include <array>
#include <cstdint>

// ============================================================
// 1. 编译期字符串的基本需求
// ============================================================

// 为什么需要编译期字符串？
// - 编译期哈希：用于 switch-case 字符串匹配
// - 编译期字符串比较：模板参数中的字符串
// - 编译期格式验证：检查格式字符串
// - 编译期配置：字符串作为模板参数

void demo_why_compile_time_string() {
    std::cout << "=== 为什么需要编译期字符串 ===\n";
    std::cout << "1. 编译期哈希 → switch-case 字符串匹配\n";
    std::cout << "2. 编译期字符串比较 → 模板参数\n";
    std::cout << "3. 编译期格式验证 → 检查格式字符串\n";
    std::cout << "4. 编译期配置 → 字符串作为模板参数\n\n";
}

// ============================================================
// 2. C++20 编译期字符串类
// ============================================================

// C++20 允许字符串字面量作为模板参数
// FixedString: 编译期字符串包装

template<std::size_t N>
struct FixedString {
    char data[N]{};
    std::size_t length = N - 1;

    constexpr FixedString() = default;

    constexpr FixedString(const char (&str)[N]) {
        for (std::size_t i = 0; i < N; ++i) {
            data[i] = str[i];
        }
    }

    constexpr char operator[](std::size_t i) const {
        return data[i];
    }

    constexpr std::size_t size() const {
        return length;
    }

    constexpr const char* c_str() const {
        return data;
    }

    constexpr bool operator==(const FixedString& other) const {
        for (std::size_t i = 0; i < N; ++i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }

    template<std::size_t M>
    constexpr bool operator==(const FixedString<M>& other) const {
        if (length != other.length) return false;
        for (std::size_t i = 0; i < length; ++i) {
            if (data[i] != other.data[i]) return false;
        }
        return true;
    }
};

// 字符串字面量运算符模板（C++20）
template<FixedString S>
constexpr auto operator""_fs() {
    return S;
}

// 使用 FixedString 作为模板参数
template<FixedString Name>
struct NamedType {
    static constexpr const char* name = Name.c_str();

    constexpr bool has_name(std::string_view sv) const {
        return sv == Name.c_str();
    }
};

void demo_fixed_string() {
    std::cout << "=== C++20 编译期字符串类 ===\n";

    // 编译期字符串
    constexpr FixedString hello("Hello");
    constexpr FixedString world("World");

    std::cout << "hello = " << hello.c_str() << "\n";
    std::cout << "world = " << world.c_str() << "\n";
    std::cout << "hello.size() = " << hello.size() << "\n";

    // 编译期比较
    constexpr bool same = (hello == hello);
    constexpr bool diff = (hello == world);
    std::cout << "hello == hello: " << same << "\n";
    std::cout << "hello == world: " << diff << "\n";

    // 作为模板参数
    using UserName = NamedType<"username">;
    using PassWord = NamedType<"password">;

    std::cout << "\nUserName::name = " << UserName::name << "\n";
    std::cout << "PassWord::name = " << PassWord::name << "\n";

    UserName un;
    std::cout << "has_name(\"username\"): " << un.has_name("username") << "\n";
    std::cout << "has_name(\"other\"): " << un.has_name("other") << "\n";

    std::cout << "\n";
}

// ============================================================
// 3. 编译期字符串哈希
// ============================================================

// DJB2 哈希算法
consteval std::uint32_t djb2_hash(std::string_view str) {
    std::uint32_t hash = 5381;
    for (char c : str) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
    }
    return hash;
}

// FNV-1a 哈希算法
consteval std::uint32_t fnv1a_hash(std::string_view str) {
    std::uint32_t hash = 0x811c9dc5;  // FNV offset basis
    for (char c : str) {
        hash ^= static_cast<unsigned char>(c);
        hash *= 0x01000193;  // FNV prime
    }
    return hash;
}

// CRC32 哈希（简化版）
consteval std::uint32_t crc32_hash(std::string_view str) {
    std::uint32_t crc = 0xFFFFFFFF;
    for (char c : str) {
        crc ^= static_cast<unsigned char>(c);
        for (int i = 0; i < 8; ++i) {
            crc = (crc >> 1) ^ (0xEDB88320 & (-(crc & 1)));
        }
    }
    return ~crc;
}

void demo_compile_time_hash() {
    std::cout << "=== 编译期字符串哈希 ===\n";

    // 编译期计算哈希
    constexpr auto h1 = djb2_hash("hello");
    constexpr auto h2 = djb2_hash("world");
    constexpr auto h3 = fnv1a_hash("hello");
    constexpr auto h4 = fnv1a_hash("world");
    constexpr auto h5 = crc32_hash("hello");
    constexpr auto h6 = crc32_hash("world");

    std::cout << "DJB2:\n";
    std::cout << "  hash(\"hello\") = 0x" << std::hex << h1 << std::dec << "\n";
    std::cout << "  hash(\"world\") = 0x" << std::hex << h2 << std::dec << "\n";

    std::cout << "FNV-1a:\n";
    std::cout << "  hash(\"hello\") = 0x" << std::hex << h3 << std::dec << "\n";
    std::cout << "  hash(\"world\") = 0x" << std::hex << h4 << std::dec << "\n";

    std::cout << "CRC32:\n";
    std::cout << "  hash(\"hello\") = 0x" << std::hex << h5 << std::dec << "\n";
    std::cout << "  hash(\"world\") = 0x" << std::hex << h6 << std::dec << "\n";

    // 编译期验证
    static_assert(djb2_hash("hello") == djb2_hash("hello"));
    static_assert(djb2_hash("hello") != djb2_hash("world"));

    std::cout << "\n";
}

// ============================================================
// 4. 编译期哈希实现字符串 switch-case
// ============================================================

// 经典应用：用编译期哈希实现字符串的 switch-case
// C++ 的 switch 只支持整型，但通过哈希可以间接实现

// 运行期哈希（与编译期使用相同算法）
constexpr std::uint32_t rt_djb2_hash(std::string_view str) {
    std::uint32_t hash = 5381;
    for (char c : str) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(c);
    }
    return hash;
}

// 字符串 switch-case 辅助宏
#define STRING_HASH(s) (::djb2_hash(s))
#define SWITCH_HASH(str) switch (::rt_djb2_hash(str))
#define CASE_HASH(s) case (::djb2_hash(s))

void demo_string_switch() {
    std::cout << "=== 编译期哈希实现字符串 switch ===\n";

    auto handle_command = [](const std::string& cmd) {
        SWITCH_HASH(cmd) {
            CASE_HASH("start"):
                std::cout << "  执行: 启动\n";
                break;
            CASE_HASH("stop"):
                std::cout << "  执行: 停止\n";
                break;
            CASE_HASH("pause"):
                std::cout << "  执行: 暂停\n";
                break;
            CASE_HASH("resume"):
                std::cout << "  执行: 恢复\n";
                break;
            CASE_HASH("exit"):
                std::cout << "  执行: 退出\n";
                break;
            default:
                std::cout << "  未知命令: " << cmd << "\n";
                break;
        }
    };

    handle_command("start");
    handle_command("stop");
    handle_command("pause");
    handle_command("resume");
    handle_command("exit");
    handle_command("unknown");

    std::cout << "\n注意: 哈希碰撞可能导致错误匹配\n";
    std::cout << "  实际使用时应在 case 分支内做字符串相等验证\n";

    std::cout << "\n";
}

// ============================================================
// 5. 编译期字符串操作
// ============================================================

// 编译期字符串拼接
template<std::size_t N1, std::size_t N2>
consteval auto concat_strings(const char (&s1)[N1], const char (&s2)[N2]) {
    FixedString<N1 + N2 - 1> result{};
    for (std::size_t i = 0; i < N1 - 1; ++i) result.data[i] = s1[i];
    for (std::size_t i = 0; i < N2 - 1; ++i) result.data[N1 - 1 + i] = s2[i];
    result.data[N1 + N2 - 2] = '\0';
    result.length = N1 + N2 - 2;
    return result;
}

// 编译期字符串截取
template<std::size_t Start, std::size_t Len, std::size_t N>
consteval auto substring(const char (&str)[N]) {
    static_assert(Start + Len <= N - 1, "子串超出范围");
    FixedString<Len + 1> result{};
    for (std::size_t i = 0; i < Len; ++i) {
        result.data[i] = str[Start + i];
    }
    result.data[Len] = '\0';
    result.length = Len;
    return result;
}

// 编译期字符串查找
consteval std::size_t ct_find(char c, std::string_view str) {
    for (std::size_t i = 0; i < str.size(); ++i) {
        if (str[i] == c) return i;
    }
    return std::string_view::npos;
}

// 编译期字符串计数
consteval std::size_t ct_count(char c, std::string_view str) {
    std::size_t count = 0;
    for (char ch : str) {
        if (ch == c) ++count;
    }
    return count;
}

// 编译期大小写转换
consteval char to_upper(char c) {
    return (c >= 'a' && c <= 'z') ? c - 32 : c;
}

consteval char to_lower(char c) {
    return (c >= 'A' && c <= 'Z') ? c + 32 : c;
}

template<std::size_t N>
consteval FixedString<N> to_upper_all(const char (&str)[N]) {
    FixedString<N> result{};
    for (std::size_t i = 0; i < N; ++i) {
        result.data[i] = to_upper(str[i]);
    }
    result.length = N - 1;
    return result;
}

template<std::size_t N>
consteval FixedString<N> to_lower_all(const char (&str)[N]) {
    FixedString<N> result{};
    for (std::size_t i = 0; i < N; ++i) {
        result.data[i] = to_lower(str[i]);
    }
    result.length = N - 1;
    return result;
}

void demo_compile_time_string_ops() {
    std::cout << "=== 编译期字符串操作 ===\n";

    // 拼接
    constexpr auto joined = concat_strings("Hello, ", "World!");
    std::cout << "拼接: " << joined.c_str() << "\n";

    // 截取
    constexpr auto sub = substring<0, 5>("Hello, World!");
    std::cout << "截取[0:5]: " << sub.c_str() << "\n";

    // 查找
    constexpr auto pos = ct_find(',', "Hello, World!");
    std::cout << "查找逗号位置: " << pos << "\n";

    // 计数
    constexpr auto cnt = ct_count('l', "Hello, World!");
    std::cout << "统计 'l' 出现次数: " << cnt << "\n";

    // 大小写
    constexpr auto upper = to_upper_all("hello world");
    constexpr auto lower = to_lower_all("HELLO WORLD");
    std::cout << "转大写: " << upper.c_str() << "\n";
    std::cout << "转小写: " << lower.c_str() << "\n";

    std::cout << "\n";
}

// ============================================================
// 6. 编译期字符串解析
// ============================================================

// 编译期判断是否为数字字符串
consteval bool is_digit_string(std::string_view str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (c < '0' || c > '9') return false;
    }
    return true;
}

// 编译期判断是否为合法标识符
consteval bool is_identifier(std::string_view str) {
    if (str.empty()) return false;
    char first = str[0];
    if (!(first == '_' || (first >= 'a' && first <= 'z') || (first >= 'A' && first <= 'Z'))) {
        return false;
    }
    for (std::size_t i = 1; i < str.size(); ++i) {
        char c = str[i];
        if (!(c == '_' || (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9'))) {
            return false;
        }
    }
    return true;
}

// 编译期解析整数
consteval int ct_stoi(std::string_view str) {
    int result = 0;
    bool negative = false;
    std::size_t i = 0;
    if (i < str.size() && str[i] == '-') {
        negative = true;
        ++i;
    }
    for (; i < str.size(); ++i) {
        if (str[i] < '0' || str[i] > '9') break;
        result = result * 10 + (str[i] - '0');
    }
    return negative ? -result : result;
}

// 编译期版本号解析
struct Version {
    int major, minor, patch;
};

consteval Version parse_version(std::string_view str) {
    Version v{0, 0, 0};
    int* current = &v.major;
    int value = 0;

    for (char c : str) {
        if (c == '.') {
            *current = value;
            value = 0;
            if (current == &v.major) current = &v.minor;
            else if (current == &v.minor) current = &v.patch;
        } else if (c >= '0' && c <= '9') {
            value = value * 10 + (c - '0');
        }
    }
    *current = value;
    return v;
}

void demo_compile_time_parsing() {
    std::cout << "=== 编译期字符串解析 ===\n";

    // 数字判断
    constexpr bool d1 = is_digit_string("12345");
    constexpr bool d2 = is_digit_string("12a45");
    std::cout << "is_digit_string(\"12345\"): " << d1 << "\n";
    std::cout << "is_digit_string(\"12a45\"): " << d2 << "\n";

    // 标识符判断
    constexpr bool id1 = is_identifier("my_var");
    constexpr bool id2 = is_identifier("123abc");
    constexpr bool id3 = is_identifier("_private");
    std::cout << "is_identifier(\"my_var\"): " << id1 << "\n";
    std::cout << "is_identifier(\"123abc\"): " << id2 << "\n";
    std::cout << "is_identifier(\"_private\"): " << id3 << "\n";

    // 整数解析
    constexpr int n1 = ct_stoi("42");
    constexpr int n2 = ct_stoi("-123");
    std::cout << "ct_stoi(\"42\"): " << n1 << "\n";
    std::cout << "ct_stoi(\"-123\"): " << n2 << "\n";

    // 版本号解析
    constexpr auto ver = parse_version("2.1.5");
    std::cout << "parse_version(\"2.1.5\"): " << ver.major << "." << ver.minor << "." << ver.patch << "\n";

    // 编译期验证
    static_assert(d1 == true);
    static_assert(d2 == false);
    static_assert(n1 == 42);
    static_assert(n2 == -123);
    static_assert(ver.major == 2 && ver.minor == 1 && ver.patch == 5);

    std::cout << "\n";
}

// ============================================================
// 7. 编译期字符串映射表
// ============================================================

// 编译期字符串到值的映射
template<std::size_t N>
struct StringMap {
    std::array<std::pair<std::string_view, int>, N> entries{};

    constexpr int lookup(std::string_view key, int default_val = -1) const {
        for (const auto& [k, v] : entries) {
            if (k == key) return v;
        }
        return default_val;
    }

    constexpr bool contains(std::string_view key) const {
        for (const auto& [k, v] : entries) {
            if (k == key) return true;
        }
        return false;
    }
};

// 编译期构建映射表
consteval StringMap<5> make_error_map() {
    StringMap<5> map{};
    map.entries[0] = {"OK", 0};
    map.entries[1] = {"NOT_FOUND", 404};
    map.entries[2] = {"BAD_REQUEST", 400};
    map.entries[3] = {"SERVER_ERROR", 500};
    map.entries[4] = {"UNAUTHORIZED", 401};
    return map;
}

// 编译期枚举到字符串映射
enum class Color { Red, Green, Blue };

consteval StringMap<3> make_color_map() {
    StringMap<3> map{};
    map.entries[0] = {"Red", 0};
    map.entries[1] = {"Green", 1};
    map.entries[2] = {"Blue", 2};
    return map;
}

void demo_compile_time_map() {
    std::cout << "=== 编译期字符串映射表 ===\n";

    constexpr auto error_map = make_error_map();

    std::cout << "HTTP 错误码映射:\n";
    std::cout << "  OK -> " << error_map.lookup("OK") << "\n";
    std::cout << "  NOT_FOUND -> " << error_map.lookup("NOT_FOUND") << "\n";
    std::cout << "  BAD_REQUEST -> " << error_map.lookup("BAD_REQUEST") << "\n";
    std::cout << "  UNKNOWN -> " << error_map.lookup("UNKNOWN", -1) << "\n";

    std::cout << "\ncontains:\n";
    std::cout << "  \"OK\": " << error_map.contains("OK") << "\n";
    std::cout << "  \"INVALID\": " << error_map.contains("INVALID") << "\n";

    // 编译期验证
    static_assert(error_map.lookup("OK") == 0);
    static_assert(error_map.lookup("NOT_FOUND") == 404);
    static_assert(error_map.contains("OK") == true);

    std::cout << "\n";
}

// ============================================================
// 8. 实战：编译期路由表
// ============================================================

// 简化的编译期路由匹配
struct Route {
    std::string_view path;
    std::string_view handler;
    std::uint32_t hash;
};

template<std::size_t N>
struct RouteTable {
    std::array<Route, N> routes{};

    constexpr const Route* match(std::string_view path) const {
        std::uint32_t h = rt_djb2_hash(path);
        for (const auto& r : routes) {
            if (r.hash == h && r.path == path) return &r;
        }
        return nullptr;
    }
};

consteval RouteTable<4> make_route_table() {
    RouteTable<4> table{};
    table.routes[0] = {"/", "home_handler", djb2_hash("/")};
    table.routes[1] = {"/api/users", "users_handler", djb2_hash("/api/users")};
    table.routes[2] = {"/api/posts", "posts_handler", djb2_hash("/api/posts")};
    table.routes[3] = {"/about", "about_handler", djb2_hash("/about")};
    return table;
}

void demo_compile_time_routing() {
    std::cout << "=== 编译期路由表 ===\n";

    constexpr auto routes = make_route_table();

    auto test_route = [&](const char* path) {
        auto* route = routes.match(path);
        if (route) {
            std::cout << "  " << path << " -> " << route->handler << "\n";
        } else {
            std::cout << "  " << path << " -> 404 Not Found\n";
        }
    };

    test_route("/");
    test_route("/api/users");
    test_route("/api/posts");
    test_route("/about");
    test_route("/unknown");

    std::cout << "\n编译期路由表优势:\n";
    std::cout << "  1. 路由表在编译期生成\n";
    std::cout << "  2. 哈希加速查找\n";
    std::cout << "  3. 无动态内存分配\n";
    std::cout << "  4. 适合嵌入式场景\n";

    std::cout << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "  编译期字符串处理与哈希\n";
    std::cout << "============================================\n\n";

    demo_why_compile_time_string();
    demo_fixed_string();
    demo_compile_time_hash();
    demo_string_switch();
    demo_compile_time_string_ops();
    demo_compile_time_parsing();
    demo_compile_time_map();
    demo_compile_time_routing();

    std::cout << "============================================\n";
    std::cout << "  核心要点总结:\n";
    std::cout << "  1. FixedString: C++20 编译期字符串\n";
    std::cout << "  2. 编译期哈希: DJB2, FNV-1a, CRC32\n";
    std::cout << "  3. 字符串 switch: 哈希+验证\n";
    std::cout << "  4. 编译期解析: 版本号、标识符\n";
    std::cout << "  5. 编译期映射: 路由表、错误码\n";
    std::cout << "============================================\n";

    return 0;
}
