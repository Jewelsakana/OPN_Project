#include "HttpSpellCheckerAdapter.h"
#include <cctype>
#include <sstream>
#include <cstdio>
#include <stdexcept>

namespace {

std::string urlEncode(const std::string& str) {
    std::ostringstream result;
    for (unsigned char c : str) {
        if (std::isalnum(static_cast<int>(c)) || c == '-' || c == '_' || c == '.' || c == '~') {
            result << c;
        } else if (c == ' ') {
            result << "%20";
        } else {
            char hex[4];
            std::snprintf(hex, sizeof(hex), "%%%02X", c);
            result << hex;
        }
    }
    return result.str();
}

int extractInt(const std::string& json, const std::string& key, size_t startPos) {
    size_t keyPos = json.find(key, startPos);
    if (keyPos == std::string::npos) return 0;
    size_t numStart = keyPos + key.size();
    while (numStart < json.size() &&
           (json[numStart] == ' ' || json[numStart] == ':' || json[numStart] == '\t')) {
        ++numStart;
    }
    std::string numStr;
    while (numStart < json.size() &&
           (std::isdigit(static_cast<unsigned char>(json[numStart])) || json[numStart] == '-')) {
        numStr += json[numStart];
        ++numStart;
    }
    return numStr.empty() ? 0 : std::stoi(numStr);
}

void parseMatchObject(const std::string& objStr, const TextSegment& segment,
                      std::vector<SpellCheckResult>& results) {
    int offset = extractInt(objStr, "\"offset\"", 0);
    int length = extractInt(objStr, "\"length\"", 0);

    std::string original;
    if (offset >= 0 && length > 0 && static_cast<size_t>(offset + length) <= segment.text.size()) {
        original = segment.text.substr(static_cast<size_t>(offset), static_cast<size_t>(length));
    }

    std::vector<std::string> suggestions;
    size_t valSearchPos = 0;
    while (true) {
        size_t valPos = objStr.find("\"value\":", valSearchPos);
        if (valPos == std::string::npos) break;

        size_t quote1 = objStr.find('"', valPos + 8);
        if (quote1 == std::string::npos) break;
        size_t quote2 = objStr.find('"', quote1 + 1);
        if (quote2 == std::string::npos) break;

        suggestions.push_back(objStr.substr(quote1 + 1, quote2 - quote1 - 1));
        valSearchPos = quote2 + 1;
    }

    SpellCheckResult result;
    result.line      = segment.line;
    result.column    = segment.column + offset;
    result.elementId = segment.elementId;
    result.original  = original;
    result.suggestions = suggestions;
    results.push_back(result);
}

void parseResponseBody(const std::string& json, const TextSegment& segment,
                       std::vector<SpellCheckResult>& results) {
    size_t matchesPos = json.find("\"matches\":");
    if (matchesPos == std::string::npos) return;

    size_t arrayStart = json.find('[', matchesPos);
    if (arrayStart == std::string::npos) return;

    size_t pos = json.find('{', arrayStart);
    size_t arrayEnd = json.find(']', arrayStart);

    while (pos != std::string::npos && pos < arrayEnd) {
        int depth = 0;
        size_t endPos = pos;
        for (size_t i = pos; i < json.size(); ++i) {
            if (json[i] == '{') ++depth;
            else if (json[i] == '}') {
                --depth;
                if (depth == 0) { endPos = i; break; }
            }
        }
        parseMatchObject(json.substr(pos, endPos - pos + 1), segment, results);
        pos = json.find('{', endPos + 1);
    }
}

} // anonymous namespace

HttpSpellCheckerAdapter::HttpSpellCheckerAdapter(const std::string& apiUrl)
    : apiUrl_(apiUrl) {}

std::vector<SpellCheckResult> HttpSpellCheckerAdapter::checkText(const TextSegment& segment) {
    if (segment.text.empty()) return {};

    std::string requestBody = buildRequest(segment.text);
    std::string response = httpClient_.post(apiUrl_, requestBody);
    return parseResponse(response, segment);
}

std::string HttpSpellCheckerAdapter::buildRequest(const std::string& text, const std::string& language) const {
    return "language=" + language + "&text=" + urlEncode(text);
}

std::vector<SpellCheckResult> HttpSpellCheckerAdapter::parseResponse(
    const std::string& responseBody, const TextSegment& segment) const {

    std::vector<SpellCheckResult> results;
    parseResponseBody(responseBody, segment, results);
    return results;
}
