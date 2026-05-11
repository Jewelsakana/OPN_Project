#include "XmlEditor.h"
#include "XmlDocumentWrapper.h"
#include "IXmlDocument.h"
#include "CommandFactory.h"
#include "CommandParser.h"
#include "CommandParserStrategy.h"
#include <iostream>
#include <cassert>
#include <string>

void printTestResult(const std::string& testName, bool passed) {
    std::cout << (passed ? "[PASS]" : "[FAIL]") << " " << testName << std::endl;
}

void loadXml(XmlEditor& editor, const std::string& xml) {
    editor.loadFromString(xml);
}

const char* SIMPLE_XML = R"(<?xml version="1.0"?>
<root id="root">
  <child id="c1">text1</child>
  <child id="c2">text2</child>
</root>)";

// ============================================================
// 1. InsertBefore 测试
// ============================================================
void testInsertBefore() {
    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        auto& doc = editor.getDocument();
        assert(doc.hasNodeWithId("c2"));
        doc.insertBefore("item", "new1", "c2", "hello");

        assert(doc.hasNodeWithId("new1"));
        assert(doc.getNodeName("new1") == "item");
        assert(doc.getNodeValue("new1") == "hello");

        auto ids = doc.getAllIds();
        assert(ids.size() == 4);

        printTestResult("testInsertBefore - basic insert", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        auto& doc = editor.getDocument();
        doc.insertBefore("empty", "e1", "c1", "");

        assert(doc.hasNodeWithId("e1"));
        assert(doc.getNodeValue("e1") == "");

        printTestResult("testInsertBefore - no text", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        bool thrown = false;
        try {
            editor.getDocument().insertBefore("x", "c1", "c2", "");
        } catch (const XmlDocumentException&) {
            thrown = true;
        }
        assert(thrown);
        printTestResult("testInsertBefore - duplicate newId", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        bool thrown = false;
        try {
            editor.getDocument().insertBefore("x", "newX", "nonexist", "");
        } catch (const XmlDocumentException&) {
            thrown = true;
        }
        assert(thrown);
        printTestResult("testInsertBefore - target not found", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        bool thrown = false;
        try {
            editor.getDocument().insertBefore("x", "newRoot", "root", "");
        } catch (const XmlDocumentException&) {
            thrown = true;
        }
        assert(thrown);
        printTestResult("testInsertBefore - before root (should fail)", true);
    }
}

// ============================================================
// 2. AppendChild 测试
// ============================================================
void testAppendChild() {
    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        auto& doc = editor.getDocument();
        doc.appendChild("item", "newChild", "root", "child text");

        assert(doc.hasNodeWithId("newChild"));
        assert(doc.getNodeName("newChild") == "item");
        assert(doc.getNodeValue("newChild") == "child text");

        printTestResult("testAppendChild - basic append", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        auto& doc = editor.getDocument();
        doc.appendChild("empty", "e1", "root", "");

        assert(doc.hasNodeWithId("e1"));
        assert(doc.getNodeValue("e1") == "");

        printTestResult("testAppendChild - no text", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        bool thrown = false;
        try {
            editor.getDocument().appendChild("x", "newX", "nobody", "");
        } catch (const XmlDocumentException&) {
            thrown = true;
        }
        assert(thrown);
        printTestResult("testAppendChild - parent not found", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        bool thrown = false;
        try {
            editor.getDocument().appendChild("x", "c1", "root", "");
        } catch (const XmlDocumentException&) {
            thrown = true;
        }
        assert(thrown);
        printTestResult("testAppendChild - duplicate newId", true);
    }
}

// ============================================================
// 3. EditId 测试
// ============================================================
void testEditId() {
    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        auto& doc = editor.getDocument();
        doc.editId("c1", "child_one");

        assert(!doc.hasNodeWithId("c1"));
        assert(doc.hasNodeWithId("child_one"));
        assert(doc.getNodeName("child_one") == "child");

        printTestResult("testEditId - basic rename", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        bool thrown = false;
        try {
            editor.getDocument().editId("nobody", "newId");
        } catch (const XmlDocumentException&) {
            thrown = true;
        }
        assert(thrown);
        printTestResult("testEditId - oldId not found", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        bool thrown = false;
        try {
            editor.getDocument().editId("c1", "c2");
        } catch (const XmlDocumentException&) {
            thrown = true;
        }
        assert(thrown);
        printTestResult("testEditId - newId already exists", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        bool thrown = false;
        try {
            editor.getDocument().editId("root", "newRoot");
        } catch (const XmlDocumentException&) {
            thrown = true;
        }
        assert(thrown);
        printTestResult("testEditId - rename root (should fail)", true);
    }
}

// ============================================================
// 4. EditText 测试
// ============================================================
void testEditText() {
    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        auto& doc = editor.getDocument();
        doc.editText("c1", "new text");

        assert(doc.getNodeValue("c1") == "new text");

        printTestResult("testEditText - set new text", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        auto& doc = editor.getDocument();
        doc.editText("c1", "");

        assert(doc.getNodeValue("c1") == "");

        printTestResult("testEditText - clear text", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        bool thrown = false;
        try {
            editor.getDocument().editText("nobody", "text");
        } catch (const XmlDocumentException&) {
            thrown = true;
        }
        assert(thrown);
        printTestResult("testEditText - element not found", true);
    }
}

// ============================================================
// 5. Delete 测试
// ============================================================
void testXmlDelete() {
    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        auto& doc = editor.getDocument();
        assert(doc.hasNodeWithId("c1"));

        doc.deleteElement("c1");

        assert(!doc.hasNodeWithId("c1"));
        assert(doc.hasNodeWithId("c2"));
        assert(doc.hasNodeWithId("root"));

        printTestResult("testXmlDelete - basic delete", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        bool thrown = false;
        try {
            editor.getDocument().deleteElement("nobody");
        } catch (const XmlDocumentException&) {
            thrown = true;
        }
        assert(thrown);
        printTestResult("testXmlDelete - element not found", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        bool thrown = false;
        try {
            editor.getDocument().deleteElement("root");
        } catch (const XmlDocumentException&) {
            thrown = true;
        }
        assert(thrown);
        printTestResult("testXmlDelete - delete root (should fail)", true);
    }
}

// ============================================================
// 6. Undo 测试
// ============================================================
void testUndo() {
    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        auto& doc = editor.getDocument();
        doc.insertBefore("item", "new1", "c2", "hello");
        assert(doc.hasNodeWithId("new1"));

        doc.deleteElement("new1");
        assert(!doc.hasNodeWithId("new1"));

        printTestResult("testUndo - insertBefore undo", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        auto& doc = editor.getDocument();
        doc.appendChild("item", "newChild", "root", "text");
        assert(doc.hasNodeWithId("newChild"));

        doc.deleteElement("newChild");
        assert(!doc.hasNodeWithId("newChild"));

        printTestResult("testUndo - appendChild undo", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        auto& doc = editor.getDocument();
        doc.editId("c1", "renamed");
        assert(doc.hasNodeWithId("renamed"));
        assert(!doc.hasNodeWithId("c1"));

        doc.editId("renamed", "c1");
        assert(doc.hasNodeWithId("c1"));
        assert(!doc.hasNodeWithId("renamed"));

        printTestResult("testUndo - editId undo", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        auto& doc = editor.getDocument();
        assert(doc.getNodeValue("c1") == "text1");

        doc.editText("c1", "modified");
        assert(doc.getNodeValue("c1") == "modified");

        doc.editText("c1", "text1");
        assert(doc.getNodeValue("c1") == "text1");

        printTestResult("testUndo - editText undo", true);
    }

    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        auto& doc = editor.getDocument();

        std::string savedXml = doc.getNodeXml("c1");
        std::string parentId = doc.getParentId("c1");
        std::string tagName = doc.getNodeName("c1");
        std::string oldText = doc.getNodeValue("c1");

        doc.deleteElement("c1");
        assert(!doc.hasNodeWithId("c1"));

        doc.appendChild(tagName, "c1", parentId, oldText);
        assert(doc.hasNodeWithId("c1"));
        assert(doc.getNodeName("c1") == "child");

        printTestResult("testUndo - delete undo (re-insert)", true);
    }
}

// ============================================================
// 7. ID映射同步测试
// ============================================================
void testIdMapSync() {
    {
        XmlEditor editor;
        loadXml(editor, SIMPLE_XML);

        auto& doc = editor.getDocument();
        auto ids = doc.getAllIds();
        assert(ids.size() == 3);

        doc.insertBefore("item", "new1", "c2", "");
        ids = doc.getAllIds();
        assert(ids.size() == 4);

        doc.deleteElement("c1");
        ids = doc.getAllIds();
        assert(ids.size() == 3);
        assert(!doc.hasNodeWithId("c1"));

        doc.editId("c2", "child2");
        assert(!doc.hasNodeWithId("c2"));
        assert(doc.hasNodeWithId("child2"));

        printTestResult("testIdMapSync - ID map correctly updated", true);
    }
}

// ============================================================
// 8. 命令行解析测试
// ============================================================
void testCommandParsing() {
    {
        CommandParser parser;
        auto parsed = parser.parse("insert-before item newId targetId");
        auto* ed = parsed.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::InsertBefore);
        assert(ed->tagName.value_or("") == "item");
        assert(ed->newId.value_or("") == "newId");
        assert(ed->targetId.value_or("") == "targetId");
        assert(ed->text.value_or("missing") == "missing");

        printTestResult("testCommandParsing - insert-before", true);
    }

    {
        CommandParser parser;
        auto parsed = parser.parse("insert-before item newId targetId \"hello world\"");
        auto* ed = parsed.asEditor();
        assert(ed->text.value_or("") == "hello world");

        printTestResult("testCommandParsing - insert-before with text", true);
    }

    {
        CommandParser parser;
        auto parsed = parser.parse("append-child child newId parentId");
        auto* ed = parsed.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::AppendChild);

        printTestResult("testCommandParsing - append-child", true);
    }

    {
        CommandParser parser;
        auto parsed = parser.parse("edit-id oldId newId");
        auto* ed = parsed.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::EditId);
        assert(ed->targetId.value_or("") == "oldId");
        assert(ed->newId.value_or("") == "newId");

        printTestResult("testCommandParsing - edit-id", true);
    }

    {
        CommandParser parser;
        auto parsed = parser.parse("edit-text elem \"new content\"");
        auto* ed = parsed.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::EditText_);
        assert(ed->targetId.value_or("") == "elem");
        assert(ed->text.value_or("") == "new content");

        printTestResult("testCommandParsing - edit-text", true);
    }

    {
        CommandParser parser;
        auto parsed = parser.parse("edit-text elem");
        auto* ed = parsed.asEditor();
        assert(ed->text.value_or("KEEP") == "KEEP");

        printTestResult("testCommandParsing - edit-text without text", true);
    }

    {
        CommandParser parser;
        auto parsed = parser.parse("delete myElement");
        auto* ed = parsed.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::XmlDelete);
        assert(ed->targetId.value_or("") == "myElement");

        printTestResult("testCommandParsing - xml delete", true);
    }

    {
        CommandParser parser;
        auto parsed = parser.parse("delete 1:5 10");
        auto* ed = parsed.asEditor();
        assert(ed != nullptr);
        assert(ed->editorType == EditorCommandType::Delete);
        assert(ed->line.value_or(0) == 1);
        assert(ed->column.value_or(0) == 5);
        assert(ed->length.value_or(0) == 10);

        printTestResult("testCommandParsing - text delete", true);
    }
}

// ============================================================
// 9. IXmlDocument::isRootNode 测试
// ============================================================
void testIsRootNode() {
    XmlEditor editor;
    loadXml(editor, SIMPLE_XML);

    auto& doc = editor.getDocument();
    assert(doc.isRootNode("root"));
    assert(!doc.isRootNode("c1"));
    assert(!doc.isRootNode("nobody"));

    printTestResult("testIsRootNode", true);
}


int main() {
    std::cout << "Running XML Commands tests..." << std::endl;
    std::cout << "=============================" << std::endl;

    try {
        testInsertBefore();
        testAppendChild();
        testEditId();
        testEditText();
        testXmlDelete();
        testUndo();
        testIdMapSync();
        testCommandParsing();
        testIsRootNode();

        std::cout << "=============================" << std::endl;
        std::cout << "All tests passed!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "Test failed with unknown exception" << std::endl;
        return 1;
    }
}
