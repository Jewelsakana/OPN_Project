#ifndef EDITORFACTORY_H
#define EDITORFACTORY_H

#include "Editor.h"
#include <memory>
#include <string>
#include <functional>
#include <unordered_map>

// 编辑器创建器类型：无参数，返回unique_ptr<Editor>
using EditorCreator = std::function<std::unique_ptr<Editor>()>;

// EditorFactory：根据文件后缀返回对应的Editor实例
// 使用注册表模式，支持以插件形式扩展新的编辑器类型
// 避免大量if-else或switch-case语句
class EditorFactory {
public:
    // 根据文件后缀创建对应的Editor
    // 返回unique_ptr<Editor>，如无匹配的后缀则返回nullptr
    static std::unique_ptr<Editor> createEditor(const std::string& fileExtension);

    // 注册编辑器创建器（由各Editor类在启动时调用）
    static void registerEditor(const std::string& extension, EditorCreator creator);

private:
    // 获取注册表（函数内静态对象，避免静态初始化顺序问题）
    static std::unordered_map<std::string, EditorCreator>& registry();
};

// 自注册宏：统一Editor的注册模式
// 用法：在Editor子类的.cpp文件中使用，实现启动时自动注册
// 示例：REGISTER_EDITOR(".xml", XmlEditor)
#define REGISTER_EDITOR(EXT, CLASS) \
    static bool _reg_editor_##CLASS = []() { \
        EditorFactory::registerEditor(EXT, \
            []() -> std::unique_ptr<Editor> { \
                return std::make_unique<CLASS>(); \
            }); \
        return true; \
    }();

#endif // EDITORFACTORY_H
