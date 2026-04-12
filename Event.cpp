#include "Event.h"

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