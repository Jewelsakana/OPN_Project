#ifndef CONFIGCOORDINATOR_H
#define CONFIGCOORDINATOR_H

#include <string>

class ConfigManager;

// ConfigCoordinator：配置保存/恢复
class ConfigCoordinator {
public:
    explicit ConfigCoordinator(ConfigManager& cm);

    void saveConfig(const std::string& configFile);
    bool loadConfig(const std::string& configFile);

private:
    ConfigManager& configManager_;
};

#endif // CONFIGCOORDINATOR_H
