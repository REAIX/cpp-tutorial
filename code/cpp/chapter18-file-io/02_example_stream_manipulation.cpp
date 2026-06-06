/** @file 02_example_stream_manipulation.cpp
 *  @brief 流操纵器、格式化I/O、流状态、错误处理
 *  @description 对应文档: 02-CPP/18-file-io | 演示格式化输出和流状态管理
 *  编译命令: g++ -std=c++20 02_example_stream_manipulation.cpp -o 02_example_stream_manipulation
 */

#include <iostream>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <string>
#include <vector>
#include <limits>

const std::string TEST_DIR = "test_io_data/";

void demo_format_manipulators() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  格式化操纵器\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 宽度与对齐:\n";
    std::cout << "  左对齐:\n";
    std::cout << std::left;
    std::cout << "  |" << std::setw(10) << "姓名"
              << "|" << std::setw(8) << "年龄"
              << "|" << std::setw(10) << "成绩" << "|\n";
    std::cout << "  |" << std::setw(10) << "张三"
              << "|" << std::setw(8) << 20
              << "|" << std::setw(10) << 95.5 << "|\n";
    std::cout << "  |" << std::setw(10) << "李四"
              << "|" << std::setw(8) << 21
              << "|" << std::setw(10) << 87.3 << "|\n";

    std::cout << "\n  右对齐:\n";
    std::cout << std::right;
    std::cout << "  |" << std::setw(10) << "姓名"
              << "|" << std::setw(8) << "年龄"
              << "|" << std::setw(10) << "成绩" << "|\n";
    std::cout << "  |" << std::setw(10) << "张三"
              << "|" << std::setw(8) << 20
              << "|" << std::setw(10) << 95.5 << "|\n";

    std::cout << "\n2. 填充字符:\n";
    std::cout << std::setfill('0');
    std::cout << "  编号: " << std::setw(5) << 42 << "\n";
    std::cout << "  编号: " << std::setw(5) << 7 << "\n";
    std::cout << std::setfill(' ');

    std::cout << "\n3. 数值格式:\n";
    int val = 255;
    std::cout << "  十进制: " << std::dec << val << "\n";
    std::cout << "  十六进制: " << std::hex << val << "\n";
    std::cout << "  八进制: " << std::oct << val << "\n";
    std::cout << std::dec;

    std::cout << "\n4. 浮点精度:\n";
    double pi = 3.141592653589793;
    std::cout << "  默认:     " << pi << "\n";
    std::cout << "  2位小数:  " << std::setprecision(2) << pi << "\n";
    std::cout << "  6位小数:  " << std::setprecision(6) << pi << "\n";
    std::cout << "  科学计数: " << std::scientific << pi << "\n";
    std::cout << std::fixed << std::setprecision(4);
    std::cout << "  定点4位:  " << pi << "\n";
    std::cout << std::defaultfloat << std::setprecision(6);

    std::cout << "\n5. 布尔值格式:\n";
    std::cout << "  默认: " << true << " " << false << "\n";
    std::cout << "  字面: " << std::boolalpha << true << " " << false << "\n";
    std::cout << std::noboolalpha;

    std::cout << "\n6. 正号显示:\n";
    std::cout << "  默认: " << 42 << " " << -42 << "\n";
    std::cout << "  显示正号: " << std::showpos << 42 << " " << -42 << std::noshowpos << "\n";
}

void demo_formatted_table() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  格式化表格输出\n";
    std::cout << "═══════════════════════════════════════\n\n";

    struct Student {
        std::string name;
        int id;
        double score;
    };

    std::vector<Student> students = {
        {"张三", 1001, 95.5},
        {"李四", 1002, 87.3},
        {"王五", 1003, 92.1},
        {"赵六", 1004, 78.9},
    };

    {
        std::ofstream out(TEST_DIR + "report.txt");
        if (!out) {
            std::cerr << "无法创建报表文件\n";
            return;
        }

        auto write_line = [&](std::ostream& os) {
            os << "+" << std::string(8, '-') << "+"
               << std::string(8, '-') << "+"
               << std::string(10, '-') << "+\n";
        };

        write_line(out);
        out << "|" << std::left << std::setw(8) << "姓名"
            << "|" << std::right << std::setw(8) << "学号"
            << "|" << std::setw(10) << "成绩" << "|\n";
        write_line(out);

        for (const auto& s : students) {
            out << "|" << std::left << std::setw(8) << s.name
                << "|" << std::right << std::setw(8) << s.id
                << "|" << std::fixed << std::setprecision(1)
                << std::setw(10) << s.score << "|\n";
        }
        write_line(out);
    }

    std::ifstream in(TEST_DIR + "report.txt");
    std::cout << "报表内容:\n";
    std::string line;
    while (std::getline(in, line)) {
        std::cout << "  " << line << "\n";
    }
}

