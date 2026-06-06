/** @file 01_deep_dive_regex_engine.cpp
 *  @brief 深入正则引擎：DFA vs NFA、性能陷阱、灾难性回溯、优化技巧
 *  @description 对应文档: 02-CPP/16-regex | 举一反三：理解正则引擎内部机制，避免性能陷阱
 *  编译命令: g++ -std=c++20 01_deep_dive_regex_engine.cpp -o 01_deep_dive_regex_engine
 */

#include <iostream>
#include <string>
#include <regex>
#include <chrono>
#include <vector>

void demo_engine_types() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  正则引擎类型：DFA vs NFA\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. DFA (确定性有限自动机)\n";
    std::cout << "   - 匹配时间与正则表达式无关，只与输入长度有关\n";
    std::cout << "   - 不支持捕获组和反向引用\n";
    std::cout << "   - 内存消耗可能较大(状态数可能指数增长)\n";
    std::cout << "   - 代表: grep(部分模式)、lex\n\n";

    std::cout << "2. NFA (非确定性有限自动机) —— C++ std::regex 使用\n";
    std::cout << "   - 支持捕获组、反向引用等高级特性\n";
    std::cout << "   - 匹配时间与正则表达式复杂度相关\n";
    std::cout << "   - 可能出现灾难性回溯(catastrophic backtracking)\n";
    std::cout << "   - 代表: PCRE、Python re、Java regex、std::regex\n\n";

    std::cout << "3. C++ std::regex 的选择:\n";
    std::cout << "   - 默认使用 ECMAScript 语法(NFA引擎)\n";
    std::cout << "   - 也支持 basic, extended, awk, grep, egrep 语法\n";
    std::cout << "   - 注意: std::regex 性能通常不如 PCRE/RE2\n";
    std::cout << "   - 生产环境建议考虑 re2 或 boost::regex\n";
}

void demo_catastrophic_backtracking() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  灾难性回溯 (Catastrophic Backtracking)\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "经典陷阱: (a+)+ 匹配非全a字符串\n";
    std::cout << "引擎会尝试所有可能的分组方式，导致指数级回溯\n\n";

    auto measure = [](const std::string& pattern, const std::string& text) -> double {
        try {
            std::regex re(pattern);
            auto start = std::chrono::high_resolution_clock::now();
            bool matched = std::regex_match(text, re);
            auto end = std::chrono::high_resolution_clock::now();
            double ms = std::chrono::duration<double, std::milli>(end - start).count();
            std::cout << "  文本长度 " << text.length()
                      << ", 耗时 " << ms << " ms"
                      << ", 匹配: " << matched << "\n";
            return ms;
        } catch (const std::regex_error& e) {
            std::cout << "  正则错误: " << e.what() << "\n";
            return -1;
        }
    };

    std::cout << "危险模式: (a+)+b\n";
    std::string dangerous = "(a+)+b";
    for (int len : {5, 10, 15, 20, 25}) {
        std::string text(len, 'a');
        measure(dangerous, text);
    }

    std::cout << "\n安全模式: a+b (等价但无回溯问题)\n";
    std::string safe = "a+b";
    for (int len : {5, 10, 15, 20, 25}) {
        std::string text(len, 'a');
        measure(safe, text);
    }

    std::cout << "\n关键教训:\n";
    std::cout << "  - 嵌套量词 (如 (a+)+, (a*)*) 是灾难性回溯的根源\n";
    std::cout << "  - 当匹配失败时，引擎会尝试所有可能的路径\n";
    std::cout << "  - 解决方案: 使用占有量词(如 a++)或原子组(如 (?>a+))\n";
    std::cout << "  - 注意: std::regex 不支持占有量词和原子组，需重构模式\n";
}

void demo_nesting_quantifiers() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  常见嵌套量词陷阱与修复\n";
    std::cout << "═══════════════════════════════════════\n\n";

    struct TrapExample {
        std::string desc;
        std::string bad;
        std::string good;
        std::string reason;
    };

    TrapExample traps[] = {
        {
            "重复的字符组",
            "(\\w+\\s+)+\\w+",
            "\\w+(\\s+\\w+)+",
            "将量词移到外层，减少回溯路径"
        },
        {
            "可选的重复组",
            "(a+)+b",
            "a+b",
            "消除不必要的嵌套量词"
        },
        {
            "贪婪的替代",
            "(.|\\n)*",
            "[\\s\\S]*",
            "使用字符类替代交替，减少分支"
        },
    };

    for (const auto& t : traps) {
        std::cout << "场景: " << t.desc << "\n";
        std::cout << "  危险: " << t.bad << "\n";
        std::cout << "  安全: " << t.good << "\n";
        std::cout << "  原因: " << t.reason << "\n\n";
    }
}

