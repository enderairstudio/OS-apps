#pragma once

#include "ui/Window.hpp"
#include <string>
#include <vector>

namespace PaperPad {

struct ExplorerEntry {
    std::wstring name;
    std::wstring path;
    bool isDirectory = false;
    bool isExpanded = false;
    int depth = 0;
};

class FileExplorer : public Window {
public:
    FileExplorer();
    ~FileExplorer() override = default;

    void setWorkingDirectory(const std::wstring& path);
    const std::wstring& getWorkingDirectory() const { return m_workingDir; }

    void draw(Console& console) override;
    void handleInput(const UIEvent& ev) override;

    bool hasPendingFileToOpen() const { return !m_pendingFile.empty(); }
    std::wstring getPendingFileToOpen();

    void refresh();

private:
    std::wstring m_workingDir;
    std::vector<ExplorerEntry> m_entries;
    int m_selectedIndex = 0;
    int m_scrollOffset = 0;

    std::wstring m_pendingFile;

    void populateDirectory(const std::wstring& dir, std::vector<ExplorerEntry>& list, int depth);
    void rebuildExplorerList();
    void toggleFolder(int index);
};

} // namespace PaperPad
