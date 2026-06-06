/**
 * @file string_utils.cpp
 * @brief 字符串处理工具实现
 *
 * 实现字符串截取、空白判断、填充、十六进制转换等功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#include "cu_utils/string_utils.h"
#include <cstdarg>
#include <regex>
#include <sstream>
#include <iomanip>
#include <cstdint>

namespace cu {

/**
 * @brief 将 nullptr 转换为空字符串
 * @param s 字符串指针
 * @return 非空字符串
 */
std::string StringUtils::nullToEmpty(const std::string* s) {
    return s ? *s : "";
}

/**
 * @brief 重复字符串
 *
 * 将字符串 s 重复 count 次。
 *
 * @param s 要重复的字符串
 * @param count 重复次数
 * @return 重复后的字符串，count<=0 返回空字符串
 */
std::string StringUtils::repeat(const std::string& s, int count) {
    if (count <= 0) return "";
    std::string result;
    result.reserve(s.size() * count);
    for (int i = 0; i < count; ++i) {
        result += s;
    }
    return result;
}

/**
 * @brief 截取子串（左开右开区间）
 *
 * 从 src 中截取「第一个 fstr 之后」到「第一个 lstr 之前」的子串，
 * 不包含 fstr 和 lstr 本身。
 *
 * @param src 源字符串
 * @param fstr 起始标记
 * @param lstr 结束标记
 * @return 截取的子串
 */
std::string StringUtils::subStr(const std::string& src, const std::string& fstr, const std::string& lstr) {
    if (src.empty()) return "";

    size_t v1 = src.find(fstr);
    if (v1 == std::string::npos) return "";
    v1 += fstr.size();

    size_t v2 = lstr.empty() ? src.size() : src.find(lstr, v1);
    if (v2 == std::string::npos) return "";

    return src.substr(v1, v2 - v1);
}

/**
 * @brief 截取子串（左闭右开区间）
 *
 * 截取「从第一个 fstr 开始」到「第一个 lstr 结束」的子串，
 * 包含 fstr 但不包含 lstr。
 *
 * @param src 源字符串
 * @param fstr 起始标记
 * @param lstr 结束标记
 * @return 截取的子串
 */
std::string StringUtils::lsubStr(const std::string& src, const std::string& fstr, const std::string& lstr) {
    if (src.empty()) return "";

    size_t v1 = src.find(fstr);
    if (v1 == std::string::npos) return "";

    size_t v2 = lstr.empty() ? src.size() : src.find(lstr, v1);
    if (v2 == std::string::npos) return "";

    return src.substr(v1, v2 - v1);
}

/**
 * @brief 截取子串（左开右闭区间）
 *
 * 截取「第一个 fstr 之后」到「第一个 lstr 结束」的子串，
 * 不包含 fstr 但包含 lstr。
 *
 * @param src 源字符串
 * @param fstr 起始标记
 * @param lstr 结束标记
 * @return 截取的子串
 */
std::string StringUtils::rsubStr(const std::string& src, const std::string& fstr, const std::string& lstr) {
    if (src.empty()) return "";

    size_t v1 = src.find(fstr);
    if (v1 == std::string::npos) return "";
    v1 += fstr.size();

    size_t v2 = lstr.empty() ? src.size() : src.find(lstr, v1);
    if (v2 == std::string::npos) return "";
    v2 += lstr.size();

    return src.substr(v1, v2 - v1);
}

/**
 * @brief 截取子串（左闭右闭区间）
 *
 * 截取「从第一个 fstr 开始」到「第一个 lstr 结束」的子串，
 * 同时包含 fstr 和 lstr。
 *
 * @param src 源字符串
 * @param fstr 起始标记
 * @param lstr 结束标记
 * @return 截取的子串
 */
