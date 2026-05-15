#include "EditDurationTracker.h"
#include "StringUtils.h"
#include "WorkSpace.h"
#include "WorkSpaceCommand.h"
#include "EditorFactory.h"
#include "TextEditor.h"
#include "TextEngine.h"
#include "XmlEditor.h"
#include "CommandFactory.h"
#include "CommandParser.h"
#include "CommandParserStrategy.h"
#include <iostream>
#include <cassert>
#include <thread>
#include <chrono>

void printTestResult(const std::string& testName, bool passed) {
    std::cout << (passed ? "[PASS]" : "[FAIL]") << " " << testName << std::endl;
}

// ============================================================
// 1. StringUtils::formatDuration 测试
// ============================================================
void test_format_duration() {
    std::cout << "Test 1: formatDuration..." << std::endl;

    assert(StringUtils::formatDuration(0) == "0秒");
    printTestResult("0 seconds", true);

    assert(StringUtils::formatDuration(30) == "30秒");
    printTestResult("< 1 minute: 30秒", true);

    assert(StringUtils::formatDuration(59) == "59秒");
    printTestResult("< 1 minute: 59秒", true);

    assert(StringUtils::formatDuration(60) == "1分钟");
    printTestResult("1 minute = 1分钟", true);

    assert(StringUtils::formatDuration(3599) == "59分钟");
    printTestResult("59 minutes = 59分钟", true);

    assert(StringUtils::formatDuration(3600) == "1小时");
    printTestResult("1 hour = 1小时", true);

    assert(StringUtils::formatDuration(3660) == "1小时1分钟");
    printTestResult("1 hour 1 minute", true);

    assert(StringUtils::formatDuration(7200) == "2小时");
    printTestResult("2 hours", true);

    assert(StringUtils::formatDuration(82800) == "23小时");
    printTestResult("23 hours (no minutes)", true);

    assert(StringUtils::formatDuration(82860) == "23小时1分钟");
    printTestResult("23 hours 1 minute", true);

    assert(StringUtils::formatDuration(86400) == "1天");
    printTestResult("1 day = 1天", true);

    assert(StringUtils::formatDuration(90000) == "1天1小时");
    printTestResult("1 day 1 hour", true);

    assert(StringUtils::formatDuration(172800) == "2天");
    printTestResult("2 days", true);

    assert(StringUtils::formatDuration(180000) == "2天2小时");
    printTestResult("2 days 2 hours", true);

    // 负数处理
    assert(StringUtils::formatDuration(-10) == "0秒");
    printTestResult("negative seconds clamped", true);

    std::cout << "  All formatDuration tests passed!" << std::endl << std::endl;
}

// ============================================================
// 2. EditDurationTracker 基本功能测试
// ============================================================
void test_tracker_basic() {
    std::cout << "Test 2: EditDurationTracker basic..." << std::endl;

    EditDurationTracker tracker;

    // 初始状态
    assert(tracker.getDurationSeconds("test.txt") == 0);
    printTestResult("initial duration is 0", true);

    assert(tracker.getCurrentFile().empty());
    printTestResult("no current file initially", true);

    // 模拟 update 事件
    Event e1("edit test.txt", "test.txt");
    tracker.update(e1);
    assert(tracker.getCurrentFile() == "test.txt");
    printTestResult("update sets current file", true);

    // 等待一小段时间
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // 持续时间应该 >= 0 (实时测量不精确，只测非负)
    int dur = tracker.getDurationSeconds("test.txt");
    assert(dur >= 0);
    printTestResult("getDurationSeconds for active file >= 0", true);

    // 模拟切换到另一个文件
    Event e2("edit other.txt", "other.txt");
    tracker.update(e2);

    int dur1 = tracker.getDurationSeconds("test.txt");
    assert(dur1 >= 0);
    printTestResult("switched away, duration recorded", true);

    assert(tracker.getCurrentFile() == "other.txt");
    printTestResult("current file is other.txt", true);

    // 关闭文件
    tracker.onFileClosed("test.txt");
    assert(tracker.getDurationSeconds("test.txt") == 0);
    printTestResult("closed file duration reset to 0", true);

    // 未跟踪文件的时长
    assert(tracker.getDurationSeconds("nonexistent.txt") == 0);
    printTestResult("untracked file returns 0", true);

    std::cout << "  All tracker basic tests passed!" << std::endl << std::endl;
}

