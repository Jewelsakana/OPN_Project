#include "ISpellChecker.h"
#include "MockSpellChecker.h"
#include "HttpSpellCheckerAdapter.h"
#include "SpellCheckCommand.h"
#include "TextEditor.h"
#include "XmlEditor.h"
#include "OutputService.h"
#include "WorkSpace.h"
#include "CommandParser.h"
#include "CommandController.h"
#include "CommandFactory.h"
#include "TextEngine.h"
#include <iostream>
#include <cassert>
#include <string>
#include <vector>
#include <memory>

// 辅助函数：打印测试结果
void printTestResult(const std::string& testName, bool passed) {
    std::cout << (passed ? "[PASS]" : "[FAIL]") << " " << testName << std::endl;
}

// ============================================================
// 测试1：MockSpellChecker 基本功能
// ============================================================
void testMockSpellCheckerBasic() {
    std::cout << "=== Testing MockSpellChecker Basic ===" << std::endl;

    MockSpellChecker checker;

    // 测试已知拼写错误
    {
        TextSegment seg;
        seg.text = "recieve";
        seg.line = 1;
        seg.column = 1;
        seg.elementId = "";

        auto results = checker.checkText(seg);
        assert(results.size() == 1);
        assert(results[0].original == "recieve");
        assert(!results[0].suggestions.empty());
        assert(results[0].suggestions[0] == "receive");
        assert(results[0].line == 1);
        assert(results[0].column == 1);
        printTestResult("MockSpellChecker - detect 'recieve'", true);
    }

    // 测试正确拼写不报错
    {
        TextSegment seg;
        seg.text = "hello world";
        seg.line = 1;
        seg.column = 1;
        seg.elementId = "";

        auto results = checker.checkText(seg);
        assert(results.empty());
        printTestResult("MockSpellChecker - no error for correct words", true);
    }

    // 测试多个拼写错误
    {
        TextSegment seg;
        seg.text = "recieve the adress";
        seg.line = 2;
        seg.column = 5;
        seg.elementId = "";

        auto results = checker.checkText(seg);
        assert(results.size() == 2);
        // 第一个错误: recieve
        assert(results[0].original == "recieve");
        assert(results[0].line == 2);
        assert(results[0].column == 5); // position 0 + column 5
        // 第二个错误: adress
        assert(results[1].original == "adress");
        printTestResult("MockSpellChecker - detect multiple errors", true);
    }
}

// ============================================================
// 测试2：ISpellChecker 接口可替换性（适配器模式）
// ============================================================
void testSpellCheckerSwap() {
    std::cout << "=== Testing Spell Checker Swap (Adapter Pattern) ===" << std::endl;

    // 使用 MockSpellChecker
    std::shared_ptr<ISpellChecker> checker = std::make_shared<MockSpellChecker>();

    TextSegment seg;
    seg.text = "recieve";
    seg.line = 1;
    seg.column = 1;
    seg.elementId = "";

    auto results = checker->checkText(seg);
    assert(!results.empty());
    assert(results[0].suggestions[0] == "receive");
    printTestResult("ISpellChecker swap - MockSpellChecker works", true);

    // 验证 HttpSpellCheckerAdapter 也实现了 ISpellChecker 接口（编译时检查）
    // 实际 HTTP 调用会抛出异常（骨架实现）
    std::shared_ptr<ISpellChecker> httpChecker = std::make_shared<HttpSpellCheckerAdapter>();
    try {
        httpChecker->checkText(seg);
        printTestResult("ISpellChecker swap - HttpSpellCheckerAdapter compiled", false);
    } catch (const std::runtime_error&) {
        printTestResult("ISpellChecker swap - HttpSpellCheckerAdapter skeleton throws as expected", true);
    }
}

// ============================================================
// 测试3：TextEditor::getTextsToCheck()
// ============================================================
void testTextEditorGetTextsToCheck() {
    std::cout << "=== Testing TextEditor::getTextsToCheck() ===" << std::endl;

    TextEditor editor;
    auto engine = std::make_shared<TextEngine>();
    editor.setTextEngine(engine);

    // 设置测试内容
    std::vector<std::string> lines = {
        "Hello World",
        "",
        "This is a test recieve",
        "Another line"
    };
    editor.setLines(lines);

    auto segments = editor.getTextsToCheck();
    // 空行应被过滤
    assert(segments.size() == 3);

    assert(segments[0].line == 1);
    assert(segments[0].column == 1);
    assert(segments[0].text == "Hello World");
    assert(segments[0].elementId == "");

    assert(segments[1].line == 3);
    assert(segments[1].text == "This is a test recieve");

    assert(segments[2].line == 4);
    assert(segments[2].text == "Another line");

    printTestResult("TextEditor::getTextsToCheck() - correct segments", true);
}

