#include "CommandParser.h"
#include "Command.h"
#include <iostream>
#include <cassert>
#include <memory>

void test_work_space_commands() {
    std::cout << "Testing WorkSpace commands parsing..." << std::endl;

    CommandParser parser;

    // 测试load命令
    try {
        ParsedCommand parsed = parser.parse("load test.txt");
        assert(parsed.type == CommandType::WorkSpaceCommand);
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::Load);
        assert(ws->fileName && *ws->fileName == "test.txt");
        std::cout << "  ✓ load test.txt - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ load test.txt - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试save命令（无参数）
    try {
        ParsedCommand parsed = parser.parse("save");
        assert(parsed.type == CommandType::WorkSpaceCommand);
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::Save);
        assert(!ws->target || ws->target->empty());
        std::cout << "  ✓ save - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ save - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试save命令（带文件名）
    try {
        ParsedCommand parsed = parser.parse("save output.txt");
        assert(parsed.type == CommandType::WorkSpaceCommand);
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::Save);
        assert(ws->target && *ws->target == "output.txt");
        std::cout << "  ✓ save output.txt - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ save output.txt - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试init命令
    try {
        ParsedCommand parsed = parser.parse("init newfile.txt");
        assert(parsed.type == CommandType::WorkSpaceCommand);
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::Init);
        assert(ws->fileName && *ws->fileName == "newfile.txt");
        assert(ws->withLog && *ws->withLog == false);
        std::cout << "  ✓ init newfile.txt - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ init newfile.txt - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试init命令（带with-log）
    try {
        ParsedCommand parsed = parser.parse("init newfile.txt with-log");
        assert(parsed.type == CommandType::WorkSpaceCommand);
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::Init);
        assert(ws->fileName && *ws->fileName == "newfile.txt");
        assert(ws->withLog && *ws->withLog == true);
        std::cout << "  ✓ init newfile.txt with-log - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ init newfile.txt with-log - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试close命令（无参数）
    try {
        ParsedCommand parsed = parser.parse("close");
        assert(parsed.type == CommandType::WorkSpaceCommand);
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::Close);
        assert(!ws->fileName || ws->fileName->empty());
        std::cout << "  ✓ close - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ close - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试close命令（带文件名）
    try {
        ParsedCommand parsed = parser.parse("close test.txt");
        assert(parsed.type == CommandType::WorkSpaceCommand);
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::Close);
        assert(ws->fileName && *ws->fileName == "test.txt");
        std::cout << "  ✓ close test.txt - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ close test.txt - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试edit命令
    try {
        ParsedCommand parsed = parser.parse("edit another.txt");
        assert(parsed.type == CommandType::WorkSpaceCommand);
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::Edit);
        assert(ws->fileName && *ws->fileName == "another.txt");
        std::cout << "  ✓ edit another.txt - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ edit another.txt - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试editor-list命令
    try {
        ParsedCommand parsed = parser.parse("editor-list");
        assert(parsed.type == CommandType::WorkSpaceCommand);
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::EditorList);
        std::cout << "  ✓ editor-list - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ editor-list - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试dir-tree命令（无参数）
    try {
        ParsedCommand parsed = parser.parse("dir-tree");
        assert(parsed.type == CommandType::WorkSpaceCommand);
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::DirTree);
        assert(!ws->path || ws->path->empty());
        std::cout << "  ✓ dir-tree - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ dir-tree - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试dir-tree命令（带路径）
    try {
        ParsedCommand parsed = parser.parse("dir-tree /home/user");
        assert(parsed.type == CommandType::WorkSpaceCommand);
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::DirTree);
        assert(ws->path && *ws->path == "/home/user");
        std::cout << "  ✓ dir-tree /home/user - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ dir-tree /home/user - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试undo命令
    try {
        ParsedCommand parsed = parser.parse("undo");
        assert(parsed.type == CommandType::WorkSpaceCommand);
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::Undo);
        std::cout << "  ✓ undo - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ undo - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试redo命令
    try {
        ParsedCommand parsed = parser.parse("redo");
        assert(parsed.type == CommandType::WorkSpaceCommand);
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::Redo);
        std::cout << "  ✓ redo - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ redo - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试exit命令
    try {
        ParsedCommand parsed = parser.parse("exit");
        assert(parsed.type == CommandType::WorkSpaceCommand);
        auto* ws = parsed.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::Exit);
        std::cout << "  ✓ exit - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ exit - FAILED: " << e.what() << std::endl;
        throw;
    }

    std::cout << "All WorkSpace commands tests passed!" << std::endl << std::endl;
}

