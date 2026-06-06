/**
 * @file constants.h
 * @brief 通用常量定义
 *
 * 提供字符编码、换行符等通用常量，供所有工具类统一引用。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_UTILS_CONSTANTS_H
#define CU_UTILS_CONSTANTS_H

namespace cu {

/**
 * @brief 常量命名空间
 */
namespace constants {

/**
 * @brief 常用字符编码名称
 */
namespace CharSets {
    /** @brief UTF-8 字符编码 */
    inline constexpr const char* UTF8 = "UTF-8";
    /** @brief UTF-16 字符编码 */
    inline constexpr const char* UTF16 = "UTF-16";
    /** @brief UTF-32 字符编码 */
    inline constexpr const char* UTF32 = "UTF-32";
    /** @brief ISO-8859-1 字符编码 */
    inline constexpr const char* ISO_8859_1 = "ISO-8859-1";
    /** @brief GB2312 字符编码 */
    inline constexpr const char* GB2312 = "GB2312";
    /** @brief GB18030 字符编码 */
    inline constexpr const char* GB18030 = "GB18030";
    /** @brief GBK 字符编码 */
    inline constexpr const char* GBK = "GBK";
    /** @brief BIG5 字符编码 */
    inline constexpr const char* BIG5 = "Big5";
    /** @brief US-ASCII 字符编码 */
    inline constexpr const char* US_ASCII = "US-ASCII";
    /** @brief 西欧 Windows 字符编码 */
    inline constexpr const char* WEST_EUROPE_WINDOWS = "Windows-1252";
    /** @brief 中欧 Windows 字符编码 */
    inline constexpr const char* CENTRAL_EUROPE_WINDOWS = "Windows-1250";
    /** @brief 默认字符编码 */
    inline constexpr const char* DEFAULT = UTF8;
}

/**
 * @brief 换行符常量
 */
namespace Const {
    /** @brief Windows 换行符 (\r\n) */
    inline constexpr const char* WINDOWS_LINEFEED = "\r\n";
    /** @brief Unix/Linux 换行符 (\n) */
    inline constexpr const char* UNIX_LINEFEED = "\n";
}

}

}

#endif