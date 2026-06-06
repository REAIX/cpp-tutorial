/** @file 02_example_regex_practice.cpp
 *  @brief 正则表达式实战：邮箱验证、URL解析、日志解析、数据提取
 *  @description 对应文档: 02-CPP/16-regex | 正则表达式在实际场景中的应用
 *  编译命令: g++ -std=c++20 02_example_regex_practice.cpp -o 02_example_regex_practice
 */

#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <map>

void demo_email_validation() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  邮箱地址验证\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::regex email_pattern(
        R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})"
    );

    std::vector<std::string> test_emails = {
        "user@example.com",
        "john.doe@company.co.uk",
        "admin+tag@domain.org",
        "invalid-email",
        "@no-user.com",
        "user@.com",
        "user@domain",
        "test@123.456.789.com",
    };

    std::cout << "邮箱模式: [a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}\n\n";

    for (const auto& email : test_emails) {
        bool valid = std::regex_match(email, email_pattern);
        std::cout << (valid ? "  ✓ " : "  ✗ ") << email << "\n";
    }

    std::cout << "\n提取邮箱各部分:\n";
    std::regex email_parts(R"(([a-zA-Z0-9._%+-]+)@([a-zA-Z0-9.-]+)\.([a-zA-Z]{2,}))");
    std::string email = "john.doe@company.co.uk";
    std::smatch match;
    if (std::regex_match(email, match, email_parts)) {
        std::cout << "  完整: " << match.str(0) << "\n";
        std::cout << "  用户名: " << match.str(1) << "\n";
        std::cout << "  域名: " << match.str(2) << "\n";
        std::cout << "  顶级域: " << match.str(3) << "\n";
    }
}

void demo_url_parsing() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  URL 解析\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::regex url_pattern(
        R"((https?)://([^/:]+)(?::(\d+))?(/[^?#]*)?(\?[^#]*)?(#.*)?)"
    );

    std::vector<std::string> urls = {
        "https://www.example.com/path/to/page?query=hello#section1",
        "http://localhost:8080/api/data",
        "https://github.com/user/repo",
        "ftp://invalid-protocol.com/file",
    };

    for (const auto& url : urls) {
        std::smatch match;
        if (std::regex_match(url, match, url_pattern)) {
            std::cout << "URL: " << url << "\n";
            std::cout << "  协议: " << match.str(1) << "\n";
            std::cout << "  主机: " << match.str(2) << "\n";
            std::cout << "  端口: " << (match.str(3).empty() ? "默认" : match.str(3)) << "\n";
            std::cout << "  路径: " << (match.str(4).empty() ? "/" : match.str(4)) << "\n";
            std::cout << "  查询: " << (match.str(5).empty() ? "无" : match.str(5)) << "\n";
            std::cout << "  锚点: " << (match.str(6).empty() ? "无" : match.str(6)) << "\n\n";
        } else {
            std::cout << "URL: " << url << " —— 不匹配\n\n";
        }
    }

    std::cout << "从文本中提取所有URL:\n";
    std::string text = "访问 https://example.com 或 http://test.org:3000/api 获取数据";
    auto url_begin = std::sregex_iterator(text.begin(), text.end(), url_pattern);
    auto url_end = std::sregex_iterator();
    for (auto it = url_begin; it != url_end; ++it) {
        std::cout << "  找到: " << it->str() << "\n";
    }
}

