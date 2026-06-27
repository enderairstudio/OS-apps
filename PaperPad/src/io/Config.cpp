#include "io/Config.hpp"
#include "utils/Logger.hpp"
#include <fstream>
#include <sstream>
#include <windows.h>

namespace PaperPad {

ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

void ConfigManager::load() {
    std::ifstream file("config.ini");
    if (!file.is_open()) {
        LOG_INFO("config.ini not found, loading defaults.");
        // Try to get current directory as working directory
        wchar_t buffer[MAX_PATH];
        GetCurrentDirectoryW(MAX_PATH, buffer);
        m_config.workingDirectory = buffer;
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == ';') continue;
        size_t equalSign = line.find('=');
        if (equalSign == std::string::npos) continue;

        std::string key = line.substr(0, equalSign);
        std::string val = line.substr(equalSign + 1);

        if (key == "tabSize") {
            m_config.tabSize = std::stoi(val);
        } else if (key == "showLineNumbers") {
            m_config.showLineNumbers = (val == "true" || val == "1");
        } else if (key == "wordWrap") {
            m_config.wordWrap = (val == "true" || val == "1");
        } else if (key == "showSidebar") {
            m_config.showSidebar = (val == "true" || val == "1");
        } else if (key == "sidebarWidth") {
            m_config.sidebarWidth = std::stoi(val);
        } else if (key == "workingDirectory") {
            int len = MultiByteToWideChar(CP_UTF8, 0, val.c_str(), -1, NULL, 0);
            if (len > 0) {
                std::wstring wstr(len, 0);
                MultiByteToWideChar(CP_UTF8, 0, val.c_str(), -1, &wstr[0], len);
                if (!wstr.empty() && wstr.back() == L'\0') {
                    wstr.pop_back(); // Remove null terminator
                }
                m_config.workingDirectory = wstr;
            }
        }
    }
    file.close();
    LOG_INFO("Config loaded from config.ini");
}

void ConfigManager::save() {
    std::ofstream file("config.ini");
    if (!file.is_open()) {
        LOG_ERROR("Failed to save config.ini");
        return;
    }

    file << "; PaperPad settings\n";
    file << "tabSize=" << m_config.tabSize << "\n";
    file << "showLineNumbers=" << (m_config.showLineNumbers ? "true" : "false") << "\n";
    file << "wordWrap=" << (m_config.wordWrap ? "true" : "false") << "\n";
    file << "showSidebar=" << (m_config.showSidebar ? "true" : "false") << "\n";
    file << "sidebarWidth=" << m_config.sidebarWidth << "\n";

    // Convert working directory to UTF-8
    int len = WideCharToMultiByte(CP_UTF8, 0, m_config.workingDirectory.c_str(), -1, NULL, 0, NULL, NULL);
    if (len > 0) {
        std::string str(len, 0);
        WideCharToMultiByte(CP_UTF8, 0, m_config.workingDirectory.c_str(), -1, &str[0], len, NULL, NULL);
        if (!str.empty() && str.back() == '\0') {
            str.pop_back();
        }
        file << "workingDirectory=" << str << "\n";
    }

    file.close();
    LOG_INFO("Config saved to config.ini");
}

} // namespace PaperPad
