/**
 * @file process_utils.h
 * @brief 进程工具
 *
 * 提供延时、命令执行、进程信息获取等功能，支持跨平台。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#ifndef CU_UTILS_PROCESS_UTILS_H
#define CU_UTILS_PROCESS_UTILS_H

#include "cu_utils/export.h"

#include <string>
#include <tuple>

namespace cu {

/**
 * @brief 进程工具类
 *
 * 提供静态方法进行进程相关操作，支持 Windows 和 Linux/macOS。
 */
class CXXU_API ProcessUtils {
public:
    /** @brief 禁用默认构造函数 */
    ProcessUtils() = delete;
    /** @brief 禁用拷贝构造函数 */
    ProcessUtils(const ProcessUtils&) = delete;
    /** @brief 禁用赋值运算符 */
    ProcessUtils& operator=(const ProcessUtils&) = delete;

    /**
     * @brief 阻塞式延时
     *
     * 使当前线程阻塞指定的毫秒数。
     *
     * @param milliseconds 延时的毫秒数
     */
    static void sleep(unsigned int milliseconds);

    /**
     * @brief 执行系统命令
     *
     * 执行指定的系统命令并捕获输出。
     *
     * @param command 要执行的命令字符串
     * @param timeout 超时时间（秒），-1 表示无超时
     * @return 元组(退出码, 标准输出, 标准错误)
     */
    static std::tuple<int, std::string, std::string> executeCommand(const std::string& command, int timeout = -1);

    /**
     * @brief 打开文件
     *
     * 使用系统默认程序打开文件。
     *
     * @param filePath 文件路径
     * @return 打开成功返回 true
     */
    static bool openFile(const std::string& filePath);

    /**
     * @brief 打开 URL
     *
     * 使用系统默认浏览器打开 URL。
     *
     * @param url URL 地址
     * @return 打开成功返回 true
     */
    static bool openUrl(const std::string& url);

    /**
     * @brief 获取当前进程 ID
     *
     * @return 当前进程的 ID
     */
    static int getProcessId();

    /**
     * @brief 获取环境变量值
     *
     * @param name 环境变量名
     * @param defaultValue 默认值（当环境变量不存在时返回）
     * @return 环境变量值
     */
    static std::string getEnvironmentVariable(const std::string& name, const std::string& defaultValue = "");
};

}

#endif