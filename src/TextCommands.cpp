#include "TextCommands.h"
#include "CommandFactory.h"

// 自注册：编辑器命令工厂
namespace {
    static bool _reg_append = []() {
        CommandFactory::registerEditorCreator(EditorCommandType::Append,
            [](const EditorParsedCommand& ed, const EditorCommandContext& ctx) -> std::unique_ptr<Command> {
                if (ed.text)
                    return std::make_unique<AppendCommand>(ctx.lines, ctx.textEngine, *ed.text);
                return nullptr;
            });
        return true;
    }();

    static bool _reg_insert = []() {
        CommandFactory::registerEditorCreator(EditorCommandType::Insert,
            [](const EditorParsedCommand& ed, const EditorCommandContext& ctx) -> std::unique_ptr<Command> {
                if (ed.line && ed.column && ed.text)
                    return std::make_unique<InsertCommand>(ctx.lines, ctx.textEngine, *ed.line, *ed.column, *ed.text);
                return nullptr;
            });
        return true;
    }();

    static bool _reg_delete = []() {
        CommandFactory::registerEditorCreator(EditorCommandType::Delete,
            [](const EditorParsedCommand& ed, const EditorCommandContext& ctx) -> std::unique_ptr<Command> {
                if (ed.line && ed.column && ed.length)
                    return std::make_unique<DeleteCommand>(ctx.lines, ctx.textEngine, *ed.line, *ed.column, *ed.length);
                return nullptr;
            });
        return true;
    }();

    static bool _reg_replace = []() {
        CommandFactory::registerEditorCreator(EditorCommandType::Replace,
            [](const EditorParsedCommand& ed, const EditorCommandContext& ctx) -> std::unique_ptr<Command> {
                if (ed.line && ed.column && ed.length && ed.text)
                    return std::make_unique<ReplaceCommand>(ctx.lines, ctx.textEngine, *ed.line, *ed.column, *ed.length, *ed.text);
                return nullptr;
            });
        return true;
    }();

    static bool _reg_show = []() {
        CommandFactory::registerEditorCreator(EditorCommandType::Show,
            [](const EditorParsedCommand& ed, const EditorCommandContext& ctx) -> std::unique_ptr<Command> {
                if (ed.startLine && ed.endLine)
                    return std::make_unique<ShowCommand>(ctx.lines, ctx.textEngine, ctx.outputService, *ed.startLine, *ed.endLine);
                else
                    return std::make_unique<ShowCommand>(ctx.lines, ctx.textEngine, ctx.outputService);
            });
        return true;
    }();
}

// TextCommand基类实现
TextCommand::TextCommand(std::vector<std::string>& lines, TextEngine* engine)
    : lines(lines)
    , textEngine(engine) {
}

bool TextCommand::recordDeletedText(int row, int col, int length, std::string& deletedTextOut) {
    if (row >= 1 && row <= static_cast<int>(lines.size())) {
        const std::string& line = lines[row - 1];
        if (col >= 1 && col <= static_cast<int>(line.length()) + 1) {
            int startIdx = col - 1;
            int endIdx = std::min(startIdx + length, static_cast<int>(line.length()));
            if (startIdx < endIdx) {
                deletedTextOut = line.substr(startIdx, endIdx - startIdx);
                return true;
            }
        }
    }
    deletedTextOut = "";
    return false;
}

// InsertCommand实现
InsertCommand::InsertCommand(std::vector<std::string>& lines, TextEngine* engine,
                             Position pos, const std::string& text)
    : TextCommand(lines, engine)
    , pos(pos)
    , text(text)
    , insertedLength(0)
    , newLineCount(0)
    , isMultiLineInsert(false) {
}

