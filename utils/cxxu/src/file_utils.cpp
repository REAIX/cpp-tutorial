/**
 * @file file_utils.cpp
 * @brief 文件处理工具实现
 *
 * 实现文件读写、目录操作、路径处理等功能。
 * 支持多种字符编码的读写。
 *
 * 编码支持：
 * - Windows: 使用 Windows API (MultiByteToWideChar)
 * - iconv: 使用 iconv 库（如果可用）
 *
 * @author CU Utils Project
 * @version 1.0
 */

#include "cu_utils/file_utils.h"
#include "cu_utils/constants.h"
#include <fstream>
#include <sstream>
#include <iostream>
#include <algorithm>
#include <iterator>
#include <stdexcept>
#include <memory>
#include <locale>

#ifdef _WIN32
#include <windows.h>
#endif

// 检测 iconv 是否可用
#ifdef HAVE_ICONV
#include <iconv.h>
#define USE_ICONV 1
#else
#define USE_ICONV 0
#endif

namespace cu {

/**
 * @brief 字符编码转换器
 *
 * 支持两种实现：
 * 1. Windows API (MultiByteToWideChar)
 * 2. iconv 库
 */
class CharsetConverter {
public:
    static std::string convert(const std::string& input, const std::string& fromCharset, const std::string& toCharset) {
        if (fromCharset == toCharset || input.empty()) {
            return input;
        }

#if USE_ICONV
        return convertWithIconv(input, fromCharset, toCharset);
#elif defined(_WIN32)
        return convertWithWindowsAPI(input, fromCharset, toCharset);
#else
        return input;
#endif
    }

private:
#if USE_ICONV
    /**
     * @brief 使用 iconv 进行编码转换
     */
    static std::string convertWithIconv(const std::string& input, const std::string& fromCharset, const std::string& toCharset) {
        iconv_t cd = iconv_open(toCharset.c_str(), fromCharset.c_str());
        if (cd == (iconv_t)-1) {
            return input;
        }

        char* inbuf = const_cast<char*>(input.c_str());
        size_t inbytesleft = input.size();

        // 预分配输出缓冲区（最大可能是输入的4倍）
        size_t outbytesleft = input.size() * 4;
        std::string output(outbytesleft, '\0');
        char* outbuf = &output[0];

        size_t result = iconv(cd, &inbuf, &inbytesleft, &outbuf, &outbytesleft);
        iconv_close(cd);

        if (result == (size_t)-1) {
            return input;
        }

        output.resize(output.size() - outbytesleft);
        return output;
    }
#endif

#if defined(_WIN32)
    /**
     * @brief 使用 Windows API 进行编码转换
     */
    static std::string convertWithWindowsAPI(const std::string& input, const std::string& fromCharset, const std::string& toCharset) {
        int fromCode = charsetToCodePage(fromCharset);
        int toCode = charsetToCodePage(toCharset);

        if (fromCode == 0 || toCode == 0) {
            return input;
        }

        // 转换为宽字符
        int len = MultiByteToWideChar(fromCode, 0, input.c_str(), -1, NULL, 0);
        if (len == 0) return input;

        std::wstring wstr(len, L'\0');
        MultiByteToWideChar(fromCode, 0, input.c_str(), -1, &wstr[0], len);
        wstr.resize(len - 1);

        // 转换为目标编码
        int outLen = WideCharToMultiByte(toCode, 0, wstr.c_str(), -1, NULL, 0, NULL, NULL);
        if (outLen == 0) return input;

        std::string result(outLen, '\0');
        WideCharToMultiByte(toCode, 0, wstr.c_str(), -1, &result[0], outLen, NULL, NULL);
        result.resize(outLen - 1);

        return result;
    }

