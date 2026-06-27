#include "core/SidebarCtrl.hpp"
#include "io/FileHandler.hpp"
#include "utils/Logger.hpp"
#include <algorithm>

namespace PaperPad {

SidebarCtrl::SidebarCtrl() : m_hWnd(NULL), m_hFont(NULL), m_workingDir(L"pads") {}

SidebarCtrl::~SidebarCtrl() {
    destroy();
}

bool SidebarCtrl::create(HWND hwndParent, int id) {
    // Ensure the pads directory exists
    CreateDirectoryW(L"pads", NULL);

    m_hWnd = CreateWindowExW(
        0,
        L"LISTBOX",
        L"",
        WS_CHILD | WS_VISIBLE | LBS_NOTIFY | WS_VSCROLL | WS_BORDER,
        0, 0, 0, 0,
        hwndParent,
        (HMENU)(UINT_PTR)id,
        GetModuleHandle(NULL),
        NULL
    );

    if (!m_hWnd) {
        return false;
    }

    // Set nice clean UI Font (Segoe UI size 10)
    HDC hdc = GetDC(m_hWnd);
    int height = -MulDiv(10, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(m_hWnd, hdc);

    m_hFont = CreateFontW(
        height, 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        L"Segoe UI"
    );

    if (m_hFont) {
        SendMessageW(m_hWnd, WM_SETFONT, (WPARAM)m_hFont, TRUE);
    }

    return true;
}

void SidebarCtrl::destroy() {
    if (m_hFont) {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }
    m_hWnd = NULL;
}

void SidebarCtrl::setBounds(int x, int y, int w, int h) {
    if (m_hWnd) {
        MoveWindow(m_hWnd, x, y, w, h, TRUE);
    }
}

void SidebarCtrl::setWorkingDirectory(const std::wstring& dir) {
    m_workingDir = dir;
    populateList();
}

void SidebarCtrl::refresh() {
    populateList();
}

void SidebarCtrl::populateList() {
    if (!m_hWnd) return;

    SendMessageW(m_hWnd, LB_RESETCONTENT, 0, 0);
    m_items.clear();

    WIN32_FIND_DATAW findData;
    std::wstring searchPath = m_workingDir + L"\\*.txt";
    HANDLE hFind = FindFirstFileW(searchPath.c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE) {
        return;
    }

    do {
        std::wstring name = findData.cFileName;
        // Skip hidden files
        if (findData.dwFileAttributes & FILE_ATTRIBUTE_HIDDEN) {
            continue;
        }

        SidebarItem item;
        item.name = name;
        item.path = m_workingDir + L"\\" + name;
        m_items.push_back(item);
    } while (FindNextFileW(hFind, &findData));

    FindClose(hFind);

    // Sort pads alphabetically
    std::sort(m_items.begin(), m_items.end(), [](const SidebarItem& a, const SidebarItem& b) {
        return a.name < b.name;
    });

    // Populate ListBox without '.txt' extension for cleaner tab representation
    for (const auto& item : m_items) {
        std::wstring displayName = item.name;
        if (displayName.length() > 4 && displayName.substr(displayName.length() - 4) == L".txt") {
            displayName = displayName.substr(0, displayName.length() - 4);
        }
        SendMessageW(m_hWnd, LB_ADDSTRING, 0, (LPARAM)displayName.c_str());
    }
}

std::wstring SidebarCtrl::getSelectedPath() const {
    if (!m_hWnd) return L"";

    int index = (int)SendMessageW(m_hWnd, LB_GETCURSEL, 0, 0);
    if (index >= 0 && index < static_cast<int>(m_items.size())) {
        return m_items[index].path;
    }
    return L"";
}

std::wstring SidebarCtrl::createNewPad() {
    // Ensure the pads directory exists
    CreateDirectoryW(m_workingDir.c_str(), NULL);

    int count = 1;
    std::wstring padPath;
    std::wstring padName;
    
    // Find unique filename
    do {
        padName = L"Untitled " + std::to_wstring(count);
        padPath = m_workingDir + L"\\" + padName + L".txt";
        count++;
    } while (GetFileAttributesW(padPath.c_str()) != INVALID_FILE_ATTRIBUTES);

    // Create the empty text file pad
    if (FileHandler::saveFile(padPath, L"")) {
        LOG_INFO("Created new pad file.");
        populateList();
        
        // Find newly added item in listbox and highlight it
        for (size_t i = 0; i < m_items.size(); ++i) {
            if (m_items[i].path == padPath) {
                SendMessageW(m_hWnd, LB_SETCURSEL, i, 0);
                break;
            }
        }
        return padPath;
    }
    return L"";
}

} // namespace PaperPad
