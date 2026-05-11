#include "XmlEditor.h"
#include "XmlDocumentWrapper.h"
#include "IXmlDocument.h"
#include "TextEditor.h"
#include "TextEngine.h"
#include "CommandFactory.h"
#include "CommandParser.h"
#include "CommandParserStrategy.h"
#include "OutputService.h"
#include "WorkSpace.h"
#include "XMLCommand.h"
#include "Editor.h"
#include "EditorFactory.h"
#include <iostream>
#include <cassert>
#include <string>
#include <cstdio>
#include <fstream>
#include <sstream>

void printTestResult(const std::string& testName, bool passed) {
    std::cout << (passed ? "[PASS]" : "[FAIL]") << " " << testName << std::endl;
}

// ============================================================
// 1. XmlTree 命令解析测试
// ============================================================
void test_xml_tree_parsing() {
    std::cout << "Test 1: XmlTree command parsing..." << std::endl;

    {
        CommandParser parser;
        auto parsed = parser.parse("xml-tree");
        auto* ed = parsed.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::XmlTree);
        printTestResult("xml-tree without file parameter", true);
    }

    {
        CommandParser parser;
        auto parsed = parser.parse("xml-tree test.xml");
        auto* ed = parsed.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::XmlTree);
        assert(ed->text.has_value());
        assert(ed->text.value() == "test.xml");
        printTestResult("xml-tree with file parameter", true);
    }

    std::cout << "  All XmlTree parsing tests passed!" << std::endl << std::endl;
}

// ============================================================
// 2. XmlTree 命令执行测试
// ============================================================
void test_xml_tree_execution() {
    std::cout << "Test 2: XmlTree command execution..." << std::endl;

    const std::string xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<bookstore id="root">
    <book id="book1" category="COOKING">
        <title id="title1" lang="en">Everyday Italian</title>
    </book>
</bookstore>)";

    XmlEditor editor;
    editor.loadFromString(xmlContent);

    OutputService outputService;
    XmlTreeCommand cmd(&editor.getDocument(), &outputService);

    // isReadOnly should be true
    assert(cmd.isReadOnly());
    printTestResult("XmlTreeCommand is read-only", true);

    // execute should not throw
    bool threw = false;
    try {
        cmd.execute();
    } catch (const std::exception& e) {
        threw = true;
        std::cerr << "Unexpected exception: " << e.what() << std::endl;
    }
    assert(!threw);
    printTestResult("XmlTreeCommand executes without error", true);

    // undo should be a no-op
    cmd.undo();
    printTestResult("XmlTreeCommand undo is no-op", true);

    std::cout << "  All XmlTree execution tests passed!" << std::endl << std::endl;
}

// ============================================================
// 3. IXmlDocument 遍历方法测试
// ============================================================
void test_xml_traversal_methods() {
    std::cout << "Test 3: IXmlDocument traversal methods..." << std::endl;

    const std::string xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<root id="root">
    <child1 id="c1">
        <grandchild id="gc1">text1</grandchild>
    </child1>
    <child2 id="c2">text2</child2>
</root>)";

    auto doc = std::make_unique<XmlDocumentWrapper>();
    doc->loadFromString(xmlContent);
    doc->collectIds();

    // getRootId
    assert(doc->getRootId() == "root");
    printTestResult("getRootId returns 'root'", true);

    // getChildIds for root
    auto rootChildren = doc->getChildIds("root");
    assert(rootChildren.size() == 2);
    assert(rootChildren[0] == "c1");
    assert(rootChildren[1] == "c2");
    printTestResult("getChildIds for root returns 2 children", true);

    // getChildIds for child1
    auto c1Children = doc->getChildIds("c1");
    assert(c1Children.size() == 1);
    assert(c1Children[0] == "gc1");
    printTestResult("getChildIds for c1 returns 1 grandchild", true);

    // getChildIds for leaf node
    auto gc1Children = doc->getChildIds("gc1");
    assert(gc1Children.size() == 0);
    printTestResult("getChildIds for leaf returns empty", true);

    // getChildIds for non-existent node
    auto nonexistent = doc->getChildIds("nonexistent");
    assert(nonexistent.empty());
    printTestResult("getChildIds for non-existent returns empty", true);

    // getNodeAttributes
    auto rootAttrs = doc->getNodeAttributes("root");
    assert(rootAttrs.size() == 1);
    assert(rootAttrs[0].first == "id");
    assert(rootAttrs[0].second == "root");
    printTestResult("getNodeAttributes for root returns id attribute", true);

    // getNodeAttributes for non-existent
    auto noAttrs = doc->getNodeAttributes("nonexistent");
    assert(noAttrs.empty());
    printTestResult("getNodeAttributes for non-existent returns empty", true);

    std::cout << "  All traversal method tests passed!" << std::endl << std::endl;
}

