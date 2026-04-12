#include "TextEditor.h"
#include "TextEngine.h"

TextEditor::TextEditor()
    : modified(false)
    , textEngine(nullptr) {
}

TextEditor::~TextEditor() {
}

const std::vector<std::string>& TextEditor::getLines() const {
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