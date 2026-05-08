#include "ConfigSerializer.h"
#include "WorkSpace.h"
#include "StringUtils.h"
#include <fstream>
#include <iostream>
#include <sstream>

void ConfigSerializer::saveConfig(const std::string& fileName, const WorkspaceMemento& memento) {
    safeExecute([this, &fileName, &memento]() {
        std::ofstream file(fileName);
        if (!file) {
            throw std::runtime_error("Unable to write config file: " + fileName);
        }

        const auto& openFiles = memento.getOpenFiles();
        file << "openFiles:";
        for (size_t i = 0; i < openFiles.size(); ++i) {
            file << openFiles[i];
            if (i != openFiles.size() - 1) file << ",";
        }
        file << "\n";

        file << "activeFileName: " << memento.getActiveFileName() << "\n";

        const auto& modifiedStates = memento.getFileModifiedStates();
        file << "fileModifiedStates:";
        size_t count = 0;
        for (const auto& pair : modifiedStates) {
            file << pair.first << "=" << (pair.second ? "true" : "false");
            if (++count < modifiedStates.size()) file << ",";
        }
        file << "\n";

        file << "logEnabled: " << (memento.isLogEnabled() ? "true" : "false") << "\n";

        const auto& loggedFiles = memento.getLoggedFiles();
        file << "loggedFiles:";
        for (size_t i = 0; i < loggedFiles.size(); ++i) {
            file << loggedFiles[i];
            if (i != loggedFiles.size() - 1) file << ",";
        }
        file << "\n";

        file.close();
    });
}

std::shared_ptr<WorkspaceMemento> ConfigSerializer::loadConfig(const std::string& fileName) {
    return safeExecute([this, &fileName]() -> std::shared_ptr<WorkspaceMemento> {
        std::ifstream file(fileName);
        if (!file) {
            return nullptr;
        }

        std::vector<std::string> openFiles;
        std::string activeFileName;
        std::map<std::string, bool> fileModifiedStates;
        bool logEnabled = false;
        std::vector<std::string> loggedFiles;

        std::string line;
        while (std::getline(file, line)) {
            line = StringUtils::trim(line);
            if (line.empty()) continue;

            size_t colonPos = line.find(':');
            if (colonPos == std::string::npos) continue;

            std::string key = StringUtils::trim(line.substr(0, colonPos));
            std::string value = StringUtils::trim(line.substr(colonPos + 1));

            if (key == "openFiles") {
                if (!value.empty()) {
                    auto files = StringUtils::splitString(value, ',');
                    for (const auto& f : files) {
                        std::string trimmed = StringUtils::trim(f);
                        if (!trimmed.empty()) openFiles.push_back(trimmed);
                    }
                }
            } else if (key == "activeFileName") {
                activeFileName = value;
            } else if (key == "fileModifiedStates") {
                if (!value.empty()) {
                    auto pairs = StringUtils::splitString(value, ',');
                    for (const auto& pair : pairs) {
                        std::string trimmed = StringUtils::trim(pair);
                        size_t eqPos = trimmed.find('=');
                        if (eqPos != std::string::npos) {
                            std::string fk = StringUtils::trim(trimmed.substr(0, eqPos));
                            std::string bv = StringUtils::trim(trimmed.substr(eqPos + 1));
                            fileModifiedStates[fk] = (bv == "true");
                        }
                    }
                }
            } else if (key == "logEnabled") {
                logEnabled = (value == "true");
            } else if (key == "loggedFiles") {
                if (!value.empty()) {
                    auto files = StringUtils::splitString(value, ',');
                    for (const auto& f : files) {
                        std::string trimmed = StringUtils::trim(f);
                        if (!trimmed.empty()) loggedFiles.push_back(trimmed);
                    }
                }
            }
        }

        file.close();
        return std::make_shared<WorkspaceMemento>(openFiles, activeFileName, fileModifiedStates, logEnabled, loggedFiles);
    });
}

bool ConfigSerializer::resetConfig(const std::string& fileName) {
    return safeExecute([this, &fileName]() -> bool {
        std::ifstream file(fileName);
        if (!file.good()) {
            return true; // 文件不存在，视为成功
        }
        file.close();
        return std::remove(fileName.c_str()) == 0;
    });
}

void ConfigSerializer::handleException(const std::exception& e) const {
    std::cerr << "ConfigSerializer error: " << e.what() << std::endl;
}
