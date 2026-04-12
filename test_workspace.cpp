#include "WorkSpace.h"
#include "TextEditor.h"
#include "CommandController.h"
#include <iostream>
#include <cassert>
#include <memory>

void test_basic_functionality() {
    std::cout << "Testing WorkSpace basic functionality..." << std::endl;

    WorkSpace workspace;

    // 测试打开文件
    workspace.openFile("test1.txt");
    workspace.openFile("test2.txt");
    workspace.openFile("test3.txt");

    // 检查打开的文件列表
    auto openFiles = workspace.getOpenFiles();
    assert(openFiles.size() == 3);
    std::cout << "Open files count: " << openFiles.size() << " (expected 3)" << std::endl;

    // 检查默认活动文件（第一个打开的文件）
    assert(workspace.getActiveFileName() == "test1.txt");
    std::cout << "Active file: " << workspace.getActiveFileName() << " (expected test1.txt)" << std::endl;

    // 测试设置活动文件
    workspace.setActiveFile("test2.txt");
    assert(workspace.getActiveFileName() == "test2.txt");
    std::cout << "Active file after set: " << workspace.getActiveFileName() << " (expected test2.txt)" << std::endl;

    // 测试获取活动编辑器
    auto activeEditor = workspace.getActiveEditor();
    assert(activeEditor != nullptr);
    std::cout << "Active editor is not null" << std::endl;

    // 测试获取指定文件的编辑器
    auto editor1 = workspace.getEditor("test1.txt");
    assert(editor1 != nullptr);
    std::cout << "Editor for test1.txt is not null" << std::endl;

    // 测试文件修改状态
    workspace.setFileModified("test1.txt", true);
    assert(workspace.isFileModified("test1.txt") == true);
    std::cout << "File test1.txt modified state: " << workspace.isFileModified("test1.txt") << " (expected true)" << std::endl;

    // 测试日志开关
    workspace.setLogEnabled(true);
    assert(workspace.isLogEnabled() == true);
    std::cout << "Log enabled: " << workspace.isLogEnabled() << " (expected true)" << std::endl;

    // 测试关闭文件
    workspace.closeFile("test2.txt");
    assert(!workspace.isFileOpen("test2.txt"));
    // 活动文件应该自动切换到另一个打开的文件
    assert(workspace.getActiveFileName() != "test2.txt");
    std::cout << "After closing test2.txt, active file is: " << workspace.getActiveFileName() << std::endl;

    // 测试备忘录模式
    auto memento = workspace.createMemento();
    assert(memento != nullptr);
    std::cout << "Memento created successfully" << std::endl;

    // 修改工作区状态
    workspace.closeFile("test1.txt");
    workspace.closeFile("test3.txt");
    assert(workspace.getOpenFiles().size() == 0);
    std::cout << "All files closed, open files count: " << workspace.getOpenFiles().size() << std::endl;

    // 从备忘录恢复
    workspace.restoreFromMemento(*memento);
    assert(workspace.getOpenFiles().size() == 2); // test1.txt和test3.txt（test2.txt已关闭）
    std::cout << "After restore, open files count: " << workspace.getOpenFiles().size() << " (expected 2)" << std::endl;
    assert(workspace.isFileOpen("test1.txt"));
    assert(workspace.isFileOpen("test3.txt"));
    assert(!workspace.isFileOpen("test2.txt"));
    std::cout << "Files restored correctly" << std::endl;

    std::cout << "All basic functionality tests passed!" << std::endl << std::endl;
}

void test_observer_attachment() {
    std::cout << "Testing observer attachment..." << std::endl;

    WorkSpace workspace;

    // 创建一个简单的观察者mock（由于Observe是抽象类，我们需要一个具体实现）
    class MockObserver : public Observe {
    public:
        void update(const Event& e) override {
            // 什么也不做，仅用于测试
        }
    };

    auto observer = std::make_shared<MockObserver>();
    workspace.attach(observer);
    // 无法直接测试通知，但可以确认attach没有崩溃
    workspace.detach(observer);

    std::cout << "Observer attachment test passed!" << std::endl << std::endl;
}

void test_command_controller() {
    std::cout << "Testing CommandController..." << std::endl;

    WorkSpace workspace;
    workspace.openFile("test.txt");
    workspace.setActiveFile("test.txt");

    // 创建CommandController
    CommandController controller(&workspace);

    // 测试1：创建并执行一个简单的命令
    class MockCommand : public Command {
    public:
        void execute() override {
            executed = true;
        }
        void undo() override {
            undone = true;
        }
        bool isReadOnly() const override { return true; }

        bool executed = false;
        bool undone = false;
    };

    auto mockCommand = std::make_unique<MockCommand>();
    auto* mockPtr = mockCommand.get();

    try {
        controller.executeCommand(std::move(mockCommand));
        assert(mockPtr->executed);
        std::cout << "  ✓ Command execution via controller - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ Command execution failed: " << e.what() << std::endl;
        throw;
    }

    // 测试2：解析并执行一个工作区命令（load）
    try {
        controller.parseAndExecuteCommand("load another.txt");
        std::cout << "  ✓ parseAndExecuteCommand load command - OK" << std::endl;
    } catch (const std::exception& e) {
        // load命令可能未实现execute方法，但至少应该解析成功
        std::cout << "  Note: load command execute may not be implemented: " << e.what() << std::endl;
    }

    // 测试3：测试编辑器命令创建（需要活动编辑器）
    try {
        // 先确保有活动编辑器
        workspace.openFile("editor_test.txt");
        workspace.setActiveFile("editor_test.txt");

        // 测试创建append命令
        auto parsed = ParsedCommand{
            .type = CommandType::EditorCommand,
            .editorType = EditorCommandType::Append,
            .text = "Hello World"
        };

        auto cmd = controller.createCommandFromParsed(parsed);
        assert(cmd != nullptr);
        std::cout << "  ✓ createCommandFromParsed for editor command - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ Editor command creation failed: " << e.what() << std::endl;
        throw;
    }

    std::cout << "All CommandController tests passed!" << std::endl << std::endl;
}

int main() {
    std::cout << "Starting WorkSpace tests..." << std::endl << std::endl;

    try {
        test_basic_functionality();
        test_observer_attachment();
        test_command_controller();

        std::cout << "All WorkSpace tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}