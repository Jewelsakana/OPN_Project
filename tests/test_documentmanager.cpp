#include "DocumentManager.h"
#include "TextEditor.h"
#include <iostream>
#include <cassert>
#include <memory>
#include <vector>
#include <string>
#include <algorithm>

// 辅助函数：打印测试结果
void printTestResult(const std::string& testName, bool passed) {
    std::cout << (passed ? "[PASS]" : "[FAIL]") << " " << testName << std::endl;
}

// 测试基本功能：打开、关闭、活动文件管理
void testBasicFunctionality() {
    std::cout << "=== Testing DocumentManager Basic Functionality ===" << std::endl;

    DocumentManager manager;

    // 测试初始状态
    assert(manager.getOpenFileCount() == 0);
    assert(manager.getActiveFileName().empty());
    assert(manager.getActiveEditor() == nullptr);
    assert(!manager.hasUnsavedFiles());
    printTestResult("Initial state", true);

    // 创建一些编辑器实例
    auto editor1 = std::make_shared<TextEditor>();
    auto editor2 = std::make_shared<TextEditor>();
    auto editor3 = std::make_shared<TextEditor>();

    // 测试打开文件
    manager.openFile("test1.txt", editor1);
    assert(manager.isFileOpen("test1.txt"));
    assert(manager.getOpenFileCount() == 1);
    assert(manager.getActiveFileName() == "test1.txt");
    assert(manager.getActiveEditor() == editor1);
    printTestResult("Open first file", true);

    manager.openFile("test2.txt", editor2);
    assert(manager.isFileOpen("test2.txt"));
    assert(manager.getOpenFileCount() == 2);
    assert(manager.getActiveFileName() == "test1.txt"); // 第一个文件仍然是活动的
    printTestResult("Open second file", true);

    // 测试打开重复文件（应该更新编辑器）
    auto editor1_new = std::make_shared<TextEditor>();
    manager.openFile("test1.txt", editor1_new);
    assert(manager.getEditor("test1.txt") == editor1_new); // 应该更新为新的编辑器
    printTestResult("Update existing file with new editor", true);

    // 测试获取打开文件列表
    auto openFiles = manager.getOpenFiles();
    assert(openFiles.size() == 2);
    assert(std::find(openFiles.begin(), openFiles.end(), "test1.txt") != openFiles.end());
    assert(std::find(openFiles.begin(), openFiles.end(), "test2.txt") != openFiles.end());
    printTestResult("Get open files list", true);

    // 测试文件信息列表
    auto fileInfos = manager.getFileInfoList();
    assert(fileInfos.size() == 2);
    // 检查第一个文件是活动的
    bool foundActive = false;
    for (const auto& info : fileInfos) {
        if (info.fileName == "test1.txt") {
            assert(info.isActive);
            foundActive = true;
        } else if (info.fileName == "test2.txt") {
            assert(!info.isActive);
        }
    }
    assert(foundActive);
    printTestResult("Get file info list", true);

    // 测试设置活动文件
    manager.setActiveFile("test2.txt");
    assert(manager.getActiveFileName() == "test2.txt");
    assert(manager.getActiveEditor() == editor2);
    printTestResult("Set active file", true);

    // 测试设置不存在的文件为活动文件（应该抛出异常）
    try {
        manager.setActiveFile("nonexistent.txt");
        assert(false); // 不应该到达这里
    } catch (const std::exception& e) {
        // 应该抛出异常
        printTestResult("Set active file for non-existent file throws exception", true);
    }

    // 测试修改状态管理
    manager.setFileModified("test1.txt", true);
    assert(manager.isFileModified("test1.txt"));
    assert(!manager.isFileModified("test2.txt")); // test2.txt 应该未修改
    printTestResult("Set file modified state", true);

    // 测试检查未保存文件
    assert(manager.hasUnsavedFiles());
    auto unsavedFiles = manager.getUnsavedFiles();
    assert(unsavedFiles.size() == 1);
    assert(unsavedFiles[0] == "test1.txt");
    printTestResult("Check unsaved files", true);

    // 清除test1.txt的修改状态
    manager.setFileModified("test1.txt", false);
    assert(!manager.hasUnsavedFiles());
    printTestResult("Clear modified state", true);

    // 测试关闭文件
    manager.closeFile("test1.txt");
    assert(!manager.isFileOpen("test1.txt"));
    assert(manager.isFileOpen("test2.txt"));
    assert(manager.getOpenFileCount() == 1);
    // 活动文件应该自动切换到test2.txt（如果test1.txt是活动的，但我们已经切换到了test2.txt）
    assert(manager.getActiveFileName() == "test2.txt");
    printTestResult("Close file", true);

    // 测试关闭活动文件
    manager.openFile("test3.txt", editor3);
    manager.setActiveFile("test3.txt");
    manager.closeFile("test3.txt");
    // 活动文件应该自动切换到剩余的打开文件（test2.txt）
    assert(manager.getActiveFileName() == "test2.txt");
    assert(manager.getOpenFileCount() == 1);
    printTestResult("Close active file (auto-switch)", true);

    // 测试清空所有状态
    manager.clear();
    assert(manager.getOpenFileCount() == 0);
    assert(manager.getActiveFileName().empty());
    assert(!manager.hasUnsavedFiles());
    printTestResult("Clear all state", true);

    std::cout << "=== DocumentManager Basic Functionality Tests Complete ===" << std::endl;
}