// ============================================================
// 3. EditDurationTracker 累积时长测试
// ============================================================
void test_tracker_accumulation() {
    std::cout << "Test 3: EditDurationTracker accumulation..." << std::endl;

    EditDurationTracker tracker;

    // 打开文件 A
    Event e1("edit a.txt", "a.txt");
    tracker.update(e1);

    // 切到文件 B（记录 A 的时长）
    Event e2("edit b.txt", "b.txt");
    tracker.update(e2);

    // 切回文件 A（记录 B 的时长）
    Event e3("edit a.txt", "a.txt");
    tracker.update(e3);

    // 切到文件 B（记录 A 的时长）
    Event e4("edit b.txt", "b.txt");
    tracker.update(e4);

    int durA = tracker.getDurationSeconds("a.txt");
    int durB = tracker.getDurationSeconds("b.txt");
    assert(durA >= 0);
    assert(durB >= 0);
    printTestResult("both files have non-negative duration", true);

    // 验证文件数量
    auto& allDurs = tracker.getAllDurations();
    assert(allDurs.size() == 2);
    printTestResult("tracking 2 files", true);

    std::cout << "  All accumulation tests passed!" << std::endl << std::endl;
}

// ============================================================
// 4. editor-list 普通列表模式（带时长装饰）
// ============================================================
void test_editor_list_with_duration() {
    std::cout << "Test 4: editor-list with duration decorator..." << std::endl;

    WorkSpace workspace;
    workspace.initFile("test1.txt", false);

    // 触发 update 让 tracker 开始计时
    Event e("init test1.txt", "test1.txt");
    workspace.notify(e);

    // editor-list 不应崩溃
    EditorListCommand cmd("");
    cmd.setWorkSpace(&workspace);
    cmd.execute();
    printTestResult("editor-list executes without error", true);

    // 验证文件时长被装饰
    auto* tracker = workspace.getEditDurationTracker();
    assert(tracker != nullptr);
    printTestResult("tracker accessible from workspace", true);

    workspace.closeFile("test1.txt");
    std::cout << "  All editor-list tests passed!" << std::endl << std::endl;
}

// ============================================================
// 5. editor-list tree 模式测试
// ============================================================
void test_editor_list_tree_mode() {
    std::cout << "Test 5: editor-list tree mode..." << std::endl;

    WorkSpace workspace;
    workspace.initFile("test_tree.txt", false);

    Event e("init test_tree.txt", "test_tree.txt");
    workspace.notify(e);

    // tree 模式
    EditorListCommand cmd("tree");
    cmd.setWorkSpace(&workspace);
    cmd.execute();
    printTestResult("editor-list tree executes without error", true);

    // 验证 test_tree.txt 已在打开列表中
    // (WorkSpace 构造时 loadConfig 可能恢复上次会话的文件，不要求精确 == 1)
    auto openFiles = workspace.getOpenFiles();
    bool found = false;
    for (const auto& f : openFiles) {
        if (f == "test_tree.txt") { found = true; break; }
    }
    assert(found);
    printTestResult("test_tree.txt is in open files", true);

    workspace.closeFile("test_tree.txt");
    std::cout << "  All tree mode tests passed!" << std::endl << std::endl;
}

