/** @file 06_example_macro_reflection.cpp
 *  @brief 宏反射：基于宏的反射机制、成员注册、字段遍历
 *  @description 对应文档: 07-模板元编程与编译期计算 / 编译期反射与代码生成
 */

#include <iostream>
#include <string>
#include <vector>
#include <tuple>
#include <sstream>
#include <type_traits>
#include <typeinfo>
#include <cstddef>

// ============================================================
// 1. 宏反射的基本概念
// ============================================================

// C++ 没有原生的反射机制（C++26 可能引入静态反射）
// 目前常用的方案：
//   1. 宏 + 聚合类型：手动注册成员
//   2. 编译器魔术：__builtin_dump_struct (Clang)
//   3. 代码生成：通过工具自动生成反射代码
//   4. 结构化绑定 + if constexpr 递归

void demo_reflection_concept() {
    std::cout << "=== 宏反射基本概念 ===\n";
    std::cout << "C++ 没有原生反射，需要手动实现\n";
    std::cout << "常见方案:\n";
    std::cout << "  1. 宏 + 手动注册\n";
    std::cout << "  2. 结构化绑定(C++17)\n";
    std::cout << "  3. 编译器扩展\n";
    std::cout << "  4. 代码生成工具\n\n";
}

// ============================================================
// 2. 基于宏的成员注册
// ============================================================

// 核心宏：注册成员变量
// 每个成员注册其名称、类型和指针

// 成员描述符
struct MemberInfo {
    const char* name;
    std::size_t offset;
    const char* type_name;
};

// 反射基类：存储成员信息
template<typename T>
struct Reflection {
    static std::vector<MemberInfo>& members() {
        static std::vector<MemberInfo> infos;
        return infos;
    }

    static bool& registered() {
        static bool done = false;
        return done;
    }
};

// 注册宏
#define REFLECT_REGISTER_MEMBER(Class, Member) \
    Reflection<Class>::members().push_back(MemberInfo{    \
        #Member,                                \
        offsetof(Class, Member),                \
        typeid(decltype(Class::Member)).name()  \
    });

// 自动注册宏：在全局作用域使用（不能在类体内使用）
#define REFLECT_BEGIN(Class) \
    namespace { \
        struct Class##_Registrator { \
            Class##_Registrator() {

#define REFLECT_MEMBER(Class, Member) \
                REFLECT_REGISTER_MEMBER(Class, Member)

#define REFLECT_END(Class) \
                Reflection<Class>::registered() = true; \
            } \
        }; \
        static Class##_Registrator Class##_registrator_; \
    }

// ============================================================
// 3. 使用宏反射的示例类
// ============================================================

struct Person {
    std::string name;
    int age;
    double height;
};

// 在全局作用域注册成员
REFLECT_BEGIN(Person)
    REFLECT_MEMBER(Person, name)
    REFLECT_MEMBER(Person, age)
    REFLECT_MEMBER(Person, height)
REFLECT_END(Person)

struct Product {
    std::string id;
    std::string name;
    double price;
    int stock;
};

REFLECT_BEGIN(Product)
    REFLECT_MEMBER(Product, id)
    REFLECT_MEMBER(Product, name)
    REFLECT_MEMBER(Product, price)
    REFLECT_MEMBER(Product, stock)
REFLECT_END(Product)

void demo_macro_reflection() {
    std::cout << "=== 基于宏的成员注册 ===\n";

    Person p;
    p.name = "Alice";
    p.age = 30;
    p.height = 1.68;

    std::cout << "Person 的成员:\n";
    for (const auto& m : Reflection<Person>::members()) {
        std::cout << "  名称: " << m.name
                  << ", 偏移: " << m.offset
                  << ", 类型: " << m.type_name << "\n";
    }

    std::cout << "\nProduct 的成员:\n";
    for (const auto& m : Reflection<Product>::members()) {
        std::cout << "  名称: " << m.name
                  << ", 偏移: " << m.offset
                  << ", 类型: " << m.type_name << "\n";
    }

    std::cout << "\n";
}

// ============================================================
// 4. 基于偏移量的字段访问
// ============================================================

// 通过偏移量访问成员的通用函数
template<typename T, typename V>
void set_member(T& obj, const MemberInfo& info, const V& value) {
    // 通过偏移量获取成员指针并赋值
    char* base = reinterpret_cast<char*>(&obj);
    auto* member_ptr = reinterpret_cast<V*>(base + info.offset);
    *member_ptr = value;
}

