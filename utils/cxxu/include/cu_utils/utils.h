/**
 * @file utils.h
 * @brief 统一入口头文件
 *
 * 包含所有工具模块的封装类，方便统一引用。
 * 提供静态方法调用所有工具功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_UTILS_UTILS_H
#define CU_UTILS_UTILS_H

#include "cu_utils/constants.h"
#include "cu_utils/string_utils.h"
#include "cu_utils/collection_utils.h"
#include "cu_utils/file_utils.h"
#include "cu_utils/encoding_utils.h"
#include "cu_utils/number_utils.h"
#include "cu_utils/random_utils.h"
#include "cu_utils/process_utils.h"
#include "cu_utils/date_time_utils.h"
#include "cu_utils/log_utils.h"
#include "cu_utils/config_utils.h"
#include "cu_utils/json_utils.h"
#include "cu_utils/hash_utils.h"

#include <string>
#include <vector>
#include <map>
#include <unordered_set>
#include <filesystem>

namespace cu {

/**
 * @brief 统一工具类
 *
 * 封装所有工具模块的静态方法，提供统一的调用入口。
 * 此类不可实例化，仅提供静态方法调用。
 */
class Utils {
public:
    /** @brief 禁用默认构造函数 */
    Utils() = delete;
    /** @brief 禁用拷贝构造函数 */
    Utils(const Utils&) = delete;
    /** @brief 禁用赋值运算符 */
    Utils& operator=(const Utils&) = delete;

    // ========== 常量 ==========

    /** @brief UTF-8 字符编码 */
    static constexpr const char* UTF8 = constants::CharSets::UTF8;
    /** @brief UTF-16 字符编码 */
    static constexpr const char* UTF16 = constants::CharSets::UTF16;
    /** @brief UTF-32 字符编码 */
    static constexpr const char* UTF32 = constants::CharSets::UTF32;
    /** @brief ISO-8859-1 字符编码 */
    static constexpr const char* ISO_8859_1 = constants::CharSets::ISO_8859_1;
    /** @brief GB2312 字符编码 */
    static constexpr const char* GB2312 = constants::CharSets::GB2312;
    /** @brief GB18030 字符编码 */
    static constexpr const char* GB18030 = constants::CharSets::GB18030;
    /** @brief GBK 字符编码 */
    static constexpr const char* GBK = constants::CharSets::GBK;
    /** @brief BIG5 字符编码 */
    static constexpr const char* BIG5 = constants::CharSets::BIG5;
    /** @brief US-ASCII 字符编码 */
    static constexpr const char* US_ASCII = constants::CharSets::US_ASCII;
    /** @brief 默认字符编码 */
    static constexpr const char* DEFAULT = constants::CharSets::DEFAULT;

    /** @brief Windows 换行符 */
    static constexpr const char* WINDOWS_LINEFEED = constants::Const::WINDOWS_LINEFEED;
    /** @brief Unix/Linux 换行符 */
    static constexpr const char* UNIX_LINEFEED = constants::Const::UNIX_LINEFEED;

    // ========== 字符串工具 ==========

    /**
     * @brief 截取子串（左开右开区间）
     * @param src 源字符串
     * @param fstr 起始标记
     * @param lstr 结束标记
     * @return 截取的子串
     */
    static std::string subStr(const std::string& src, const std::string& fstr, const std::string& lstr) {
        return StringUtils::subStr(src, fstr, lstr);
    }

    static std::string lsubStr(const std::string& src, const std::string& fstr, const std::string& lstr) {
        return StringUtils::lsubStr(src, fstr, lstr);
    }

    static std::string rsubStr(const std::string& src, const std::string& fstr, const std::string& lstr) {
        return StringUtils::rsubStr(src, fstr, lstr);
    }

    static std::string asubStr(const std::string& src, const std::string& fstr, const std::string& lstr) {
        return StringUtils::asubStr(src, fstr, lstr);
    }

    static bool isEmpty(const std::string* str) {
        return StringUtils::isEmpty(str);
    }

    static bool isBlank(const std::string* str) {
        return StringUtils::isBlank(str);
    }

    static std::string leftPad(const std::string* input, int length, char pad) {
        return StringUtils::leftPad(input, length, pad);
    }

    static std::string rightPad(const std::string* input, int length, char pad) {
        return StringUtils::rightPad(input, length, pad);
    }

    static std::string oneTab(const std::string* str) {
        return StringUtils::oneTab(str);
    }

