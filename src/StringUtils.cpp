#include "StringUtils.h"

namespace StringUtils {

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (std::string::npos == first) {
        return "";
    }
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

std::vector<std::string> splitString(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

bool startsWith(const std::string& str, const std::string& prefix) {
    if (str.length() < prefix.length()) return false;
    return std::equal(prefix.begin(), prefix.end(), str.begin());
}

std::string toLower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string formatDuration(int totalSeconds) {
    if (totalSeconds < 0) totalSeconds = 0;

    if (totalSeconds < 60) {
        return std::to_string(totalSeconds) + "秒";
    }

    int minutes = totalSeconds / 60;
    if (minutes < 60) {
        return std::to_string(minutes) + "分钟";
    }

    int hours = minutes / 60;
    minutes = minutes % 60;
    if (hours < 24) {
        if (minutes > 0) {
            return std::to_string(hours) + "小时" + std::to_string(minutes) + "分钟";
        }
        return std::to_string(hours) + "小时";
    }

    int days = hours / 24;
    hours = hours % 24;
    if (hours > 0) {
        return std::to_string(days) + "天" + std::to_string(hours) + "小时";
    }
    return std::to_string(days) + "天";
}

} // namespace StringUtils
