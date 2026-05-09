#include "ConfigCoordinator.h"
#include "ConfigManager.h"

ConfigCoordinator::ConfigCoordinator(ConfigManager& cm)
    : configManager_(cm) {
}

void ConfigCoordinator::saveConfig(const std::string& configFile) {
    configManager_.saveConfig(configFile);
}

bool ConfigCoordinator::loadConfig(const std::string& configFile) {
    return configManager_.loadConfig(configFile);
}
