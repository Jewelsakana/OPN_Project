#include "MockSpellChecker.h"
#include <cctype>

MockSpellChecker::MockSpellChecker() {
    // 常见拼写错误映射表
    dictionary_["recieve"]  = {"receive"};
    dictionary_["occured"]  = {"occurred"};
    dictionary_["occuring"] = {"occurring"};
    dictionary_["adress"]   = {"address"};
    dictionary_["accomodate"] = {"accommodate"};
    dictionary_["acheive"]  = {"achieve"};
    dictionary_["arguement"] = {"argument"};
    dictionary_["begining"] = {"beginning"};
    dictionary_["beleive"]  = {"believe"};
    dictionary_["calender"] = {"calendar"};
    dictionary_["definately"] = {"definitely"};
    dictionary_["embarass"] = {"embarrass"};
    dictionary_["enviroment"] = {"environment"};
    dictionary_["goverment"] = {"government"};
    dictionary_["immediatly"] = {"immediately"};
    dictionary_["independant"] = {"independent"};
    dictionary_["Itallian"] = {"Italian"};
    dictionary_["Rowlling"] = {"Rowling"};
    dictionary_["lisence"]  = {"license"};
    dictionary_["maintainance"] = {"maintenance"};
    dictionary_["neccessary"] = {"necessary"};
    dictionary_["occassion"] = {"occasion"};
    dictionary_["paralell"] = {"parallel"};
    dictionary_["reciept"]  = {"receipt"};
    dictionary_["seperate"] = {"separate"};
    dictionary_["tommorow"] = {"tomorrow"};
    dictionary_["untill"]   = {"until"};
    dictionary_["wierd"]    = {"weird"};
}

std::vector<MockSpellChecker::WordPosition> MockSpellChecker::splitWords(const std::string& text) {
    std::vector<WordPosition> words;
    std::string current;
    int wordStart = -1;

    for (size_t i = 0; i <= text.size(); ++i) {
        char c = (i < text.size()) ? text[i] : ' ';
        if (std::isalpha(static_cast<unsigned char>(c))) {
            if (wordStart == -1) {
                wordStart = static_cast<int>(i);
            }
            current += c;
        } else {
            if (!current.empty()) {
                words.push_back({current, wordStart});
                current.clear();
                wordStart = -1;
            }
        }
    }
    return words;
}

std::vector<SpellCheckResult> MockSpellChecker::checkText(const TextSegment& segment) {
    std::vector<SpellCheckResult> results;
    auto words = splitWords(segment.text);

    for (const auto& wp : words) {
        // 查找原始单词（保留大小写敏感匹配）
        std::string lookup = wp.word;
        auto it = dictionary_.find(lookup);
        if (it == dictionary_.end()) {
            // 尝试首字母大写匹配
            if (!lookup.empty() && std::isupper(static_cast<unsigned char>(lookup[0]))) {
                std::string lower = lookup;
                lower[0] = static_cast<char>(std::tolower(static_cast<unsigned char>(lower[0])));
                it = dictionary_.find(lower);
            }
        }
        // 尝试全小写匹配
        if (it == dictionary_.end()) {
            std::string lower = lookup;
            for (auto& ch : lower) {
                ch = static_cast<char>(std::tolower(static_cast<unsigned char>(ch)));
            }
            it = dictionary_.find(lower);
            if (it != dictionary_.end()) {
                lookup = lower; // 使用小写形式作为 original
            }
        }

        if (it != dictionary_.end()) {
            SpellCheckResult result;
            result.line      = segment.line;
            result.column    = segment.column + wp.position;
            result.elementId = segment.elementId;
            result.original  = lookup;
            result.suggestions = it->second;
            results.push_back(result);
        }
    }

    return results;
}
