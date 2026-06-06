/**
 * @file file_utils.h
 * @brief 文件处理工具
 *
 * 提供文件读写、目录操作、路径处理等功能。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_UTILS_FILE_UTILS_H
#define CU_UTILS_FILE_UTILS_H

#include "cu_utils/export.h"

#include <string>
#include <vector>
#include <unordered_set>
#include <filesystem>

namespace cu {

/**
 * @brief 文件处理工具类
 *
 * 提供静态方法进行文件的各种操作，包括读写、复制、删除等。
 */
class CXXU_API FileUtils {
public:
    /** @brief 禁用默认构造函数 */
    FileUtils() = delete;
    /** @brief 禁用拷贝构造函数 */
    FileUtils(const FileUtils&) = delete;
    /** @brief 禁用赋值运算符 */
    FileUtils& operator=(const FileUtils&) = delete;

    /**
     * @brief 按行读取文件
     *
     * 读取文件所有行到向量中，可选择是否去除首尾空白。
     *
     * @param fileName 文件名
     * @param trim 是否去除每行首尾空白
     * @param charSet 字符编码，默认为 UTF-8
     * @return 包含文件所有行的字符串向量
     */
    static std::vector<std::string> readFile(const std::string& fileName, bool trim, const std::string& charSet = "UTF-8");

    /**
     * @brief 按行读取文件并跳过指定行
     *
     * 读取文件时跳过以 skip 列表中任一字符串开头的行。
     *
     * @param fileName 文件名
     * @param trim 是否去除每行首尾空白
     * @param charSet 字符编码
     * @param skip 需要跳过的前缀列表
     * @return 包含文件所有行的字符串向量
     */
    static std::vector<std::string> readFile(const std::string& fileName, bool trim, const std::string& charSet, const std::vector<std::string>& skip);

    /**
     * @brief 使用文件系统复制文件
     *
     * 使用 C++17 filesystem 复制文件，目标已存在则覆盖。
     *
     * @param source 源文件路径
     * @param dest 目标文件路径
     */
    static void copyFileNIO(const std::filesystem::path& source, const std::filesystem::path& dest);

    /**
     * @brief 创建目录（含所有必要父目录）
     *
     * 创建指定目录及其所有父目录。目录已存在时返回 true。
     *
     * @param directoryPath 目录路径
     * @return 创建成功返回 true
     */
    static bool mkdirs(const std::string& directoryPath);

    /**
     * @brief 递归删除目录
     *
     * 删除目录及其所有内容和子目录。
     *
     * @param directory 要删除的目录路径
     * @param backupBeforeDelete 是否在删除前重命名备份
     * @return 删除成功返回 true
     */
    static bool rmrf(const std::filesystem::path& directory, bool backupBeforeDelete = false);

    /**
     * @brief 递归删除目录内容（内部函数）
     * @param path 要删除的路径
     */
    static void deleteRecursively(const std::filesystem::path& path);

    /**
     * @brief 删除单个文件
     *
     * @param delFilePath 文件路径
     * @return 删除成功返回 true
     */
    static bool delFile(const std::string& delFilePath);

    /**
     * @brief 重命名或移动文件/目录
     *
     * @param oldName 旧名称/路径
     * @param newName 新名称/路径
     * @return 重命名成功返回 true
     */
    static bool renameFile(const std::string& oldName, const std::string& newName);

    /**
     * @brief 清空文件内容
     *
     * 向文件写入空字符串。文件不存在会先创建。
     *
     * @param filePath 文件路径
     */
    static void clearFile(const std::string& filePath);

    /**
     * @brief 获取目录下所有文件
     *
     * @param directoryPath 目录路径
     * @param recursive 是否递归遍历子目录
     * @return 包含所有文件路径的字符串向量
     */
    static std::vector<std::string> getAllFiles(const std::string& directoryPath, bool recursive);

    /**
     * @brief 获取目录下所有文件（递归）
     *
     * @param directoryPath 目录路径
     * @return 包含所有文件路径的字符串向量
     */
    static std::vector<std::string> getAllFiles(const std::string& directoryPath);

