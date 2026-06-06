/** @file 02_deep_dive_regex_patterns.cpp
 *  @brief 正则模式手册、Unicode支持、正则 vs 手动解析对比
 *  @description 对应文档: 02-CPP/16-regex | 举一反三：常用模式速查、Unicode处理、选择合适的工具
 *  编译命令: g++ -std=c++20 02_deep_dive_regex_patterns.cpp -o 02_deep_dive_regex_patterns
 */

#include <iostream>
#include <string>
#include <regex>
#include <vector>
#include <chrono>
#include <algorithm>
#include <cctype>

void demo_common_patterns_cookbook() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  常用正则模式速查手册\n";
    std::cout << "═══════════════════════════════════════\n\n";

    struct Pattern {
        std::string name;
        std::string pattern;
        std::string description;
        std::string example;
    };

    Pattern patterns[] = {
        {"整数", R"(-?\d+)", "可选负号+数字", "-42"},
        {"正整数", R"(\d+)", "一个或多个数字", "123"},
        {"浮点数", R"(-?\d+\.?\d*)", "整数部分+可选小数", "3.14"},
        {"科学计数", R"(-?\d+\.?\d*[eE][+-]?\d+)", "浮点+e/E+指数", "1.5e-3"},
        {"十六进制", R"(0[xX][0-9a-fA-F]+)", "0x/0X前缀", "0xDEADBEEF"},
        {"邮箱", R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})", "标准邮箱格式", "user@mail.com"},
        {"手机号", R"(1[3-9]\d{9})", "中国大陆手机号", "13800138000"},
        {"身份证", R"(\d{17}[\dXx])", "18位身份证号", "110101199001011234"},
        {"日期", R"(\d{4}[-/]\d{2}[-/]\d{2})", "YYYY-MM-DD 或 YYYY/MM/DD", "2024-01-15"},
        {"时间", R"(\d{2}:\d{2}(:\d{2})?)", "HH:MM 或 HH:MM:SS", "08:30:15"},
        {"IPv4", R"((\d{1,3}\.){3}\d{1,3})", "四段点分十进制", "192.168.1.1"},
        {"URL", R"(https?://[\w./%-]+)", "HTTP/HTTPS链接", "https://example.com/path"},
        {"中文字符", R"([\u4e00-\u9fa5]+)", "CJK统一汉字", "中文"},
        {"空白行", R"(\n\s*\r?)", "仅含空白的行", ""},
        {"HTML标签", R"(<[^>]+>)", "尖括号标签", "<div class='x'>"},
        {"重复单词", R"(\b(\w+)\s+\1\b)", "连续出现两次的单词", "the the"},
    };

    for (const auto& p : patterns) {
        std::cout << "【" << p.name << "】\n";
        std::cout << "  模式: " << p.pattern << "\n";
        std::cout << "  说明: " << p.description << "\n";
        if (!p.example.empty()) {
            try {
                std::regex re(p.pattern);
                std::cout << "  示例: \"" << p.example << "\" → "
                          << (std::regex_search(p.example, re) ? "匹配" : "不匹配") << "\n";
            } catch (const std::regex_error& e) {
                std::cout << "  示例: 编译错误\n";
            }
        }
        std::cout << "\n";
    }
}

void demo_regex_unicode() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  正则表达式与 Unicode\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. Unicode 范围匹配\n";
    std::cout << "   C++ regex 支持 \\uXXXX 形式的 Unicode 转义\n\n";

    std::cout << "   常用 Unicode 范围:\n";
    std::cout << "   \\u4e00-\\u9fa5  —— CJK统一汉字(基本)\n";
    std::cout << "   \\u3000-\\u303f  —— CJK标点符号\n";
    std::cout << "   \\uff00-\\uffef  —— 全角字符\n";
    std::cout << "   \\u0080-\\u00ff  —— Latin-1补充\n";
    std::cout << "   \\u0400-\\u04ff  —— 西里尔字母\n\n";

    std::cout << "2. 中文文本处理示例:\n";
    std::string cn_text = "你好，世界！Hello World 123";
    std::regex cn_char(R"([\u4e00-\u9fa5])");
    std::regex cn_punct(R"([\u3000-\u303f\uff00-\uffef])");

    auto cn_begin = std::sregex_iterator(cn_text.begin(), cn_text.end(), cn_char);
    auto cn_end = std::sregex_iterator();
    int cn_count = std::distance(cn_begin, cn_end);
    std::cout << "   文本: \"" << cn_text << "\"\n";
    std::cout << "   中文字符数: " << cn_count << "\n";

    std::cout << "\n3. Unicode 注意事项:\n";
    std::cout << "   - std::regex 基于 char，处理 UTF-8 需注意多字节\n";
    std::cout << "   - \\w 不匹配中文字符，需用 [\\u4e00-\\u9fa5] 显式指定\n";
    std::cout << "   - 宽字符版本 std::wregex 可处理 wchar_t\n";
    std::cout << "   - C++20 引入 <unicode> 但 std::regex 的 Unicode 支持仍有限\n";
    std::cout << "   - 复杂 Unicode 处理建议使用 ICU 库\n";

    std::cout << "\n4. 宽字符正则示例:\n";
    std::wstring wtext = L"中文测试ABC123";
    std::wregex wcn(L"[\\u4e00-\\u9fa5]+");
    std::wsmatch wmatch;
    if (std::regex_search(wtext, wmatch, wcn)) {
        std::wcout << L"   找到中文: " << wmatch.str() << L"\n";
    }
}

