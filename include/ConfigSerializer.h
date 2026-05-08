#ifndef CONFIGSERIALIZER_H
#define CONFIGSERIALIZER_H

#include "Model.h"
#include <string>
#include <memory>

class WorkspaceMemento;

class ConfigSerializer : public Model {
public:
    ConfigSerializer() = default;
    std::string getName() const override { return "ConfigSerializer"; }

    void saveConfig(const std::string& fileName, const WorkspaceMemento& memento);
    std::shared_ptr<WorkspaceMemento> loadConfig(const std::string& fileName);
    bool resetConfig(const std::string& fileName);

protected:
    void handleException(const std::exception& e) const override;
};

#endif // CONFIGSERIALIZER_H