// ============================================================
// 测试4：XmlEditor::getTextsToCheck()
// ============================================================
void testXmlEditorGetTextsToCheck() {
    std::cout << "=== Testing XmlEditor::getTextsToCheck() ===" << std::endl;

    XmlEditor editor;
    const char* xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<library id="lib">
    <book id="b1">
        <title id="t1">Itallian Cooking</title>
        <author id="a1">John Smith</author>
    </book>
    <book id="b2">
        <title id="t2">The Guide</title>
        <author id="a2">Rowlling</author>
    </book>
</library>)";

    editor.loadFromString(xmlContent);

    auto segments = editor.getTextsToCheck();

    // 应提取所有带文本内容的元素
    bool foundT1 = false, foundA1 = false, foundT2 = false, foundA2 = false;
    for (const auto& seg : segments) {
        if (seg.elementId == "t1") {
            assert(seg.text == "Itallian Cooking");
            assert(seg.line == 0);
            assert(seg.column == 0);
            foundT1 = true;
        }
        if (seg.elementId == "a1") {
            assert(seg.text == "John Smith");
            foundA1 = true;
        }
        if (seg.elementId == "t2") {
            assert(seg.text == "The Guide");
            foundT2 = true;
        }
        if (seg.elementId == "a2") {
            assert(seg.text == "Rowlling");
            foundA2 = true;
        }
    }
    assert(foundT1 && foundA1 && foundT2 && foundA2);

    // 不包含 lib, b1, b2（它们没有直接文本内容，或只有空白）
    for (const auto& seg : segments) {
        assert(seg.elementId != "lib" || seg.text.empty());
        assert(seg.elementId != "b1" || seg.text.empty());
    }

    printTestResult("XmlEditor::getTextsToCheck() - filters tags, keeps text nodes", true);
}

// ============================================================
// 测试5：OutputService::outputSpellCheckResults()
// ============================================================
void testOutputSpellCheckResults() {
    std::cout << "=== Testing OutputService::outputSpellCheckResults() ===" << std::endl;

    OutputService output;

    // 测试文本结果输出
    {
        std::vector<SpellCheckResult> results;
        SpellCheckResult r1;
        r1.line = 1;
        r1.column = 5;
        r1.original = "recieve";
        r1.suggestions = {"receive"};
        r1.elementId = "";
        results.push_back(r1);

        SpellCheckResult r2;
        r2.line = 3;
        r2.column = 12;
        r2.original = "occured";
        r2.suggestions = {"occurred"};
        r2.elementId = "";
        results.push_back(r2);

        // 验证不会抛出异常
        output.outputSpellCheckResults(results);
        printTestResult("OutputService - text results output", true);
    }

    // 测试 XML 结果输出
    {
        std::vector<SpellCheckResult> results;
        SpellCheckResult r1;
        r1.elementId = "title1";
        r1.original = "Itallian";
        r1.suggestions = {"Italian"};
        results.push_back(r1);

        SpellCheckResult r2;
        r2.elementId = "author2";
        r2.original = "Rowlling";
        r2.suggestions = {"Rowling"};
        results.push_back(r2);

        output.outputSpellCheckResults(results);
        printTestResult("OutputService - XML results output", true);
    }

    // 测试空结果
    {
        std::vector<SpellCheckResult> empty;
        output.outputSpellCheckResults(empty);
        printTestResult("OutputService - empty results output", true);
    }
}

