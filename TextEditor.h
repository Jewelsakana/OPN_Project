#ifndef TEXTEDITOR_H
#define TEXTEDITOR_H

#include "Editor.h"
#include <vector>
#include <string>
#include <memory>

// 前向声明
class TextEngine;

// TextEditor类:使用行数组存储文本，每个元素是一行，需要标记 modified 状态
class TextEditor : public Editor {
public:
    // 构造函数
    TextEditor();

    // 析构函数
    ~TextEditor();

    // 获取行数组
    const std::vector<std::string>& getLines() const;

    // 获取修改状态
    bool isModified() const;

    // 设置修改状态
    void setModified(bool modified);

    // 获取关联的TextEngine
    std::shared_ptr<TextEngine> getTextEngine() const;

    // 设置TextEngine
    void setTextEngine(std::shared_ptr<TextEngine> engine);

    // 其他方法将在后续实现...

private:
    std::vector<std::string> lines={""};    // 行数组，每个元素是一行文本
    bool modified;                      // 修改状态标记
    std::shared_ptr<TextEngine> textEngine; // TextEngine用于逻辑计算
};

#endif // TEXTEDITOR_H