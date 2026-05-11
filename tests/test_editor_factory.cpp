#include "EditorFactory.h"
#include "Editor.h"
#include "XmlEditor.h"
#include "XmlDocumentWrapper.h"
#include "IXmlDocument.h"
#include "XMLEngine.h"
#include "XMLCommand.h"
#include <iostream>
#include <cassert>
#include <memory>
#include <fstream>
#include <cstdio>

// 测试辅助函数
void printTestResult(const std::string& testName, bool passed) {
    if (passed) {
        std::cout << "  [PASS] " << testName << std::endl;
    } else {
        std::cout << "  [FAIL] " << testName << std::endl;
    }
}

// ============================================================
// 测试1：EditorFactory根据后缀正确创建Editor
// ============================================================
void test_editor_factory_creation() {
    std::cout << "Test 1: EditorFactory creates correct editor by extension..." << std::endl;

    // 测试.txt后缀创建TextEditor
    auto txtEditor = EditorFactory::createEditor(".txt");
    assert(txtEditor != nullptr);
    printTestResult("Create editor for '.txt' extension", true);

    // 测试.xml后缀创建XmlEditor
    auto xmlEditor = EditorFactory::createEditor(".xml");
    assert(xmlEditor != nullptr);
    printTestResult("Create editor for '.xml' extension", true);

    // 测试未注册的后缀返回nullptr
    auto unknownEditor = EditorFactory::createEditor(".unknown");
    assert(unknownEditor == nullptr);
    printTestResult("Unknown extension returns nullptr", true);

    // 测试空字符串后缀
    auto emptyEditor = EditorFactory::createEditor("");
    assert(emptyEditor == nullptr);
    printTestResult("Empty extension returns nullptr", true);

    // 验证XML编辑器可以动态转换为XmlEditor
    auto xmlEditor2 = EditorFactory::createEditor(".xml");
    XmlEditor* xmlPtr = dynamic_cast<XmlEditor*>(xmlEditor2.get());
    assert(xmlPtr != nullptr);
    printTestResult("Dynamic cast to XmlEditor works", true);

    std::cout << "  All EditorFactory creation tests passed!" << std::endl << std::endl;
}

// ============================================================
// 测试2：XmlDocumentWrapper 加载和保存XML
// ============================================================
void test_xml_document_wrapper_load_save() {
    std::cout << "Test 2: XmlDocumentWrapper load and save..." << std::endl;

    const std::string xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<root id="root1">
    <child id="child1">Hello</child>
    <child id="child2">World</child>
</root>)";

    // 测试通过IXmlDocument接口加载
    std::unique_ptr<IXmlDocument> doc = std::make_unique<XmlDocumentWrapper>();
    doc->loadFromString(xmlContent);
    assert(doc->isLoaded());
    printTestResult("Load XML from string via IXmlDocument interface", true);

    // 测试ID收集
    doc->collectIds();
    assert(doc->hasNodeWithId("root1"));
    assert(doc->hasNodeWithId("child1"));
    assert(doc->hasNodeWithId("child2"));
    printTestResult("collectIds and hasNodeWithId via interface", true);

    // 测试获取节点名称
    assert(doc->getNodeName("root1") == "root");
    assert(doc->getNodeName("child1") == "child");
    printTestResult("getNodeName via interface", true);

    // 测试获取节点值
    assert(doc->getNodeValue("child1") == "Hello");
    assert(doc->getNodeValue("child2") == "World");
    printTestResult("getNodeValue via interface", true);

    // 测试保存为字符串
    std::string saved = doc->saveToString();
    assert(!saved.empty());
    printTestResult("Save to string via interface", true);

    // 测试保存到文件并重新加载
    const std::string testFile = "test_xml_temp.xml";
    doc->saveToFile(testFile);

    auto doc2 = std::make_unique<XmlDocumentWrapper>();
    doc2->loadFromFile(testFile);
    assert(doc2->isLoaded());
    doc2->collectIds();
    assert(doc2->getNodeName("root1") == "root");
    printTestResult("Save to file and reload via interface", true);

    // 清理测试文件
    std::remove(testFile.c_str());

    // 测试解析错误异常
    try {
        auto badDoc = std::make_unique<XmlDocumentWrapper>();
        badDoc->loadFromString("<invalid>");
        assert(!"Should have thrown");
    } catch (const XmlParseException& e) {
        printTestResult("Parse error throws XmlParseException", true);
    }

    // 测试加载不存在的文件
    try {
        auto badDoc = std::make_unique<XmlDocumentWrapper>();
        badDoc->loadFromFile("nonexistent_file.xml");
        assert(!"Should have thrown");
    } catch (const XmlParseException& e) {
        printTestResult("Loading nonexistent file throws XmlParseException", true);
    }

    std::cout << "  All XmlDocumentWrapper tests passed!" << std::endl << std::endl;
}

