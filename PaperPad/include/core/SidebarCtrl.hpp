#pragma once

#include <windows.h>
#include <string>
#include <vector>

namespace PaperPad {

struct SidebarItem {
    std::wstring name;
    std::wstring path;
};

class SidebarCtrl {
public:
    SidebarCtrl();
    ~SidebarCtrl();

    bool create(HWND hwndParent, int id);
    void destroy();

    HWND getHWND() const { return m_hWnd; }

    void setBounds(int x, int y, int w, int h);
    void setWorkingDirectory(const std::wstring& dir);
    void refresh();

    std::wstring getSelectedPath() const;
    std::wstring createNewPad();

private:
    HWND m_hWnd;
    HFONT m_hFont;
    std::wstring m_workingDir;
    std::vector<SidebarItem> m_items;

    void populateList();
};

} // namespace PaperPad
