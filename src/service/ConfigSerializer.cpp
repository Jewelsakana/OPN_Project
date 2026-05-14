#include "ConfigSerializer.h"
#include "WorkSpace.h"
#include "StringUtils.h"
#include <fstream>
#include <iostream>
#include <sstream>

// ConfigData 转 WorkspaceMemento
std::shared_ptr<WorkspaceMemento> ConfigData::toMemento() const {
    return std::make_shared<WorkspaceMemento>(openFiles, activeFileName,
                                               fileModifiedStates, logEnabled,
                                               loggedFiles, spellCheckerProduct);
}

// 辅助：将字符串列表序列化为逗号分隔值
void ConfigSerializer::writeCommaSeparatedList(std::ofstream& file, const std::string& key,
                                                const std::vector<std::string>& items) {
    file << key << ":";
    for (size_t i = 0; i < items.size(); ++i) {
        file << items[i];
        if (i != items.size() - 1) file << ",";
    }
    file << "\n";
}

void ConfigSerializer::saveConfig(const std::string& fileName, const WorkspaceMemento& memento) {
    safeExecute([this, &fileName, &memento]() {
        std::ofstream file(fileName);
        if (!file) {
            throw std::runtime_error("Unable to write config file: " + fileName);
        }

        writeCommaSeparatedList(file, "openFiles", memento.getOpenFiles());

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

        writeCommaSeparatedList(file, "loggedFiles", memento.getLoggedFiles());

        file << "spellCheckerProduct: " << memento.getSpellCheckerProduct() << "\n";

        file.close();
    });
}

void ConfigSerializer::parseConfigLine(const std::string& key, const std::string& value,
                                        ConfigData& data) {
    if (key == "openFiles") {
        if (!value.empty()) {
            auto files = StringUtils::splitString(value, ',');
            for (const auto& f : files) {
                std::string trimmed = StringUtils::trim(f);
                if (!trimmed.empty()) data.openFiles.push_back(trimmed);
            }
        }
    } else if (key == "activeFileName") {
        data.activeFileName = value;
    } else if (key == "fileModifiedStates") {
        if (!value.empty()) {
            auto pairs = StringUtils::splitString(value, ',');
            for (const auto& pair : pairs) {
                std::string trimmed = StringUtils::trim(pair);
                size_t eqPos = trimmed.find('=');
                if (eqPos != std::string::npos) {
                    std::string fk = StringUtils::trim(trimmed.substr(0, eqPos));
                    std::string bv = StringUtils::trim(trimmed.substr(eqPos + 1));
                    data.fileModifiedStates[fk] = (bv == "true");
                }
            }
        }
    } else if (key == "logEnabled") {
        data.logEnabled = (value == "true");
    } else if (key == "loggedFiles") {
        if (!value.empty()) {
            auto files = StringUtils::splitString(value, ',');
            for (const auto& f : files) {
                std::string trimmed = StringUtils::trim(f);
                if (!trimmed.empty()) data.loggedFiles.push_back(trimmed);
            }
        }
    } else if (key == "spellCheckerProduct") {
        data.spellCheckerProduct = value;
    }
}

std::shared_ptr<WorkspaceMemento> ConfigSerializer::loadConfig(const std::string& fileName) {
    return safeExecute([this, &fileName]() -> std::shared_ptr<WorkspaceMemento> {
        std::ifstream file(fileName);
        if (!file) {
            return nullptr;
        }

        ConfigData data;

        std::string line;
        while (std::getline(file, line)) {
            line = StringUtils::trim(line);
            if (line.empty()) continue;

            size_t colonPos = line.find(':');
            if (colonPos == std::string::npos) continue;

            std::string key = StringUtils::trim(line.substr(0, colonPos));
            std::string value = StringUtils::trim(line.substr(colonPos + 1));
            parseConfigLine(key, value, data);
        }

        file.close();
        return data.toMemento();
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
