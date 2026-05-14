#include "Event.h"
#include <sstream>
#include <iomanip>
#include <ctime>

Event::Event(const std::string& commandContent, const std::string& targetFileName)
    : timestamp(std::chrono::system_clock::now())
    , commandContent(commandContent)
    , targetFileName(targetFileName) {
}

Event::TimePoint Event::getTimestamp() const {
    return timestamp;
}

std::string Event::getCommandContent() const {
    return commandContent;
}

std::string Event::getTargetFileName() const {
    return targetFileName;
}

std::string Event::currentTimestampString() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&time);
    std::stringstream ss;
    ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    return ss.str();
}