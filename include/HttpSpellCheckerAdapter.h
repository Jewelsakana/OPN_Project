#ifndef HTTPSPELLCHECKERADAPTER_H
#define HTTPSPELLCHECKERADAPTER_H

#include "ISpellChecker.h"
#include <string>

// HttpSpellCheckerAdapter：基于 LanguageTool HTTP API 的拼写检查适配器
// 使用 https://dev.languagetool.org/public-http-api 进行拼写检查
// 当前为骨架实现，预留了 HTTP 请求结构和接口
class HttpSpellCheckerAdapter : public ISpellChecker {
public:
    explicit HttpSpellCheckerAdapter(const std::string& apiUrl = "https://api.languagetool.org/v2/check");
    ~HttpSpellCheckerAdapter() override = default;

    std::vector<SpellCheckResult> checkText(const TextSegment& segment) override;

private:
    std::string apiUrl_;

    // 构建 LanguageTool API 请求体
    std::string buildRequest(const std::string& text, const std::string& language = "en-US") const;

    // 解析 LanguageTool API 响应
    std::vector<SpellCheckResult> parseResponse(const std::string& responseBody,
                                                 const TextSegment& segment) const;
};

#endif // HTTPSPELLCHECKERADAPTER_H
