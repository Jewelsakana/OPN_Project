#ifndef ISPELLCHECKER_H
#define ISPELLCHECKER_H

#include <string>
#include <vector>

// 待检查的文本片段（由 Editor 子类提供）
struct TextSegment {
    std::string text;        // 待检查的文本内容
    int line;                // 1-based 行号（XML 片段为 0）
    int column;              // 1-based 起始列号（XML 片段为 0）
    std::string elementId;   // XML 元素 ID（文本片段为空）
};

// 拼写检查结果（由适配器返回的统一结构体）
struct SpellCheckResult {
    int line;                            // 1-based 行号（XML 结果为 0）
    int column;                          // 1-based 列号（XML 结果为 0）
    std::string elementId;               // XML 元素 ID（文本结果为空）
    std::string original;                // 错误单词
    std::vector<std::string> suggestions; // 建议修正列表
};

// ISpellChecker：拼写检查适配器接口（适配器模式目标接口）
// 编辑器依赖此接口而非具体实现，支持方便切换不同拼写检查服务
class ISpellChecker {
public:
    virtual ~ISpellChecker() = default;

    // 检查文本片段，返回拼写检查结果列表
    virtual std::vector<SpellCheckResult> checkText(const TextSegment& segment) = 0;
};

#endif // ISPELLCHECKER_H
