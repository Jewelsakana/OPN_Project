#include "TextEngine.h"
#include <iostream>
#include <cassert>
#include <vector>
#include <string>

// 辅助函数：打印测试结果
void printTestResult(const std::string& testName, bool passed) {
    std::cout << (passed ? "[PASS]" : "[FAIL]") << " " << testName << std::endl;
}

// 测试append函数
void testAppend() {
    TextEngine engine;

    // 测试1：追加单行文本
    {
        std::vector<std::string> lines;
        engine.append(lines, "Hello World");
        assert(lines.size() == 1);
        assert(lines[0] == "Hello World");
        printTestResult("testAppend - single line", true);
    }

    // 测试2：追加多行文本（包含换行符）
    {
        std::vector<std::string> lines;
        engine.append(lines, "Line1\nLine2\nLine3");
        assert(lines.size() == 3);
        assert(lines[0] == "Line1");
        assert(lines[1] == "Line2");
        assert(lines[2] == "Line3");
        printTestResult("testAppend - multiple lines with newline", true);
    }

    // 测试3：追加空文本
    {
        std::vector<std::string> lines = {"Existing"};
        engine.append(lines, "");
        assert(lines.size() == 1);
        assert(lines[0] == "Existing");
        printTestResult("testAppend - empty text", true);
    }

    // 测试4：追加只包含换行符的文本
    {
        std::vector<std::string> lines;
        engine.append(lines, "\n\n");
        assert(lines.size() == 3);  // 三个空行
        assert(lines[0] == "");
        assert(lines[1] == "");
        assert(lines[2] == "");
        printTestResult("testAppend - newlines only", true);
    }
}

// 测试insert函数
void testInsert() {
    TextEngine engine;

    // 测试1：在行首插入单行文本
    {
        std::vector<std::string> lines = {"World"};
        engine.insert(lines, 1, 1, "Hello ");
        assert(lines.size() == 1);
        assert(lines[0] == "Hello World");
        printTestResult("testInsert - at line beginning", true);
    }

    // 测试2：在行中插入文本
    {
        std::vector<std::string> lines = {"Hello World"};
        engine.insert(lines, 1, 7, "Beautiful ");
        assert(lines[0] == "Hello Beautiful World");
        printTestResult("testInsert - in middle of line", true);
    }

    // 测试3：在行尾插入文本
    {
        std::vector<std::string> lines = {"Hello"};
        engine.insert(lines, 1, 6, " World");  // 列号6（行尾之后）
        assert(lines[0] == "Hello World");
        printTestResult("testInsert - at line end", true);
    }

    // 测试4：插入包含换行符的文本
    {
        std::vector<std::string> lines = {"StartEnd"};
        engine.insert(lines, 1, 6, "\nMiddle\n");
        assert(lines.size() == 3);
        assert(lines[0] == "Start");
        assert(lines[1] == "Middle");
        assert(lines[2] == "End");
        printTestResult("testInsert - with newlines", true);
    }

    // 测试5：在起始位置插入空字符串（应该允许）
    {
        std::vector<std::string> lines = {"Hello"};
        bool exceptionThrown = false;
        try {
            engine.insert(lines, 1, 1, "");
            // 不应该抛出异常
        } catch (const EmptyStringInsertionException&) {
            exceptionThrown = true;
        }
        assert(!exceptionThrown);
        assert(lines[0] == "Hello");  // 插入空字符串不应该改变内容
        printTestResult("testInsert - empty string at start position", true);
    }


    // 测试7：lines为空时在起始位置插入空字符串
    {
        std::vector<std::string> lines;
        bool exceptionThrown = false;
        try {
            engine.insert(lines, 1, 1, "");
        } catch (const std::exception&) {
            exceptionThrown = true;
        }
        assert(!exceptionThrown);
        assert(lines.size() == 1);
        assert(lines[0] == "");  // 应该创建一个空行
        printTestResult("testInsert - empty string with empty lines", true);
    }

    // 测试8：lines为空时在非起始位置插入空字符串（应该抛出异常）
    {
        std::vector<std::string> lines;
        bool exceptionThrown = false;
        try {
            engine.insert(lines, 1, 2, "");  // 列号不是1
        } catch (const RowOutOfBoundsException&) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
        printTestResult("testInsert - empty string with empty lines at wrong position", true);
    }

    // 测试9：行号越界异常
    {
        std::vector<std::string> lines = {"Line1", "Line2"};
        bool exceptionThrown = false;
        try {
            engine.insert(lines, 3, 1, "Text");
        } catch (const RowOutOfBoundsException&) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
        printTestResult("testInsert - row out of bounds exception", true);
    }

    // 测试10：列号越界异常
    {
        std::vector<std::string> lines = {"Hello"};
        bool exceptionThrown = false;
        try {
            engine.insert(lines, 1, 10, "Text");  // 列号10超出"Hello"长度+1
        } catch (const ColumnOutOfBoundsException&) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
        printTestResult("testInsert - column out of bounds exception", true);
    }
}

