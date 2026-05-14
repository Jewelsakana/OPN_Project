#include "TextEditor.h"
#include "TextEngine.h"
#include "TextCommands.h"
#include "EditorFactory.h"
#include "CommandFactory.h"

namespace {
    REGISTER_EDITOR(".txt", TextEditor)
}

TextEditor::TextEditor()
    : modified(false)
    , textEngine(nullptr) {
}

TextEditor::~TextEditor() {
}

const std::vector<std::string>& TextEditor::getLines() const {
    return lines;
}

std::vector<std::string>& TextEditor::getLinesRef() {
    return lines;
}

bool TextEditor::isModified() const {
    return modified;
}

void TextEditor::setModified(bool modified) {
    this->modified = modified;
}

std::shared_ptr<TextEngine> TextEditor::getTextEngine() const {
    return textEngine;
}

void TextEditor::setTextEngine(std::shared_ptr<TextEngine> engine) {
    textEngine = engine;
}

void TextEditor::executeCommand(std::unique_ptr<Command> command) {
    try {
        commandManager.executeCommand(std::move(command));
        setModified(true);  // 命令执行成功，标记为已修改
    } catch (const std::exception& e) {
        // 重新抛出异常，让上层处理
        throw;
    }
}

void TextEditor::undo() {
    commandManager.undo();
    // 可以根据需要更新修改状态
}

void TextEditor::redo() {
    commandManager.redo();
    setModified(true);  // 重做也视为修改
}

bool TextEditor::canUndo() const {
    return commandManager.canUndo();
}

bool TextEditor::canRedo() const {
    return commandManager.canRedo();
}

void TextEditor::setLines(const std::vector<std::string>& newLines) {
    lines = newLines;
    setModified(false); // 加载文件时视为未修改
}

bool TextEditor::supportsCommand(CommandTypeId type) const {
    switch (static_cast<EditorCommandType>(type)) {
        case EditorCommandType::Append:
        case EditorCommandType::Insert:
        case EditorCommandType::Delete:
        case EditorCommandType::Replace:
        case EditorCommandType::Show:
            return true;
        default:
            // 插件注册的类型（ID >= 1000），TextEditor 默认不支持
            return false;
    }
}

void TextEditor::loadFromData(const std::string& content) {
    lines = textEngine->stringToLines(content);
    setModified(false);
}

std::string TextEditor::saveToData() const {
    return textEngine->linesToString(lines);
}

void TextEditor::initContent(bool withLog) {
    lines = textEngine->initContent(withLog);
    setModified(true);
}

std::vector<TextSegment> TextEditor::getTextsToCheck() const {
    return textEngine->getTextsToCheck(lines);
}

void TextEditor::populateContext(EditorCommandContext& ctx) {
    ctx.lines = &lines;
    ctx.textEngine = textEngine.get();
}

void TextEditor::initialize() {
    if (!textEngine) {
        textEngine = std::make_shared<TextEngine>();
    }
}

void TextEditor::clear() {
    lines.clear();
    lines.push_back(""); // 保持至少一个空行
    setModified(false);
}