// ============================================================
// 4. OutputService::outputXmlTree 测试
// ============================================================
void test_output_xml_tree() {
    std::cout << "Test 4: OutputService::outputXmlTree..." << std::endl;

    const std::string xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<bookstore id="root">
    <book id="book1" category="COOKING">
        <title id="title1" lang="en">Everyday Italian</title>
    </book>
</bookstore>)";

    auto doc = std::make_unique<XmlDocumentWrapper>();
    doc->loadFromString(xmlContent);
    doc->collectIds();

    OutputService outputService;
    // Capture stdout to verify output
    // Since we can't easily capture cout, just test that it doesn't throw
    bool threw = false;
    try {
        outputService.outputXmlTree(*doc);
    } catch (const std::exception& e) {
        threw = true;
        std::cerr << "Unexpected: " << e.what() << std::endl;
    }
    assert(!threw);
    printTestResult("outputXmlTree does not throw", true);

    // Test with empty document
    auto emptyDoc = std::make_unique<XmlDocumentWrapper>();
    bool emptyThrew = false;
    try {
        outputService.outputXmlTree(*emptyDoc);
    } catch (const std::exception& e) {
        emptyThrew = true;
    }
    assert(!emptyThrew);
    printTestResult("outputXmlTree with empty doc does not throw", true);

    std::cout << "  All outputXmlTree tests passed!" << std::endl << std::endl;
}

// ============================================================
// 5. supportsCommand 测试 - TextEditor
// ============================================================
void test_supports_command_text_editor() {
    std::cout << "Test 5: TextEditor::supportsCommand..." << std::endl;

    TextEditor editor;

    // Text commands should be supported
    assert(editor.supportsCommand(EditorCommandType::Append));
    assert(editor.supportsCommand(EditorCommandType::Insert));
    assert(editor.supportsCommand(EditorCommandType::Delete));
    assert(editor.supportsCommand(EditorCommandType::Replace));
    assert(editor.supportsCommand(EditorCommandType::Show));
    printTestResult("TextEditor supports all text commands", true);

    // XML commands should NOT be supported
    assert(!editor.supportsCommand(EditorCommandType::InsertBefore));
    assert(!editor.supportsCommand(EditorCommandType::AppendChild));
    assert(!editor.supportsCommand(EditorCommandType::EditId));
    assert(!editor.supportsCommand(EditorCommandType::EditText_));
    assert(!editor.supportsCommand(EditorCommandType::XmlDelete));
    assert(!editor.supportsCommand(EditorCommandType::XmlTree));
    printTestResult("TextEditor rejects all XML commands", true);

    std::cout << "  All TextEditor supportsCommand tests passed!" << std::endl << std::endl;
}

// ============================================================
// 6. supportsCommand 测试 - XmlEditor
// ============================================================
void test_supports_command_xml_editor() {
    std::cout << "Test 6: XmlEditor::supportsCommand..." << std::endl;

    XmlEditor editor;

    // XML commands should be supported
    assert(editor.supportsCommand(EditorCommandType::InsertBefore));
    assert(editor.supportsCommand(EditorCommandType::AppendChild));
    assert(editor.supportsCommand(EditorCommandType::EditId));
    assert(editor.supportsCommand(EditorCommandType::EditText_));
    assert(editor.supportsCommand(EditorCommandType::XmlDelete));
    assert(editor.supportsCommand(EditorCommandType::XmlTree));
    printTestResult("XmlEditor supports all XML commands", true);

    // Text commands should NOT be supported
    assert(!editor.supportsCommand(EditorCommandType::Append));
    assert(!editor.supportsCommand(EditorCommandType::Insert));
    assert(!editor.supportsCommand(EditorCommandType::Delete));
    assert(!editor.supportsCommand(EditorCommandType::Replace));
    assert(!editor.supportsCommand(EditorCommandType::Show));
    printTestResult("XmlEditor rejects all text commands", true);

    std::cout << "  All XmlEditor supportsCommand tests passed!" << std::endl << std::endl;
}