std::string StringUtils::asubStr(const std::string& src, const std::string& fstr, const std::string& lstr) {
    if (src.empty()) return "";

    size_t v1 = src.find(fstr);
    if (v1 == std::string::npos) return "";

    size_t v2 = lstr.empty() ? src.size() : src.find(lstr, v1);
    if (v2 == std::string::npos) return "";
    v2 += lstr.size();

    return src.substr(v1, v2 - v1);
}

/**
 * @brief 判断字符串是否为空
 * @param str 字符串指针
 * @return 如果为 nullptr 或空字符串返回 true
 */
bool StringUtils::isEmpty(const std::string* str) {
    return !str || str->empty();
}

/**
 * @brief 判断字符串是否为空白
 *
 * 检查字符串是否为 nullptr、空字符串，或仅包含空白字符
 *（空格、制表符、换行符等）。
 *
 * @param str 字符串指针
 * @return 如果为空白返回 true
 */
bool StringUtils::isBlank(const std::string* str) {
    if (!str || str->empty()) return true;

    for (char c : *str) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            return false;
        }
    }

    return true;
}

/**
 * @brief 左侧填充字符
 *
 * 使用 pad 字符从左侧填充，使字符串达到 length 长度。
 * 如果 input 已够长则原样返回。input 为 nullptr 当作空字符串处理。
 *
 * @param input 输入字符串指针
 * @param length 目标长度
 * @param pad 填充字符
 * @return 填充后的字符串
 */
std::string StringUtils::leftPad(const std::string* input, int length, char pad) {
    std::string str = nullToEmpty(input);
    if (length < 0 || str.size() >= static_cast<size_t>(length)) return str;

    return repeat(std::string(1, pad), length - str.size()) + str;
}

/**
 * @brief 右侧填充字符
 *
 * 使用 pad 字符从右侧填充，使字符串达到 length 长度。
 * 如果 input 已够长则原样返回。input 为 nullptr 当作空字符串处理。
 *
 * @param input 输入字符串指针
 * @param length 目标长度
 * @param pad 填充字符
 * @return 填充后的字符串
 */
std::string StringUtils::rightPad(const std::string* input, int length, char pad) {
    std::string str = nullToEmpty(input);
    if (length < 0 || str.size() >= static_cast<size_t>(length)) return str;

    return str + repeat(std::string(1, pad), length - str.size());
}

/**
 * @brief 合并连续制表符
 *
 * 将连续多个制表符 \t 合并为一个 \t。
 *
 * @param str 输入字符串指针
 * @return 合并后的字符串
 */
std::string StringUtils::oneTab(const std::string* str) {
    if (!str) return "";

    std::regex regex("\\t+");
    return std::regex_replace(*str, regex, "\t");
}

/**
 * @brief 合并连续空格
 *
 * 将制表符替换为空格，再将连续空格合并为一个空格。
 *
 * @param str 输入字符串指针
 * @return 合并后的字符串
 */
std::string StringUtils::oneBlank(const std::string* str) {
    if (!str) return "";

    std::regex tabRegex("\\t+");
    std::string result = std::regex_replace(*str, tabRegex, " ");

    std::regex spaceRegex(" +");
    return std::regex_replace(result, spaceRegex, " ");
}

/**
 * @brief 统计子字符串出现次数
 *
 * 统计 substr 在 str 中不重叠出现的次数。
 *
 * @param str 源字符串
 * @param substr 子字符串
 * @return 子字符串出现的次数
 */
int StringUtils::strCount(const std::string& str, const std::string& substr) {
    if (str.empty() || substr.empty()) return 0;

    int count = 0;
    size_t idx = 0;
    while ((idx = str.find(substr, idx)) != std::string::npos) {
        count++;
        idx += substr.size();
    }

    return count;
}

/**
 * @brief 十六进制字符串转 ASCII 字符串
 *
 * 将十六进制字符串解码为 ASCII 字符串，会去掉空格；
 * 长度为奇数时左侧补 0。
 *
 * @param hexStr 十六进制字符串
 * @return 解码后的 ASCII 字符串
 */
