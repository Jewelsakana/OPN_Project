#include "LoggerManager.h"
#include "FileSystemService.h"
#include "WorkSpace.h"
#include <iostream>
#include <cassert>
#include <algorithm>

// 测试LoggerManager基本功能
void testLoggerManagerBasic() {
    std::cout << "Testing LoggerManager basic functionality..." << std::endl;

    FileSystemService fsService;
    WorkSpace workspace;
    LoggerManager manager(fsService, workspace);

    // 测试初始状态
    assert(manager.getLoggerCount() == 0);
    assert(manager.getLoggedFiles().empty());
    assert(workspace.getObserverCount() == 0);

    // 启动日志记录
    manager.startLoggingForFile("test1.txt");
    assert(manager.getLoggerCount() == 1);
    assert(manager.isLoggingForFile("test1.txt"));
    assert(workspace.getObserverCount() == 1);

    // 重复启动应该无变化
    manager.startLoggingForFile("test1.txt");
    assert(manager.getLoggerCount() == 1);
    assert(workspace.getObserverCount() == 1); // 观察者不应重复添加

    // 启动另一个文件的日志
    manager.startLoggingForFile("test2.txt");
    assert(manager.getLoggerCount() == 2);
    assert(manager.isLoggingForFile("test2.txt"));
    assert(workspace.getObserverCount() == 2);

    // 获取已记录的文件列表
    auto files = manager.getLoggedFiles();
    assert(files.size() == 2);
    assert(std::find(files.begin(), files.end(), "test1.txt") != files.end());
    assert(std::find(files.begin(), files.end(), "test2.txt") != files.end());

    // 停止日志记录
    manager.stopLoggingForFile("test1.txt");
    assert(manager.getLoggerCount() == 1);
    assert(!manager.isLoggingForFile("test1.txt"));
    assert(manager.isLoggingForFile("test2.txt"));
    assert(workspace.getObserverCount() == 1);

    // 停止不存在的文件应该无影响
    manager.stopLoggingForFile("nonexistent.txt");
    assert(manager.getLoggerCount() == 1);
    assert(workspace.getObserverCount() == 1);

    // 清理
    manager.stopLoggingForFile("test2.txt");
    assert(manager.getLoggerCount() == 0);
    assert(workspace.getObserverCount() == 0);

    std::cout << "LoggerManager basic test passed." << std::endl;
}

// 测试LoggerManager继承Model基类
void testLoggerManagerModelInheritance() {
    std::cout << "Testing LoggerManager Model inheritance..." << std::endl;

    FileSystemService fsService;
    WorkSpace workspace;
    LoggerManager manager(fsService, workspace);

    // 测试getName方法
    assert(manager.getName() == "LoggerManager");

    // 测试异常安全（使用safeExecute包装）
    // 启动日志记录应该成功
    manager.startLoggingForFile("test.txt");
    assert(manager.getLoggerCount() == 1);

    // 停止日志记录
    manager.stopLoggingForFile("test.txt");
    assert(manager.getLoggerCount() == 0);

    std::cout << "Model inheritance test passed." << std::endl;
}

// 测试LoggerManager与FileSystemService的集成
void testLoggerManagerFileIntegration() {
    std::cout << "Testing LoggerManager file integration..." << std::endl;

    FileSystemService fsService;
    WorkSpace workspace;
    LoggerManager manager(fsService, workspace);

    // 启动日志记录并执行事件
    manager.startLoggingForFile("integration.txt");

    // 创建事件并通知（模拟命令执行）
    Event event1("insert 1:1 \"Hello\"", "integration.txt");
    workspace.notify(event1);

    Event event2("append \" World\"", "integration.txt");
    workspace.notify(event2);

    // 日志文件应该被创建
    // 注意：FileLogger会在当前目录创建".integration.txt.log"文件
    // 我们无法直接验证文件内容，但可以验证日志记录器正常工作
    assert(manager.isLoggingForFile("integration.txt"));

    // 停止日志记录
    manager.stopLoggingForFile("integration.txt");
    assert(manager.getLoggerCount() == 0);
    assert(workspace.getObserverCount() == 0);

    std::cout << "File integration test passed." << std::endl;
}

int main() {
    try {
        testLoggerManagerBasic();
        testLoggerManagerModelInheritance();
        testLoggerManagerFileIntegration();

        std::cout << "\nAll LoggerManager tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed: " << e.what() << std::endl;
        return 1;
    }
}