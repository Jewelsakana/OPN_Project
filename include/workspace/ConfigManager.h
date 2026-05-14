#ifndef CONFIGMANAGER_H
#define CONFIGMANAGER_H

#include "Model.h"
#include <string>

class WorkSpace;
class ConfigSerializer;

class ConfigManager : public Model {
public:
    ConfigManager(WorkSpace& workspace, ConfigSerializer& serializer);

    void saveConfig(const std::string& configFile);
    bool loadConfig(const std::string& configFile);

private:
    WorkSpace& workspace_;
    ConfigSerializer& serializer_;
};

#endif // CONFIGMANAGER_H