std::string StringUtils::hex2ASCII(const std::string& hexStr) {
    if (hexStr.empty()) return "";

    std::string cleaned;
    for (char c : hexStr) {
        if (!std::isspace(static_cast<unsigned char>(c))) {
            cleaned += c;
        }
    }

    if (cleaned.empty()) return "";

    if (cleaned.size() % 2 != 0) {
        cleaned = "0" + cleaned;
    }

    std::string result;
    result.reserve(cleaned.size() / 2);

    for (size_t i = 0; i < cleaned.size(); i += 2) {
        std::string hex = cleaned.substr(i, 2);
        try {
            char byte = static_cast<char>(std::stoul(hex, nullptr, 16));
            result += byte;
        } catch (const std::invalid_argument&) {
            continue;
        } catch (const std::out_of_range&) {
            continue;
        }
    }

    return result;
}

/**
 * @brief 字符串居中填充
 *
 * 将字符串居中填充到 totalLength，左右用 paddingChar 填充；
 * 不足时左少右多。paddingChar 为 nullptr 或空时用空格。
 *
 * @param input 输入字符串指针
 * @param totalLength 总长度
 * @param paddingChar 填充字符指针
 * @return 居中后的字符串
 */
std::string StringUtils::formatCenter(const std::string* input, int totalLength, const std::string* paddingChar) {
    std::string str = input ? *input : "";
    if (str.size() >= static_cast<size_t>(totalLength)) return str;

    std::string p = paddingChar && !paddingChar->empty() ? *paddingChar : " ";
    int pad = totalLength - static_cast<int>(str.size());
    int left = pad / 2;
    int right = pad - left;

    return repeat(p, left) + str + repeat(p, right);
}

/**
 * @brief 字符串转十六进制表示
 *
 * 将字符串按 UTF-8 转为十六进制表示（大写、无分隔）。
 *
 * @param input 输入字符串指针
 * @return 十六进制字符串
 */
std::string StringUtils::stringToHex(const std::string* input) {
    if (!input) return "";

    std::ostringstream oss;
    oss << std::hex << std::uppercase;

    for (unsigned char c : *input) {
        oss << std::setw(2) << std::setfill('0') << static_cast<int>(c);
    }

    return oss.str();
}

/**
 * @brief 字符串转十六进制字节列表
 *
 * 将字符串按 UTF-8 转为十六进制字节列表，每字节两位如 "4A"。
 *
 * @param input 输入字符串指针
 * @return 十六进制字节列表
 */
std::vector<std::string> StringUtils::stringToHexList(const std::string* input) {
    std::vector<std::string> result;
    if (!input) return result;

    for (unsigned char c : *input) {
        std::ostringstream oss;
        oss << std::hex << std::uppercase << std::setw(2) << std::setfill('0') << static_cast<int>(c);
        result.push_back(oss.str());
    }

    return result;
}

/**
 * @brief 去掉行首的第一个制表符
 *
 * 如果行首是制表符则去掉，否则返回原字符串。
 *
 * @param line 输入字符串指针
 * @return 处理后的字符串
 */
std::string StringUtils::removeFirstTab(const std::string* line) {
    if (!line || line->empty()) return "";

    if (!line->empty() && line->front() == '\t') {
        return line->substr(1);
    }

    return *line;
}

std::vector<std::string> StringUtils::split(const std::string& str, const std::string& delimiter) {
    std::vector<std::string> result;
    if (delimiter.empty()) {
        for (char c : str) {
            result.push_back(std::string(1, c));
        }
        return result;
    }

    size_t start = 0;
    size_t end = str.find(delimiter);
    while (end != std::string::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + delimiter.size();
        end = str.find(delimiter, start);
    }
    result.push_back(str.substr(start));
    return result;
}

