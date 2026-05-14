#ifndef LOGCOORDINATOR_H
#define LOGCOORDINATOR_H

#include <string>
#include <vector>

class LoggerManager;
class OutputService;
class ObserverManager;

// LogCoordinator：日志启停和查询
class LogCoordinator {
public:
    LogCoordinator(LoggerManager& lm, OutputService& out);

    void startLoggingForFile(const std::string& fileName);
    void stopLoggingForFile(const std::string& fileName);
    bool isLoggingForFile(const std::string& fileName) const;
    void showLog(const std::string& fileName, const std::string& activeFileName = "");
    std::vector<std::string> getLoggedFiles() const;

    void setLogEnabled(bool enabled);
    bool isLogEnabled() const;

private:
    LoggerManager& loggerManager_;
    OutputService& outputService_;
    bool logEnabled_ = false;
};

#endif // LOGCOORDINATOR_H
