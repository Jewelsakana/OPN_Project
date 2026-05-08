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

} // namespace StringUtils

#endif // STRINGUTILS_H
