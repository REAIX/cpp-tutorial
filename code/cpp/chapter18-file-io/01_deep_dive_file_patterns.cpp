/** @file 01_deep_dive_file_patterns.cpp
 *  @brief 二进制序列化、CSV解析、配置文件读取、流缓冲定制
 *  @description 对应文档: 02-CPP/18-file-io | 举一反三：文件I/O高级模式和实用技巧
 *  编译命令: g++ -std=c++20 01_deep_dive_file_patterns.cpp -o 01_deep_dive_file_patterns
 */

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <cstdint>
#include <cstring>

const std::string TEST_DIR = "test_io_data/";

void demo_binary_serialization() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  二进制序列化\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 固定大小结构体序列化:\n";
    struct Player {
        int32_t id;
        float x, y, z;
        int32_t health;
        char name[16];
    };

    {
        std::ofstream out(TEST_DIR + "players.bin", std::ios::binary);
        Player players[] = {
            {1, 10.5f, 20.3f, 0.0f, 100, "Warrior"},
            {2, -5.0f, 15.0f, 3.0f, 85, "Mage"},
            {3, 0.0f, 0.0f, 0.0f, 100, "Archer"},
        };

        int32_t count = 3;
        out.write(reinterpret_cast<const char*>(&count), sizeof(count));
        for (const auto& p : players) {
            out.write(reinterpret_cast<const char*>(&p), sizeof(Player));
        }
        std::cout << "  写入 " << count << " 条玩家记录\n";
    }

    {
        std::ifstream in(TEST_DIR + "players.bin", std::ios::binary);
        int32_t count;
        in.read(reinterpret_cast<char*>(&count), sizeof(count));
        std::cout << "  读取 " << count << " 条记录:\n";

        for (int32_t i = 0; i < count; i++) {
            Player p;
            in.read(reinterpret_cast<char*>(&p), sizeof(Player));
            std::cout << "    " << p.name << " (" << p.id << ") pos=("
                      << p.x << "," << p.y << "," << p.z << ") HP=" << p.health << "\n";
        }
    }

    std::cout << "\n2. 变长字符串序列化:\n";
    {
        std::ofstream out(TEST_DIR + "messages.bin", std::ios::binary);
        std::vector<std::string> messages = {"Hello", "世界", "C++17 序列化"};

        for (const auto& msg : messages) {
            uint32_t len = static_cast<uint32_t>(msg.size());
            out.write(reinterpret_cast<const char*>(&len), sizeof(len));
            out.write(msg.data(), len);
        }
        std::cout << "  写入 " << messages.size() << " 条变长消息\n";
    }

    {
        std::ifstream in(TEST_DIR + "messages.bin", std::ios::binary);
        std::cout << "  读取变长消息:\n";
        while (in) {
            uint32_t len;
            in.read(reinterpret_cast<char*>(&len), sizeof(len));
            if (!in) break;
            std::string msg(len, '\0');
            in.read(&msg[0], len);
            if (!in) break;
            std::cout << "    [" << len << "字节] " << msg << "\n";
        }
    }

    std::cout << "\n序列化注意事项:\n";
    std::cout << "  - 结构体可能有填充字节，sizeof可能大于字段之和\n";
    std::cout << "  - 不同平台字节序可能不同(大端 vs 小端)\n";
    std::cout << "  - 指针不能序列化，需要序列化指向的数据\n";
    std::cout << "  - 生产环境建议使用 protobuf/cereal 等库\n";
}

void demo_csv_parsing() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  CSV 文件解析\n";
    std::cout << "═══════════════════════════════════════\n\n";

    {
        std::ofstream out(TEST_DIR + "data.csv");
        out << "姓名,年龄,城市,分数\n";
        out << "张三,25,北京,95.5\n";
        out << "李四,30,上海,87.3\n";
        out << "王五,28,\"广州,广东\",92.1\n";
        out << "\"赵,六\",22,深圳,78.9\n";
    }

    auto parse_csv_line = [](const std::string& line) -> std::vector<std::string> {
        std::vector<std::string> fields;
        std::string field;
        bool in_quotes = false;

        for (size_t i = 0; i < line.size(); i++) {
            char c = line[i];
            if (in_quotes) {
                if (c == '"') {
                    if (i + 1 < line.size() && line[i + 1] == '"') {
                        field += '"';
                        i++;
                    } else {
                        in_quotes = false;
                    }
                } else {
                    field += c;
                }
            } else {
                if (c == '"') {
                    in_quotes = true;
                } else if (c == ',') {
                    fields.push_back(field);
                    field.clear();
                } else {
                    field += c;
                }
            }
        }
        fields.push_back(field);
        return fields;
    };

    {
        std::ifstream in(TEST_DIR + "data.csv");
        std::string line;
        bool header = true;
        while (std::getline(in, line)) {
            if (line.empty()) continue;
            auto fields = parse_csv_line(line);
            if (header) {
                std::cout << "表头: ";
                for (size_t i = 0; i < fields.size(); i++) {
                    if (i > 0) std::cout << " | ";
                    std::cout << fields[i];
                }
                std::cout << "\n";
                header = false;
            } else {
                std::cout << "数据: ";
                for (size_t i = 0; i < fields.size(); i++) {
                    if (i > 0) std::cout << " | ";
                    std::cout << fields[i];
                }
                std::cout << "\n";
            }
        }
    }

    std::cout << "\nCSV解析要点:\n";
    std::cout << "  - 字段包含逗号时用双引号包裹\n";
    std::cout << "  - 字段包含双引号时用两个双引号转义\n";
    std::cout << "  - 注意处理换行符在引号内的情况\n";
    std::cout << "  - 生产环境建议使用专用CSV库\n";
}

