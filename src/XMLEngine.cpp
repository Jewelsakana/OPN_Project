#include "XMLEngine.h"
#include "XmlEditor.h"

XMLEngine::XMLEngine(XmlEditor* editor)
    : editor_(editor) {
}

void XMLEngine::validate() const {
    // XMLEngine的验证逻辑：检查关联的编辑器是否存在
}

bool XMLEngine::isValid() const {
    return editor_ != nullptr && editor_->getDocument().isLoaded();
}

pugi::xml_node XMLEngine::findNodeById(const std::string& id) const {
    if (!editor_) {
        throw XmlDocumentException("XMLEngine: No associated XmlEditor");
    }
    return editor_->findNodeById(id);
}

bool XMLEngine::isDocumentLoaded() const {
    return editor_ && editor_->getDocument().isLoaded();
}

void XMLEngine::handleException(const std::exception& e) const {
    // XMLEngine特定的异常处理，当前暂为空实现
}
