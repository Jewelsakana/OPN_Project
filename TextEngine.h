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

// 行号越界异常
class RowOutOfBoundsException : public TextEngineException {
public:
    explicit RowOutOfBoundsException(int row, int maxRow)
        : TextEngineException("Row " + std::to_string(row) + " out of bounds. Maximum row is " + std::to_string(maxRow)) {}
};

// 列号越界异常
class ColumnOutOfBoundsException : public TextEngineException {
public:
    explicit ColumnOutOfBoundsException(int col, int maxCol)
        : TextEngineException("Column " + std::to_string(col) + " out of bounds. Maximum column is " + std::to_string(maxCol)) {}
};

// 空字符串插入异常
class EmptyStringInsertionException : public TextEngineException {
public:
    explicit EmptyStringInsertionException()
        : TextEngineException("Cannot insert empty string at this position") {}
};

// 删除长度超出行尾异常
class DeleteLengthExceedsLineException : public TextEngineException {
public:
    explicit DeleteLengthExceedsLineException(int length, int remaining)
        : TextEngineException("Delete length " + std::to_string(length) + " exceeds remaining characters " + std::to_string(remaining) + " in line") {}
};

// TextEngine类：作为实现复杂的字符串算法的类，不持有数据，仅负责算法
class TextEngine {
public:
    TextEngine() = default;
    ~TextEngine() = default;

    // 追加文本到行数组末尾
    // 参数：lines - 行数组引用，text - 要追加的文本
    // 注意：text中可以包含换行符('\n')，将自动拆分为多行
    void append(std::vector<std::string>& lines, const std::string& text);

    // 在指定位置插入文本
    // 参数：lines - 行数组引用，row - 行号（1-based），col - 列号（1-based），text - 要插入的文本
    // 注意：text中可以包含换行符('\n')，将自动拆分为多行
    // 异常：行号或列号越界时抛出相应异常，空字符串插入时抛出异常
    void insert(std::vector<std::string>& lines, int row, int col, const std::string& text);

    // 在指定位置删除字符串
    // 参数：lines - 行数组引用，row - 行号（1-based），col - 列号（1-based），length - 要删除的字符数
    // 注意：删除范围不可以跨行
    // 异常：行号或列号越界时抛出相应异常，删除长度超出该行剩余字符时抛出异常
    void deleteText(std::vector<std::string>& lines, int row, int col, int length);

    // 显示字符串
    // 参数：lines - 行数组引用，startLine - 起始行号（0-based，默认0），endLine - 结束行号（0-based，默认-1表示到末尾）
    // 返回：用换行符连接指定范围行的字符串
    std::string show(const std::vector<std::string>& lines, int startLine = 0, int endLine = -1) const;

    // 辅助方法：将1-based行列号转换为0-based索引，并进行边界检查
    void validateAndConvertIndices(const std::vector<std::string>& lines, int& row, int& col, bool checkColBounds = true) const;

    // 辅助方法：分割包含换行符的文本为多行
    std::vector<std::string> splitTextByNewlines(const std::string& text) const;

private:
    // 其他私有辅助方法...
};

#endif // TEXTENGINE_H