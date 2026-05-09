#ifndef TEXTCOMMANDS_H
#define TEXTCOMMANDS_H

#include "Command.h"
#include "TextEngine.h"
#include "OutputService.h"
#include <vector>
#include <string>
#include <memory>

// 位置值对象，封装行号和列号（均为1-based）
struct Position {
    int line;   // 行号（1-based）
    int column; // 列号（1-based）

    Position(int l, int c) : line(l), column(c) {}
};

// 基类：文本命令，持有TextEngine指针和文本行引用
class TextCommand : public Command {
public:
    TextCommand(std::vector<std::string>& lines, TextEngine* engine);
    virtual ~TextCommand() = default;

protected:
    std::vector<std::string>& lines;  // 引用文本行数组
    TextEngine* textEngine;           // TextEngine指针

    // 记录指定位置将被删除的文本，用于undo
    bool recordDeletedText(int row, int col, int length, std::string& deletedTextOut);
};

// InsertCommand类：插入文本命令
class InsertCommand : public TextCommand {
public:
    InsertCommand(std::vector<std::string>& lines, TextEngine* engine,
                  Position pos, const std::string& text);
    InsertCommand(std::vector<std::string>& lines, TextEngine* engine,
                  int row, int col, const std::string& text);

    void execute() override;
    void undo() override;

private:
    Position pos;       // 插入位置
    std::string text;   // 要插入的文本
    int insertedLength; // 插入的文本长度（用于undo）
    int newLineCount;   // 插入文本中的换行符数量（即新添加的行数-1）
    bool isMultiLineInsert; // 是否为多行插入
    std::string beforeInsert; // 插入点前的文本（用于多行插入的undo）
    std::string afterInsert;  // 插入点后的文本（用于多行插入的undo）
};

// DeleteCommand类：删除文本命令
class DeleteCommand : public TextCommand {
public:
    DeleteCommand(std::vector<std::string>& lines, TextEngine* engine,
                  Position pos, int length);
    DeleteCommand(std::vector<std::string>& lines, TextEngine* engine,
                  int row, int col, int length);

    void execute() override;
    void undo() override;

private:
    Position pos;       // 删除位置
    int length;         // 要删除的长度
    std::string deletedText; // 被删除的文本（用于undo）
    bool executed;      // 是否已执行过（用于防止重复undo）
};

// AppendCommand类：追加文本命令
class AppendCommand : public TextCommand {
public:
    AppendCommand(std::vector<std::string>& lines, TextEngine* engine,
                  const std::string& text);

    void execute() override;
    void undo() override;

private:
    std::string text;               // 要追加的文本
    size_t originalLineCount;       // 执行前的行数（用于undo）
    size_t appendedLineCount;       // 追加的行数
};

// ShowCommand类：显示文本命令（只读，不进入UndoStack）
// 直接继承Command，不继承TextCommand，避免const_cast
class ShowCommand : public Command {
public:
    ShowCommand(const std::vector<std::string>& lines, TextEngine* engine,
                OutputService* outputService,
                int startLine = 0, int endLine = -1);

    void execute() override;
    void undo() override;  // ShowCommand是只读的，undo什么也不做

    // 重写isReadOnly方法，返回true
    bool isReadOnly() const override { return true; }

    // 获取显示结果
    std::string getResult() const;

private:
    const std::vector<std::string>& lines_; // 常量引用，只读
    TextEngine* textEngine_;               // TextEngine指针
    OutputService* outputService_;          // 输出服务
    int startLine_;
    int endLine_;
    std::string result_;                    // 显示结果
};

// ReplaceCommand类：替换文本命令（先删除，再插入）
class ReplaceCommand : public TextCommand {
public:
    ReplaceCommand(std::vector<std::string>& lines, TextEngine* engine,
                   Position pos, int deleteLength, const std::string& replaceText);
    ReplaceCommand(std::vector<std::string>& lines, TextEngine* engine,
                   int row, int col, int deleteLength, const std::string& replaceText);

    void execute() override;
    void undo() override;

private:
    Position pos;           // 替换位置
    int deleteLength;       // 要删除的长度
    std::string replaceText;// 替换文本（可以为空）
    std::string deletedText;// 被删除的文本（用于undo）
    bool executed;          // 是否已执行过
};

#endif // TEXTCOMMANDS_H