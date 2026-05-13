#ifndef HTTPSPELLCHECKERADAPTER_H
#define HTTPSPELLCHECKERADAPTER_H

#include "ISpellChecker.h"
#include "HttpClient.h"
#include <string>
#include <memory>

// HttpSpellCheckerAdapter：基于 LanguageTool HTTP API 的拼写检查适配器
// 职责：LanguageTool 协议（请求构建 + JSON 响应解析），HTTP 传输委托给 HttpClient
class HttpSpellCheckerAdapter : public ISpellChecker {
public:
    explicit HttpSpellCheckerAdapter(const std::string& apiUrl = "https://api.languagetool.org/v2/check");
    ~HttpSpellCheckerAdapter() override = default;

    std::vector<SpellCheckResult> checkText(const TextSegment& segment) override;

private:
    std::string apiUrl_;
    HttpClient httpClient_;

    std::string buildRequest(const std::string& text, const std::string& language = "en-US") const;
    std::vector<SpellCheckResult> parseResponse(const std::string& responseBody,
                                                 const TextSegment& segment) const;
};

#endif // HTTPSPELLCHECKERADAPTER_H
