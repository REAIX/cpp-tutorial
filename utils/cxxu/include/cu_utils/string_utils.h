/**
 * @file string_utils.h
 * @brief 字符串处理工具
 *
 * 提供字符串截取、空白判断、填充、十六进制转换等功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_UTILS_STRING_UTILS_H
#define CU_UTILS_STRING_UTILS_H

#include "cu_utils/export.h"

// 注意：CXXU_API 不是 C++ 关键字，而是 export.h 中定义的跨平台导出宏。
// 它用于控制动态库符号的可见性，静态链接时展开为空。

#include <string>
#include <vector>

namespace cu {

/**
 * @brief 字符串处理工具类
 *
 * 提供静态方法进行字符串的各种操作，包括截取、填充、转换等。
 */
class CXXU_API StringUtils {
public:
    /** @brief 禁用默认构造函数 */
    StringUtils() = delete;
    /** @brief 禁用拷贝构造函数 */
    StringUtils(const StringUtils&) = delete;
    /** @brief 禁用赋值运算符 */
    StringUtils& operator=(const StringUtils&) = delete;

    /**
     * @brief 截取子串（左开右开区间）
     *
     * 从 src 中截取「第一个 fstr 之后」到「第一个 lstr 之前」的子串，
     * 不包含 fstr 和 lstr 本身。
     *
     * @param src 源字符串
     * @param fstr 起始标记字符串
     * @param lstr 结束标记字符串
     * @return 截取的子串，未找到时返回空字符串
     */
    static std::string subStr(const std::string& src, const std::string& fstr, const std::string& lstr);

    /**
     * @brief 截取子串（左闭右开区间）
     *
     * 截取「从第一个 fstr 开始」到「第一个 lstr 结束」的子串，
     * 包含 fstr 但不包含 lstr。
     *
     * @param src 源字符串
     * @param fstr 起始标记字符串
     * @param lstr 结束标记字符串
     * @return 截取的子串
     */
    static std::string lsubStr(const std::string& src, const std::string& fstr, const std::string& lstr);

    /**
     * @brief 截取子串（左开右闭区间）
     *
     * 截取「第一个 fstr 之后」到「第一个 lstr 结束」的子串，
     * 不包含 fstr 但包含 lstr。
     *
     * @param src 源字符串
     * @param fstr 起始标记字符串
     * @param lstr 结束标记字符串
     * @return 截取的子串
     */
    static std::string rsubStr(const std::string& src, const std::string& fstr, const std::string& lstr);

    /**
     * @brief 截取子串（左闭右闭区间）
     *
     * 截取「从第一个 fstr 开始」到「第一个 lstr 结束」的子串，
     * 同时包含 fstr 和 lstr。
     *
     * @param src 源字符串
     * @param fstr 起始标记字符串
     * @param lstr 结束标记字符串
     * @return 截取的子串
     */
    static std::string asubStr(const std::string& src, const std::string& fstr, const std::string& lstr);

    /**
     * @brief 判断字符串是否为空
     * @param str 字符串指针
     * @return 如果为 nullptr 或空字符串返回 true
     */
    static bool isEmpty(const std::string* str);

    /**
     * @brief 判断字符串是否为空白
     *
     * 检查字符串是否为 nullptr、空字符串，或仅包含空白字符
     *（空格、制表符、换行符等）。
     *
     * @param str 字符串指针
     * @return 如果为空白返回 true
     */
    static bool isBlank(const std::string* str);

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
    static std::string leftPad(const std::string* input, int length, char pad);

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
    static std::string rightPad(const std::string* input, int length, char pad);

    /**
     * @brief 合并连续制表符
     *
     * 将连续多个制表符 \t 合并为一个 \t。
     *
     * @param str 输入字符串指针
     * @return 合并后的字符串
     */
    static std::string oneTab(const std::string* str);

    /**
     * @brief 合并连续空格
     *
     * 将制表符替换为空格，再将连续空格合并为一个空格。
     *
     * @param str 输入字符串指针
     * @return 合并后的字符串
     */
    static std::string oneBlank(const std::string* str);

    /**
     * @brief 统计子字符串出现次数
     *
     * 统计 substr 在 str 中不重叠出现的次数。
     *
     * @param str 源字符串
     * @param substr 子字符串
     * @return 子字符串出现的次数
     */
    static int strCount(const std::string& str, const std::string& substr);

    /**
     * @brief 十六进制字符串转 ASCII 字符串
     *
     * 将十六进制字符串解码为 ASCII 字符串，会去掉空格；
     * 长度为奇数时左侧补 0。
     *
     * @param hexStr 十六进制字符串
     * @return 解码后的 ASCII 字符串
     */
    static std::string hex2ASCII(const std::string& hexStr);

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
    static std::string formatCenter(const std::string* input, int totalLength, const std::string* paddingChar);

    /**
     * @brief 字符串转十六进制表示
     *
     * 将字符串按 UTF-8 转为十六进制表示（大写、无分隔）。
     *
     * @param input 输入字符串指针
     * @return 十六进制字符串
     */
    static std::string stringToHex(const std::string* input);

    /**
     * @brief 字符串转十六进制字节列表
     *
     * 将字符串按 UTF-8 转为十六进制字节列表，每字节两位如 "4A"。
     *
     * @param input 输入字符串指针
     * @return 十六进制字节列表
     */
    static std::vector<std::string> stringToHexList(const std::string* input);

    /**
     * @brief 去掉行首的第一个制表符
     *
     * 如果行首是制表符则去掉，否则返回原字符串。
     *
     * @param line 输入字符串指针
     * @return 处理后的字符串
     */
    static std::string removeFirstTab(const std::string* line);

    static std::vector<std::string> split(const std::string& str, const std::string& delimiter);
    static std::string join(const std::vector<std::string>& list, const std::string& delimiter);
    static std::string trim(const std::string& str);
    static std::string ltrim(const std::string& str);
    static std::string rtrim(const std::string& str);
    static std::string replace(const std::string& str, const std::string& oldStr, const std::string& newStr);
    static std::string replaceFirst(const std::string& str, const std::string& oldStr, const std::string& newStr);
    static std::string toUpperCase(const std::string& str);
    static std::string toLowerCase(const std::string& str);
    static std::string capitalize(const std::string& str);
    static bool startsWith(const std::string& str, const std::string& prefix);
    static bool endsWith(const std::string& str, const std::string& suffix);
    static bool contains(const std::string& str, const std::string& substr);
    static std::string format(const std::string& fmt, ...);
    static bool isAlpha(const std::string& str);
    static bool isNumeric(const std::string& str);
    static bool isAlphanumeric(const std::string& str);
    static bool isEmail(const std::string& str);
    static bool matches(const std::string& str, const std::string& pattern);
    static std::vector<std::string> findAll(const std::string& str, const std::string& pattern);

private:
    /**
     * @brief 将 nullptr 转换为空字符串
     * @param s 字符串指针
     * @return 非空字符串
     */
    static std::string nullToEmpty(const std::string* s);

    /**
     * @brief 重复字符串
     *
     * 将字符串 s 重复 count 次。
     *
     * @param s 要重复的字符串
     * @param count 重复次数
     * @return 重复后的字符串，count<=0 返回空字符串
     */
    static std::string repeat(const std::string& s, int count);
};

}

#endif