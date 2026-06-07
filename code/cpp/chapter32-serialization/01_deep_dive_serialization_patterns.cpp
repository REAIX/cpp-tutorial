/**
 * @file 01_deep_dive_serialization_patterns.cpp
 * @brief 序列化模式: Visitor, 反射概念, Schema演进
 * @description 对应文档: 02-CPP/36-序列化与日志
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <memory>
#include <sstream>
#include <cstdint>
#include <functional>

class JsonSerializer;
class BinarySerializer;

struct SerializableField {
    std::string name;
    enum Type { INT, STRING, DOUBLE, BOOL } type;
    int int_val = 0;
    std::string str_val;
    double dbl_val = 0.0;
    bool bool_val = false;
};

class Serializable {
public:
    virtual ~Serializable() = default;
    virtual std::string type_name() const = 0;
    virtual std::vector<SerializableField> get_fields() const = 0;
    virtual void set_fields(const std::vector<SerializableField>& fields) = 0;
};

void demo_visitor_pattern() {
    std::cout << "\n=== demo_visitor_pattern ===\n";
    std::cout << "Visitor模式实现序列化\n\n";

    struct Person : Serializable {
        std::string name;
        int age = 0;
        double height = 0.0;
        bool active = false;

        std::string type_name() const override { return "Person"; }

        std::vector<SerializableField> get_fields() const override {
            return {
                {"name", SerializableField::STRING, 0, name, 0.0, false},
                {"age", SerializableField::INT, age, "", 0.0, false},
                {"height", SerializableField::DOUBLE, 0, "", height, false},
                {"active", SerializableField::BOOL, 0, "", 0.0, active}
            };
        }

        void set_fields(const std::vector<SerializableField>& fields) override {
            for (const auto& f : fields) {
                if (f.name == "name") name = f.str_val;
                else if (f.name == "age") age = f.int_val;
                else if (f.name == "height") height = f.dbl_val;
                else if (f.name == "active") active = f.bool_val;
            }
        }
    };

    class SerializationVisitor {
    public:
        virtual ~SerializationVisitor() = default;
        virtual std::string serialize(const Serializable& obj) = 0;
        virtual void deserialize(const std::string& data, Serializable& obj) = 0;
    };

    class JsonVisitor : public SerializationVisitor {
    public:
        std::string serialize(const Serializable& obj) override {
            std::ostringstream oss;
            oss << "{\"type\":\"" << obj.type_name() << "\",\"fields\":{";
            auto fields = obj.get_fields();
            for (size_t i = 0; i < fields.size(); ++i) {
                if (i > 0) oss << ",";
                oss << "\"" << fields[i].name << "\":";
                switch (fields[i].type) {
                    case SerializableField::INT: oss << fields[i].int_val; break;
                    case SerializableField::STRING: oss << "\"" << fields[i].str_val << "\""; break;
                    case SerializableField::DOUBLE: oss << fields[i].dbl_val; break;
                    case SerializableField::BOOL: oss << (fields[i].bool_val ? "true" : "false"); break;
                }
            }
            oss << "}}";
            return oss.str();
        }

        void deserialize(const std::string& data, Serializable& obj) override {
            std::cout << "  JSON反序列化: " << data << "\n";
            (void)obj;
        }
    };

    Person p;
    p.name = "张三";
    p.age = 25;
    p.height = 1.75;
    p.active = true;

    JsonVisitor json_visitor;
    std::string json = json_visitor.serialize(p);
    std::cout << "序列化结果: " << json << "\n\n";

    std::cout << "Visitor模式优势:\n";
    std::cout << "  1. 序列化逻辑与数据结构分离\n";
    std::cout << "  2. 新增序列化格式无需修改数据类\n";
    std::cout << "  3. 支持多种序列化格式\n";
    std::cout << "  4. 符合开闭原则\n\n";

    std::cout << "Visitor模式劣势:\n";
    std::cout << "  1. 需要手动维护字段映射\n";
    std::cout << "  2. 新增字段需修改多处代码\n";
    std::cout << "  3. 代码冗余, 容易遗漏\n";
}

void demo_reflection_concept() {
    std::cout << "\n=== demo_reflection_concept ===\n";
    std::cout << "反射式序列化概念\n\n";

    std::cout << "C++目前没有原生反射, 但可以通过以下方式模拟:\n\n";

    std::cout << "1. 宏注册法:\n";
    std::cout << "   DEFINE_FIELD(name, type)\n";
    std::cout << "   自动生成get_fields/set_fields\n\n";

    std::cout << "2. 编译期反射 (Magic Enum + 结构化绑定):\n";
    std::cout << "   C++26可能支持原生反射\n";
    std::cout << "   目前: boost.pfr (POD反射)\n\n";

    std::cout << "3. 代码生成:\n";
    std::cout << "   Protocol Buffers: .proto -> C++代码\n";
    std::cout << "   自动生成序列化/反序列化代码\n\n";

    std::cout << "4. 模板元编程:\n";
    std::cout << "   编译期遍历结构体成员\n";
    std::cout << "   复杂但零运行时开销\n\n";

    struct SimpleReflField {
        const char* name;
        size_t offset;
        enum { INT, STRING } type;
    };

    struct Config {
        int port;
        std::string host;
    };

    SimpleReflField config_fields[] = {
        {"port", offsetof(Config, port), SimpleReflField::INT},
        {"host", offsetof(Config, host), SimpleReflField::STRING}
    };

    Config cfg{8080, "localhost"};
    std::cout << "手动反射示例:\n";
    for (const auto& f : config_fields) {
        std::cout << "  " << f.name << " (offset=" << f.offset << "): ";
        if (f.type == SimpleReflField::INT) {
            int val = *reinterpret_cast<int*>(reinterpret_cast<char*>(&cfg) + f.offset);
            std::cout << val;
        }
        std::cout << "\n";
    }

    std::cout << "\nC++反射未来:\n";
    std::cout << "  C++26: 静态反射 (P2996)\n";
    std::cout << "  可遍历结构体成员\n";
    std::cout << "  自动生成序列化代码\n";
    std::cout << "  目前: 使用boost.pfr或宏模拟\n";
}

void demo_schema_evolution() {
    std::cout << "\n=== demo_schema_evolution ===\n";
    std::cout << "Schema演进策略\n\n";

    std::cout << "Schema: 数据结构的定义/模式\n";
    std::cout << "Schema演进: 数据结构随时间变化\n\n";

    std::cout << "演进规则 (Protocol Buffers风格):\n\n";

    std::cout << "允许的操作:\n";
    std::cout << "  1. 添加新字段 (用新字段编号)\n";
    std::cout << "  2. 删除字段 (标记为reserved, 不重用编号)\n";
    std::cout << "  3. 重命名字段 (编号不变即可)\n\n";

    std::cout << "禁止的操作:\n";
    std::cout << "  1. 修改字段类型\n";
    std::cout << "  2. 重用已删除的字段编号\n";
    std::cout << "  3. 修改字段的默认值语义\n\n";

    std::cout << "Schema演进示例:\n\n";

    std::cout << "V1: message User {\n";
    std::cout << "  string name = 1;\n";
    std::cout << "  int32 age = 2;\n";
    std::cout << "}\n\n";

    std::cout << "V2: message User {\n";
    std::cout << "  string name = 1;\n";
    std::cout << "  int32 age = 2;\n";
    std::cout << "  string email = 3;    // 新增\n";
    std::cout << "  bool active = 4;     // 新增\n";
    std::cout << "}\n\n";

    std::cout << "V3: message User {\n";
    std::cout << "  string name = 1;\n";
    std::cout << "  // age已删除, 编号2保留\n";
    std::cout << "  reserved 2;\n";
    std::cout << "  string email = 3;\n";
    std::cout << "  bool active = 4;\n";
    std::cout << "  int32 level = 5;     // 新增\n";
    std::cout << "}\n\n";

    std::cout << "兼容性保证:\n";
    std::cout << "  V1数据 -> V3代码: 新字段使用默认值\n";
    std::cout << "  V3数据 -> V1代码: 未知字段被忽略\n";
    std::cout << "  关键: 字段编号唯一且永不重用\n";

    std::cout << "\nSchema演进工具:\n";
    std::cout << "  Protocol Buffers: 最成熟的方案\n";
    std::cout << "  FlatBuffers: 零拷贝, 游戏常用\n";
    std::cout << "  Apache Avro: Hadoop生态\n";
    std::cout << "  Cap'n Proto: 极致性能\n";
}

void demo_serialization_comparison() {
    std::cout << "\n=== demo_serialization_comparison ===\n";
    std::cout << "序列化方案对比\n\n";

    std::cout << "  ┌──────────────┬────────┬────────┬──────────┬──────────┐\n";
    std::cout << "  │ 方案         │ 格式   │ 性能   │ 可读性   │ Schema   │\n";
    std::cout << "  ├──────────────┼────────┼────────┼──────────┼──────────┤\n";
    std::cout << "  │ JSON         │ 文本   │ 慢     │ 高       │ 无       │\n";
    std::cout << "  │ MessagePack  │ 二进制 │ 中     │ 低       │ 无       │\n";
    std::cout << "  │ Protobuf     │ 二进制 │ 快     │ 低       │ 有       │\n";
    std::cout << "  │ FlatBuffers  │ 二进制 │ 极快   │ 低       │ 有       │\n";
    std::cout << "  │ Cap'n Proto  │ 二进制 │ 极快   │ 低       │ 有       │\n";
    std::cout << "  │ BSON         │ 二进制 │ 中     │ 低       │ 无       │\n";
    std::cout << "  │ CBOR         │ 二进制 │ 中     │ 低       │ 无       │\n";
    std::cout << "  └──────────────┴────────┴────────┴──────────┴──────────┘\n\n";

    std::cout << "选择建议:\n";
    std::cout << "  Web API: JSON (通用, 可读)\n";
    std::cout << "  微服务内部: Protobuf (高效, 有Schema)\n";
    std::cout << "  游戏/实时: FlatBuffers (零拷贝)\n";
    std::cout << "  配置文件: JSON/YAML/TOML\n";
    std::cout << "  日志: JSON (便于日志系统解析)\n";
}

int main() {
    std::cout << "序列化模式深入\n";

    demo_visitor_pattern();
    demo_reflection_concept();
    demo_schema_evolution();
    demo_serialization_comparison();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
