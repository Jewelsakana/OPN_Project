#ifndef CONFIGSERIALIZER_H
#define CONFIGSERIALIZER_H

#include "Model.h"
#include <string>
#include <memory>
#include <vector>
#include <map>

class WorkspaceMemento;

// ConfigData: 解析配置文件时的中间数据结构，替代 parseConfigLine 的多个 out-参数
struct ConfigData {
    std::vector<std::string> openFiles;
    std::string activeFileName;
    std::map<std::string, bool> fileModifiedStates;
    bool logEnabled = false;
    std::vector<std::string> loggedFiles;
    std::string spellCheckerProduct;

    std::shared_ptr<WorkspaceMemento> toMemento() const;
};

class ConfigSerializer : public Model {
public:
    ConfigSerializer() = default;

    void saveConfig(const std::string& fileName, const WorkspaceMemento& memento);
    std::shared_ptr<WorkspaceMemento> loadConfig(const std::string& fileName);
    bool resetConfig(const std::string& fileName);

protected:
    void handleException(const std::exception& e) const override;

private:
    void parseConfigLine(const std::string& key, const std::string& value,
                         ConfigData& data);

    // 将字符串列表序列化为逗号分隔值（如 openFiles: a.txt,b.txt）
    static void writeCommaSeparatedList(std::ofstream& file, const std::string& key,
                                        const std::vector<std::string>& items);
};

#endif // CONFIGSERIALIZER_H
