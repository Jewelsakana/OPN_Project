#include "EditorFactory.h"

std::unique_ptr<Editor> EditorFactory::createEditor(const std::string& fileExtension) {
    auto& reg = registry();
    auto it = reg.find(fileExtension);
    if (it != reg.end()) {
        return it->second();
    }
    return nullptr;
}

void EditorFactory::registerEditor(const std::string& extension, EditorCreator creator) {
    registry()[extension] = std::move(creator);
}

std::unordered_map<std::string, EditorCreator>& EditorFactory::registry() {
    static std::unordered_map<std::string, EditorCreator> reg;
    return reg;
}
