#ifndef TEXTCOMMANDS_H
#define TEXTCOMMANDS_H

#include "Command.h"
#include "TextEngine.h"
#include "OutputService.h"
#include <vector>
#include <string>
#include <memory>

// 基类：文本命令，持有TextEngine指针和文本行引用
class TextCommand : public Command {
public:
    TextCommand(std::vector<std::string>& lines, TextEngine* engine);
    virtual ~TextCommand() = default;

protected:
    std::vector<std::string>& lines;  // 引用文本行数组
    TextEngine* textEngine;           // TextEngine指针
};

// InsertCommand类：插入文本命令
class InsertCommand : public TextCommand {
public:
    InsertCommand(std::vector<std::string>& lines, TextEngine* engine,
                  int row, int col, const std::string& text);

    void execute() override;
    void undo() override;

private:
    int row;            // 行号（1-based）
    int col;            // 列号（1-based）
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
                  int row, int col, int length);

    void execute() override;
    void undo() override;

private:
    int row;            // 行号（1-based）
    int col;            // 列号（1-based）
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
class ShowCommand : public TextCommand {
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
    const std::vector<std::string>& constLines; // 常量引用，只读
    OutputService* outputService; // 输出服务
    int startLine;
    int endLine;
    std::string result;     // 显示结果
};

// ReplaceCommand类：替换文本命令（先删除，再插入）
class ReplaceCommand : public TextCommand {
public:
    ReplaceCommand(std::vector<std::string>& lines, TextEngine* engine,
                   int row, int col, int deleteLength, const std::string& replaceText);

    void execute() override;
    void undo() override;

private:
    int row;                // 行号（1-based）
    int col;                // 列号（1-based）
    int deleteLength;       // 要删除的长度
    std::string replaceText;// 替换文本（可以为空）
    std::string deletedText;// 被删除的文本（用于undo）
    bool executed;          // 是否已执行过
};

#endif // TEXTCOMMANDS_H