    static std::string oneBlank(const std::string* str) {
        return StringUtils::oneBlank(str);
    }

    static int strCount(const std::string& str, const std::string& substr) {
        return StringUtils::strCount(str, substr);
    }

    static std::string hex2ASCII(const std::string& hexStr) {
        return StringUtils::hex2ASCII(hexStr);
    }

    static std::string formatCenter(const std::string* input, int totalLength, const std::string* paddingChar) {
        return StringUtils::formatCenter(input, totalLength, paddingChar);
    }

    static std::string stringToHex(const std::string* input) {
        return StringUtils::stringToHex(input);
    }

    static std::vector<std::string> stringToHexList(const std::string* input) {
        return StringUtils::stringToHexList(input);
    }

    template <typename T>
    static std::unordered_set<T> setIntersection(const std::unordered_set<T>& set1, const std::unordered_set<T>& set2) {
        return CollectionUtils::setIntersection(set1, set2);
    }

    template <typename T>
    static std::unordered_set<T> setUnion(const std::unordered_set<T>& set1, const std::unordered_set<T>& set2) {
        return CollectionUtils::setUnion(set1, set2);
    }

    template <typename T>
    static std::unordered_set<T> setDifference(const std::unordered_set<T>& set1, const std::unordered_set<T>& set2) {
        return CollectionUtils::setDifference(set1, set2);
    }

    template <typename T>
    static std::vector<T> listIntersection(const std::vector<T>& list1, const std::vector<T>& list2) {
        return CollectionUtils::listIntersection(list1, list2);
    }

    template <typename T>
    static std::vector<T> listUnion(const std::vector<T>& list1, const std::vector<T>& list2) {
        return CollectionUtils::listUnion(list1, list2);
    }

    template <typename T>
    static std::vector<T> listDifference(const std::vector<T>& list1, const std::vector<T>& list2) {
        return CollectionUtils::listDifference(list1, list2);
    }

    template <typename T>
    static void prtList(const std::vector<T>* elements) {
        CollectionUtils::prtList(elements);
    }

    static int getIMapMaxIndex(const std::map<int, std::string>& map) {
        return CollectionUtils::getIMapMaxIndex(map);
    }

    template <typename K, typename V>
    static K getKeyByValue(const std::map<K, V>& map, const V& value) {
        return CollectionUtils::getKeyByValue(map, value);
    }

    static std::string listToString(const std::vector<std::string>* lines) {
        return CollectionUtils::listToString(lines);
    }

    static std::string listToString(const std::vector<std::string>* lines, const std::string* separator) {
        return CollectionUtils::listToString(lines, separator);
    }

    static std::vector<std::string> readFile(const std::string& fileName, bool trim, const std::string& charSet = "UTF-8") {
        return FileUtils::readFile(fileName, trim, charSet);
    }

    static std::vector<std::string> readFile(const std::string& fileName, bool trim, const std::string& charSet, const std::vector<std::string>& skip) {
        return FileUtils::readFile(fileName, trim, charSet, skip);
    }

    static void copyFileNIO(const std::filesystem::path& source, const std::filesystem::path& dest) {
        FileUtils::copyFileNIO(source, dest);
    }

    static bool mkdirs(const std::string& directoryPath) {
        return FileUtils::mkdirs(directoryPath);
    }

    static bool rmrf(const std::filesystem::path& directory, bool backupBeforeDelete = false) {
        return FileUtils::rmrf(directory, backupBeforeDelete);
    }

    static bool delFile(const std::string& delFilePath) {
        return FileUtils::delFile(delFilePath);
    }

    static bool renameFile(const std::string& oldName, const std::string& newName) {
        return FileUtils::renameFile(oldName, newName);
    }

    static void clearFile(const std::string& filePath) {
        FileUtils::clearFile(filePath);
    }

    static std::vector<std::string> getAllFiles(const std::string& directoryPath, bool recursive) {
        return FileUtils::getAllFiles(directoryPath, recursive);
    }

    static std::vector<std::string> getAllFiles(const std::string& directoryPath) {
        return FileUtils::getAllFiles(directoryPath);
    }

    static std::vector<std::string> getAllFilesWithMaxDepth(const std::string& directoryPath, bool recursive, int maxDepth, int currentDepth = 0) {
        return FileUtils::getAllFilesWithMaxDepth(directoryPath, recursive, maxDepth, currentDepth);
    }

    static std::string readFileLine(const std::string& fileName, int lineNum) {
        return FileUtils::readFileLine(fileName, lineNum);
    }