void test_editor_commands_parsing() {
    std::cout << "Testing Editor commands parsing (format validation)..." << std::endl;

    CommandParser parser;

    // 测试append命令
    try {
        ParsedCommand parsed = parser.parse("append \"Hello World\"");
        assert(parsed.type == CommandType::EditorCommand);
        auto* ed = parsed.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::Append);
        assert(ed->text && *ed->text == "Hello World");
        std::cout << "  ✓ append \"Hello World\" - format OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ append \"Hello World\" - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试insert命令
    try {
        ParsedCommand parsed = parser.parse("insert 1:5 \"text to insert\"");
        assert(parsed.type == CommandType::EditorCommand);
        auto* ed = parsed.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::Insert);
        assert(ed->line && *ed->line == 1);
        assert(ed->column && *ed->column == 5);
        assert(ed->text && *ed->text == "text to insert");
        std::cout << "  ✓ insert 1:5 \"text to insert\" - format OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ insert 1:5 \"text to insert\" - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试delete命令
    try {
        ParsedCommand parsed = parser.parse("delete 2:10 5");
        assert(parsed.type == CommandType::EditorCommand);
        auto* ed = parsed.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::Delete);
        assert(ed->line && *ed->line == 2);
        assert(ed->column && *ed->column == 10);
        assert(ed->length && *ed->length == 5);
        std::cout << "  ✓ delete 2:10 5 - format OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ delete 2:10 5 - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试replace命令
    try {
        ParsedCommand parsed = parser.parse("replace 3:2 4 \"replacement\"");
        assert(parsed.type == CommandType::EditorCommand);
        auto* ed = parsed.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::Replace);
        assert(ed->line && *ed->line == 3);
        assert(ed->column && *ed->column == 2);
        assert(ed->length && *ed->length == 4);
        assert(ed->text && *ed->text == "replacement");
        std::cout << "  ✓ replace 3:2 4 \"replacement\" - format OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ replace 3:2 4 \"replacement\" - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试show命令（无参数）
    try {
        ParsedCommand parsed = parser.parse("show");
        assert(parsed.type == CommandType::EditorCommand);
        auto* ed = parsed.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::Show);
        std::cout << "  ✓ show - format OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ show - FAILED: " << e.what() << std::endl;
        throw;
    }

    // 测试show命令（带范围）
    try {
        ParsedCommand parsed = parser.parse("show 1:10");
        assert(parsed.type == CommandType::EditorCommand);
        auto* ed = parsed.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::Show);
        assert(ed->startLine && *ed->startLine == 1);
        assert(ed->endLine && *ed->endLine == 10);
        std::cout << "  ✓ show 1:10 - format OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ show 1:10 - FAILED: " << e.what() << std::endl;
        throw;
    }

    std::cout << "All Editor commands format tests passed!" << std::endl << std::endl;
}

void test_escape_sequences() {
    std::cout << "Testing escape sequence handling..." << std::endl;

    CommandParser parser;

    try {
        ParsedCommand cmd = parser.parse("append \"Line1\\nLine2\\tTab\"");
        assert(cmd.type == CommandType::EditorCommand);
        auto* ed = cmd.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::Append);
        assert(ed->text && !ed->text->empty());
        std::cout << "  ✓ append with escape sequences - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ append with escape sequences - FAILED: " << e.what() << std::endl;
        throw;
    }

    try {
        ParsedCommand cmd = parser.parse("append \"He said \\\"Hello\\\"\"");
        assert(cmd.type == CommandType::EditorCommand);
        auto* ed = cmd.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::Append);
        assert(ed->text && !ed->text->empty());
        std::cout << "  ✓ append with escaped quotes - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ append with escaped quotes - FAILED: " << e.what() << std::endl;
        throw;
    }

    try {
        ParsedCommand cmd = parser.parse("append \"Path: C:\\\\Users\\\\test\"");
        assert(cmd.type == CommandType::EditorCommand);
        auto* ed = cmd.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::Append);
        assert(ed->text && !ed->text->empty());
        std::cout << "  ✓ append with escaped backslashes - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ append with escaped backslashes - FAILED: " << e.what() << std::endl;
        throw;
    }

    std::cout << "All escape sequence tests passed!" << std::endl << std::endl;
}