// 通用打印：遍历所有成员
template<typename T>
void print_members(const T& obj) {
    const auto& members = Reflection<T>::members();
    const char* base = reinterpret_cast<const char*>(&obj);

    for (const auto& m : members) {
        std::cout << "  " << m.name << " = ";

        // 根据类型信息打印（简化版，实际需要更复杂的类型分发）
        // 这里使用编译期类型判断
        if (m.offset == offsetof(T, name) || std::string(m.type_name).find("string") != std::string::npos) {
            // 尝试作为 string 读取
            const auto* str_ptr = reinterpret_cast<const std::string*>(base + m.offset);
            std::cout << "\"" << *str_ptr << "\"";
        } else {
            // 尝试作为数值读取
            const auto* int_ptr = reinterpret_cast<const int*>(base + m.offset);
            const auto* dbl_ptr = reinterpret_cast<const double*>(base + m.offset);

            // 简单启发式：根据大小判断类型
            if (sizeof(T::age) == sizeof(int) && m.offset == offsetof(T, age)) {
                std::cout << *int_ptr;
            } else {
                std::cout << *dbl_ptr;
            }
        }
        std::cout << "\n";
    }
}

void demo_offset_access() {
    std::cout << "=== 基于偏移量的字段访问 ===\n";

    Person p;
    p.name = "Bob";
    p.age = 25;
    p.height = 1.80;

    std::cout << "Person 对象:\n";
    print_members(p);

    // 通过偏移量修改成员
    for (auto& m : Reflection<Person>::members()) {
        if (std::string(m.name) == "age") {
            set_member(p, m, 26);
        }
    }

    std::cout << "\n修改年龄后:\n";
    std::cout << "  age = " << p.age << "\n";

    std::cout << "\n";
}

// ============================================================
// 5. 基于宏的序列化/反序列化
// ============================================================

// 简化的 JSON 序列化宏
#define REFLECT_TO_JSON_BEGIN(Class) \
    std::string to_json() const { \
        std::ostringstream oss; \
        oss << "{";

#define REFLECT_TO_JSON_FIELD(Field) \
        oss << "\"" #Field "\": "; \
        if constexpr (std::is_same_v<decltype(Field), std::string>) { \
            oss << "\"" << Field << "\""; \
        } else if constexpr (std::is_same_v<decltype(Field), bool>) { \
            oss << (Field ? "true" : "false"); \
        } else { \
            oss << Field; \
        } \
        oss << ", ";

#define REFLECT_TO_JSON_END() \
        std::string result = oss.str(); \
        if (result.size() > 1) { result.erase(result.size() - 2, 2); } \
        result += "}"; \
        return result; \
    }

// 更实用的序列化方案：使用模板和 if constexpr
template<typename T>
struct JsonSerializer;

// 为 Person 特化
template<>
struct JsonSerializer<Person> {
    static std::string to_json(const Person& p) {
        std::ostringstream oss;
        oss << "{\"name\":\"" << p.name
            << "\",\"age\":" << p.age
            << ",\"height\":" << p.height << "}";
        return oss.str();
    }

    static void from_json(Person& p, const std::string& json) {
        // 简化的解析（实际应使用 JSON 库）
        // 这里仅演示概念
        std::cout << "  (从 JSON 解析 Person - 简化版)\n";
    }
};

// 为 Product 特化
template<>
struct JsonSerializer<Product> {
    static std::string to_json(const Product& p) {
        std::ostringstream oss;
        oss << "{\"id\":\"" << p.id
            << "\",\"name\":\"" << p.name
            << "\",\"price\":" << p.price
            << ",\"stock\":" << p.stock << "}";
        return oss.str();
    }
};

void demo_serialization() {
    std::cout << "=== 宏反射序列化 ===\n";

    Person p;
    p.name = "Charlie";
    p.age = 28;
    p.height = 1.75;

    Product prod;
    prod.id = "P001";
    prod.name = "Widget";
    prod.price = 9.99;
    prod.stock = 100;

    std::cout << "Person JSON: " << JsonSerializer<Person>::to_json(p) << "\n";
    std::cout << "Product JSON: " << JsonSerializer<Product>::to_json(prod) << "\n";

    std::cout << "\n";
}

// ============================================================
// 6. 结构化绑定实现轻量级反射（C++17）
// ============================================================

// 利用结构化绑定 + if constexpr 实现编译期"反射"
// 不需要宏，但需要手动为每个类型特化

// 获取对象字段数的辅助结构
template<typename T>
constexpr auto field_count() {
    // 通过尝试结构化绑定来检测字段数
    // 这是一个编译期技巧
    T val{};
    int count = 0;

    // 使用聚合初始化测试
    // 这种方法有限制，只能用于聚合类型
    if constexpr (std::is_aggregate_v<T>) {
        // 尝试不同数量的字段
        // 实际实现需要更复杂的 SFINAE
        return -1;  // 需要手动指定
    }
    return -1;
}

// 手动特化：为每个类型定义如何遍历字段
template<typename T>
struct FieldAccessor;

template<>
struct FieldAccessor<Person> {
    template<typename Func>
    static void for_each(Person& p, Func&& func) {
        func("name", p.name);
        func("age", p.age);
        func("height", p.height);
    }

    template<typename Func>
    static void for_each(const Person& p, Func&& func) {
        func("name", p.name);
        func("age", p.age);
        func("height", p.height);
    }
};

template<>
struct FieldAccessor<Product> {
    template<typename Func>
    static void for_each(Product& p, Func&& func) {
        func("id", p.id);
        func("name", p.name);
        func("price", p.price);
        func("stock", p.stock);
    }

