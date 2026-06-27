#include "ui/FileExplorer.hpp"
#include "utils/Logger.hpp"
#include <windows.h>
#include <algorithm>
#include <functional>
#include <unordered_map>

namespace PaperPad {

FileExplorer::FileExplorer() {
    m_bounds = {0, 1, 22, 23};
    m_workingDir = L".";
}

void FileExplorer::setWorkingDirectory(const std::wstring& path) {
    m_workingDir = path;
    refresh();
}

void FileExplorer::refresh() {
    m_selectedIndex = 0;
    m_scrollOffset = 0;
    m_entries.clear();
    rebuildExplorerList();
}

void FileExplorer::populateDirectory(const std::wstring& dir, std::vector<ExplorerEntry>& list, int depth) {
    WIN32_FIND_DATAW findData;
    std::wstring searchPath = dir + L"\\*";
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    std::vector<ExplorerEntry> folders;
    std::vector<ExplorerEntry> files;

    do {
        std::wstring name = findData.cFileName;
        // Skip . and .. and hidden files
        if (name == L"." || name == L".." || (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN)) {
            continue;
        }

        ExplorerEntry entry;
        entry.name = name;
        entry.path = dir + L"\\" + name;
        entry.isDirectory = (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0;
        entry.isExpanded = false;
        entry.depth = depth;

        if (entry.isDirectory) {
            folders.push_back(entry);
        } else {
            files.push_back(entry);
        }
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);

    // Sort folders alphabetically
    std::sort(folders.begin(), folders.end(), [](const ExplorerEntry& a, const ExplorerEntry& b) {
        return a.name < b.name;
    });

    // Sort files alphabetically
    std::sort(files.begin(), files.end(), [](const ExplorerEntry& a, const ExplorerEntry& b) {
        return a.name < b.name;
    });

    // Combine folders and files (folders first)
    list.reserve(folders.size() + files.size());
    list.insert(list.end(), folders.begin(), folders.end());
    list.insert(list.end(), files.begin(), files.end());
}

void FileExplorer::rebuildExplorerList() {
    std::unordered_map<std::wstring, bool> expandedStates;
    for (const auto& entry : m_entries) {
        if (entry.isDirectory) {
            expandedStates[entry.path] = entry.isExpanded;
        }
    }

    m_entries.clear();
    std::vector<ExplorerEntry> rootList;
    populateDirectory(m_workingDir, rootList, 0);

    std::vector<ExplorerEntry> finalEntries;
    std::function<void(const std::vector<ExplorerEntry>&)> traverse = [&](const std::vector<ExplorerEntry>& currentList) {
        for (auto entry : currentList) {
            if (entry.isDirectory) {
                auto it = expandedStates.find(entry.path);
                if (it != expandedStates.end()) {
                    entry.isExpanded = it->second;
                }
            }
            finalEntries.push_back(entry);
            if (entry.isDirectory && entry.isExpanded) {
                std::vector<ExplorerEntry> subList;
                populateDirectory(entry.path, subList, entry.depth + 1);
                traverse(subList);
            }
        }
    };
    traverse(rootList);
    m_entries = std::move(finalEntries);

    if (m_selectedIndex >= static_cast<int>(m_entries.size())) {
        m_selectedIndex = static_cast<int>(m_entries.size()) - 1;
    }
    if (m_selectedIndex < 0) m_selectedIndex = 0;
}

void FileExplorer::draw(Console& console) {
    const auto& theme = console.getTheme();

    // Fill background
    console.fillRect(m_bounds, L' ', theme.sidebarBackground);
    
    // Draw right sidebar border
    for (int y = m_bounds.y; y < m_bounds.y + m_bounds.h; ++y) {
        console.writeChar(m_bounds.x + m_bounds.w - 1, y, L'│', theme.border);
    }

    // Header title
    std::wstring title = L" WORKSPACE ";
    console.writeText(m_bounds.x + 2, m_bounds.y, title, theme.sidebarText | FOREGROUND_INTENSITY);

    int viewHeight = m_bounds.h - 2; // Subtract border/header space
    if (m_entries.empty()) {
        console.writeText(m_bounds.x + 2, m_bounds.y + 2, L"(Empty Workspace)", theme.sidebarText);
        return;
    }

    // Scroll mapping
    if (m_selectedIndex < m_scrollOffset) {
        m_scrollOffset = m_selectedIndex;
    }
    if (m_selectedIndex >= m_scrollOffset + viewHeight) {
        m_scrollOffset = m_selectedIndex - viewHeight + 1;
    }

    for (int i = 0; i < viewHeight; ++i) {
        int index = m_scrollOffset + i;
        if (index >= static_cast<int>(m_entries.size())) break;

        const auto& entry = m_entries[index];
        int drawY = m_bounds.y + 2 + i;

        WORD attr = theme.sidebarText;
        if (index == m_selectedIndex) {
            attr = theme.selectedText;
            console.fillRect({m_bounds.x, drawY, m_bounds.w - 1, 1}, L' ', theme.selectedBackground);
        }

        // Build indentation
        std::wstring indent(entry.depth * 2, L' ');
        
        // Build icon and name
        std::wstring prefix = entry.isDirectory ? (entry.isExpanded ? L"▼ 📁 " : L"▶ 📁 ") : L"  📄 ";
        std::wstring lineText = indent + prefix + entry.name;

        // Truncate to sidebar width
        int maxLen = m_bounds.w - 2;
        if (static_cast<int>(lineText.length()) > maxLen) {
            lineText = lineText.substr(0, maxLen - 3) + L"...";
        }

        console.writeText(m_bounds.x + 1, drawY, lineText, attr);
    }
}

void FileExplorer::handleInput(const UIEvent& ev) {
    if (m_entries.empty()) return;

    if (ev.type == UIEvent::Type::Key) {
        if (ev.key.code == Key::Down) {
            m_selectedIndex = (m_selectedIndex + 1) % static_cast<int>(m_entries.size());
        } 
        else if (ev.key.code == Key::Up) {
            m_selectedIndex = (m_selectedIndex - 1 + static_cast<int>(m_entries.size())) % static_cast<int>(m_entries.size());
        } 
        else if (ev.key.code == Key::Enter) {
            toggleFolder(m_selectedIndex);
        }
        else if (ev.key.code == Key::Right) {
            auto& entry = m_entries[m_selectedIndex];
            if (entry.isDirectory && !entry.isExpanded) {
                entry.isExpanded = true;
                rebuildExplorerList();
            }
        }
        else if (ev.key.code == Key::Left) {
            auto& entry = m_entries[m_selectedIndex];
            if (entry.isDirectory && entry.isExpanded) {
                entry.isExpanded = false;
                rebuildExplorerList();
            }
        }
    } 
    else if (ev.type == UIEvent::Type::Mouse) {
        Point m = ev.mouse.pos;
        bool isClick = (ev.mouse.buttonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;
        bool isDoubleClick = (ev.mouse.eventFlags & DOUBLE_CLICK) != 0;

        if (m_bounds.contains(m) && m.x < m_bounds.x + m_bounds.w - 1) {
            int clickedRow = m.y - m_bounds.y - 2;
            int index = m_scrollOffset + clickedRow;

            if (index >= 0 && index < static_cast<int>(m_entries.size())) {
                if (isClick || isDoubleClick) {
                    m_selectedIndex = index;
                }

                if (isDoubleClick || (isClick && ev.key.controlPressed)) {
                    toggleFolder(index);
                } 
                else if (isClick) {
                    // Click folder icon to toggle or select file
                    // Let's toggle folders on single click for ease of terminal use
                    toggleFolder(index);
                }
            }
        }
    }
}

void FileExplorer::toggleFolder(int index) {
    if (index < 0 || index >= static_cast<int>(m_entries.size())) return;
    auto& entry = m_entries[index];
    if (entry.isDirectory) {
        entry.isExpanded = !entry.isExpanded;
        rebuildExplorerList();
    } else {
        m_pendingFile = entry.path;
    }
}

std::wstring FileExplorer::getPendingFileToOpen() {
    std::wstring f = m_pendingFile;
    m_pendingFile.clear();
    return f;
}

} // namespace PaperPad
