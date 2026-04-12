#ifndef TEXTENGINE_H
#define TEXTENGINE_H

#include <vector>
#include <string>
#include <stdexcept>

// 自定义异常类，当TextEngine检测到非法操作时抛出
class TextEngineException : public std::runtime_error {
public:
    explicit TextEngineException(const std::string& message)
        : std::runtime_error(message) {}
};

// TextEngine类：作为实现复杂的字符串算法的类，不持有数据，仅负责算法
class TextEngine {
public:
    TextEngine() = default;
    ~TextEngine() = default;

    // 在指定行末尾追加文本
    // 参数：lines - 只读的行数组引用，row - 行号（0-based），text - 要追加的文本
    // 返回：操作后的新行数组
    std::vector<std::string> append(const std::vector<std::string>& lines, int row, const std::string& text);

    // 在指定位置插入文本
    // 参数：lines - 只读的行数组引用，row - 行号（0-based），col - 列号（0-based），text - 要插入的文本
    // 返回：操作后的新行数组
    std::vector<std::string> insert(const std::vector<std::string>& lines, int row, int col, const std::string& text);

    // 删除指定位置的文本
    // 参数：lines - 只读的行数组引用，row - 行号（0-based），col - 列号（0-based），length - 要删除的字符数
    // 返回：操作后的新行数组
    std::vector<std::string> deleteText(const std::vector<std::string>& lines, int row, int col, int length);

    // 显示文本（可能返回格式化后的字符串）
    // 参数：lines - 只读的行数组引用
    // 返回：格式化后的文本字符串
    std::string show(const std::vector<std::string>& lines) const;

    // 其他算法方法将在后续实现...
};

#endif // TEXTENGINE_H