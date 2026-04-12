#include "TextEditor.h"
#include "TextEngine.h"
#include "TextCommands.h"

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