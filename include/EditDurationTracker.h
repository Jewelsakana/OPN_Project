#ifndef EDITDURATIONTRACKER_H
#define EDITDURATIONTRACKER_H

#include "Observe.h"
#include <string>
#include <unordered_map>
#include <chrono>

// EditDurationTracker：编辑时长统计器（观察者模式）
// 统计每个打开文件在当前会话中的累计编辑时长（秒）
class EditDurationTracker : public Observe {
public:
    EditDurationTracker();
    ~EditDurationTracker() override = default;

    // Observe接口：通过命令事件检测文件切换并计时
    void update(const Event& e) override;

    // 文件打开时初始化跟踪
    void onFileOpened(const std::string& fileName);

    // 文件关闭时停止跟踪并清除记录
    void onFileClosed(const std::string& fileName);

    // 获取文件累计编辑时长（秒），未跟踪返回 0
    int getDurationSeconds(const std::string& fileName) const;

    // 获取所有被跟踪文件的时长
    const std::unordered_map<std::string, int>& getAllDurations() const;

    // 重置所有跟踪数据
    void reset();

    // 获取当前活动文件名
    const std::string& getCurrentFile() const;

private:
    void recordElapsed();

    std::string currentFile_;
    std::chrono::steady_clock::time_point lastTick_;
    std::unordered_map<std::string, int> durationMap_;  // fileName -> 累计秒数
};

#endif // EDITDURATIONTRACKER_H
