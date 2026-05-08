#include "ConfigManager.h"
#include "ConfigSerializer.h"
#include "WorkSpace.h"
#include <iostream>

ConfigManager::ConfigManager(WorkSpace& workspace, ConfigSerializer& serializer)
    : workspace_(workspace), serializer_(serializer) {
}

void ConfigManager::saveConfig(const std::string& configFile) {
    try {
        auto memento = workspace_.createMemento();
        serializer_.saveConfig(configFile, *memento);
        Event event("config saved", configFile);
        workspace_.notify(event);
    } catch (const std::exception& e) {
        workspace_.getOutputService().outputError("Warning: Failed to save configuration: " + std::string(e.what()));
    }
}

bool ConfigManager::loadConfig(const std::string& configFile) {
    try {
        auto memento = serializer_.loadConfig(configFile);
        if (!memento) {
            return false;
        }
        workspace_.restoreFromMemento(*memento);
        Event event("config loaded", configFile);
        workspace_.notify(event);
        return true;
    } catch (const std::exception& e) {
        workspace_.getOutputService().outputError("Warning: Failed to load configuration (using default): " + std::string(e.what()));
        return false;
    }
}
