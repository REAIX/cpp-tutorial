/**
 * @file constants.h
 * @brief 通用常量定义 (C 版本)
 *
 * 提供字符编码、换行符等通用常量。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_CONSTANTS_H
#define CU_CONSTANTS_H

#ifdef __cplusplus
extern "C" {
#endif

#define CU_CHARSET_UTF8 "UTF-8"
#define CU_CHARSET_UTF16 "UTF-16"
#define CU_CHARSET_UTF32 "UTF-32"
#define CU_CHARSET_ISO_8859_1 "ISO-8859-1"
#define CU_CHARSET_GB2312 "GB2312"
#define CU_CHARSET_GB18030 "GB18030"
#define CU_CHARSET_GBK "GBK"
#define CU_CHARSET_BIG5 "Big5"
#define CU_CHARSET_US_ASCII "US-ASCII"
#define CU_CHARSET_DEFAULT CU_CHARSET_UTF8

#define CU_LINEFEED_WINDOWS "\r\n"
#define CU_LINEFEED_UNIX "\n"
#define CU_LINEFEED_DEFAULT CU_LINEFEED_UNIX

#ifdef __cplusplus
}
#endif

#endif
