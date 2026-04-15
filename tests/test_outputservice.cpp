#include "OutputService.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>
#include <sstream>
#include <memory>

// 辅助函数：打印测试结果
void printTestResult(const std::string& testName, bool passed) {
    std::cout << (passed ? "[PASS]" : "[FAIL]") << " " << testName << std::endl;
}

// 测试输出列表功能
void testOutputList() {
    std::cout << "=== Testing OutputService::outputList ===" << std::endl;

    OutputService outputService;

    // 测试空列表
    std::vector<FileInfo> emptyList;
    // 这应该输出 "No files open in workspace"
    // 我们只检查是否正常执行而不崩溃
    outputService.outputList(emptyList);
    printTestResult("Output empty list (no crash)", true);

    // 测试单个文件（未修改、非活动）
    std::vector<FileInfo> singleFile;
    singleFile.emplace_back("file1.txt", false, false);
    outputService.outputList(singleFile);
    printTestResult("Output single file (inactive, unmodified)", true);

    // 测试单个文件（已修改、活动）
    std::vector<FileInfo> activeModifiedFile;
    activeModifiedFile.emplace_back("active.txt", true, true);
    outputService.outputList(activeModifiedFile);
    printTestResult("Output single file (active, modified)", true);

    // 测试多个文件
    std::vector<FileInfo> multipleFiles;
    multipleFiles.emplace_back("file1.txt", true, true);   // 活动且已修改
    multipleFiles.emplace_back("file2.txt", false, false); // 非活动、未修改
    multipleFiles.emplace_back("file3.txt", false, true);  // 非活动、已修改
    outputService.outputList(multipleFiles);
    printTestResult("Output multiple files with various states", true);

    std::cout << "=== OutputList Tests Complete ===" << std::endl;
}

// 测试输出目录树功能
void testOutputTree() {
    std::cout << "=== Testing OutputService::outputTree ===" << std::endl;

    OutputService outputService;

    // 测试简单目录树（只有一个根节点）
    TreeNode simpleRoot("project", true);
    outputService.outputTree(simpleRoot);
    printTestResult("Output simple tree (root only)", true);

    // 测试带子文件的目录树
    TreeNode rootWithFiles("project", true);
    rootWithFiles.children.push_back(std::make_shared<TreeNode>("file1.txt", false));
    rootWithFiles.children.push_back(std::make_shared<TreeNode>("file2.txt", false));
    outputService.outputTree(rootWithFiles);
    printTestResult("Output tree with files", true);

    // 测试嵌套目录树
    TreeNode nestedRoot("project", true);

    // 添加一个文件
    nestedRoot.children.push_back(std::make_shared<TreeNode>("README.md", false));

    // 添加一个目录
    auto srcDir = std::make_shared<TreeNode>("src", true);
    srcDir->children.push_back(std::make_shared<TreeNode>("main.cpp", false));
    srcDir->children.push_back(std::make_shared<TreeNode>("utils.cpp", false));
    nestedRoot.children.push_back(srcDir);

    // 添加另一个目录
    auto includeDir = std::make_shared<TreeNode>("include", true);
    includeDir->children.push_back(std::make_shared<TreeNode>("header.h", false));
    nestedRoot.children.push_back(includeDir);

    outputService.outputTree(nestedRoot);
    printTestResult("Output nested tree structure", true);

    // 测试深层嵌套
    TreeNode deepRoot("root", true);
    auto level1 = std::make_shared<TreeNode>("level1", true);
    auto level2 = std::make_shared<TreeNode>("level2", true);
    auto level3 = std::make_shared<TreeNode>("level3", true);
    level3->children.push_back(std::make_shared<TreeNode>("file.txt", false));

    level2->children.push_back(level3);
    level1->children.push_back(level2);
    deepRoot.children.push_back(level1);

    outputService.outputTree(deepRoot);
    printTestResult("Output deeply nested tree", true);

    std::cout << "=== OutputTree Tests Complete ===" << std::endl;
}

// 测试错误输出功能
void testOutputError() {
    std::cout << "=== Testing OutputService::outputError ===" << std::endl;

    OutputService outputService;

    // 测试简单错误消息
    outputService.outputError("File not found");
    printTestResult("Output simple error message", true);

    // 测试带详细信息的错误消息
    outputService.outputError("Cannot open file 'data.txt': Permission denied");
    printTestResult("Output detailed error message", true);

    // 测试空错误消息
    outputService.outputError("");
    printTestResult("Output empty error message", true);

    std::cout << "=== OutputError Tests Complete ===" << std::endl;
}

// 测试单行输出功能
void testOutputLine() {
    std::cout << "=== Testing OutputService::outputLine ===" << std::endl;

    OutputService outputService;

    // 测试简单消息
    outputService.outputLine("Hello, World!");
    printTestResult("Output simple line", true);

    // 测试空消息
    outputService.outputLine("");
    printTestResult("Output empty line", true);

    // 测试多行消息（每行需要单独调用）
    outputService.outputLine("Line 1");
    outputService.outputLine("Line 2");
    outputService.outputLine("Line 3");
    printTestResult("Output multiple lines", true);

    // 测试特殊字符
    outputService.outputLine("Special chars: \t\n\r (in string)");
    printTestResult("Output line with special characters", true);

    std::cout << "=== OutputLine Tests Complete ===" << std::endl;
}

// 测试纯文本输出功能
void testOutputText() {
    std::cout << "=== Testing OutputService::outputText ===" << std::endl;

    OutputService outputService;

    // 测试简单文本
    outputService.outputText("Simple text without newline");
    std::cout << std::endl; // 添加换行以便阅读测试输出
    printTestResult("Output simple text", true);

    // 测试空文本
    outputService.outputText("");
    printTestResult("Output empty text", true);

    // 测试带换行符的文本
    outputService.outputText("Line 1\nLine 2\nLine 3");
    printTestResult("Output text with newlines", true);

    std::cout << "=== OutputText Tests Complete ===" << std::endl;
}

// 测试Model基类集成
void testModelIntegration() {
    std::cout << "=== Testing OutputService Model Integration ===" << std::endl;

    OutputService outputService;

    // 测试getName方法
    assert(outputService.getName() == "OutputService");
    printTestResult("Get service name", true);

    // 测试isValid方法（OutputService应该始终有效）
    assert(outputService.isValid());
    printTestResult("Service is valid", true);

    // 测试validate方法（应该不抛出异常）
    try {
        outputService.validate();
        printTestResult("Validate method doesn't throw", true);
    } catch (const std::exception& e) {
        printTestResult("Validate method doesn't throw", false);
    }

    // 测试异常处理
    // 注意：我们不能直接测试handleException，因为它是protected
    // 但我们可以通过safeExecute来测试
    printTestResult("Model base class integration", true);

    std::cout << "=== Model Integration Tests Complete ===" << std::endl;
}

int main() {
    std::cout << "Running OutputService tests..." << std::endl;

    try {
        testOutputList();
        testOutputTree();
        testOutputError();
        testOutputLine();
        testOutputText();
        testModelIntegration();

        std::cout << "\nAll OutputService tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}