void demo_stream_state() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  流状态管理\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "流状态标志:\n";
    std::cout << "  goodbit  —— 一切正常\n";
    std::cout << "  eofbit   —— 到达文件末尾\n";
    std::cout << "  failbit  —— 格式错误(可恢复)\n";
    std::cout << "  badbit   —— 严重错误(不可恢复)\n\n";

    {
        std::istringstream iss("42 hello 3.14");
        int n;
        std::string s;
        double d;

        iss >> n >> s >> d;
        std::cout << "正常读取: n=" << n << ", s=" << s << ", d=" << d << "\n";
        std::cout << "  good(): " << iss.good() << ", eof(): " << iss.eof() << "\n";
    }

    {
        std::istringstream iss("abc");
        int n;
        iss >> n;
        std::cout << "\n类型不匹配: 试图从\"abc\"读取int\n";
        std::cout << "  fail(): " << iss.fail() << ", bad(): " << iss.bad() << "\n";
        std::cout << "  !iss: " << !iss << " (operator bool 取反)\n";

        iss.clear();
        std::cout << "  clear()后 fail(): " << iss.fail() << "\n";
    }

    std::cout << "\n状态检查方法:\n";
    std::cout << "  good()  —— 无任何错误\n";
    std::cout << "  eof()   —— 到达末尾\n";
    std::cout << "  fail()  —— failbit 或 badbit 被设置\n";
    std::cout << "  bad()   —— badbit 被设置\n";
    std::cout << "  operator bool() —— !fail()\n";
    std::cout << "  operator!()     —— fail()\n";
}

void demo_error_handling() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  文件I/O错误处理\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 打开文件失败:\n";
    {
        std::ifstream in("nonexistent_file.txt");
        if (!in) {
            std::cout << "  文件不存在，打开失败\n";
        }
        if (in.fail()) {
            std::cout << "  fail() 返回 true\n";
        }
    }

    std::cout << "\n2. 读取失败后恢复:\n";
    {
        std::istringstream iss("42 abc 99");
        int val;

        iss >> val;
        std::cout << "  读取 \"42\": " << val << "\n";

        iss >> val;
        std::cout << "  读取 \"abc\" 为int: 失败, fail=" << iss.fail() << "\n";

        iss.clear();
        std::string bad_input;
        iss >> bad_input;
        std::cout << "  跳过错误输入: \"" << bad_input << "\"\n";

        iss >> val;
        std::cout << "  继续读取 \"99\": " << val << "\n";
    }

    std::cout << "\n3. 异常模式:\n";
    {
        std::ifstream in;
        in.exceptions(std::ios::failbit | std::ios::badbit);
        try {
            in.open("nonexistent_file.txt");
        } catch (const std::ios_base::failure& e) {
            std::cout << "  捕获异常: " << e.what() << "\n";
            in.exceptions(std::ios::goodbit);
        }
    }

    std::cout << "\n4. 最佳实践:\n";
    std::cout << "  - 始终检查文件是否成功打开\n";
    std::cout << "  - 读取循环中检查流状态\n";
    std::cout << "  - 读取失败后 clear() 再继续\n";
    std::cout << "  - 使用 RAII 让析构函数自动关闭文件\n";
    std::cout << "  - 异常模式适合需要严格错误处理的场景\n";
}

void demo_stringstream() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  stringstream —— 字符串流\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 数值与字符串转换:\n";
    {
        std::ostringstream oss;
        oss << "数值: " << 42 << ", 浮点: " << std::fixed << std::setprecision(2) << 3.14159;
        std::string result = oss.str();
        std::cout << "  构建: \"" << result << "\"\n";
    }

    {
        std::istringstream iss("100 3.14 hello");
        int n;
        double d;
        std::string s;
        iss >> n >> d >> s;
        std::cout << "  解析: n=" << n << ", d=" << d << ", s=" << s << "\n";
    }

    std::cout << "\n2. 安全的数值转换函数:\n";
    auto to_string_safe = [](const std::string& str) -> int {
        std::istringstream iss(str);
        int val;
        if (iss >> val) {
            char remaining;
            if (!iss.get(remaining)) {
                return val;
            }
        }
        throw std::invalid_argument("无法转换: " + str);
    };

    try {
        std::cout << "  \"42\" → " << to_string_safe("42") << "\n";
        std::cout << "  \"abc\" → " << to_string_safe("abc") << "\n";
    } catch (const std::exception& e) {
        std::cout << "  异常: " << e.what() << "\n";
    }

    std::cout << "\n3. 逐行解析CSV:\n";
    std::string csv = "张三,20,95.5\n李四,21,87.3\n王五,22,92.1";
    std::istringstream csv_stream(csv);
    std::string line;
    while (std::getline(csv_stream, line)) {
        std::istringstream line_stream(line);
        std::string name, age_str, score_str;
        std::getline(line_stream, name, ',');
        std::getline(line_stream, age_str, ',');
        std::getline(line_stream, score_str, ',');
        std::cout << "  姓名: " << name << ", 年龄: " << age_str
                  << ", 成绩: " << score_str << "\n";
    }
}

int main() {
    demo_format_manipulators();
    demo_formatted_table();
    demo_stream_state();
    demo_error_handling();
    demo_stringstream();
    return 0;
}
