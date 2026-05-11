#ifndef XMLEDITOR_H
#define XMLEDITOR_H

#include "Editor.h"
#include "CommandManager.h"
#include "IXmlDocument.h"
#include <memory>
#include <string>
#include <vector>

// XmlEditor：XML文件编辑器，派生自Editor
// 通过IXmlDocument接口依赖XML文档，不耦合具体解析库
class XmlEditor : public Editor {
public:
    XmlEditor();
    ~XmlEditor() override;

    // Editor接口实现
    void executeCommand(std::unique_ptr<Command> command) override;
    void undo() override;
    void redo() override;
    bool canUndo() const override;
    bool canRedo() const override;

    // 修改状态管理
    bool isModified() const;
    void setModified(bool modified);

    // 获取文档接口（返回抽象接口引用，不暴露具体实现）
    IXmlDocument& getDocument();
    const IXmlDocument& getDocument() const;

    // 加载XML文件并构建ID映射
    void loadFromFile(const std::string& filePath);

    // 从字符串加载XML并构建ID映射
    void loadFromString(const std::string& xmlContent);

    // 保存XML到文件
    void saveToFile(const std::string& filePath);

    // ID查询（基于字符串，不依赖具体XML库）
    bool hasNodeWithId(const std::string& id) const;
    std::string getNodeName(const std::string& id) const;
    std::string getNodeValue(const std::string& id) const;
    std::string getNodeAttribute(const std::string& id, const std::string& attrName) const;
    std::vector<std::string> getAllIds() const;

    // 检查是否支持某个命令类型
    bool supportsCommand(EditorCommandType type) const override;

    // 清空编辑器内容
    void clear();

private:
    std::unique_ptr<IXmlDocument> document_;
    CommandManager commandManager_;
    bool modified_;
};

#endif // XMLEDITOR_H