void demo_config_file() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  配置文件读取\n";
    std::cout << "═══════════════════════════════════════\n\n";

    {
        std::ofstream out(TEST_DIR + "config.ini");
        out << "# 应用配置文件\n";
        out << "\n";
        out << "[database]\n";
        out << "host = localhost\n";
        out << "port = 3306\n";
        out << "name = mydb\n";
        out << "\n";
        out << "[server]\n";
        out << "port = 8080\n";
        out << "debug = true\n";
        out << "max_connections = 100\n";
    }

    auto trim = [](std::string s) -> std::string {
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.front()))) s.erase(s.begin());
        while (!s.empty() && std::isspace(static_cast<unsigned char>(s.back()))) s.pop_back();
        return s;
    };

    std::map<std::string, std::map<std::string, std::string>> config;
    std::string current_section;

    {
        std::ifstream in(TEST_DIR + "config.ini");
        std::string line;
        while (std::getline(in, line)) {
            line = trim(line);
            if (line.empty() || line[0] == '#') continue;

            if (line.front() == '[' && line.back() == ']') {
                current_section = line.substr(1, line.size() - 2);
                continue;
            }

            auto eq_pos = line.find('=');
            if (eq_pos != std::string::npos) {
                std::string key = trim(line.substr(0, eq_pos));
                std::string value = trim(line.substr(eq_pos + 1));
                config[current_section][key] = value;
            }
        }
    }

    std::cout << "解析结果:\n";
    for (const auto& [section, entries] : config) {
        std::cout << "  [" << section << "]\n";
        for (const auto& [key, value] : entries) {
            std::cout << "    " << key << " = " << value << "\n";
        }
    }

    std::cout << "\n配置文件格式要点:\n";
    std::cout << "  - INI格式: [section] + key=value\n";
    std::cout << "  - 注释以 # 或 ; 开头\n";
    std::cout << "  - 等号两侧可有空格\n";
    std::cout << "  - 空行应被忽略\n";
}

void demo_streambuf_customization() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  流缓冲定制基础\n";
    std::cout << "═══════════════════════════════════════\n\n";

    class TeeBuf : public std::streambuf {
    public:
        TeeBuf(std::streambuf* buf1, std::streambuf* buf2)
            : buf1_(buf1), buf2_(buf2) {}

    protected:
        int_type overflow(int_type c) override {
            if (c != EOF) {
                if (buf1_->sputc(c) == EOF) return EOF;
                if (buf2_->sputc(c) == EOF) return EOF;
            }
            return c;
        }

        int sync() override {
            int ok = 0;
            if (buf1_->pubsync() != 0) ok = -1;
            if (buf2_->pubsync() != 0) ok = -1;
            return ok;
        }

    private:
        std::streambuf* buf1_;
        std::streambuf* buf2_;
    };

    class TeeStream : public std::ostream {
    public:
        TeeStream(std::ostream& os1, std::ostream& os2)
            : std::ostream(&tee_buf_), tee_buf_(os1.rdbuf(), os2.rdbuf()) {}

    private:
        TeeBuf tee_buf_;
    };

    {
        std::ofstream log_file(TEST_DIR + "tee_output.log");
        TeeStream tee(std::cout, log_file);

        tee << "这行同时输出到控制台和文件!\n";
        tee << "TeeBuf 将输出复制到两个流缓冲\n";
        tee << "适用于日志记录场景\n";
    }

    std::cout << "\nstreambuf 定制要点:\n";
    std::cout << "  - overflow() 处理单个字符写入\n";
    std::cout << "  - underflow() 处理读取\n";
    std::cout << "  - sync() 刷新缓冲区\n";
    std::cout << "  - 可实现: 日志分流、加密流、压缩流等\n";
}

int main() {
    demo_binary_serialization();
    demo_csv_parsing();
    demo_config_file();
    demo_streambuf_customization();
    return 0;
}