// ============================================================
// 测试3：XML节点ID映射正确性
// ============================================================
void test_xml_id_mapping() {
    std::cout << "Test 3: XML node ID mapping..." << std::endl;

    const std::string xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<library id="lib1">
    <book id="book1">
        <title id="title1">The C++ Programming Language</title>
        <author id="author1">Bjarne Stroustrup</author>
    </book>
    <book id="book2">
        <title id="title2">Clean Code</title>
        <author id="author2">Robert C. Martin</author>
    </book>
</library>)";

    auto doc = std::make_unique<XmlDocumentWrapper>();
    doc->loadFromString(xmlContent);
    doc->collectIds();

    // 验证所有ID都存在
    auto allIds = doc->getAllIds();
    assert(allIds.size() == 7);
    printTestResult("All 7 IDs collected correctly", true);

    // 验证节点名称正确
    assert(doc->getNodeName("lib1") == "library");
    printTestResult("ID 'lib1' maps to 'library' element", true);

    assert(doc->getNodeName("book1") == "book");
    printTestResult("ID 'book1' maps to 'book' element", true);

    assert(doc->getNodeName("title1") == "title");
    assert(doc->getNodeName("author1") == "author");
    printTestResult("Child element names are correct", true);

    // 验证节点内容
    assert(doc->getNodeValue("title1") == "The C++ Programming Language");
    assert(doc->getNodeValue("author2") == "Robert C. Martin");
    printTestResult("Node content (getNodeValue) is correct", true);

    std::cout << "  All ID mapping tests passed!" << std::endl << std::endl;
}

// ============================================================
// 测试4：重复ID检测
// ============================================================
void test_duplicate_id_detection() {
    std::cout << "Test 4: Duplicate ID detection..." << std::endl;

    // XML包含重复ID
    const std::string xmlWithDuplicate = R"(<?xml version="1.0" encoding="UTF-8"?>
<root id="root1">
    <item id="dup1">First</item>
    <item id="dup1">Second</item>
</root>)";

    auto doc = std::make_unique<XmlDocumentWrapper>();
    doc->loadFromString(xmlWithDuplicate);

    try {
        doc->collectIds();
        assert(!"Should have thrown DuplicateIdException");
    } catch (const DuplicateIdException& e) {
        printTestResult("Duplicate ID 'dup1' throws DuplicateIdException", true);
    }

    // XML缺少ID属性
    const std::string xmlMissingId = R"(<?xml version="1.0" encoding="UTF-8"?>
<root id="root1">
    <item>No ID here</item>
</root>)";

    auto doc2 = std::make_unique<XmlDocumentWrapper>();
    doc2->loadFromString(xmlMissingId);

    try {
        doc2->collectIds();
        assert(!"Should have thrown MissingIdException");
    } catch (const MissingIdException& e) {
        printTestResult("Missing ID throws MissingIdException", true);
    }

    std::cout << "  All duplicate/missing ID tests passed!" << std::endl << std::endl;
}

// ============================================================
// 测试5：XmlEditor Modified状态切换
// ============================================================
void test_xml_editor_modified_state() {
    std::cout << "Test 5: XmlEditor modified state switching..." << std::endl;

    // 新建XmlEditor，初始状态为未修改
    XmlEditor editor;
    assert(!editor.isModified());
    printTestResult("New XmlEditor is not modified", true);

    // 加载XML后应标记为未修改
    const std::string xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<root id="root1">
    <data id="data1">Test data</data>
</root>)";
    editor.loadFromString(xmlContent);
    assert(!editor.isModified());
    printTestResult("After loading XML, not modified", true);

    // 设置修改状态
    editor.setModified(true);
    assert(editor.isModified());
    printTestResult("setModified(true) works", true);

    editor.setModified(false);
    assert(!editor.isModified());
    printTestResult("setModified(false) works", true);

    // 保存后应标记为未修改
    editor.setModified(true);
    editor.saveToFile("test_modified_temp.xml");
    assert(!editor.isModified());
    printTestResult("After save, not modified", true);

    // 清理
    std::remove("test_modified_temp.xml");

    // clear后应标记为未修改
    editor.setModified(true);
    editor.clear();
    assert(!editor.isModified());
    printTestResult("After clear, not modified", true);

    std::cout << "  All modified state tests passed!" << std::endl << std::endl;
}