// ============================================================
// 7. init 命令 - 创建 XML 文件测试
// ============================================================
void test_init_xml_file() {
    std::cout << "Test 7: init command creates XML file..." << std::endl;

    WorkSpace workspace;

    // 初始化XML文件
    workspace.initFile("test_init.xml", false);

    auto editor = workspace.getEditor("test_init.xml");
    assert(editor != nullptr);

    auto xmlEditor = dynamic_cast<XmlEditor*>(editor.get());
    assert(xmlEditor != nullptr);
    printTestResult("init creates XmlEditor for .xml file", true);

    auto& doc = xmlEditor->getDocument();
    assert(doc.isLoaded());
    printTestResult("XML document is loaded after init", true);

    // 验证根节点存在且 id="root"
    assert(doc.hasNodeWithId("root"));
    assert(doc.getNodeName("root") == "root");
    printTestResult("Root element 'root' with id='root' exists", true);

    // 验证修改状态为true（新创建的文件）
    assert(workspace.isFileModified("test_init.xml"));
    printTestResult("Newly init'd file is marked as modified", true);

    // 清理
    std::remove("test_init.xml");

    std::cout << "  All init XML tests passed!" << std::endl << std::endl;
}

// ============================================================
// 8. init 命令 - 创建文本文件测试
// ============================================================
void test_init_txt_file() {
    std::cout << "Test 8: init command still creates text files..." << std::endl;

    WorkSpace workspace;

    workspace.initFile("test_init.txt", false);

    auto editor = workspace.getEditor("test_init.txt");
    assert(editor != nullptr);

    auto textEditor = dynamic_cast<TextEditor*>(editor.get());
    assert(textEditor != nullptr);
    printTestResult("init creates TextEditor for .txt file", true);

    // 验证初始内容为空
    const auto& lines = textEditor->getLines();
    assert(lines.size() == 1 && lines[0] == "");
    printTestResult("Init'd text file has one empty line", true);

    assert(workspace.isFileModified("test_init.txt"));
    printTestResult("Newly init'd text file is marked as modified", true);

    std::remove("test_init.txt");

    std::cout << "  All init text tests passed!" << std::endl << std::endl;
}

// ============================================================
// 9. save/load XML 文件测试
// ============================================================
void test_save_load_xml() {
    std::cout << "Test 9: save and load XML files..." << std::endl;

    const std::string testFile = "test_save_load.xml";

    // 创建并初始化XML编辑器
    {
        XmlEditor editor;
        editor.loadFromString(
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n"
            "<root id=\"root\">\n"
            "  <item id=\"item1\">Hello World</item>\n"
            "</root>\n"
        );
        editor.saveToFile(testFile);
        printTestResult("XML file saved successfully", true);
    }

    // 加载并验证
    {
        XmlEditor editor;
        editor.loadFromFile(testFile);

        auto& doc = editor.getDocument();
        assert(doc.isLoaded());
        assert(doc.hasNodeWithId("root"));
        assert(doc.hasNodeWithId("item1"));
        assert(doc.getNodeValue("item1") == "Hello World");
        printTestResult("XML file loaded and content verified", true);
    }

    // 使用WorkSpace load/save
    {
        WorkSpace workspace;
        workspace.loadFile(testFile);

        auto editor = workspace.getEditor(testFile);
        assert(editor != nullptr);

        auto xmlEditor = dynamic_cast<XmlEditor*>(editor.get());
        assert(xmlEditor != nullptr);
        printTestResult("WorkSpace loads .xml file as XmlEditor", true);

        assert(xmlEditor->getDocument().hasNodeWithId("root"));
        printTestResult("Loaded XML via WorkSpace has correct content", true);

        // 保存
        workspace.saveFile(testFile);
        printTestResult("WorkSpace saves XML file successfully", true);
    }

    // 清理
    std::remove(testFile.c_str());

    std::cout << "  All save/load XML tests passed!" << std::endl << std::endl;
}

