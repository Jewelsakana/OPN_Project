#include "WorkSpace.h"
#include "CommandController.h"
#include "FileSystemService.h"
#include <iostream>
#include <cassert>
#include <memory>
#include <fstream>
#include <filesystem>

namespace fs = std::filesystem;

void test_log_recovery() {
    std::cout << "\n=== Testing Log Recovery ===" << std::endl;

    // 清理可能存在的旧文件
    if (fs::exists("test_log.txt")) fs::remove("test_log.txt");
    if (fs::exists(".editor_config")) fs::remove(".editor_config");
    if (fs::exists(".test_log.txt.log")) fs::remove(".test_log.txt.log");

    {
        std::cout << "1. Creating workspace and starting log..." << std::endl;
        WorkSpace workspace;

        // 加载一个文件
        workspace.loadFile("test_log.txt");

        // 启动日志记录
        workspace.startLoggingForFile("test_log.txt");
        assert(workspace.isLoggingForFile("test_log.txt") == true);
        std::cout << "   Log started for test_log.txt" << std::endl;

        // 执行一些操作来生成日志
        workspace.notifySessionStart(); // 模拟会话开始

        // 保存配置
        workspace.saveConfig(".editor_config");
        std::cout << "   Configuration saved" << std::endl;

        // 检查日志文件是否存在
        assert(fs::exists(".test_log.txt.log") == true);
        std::cout << "   Log file created: .test_log.txt.log" << std::endl;
    }

    std::cout << "\n2. Creating new workspace to restore state..." << std::endl;
    {
        WorkSpace workspace2;
        // 构造函数应该自动加载配置

        // 检查文件是否恢复打开
        assert(workspace2.isFileOpen("test_log.txt") == true);
        std::cout << "   File test_log.txt is open (restored)" << std::endl;

        // 检查日志是否恢复
        assert(workspace2.isLoggingForFile("test_log.txt") == true);
        std::cout << "   Logging restored for test_log.txt" << std::endl;

        // 执行一个操作来验证日志仍在记录
        workspace2.notifySessionStart(); // 第二次会话开始

        // 检查日志文件是否有新条目
        std::ifstream logFile(".test_log.txt.log");
        std::string line;
        int lineCount = 0;
        while (std::getline(logFile, line)) {
            lineCount++;
        }
        logFile.close();

        // 应该至少有2行：第一次会话开始和第二次会话开始
        assert(lineCount >= 2);
        std::cout << "   Log file has " << lineCount << " entries (expected >= 2)" << std::endl;
    }

    std::cout << "\n3. Testing with #log in file content..." << std::endl;
    {
        // 创建一个有"# log"的文件
        std::ofstream file("test_with_hashlog.txt");
        file << "# log\nThis is a test file with log marker";
        file.close();

        WorkSpace workspace3;
        workspace3.loadFile("test_with_hashlog.txt");

        // 检查是否自动启动了日志
        assert(workspace3.isLoggingForFile("test_with_hashlog.txt") == true);
        std::cout << "   Auto-logging started for file with #log" << std::endl;

        // 保存配置
        workspace3.saveConfig(".editor_config");
    }

    std::cout << "\n4. Restoring workspace with #log file..." << std::endl;
    {
        WorkSpace workspace4;
        // 应该自动恢复

        assert(workspace4.isFileOpen("test_with_hashlog.txt") == true);
        std::cout << "   File with #log restored" << std::endl;

        // 检查日志是否恢复
        assert(workspace4.isLoggingForFile("test_with_hashlog.txt") == true);
        std::cout << "   Logging restored for file with #log" << std::endl;
    }

    // 清理测试文件
    if (fs::exists("test_log.txt")) fs::remove("test_log.txt");
    if (fs::exists("test_with_hashlog.txt")) fs::remove("test_with_hashlog.txt");
    if (fs::exists(".editor_config")) fs::remove(".editor_config");
    if (fs::exists(".test_log.txt.log")) fs::remove(".test_log.txt.log");
    if (fs::exists(".test_with_hashlog.txt.log")) fs::remove(".test_with_hashlog.txt.log");

    std::cout << "\n=== All log recovery tests passed! ===" << std::endl;
}

void test_config_file_format() {
    std::cout << "\n=== Testing Config File Format ===" << std::endl;

    if (fs::exists(".editor_config")) fs::remove(".editor_config");

    {
        WorkSpace workspace;
        workspace.loadFile("file1.txt");
        workspace.loadFile("file2.txt");
        workspace.setActiveFile("file2.txt");
        workspace.setFileModified("file1.txt", true);
        workspace.startLoggingForFile("file1.txt");

        workspace.saveConfig(".editor_config");
        std::cout << "Configuration saved" << std::endl;
    }

    // 读取配置文件检查格式
    std::ifstream configFile(".editor_config");
    std::string line;
    bool hasLoggedFiles = false;
    while (std::getline(configFile, line)) {
        std::cout << "Config line: " << line << std::endl;
        if (line.find("loggedFiles:") == 0) {
            hasLoggedFiles = true;
            // 检查是否包含file1.txt
            if (line.find("file1.txt") != std::string::npos) {
                std::cout << "   Contains logged file: file1.txt" << std::endl;
            }
        }
    }
    configFile.close();

    assert(hasLoggedFiles == true);
    std::cout << "Config file contains loggedFiles field" << std::endl;

    // 清理
    if (fs::exists(".editor_config")) fs::remove(".editor_config");
    if (fs::exists("file1.txt")) fs::remove("file1.txt");
    if (fs::exists("file2.txt")) fs::remove("file2.txt");

    std::cout << "Config file format test passed!" << std::endl;
}

int main() {
    std::cout << "Starting log recovery tests..." << std::endl;

    try {
        test_config_file_format();
        test_log_recovery();

        std::cout << "\nAll tests completed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}