    template<typename Func>
    static void for_each(const Product& p, Func&& func) {
        func("id", p.id);
        func("name", p.name);
        func("price", p.price);
        func("stock", p.stock);
    }
};

// 通用遍历函数
template<typename T, typename Func>
void for_each_field(T& obj, Func&& func) {
    FieldAccessor<T>::for_each(obj, std::forward<Func>(func));
}

void demo_structured_binding_reflection() {
    std::cout << "=== 结构化绑定反射 ===\n";

    Person p;
    p.name = "Diana";
    p.age = 35;
    p.height = 1.65;

    // 遍历所有字段
    std::cout << "Person 字段:\n";
    for_each_field(p, [](const char* name, auto& value) {
        using T = std::decay_t<decltype(value)>;
        if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "  " << name << " = \"" << value << "\"\n";
        } else if constexpr (std::is_integral_v<T>) {
            std::cout << "  " << name << " = " << value << " (整型)\n";
        } else if constexpr (std::is_floating_point_v<T>) {
            std::cout << "  " << name << " = " << value << " (浮点型)\n";
        }
    });

    // 通用序列化
    std::cout << "\n通用 JSON 序列化:\n";
    std::cout << "  ";
    std::cout << "{";
    bool first = true;
    for_each_field(p, [&first](const char* name, auto& value) {
        if (!first) std::cout << ", ";
        first = false;
        using T = std::decay_t<decltype(value)>;
        std::cout << "\"" << name << "\":";
        if constexpr (std::is_same_v<T, std::string>) {
            std::cout << "\"" << value << "\"";
        } else {
            std::cout << value;
        }
    });
    std::cout << "}\n";

    // 通用修改
    std::cout << "\n通用字段修改:\n";
    for_each_field(p, [](const char* name, auto& value) {
        if constexpr (std::is_same_v<std::decay_t<decltype(value)>, int>) {
            if (std::string(name) == "age") {
                value += 1;  // 年龄加1
            }
        }
    });
    std::cout << "  修改后 age = " << p.age << "\n";

    std::cout << "\n";
}

// ============================================================
// 7. 综合示例：ORM 风格的表映射
// ============================================================

// 模拟数据库表映射
struct ColumnInfo {
    const char* name;
    const char* type;
    bool nullable;
};

template<typename T>
struct TableSchema;

template<>
struct TableSchema<Person> {
    static const char* table_name() { return "persons"; }

    static std::vector<ColumnInfo> columns() {
        return {
            {"name",   "VARCHAR(100)", false},
            {"age",    "INTEGER",      false},
            {"height", "REAL",         true}
        };
    }

    static std::string create_table_sql() {
        std::string sql = "CREATE TABLE persons (";
        auto cols = columns();
        for (std::size_t i = 0; i < cols.size(); ++i) {
            if (i > 0) sql += ", ";
            sql += cols[i].name;
            sql += " ";
            sql += cols[i].type;
            if (!cols[i].nullable) sql += " NOT NULL";
        }
        sql += ");";
        return sql;
    }

    static std::string insert_sql(const Person& p) {
        std::ostringstream oss;
        oss << "INSERT INTO persons VALUES ('"
            << p.name << "', " << p.age << ", " << p.height << ");";
        return oss.str();
    }
};

void demo_orm_mapping() {
    std::cout << "=== ORM 风格表映射 ===\n";

    std::cout << "表名: " << TableSchema<Person>::table_name() << "\n";

    std::cout << "\n列信息:\n";
    for (const auto& col : TableSchema<Person>::columns()) {
        std::cout << "  " << col.name << " " << col.type
                  << (col.nullable ? " (可空)" : " (非空)") << "\n";
    }

    std::cout << "\n建表 SQL:\n";
    std::cout << "  " << TableSchema<Person>::create_table_sql() << "\n";

    Person p;
    p.name = "Eve";
    p.age = 22;
    p.height = 1.60;

    std::cout << "\n插入 SQL:\n";
    std::cout << "  " << TableSchema<Person>::insert_sql(p) << "\n";

    std::cout << "\n";
}

// ============================================================
// main
// ============================================================

int main() {
    std::cout << "============================================\n";
    std::cout << "  宏反射与编译期反射\n";
    std::cout << "============================================\n\n";

    demo_reflection_concept();
    demo_macro_reflection();
    demo_offset_access();
    demo_serialization();
    demo_structured_binding_reflection();
    demo_orm_mapping();

    std::cout << "============================================\n";
    std::cout << "  核心要点总结:\n";
    std::cout << "  1. C++ 无原生反射，需手动实现\n";
    std::cout << "  2. 宏注册: 简单但侵入性强\n";
    std::cout << "  3. 偏移量访问: 通用但需注意对齐\n";
    std::cout << "  4. 结构化绑定: 非侵入但需手动特化\n";
    std::cout << "  5. 实际项目推荐: 使用成熟反射库\n";
    std::cout << "============================================\n";

    return 0;
}
