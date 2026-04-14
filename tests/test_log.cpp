#include "WorkSpace.h"
#include "CommandController.h"
#include "Logger.h"
#include "Event.h"
#include <iostream>
#include <cassert>
#include <fstream>
#include <string>

// 辅助函数：检查文件是否存在
bool fileExists(const std::string& filename) {
    std::ifstream file(filename);
    return file.good();
}

// 辅助函数：读取文件内容
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) return "";
    std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
    return content;
}

// 测试日志命令
void testLogCommands() {
    std::cout << "Testing log commands..." << std::endl;
    WorkSpace ws;
    CommandController controller(&ws);

    // 创建新文件并启动日志
    controller.parseAndExecuteCommand("init test.txt");
    controller.parseAndExecuteCommand("log-on test.txt");

    // 执行一些命令，应该被记录
    controller.parseAndExecuteCommand("insert 1:1 \"Hello\"");
    controller.parseAndExecuteCommand("append \" World\"");

    // 停止日志
    controller.parseAndExecuteCommand("log-off test.txt");

    // 检查日志文件是否存在
    std::string logFile = ".test.txt.log";
    assert(fileExists(logFile));

    // 读取日志文件，检查内容
    std::string logContent = readFile(logFile);
    assert(logContent.find("insert") != std::string::npos);
    assert(logContent.find("append") != std::string::npos);

    // 清理
    std::remove("test.txt");
    std::remove(logFile.c_str());
    std::cout << "Log commands test passed." << std::endl;
}

// 测试自动日志启动（# log）
void testAutoLogStart() {
    std::cout << "Testing auto log start..." << std::endl;
    // 创建一个带有 "# log" 的文件
    std::ofstream file("autolog.txt");
    file << "# log\nHello World";
    file.close();

    WorkSpace ws;
    CommandController controller(&ws);
    controller.parseAndExecuteCommand("load autolog.txt");

    // 日志应该自动启动
    // 执行命令
    controller.parseAndExecuteCommand("insert 2:1 \"Test\"");

    // 检查日志文件
    std::string logFile = ".autolog.txt.log";
    assert(fileExists(logFile));

    // 清理
    std::remove("autolog.txt");
    std::remove(logFile.c_str());
    std::cout << "Auto log start test passed." << std::endl;
}

// 测试log-show命令
void testLogShowCommand() {
    std::cout << "Testing log-show command..." << std::endl;
    WorkSpace ws;
    CommandController controller(&ws);

    // 创建新文件并启动日志
    controller.parseAndExecuteCommand("init showtest.txt");
    controller.parseAndExecuteCommand("log-on showtest.txt");

    // 执行一些命令，应该被记录
    controller.parseAndExecuteCommand("insert 1:1 \"Test log show\"");
    controller.parseAndExecuteCommand("append \" additional text\"");

    // 使用log-show命令显示日志
    controller.parseAndExecuteCommand("log-show showtest.txt");

    // 停止日志并清理
    controller.parseAndExecuteCommand("log-off showtest.txt");

    // 检查日志文件是否存在
    std::string logFile = ".showtest.txt.log";
    assert(fileExists(logFile));

    // 读取日志文件，检查内容
    std::string logContent = readFile(logFile);
    assert(logContent.find("Test log show") != std::string::npos);
    assert(logContent.find("additional text") != std::string::npos);

    // 清理
    std::remove("showtest.txt");
    std::remove(logFile.c_str());
    std::cout << "Log-show command test passed." << std::endl;
}

// 测试日志写入失败警告（模拟权限错误）
void testLogWriteFailure() {
    std::cout << "Testing log write failure handling..." << std::endl;
    // 创建一个只读目录？跳过，因为需要特殊环境
    std::cout << "Log write failure test skipped (requires special environment)." << std::endl;
}

int main() {
    try {
        testLogCommands();
        testAutoLogStart();
        testLogShowCommand();
        testLogWriteFailure();
        std::cout << "All log tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}