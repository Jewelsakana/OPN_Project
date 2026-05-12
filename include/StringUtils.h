#ifndef STRINGUTILS_H
#define STRINGUTILS_H

#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
#include <sstream>

namespace StringUtils {

std::string trim(const std::string& str);

std::vector<std::string> splitString(const std::string& str, char delimiter);

bool startsWith(const std::string& str, const std::string& prefix);

std::string toLower(const std::string& str);

// 格式化编辑时长为可读字符串
// < 1分钟: "X秒"  |  1-59分钟: "X分钟"  |  1-23小时: "X小时Y分钟"  |  ≥ 24小时: "X天Y小时"
std::string formatDuration(int totalSeconds);

} // namespace StringUtils

#endif // STRINGUTILS_H
