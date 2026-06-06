/** @file 01_example_regex_basics.cpp
 *  @brief C++正则表达式基础：std::regex, regex_match, regex_search, regex_replace
 *  @description 对应文档: 02-CPP/16-regex | 演示正则表达式基本用法和常见模式
 *  编译命令: g++ -std=c++20 01_example_regex_basics.cpp -o 01_example_regex_basics
 */

#include <iostream>
#include <string>
#include <regex>

void demo_regex_match() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  std::regex_match —— 完整匹配\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::regex pattern("\\d{3}-\\d{4}");

    std::string phone1 = "010-1234";
    std::string phone2 = "010-12345";
    std::string phone3 = "abc-1234";

    std::cout << "模式: \\d{3}-\\d{4} (三位数字-四位数字)\n\n";
    std::cout << "\"" << phone1 << "\" 匹配: " << std::regex_match(phone1, pattern) << "\n";
    std::cout << "\"" << phone2 << "\" 匹配: " << std::regex_match(phone2, pattern) << "\n";
    std::cout << "\"" << phone3 << "\" 匹配: " << std::regex_match(phone3, pattern) << "\n";

    std::cout << "\nregex_match 要求整个字符串完全匹配模式，\n";
    std::cout << "不能只匹配子串。这是与 regex_search 的关键区别。\n";
}

void demo_regex_search() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::regex_search —— 子串搜索\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::regex pattern("\\d+");

    std::string text = "abc123def456ghi789";
    std::cout << "文本: \"" << text << "\"\n";
    std::cout << "模式: \\d+ (一个或多个数字)\n\n";

    std::smatch match;
    if (std::regex_search(text, match, pattern)) {
        std::cout << "找到第一个匹配: \"" << match.str() << "\"\n";
        std::cout << "匹配位置: " << match.position() << "\n";
        std::cout << "匹配长度: " << match.length() << "\n";
    }

    std::cout << "\n迭代搜索所有匹配:\n";
    std::string::const_iterator searchStart = text.cbegin();
    int count = 0;
    while (std::regex_search(searchStart, text.cend(), match, pattern)) {
        std::cout << "  匹配 " << ++count << ": \"" << match.str()
                  << "\" (位置 " << match.position() << ")\n";
        searchStart = match.suffix().first;
    }
}

void demo_regex_replace() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  std::regex_replace —— 替换匹配\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::string text = "2024-01-15 和 2024-12-25";
    std::regex date_pattern("(\\d{4})-(\\d{2})-(\\d{2})");

    std::cout << "原文: \"" << text << "\"\n";
    std::cout << "模式: (\\d{4})-(\\d{2})-(\\d{2})\n\n";

    std::string result1 = std::regex_replace(text, date_pattern, "$2/$3/$1");
    std::cout << "替换为 $2/$3/$1: \"" << result1 << "\"\n";

    std::string result2 = std::regex_replace(text, date_pattern, "YYYY-MM-DD");
    std::cout << "替换为固定文本: \"" << result2 << "\"\n";

    std::string text2 = "hello   world   cpp";
    std::regex space_pattern("\\s+");
    std::string result3 = std::regex_replace(text2, space_pattern, " ");
    std::cout << "\n多空格合并: \"" << text2 << "\" → \"" << result3 << "\"\n";

    std::string text3 = "Hello World Cpp";
    std::regex word_pattern("\\b\\w");
    std::string result4 = std::regex_replace(text3, word_pattern, "[$&]");
    std::cout << "标记单词首字母: \"" << text3 << "\" → \"" << result4 << "\"\n";
    std::cout << "  ($& 表示整个匹配)\n";
}

void demo_capture_groups() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  捕获组与匹配结果\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::string text = "张三: 13800138000, 李四: 13900139000";
    std::regex pattern("(\\w+)[:：]\\s*(1\\d{10})");

    std::cout << "文本: \"" << text << "\"\n";
    std::cout << "模式: (\\w+)[:：]\\s*(1\\d{10})\n\n";

    std::sregex_iterator it(text.begin(), text.end(), pattern);
    std::sregex_iterator end;

    int matchCount = 0;
    for (; it != end; ++it) {
        const std::smatch& match = *it;
        std::cout << "匹配 " << ++matchCount << ":\n";
        std::cout << "  完整匹配: \"" << match.str(0) << "\"\n";
        std::cout << "  姓名(组1): \"" << match.str(1) << "\"\n";
        std::cout << "  电话(组2): \"" << match.str(2) << "\"\n\n";
    }
}

void demo_regex_flags() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  正则表达式标志位\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::string text = "Hello\nWorld";

    std::regex default_regex("Hello.World");
    std::cout << "默认(ECMAScript): \"Hello.World\" 匹配 \"Hello\\nWorld\": "
              << std::regex_match(text, default_regex) << "\n";

    std::cout << "\n注意: std::regex::dotall 在部分实现中不可用\n";
    std::cout << "  替代方案: 使用 [\\s\\S] 代替 . 来匹配任意字符(含换行)\n";

    std::string text2 = "ABCabc";
    std::regex icase_regex("abc", std::regex::ECMAScript | std::regex::icase);
    std::cout << "icase模式: \"abc\" 匹配 \"ABCabc\"(search): "
              << std::regex_search(text2, icase_regex) << "\n";

    std::cout << "\n常用标志位:\n";
    std::cout << "  std::regex::ECMAScript —— 默认，ECMAScript语法\n";
    std::cout << "  std::regex::icase      —— 忽略大小写\n";
    std::cout << "  std::regex::nosubs     —— 不保存捕获组\n";
    std::cout << "  std::regex::dotall     —— . 匹配换行符\n";
}

void demo_basic_patterns() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  常见正则模式速查\n";
    std::cout << "═══════════════════════════════════════\n\n";

    struct PatternTest {
        std::string name;
        std::string pattern;
        std::string positive;
        std::string negative;
    };

    PatternTest tests[] = {
        {"整数", "-?\\d+", "123", "abc"},
        {"浮点数", "-?\\d+\\.\\d+", "3.14", "42"},
        {"标识符", "[a-zA-Z_]\\w*", "_var1", "123abc"},
        {"IPv4", "\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}", "192.168.1.1", "999.999.999.999"},
        {"十六进制", "0[xX][0-9a-fA-F]+", "0xFF", "0xGG"},
    };

    for (const auto& t : tests) {
        std::regex re(t.pattern);
        std::cout << t.name << " (" << t.pattern << ")\n";
        std::cout << "  正例 \"" << t.positive << "\": " << std::regex_match(t.positive, re) << "\n";
        std::cout << "  反例 \"" << t.negative << "\": " << std::regex_match(t.negative, re) << "\n\n";
    }
}

int main() {
    demo_regex_match();
    demo_regex_search();
    demo_regex_replace();
    demo_capture_groups();
    demo_regex_flags();
    demo_basic_patterns();
    return 0;
}