std::string StringUtils::join(const std::vector<std::string>& list, const std::string& delimiter) {
    if (list.empty()) return "";

    std::string result = list[0];
    for (size_t i = 1; i < list.size(); ++i) {
        result += delimiter + list[i];
    }
    return result;
}

std::string StringUtils::ltrim(const std::string& str) {
    size_t start = 0;
    while (start < str.size() && std::isspace(static_cast<unsigned char>(str[start]))) {
        ++start;
    }
    return str.substr(start);
}

std::string StringUtils::rtrim(const std::string& str) {
    size_t end = str.size();
    while (end > 0 && std::isspace(static_cast<unsigned char>(str[end - 1]))) {
        --end;
    }
    return str.substr(0, end);
}

std::string StringUtils::trim(const std::string& str) {
    return ltrim(rtrim(str));
}

std::string StringUtils::replace(const std::string& str, const std::string& oldStr, const std::string& newStr) {
    if (oldStr.empty()) return str;

    std::string result;
    size_t start = 0;
    size_t pos = str.find(oldStr);
    while (pos != std::string::npos) {
        result += str.substr(start, pos - start);
        result += newStr;
        start = pos + oldStr.size();
        pos = str.find(oldStr, start);
    }
    result += str.substr(start);
    return result;
}

std::string StringUtils::replaceFirst(const std::string& str, const std::string& oldStr, const std::string& newStr) {
    if (oldStr.empty()) return str;

    size_t pos = str.find(oldStr);
    if (pos == std::string::npos) return str;

    return str.substr(0, pos) + newStr + str.substr(pos + oldStr.size());
}

std::string StringUtils::toUpperCase(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    }
    return result;
}

std::string StringUtils::toLowerCase(const std::string& str) {
    std::string result = str;
    for (char& c : result) {
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    }
    return result;
}

std::string StringUtils::capitalize(const std::string& str) {
    if (str.empty()) return "";

    std::string result = toLowerCase(str);
    result[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(result[0])));
    return result;
}

bool StringUtils::startsWith(const std::string& str, const std::string& prefix) {
    if (prefix.size() > str.size()) return false;
    return str.compare(0, prefix.size(), prefix) == 0;
}

bool StringUtils::endsWith(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool StringUtils::contains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

std::string StringUtils::format(const std::string& fmt, ...) {
    va_list args;
    va_start(args, fmt);

    va_list argsCopy;
    va_copy(argsCopy, args);

    int size = vsnprintf(nullptr, 0, fmt.c_str(), args);
    va_end(args);

    if (size < 0) {
        va_end(argsCopy);
        return "";
    }

    std::string result(size, '\0');
    vsnprintf(&result[0], size + 1, fmt.c_str(), argsCopy);
    va_end(argsCopy);

    return result;
}

bool StringUtils::isAlpha(const std::string& str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (!std::isalpha(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

bool StringUtils::isNumeric(const std::string& str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (!std::isdigit(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

bool StringUtils::isAlphanumeric(const std::string& str) {
    if (str.empty()) return false;
    for (char c : str) {
        if (!std::isalnum(static_cast<unsigned char>(c))) return false;
    }
    return true;
}

bool StringUtils::isEmail(const std::string& str) {
    if (str.empty()) return false;
    std::regex emailRegex("^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}$");
    return std::regex_match(str, emailRegex);
}

bool StringUtils::matches(const std::string& str, const std::string& pattern) {
    try {
        std::regex re(pattern);
        return std::regex_match(str, re);
    } catch (const std::regex_error&) {
        return false;
    }
}

std::vector<std::string> StringUtils::findAll(const std::string& str, const std::string& pattern) {
    std::vector<std::string> result;
    try {
        std::regex re(pattern);
        auto begin = std::sregex_iterator(str.begin(), str.end(), re);
        auto end = std::sregex_iterator();
        for (auto it = begin; it != end; ++it) {
            result.push_back(it->str());
        }
    } catch (const std::regex_error&) {
    }
    return result;
}

}