void demo_log_parsing() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  日志解析\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::vector<std::string> log_lines = {
        R"([2024-01-15 08:30:12] INFO  [main] Application started)",
        R"([2024-01-15 08:30:15] WARN  [worker-1] Connection timeout, retrying...)",
        R"([2024-01-15 08:30:20] ERROR [db-pool] Failed to connect to database: Connection refused)",
        R"([2024-01-15 08:30:25] INFO  [main] Server listening on port 8080)",
        R"([2024-01-15 08:31:00] DEBUG [http-2] Request: GET /api/users?id=42)",
    };

    std::regex log_pattern(
        R"(\[(\d{4}-\d{2}-\d{2} \d{2}:\d{2}:\d{2})\]\s+(\w+)\s+\[(\w[\w-]*)\]\s+(.*))"
    );

    std::map<std::string, int> level_count;

    std::cout << "日志格式: [时间] 级别 [模块] 消息\n\n";

    for (const auto& line : log_lines) {
        std::smatch match;
        if (std::regex_match(line, match, log_pattern)) {
            std::string timestamp = match.str(1);
            std::string level = match.str(2);
            std::string module = match.str(3);
            std::string message = match.str(4);

            level_count[level]++;

            if (level == "ERROR" || level == "WARN") {
                std::cout << "  ⚠ [" << level << "] " << timestamp
                          << " [" << module << "] " << message << "\n";
            }
        }
    }

    std::cout << "\n日志级别统计:\n";
    for (const auto& [level, count] : level_count) {
        std::cout << "  " << level << ": " << count << " 条\n";
    }

    std::cout << "\n提取所有IP地址:\n";
    std::string access_log =
        "192.168.1.100 - - [15/Jan/2024:08:30:12] GET /index.html\n"
        "10.0.0.1 - - [15/Jan/2024:08:30:15] POST /api/login\n"
        "172.16.0.50 - - [15/Jan/2024:08:30:20] GET /dashboard";

    std::regex ip_pattern(R"((\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}))");
    auto ip_begin = std::sregex_iterator(access_log.begin(), access_log.end(), ip_pattern);
    auto ip_end = std::sregex_iterator();
    for (auto it = ip_begin; it != ip_end; ++it) {
        std::cout << "  IP: " << it->str() << "\n";
    }
}

void demo_data_extraction() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  数据提取模式\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 提取HTML标签内容:\n";
    std::string html = "<title>Cpp教程</title><p>正则表达式</p><h1>标题</h1>";
    std::regex tag_pattern(R"(<(\w+)>([^<]*)</\1>)");
    auto tag_begin = std::sregex_iterator(html.begin(), html.end(), tag_pattern);
    auto tag_end = std::sregex_iterator();
    for (auto it = tag_begin; it != tag_end; ++it) {
        std::cout << "  标签<" << it->str(1) << ">: " << it->str(2) << "\n";
    }

    std::cout << "\n2. 提取键值对:\n";
    std::string config = "host=localhost; port=3306; dbname=testdb; user=admin";
    std::regex kv_pattern(R"((\w+)\s*=\s*([^;]+))");
    auto kv_begin = std::sregex_iterator(config.begin(), config.end(), kv_pattern);
    auto kv_end = std::sregex_iterator();
    for (auto it = kv_begin; it != kv_end; ++it) {
        std::cout << "  " << it->str(1) << " = " << it->str(2) << "\n";
    }

    std::cout << "\n3. 提取中文姓名和手机号:\n";
    std::string contacts = "联系人: 张三 13800138000, 李四 13900139000, 王五 15012345678";
    std::regex cn_pattern(R"(([\u4e00-\u9fa5]{2,4})\s*(1[3-9]\d{9}))");
    auto cn_begin = std::sregex_iterator(contacts.begin(), contacts.end(), cn_pattern);
    auto cn_end = std::sregex_iterator();
    for (auto it = cn_begin; it != cn_end; ++it) {
        std::cout << "  姓名: " << it->str(1) << ", 手机: " << it->str(2) << "\n";
    }

    std::cout << "\n4. 敏感信息脱敏:\n";
    std::string sensitive = "身份证: 110101199001011234, 银行卡: 6222021234567890123";
    std::regex id_pattern(R"((\d{6})\d{8}(\d{4}))");
    std::regex card_pattern(R"((\d{4})\d{8,11}(\d{4}))");

    std::string masked = std::regex_replace(sensitive, id_pattern, "$1********$2");
    masked = std::regex_replace(masked, card_pattern, "$1***********$2");
    std::cout << "  原文: " << sensitive << "\n";
    std::cout << "  脱敏: " << masked << "\n";
}

int main() {
    demo_email_validation();
    demo_url_parsing();
    demo_log_parsing();
    demo_data_extraction();
    return 0;
}
