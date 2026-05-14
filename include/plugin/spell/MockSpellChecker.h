#ifndef MOCKSPELLCHECKER_H
#define MOCKSPELLCHECKER_H

#include "ISpellChecker.h"
#include <unordered_map>
#include <vector>
#include <string>

// MockSpellChecker：Mock 拼写检查适配器
// 使用内置的常见拼写错误映射表进行测试，不依赖任何外部服务
class MockSpellChecker : public ISpellChecker {
public:
    MockSpellChecker();

    std::vector<SpellCheckResult> checkText(const TextSegment& segment) override;

private:
    // 常见拼写错误 -> 建议修正 映射表
    std::unordered_map<std::string, std::vector<std::string>> dictionary_;

    // 将文本拆分为单词，返回每个单词及其在文本中的位置
    struct WordPosition {
        std::string word;
        int position;  // 0-based 字符偏移
    };
    std::vector<WordPosition> splitWords(const std::string& text);
};

#endif // MOCKSPELLCHECKER_H
