#pragma once

#include <string>

namespace PaperPad {

struct EditorConfig {
    int tabSize = 4;
    bool showLineNumbers = true;
    bool wordWrap = false;
    bool showSidebar = true;
    int sidebarWidth = 22;
    std::wstring workingDirectory = L".";
};

class ConfigManager {
public:
    static ConfigManager& getInstance();

    void load();
    void save();

    EditorConfig& getConfig() { return m_config; }

private:
    ConfigManager() = default;
    ~ConfigManager() = default;
    ConfigManager(const ConfigManager&) = delete;
    ConfigManager& operator=(const ConfigManager&) = delete;

    EditorConfig m_config;
};

} // namespace PaperPad
