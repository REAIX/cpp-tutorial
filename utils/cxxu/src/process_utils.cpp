/**
 * @file process_utils.cpp
 * @brief 进程工具实现
 *
 * 实现延时、命令执行、进程信息获取等功能，支持跨平台。
 *
 * @author CU Utils Project
 * @version 1.0
 */

#include "cu_utils/process_utils.h"
#include <atomic>
#include <cstdlib>
#include <cstdio>
#include <array>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <windows.h>
#include <process.h>
#else
#include <unistd.h>
#include <sys/wait.h>
#endif

namespace cu {

/**
 * @brief 阻塞式延时
 * @param milliseconds 延时的毫秒数
 */
void ProcessUtils::sleep(unsigned int milliseconds) {
#ifdef _WIN32
    ::Sleep(milliseconds);
#else
    std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
#endif
}

/**
 * @brief 执行系统命令
 * @param command 要执行的命令字符串
 * @param timeout 超时时间（秒），-1 表示无超时
 * @return 元组(退出码, 标准输出, 标准错误)
 */
std::tuple<int, std::string, std::string> ProcessUtils::executeCommand(const std::string& command, [[maybe_unused]] int timeout) {
    // TODO: timeout 参数尚未实现，当前命令可能无限期阻塞。
    //       完整实现需要使用独立线程 + std::future::wait_for 来强制超时，
    //       并在超时后终止子进程，此逻辑高度依赖平台 API。

    if (command.empty()) {
        return {-1, "", "Command is empty"};
    }

    std::string stdoutResult;
    std::string stderrResult;

    static std::atomic<int> stderrFileCounter{0};
    std::string stderrFilePath;
#ifdef _WIN32
    char tmpDir[MAX_PATH];
    GetTempPathA(MAX_PATH, tmpDir);
    stderrFilePath = std::string(tmpDir) + "cmd_stderr_" + std::to_string(getProcessId()) + "_" + std::to_string(++stderrFileCounter) + ".tmp";
#else
    const char* envTmp = std::getenv("TMPDIR");
    stderrFilePath = std::string(envTmp ? envTmp : "/tmp") + "/cmd_stderr_" + std::to_string(getProcessId()) + "_" + std::to_string(++stderrFileCounter) + ".tmp";
#endif

    std::string fullCommand = command + " 2>\"" + stderrFilePath + "\"";

#ifdef _WIN32
    FILE* pipe = _popen(fullCommand.c_str(), "r");
#else
    FILE* pipe = popen(fullCommand.c_str(), "r");
#endif

    if (!pipe) {
        std::remove(stderrFilePath.c_str());
        return {-1, "", "Failed to execute command"};
    }

    std::array<char, 256> buffer;
    while (fgets(buffer.data(), buffer.size(), pipe) != nullptr) {
        stdoutResult += buffer.data();
    }

#ifdef _WIN32
    int result = _pclose(pipe);
    int exitCode = result;
#else
    int result = pclose(pipe);
    int exitCode = WIFEXITED(result) ? WEXITSTATUS(result) : -1;
#endif

    FILE* stderrFile = std::fopen(stderrFilePath.c_str(), "r");
    if (stderrFile) {
        while (fgets(buffer.data(), buffer.size(), stderrFile) != nullptr) {
            stderrResult += buffer.data();
        }
        std::fclose(stderrFile);
    }
    std::remove(stderrFilePath.c_str());

    return {exitCode, stdoutResult, stderrResult};
}

/**
 * @brief 打开文件
 * @param filePath 文件路径
 * @return 打开成功返回 true
 */
bool ProcessUtils::openFile(const std::string& filePath) {
    if (filePath.empty()) return false;

#ifdef _WIN32
    return (intptr_t)ShellExecuteA(NULL, "open", filePath.c_str(), NULL, NULL, SW_SHOWNORMAL) > 32;
#else
    auto escapePath = [](const std::string& path) -> std::string {
        std::string escaped;
        escaped.reserve(path.size() + 2);
        for (char c : path) {
            if (c == '"' || c == '$' || c == '`' || c == '(' || c == ')' || c == '\\') {
                escaped += '\\';
                escaped += c;
            } else {
                escaped += c;
            }
        }
        return "\"" + escaped + "\"";
    };

#ifdef __APPLE__
    return system(("open " + escapePath(filePath)).c_str()) == 0;
#else
    return system(("xdg-open " + escapePath(filePath)).c_str()) == 0;
#endif
#endif
}

/**
 * @brief 打开 URL
 * @param url URL 地址
 * @return 打开成功返回 true
 */
bool ProcessUtils::openUrl(const std::string& url) {
    return openFile(url);
}

/**
 * @brief 获取当前进程 ID
 * @return 当前进程的 ID
 */
int ProcessUtils::getProcessId() {
#ifdef _WIN32
    return _getpid();
#else
    return getpid();
#endif
}

/**
 * @brief 获取环境变量值
 * @param name 环境变量名
 * @param defaultValue 默认值（当环境变量不存在时返回）
 * @return 环境变量值
 */
std::string ProcessUtils::getEnvironmentVariable(const std::string& name, const std::string& defaultValue) {
    if (name.empty()) return defaultValue;

    const char* value = std::getenv(name.c_str());
    return value ? std::string(value) : defaultValue;
}

}