/**
 * @file 01_example_json_serialize.cpp
 * @brief JSON序列化/反序列化: 简单JSON写入/解析, 结构化数据转JSON
 * @description 对应文档: 02-CPP/36-序列化与日志
 */

#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>
#include <cstdint>
#include <cmath>
#include <algorithm>

class SimpleJsonWriter {
    std::ostringstream oss_;
    bool first_ = true;

    void comma() {
        if (!first_) oss_ << ",";
        first_ = false;
    }

    static std::string escape(const std::string& s) {
        std::string result;
        for (char c : s) {
            switch (c) {
                case '"': result += "\\\""; break;
                case '\\': result += "\\\\"; break;
                case '\n': result += "\\n"; break;
                case '\r': result += "\\r"; break;
                case '\t': result += "\\t"; break;
                default: result += c;
            }
        }
        return result;
    }

public:
    SimpleJsonWriter& begin_object() {
        comma();
        oss_ << "{";
        first_ = true;
        return *this;
    }

    SimpleJsonWriter& end_object() {
        oss_ << "}";
        first_ = false;
        return *this;
    }

    SimpleJsonWriter& begin_array() {
        comma();
        oss_ << "[";
        first_ = true;
        return *this;
    }

    SimpleJsonWriter& end_array() {
        oss_ << "]";
        first_ = false;
        return *this;
    }

    SimpleJsonWriter& key(const std::string& k) {
        comma();
        oss_ << "\"" << escape(k) << "\":";
        first_ = true;
        return *this;
    }

    SimpleJsonWriter& value(const std::string& v) {
        comma();
        oss_ << "\"" << escape(v) << "\"";
        return *this;
    }

    SimpleJsonWriter& value(int v) {
        comma();
        oss_ << v;
        return *this;
    }

    SimpleJsonWriter& value(double v) {
        comma();
        if (std::isfinite(v)) oss_ << v;
        else oss_ << "null";
        return *this;
    }

    SimpleJsonWriter& value(bool v) {
        comma();
        oss_ << (v ? "true" : "false");
        return *this;
    }

    SimpleJsonWriter& null() {
        comma();
        oss_ << "null";
        return *this;
    }

    std::string str() const { return oss_.str(); }
};

class SimpleJsonParser {
    std::string json_;
    size_t pos_ = 0;

    void skip_whitespace() {
        while (pos_ < json_.size() && (json_[pos_] == ' ' || json_[pos_] == '\n'
               || json_[pos_] == '\r' || json_[pos_] == '\t')) {
            ++pos_;
        }
    }

    std::string parse_string() {
        skip_whitespace();
        if (pos_ >= json_.size() || json_[pos_] != '"') return "";
        ++pos_;
        std::string result;
        while (pos_ < json_.size() && json_[pos_] != '"') {
            if (json_[pos_] == '\\' && pos_ + 1 < json_.size()) {
                ++pos_;
                switch (json_[pos_]) {
                    case '"': result += '"'; break;
                    case '\\': result += '\\'; break;
                    case 'n': result += '\n'; break;
                    case 'r': result += '\r'; break;
                    case 't': result += '\t'; break;
                    default: result += json_[pos_];
                }
            } else {
                result += json_[pos_];
            }
            ++pos_;
        }
        if (pos_ < json_.size()) ++pos_;
        return result;
    }

    std::string parse_raw_value() {
        skip_whitespace();
        std::string result;
        while (pos_ < json_.size() && json_[pos_] != ',' && json_[pos_] != '}'
               && json_[pos_] != ']' && json_[pos_] != ' ' && json_[pos_] != '\n') {
            result += json_[pos_];
            ++pos_;
        }
        return result;
    }

public:
    explicit SimpleJsonParser(const std::string& json) : json_(json) {}

    std::map<std::string, std::string> parse_object() {
        std::map<std::string, std::string> result;
        skip_whitespace();
        if (pos_ >= json_.size() || json_[pos_] != '{') return result;
        ++pos_;

        while (pos_ < json_.size()) {
            skip_whitespace();
            if (json_[pos_] == '}') { ++pos_; break; }
            if (json_[pos_] == ',') { ++pos_; continue; }

            std::string key = parse_string();
            skip_whitespace();
            if (pos_ < json_.size() && json_[pos_] == ':') ++pos_;
            skip_whitespace();

            if (pos_ < json_.size() && json_[pos_] == '"') {
                result[key] = parse_string();
            } else {
                result[key] = parse_raw_value();
            }
        }
        return result;
    }
};

void demo_json_writer() {
    std::cout << "\n=== demo_json_writer ===\n";
    std::cout << "JSON写入器\n\n";

    SimpleJsonWriter writer;
    writer.begin_object();
    writer.key("name").value("张三");
    writer.key("age").value(25);
    writer.key("score").value(95.5);
    writer.key("active").value(true);
    writer.key("hobbies").begin_array()
        .value("编程").value("阅读").value("游戏")
        .end_array();
    writer.key("address").begin_object()
        .key("city").value("北京")
        .key("zip").value("100000")
        .end_object();
    writer.key("remark").null();
    writer.end_object();

    std::string json = writer.str();
    std::cout << "生成的JSON:\n  " << json << "\n";
}

void demo_json_parser() {
    std::cout << "\n=== demo_json_parser ===\n";
    std::cout << "JSON解析器\n\n";

    std::string json = R"({"name":"李四","age":30,"score":88.5,"active":false,"city":"上海"})";
    std::cout << "输入JSON: " << json << "\n\n";

    SimpleJsonParser parser(json);
    auto result = parser.parse_object();

    std::cout << "解析结果:\n";
    for (const auto& [key, value] : result) {
        std::cout << "  " << key << " = " << value << "\n";
    }
}

void demo_struct_to_json() {
    std::cout << "\n=== demo_struct_to_json ===\n";
    std::cout << "结构体与JSON互转\n\n";

    struct Student {
        std::string name;
        int age;
        double gpa;
        std::vector<std::string> courses;
    };

    Student stu{"王五", 20, 3.8, {"数学", "物理", "编程"}};

    auto student_to_json = [](const Student& s) {
        SimpleJsonWriter w;
        w.begin_object();
        w.key("name").value(s.name);
        w.key("age").value(s.age);
        w.key("gpa").value(s.gpa);
        w.key("courses").begin_array();
        for (const auto& c : s.courses) w.value(c);
        w.end_array();
        w.end_object();
        return w.str();
    };

    std::string json = student_to_json(stu);
    std::cout << "结构体 -> JSON:\n  " << json << "\n\n";

    std::cout << "JSON -> 结构体 (简化解析):\n";
    SimpleJsonParser parser(json);
    auto fields = parser.parse_object();
    Student parsed;
    parsed.name = fields["name"];
    parsed.age = std::stoi(fields["age"]);
    parsed.gpa = std::stod(fields["gpa"]);
    std::cout << "  name=" << parsed.name << ", age=" << parsed.age << ", gpa=" << parsed.gpa << "\n";

    std::cout << "\n生产环境建议:\n";
    std::cout << "  使用成熟的JSON库: nlohmann/json, RapidJSON, simdjson\n";
    std::cout << "  nlohmann/json: 最易用, C++11, header-only\n";
    std::cout << "  RapidJSON: 高性能, SAX/DOM两种API\n";
    std::cout << "  simdjson: 最快(SIMD加速), 仅解析\n";
}

int main() {
    std::cout << "JSON序列化演示\n";

    demo_json_writer();
    demo_json_parser();
    demo_struct_to_json();

    std::cout << "\n所有演示完成!\n";
    return 0;
}
