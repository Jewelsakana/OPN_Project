#ifndef ISPELLCHECKER_H
#define ISPELLCHECKER_H

#include "TextSegment.h"
#include <vector>

// ISpellChecker：拼写检查适配器接口（适配器模式目标接口）
// 编辑器依赖此接口而非具体实现，支持方便切换不同拼写检查服务
class ISpellChecker {
public:
    virtual ~ISpellChecker() = default;

    // 检查文本片段，返回拼写检查结果列表
    virtual std::vector<SpellCheckResult> checkText(const TextSegment& segment) = 0;
};

#endif // ISPELLCHECKER_H