    static std::string getFileName(const std::string& filePath, bool needSuffix) {
        return FileUtils::getFileName(filePath, needSuffix);
    }

    static std::string getFileExtension(const std::string& path) {
        return FileUtils::getFileExtension(path);
    }

    static std::unordered_set<std::string> readFileToSet(bool trim, const std::string& charSet, const std::string& fileName) {
        return FileUtils::readFileToSet(trim, charSet, fileName);
    }

    static void writeToFile(const std::string& content, const std::string& filePath, bool append) {
        FileUtils::writeToFile(content, filePath, append);
    }

    static void writeToFile(const std::string& content, const std::string& filePath, bool append, const std::string& charSet) {
        FileUtils::writeToFile(content, filePath, append, charSet);
    }

    static void writeToFile(const std::vector<std::string>& content, const std::string& filePath, bool append) {
        FileUtils::writeToFile(content, filePath, append);
    }

    static void writeToFile(const std::vector<std::string>& content, const std::string& filePath, bool append, const std::string& charSet) {
        FileUtils::writeToFile(content, filePath, append, charSet);
    }

    static void writeToFile(const std::vector<std::string>& content, const std::string& filePath, bool append, const std::string& charSet, const std::string& linefeed) {
        FileUtils::writeToFile(content, filePath, append, charSet, linefeed);
    }

    static void writeToFile(const std::unordered_set<std::string>& content, const std::string& filePath, bool append) {
        FileUtils::writeToFile(content, filePath, append);
    }

    static void writeToFile(const std::unordered_set<std::string>& content, const std::string& filePath, bool append, const std::string& charSet) {
        FileUtils::writeToFile(content, filePath, append, charSet);
    }

    static void writeToFile(const std::unordered_set<std::string>& content, const std::string& filePath, bool append, const std::string& charSet, const std::string& linefeed) {
        FileUtils::writeToFile(content, filePath, append, charSet, linefeed);
    }

    static std::string base64Encode(const std::string& data) {
        return EncodingUtils::base64Encode(data);
    }

    static std::string base64Decode(const std::string& encodedData) {
        return EncodingUtils::base64Decode(encodedData);
    }

    static std::string urlEncode(const std::string& url) {
        return EncodingUtils::urlEncode(url);
    }

    static std::string urlDecode(const std::string& encodedUrl) {
        return EncodingUtils::urlDecode(encodedUrl);
    }

    static std::string hexEncode(const std::string& data) {
        return EncodingUtils::hexEncode(data);
    }

    static std::string hexDecode(const std::string& hexData) {
        return EncodingUtils::hexDecode(hexData);
    }

    static int toInt(const std::string& s, int defaultValue = 0) {
        return NumberUtils::toInt(s, defaultValue);
    }

    static double toDouble(const std::string& s, double defaultValue = 0.0) {
        return NumberUtils::toDouble(s, defaultValue);
    }

    static bool toBool(const std::string& s, bool defaultValue = false) {
        return NumberUtils::toBool(s, defaultValue);
    }

    static std::string numberToString(int value) {
        return NumberUtils::toString(value);
    }

    static std::string numberToString(double value, int decimals = -1) {
        return NumberUtils::toString(value, decimals);
    }

    static bool isNumber(const std::string& s) {
        return NumberUtils::isNumber(s);
    }

    static std::string generateUuid() {
        return RandomUtils::generateUuid();
    }

    static std::string randomString(size_t length) {
        return RandomUtils::randomString(length);
    }

    static int randomInt(int minValue, int maxValue) {
        return RandomUtils::randomInt(minValue, maxValue);
    }

    static double randomDouble(double minValue, double maxValue) {
        return RandomUtils::randomDouble(minValue, maxValue);
    }

    static void sleep(unsigned int milliseconds) {
        ProcessUtils::sleep(milliseconds);
    }

    static std::tuple<int, std::string, std::string> executeCommand(const std::string& command, int timeout = -1) {
        return ProcessUtils::executeCommand(command, timeout);
    }

    static bool openFile(const std::string& filePath) {
        return ProcessUtils::openFile(filePath);
    }

    static bool openUrl(const std::string& url) {
        return ProcessUtils::openUrl(url);
    }

    static int getProcessId() {
        return ProcessUtils::getProcessId();
    }

    static std::string getEnvironmentVariable(const std::string& name, const std::string& defaultValue = "") {
        return ProcessUtils::getEnvironmentVariable(name, defaultValue);
    }

