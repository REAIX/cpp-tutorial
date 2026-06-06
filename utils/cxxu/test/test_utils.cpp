/**
 * @file test_utils.cpp
 * @brief C++ Utils 测试程序
 *
 * 测试字符串工具、集合工具、文件工具、编码工具、
 * 数值工具、随机工具、进程工具、日期时间工具等功能。
 *
 * @author CU Utils Project
 * @version 2.0
 */

#include <string>
#include "cu_utils/utils.h"
#include <iostream>
#include <vector>
#include <unordered_set>
#include <map>
#include <cmath>
#include <algorithm>

// ============================================================
// 简单测试框架
// ============================================================

static int tests_passed = 0;
static int tests_failed = 0;
static int tests_total = 0;

#define TEST_ASSERT(condition, message)                                     \
    do {                                                                    \
        tests_total++;                                                      \
        if (condition) {                                                    \
            tests_passed++;                                                 \
            std::cout << "  [PASS] " << (message) << std::endl;            \
        } else {                                                            \
            tests_failed++;                                                 \
            std::cout << "  [FAIL] " << (message)                           \
                      << " (at " << __FILE__ << ":" << __LINE__ << ")"      \
                      << std::endl;                                         \
        }                                                                   \
    } while (0)

template <typename T>
void TEST_ASSERT_EQ(const T& expected, const T& actual, const std::string& message) {
    tests_total++;
    if (expected == actual) {
        tests_passed++;
        std::cout << "  [PASS] " << message << std::endl;
    } else {
        tests_failed++;
        std::cout << "  [FAIL] " << message
                  << " (expected=" << expected << ", actual=" << actual
                  << ", at " << __FILE__ << ":" << __LINE__ << ")"
                  << std::endl;
    }
}

inline void TEST_ASSERT_STR_EQ(const std::string& expected, const std::string& actual, const std::string& message) {
    tests_total++;
    if (expected == actual) {
        tests_passed++;
        std::cout << "  [PASS] " << message << std::endl;
    } else {
        tests_failed++;
        std::cout << "  [FAIL] " << message
                  << " (expected=\"" << expected << "\", actual=\"" << actual
                  << "\", at " << __FILE__ << ":" << __LINE__ << ")"
                  << std::endl;
    }
}

void test_summary() {
    std::cout << "\n=== Test Summary ===" << std::endl;
    std::cout << "Total:  " << tests_total << std::endl;
    std::cout << "Passed: " << tests_passed << std::endl;
    std::cout << "Failed: " << tests_failed << std::endl;
    if (tests_total > 0) {
        double rate = 100.0 * tests_passed / tests_total;
        std::cout << "Pass Rate: " << rate << "%" << std::endl;
    }
    std::cout << "===================" << std::endl;
}

// ============================================================
// 测试用例
// ============================================================

void test_string_utils() {
    std::cout << "\n1. Testing String Utils:" << std::endl;

    std::string testStr = "Hello World! This is a test.";
    std::string shortStr = "Hi";

    // subStr
    std::string subResult = cu::Utils::subStr(testStr, "Hello ", "!");
    TEST_ASSERT_STR_EQ("World", subResult, "subStr(\"Hello World! This is a test.\", \"Hello \", \"!\") == \"World\"");

    // isEmpty - 非空字符串应返回 false
    TEST_ASSERT(!cu::Utils::isEmpty(&testStr), "isEmpty(non-empty) == false");

    // isBlank - 非空字符串应返回 false
    TEST_ASSERT(!cu::Utils::isBlank(&testStr), "isBlank(non-empty) == false");

    // leftPad - 结果长度应为 10
    std::string padded = cu::Utils::leftPad(&shortStr, 10, '*');
    TEST_ASSERT_EQ(static_cast<std::string::size_type>(10), padded.length(), "leftPad result length == 10");

    // rightPad
    std::string rPadded = cu::Utils::rightPad(&shortStr, 10, '*');
    TEST_ASSERT_EQ(static_cast<std::string::size_type>(10), rPadded.length(), "rightPad result length == 10");

    // hex2ASCII
    std::string asciiResult = cu::Utils::hex2ASCII("48656C6C6F");
    TEST_ASSERT_STR_EQ("Hello", asciiResult, "hex2ASCII(\"48656C6C6F\") == \"Hello\"");

    // stringToHex - round trip
    std::string hexResult = cu::Utils::stringToHex(&testStr);
    TEST_ASSERT(!hexResult.empty(), "stringToHex returns non-empty string");
}