// ============================================================
// 10. 命令验证 - 不兼容命令应被拒绝
// ============================================================
void test_command_validation() {
    std::cout << "Test 10: Command validation for incompatible commands..." << std::endl;

    // XmlEditor rejects text commands
    {
        XmlEditor editor;
        assert(!editor.supportsCommand(EditorCommandType::Append));
        assert(!editor.supportsCommand(EditorCommandType::Insert));
        assert(!editor.supportsCommand(EditorCommandType::Delete));
        assert(!editor.supportsCommand(EditorCommandType::Replace));
        assert(!editor.supportsCommand(EditorCommandType::Show));
        printTestResult("XmlEditor rejects all 5 text commands", true);
    }

    // TextEditor rejects XML commands
    {
        TextEditor editor;
        assert(!editor.supportsCommand(EditorCommandType::InsertBefore));
        assert(!editor.supportsCommand(EditorCommandType::AppendChild));
        assert(!editor.supportsCommand(EditorCommandType::EditId));
        assert(!editor.supportsCommand(EditorCommandType::EditText_));
        assert(!editor.supportsCommand(EditorCommandType::XmlDelete));
        assert(!editor.supportsCommand(EditorCommandType::XmlTree));
        printTestResult("TextEditor rejects all 6 XML commands", true);
    }

    std::cout << "  All command validation tests passed!" << std::endl << std::endl;
}

// ============================================================
// 11. EditorFactory 创建 XML editor 测试
// ============================================================
void test_editor_factory_xml_integration() {
    std::cout << "Test 11: EditorFactory XML integration..." << std::endl;

    auto editor = EditorFactory::createEditor(".xml");
    assert(editor != nullptr);

    auto xmlEditor = dynamic_cast<XmlEditor*>(editor.get());
    assert(xmlEditor != nullptr);
    printTestResult("EditorFactory creates XmlEditor for .xml", true);

    // 验证 XML editor supports XML commands
    assert(xmlEditor->supportsCommand(EditorCommandType::XmlTree));
    assert(!xmlEditor->supportsCommand(EditorCommandType::Append));
    printTestResult("Factory-created XmlEditor has correct supportsCommand", true);

    std::cout << "  All EditorFactory XML integration tests passed!" << std::endl << std::endl;
}

// ============================================================
// 12. 空XML文档的遍历测试
// ============================================================
void test_empty_xml_traversal() {
    std::cout << "Test 12: Empty XML document traversal..." << std::endl;

    auto doc = std::make_unique<XmlDocumentWrapper>();

    // 未加载的文档
    assert(doc->getRootId() == "");
    printTestResult("getRootId on empty doc returns empty string", true);

    // 加载只有根元素的文档
    doc->loadFromString("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<root id=\"root\">\n</root>\n");
    doc->collectIds();

    assert(doc->getRootId() == "root");
    printTestResult("getRootId returns root id after loading", true);

    auto children = doc->getChildIds("root");
    assert(children.empty());
    printTestResult("Root with no children returns empty child list", true);

    std::cout << "  All empty XML traversal tests passed!" << std::endl << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  XML Integration Test Suite" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;

    int result = 0;

    try {
        test_xml_tree_parsing();
        test_xml_tree_execution();
        test_xml_traversal_methods();
        test_output_xml_tree();
        test_supports_command_text_editor();
        test_supports_command_xml_editor();
        test_init_xml_file();
        test_init_txt_file();
        test_save_load_xml();
        test_command_validation();
        test_editor_factory_xml_integration();
        test_empty_xml_traversal();

        std::cout << "========================================" << std::endl;
        std::cout << "  ALL TESTS PASSED!" << std::endl;
        std::cout << "========================================" << std::endl;
        result = 0;
    } catch (const std::exception& e) {
        std::cerr << "TEST FAILED with exception: " << e.what() << std::endl;
        result = 1;
    } catch (...) {
        std::cerr << "TEST FAILED with unknown exception!" << std::endl;
        result = 1;
    }

    // 清理测试残留文件
    const char* cleanupFiles[] = {
        "test_save_load.xml",
        "test_init.xml",
        "test_init.txt"
    };
    for (const char* file : cleanupFiles) {
        std::remove(file);
    }

    return result;
}