    static std::string getDateTime() {
        return DateTimeUtils::getDateTime();
    }

    static std::string getDate() {
        return DateTimeUtils::getDate();
    }

    static std::string getTime() {
        return DateTimeUtils::getTime();
    }

    static time_t getTimestamp() {
        return DateTimeUtils::getTimestamp();
    }

    static long long getTimestampMillis() {
        return DateTimeUtils::getTimestampMillis();
    }

    static std::string formatTimestamp(time_t timestamp, const std::string& format = "%Y-%m-%d %H:%M:%S") {
        return DateTimeUtils::formatTimestamp(timestamp, format);
    }

    static double calculateTimeDifference(time_t start, time_t end) {
        return DateTimeUtils::calculateTimeDifference(start, end);
    }

    static time_t parseDateTime(const std::string& datetimeStr, const std::string& format = "%Y-%m-%d %H:%M:%S") {
        return DateTimeUtils::parseDateTime(datetimeStr, format);
    }

    static time_t addDays(time_t timestamp, int days) {
        return DateTimeUtils::addDays(timestamp, days);
    }

    static time_t addHours(time_t timestamp, int hours) {
        return DateTimeUtils::addHours(timestamp, hours);
    }

    static int diffDays(time_t start, time_t end) {
        return DateTimeUtils::diffDays(start, end);
    }

    static bool isLeapYear(int year) {
        return DateTimeUtils::isLeapYear(year);
    }

    // ========== 字符串扩展 ==========

    static std::vector<std::string> split(const std::string& str, const std::string& delimiter) {
        return StringUtils::split(str, delimiter);
    }

    static std::string join(const std::vector<std::string>& list, const std::string& delimiter) {
        return StringUtils::join(list, delimiter);
    }

    static std::string trim(const std::string& str) {
        return StringUtils::trim(str);
    }

    static std::string ltrim(const std::string& str) {
        return StringUtils::ltrim(str);
    }

    static std::string rtrim(const std::string& str) {
        return StringUtils::rtrim(str);
    }

    static std::string replace(const std::string& str, const std::string& oldStr, const std::string& newStr) {
        return StringUtils::replace(str, oldStr, newStr);
    }

    static std::string replaceFirst(const std::string& str, const std::string& oldStr, const std::string& newStr) {
        return StringUtils::replaceFirst(str, oldStr, newStr);
    }

    static std::string toUpperCase(const std::string& str) {
        return StringUtils::toUpperCase(str);
    }

    static std::string toLowerCase(const std::string& str) {
        return StringUtils::toLowerCase(str);
    }

    static std::string capitalize(const std::string& str) {
        return StringUtils::capitalize(str);
    }

    static bool startsWith(const std::string& str, const std::string& prefix) {
        return StringUtils::startsWith(str, prefix);
    }

    static bool endsWith(const std::string& str, const std::string& suffix) {
        return StringUtils::endsWith(str, suffix);
    }

    static bool contains(const std::string& str, const std::string& substr) {
        return StringUtils::contains(str, substr);
    }

    static bool isAlpha(const std::string& str) {
        return StringUtils::isAlpha(str);
    }

    static bool isNumeric(const std::string& str) {
        return StringUtils::isNumeric(str);
    }

    static bool isAlphanumeric(const std::string& str) {
        return StringUtils::isAlphanumeric(str);
    }

    static bool isEmail(const std::string& str) {
        return StringUtils::isEmail(str);
    }

    // ========== 哈希工具 ==========

    static std::string md5(const std::string& data) {
        return HashUtils::md5(data);
    }

    static std::string sha256(const std::string& data) {
        return HashUtils::sha256(data);
    }

    static std::string fileMd5(const std::string& path) {
        return HashUtils::fileMd5(path);
    }

    static uint32_t crc32(const std::string& data) {
        return HashUtils::crc32(data);
    }

    // ========== 配置工具 ==========

    static std::map<std::string, std::string> loadConfig(const std::string& path) {
        return ConfigUtils::load(path);
    }

    static std::string getConfig(const std::map<std::string, std::string>& config, const std::string& key, const std::string& defaultValue = "") {
        return ConfigUtils::get(config, key, defaultValue);
    }

    static void setConfig(std::map<std::string, std::string>& config, const std::string& key, const std::string& value) {
        ConfigUtils::set(config, key, value);
    }

    static bool saveConfig(const std::string& path, const std::map<std::string, std::string>& config) {
        return ConfigUtils::save(path, config);
    }
};

}

#endif