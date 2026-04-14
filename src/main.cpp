#include "WorkSpace.h"
#include "CommandController.h"
#include "OutputService.h"
#include <iostream>
#include <string>
#include <stdexcept>

#ifdef _WIN32
#include <windows.h>
#endif


int main() {
    try {
#ifdef _WIN32
        // 设置控制台编码为UTF-8
        SetConsoleOutputCP(CP_UTF8);
        SetConsoleCP(CP_UTF8);
#endif
        // 创建工作区
        WorkSpace workspace;

        // 创建命令控制器
        CommandController controller(&workspace);

        // 主循环
        std::string input;

        std::cout << "Text Editor System - Type 'exit' to quit" << std::endl;

        while (!workspace.isExitRequested()) {
            // 显示提示符
            std::cout << ">> ";

            // 读取用户输入
            if (!std::getline(std::cin, input)) {
                // EOF或读取错误
                break;
            }

            // 空输入，继续下一次循环
            if (input.empty()) {
                continue;
            }

            try {
                // 解析并执行命令
                controller.parseAndExecuteCommand(input);
            } catch (const std::exception& e) {
                // 异常已经被OutputService处理过，这里可以忽略
                // 如果需要，可以显示错误信息：std::cerr << "Error: " << e.what() << std::endl;
            }
        }

        std::cout << "Goodbye!" << std::endl;
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}