// 测试deleteText函数
void testDelete() {
    TextEngine engine;

    // 测试1：删除行中的文本
    {
        std::vector<std::string> lines = {"Hello World"};
        engine.deleteText(lines, 1, 7, 5);  // 删除"World"
        assert(lines[0] == "Hello ");
        printTestResult("testDelete - delete middle of line", true);
    }

    // 测试2：删除行首文本
    {
        std::vector<std::string> lines = {"Hello World"};
        engine.deleteText(lines, 1, 1, 5);  // 删除"Hello"
        assert(lines[0] == " World");
        printTestResult("testDelete - delete beginning of line", true);
    }

    // 测试3：删除行尾文本
    {
        std::vector<std::string> lines = {"Hello World"};
        engine.deleteText(lines, 1, 7, 5);  // 删除"World"
        assert(lines[0] == "Hello ");
        printTestResult("testDelete - delete end of line", true);
    }

    // 测试4：删除长度超出行尾异常
    {
        std::vector<std::string> lines = {"Hello"};
        bool exceptionThrown = false;
        try {
            engine.deleteText(lines, 1, 3, 10);  // 尝试删除10个字符，但只有3个剩余
        } catch (const DeleteLengthExceedsLineException&) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
        printTestResult("testDelete - delete length exceeds line exception", true);
    }

    // 测试5：行号越界异常
    {
        std::vector<std::string> lines = {"Hello"};
        bool exceptionThrown = false;
        try {
            engine.deleteText(lines, 2, 1, 1);  // 行号2不存在
        } catch (const RowOutOfBoundsException&) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
        printTestResult("testDelete - row out of bounds exception", true);
    }

    // 测试6：列号越界异常
    {
        std::vector<std::string> lines = {"Hello"};
        bool exceptionThrown = false;
        try {
            engine.deleteText(lines, 1, 10, 1);  // 列号10超出字符串长度
        } catch (const ColumnOutOfBoundsException&) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
        printTestResult("testDelete - column out of bounds exception", true);
    }
}

// 测试show函数
void testShow() {
    TextEngine engine;

    // 测试1：显示单行
    {
        std::vector<std::string> lines = {"Hello World"};
        std::string result = engine.show(lines);
        assert(result == "Hello World");
        printTestResult("testShow - single line", true);
    }

    // 测试2：显示多行
    {
        std::vector<std::string> lines = {"Line1", "Line2", "Line3"};
        std::string result = engine.show(lines);
        assert(result == "Line1\nLine2\nLine3");
        printTestResult("testShow - multiple lines", true);
    }

    // 测试3：显示指定范围
    {
        std::vector<std::string> lines = {"Line1", "Line2", "Line3", "Line4"};
        std::string result = engine.show(lines, 1, 2);  // 显示第2-3行（0-based索引）
        assert(result == "Line2\nLine3");
        printTestResult("testShow - specific range", true);
    }

    // 测试4：显示空向量
    {
        std::vector<std::string> lines;
        std::string result = engine.show(lines);
        assert(result == "");
        printTestResult("testShow - empty vector", true);
    }

    // 测试5：使用默认参数（startLine=0, endLine=-1）
    {
        std::vector<std::string> lines = {"Line1", "Line2", "Line3"};
        std::string result = engine.show(lines);
        assert(result == "Line1\nLine2\nLine3");
        printTestResult("testShow - default parameters", true);
    }

    // 测试6：行号越界异常
    {
        std::vector<std::string> lines = {"Line1", "Line2"};
        bool exceptionThrown = false;
        try {
            engine.show(lines, 3, 3);  // 行号3不存在
        } catch (const RowOutOfBoundsException&) {
            exceptionThrown = true;
        }
        assert(exceptionThrown);
        printTestResult("testShow - row out of bounds exception", true);
    }
}

// 综合测试
void testIntegration() {
    TextEngine engine;

    // 创建一个文档并进行一系列操作
    std::vector<std::string> lines;

    // 追加初始文本
    engine.append(lines, "First line\nSecond line");
    assert(lines.size() == 2);
    assert(lines[0] == "First line");
    assert(lines[1] == "Second line");

    // 在第二行插入文本
    engine.insert(lines, 2, 7, " beautiful");
    assert(lines[1] == "Second beautiful line");

    // 删除第一行的部分文本
    engine.deleteText(lines, 1, 7, 4);  // 删除"line"
    assert(lines[0] == "First ");

    // 显示结果
    std::string result = engine.show(lines);
    assert(result == "First \nSecond beautiful line");

    printTestResult("testIntegration - complex operations", true);
}

int main() {
    std::cout << "Running TextEngine tests..." << std::endl;
    std::cout << "=============================" << std::endl;

    try {
        testAppend();
        testInsert();
        testDelete();
        testShow();
        testIntegration();

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