    /**
     * @brief 将字符集名称转换为 Windows 代码页编号
     */
public:
    static int charsetToCodePage(const std::string& charset) {
        std::string upper = charset;
        std::transform(upper.begin(), upper.end(), upper.begin(),
            [](unsigned char c) { return std::toupper(c); });

        if (upper == "UTF-8" || upper == "UTF8") return CP_UTF8;
        if (upper == "UTF-16" || upper == "UTF16") return 1200;
        if (upper == "UTF-16LE" || upper == "UTF16LE") return 1200;
        if (upper == "UTF-16BE" || upper == "UTF16BE") return 1201;
        if (upper == "GBK") return 936;
        if (upper == "GB2312") return 936;
        if (upper == "GB18030") return 54936;
        if (upper == "BIG5") return 950;
        if (upper == "ISO-8859-1" || upper == "LATIN1") return 28591;
        if (upper == "WINDOWS-1252") return 1252;
        if (upper == "US-ASCII" || upper == "ASCII") return 20127;

        return 0;
    }
#endif
};

/**
 * @brief 使用指定编码读取整个文件内容
 * @param filePath 文件路径
 * @param charSet 字符编码
 * @return 文件内容
 */
static std::string readFileContent(const std::string& filePath, const std::string& charSet) {
#if USE_ICONV
    // 使用 iconv 读取
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // 转换为 UTF-8
    return CharsetConverter::convert(content, charSet, "UTF-8");

#elif defined(_WIN32)
    // 使用 Windows API
    int codePage = CharsetConverter::charsetToCodePage(charSet);
    if (codePage == 0) codePage = CP_UTF8;

    HANDLE hFile = CreateFileA(filePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL,
                                OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE) {
        return "";
    }

    LARGE_INTEGER fileSize;
    if (!GetFileSizeEx(hFile, &fileSize) || fileSize.QuadPart == 0) {
        CloseHandle(hFile);
        return "";
    }

    if (fileSize.QuadPart > static_cast<LONGLONG>(256 * 1024 * 1024)) {
        CloseHandle(hFile);
        return "";
    }

    std::vector<char> buffer(static_cast<size_t>(fileSize.QuadPart) + 1);
    DWORD bytesRead = 0;
    BOOL readResult = ReadFile(hFile, buffer.data(), static_cast<DWORD>(fileSize.QuadPart), &bytesRead, NULL);
    CloseHandle(hFile);

    if (!readResult || bytesRead == 0) {
        return "";
    }

    buffer[bytesRead] = '\0';

    // 转换为 UTF-8
    std::string utf8Content = CharsetConverter::convert(buffer.data(), charSet, "UTF-8");
    return utf8Content;

#else
    // 回退到标准方法
    std::ifstream file(filePath, std::ios::binary);
    if (!file.is_open()) {
        return "";
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
#endif
}

/**
 * @brief 标准化字符集名称
 */
static std::string normalizeCharset(const std::string& charSet) {
    if (charSet.empty() || charSet == "UTF-8" || charSet == "UTF8") {
        return "UTF-8";
    }
    return charSet;
}

/**
 * @brief 按行读取文件
 *
 * 使用指定字符编码读取文件。
 *
 * @param fileName 文件名
 * @param trim 是否去除每行首尾空白
 * @param charSet 字符编码（如 "UTF-8", "GBK", "GB2312" 等）
 * @return 包含文件所有行的字符串向量
 */
std::vector<std::string> FileUtils::readFile(const std::string& fileName, bool trim, const std::string& charSet) {
    std::vector<std::string> lines;

    try {
        std::string content = readFileContent(fileName, normalizeCharset(charSet));
        if (content.empty()) {
            return lines;
        }

        std::istringstream stream(content);
        std::string line;
        while (std::getline(stream, line)) {
            if (trim) {
                auto start = line.find_first_not_of(" \t\r\n");
                auto end = line.find_last_not_of(" \t\r\n");
                if (start != std::string::npos && end != std::string::npos) {
                    line = line.substr(start, end - start + 1);
                } else {
                    line.clear();
                }
            }
            lines.push_back(line);
        }
    } catch (const std::exception& e) {
        std::cerr << "Error reading file: " << e.what() << std::endl;
    }

    return lines;
}

/**
 * @brief 按行读取文件并跳过指定行
 * @param fileName 文件名
 * @param trim 是否去除每行首尾空白
 * @param charSet 字符编码
 * @param skip 需要跳过的前缀列表
 * @return 包含文件所有行的字符串向量
 */
std::vector<std::string> FileUtils::readFile(const std::string& fileName, bool trim, const std::string& charSet, const std::vector<std::string>& skip) {
    std::vector<std::string> lines = FileUtils::readFile(fileName, trim, charSet);

    if (skip.empty()) {
        return lines;
    }

    std::vector<std::string> result;
    for (const auto& line : lines) {
        bool shouldSkip = false;
        for (const auto& prefix : skip) {
            if (line.find(prefix) == 0) {
                shouldSkip = true;
                break;
            }
        }
        if (!shouldSkip) {
            result.push_back(line);
        }
    }

    return result;
}

/**
 * @brief 使用文件系统复制文件
 * @param source 源文件路径
 * @param dest 目标文件路径
 */
void FileUtils::copyFileNIO(const std::filesystem::path& source, const std::filesystem::path& dest) {
    std::filesystem::copy(source, dest, std::filesystem::copy_options::overwrite_existing);
}

/**
 * @brief 创建目录（含所有必要父目录）
 * @param directoryPath 目录路径
 * @return 创建成功返回 true
 */
bool FileUtils::mkdirs(const std::string& directoryPath) {
    if (directoryPath.empty()) {
        return false;
    }

    return std::filesystem::create_directories(directoryPath);
}

/**
 * @brief 递归删除目录内容
 * @param path 要删除的路径
 */
void FileUtils::deleteRecursively(const std::filesystem::path& path) {
    if (!std::filesystem::exists(path)) {
        return;
    }

    for (auto& entry : std::filesystem::directory_iterator(path)) {
        if (entry.is_symlink()) {
            std::filesystem::remove(entry.path());
        } else if (entry.is_directory()) {
            FileUtils::deleteRecursively(entry.path());
        } else {
            std::filesystem::remove(entry.path());
        }
    }

    std::filesystem::remove(path);
}

/**
 * @brief 递归删除目录
 * @param directory 要删除的目录路径
 * @param backupBeforeDelete 是否在删除前重命名备份
 * @return 删除成功返回 true
 */
bool FileUtils::rmrf(const std::filesystem::path& directory, bool backupBeforeDelete) {
    if (!std::filesystem::exists(directory) || !std::filesystem::is_directory(directory)) {
        return false;
    }

    try {
        if (backupBeforeDelete) {
            std::filesystem::path renamed = directory.parent_path() / (directory.filename().string() + "_del");
            if (std::filesystem::exists(renamed)) {
                FileUtils::deleteRecursively(renamed);
            }
            std::filesystem::rename(directory, renamed);
            return true;
        }

        FileUtils::deleteRecursively(directory);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error in rmrf: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 删除单个文件
 * @param delFilePath 文件路径
 * @return 删除成功返回 true
 */
bool FileUtils::delFile(const std::string& delFilePath) {
    if (delFilePath.empty()) {
        return false;
    }

    return std::filesystem::remove(delFilePath);
}

/**
 * @brief 重命名或移动文件/目录
 * @param oldName 旧名称/路径
 * @param newName 新名称/路径
 * @return 重命名成功返回 true
 */
bool FileUtils::renameFile(const std::string& oldName, const std::string& newName) {
    if (oldName.empty() || newName.empty()) {
        return false;
    }

    try {
        std::filesystem::rename(oldName, newName);
        return true;
    } catch (const std::exception& e) {
        std::cerr << "Error renaming file: " << e.what() << std::endl;
        return false;
    }
}

/**
 * @brief 清空文件内容
 * @param filePath 文件路径
 */
void FileUtils::clearFile(const std::string& filePath) {
    writeToFile("", filePath, false);
}

/**
 * @brief 获取目录下所有文件
 * @param directoryPath 目录路径
 * @param recursive 是否递归遍历子目录
 * @return 包含所有文件路径的字符串向量
 */
std::vector<std::string> FileUtils::getAllFiles(const std::string& directoryPath, bool recursive) {
    std::vector<std::string> files;

    if (directoryPath.empty()) {
        throw std::invalid_argument("directoryPath must not be null or empty");
    }

    try {
        std::filesystem::path dirPath(directoryPath);
        std::filesystem::directory_options options = std::filesystem::directory_options::skip_permission_denied;

        if (recursive) {
            for (auto& entry : std::filesystem::recursive_directory_iterator(dirPath, options)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().string());
                }
            }
        } else {
            for (auto& entry : std::filesystem::directory_iterator(dirPath, options)) {
                if (entry.is_regular_file()) {
                    files.push_back(entry.path().string());
                }
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in getAllFiles: " << e.what() << std::endl;
        throw;
    }

    return files;
}

/**
 * @brief 获取目录下所有文件（限制递归深度）
 * @param directoryPath 目录路径
 * @param recursive 是否递归
 * @param maxDepth 最大递归深度
 * @param currentDepth 当前递归深度
 * @return 包含所有文件路径的字符串向量
 */
std::vector<std::string> FileUtils::getAllFilesWithMaxDepth(const std::string& directoryPath, bool recursive,
                                                           int maxDepth, int currentDepth) {
    if (directoryPath.empty()) {
        throw std::invalid_argument("directoryPath must not be null or empty");
    }

    if (maxDepth <= 0 || currentDepth < -1) {
        throw std::invalid_argument("invalid maxDepth or currentDepth");
    }

    int level = (currentDepth == -1) ? 1 : currentDepth + 1;
    if (level > maxDepth) {
        return std::vector<std::string>();
    }

    std::vector<std::string> files;
    try {
        std::filesystem::path dirPath(directoryPath);
        std::filesystem::directory_options options = std::filesystem::directory_options::skip_permission_denied;

        for (auto& entry : std::filesystem::directory_iterator(dirPath, options)) {
            if (entry.is_regular_file()) {
                files.push_back(entry.path().string());
            } else if (entry.is_directory() && recursive) {
                auto subFiles = getAllFilesWithMaxDepth(entry.path().string(), recursive, maxDepth, level);
                files.insert(files.end(), subFiles.begin(), subFiles.end());
            }
        }
    } catch (const std::exception& e) {
        std::cerr << "Error in getAllFilesWithMaxDepth: " << e.what() << std::endl;
        throw;
    }

    return files;
}

/**
 * @brief 获取目录下所有文件（递归）
 * @param directoryPath 目录路径
 * @return 包含所有文件路径的字符串向量
 */
std::vector<std::string> FileUtils::getAllFiles(const std::string& directoryPath) {
    return getAllFiles(directoryPath, true);
}

/**
 * @brief 按行号读取一行内容
 * @param fileName 文件名
 * @param lineNum 行号（从1开始）
 * @return 指定行的内容，文件不存在或行号无效返回空字符串
 */
std::string FileUtils::readFileLine(const std::string& fileName, int lineNum) {
    auto lines = FileUtils::readFile(fileName, false, constants::CharSets::UTF8);
    if (lineNum > 0 && static_cast<size_t>(lineNum) <= lines.size()) {
        return lines[lineNum - 1];
    }
    return "";
}

/**
 * @brief 从路径中取出文件名
 * @param filePath 文件路径
 * @param needSuffix 是否需要后缀
 * @return 文件名
 */
std::string FileUtils::getFileName(const std::string& filePath, bool needSuffix) {
    if (filePath.empty()) {
        return "";
    }

    std::filesystem::path path(filePath);
    std::string name = path.filename().string();

    if (!needSuffix) {
        size_t dotPos = name.find_last_of('.');
        if (dotPos != std::string::npos) {
            return name.substr(0, dotPos);
        }
    }

    return name;
}

/**
 * @brief 获取文件扩展名
 * @param path 文件路径
 * @return 文件扩展名（不含点），如 "txt"
 */
std::string FileUtils::getFileExtension(const std::string& path) {
    if (path.empty()) {
        return "";
    }

    std::filesystem::path filePath(path);
    std::string extension = filePath.extension().string();
    if (!extension.empty() && extension[0] == '.') {
        return extension.substr(1);
    }
    return extension;
}

/**
 * @brief 将文件按行读入集合
 * @param trim 是否去除首尾空白
 * @param charSet 字符编码
 * @param fileName 文件名
 * @return 包含文件所有行的无序集合
 */
std::unordered_set<std::string> FileUtils::readFileToSet(bool trim, const std::string& charSet, const std::string& fileName) {
    auto lines = FileUtils::readFile(fileName, trim, charSet);
    return std::unordered_set<std::string>(lines.begin(), lines.end());
}

/**
 * @brief 写入字符串到文件
 * @param content 要写入的内容
 * @param filePath 文件路径
 * @param append 是否追加模式
 */
void FileUtils::writeToFile(const std::string& content, const std::string& filePath, bool append) {
    writeToFile(content, filePath, append, constants::CharSets::UTF8);
}

/**
 * @brief 写入字符串到文件（指定编码）
 * @param content 要写入的内容
 * @param filePath 文件路径
 * @param append 是否追加模式
 * @param charSet 字符编码
 */
void FileUtils::writeToFile(const std::string& content, const std::string& filePath, bool append, const std::string& charSet) {
    std::string targetCharset = normalizeCharset(charSet);

    std::string contentToWrite = content;
    if (targetCharset != "UTF-8") {
        contentToWrite = CharsetConverter::convert(content, "UTF-8", targetCharset);
    }

    std::filesystem::path fp(filePath);
    if (fp.parent_path().empty() == false) {
        std::filesystem::create_directories(fp.parent_path());
    }

    std::ofstream ofs(filePath, (append ? std::ios::app : std::ios::trunc) | std::ios::binary);
    if (!ofs.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + filePath);
    }
    ofs << contentToWrite;
    ofs.close();
}

/**
 * @brief 写入多行到文件（指定编码和换行符）
 * @param content 要写入的行列表
 * @param filePath 文件路径
 * @param append 是否追加模式
 * @param charSet 字符编码
 * @param linefeed 换行符
 */
void FileUtils::writeToFile(const std::vector<std::string>& content, const std::string& filePath, bool append,
                              const std::string& charSet, const std::string& linefeed) {
    std::string lf = FileUtils::toLinefeed(linefeed);
    std::stringstream ss;

    for (const std::string& str : content) {
        ss << str << lf;
    }

    writeToFile(ss.str(), filePath, append, charSet);
}

/**
 * @brief 写入多行到文件（指定编码）
 * @param content 要写入的行列表
 * @param filePath 文件路径
 * @param append 是否追加模式
 * @param charSet 字符编码
 */
void FileUtils::writeToFile(const std::vector<std::string>& content, const std::string& filePath, bool append, const std::string& charSet) {
    writeToFile(content, filePath, append, charSet, "");
}

/**
 * @brief 写入多行到文件
 * @param content 要写入的行列表
 * @param filePath 文件路径
 * @param append 是否追加模式
 */
void FileUtils::writeToFile(const std::vector<std::string>& content, const std::string& filePath, bool append) {
    writeToFile(content, filePath, append, constants::CharSets::UTF8, "");
}

/**
 * @brief 写入集合到文件（指定编码和换行符）
 * @param content 要写入的内容集合
 * @param filePath 文件路径
 * @param append 是否追加模式
 * @param charSet 字符编码
 * @param linefeed 换行符
 */
void FileUtils::writeToFile(const std::unordered_set<std::string>& content, const std::string& filePath, bool append,
                              const std::string& charSet, const std::string& linefeed) {
    std::string lf = FileUtils::toLinefeed(linefeed);
    std::stringstream ss;

    for (const std::string& s : content) {
        ss << s << lf;
    }

    writeToFile(ss.str(), filePath, append, charSet);
}

/**
 * @brief 写入集合到文件（指定编码）
 * @param content 要写入的内容集合
 * @param filePath 文件路径
 * @param append 是否追加模式
 * @param charSet 字符编码
 */
void FileUtils::writeToFile(const std::unordered_set<std::string>& content, const std::string& filePath, bool append, const std::string& charSet) {
    writeToFile(content, filePath, append, charSet, "");
}

/**
 * @brief 写入集合到文件
 * @param content 要写入的内容集合
 * @param filePath 文件路径
 * @param append 是否追加模式
 */
void FileUtils::writeToFile(const std::unordered_set<std::string>& content, const std::string& filePath, bool append) {
    writeToFile(content, filePath, append, constants::CharSets::UTF8, "");
}

/**
 * @brief 标准化换行符
 * @param linefeed 换行符
 * @return 标准化后的换行符
 */
std::string FileUtils::toLinefeed(const std::string& linefeed) {
    if (!linefeed.empty() &&
        (linefeed == constants::Const::UNIX_LINEFEED ||
         linefeed == constants::Const::WINDOWS_LINEFEED)) {
        return linefeed;
    }
    return constants::Const::UNIX_LINEFEED;
}

}