// 测试边界条件和异常处理
void testEdgeCasesAndExceptions() {
    std::cout << "=== Testing DocumentManager Edge Cases and Exceptions ===" << std::endl;

    DocumentManager manager;
    auto editor = std::make_shared<TextEditor>();

    // 测试空文件名
    try {
        manager.openFile("", editor);
        assert(false); // 不应该到达这里
    } catch (const std::exception& e) {
        // 应该抛出异常
        printTestResult("Open file with empty name throws exception", true);
    }

    // 测试获取不存在的文件的编辑器
    assert(manager.getEditor("nonexistent.txt") == nullptr);
    printTestResult("Get editor for non-existent file returns nullptr", true);

    // 测试检查不存在的文件的修改状态
    assert(!manager.isFileModified("nonexistent.txt")); // 应该返回false
    printTestResult("Check modified state for non-existent file returns false", true);

    // 测试设置不存在的文件的修改状态（应该创建状态记录）
    manager.setFileModified("nonexistent.txt", true);
    // 注意：这可能会在内部创建一个状态记录，但文件并没有实际打开
    printTestResult("Set modified state for non-existent file (edge case)", true);

    // 测试有效性检查
    assert(manager.isValid()); // 空管理器应该是有效的
    printTestResult("Empty manager is valid", true);

    // 打开一个文件并检查有效性
    manager.openFile("valid.txt", editor);
    assert(manager.isValid());
    printTestResult("Manager with open file is valid", true);

    std::cout << "=== DocumentManager Edge Cases Tests Complete ===" << std::endl;
}

// 测试统计信息
void testStatistics() {
    std::cout << "=== Testing DocumentManager Statistics ===" << std::endl;

    DocumentManager manager;

    // 初始统计
    assert(manager.getOpenFileCount() == 0);
    assert(manager.getModifiedFileCount() == 0);
    printTestResult("Initial statistics", true);

    // 打开几个文件
    auto editor1 = std::make_shared<TextEditor>();
    auto editor2 = std::make_shared<TextEditor>();
    auto editor3 = std::make_shared<TextEditor>();

    manager.openFile("file1.txt", editor1);
    manager.openFile("file2.txt", editor2);
    manager.openFile("file3.txt", editor3);

    assert(manager.getOpenFileCount() == 3);
    assert(manager.getModifiedFileCount() == 0);
    printTestResult("Statistics after opening files", true);

    // 设置一些文件为修改状态
    manager.setFileModified("file1.txt", true);
    manager.setFileModified("file3.txt", true);

    assert(manager.getModifiedFileCount() == 2);
    printTestResult("Statistics after modifying files", true);

    // 获取所有修改状态
    const auto& allStates = manager.getAllModifiedStates();
    assert(allStates.size() == 3); // 三个文件都有状态记录
    assert(allStates.at("file1.txt") == true);
    assert(allStates.at("file2.txt") == false);
    assert(allStates.at("file3.txt") == true);
    printTestResult("Get all modified states", true);

    std::cout << "=== DocumentManager Statistics Tests Complete ===" << std::endl;
}

int main() {
    std::cout << "Running DocumentManager tests..." << std::endl;

    try {
        testBasicFunctionality();
        testEdgeCasesAndExceptions();
        testStatistics();

        std::cout << "\nAll DocumentManager tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}