void test_collection_utils() {
    std::cout << "\n2. Testing Collection Utils:" << std::endl;

    std::unordered_set<int> set1 = {1, 2, 3, 4, 5};
    std::unordered_set<int> set2 = {3, 4, 5, 6, 7};

    // setIntersection - 结果应包含 3,4,5
    auto intersection = cu::Utils::setIntersection(set1, set2);
    TEST_ASSERT(intersection.count(3) > 0 && intersection.count(4) > 0 && intersection.count(5) > 0,
                "setIntersection contains {3,4,5}");

    // setUnion - 结果大小应为 7
    auto unionSet = cu::Utils::setUnion(set1, set2);
    TEST_ASSERT_EQ(static_cast<size_t>(7), unionSet.size(), "setUnion size == 7");

    // listToString
    std::vector<std::string> lines = {"Line 1", "Line 2", "Line 3"};
    std::string listStr = cu::Utils::listToString(&lines);
    TEST_ASSERT(!listStr.empty(), "listToString returns non-empty string");
}

void test_file_utils() {
    std::cout << "\n3. Testing File Utils:" << std::endl;

    std::vector<std::string> testContent = {"Test line 1", "Test line 2", "Test line 3"};
    std::string testFileName = "test_file.txt";

    try {
        cu::Utils::writeToFile(testContent, testFileName, false);
        std::cout << "  Created test file: " << testFileName << std::endl;

        auto readContent = cu::Utils::readFile(testFileName, false, "UTF-8");
        TEST_ASSERT_EQ(testContent.size(), readContent.size(), "readFile line count matches writeToFile");

        if (cu::Utils::delFile(testFileName)) {
            std::cout << "  Deleted test file: " << testFileName << std::endl;
        }
    } catch (const std::exception& e) {
        tests_total++;
        tests_failed++;
        std::cout << "  [FAIL] File operation error: " << e.what() << std::endl;
    }
}

void test_constants() {
    std::cout << "\n4. Testing Constants:" << std::endl;

    TEST_ASSERT_STR_EQ("UTF-8", std::string(cu::Utils::UTF8), "UTF8 constant == \"UTF-8\"");
    TEST_ASSERT_STR_EQ("\n", std::string(cu::Utils::UNIX_LINEFEED), "UNIX_LINEFEED constant == \"\\n\"");
    TEST_ASSERT_STR_EQ("\r\n", std::string(cu::Utils::WINDOWS_LINEFEED), "WINDOWS_LINEFEED constant == \"\\r\\n\"");
}

void test_encoding_utils() {
    std::cout << "\n5. Testing Encoding Utils:" << std::endl;

    std::string testData = "Hello World";

    // base64 round trip
    std::string base64 = cu::Utils::base64Encode(testData);
    std::string base64Decoded = cu::Utils::base64Decode(base64);
    TEST_ASSERT_STR_EQ(testData, base64Decoded, "base64Encode + base64Decode round trip");

    // url round trip
    std::string url = "https://example.com/?q=Test";
    std::string encodedUrl = cu::Utils::urlEncode(url);
    std::string decodedUrl = cu::Utils::urlDecode(encodedUrl);
    TEST_ASSERT_STR_EQ(url, decodedUrl, "urlEncode + urlDecode round trip");

    // hex round trip
    std::string hex = cu::Utils::hexEncode(testData);
    std::string hexDecoded = cu::Utils::hexDecode(hex);
    TEST_ASSERT_STR_EQ(testData, hexDecoded, "hexEncode + hexDecode round trip");
}

