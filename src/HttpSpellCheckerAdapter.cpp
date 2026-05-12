#include "HttpSpellCheckerAdapter.h"
#include <stdexcept>

HttpSpellCheckerAdapter::HttpSpellCheckerAdapter(const std::string& apiUrl)
    : apiUrl_(apiUrl) {}

std::vector<SpellCheckResult> HttpSpellCheckerAdapter::checkText(const TextSegment& segment) {
    // 骨架实现：构建请求并解析响应
    // 当前预留 HTTP 客户端调用结构
    throw std::runtime_error("HttpSpellCheckerAdapter: HTTP client not yet implemented");
}

std::string HttpSpellCheckerAdapter::buildRequest(const std::string& text, const std::string& language) const {
    // 构建 LanguageTool API v2 请求体
    // 格式: language=<lang>&text=<url-encoded text>
    // 当前为骨架
    return "";
}

std::vector<SpellCheckResult> HttpSpellCheckerAdapter::parseResponse(
    const std::string& responseBody, const TextSegment& segment) const {
    // 解析 LanguageTool API JSON 响应
    // matches[].message, matches[].replacements[].value, matches[].offset, matches[].length
    // 当前为骨架
    return {};
}
