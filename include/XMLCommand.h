#ifndef XMLCOMMAND_H
#define XMLCOMMAND_H

#include "Command.h"

// 前向声明
class XmlEditor;
class XMLEngine;

// XMLCommand：XML编辑命令基类，派生自Command
// XMLCommand及其派生类可以通过getEngine()访问XmlEditor的XMLEngine
// 当前为框架结构，具体命令操作暂不实现
class XMLCommand : public Command {
public:
    explicit XMLCommand(XmlEditor* editor);
    ~XMLCommand() override = default;

    // 默认实现（子类应重写）
    void execute() override = 0;
    void undo() override = 0;

protected:
    // 获取关联的XMLEngine，便于派生类操作XML文档
    XMLEngine* getEngine() const;

    // 获取关联的XmlEditor
    XmlEditor* getEditor() const;

private:
    XmlEditor* editor_;  // 不持有所有权
};

#endif // XMLCOMMAND_H