void demo_regex_vs_manual_parsing() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  正则 vs 手动解析：选择合适的工具\n";
    std::cout << "═══════════════════════════════════════\n\n";

    const int ITERATIONS = 5000;
    std::vector<std::string> test_data;
    for (int i = 0; i < ITERATIONS; i++) {
        test_data.push_back("key" + std::to_string(i) + "=value" + std::to_string(i * 2));
    }

    std::cout << "任务: 从 \"key=value\" 格式中提取键和值\n";
    std::cout << "数据量: " << ITERATIONS << " 条\n\n";

    auto regex_parse = [&]() {
        std::regex kv_re(R"((\w+)=(\w+))");
        int count = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& s : test_data) {
            std::smatch match;
            if (std::regex_match(s, match, kv_re)) {
                count++;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    };

    auto manual_parse = [&]() {
        int count = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& s : test_data) {
            auto pos = s.find('=');
            if (pos != std::string::npos && pos > 0 && pos < s.size() - 1) {
                count++;
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    };

    auto string_parse = [&]() {
        int count = 0;
        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& s : test_data) {
            size_t eq = s.find('=');
            if (eq != std::string::npos) {
                std::string key = s.substr(0, eq);
                std::string value = s.substr(eq + 1);
                if (!key.empty() && !value.empty()) {
                    count++;
                }
            }
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    };

    double regex_ms = regex_parse();
    double manual_ms = manual_parse();
    double string_ms = string_parse();

    std::cout << "  正则表达式: " << regex_ms << " ms\n";
    std::cout << "  手动查找:   " << manual_ms << " ms\n";
    std::cout << "  字符串分割: " << string_ms << " ms\n";
    std::cout << "  正则/手动比: " << (regex_ms / manual_ms) << "x\n\n";

    std::cout << "选择指南:\n";
    std::cout << "  ┌──────────────┬──────────┬──────────┐\n";
    std::cout << "  │ 场景         │ 正则     │ 手动     │\n";
    std::cout << "  ├──────────────┼──────────┼──────────┤\n";
    std::cout << "  │ 简单分隔符   │ 慢       │ 快 ✓     │\n";
    std::cout << "  │ 复杂模式     │ 快 ✓     │ 慢/复杂  │\n";
    std::cout << "  │ 可读性       │ 高 ✓     │ 低       │\n";
    std::cout << "  │ 性能关键     │ 慢       │ 快 ✓     │\n";
    std::cout << "  │ 嵌套结构     │ 不适合   │ 适合 ✓   │\n";
    std::cout << "  │ 一次性脚本   │ 适合 ✓   │ 不值得   │\n";
    std::cout << "  └──────────────┴──────────┴──────────┘\n";
}

void demo_regex_gotchas() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  正则表达式常见陷阱\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. C++字符串转义 vs 正则转义\n";
    std::cout << "   需要双重转义: \\\\ 在C++字符串中表示 \\, \\\\d 在正则中表示 \\d\n";
    std::cout << "   推荐使用原始字符串: R\"(\\d+)\" 代替 \"\\\\d+\"\n\n";

    std::regex escaped_re("\\d+");
    std::regex raw_re(R"(\d+)");
    std::cout << "   \"\\\\d+\" 和 R\"(\\d+)\" 效果相同\n\n";

    std::cout << "2. 贪婪 vs 非贪婪\n";
    std::string html = "<div>hello</div><span>world</span>";
    std::regex greedy(R"(<.*>)");
    std::regex lazy(R"(<.*?>)");
    std::smatch m;

    if (std::regex_search(html, m, greedy)) {
        std::cout << "   贪婪 <.*>: \"" << m.str() << "\"\n";
    }
    if (std::regex_search(html, m, lazy)) {
        std::cout << "   非贪婪 <.*?>: \"" << m.str() << "\"\n";
    }

    std::cout << "\n3. 锚点的重要性\n";
    std::regex no_anchor("\\d+");
    std::regex with_anchor("^\\d+$");
    std::string mixed = "abc123def";
    std::cout << "   \"abc123def\" 匹配 \\d+: " << std::regex_search(mixed, no_anchor) << "\n";
    std::cout << "   \"abc123def\" 匹配 ^\\d+$: " << std::regex_match(mixed, with_anchor) << "\n";
    std::cout << "   ^ 和 $ 确保完整匹配，防止部分匹配\n\n";

    std::cout << "4. 边界 vs 非边界\n";
    std::string words = "cat catch caterpillar";
    std::regex word_boundary(R"(\bcat\b)");
    std::regex no_boundary("cat");
    std::cout << "   文本: \"" << words << "\"\n";
    std::cout << "   \\bcat\\b: ";
    auto wb_begin = std::sregex_iterator(words.begin(), words.end(), word_boundary);
    auto wb_end = std::sregex_iterator();
    for (auto it = wb_begin; it != wb_end; ++it) {
        std::cout << "\"" << it->str() << "\" ";
    }
    std::cout << "\n   cat(无边界): ";
    auto nb_begin = std::sregex_iterator(words.begin(), words.end(), no_boundary);
    auto nb_end = std::sregex_iterator();
    for (auto it = nb_begin; it != nb_end; ++it) {
        std::cout << "\"" << it->str() << "\" ";
    }
    std::cout << "\n";
}

int main() {
    demo_common_patterns_cookbook();
    demo_regex_unicode();
    demo_regex_vs_manual_parsing();
    demo_regex_gotchas();
    return 0;
}