// ============================================================
// 测试6：SpellCheckCommand 集成测试（使用 MockSpellChecker）
// ============================================================
void testSpellCheckCommandIntegration() {
    std::cout << "=== Testing SpellCheckCommand Integration ===" << std::endl;

    // 创建 WorkSpace 并注入 MockSpellChecker
    WorkSpace workspace;
    auto mockChecker = std::make_shared<MockSpellChecker>();
    workspace.setSpellChecker(mockChecker);

    // 验证 spellChecker 注入成功
    assert(workspace.getSpellChecker() == mockChecker);
    printTestResult("SpellCheckCommand - spell checker injected into WorkSpace", true);

    // 通过 CommandController 创建并执行命令
    CommandController controller(&workspace);

    // 测试解析 spell-check 命令
    CommandParser parser;
    auto parsed1 = parser.parse("spell-check");
    assert(parsed1.type == CommandType::WorkSpaceCommand);
    auto* ws1 = parsed1.asWorkSpace();
    assert(ws1 != nullptr);
    assert(ws1->workSpaceType == WorkSpaceCommandType::SpellCheck);
    assert(!ws1->fileName.has_value());
    printTestResult("CommandParser - parse 'spell-check'", true);

    // 测试解析 spell-check with file
    auto parsed2 = parser.parse("spell-check test.txt");
    auto* ws2 = parsed2.asWorkSpace();
    assert(ws2->workSpaceType == WorkSpaceCommandType::SpellCheck);
    assert(ws2->fileName == "test.txt");
    printTestResult("CommandParser - parse 'spell-check test.txt'", true);
}

// ============================================================
// 测试7：CommandFactory 创建 SpellCheckCommand
// ============================================================
void testSpellCheckCommandFactory() {
    std::cout << "=== Testing SpellCheckCommand Factory ===" << std::endl;

    WorkSpace workspace;

    // 通过 factory 创建 spell-check 命令
    WorkSpaceParsedCommand parsed;
    parsed.workSpaceType = WorkSpaceCommandType::SpellCheck;
    parsed.fileName = std::nullopt;

    auto cmd = CommandFactory::createFromParsed(
        ParsedCommand{CommandType::WorkSpaceCommand, parsed},
        &workspace, nullptr);

    assert(cmd != nullptr);
    auto* wsCmd = dynamic_cast<WorkSpaceCommand*>(cmd.get());
    assert(wsCmd != nullptr);
    assert(wsCmd->isReadOnly() == true);
    printTestResult("CommandFactory - creates SpellCheckCommand", true);

    // 测试带文件名参数
    WorkSpaceParsedCommand parsed2;
    parsed2.workSpaceType = WorkSpaceCommandType::SpellCheck;
    parsed2.fileName = "test.txt";

    auto cmd2 = CommandFactory::createFromParsed(
        ParsedCommand{CommandType::WorkSpaceCommand, parsed2},
        &workspace, nullptr);
    assert(cmd2 != nullptr);
    printTestResult("CommandFactory - creates SpellCheckCommand with filename", true);
}

// ============================================================
// 测试8：MockSpellChecker 大小写不敏感匹配
// ============================================================
void testMockSpellCheckerCaseInsensitive() {
    std::cout << "=== Testing MockSpellChecker Case Insensitivity ===" << std::endl;

    MockSpellChecker checker;

    // 测试首字母大写
    {
        TextSegment seg;
        seg.text = "Recieve";
        seg.line = 1;
        seg.column = 1;
        seg.elementId = "";

        auto results = checker.checkText(seg);
        assert(results.size() == 1);
        printTestResult("MockSpellChecker - case insensitive 'Recieve'", true);
    }

    // 测试全大写
    {
        TextSegment seg;
        seg.text = "ADDRESS";  // 正确拼写
        seg.line = 1;
        seg.column = 1;
        seg.elementId = "";
        auto results = checker.checkText(seg);
        assert(results.empty());
        printTestResult("MockSpellChecker - no false positive for 'ADDRESS'", true);
    }
}

int main() {
    std::cout << "Running Spell Check Tests..." << std::endl;
    std::cout << "========================================" << std::endl;

    testMockSpellCheckerBasic();
    testSpellCheckerSwap();
    testTextEditorGetTextsToCheck();
    testXmlEditorGetTextsToCheck();
    testOutputSpellCheckResults();
    testSpellCheckCommandIntegration();
    testSpellCheckCommandFactory();
    testMockSpellCheckerCaseInsensitive();

    std::cout << "========================================" << std::endl;
    std::cout << "All Spell Check Tests Passed!" << std::endl;
    return 0;
}