// ============================================================
// 测试6：XmlEditor ID查找
// ============================================================
void test_xml_editor_id_lookup() {
    std::cout << "Test 6: XmlEditor ID lookup..." << std::endl;

    const std::string xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<project id="proj1">
    <module id="mod1">
        <file id="file1">main.cpp</file>
        <file id="file2">utils.cpp</file>
    </module>
    <module id="mod2">
        <file id="file3">test.cpp</file>
    </module>
</project>)";

    XmlEditor editor;
    editor.loadFromString(xmlContent);

    // 查找存在的节点——通过名称和值
    assert(editor.hasNodeWithId("file1"));
    assert(editor.getNodeName("file1") == "file");
    assert(editor.getNodeValue("file1") == "main.cpp");
    printTestResult("Find existing node by ID 'file1' via string API", true);

    assert(editor.hasNodeWithId("mod2"));
    assert(editor.getNodeName("mod2") == "module");
    printTestResult("Find existing node by ID 'mod2' via string API", true);

    // 查找不存在的节点
    bool hasGhost = editor.hasNodeWithId("nonexistent");
    assert(!hasGhost);
    assert(editor.getNodeName("nonexistent") == "");
    assert(editor.getNodeValue("nonexistent") == "");
    printTestResult("Non-existent ID returns empty strings", true);

    // 检查ID是否存在
    assert(editor.hasNodeWithId("proj1"));
    assert(editor.hasNodeWithId("mod1"));
    assert(editor.hasNodeWithId("file3"));
    assert(!editor.hasNodeWithId("ghost"));
    printTestResult("hasNodeWithId works correctly", true);

    // 获取所有ID
    auto allIds = editor.getAllIds();
    assert(allIds.size() == 6); // proj1, mod1, mod2, file1, file2, file3
    printTestResult("getAllIds returns 6 IDs", true);

    std::cout << "  All XmlEditor ID lookup tests passed!" << std::endl << std::endl;
}

// ============================================================
// 测试7：XMLEngine基本功能
// ============================================================
void test_xml_engine_basic() {
    std::cout << "Test 7: XMLEngine basic functionality..." << std::endl;

    const std::string xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<config id="cfg1">
    <setting id="set1" key="theme" value="dark"/>
    <setting id="set2" key="font" value="Consolas"/>
</config>)";

    XmlEditor editor;
    editor.loadFromString(xmlContent);

    XMLEngine* engine = editor.getXMLEngine();
    assert(engine != nullptr);
    printTestResult("getXMLEngine returns non-null", true);

    // 验证引擎状态
    assert(engine->isValid());
    printTestResult("XMLEngine is valid after document load", true);

    assert(engine->isDocumentLoaded());
    printTestResult("isDocumentLoaded returns true", true);

    // 通过引擎查找节点
    assert(engine->hasNodeWithId("set1"));
    assert(engine->getNodeName("set1") == "setting");
    assert(engine->getNodeAttribute("set1", "key") == "theme");
    printTestResult("XMLEngine getNodeAttribute works", true);

    // 查找不存在的节点
    assert(!engine->hasNodeWithId("nonexistent"));
    assert(engine->getNodeName("nonexistent") == "");
    printTestResult("XMLEngine missing ID returns empty string", true);

    std::cout << "  All XMLEngine tests passed!" << std::endl << std::endl;
}

// ============================================================
// 测试8：XMLCommand基本结构
// ============================================================
void test_xml_command_structure() {
    std::cout << "Test 8: XMLCommand structure..." << std::endl;

    const std::string xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<root id="root1">
    <item id="item1">Content</item>
</root>)";

    XmlEditor editor;
    editor.loadFromString(xmlContent);

    // 创建一个具体的XMLCommand测试子类
    class TestXMLCommand : public XMLCommand {
    public:
        TestXMLCommand(XmlEditor* editor, bool& executedFlag, bool& engineOkFlag)
            : XMLCommand(editor), executed(executedFlag), engineOk(engineOkFlag) {}

        void execute() override {
            executed = true;
            auto* engine = getEngine();
            if (engine && engine->hasNodeWithId("item1")) {
                if (engine->getNodeValue("item1") == "Content") {
                    executed = true;
                    engineOk = true;
                }
            }
        }

        void undo() override {
            executed = false;
        }

        bool& executed;
        bool& engineOk;
    };

    bool flag = false;
    bool engineOk = false;
    auto cmd = std::make_unique<TestXMLCommand>(&editor, flag, engineOk);

    // 执行命令——通过引擎的字符串API访问
    cmd->execute();
    assert(flag);
    assert(engineOk);
    printTestResult("XMLCommand execute accesses engine and finds node", true);
    printTestResult("getEngine() returns valid pointer with working API", true);

    // 撤销命令
    cmd->undo();
    assert(!flag);
    printTestResult("XMLCommand undo works", true);

    std::cout << "  All XMLCommand structure tests passed!" << std::endl << std::endl;
}

