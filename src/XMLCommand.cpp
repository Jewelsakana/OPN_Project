#include "XMLCommand.h"
#include "XmlEditor.h"
#include "XMLEngine.h"

XMLCommand::XMLCommand(XmlEditor* editor)
    : editor_(editor) {
}

XMLEngine* XMLCommand::getEngine() const {
    return editor_ ? editor_->getXMLEngine() : nullptr;
}

XmlEditor* XMLCommand::getEditor() const {
    return editor_;
}