InsertCommand::InsertCommand(std::vector<std::string>& lines, TextEngine* engine,
                             int row, int col, const std::string& text)
    : InsertCommand(lines, engine, Position(row, col), text) {
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
    if (pos.line >= 1 && pos.line <= static_cast<int>(lines.size())) {
        const std::string& currentLine = lines[pos.line - 1];
        if (pos.column >= 1 && pos.column <= static_cast<int>(currentLine.length()) + 1) {
            // 计算0-based索引
            int startIdx = pos.column - 1;
            beforeInsert = currentLine.substr(0, startIdx);
            afterInsert = currentLine.substr(startIdx);
        }
    }

    // 记录插入长度（用于单行插入的undo）
    insertedLength = static_cast<int>(text.length());

    // 调用TextEngine插入文本
    textEngine->insert(lines, pos.line, pos.column, text);
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
        int targetRow = pos.line - 1;  // 原始行号（0-based）

        // 检查边界
        if (targetRow >= static_cast<int>(lines.size())) {
            return; // 行号越界
        }

        // 重建原始行：beforeInsert + afterInsert
        std::string originalLine = beforeInsert + afterInsert;

        // 删除新增的行（从targetRow开始，删除addedLines行）
        if (targetRow + addedLines <= static_cast<int>(lines.size())) {
            // 删除从targetRow开始的addedLines行
            lines.erase(lines.begin() + targetRow, lines.begin() + targetRow + addedLines);

            // 插入原始行
            lines.insert(lines.begin() + targetRow, originalLine);
        }
    } else {
        // 单行插入的撤销：删除插入的文本
        if (insertedLength > 0) {
            textEngine->deleteText(lines, pos.line, pos.column, insertedLength);
        }
    }
}

// DeleteCommand实现
DeleteCommand::DeleteCommand(std::vector<std::string>& lines, TextEngine* engine,
                             Position pos, int length)
    : TextCommand(lines, engine)
    , pos(pos)
    , length(length)
    , executed(false) {
}

DeleteCommand::DeleteCommand(std::vector<std::string>& lines, TextEngine* engine,
                             int row, int col, int length)
    : DeleteCommand(lines, engine, Position(row, col), length) {
}

void DeleteCommand::execute() {
    if (executed) {
        return; // 防止重复执行
    }

    // 在执行删除前，先记录被删除的文本
    recordDeletedText(pos.line, pos.column, length, deletedText);

    // 执行删除
    textEngine->deleteText(lines, pos.line, pos.column, length);
    executed = true;
}

void DeleteCommand::undo() {
    // 撤销删除：将删除的文本插回原处
    if (!deletedText.empty()) {
        textEngine->insert(lines, pos.line, pos.column, deletedText);
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
    : lines_(lines)
    , textEngine_(engine)
    , outputService_(outputService)
    , startLine_(startLine)
    , endLine_(endLine) {
    if (!outputService_) {
        throw std::invalid_argument("ShowCommand: OutputService cannot be null");
    }
}

void ShowCommand::execute() {
    // 调用TextEngine显示文本
    result_ = textEngine_->show(lines_, startLine_, endLine_);

    // 使用OutputService输出结果
    outputService_->outputText(result_);
}

void ShowCommand::undo() {
    // ShowCommand是只读命令，undo什么也不做
}

std::string ShowCommand::getResult() const {
    return result_;
}

// ReplaceCommand实现
ReplaceCommand::ReplaceCommand(std::vector<std::string>& lines, TextEngine* engine,
                               Position pos, int deleteLength, const std::string& replaceText)
    : TextCommand(lines, engine)
    , pos(pos)
    , deleteLength(deleteLength)
    , replaceText(replaceText)
    , executed(false) {
}

ReplaceCommand::ReplaceCommand(std::vector<std::string>& lines, TextEngine* engine,
                               int row, int col, int deleteLength, const std::string& replaceText)
    : ReplaceCommand(lines, engine, Position(row, col), deleteLength, replaceText) {
}

void ReplaceCommand::execute() {
    if (executed) {
        return; // 防止重复执行
    }

    // 先记录被删除的文本
    recordDeletedText(pos.line, pos.column, deleteLength, deletedText);

    // 执行删除
    textEngine->deleteText(lines, pos.line, pos.column, deleteLength);

    // 再执行插入（替换文本可以为空）
    if (!replaceText.empty()) {
        textEngine->insert(lines, pos.line, pos.column, replaceText);
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
            textEngine->deleteText(lines, pos.line, pos.column, replaceLength);
        }

        // 恢复被删除的文本
        if (!deletedText.empty()) {
            textEngine->insert(lines, pos.line, pos.column, deletedText);
        }

        executed = false;
    }
}
