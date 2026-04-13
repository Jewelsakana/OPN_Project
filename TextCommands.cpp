#include "TextCommands.h"

// TextCommand基类实现
TextCommand::TextCommand(std::vector<std::string>& lines, TextEngine* engine)
    : lines(lines)
    , textEngine(engine) {
}

// InsertCommand实现
InsertCommand::InsertCommand(std::vector<std::string>& lines, TextEngine* engine,
                             int row, int col, const std::string& text)
    : TextCommand(lines, engine)
    , row(row)
    , col(col)
    , text(text)
    , insertedLength(0)
    , newLineCount(0)
    , isMultiLineInsert(false) {
}

void InsertCommand::execute() {
    // 计算换行符数量
    newLineCount = 0;
    for (char c : text) {
        if (c == '\n') {
            newLineCount++;
        }
    }

    // 判断是否为多行插入
    isMultiLineInsert = (newLineCount > 0);

    // 记录插入前的状态
    if (row >= 1 && row <= static_cast<int>(lines.size())) {
        const std::string& currentLine = lines[row - 1];
        if (col >= 1 && col <= static_cast<int>(currentLine.length()) + 1) {
            // 计算0-based索引
            int startIdx = col - 1;
            beforeInsert = currentLine.substr(0, startIdx);
            afterInsert = currentLine.substr(startIdx);
        }
    }

    // 记录插入长度（用于单行插入的undo）
    insertedLength = static_cast<int>(text.length());

    // 调用TextEngine插入文本
    textEngine->insert(lines, row, col, text);
}

void InsertCommand::undo() {
    // 如果插入的是空文本，什么也不做
    if (text.empty()) {
        return;
    }

    if (isMultiLineInsert) {
        // 多行插入的撤销
        // 计算新添加的行数（换行符数量 + 1）
        int addedLines = newLineCount + 1;

        // 计算0-based的行索引
        int targetRow = row - 1;  // 原始行号（0-based）

        // 检查边界
        if (targetRow >= static_cast<int>(lines.size())) {
            return; // 行号越界
        }

        // 重建原始行：beforeInsert + afterInsert
        std::string originalLine = beforeInsert + afterInsert;

        // 删除新增的行（从targetRow开始，删除addedLines行）
        // 注意：插入后，原来的第row行被替换成了addedLines行
        // 所以我们需要删除从targetRow开始的addedLines行
        if (targetRow + addedLines <= static_cast<int>(lines.size())) {
            // 删除从targetRow开始的addedLines行
            lines.erase(lines.begin() + targetRow, lines.begin() + targetRow + addedLines);

            // 插入原始行
            lines.insert(lines.begin() + targetRow, originalLine);
        }
    } else {
        // 单行插入的撤销：删除插入的文本
        if (insertedLength > 0) {
            textEngine->deleteText(lines, row, col, insertedLength);
        }
    }
}

// DeleteCommand实现
DeleteCommand::DeleteCommand(std::vector<std::string>& lines, TextEngine* engine,
                             int row, int col, int length)
    : TextCommand(lines, engine)
    , row(row)
    , col(col)
    , length(length)
    , executed(false) {
}

void DeleteCommand::execute() {
    if (executed) {
        return; // 防止重复执行
    }

    // 在执行删除前，先记录被删除的文本
    // 需要获取当前行的内容，提取要删除的部分
    if (row >= 1 && row <= static_cast<int>(lines.size())) {
        const std::string& line = lines[row - 1];
        if (col >= 1 && col <= static_cast<int>(line.length()) + 1) {
            // 计算0-based索引
            int startIdx = col - 1;
            int endIdx = std::min(startIdx + length, static_cast<int>(line.length()));

            if (startIdx < endIdx) {
                deletedText = line.substr(startIdx, endIdx - startIdx);
            } else {
                deletedText = "";
            }
        }
    }

    // 执行删除
    textEngine->deleteText(lines, row, col, length);
    executed = true;
}

void DeleteCommand::undo() {
    // 撤销删除：将删除的文本插回原处
    if (!deletedText.empty()) {
        textEngine->insert(lines, row, col, deletedText);
    } else if (executed) {
        // 即使删除的是空字符串，也需要标记为已撤销
        // 这里什么也不做
    }
    executed = false;
}

// AppendCommand实现
AppendCommand::AppendCommand(std::vector<std::string>& lines, TextEngine* engine,
                             const std::string& text)
    : TextCommand(lines, engine)
    , text(text)
    , originalLineCount(0)
    , appendedLineCount(0) {
}

void AppendCommand::execute() {
    // 记录追加前的行数
    originalLineCount = lines.size();

    // 执行追加
    textEngine->append(lines, text);

    // 计算追加了多少行
    appendedLineCount = lines.size() - originalLineCount;
}

void AppendCommand::undo() {
    // 撤销追加：删除追加的行
    if (appendedLineCount > 0) {
        // 删除从originalLineCount开始的所有行
        lines.resize(originalLineCount);
    }
}

// ShowCommand实现
ShowCommand::ShowCommand(const std::vector<std::string>& lines, TextEngine* engine,
                         OutputService* outputService,
                         int startLine, int endLine)
    : TextCommand(const_cast<std::vector<std::string>&>(lines), engine)
    , constLines(lines)
    , outputService(outputService)
    , startLine(startLine)
    , endLine(endLine) {
    if (!outputService) {
        throw std::invalid_argument("ShowCommand: OutputService cannot be null");
    }
}

void ShowCommand::execute() {
    // 调用TextEngine显示文本
    result = textEngine->show(constLines, startLine, endLine);

    // 使用OutputService输出结果
    outputService->outputText(result);
}

void ShowCommand::undo() {
    // ShowCommand是只读命令，undo什么也不做
}

std::string ShowCommand::getResult() const {
    return result;
}

// ReplaceCommand实现
ReplaceCommand::ReplaceCommand(std::vector<std::string>& lines, TextEngine* engine,
                               int row, int col, int deleteLength, const std::string& replaceText)
    : TextCommand(lines, engine)
    , row(row)
    , col(col)
    , deleteLength(deleteLength)
    , replaceText(replaceText)
    , executed(false) {
}

void ReplaceCommand::execute() {
    if (executed) {
        return; // 防止重复执行
    }

    // 先执行删除，记录被删除的文本
    if (row >= 1 && row <= static_cast<int>(lines.size())) {
        const std::string& line = lines[row - 1];
        if (col >= 1 && col <= static_cast<int>(line.length()) + 1) {
            // 计算0-based索引
            int startIdx = col - 1;
            int endIdx = std::min(startIdx + deleteLength, static_cast<int>(line.length()));

            if (startIdx < endIdx) {
                deletedText = line.substr(startIdx, endIdx - startIdx);
            } else {
                deletedText = "";
            }
        }
    }

    // 执行删除
    textEngine->deleteText(lines, row, col, deleteLength);

    // 再执行插入（替换文本可以为空）
    if (!replaceText.empty()) {
        textEngine->insert(lines, row, col, replaceText);
    }

    executed = true;
}

void ReplaceCommand::undo() {
    // 撤销替换：先删除插入的文本（如果有），再恢复被删除的文本
    if (executed) {
        // 如果有替换文本，先删除它
        if (!replaceText.empty()) {
            // 计算替换文本的长度
            int replaceLength = static_cast<int>(replaceText.length());
            textEngine->deleteText(lines, row, col, replaceLength);
        }

        // 恢复被删除的文本
        if (!deletedText.empty()) {
            textEngine->insert(lines, row, col, deletedText);
        }

        executed = false;
    }
}