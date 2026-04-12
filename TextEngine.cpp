#include "TextEngine.h"
#include <algorithm>
#include <sstream>
#include <iostream>

// 辅助方法：分割包含换行符的文本为多行
std::vector<std::string> TextEngine::splitTextByNewlines(const std::string& text) const {
    std::vector<std::string> result;

    if (text.empty()) {
        return result;
    }

    size_t start = 0;
    size_t end = text.find('\n');

    while (end != std::string::npos) {
        result.push_back(text.substr(start, end - start));
        start = end + 1;
        end = text.find('\n', start);
    }

    // 添加最后一部分（最后一个换行符之后的部分）
    result.push_back(text.substr(start));

    return result;
}

// 辅助方法：将1-based行列号转换为0-based索引，并进行边界检查
void TextEngine::validateAndConvertIndices(const std::vector<std::string>& lines, int& row, int& col, bool checkColBounds) const {
    // 转换为0-based索引
    row = row - 1;
    col = col - 1;

    // 检查行号边界
    if (row < 0 || row >= static_cast<int>(lines.size())) {
        throw RowOutOfBoundsException(row + 1, lines.size());
    }

    // 检查列号边界
    if (checkColBounds) {
        int lineLength = static_cast<int>(lines[row].length());
        if (col < 0 || col > lineLength) {
            throw ColumnOutOfBoundsException(col + 1, lineLength + 1); // +1是因为可以在行尾插入
        }
    }
}

// 追加文本到行数组末尾
void TextEngine::append(std::vector<std::string>& lines, const std::string& text) {
    if (text.empty()) {
        // 空文本，不执行任何操作
        return;
    }

    // 分割文本为多行
    std::vector<std::string> newLines = splitTextByNewlines(text);

    if (newLines.empty()) {
        // 如果文本只包含换行符，添加一个空行
        lines.push_back("");
    } else {
        // 将新行追加到现有行数组
        lines.insert(lines.end(), newLines.begin(), newLines.end());
    }
}

// 在指定位置插入文本
void TextEngine::insert(std::vector<std::string>& lines, int row, int col, const std::string& text) {
    // 特殊处理：lines为空的情况
    if (lines.empty()) {
        // lines为空时，只能插入在位置(1,1)
        if (row != 1 || col != 1) {
            throw RowOutOfBoundsException(row, 0);  // 最大行是0
        }

        // 分割文本为多行（即使是空文本）
        std::vector<std::string> newLines = splitTextByNewlines(text);

        if (newLines.empty()) {
            // 插入空字符串或只包含换行符的文本，创建一个空行
            lines.push_back("");
        } else {
            // 插入非空文本，将新行添加到lines
            lines = newLines;
        }
        return;
    }

    // 验证并转换索引（lines不为空）
    validateAndConvertIndices(lines, row, col);

    // 分割文本为多行
    std::vector<std::string> newLines = splitTextByNewlines(text);

    // 处理空文本的情况
    if (newLines.empty()) {
        // 空文本什么也不做
        return;
    }

    // 处理插入
    if (newLines.size() == 1) {
        // 单行插入
        std::string& currentLine = lines[row];
        currentLine.insert(col, newLines[0]);
    } else {
        // 多行插入（包含换行符）
        std::string& currentLine = lines[row];

        // 分割当前行为两部分：插入点前和插入点后
        std::string beforeInsert = currentLine.substr(0, col);
        std::string afterInsert = currentLine.substr(col);

        // 构建新的行数组
        std::vector<std::string> updatedLines;

        // 添加插入点之前的行
        for (int i = 0; i < row; ++i) {
            updatedLines.push_back(lines[i]);
        }

        // 第一行：插入点前的部分 + 新文本的第一行
        updatedLines.push_back(beforeInsert + newLines[0]);

        // 中间的新行（如果有）
        for (size_t i = 1; i < newLines.size() - 1; ++i) {
            updatedLines.push_back(newLines[i]);
        }

        // 最后一行：新文本的最后一行 + 插入点后的部分
        updatedLines.push_back(newLines[newLines.size() - 1] + afterInsert);

        // 添加插入点之后的行
        for (size_t i = row + 1; i < lines.size(); ++i) {
            updatedLines.push_back(lines[i]);
        }

        // 替换原始行数组
        lines = updatedLines;
    }
}

// 在指定位置删除字符串
void TextEngine::deleteText(std::vector<std::string>& lines, int row, int col, int length) {
    // 验证并转换索引
    validateAndConvertIndices(lines, row, col);

    // 获取当前行
    std::string& currentLine = lines[row];
    int lineLength = static_cast<int>(currentLine.length());

    // 检查删除长度是否超出该行剩余字符数
    int remainingChars = lineLength - col;
    if (length > remainingChars) {
        throw DeleteLengthExceedsLineException(length, remainingChars);
    }

    // 执行删除
    currentLine.erase(col, length);
}

// 显示字符串
std::string TextEngine::show(const std::vector<std::string>& lines, int startLine, int endLine) const {
    if (lines.empty()) {
        return "";
    }

    // 处理默认参数
    if (startLine < 0) startLine = 0;
    if (endLine < 0) endLine = static_cast<int>(lines.size()) - 1;

    // 边界检查
    if (startLine >= static_cast<int>(lines.size())) {
        throw RowOutOfBoundsException(startLine + 1, lines.size());
    }
    if (endLine >= static_cast<int>(lines.size())) {
        throw RowOutOfBoundsException(endLine + 1, lines.size());
    }
    if (startLine > endLine) {
        std::swap(startLine, endLine);
    }

    // 用换行符连接指定范围的行
    std::string result;
    for (int i = startLine; i <= endLine; ++i) {
        result += lines[i];
        if (i != endLine) {
            result += '\n';
        }
    }

    return result;
}