    /**
     * @brief 获取目录下所有文件（限制递归深度）
     *
     * @param directoryPath 目录路径
     * @param recursive 是否递归
     * @param maxDepth 最大递归深度
     * @param currentDepth 当前递归深度
     * @return 包含所有文件路径的字符串向量
     */
    static std::vector<std::string> getAllFilesWithMaxDepth(const std::string& directoryPath, bool recursive, int maxDepth, int currentDepth = 0);

    /**
     * @brief 按行号读取一行内容
     *
     * @param fileName 文件名
     * @param lineNum 行号（从1开始）
     * @return 指定行的内容，文件不存在或行号无效返回空字符串
     */
    static std::string readFileLine(const std::string& fileName, int lineNum);

    /**
     * @brief 从路径中取出文件名
     *
     * @param filePath 文件路径
     * @param needSuffix 是否需要后缀
     * @return 文件名
     */
    static std::string getFileName(const std::string& filePath, bool needSuffix);

    /**
     * @brief 获取文件扩展名
     *
     * @param path 文件路径
     * @return 文件扩展名（不含点），如 "txt"
     */
    static std::string getFileExtension(const std::string& path);

    /**
     * @brief 将文件按行读入集合
     *
     * @param trim 是否去除首尾空白
     * @param charSet 字符编码
     * @param fileName 文件名
     * @return 包含文件所有行的无序集合
     */
    static std::unordered_set<std::string> readFileToSet(bool trim, const std::string& charSet, const std::string& fileName);

    /**
     * @brief 写入字符串到文件
     *
     * @param content 要写入的内容
     * @param filePath 文件路径
     * @param append 是否追加模式
     */
    static void writeToFile(const std::string& content, const std::string& filePath, bool append);

    /**
     * @brief 写入字符串到文件（指定编码）
     *
     * @param content 要写入的内容
     * @param filePath 文件路径
     * @param append 是否追加模式
     * @param charSet 字符编码
     */
    static void writeToFile(const std::string& content, const std::string& filePath, bool append, const std::string& charSet);

    /**
     * @brief 写入多行到文件
     *
     * @param content 要写入的行列表
     * @param filePath 文件路径
     * @param append 是否追加模式
     */
    static void writeToFile(const std::vector<std::string>& content, const std::string& filePath, bool append);

    /**
     * @brief 写入多行到文件（指定编码）
     *
     * @param content 要写入的行列表
     * @param filePath 文件路径
     * @param append 是否追加模式
     * @param charSet 字符编码
     */
    static void writeToFile(const std::vector<std::string>& content, const std::string& filePath, bool append, const std::string& charSet);

    /**
     * @brief 写入多行到文件（指定编码和换行符）
     *
     * @param content 要写入的行列表
     * @param filePath 文件路径
     * @param append 是否追加模式
     * @param charSet 字符编码
     * @param linefeed 换行符
     */
    static void writeToFile(const std::vector<std::string>& content, const std::string& filePath, bool append, const std::string& charSet, const std::string& linefeed);

    /**
     * @brief 写入集合到文件
     *
     * @param content 要写入的内容集合
     * @param filePath 文件路径
     * @param append 是否追加模式
     */
    static void writeToFile(const std::unordered_set<std::string>& content, const std::string& filePath, bool append);

    /**
     * @brief 写入集合到文件（指定编码）
     *
     * @param content 要写入的内容集合
     * @param filePath 文件路径
     * @param append 是否追加模式
     * @param charSet 字符编码
     */
    static void writeToFile(const std::unordered_set<std::string>& content, const std::string& filePath, bool append, const std::string& charSet);

    /**
     * @brief 写入集合到文件（指定编码和换行符）
     *
     * @param content 要写入的内容集合
     * @param filePath 文件路径
     * @param append 是否追加模式
     * @param charSet 字符编码
     * @param linefeed 换行符
     */
    static void writeToFile(const std::unordered_set<std::string>& content, const std::string& filePath, bool append, const std::string& charSet, const std::string& linefeed);

    static std::string toLinefeed(const std::string& linefeed);
};

}

#endif