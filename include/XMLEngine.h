#ifndef XMLENGINE_H
#define XMLENGINE_H

#include "Model.h"
#include "XmlDocumentWrapper.h"
#include <string>

// 前向声明
class XmlEditor;

// XMLEngine：XML文件的具体操作引擎（继承自Model基类）
// 当前为框架结构，后续将实现具体的XML操作（增删改查等）
class XMLEngine : public Model {
public:
    explicit XMLEngine(XmlEditor* editor);
    ~XMLEngine() override = default;

    // Model接口重写
    void validate() const override;
    bool isValid() const override;

    // 通过ID查找节点
    pugi::xml_node findNodeById(const std::string& id) const;

    // 检查XML文档是否已加载
    bool isDocumentLoaded() const;

protected:
    void handleException(const std::exception& e) const override;

private:
    XmlEditor* editor_;  // 不持有所有权
};

#endif // XMLENGINE_H
