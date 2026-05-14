#ifndef TEXTSEGMENT_H
#define TEXTSEGMENT_H

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

#endif // TEXTSEGMENT_H
