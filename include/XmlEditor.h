#ifndef XMLEDITOR_H
#define XMLEDITOR_H

#include "Editor.h"
#include "CommandManager.h"
#include "XmlDocumentWrapper.h"
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>

// 前向声明
class XMLEngine;

// XmlEditor：XML文件编辑器，派生自Editor
// 解析XML文件为树形结构（DOM树），维护id到节点的快速映射
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

    // 获取关联的XMLEngine
    XMLEngine* getXMLEngine() const;

    // 获取文档访问器
    XmlDocumentWrapper& getDocument();
    const XmlDocumentWrapper& getDocument() const;

    // 加载XML文件并构建ID映射
    void loadFromFile(const std::string& filePath);

    // 从字符串加载XML并构建ID映射
    void loadFromString(const std::string& xmlContent);

    // 保存XML到文件
    void saveToFile(const std::string& filePath);

    // ID映射：查找节点
    pugi::xml_node findNodeById(const std::string& id) const;

    // 检查ID是否存在
    bool hasNodeWithId(const std::string& id) const;

    // 获取所有节点ID列表
    std::vector<std::string> getAllIds() const;

    // 重建ID映射（文档修改后调用）
    void rebuildIdMapping();

    // 清空编辑器内容
    void clear();

private:
    // 构建ID到节点的映射，验证唯一性
    void buildIdMapping();

    XmlDocumentWrapper document_;
    std::shared_ptr<XMLEngine> xmlEngine_;
    CommandManager commandManager_;
    bool modified_;

    // ID到XML节点的快速映射
    std::unordered_map<std::string, pugi::xml_node> idToNodeMap_;
};

#endif // XMLEDITOR_H
