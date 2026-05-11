#ifndef XMLENGINE_H
#define XMLENGINE_H

#include "Model.h"
#include <string>

// 前向声明
class XmlEditor;

// XMLEngine：XML文件的具体操作引擎（继承自Model基类）
// 通过字符串接口操作XML，不依赖具体XML解析库
class XMLEngine : public Model {
public:
    explicit XMLEngine(XmlEditor* editor);
    ~XMLEngine() override = default;

    // Model接口重写
    void validate() const override;
    bool isValid() const override;

    // ID查询（基于字符串接口）
    bool hasNodeWithId(const std::string& id) const;
    std::string getNodeName(const std::string& id) const;
    std::string getNodeValue(const std::string& id) const;
    std::string getNodeAttribute(const std::string& id, const std::string& attrName) const;

    // 检查XML文档是否已加载
    bool isDocumentLoaded() const;

protected:
    void handleException(const std::exception& e) const override;

private:
    XmlEditor* editor_;  // 不持有所有权
};

#endif // XMLENGINE_H