// ============================================================
// 测试9：复杂XML的ID映射
// ============================================================
void test_complex_xml_mapping() {
    std::cout << "Test 9: Complex XML with nested elements..." << std::endl;

    const std::string xmlContent = R"(<?xml version="1.0" encoding="UTF-8"?>
<universe id="u1">
    <galaxy id="g1" name="Milky Way">
        <system id="s1" name="Solar System">
            <planet id="p1" name="Mercury"/>
            <planet id="p2" name="Venus"/>
            <planet id="p3" name="Earth"/>
            <planet id="p4" name="Mars"/>
        </system>
        <system id="s2" name="Alpha Centauri">
            <planet id="p5" name="Proxima b"/>
        </system>
    </galaxy>
    <galaxy id="g2" name="Andromeda">
        <system id="s3" name="Andromeda I">
            <planet id="p6" name="Andro-1"/>
            <planet id="p7" name="Andro-2"/>
        </system>
    </galaxy>
</universe>)";

    XmlEditor editor;
    editor.loadFromString(xmlContent);

    // 验证所有节点ID存在 (1 universe + 2 galaxies + 3 systems + 7 planets = 13)
    auto allIds = editor.getAllIds();
    assert(allIds.size() == 13);
    printTestResult("13 unique IDs in complex XML", true);

    // 验证层级结构与属性（通过字符串接口）
    assert(editor.hasNodeWithId("g1"));
    assert(editor.getNodeName("g1") == "galaxy");
    assert(editor.getNodeAttribute("g1", "name") == "Milky Way");
    printTestResult("Galaxy g1: correct name and attribute via string API", true);

    assert(editor.hasNodeWithId("p3"));
    assert(editor.getNodeName("p3") == "planet");
    assert(editor.getNodeAttribute("p3", "name") == "Earth");
    printTestResult("Planet p3 is Earth via string API", true);

    assert(editor.hasNodeWithId("p7"));
    assert(editor.getNodeName("p7") == "planet");
    assert(editor.getNodeAttribute("p7", "name") == "Andro-2");
    printTestResult("Node p7: correct name and attribute via string API", true);

    std::cout << "  All complex XML mapping tests passed!" << std::endl << std::endl;
}

// ============================================================
// 测试10：EditorFactory重复注册和覆盖
// ============================================================
void test_editor_factory_registration() {
    std::cout << "Test 10: EditorFactory registration mechanism..." << std::endl;

    // 验证.txt已注册
    auto txtEditor = EditorFactory::createEditor(".txt");
    assert(txtEditor != nullptr);
    printTestResult("'.txt' is registered and creates editor", true);

    // 验证.xml已注册
    auto xmlEditor = EditorFactory::createEditor(".xml");
    assert(xmlEditor != nullptr);
    printTestResult("'.xml' is registered and creates editor", true);

    // 动态注册新的编辑器类型（模拟插件扩展）
    class MockEditor : public Editor {
    public:
        void executeCommand(std::unique_ptr<Command>) override {}
    };

    EditorFactory::registerEditor(".mock", []() -> std::unique_ptr<Editor> {
        return std::make_unique<MockEditor>();
    });

    auto mockEditor = EditorFactory::createEditor(".mock");
    assert(mockEditor != nullptr);
    auto* mockPtr = dynamic_cast<MockEditor*>(mockEditor.get());
    assert(mockPtr != nullptr);
    printTestResult("Dynamic registration of '.mock' extension works", true);

    // 测试大小写敏感性（应区分大小写）
    auto upperEditor = EditorFactory::createEditor(".XML");
    assert(upperEditor == nullptr);
    printTestResult("Extension matching is case-sensitive ('.XML' not found)", true);

    std::cout << "  All EditorFactory registration tests passed!" << std::endl << std::endl;
}

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << "  EditorFactory & XmlEditor Test Suite" << std::endl;
    std::cout << "========================================" << std::endl << std::endl;

    int result = 0;

    try {
        test_editor_factory_creation();
        test_xml_document_wrapper_load_save();
        test_xml_id_mapping();
        test_duplicate_id_detection();
        test_xml_editor_modified_state();
        test_xml_editor_id_lookup();
        test_xml_engine_basic();
        test_xml_command_structure();
        test_complex_xml_mapping();
        test_editor_factory_registration();

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
        "test_xml_temp.xml",
        "test_modified_temp.xml"
    };
    for (const char* file : cleanupFiles) {
        std::remove(file);
    }

    return result;
}