void test_number_utils() {
    std::cout << "\n6. Testing Number Utils:" << std::endl;

    // toInt
    TEST_ASSERT_EQ(123, cu::Utils::toInt("123"), "toInt(\"123\") == 123");
    TEST_ASSERT_EQ(999, cu::Utils::toInt("abc", 999), "toInt(\"abc\", 999) == 999");

    // toDouble
    double d = cu::Utils::toDouble("3.14");
    TEST_ASSERT(std::fabs(d - 3.14) < 1e-9, "toDouble(\"3.14\") ~= 3.14");

    // toBool
    TEST_ASSERT(cu::Utils::toBool("true"), "toBool(\"true\") == true");

    // isNumber
    TEST_ASSERT(cu::Utils::isNumber("456"), "isNumber(\"456\") == true");
    TEST_ASSERT(!cu::Utils::isNumber("abc"), "isNumber(\"abc\") == false");
}

void test_random_utils() {
    std::cout << "\n7. Testing Random Utils:" << std::endl;

    std::string uuid = cu::Utils::generateUuid();
    TEST_ASSERT(!uuid.empty(), "generateUuid returns non-empty string");

    std::string randStr = cu::Utils::randomString(10);
    TEST_ASSERT_EQ(static_cast<std::string::size_type>(10), randStr.length(), "randomString(10) length == 10");

    int randInt = cu::Utils::randomInt(1, 100);
    TEST_ASSERT(randInt >= 1 && randInt <= 100, "randomInt(1,100) in range [1,100]");

    double randDouble = cu::Utils::randomDouble(0.0, 1.0);
    TEST_ASSERT(randDouble >= 0.0 && randDouble <= 1.0, "randomDouble(0.0,1.0) in range [0.0,1.0]");
}

void test_process_utils() {
    std::cout << "\n8. Testing Process Utils:" << std::endl;

    int pid = cu::Utils::getProcessId();
    TEST_ASSERT(pid > 0, "getProcessId() > 0");

    std::string pathVal = cu::Utils::getEnvironmentVariable("PATH", "Not found");
    TEST_ASSERT(pathVal != "Not found", "getEnvironmentVariable(\"PATH\") found");

    auto [exitCode, stdoutStr, stderrStr] = cu::Utils::executeCommand("echo Hello World");
    TEST_ASSERT_EQ(0, exitCode, "executeCommand(\"echo Hello World\") exitCode == 0");
}

void test_datetime_utils() {
    std::cout << "\n9. Testing Date Time Utils:" << std::endl;

    std::string dateTime = cu::Utils::getDateTime();
    TEST_ASSERT(!dateTime.empty(), "getDateTime() returns non-empty string");

    std::string date = cu::Utils::getDate();
    TEST_ASSERT(!date.empty(), "getDate() returns non-empty string");

    std::string time = cu::Utils::getTime();
    TEST_ASSERT(!time.empty(), "getTime() returns non-empty string");

    time_t ts = cu::Utils::getTimestamp();
    TEST_ASSERT(ts > 0, "getTimestamp() > 0");

    time_t tsMs = cu::Utils::getTimestampMillis();
    TEST_ASSERT(tsMs > 0, "getTimestampMillis() > 0");

    std::string formatted = cu::Utils::formatTimestamp(ts, "%Y/%m/%d %H:%M:%S");
    TEST_ASSERT(!formatted.empty(), "formatTimestamp returns non-empty string");

    time_t start = ts - 3600;
    double diff = cu::Utils::calculateTimeDifference(start, ts);
    TEST_ASSERT(std::fabs(diff - 3600.0) < 1.0, "calculateTimeDifference ~= 3600 seconds");
}

// ============================================================
// 主函数
// ============================================================

int main() {
    std::cout << "=== Test C++ Utils Library ===" << std::endl;

    test_string_utils();
    test_collection_utils();
    test_file_utils();
    test_constants();
    test_encoding_utils();
    test_number_utils();
    test_random_utils();
    test_process_utils();
    test_datetime_utils();

    test_summary();
    return tests_failed > 0 ? 1 : 0;
}