void demo_optimization_tips() {
    std::cout << "═══════════════════════════════════════\n";
    std::cout << "  正则表达式优化技巧\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::cout << "1. 预编译正则表达式\n";
    std::cout << "   正则编译开销大，避免在循环中重复编译\n\n";

    auto bad_approach = []() {
        std::vector<std::string> inputs = {"abc", "123", "xyz", "456", "test"};
        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& s : inputs) {
            std::regex re("\\d+");
            std::regex_match(s, re);
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    };

    auto good_approach = []() {
        std::vector<std::string> inputs = {"abc", "123", "xyz", "456", "test"};
        std::regex re("\\d+");
        auto start = std::chrono::high_resolution_clock::now();
        for (const auto& s : inputs) {
            std::regex_match(s, re);
        }
        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration<double, std::milli>(end - start).count();
    };

    double bad_ms = bad_approach();
    double good_ms = good_approach();
    std::cout << "   循环中编译: " << bad_ms << " ms\n";
    std::cout << "   预编译一次: " << good_ms << " ms\n";
    std::cout << "   提升: " << (bad_ms / good_ms) << "x\n\n";

    std::cout << "2. 使用更具体的模式\n";
    std::cout << "   .*  →  [^,]*  (排除逗号，减少回溯)\n";
    std::cout << "   \\d+ →  [0-9]{1,4}  (限制长度)\n";
    std::cout << "   .+  →  \\w+  (限制字符范围)\n\n";

    std::cout << "3. 避免不必要的捕获组\n";
    std::cout << "   (pattern)  →  (?:pattern)  (非捕获组，减少开销)\n\n";

    std::regex capture_re("(\\d+)-(\\d+)-(\\d+)");
    std::regex nocapture_re("(?:\\d+)-(?:\\d+)-(?:\\d+)");

    std::string test_str = "2024-01-15";
    auto t1 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; i++) std::regex_match(test_str, capture_re);
    auto t2 = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 10000; i++) std::regex_match(test_str, nocapture_re);
    auto t3 = std::chrono::high_resolution_clock::now();

    double capture_ms = std::chrono::duration<double, std::milli>(t2 - t1).count();
    double nocapture_ms = std::chrono::duration<double, std::milli>(t3 - t2).count();
    std::cout << "   捕获组: " << capture_ms << " ms / 10000次\n";
    std::cout << "   非捕获组: " << nocapture_ms << " ms / 10000次\n\n";

    std::cout << "4. 使用 regex_search 代替 regex_match\n";
    std::cout << "   如果只需检查是否包含，用 search 更快\n";
    std::cout << "   match 要求整个字符串匹配，开销更大\n\n";

    std::cout << "5. 考虑替代方案\n";
    std::cout << "   - 简单匹配: string::find / string::compare\n";
    std::cout << "   - 高性能需求: re2 库(线性时间保证)\n";
    std::cout << "   - 复杂解析: 手写解析器可能更高效\n";
}

void demo_regex_error_handling() {
    std::cout << "\n═══════════════════════════════════════\n";
    std::cout << "  正则表达式错误处理\n";
    std::cout << "═══════════════════════════════════════\n\n";

    std::vector<std::string> bad_patterns = {
        "([unclosed",
        "*invalid",
        "a{3,2}",
        "[z-a]",
    };

    for (const auto& pattern : bad_patterns) {
        try {
            std::regex re(pattern);
            std::cout << "  \"" << pattern << "\" —— 编译成功(意外)\n";
        } catch (const std::regex_error& e) {
            std::cout << "  \"" << pattern << "\" —— 错误: " << e.what() << "\n";
        }
    }

    std::cout << "\nregex_error 错误码:\n";
    std::cout << "  error_collate    —— 无效的排序元素\n";
    std::cout << "  error_ctype      —— 无效的字符类名\n";
    std::cout << "  error_escape     —— 无效的转义序列\n";
    std::cout << "  error_backref    —— 无效的反向引用\n";
    std::cout << "  error_brack      —— 方括号不匹配\n";
    std::cout << "  error_paren      —— 圆括号不匹配\n";
    std::cout << "  error_brace      —— 花括号不匹配\n";
    std::cout << "  error_badbrace   —— 花括号中内容无效\n";
    std::cout << "  error_range      —— 无效的字符范围\n";
    std::cout << "  error_space      —— 内存不足\n";
    std::cout << "  error_badrepeat  —— 量词前无可重复内容\n";
    std::cout << "  error_complexity —— 匹配复杂度超限\n";
    std::cout << "  error_stack      —— 栈空间不足\n";

    std::cout << "\n最佳实践: 始终用 try-catch 包裹正则编译\n";
    std::cout << "  try { std::regex re(user_input); }\n";
    std::cout << "  catch (const std::regex_error& e) { 处理无效模式 }\n";
}

int main() {
    demo_engine_types();
    demo_catastrophic_backtracking();
    demo_nesting_quantifiers();
    demo_optimization_tips();
    demo_regex_error_handling();
    return 0;
}