void test_error_handling() {
    std::cout << "Testing error handling..." << std::endl;

    CommandParser parser;

    try {
        ParsedCommand cmd = parser.parse("unknown_command");
        std::cout << "  ✗ unknown_command - Should have thrown but didn't" << std::endl;
        assert(false);
    } catch (const UnknownCommandException& e) {
        std::cout << "  ✓ unknown_command - Correctly threw UnknownCommandException" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ unknown_command - Wrong exception: " << e.what() << std::endl;
        throw;
    }

    try {
        ParsedCommand cmd = parser.parse("insert 1:5");
        std::cout << "  ✗ insert without text - Should have thrown but didn't" << std::endl;
        assert(false);
    } catch (const CommandFormatException& e) {
        std::cout << "  ✓ insert without text - Correctly threw CommandFormatException" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ insert without text - Wrong exception: " << e.what() << std::endl;
        throw;
    }

    try {
        ParsedCommand cmd = parser.parse("insert 1-5 \"text\"");
        std::cout << "  ✗ insert with bad position format - Should have thrown but didn't" << std::endl;
        assert(false);
    } catch (const ArgumentParseException& e) {
        std::cout << "  ✓ insert with bad position format - Correctly threw ArgumentParseException" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ insert with bad position format - Wrong exception: " << e.what() << std::endl;
        throw;
    }

    try {
        ParsedCommand cmd = parser.parse("append Hello");
        std::cout << "  ✗ append without quotes - Should have thrown but didn't" << std::endl;
        assert(false);
    } catch (const ArgumentParseException& e) {
        std::cout << "  ✓ append without quotes - Correctly threw ArgumentParseException" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ append without quotes - Wrong exception: " << e.what() << std::endl;
        throw;
    }

    try {
        ParsedCommand cmd = parser.parse("init file.txt bad-param");
        std::cout << "  ✗ init with bad parameter - Should have thrown but didn't" << std::endl;
        assert(false);
    } catch (const ArgumentParseException& e) {
        std::cout << "  ✓ init with bad parameter - Correctly threw ArgumentParseException" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ init with bad parameter - Wrong exception: " << e.what() << std::endl;
        throw;
    }

    std::cout << "All error handling tests passed!" << std::endl << std::endl;
}

void test_case_insensitivity() {
    std::cout << "Testing case insensitivity..." << std::endl;

    CommandParser parser;

    try {
        ParsedCommand cmd = parser.parse("LOAD test.txt");
        assert(cmd.type == CommandType::WorkSpaceCommand);
        auto* ws = cmd.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::Load);
        assert(ws->fileName && *ws->fileName == "test.txt");
        std::cout << "  ✓ LOAD test.txt - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ LOAD test.txt - FAILED: " << e.what() << std::endl;
        throw;
    }

    try {
        ParsedCommand cmd = parser.parse("EdIt file.txt");
        assert(cmd.type == CommandType::WorkSpaceCommand);
        auto* ws = cmd.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::Edit);
        assert(ws->fileName && *ws->fileName == "file.txt");
        std::cout << "  ✓ EdIt file.txt - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ EdIt file.txt - FAILED: " << e.what() << std::endl;
        throw;
    }

    try {
        ParsedCommand cmd = parser.parse("INIT file.TXT WITH-LOG");
        assert(cmd.type == CommandType::WorkSpaceCommand);
        auto* ws = cmd.asWorkSpace();
        assert(ws != nullptr);
        assert(ws->workSpaceType == WorkSpaceCommandType::Init);
        assert(ws->fileName && *ws->fileName == "file.TXT");
        assert(ws->withLog && *ws->withLog == true);
        std::cout << "  ✓ INIT file.TXT WITH-LOG - OK" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "  ✗ INIT file.TXT WITH-LOG - FAILED: " << e.what() << std::endl;
        throw;
    }

    std::cout << "All case insensitivity tests passed!" << std::endl << std::endl;
}

int main() {
    std::cout << "Starting CommandParser tests..." << std::endl << std::endl;

    try {
        test_work_space_commands();
        test_editor_commands_parsing();
        test_escape_sequences();
        test_error_handling();
        test_case_insensitivity();

        std::cout << "All CommandParser tests passed successfully!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    }
}