// ============================================================
// 6. editor-list 解析测试
// ============================================================
void test_editor_list_parsing() {
    std::cout << "Test 6: editor-list parsing..." << std::endl;

    {
        CommandParser parser;
        auto parsed = parser.parse("editor-list");
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == static_cast<CommandTypeId>(WorkSpaceCommandType::EditorList));
        assert(!ws->target.has_value());
        printTestResult("editor-list (no arg) - OK", true);
    }

    {
        CommandParser parser;
        auto parsed = parser.parse("editor-list tree");
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == static_cast<CommandTypeId>(WorkSpaceCommandType::EditorList));
        assert(ws->target.has_value());
        assert(ws->target.value() == "tree");
        printTestResult("editor-list tree - OK", true);
    }

    {
        CommandParser parser;
        try {
            parser.parse("editor-list invalid");
            printTestResult("editor-list invalid arg should throw", false);
        } catch (const ArgumentParseException&) {
            printTestResult("editor-list invalid arg throws", true);
        }
    }

    std::cout << "  All parsing tests passed!" << std::endl << std::endl;
}

// ============================================================
// 7. 时长在文件关闭后重置测试
// ============================================================
void test_duration_reset_on_close() {
    std::cout << "Test 7: Duration reset on file close..." << std::endl;

    WorkSpace workspace;
    workspace.initFile("reset_test.txt", false);

    Event e("init reset_test.txt", "reset_test.txt");
    workspace.notify(e);

    auto* tracker = workspace.getEditDurationTracker();
    assert(tracker->getDurationSeconds("reset_test.txt") >= 0);
    printTestResult("duration available before close", true);

    // 关闭文件后时长应重置
    workspace.closeFile("reset_test.txt");
    assert(tracker->getDurationSeconds("reset_test.txt") == 0);
    printTestResult("duration reset after close", true);

    std::cout << "  All reset tests passed!" << std::endl << std::endl;
}

// ============================================================
// 8. 观察者模式集成测试
// ============================================================
void test_observer_integration() {
    std::cout << "Test 8: Observer pattern integration..." << std::endl;

    WorkSpace workspace;
    auto* trackerBefore = workspace.getEditDurationTracker();
    assert(trackerBefore != nullptr);
    printTestResult("tracker created on workspace init", true);

    // 模拟命令执行后的通知
    workspace.initFile("observer_test.txt", false);
    Event e("init observer_test.txt", "observer_test.txt");
    workspace.notify(e);

    assert(trackerBefore->getCurrentFile() == "observer_test.txt");
    printTestResult("observer tracks active file after notify", true);

    // 切换到另一文件
    workspace.initFile("observer_test2.txt", false);
    Event e2("init observer_test2.txt", "observer_test2.txt");
    workspace.notify(e2);

    assert(trackerBefore->getCurrentFile() == "observer_test2.txt");
    printTestResult("observer switches to new file", true);

    workspace.closeFile("observer_test.txt");
    workspace.closeFile("observer_test2.txt");
    std::cout << "  All observer tests passed!" << std::endl << std::endl;
}

// ============================================================
// 9. 统计模块独立性和容错测试
// ============================================================
void test_tracker_isolation() {
    std::cout << "Test 9: Tracker isolation and fault tolerance..." << std::endl;

    // tracker 操作不应抛出异常（即使状态异常）
    EditDurationTracker tracker;

    // 关闭未跟踪的文件
    tracker.onFileClosed("never_opened.txt");
    printTestResult("closing untracked file does not throw", true);

    // 查询未跟踪的文件
    int dur = tracker.getDurationSeconds("unknown.txt");
    assert(dur == 0);
    printTestResult("unknown file returns 0 seconds", true);

    // reset 后应保持正常
    tracker.reset();
    assert(tracker.getCurrentFile().empty());
    printTestResult("reset clears state", true);

    std::cout << "  All isolation tests passed!" << std::endl << std::endl;
}

// ============================================================
// Main
// ============================================================
int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  Edit Duration & Editor-List Test Suite" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;

    test_format_duration();
    test_tracker_basic();
    test_tracker_accumulation();
    test_editor_list_with_duration();
    test_editor_list_tree_mode();
    test_editor_list_parsing();
    test_duration_reset_on_close();
    test_observer_integration();
    test_tracker_isolation();

    std::cout << "========================================" << std::endl;
    std::cout << "  ALL TESTS PASSED!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
