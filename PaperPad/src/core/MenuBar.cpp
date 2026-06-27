#include "core/MenuBar.hpp"

namespace PaperPad {

HMENU MenuBar::create() {
    HMENU hMenuBar = CreateMenu();

    // File Popup Menu
    HMENU hFileMenu = CreatePopupMenu();
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_NEW, L"&New\tCtrl+N");
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_OPEN, L"&Open...\tCtrl+O");
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_SAVE, L"&Save\tCtrl+S");
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_SAVE_AS, L"Save &As...");
    AppendMenuW(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hFileMenu, MF_STRING, IDM_FILE_EXIT, L"E&xit");

    // Edit Popup Menu
    HMENU hEditMenu = CreatePopupMenu();
    AppendMenuW(hEditMenu, MF_STRING, IDM_EDIT_UNDO, L"&Undo\tCtrl+Z");
    AppendMenuW(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hEditMenu, MF_STRING, IDM_EDIT_CUT, L"Cu&t\tCtrl+X");
    AppendMenuW(hEditMenu, MF_STRING, IDM_EDIT_COPY, L"&Copy\tCtrl+C");
    AppendMenuW(hEditMenu, MF_STRING, IDM_EDIT_PASTE, L"&Paste\tCtrl+V");
    AppendMenuW(hEditMenu, MF_STRING, IDM_EDIT_DELETE, L"&Delete\tDel");
    AppendMenuW(hEditMenu, MF_SEPARATOR, 0, NULL);
    AppendMenuW(hEditMenu, MF_STRING, IDM_EDIT_SELECT_ALL, L"Select &All\tCtrl+A");

    // Search Popup Menu
    HMENU hSearchMenu = CreatePopupMenu();
    AppendMenuW(hSearchMenu, MF_STRING, IDM_SEARCH_FIND, L"&Find...\tCtrl+F");

    // View Popup Menu
    HMENU hViewMenu = CreatePopupMenu();
    AppendMenuW(hViewMenu, MF_STRING | MF_CHECKED, IDM_VIEW_SIDEBAR, L"&Show Sidebar\tCtrl+W");

    // Help Popup Menu
    HMENU hHelpMenu = CreatePopupMenu();
    AppendMenuW(hHelpMenu, MF_STRING, IDM_HELP_ABOUT, L"&About PaperPad");

    // Append to Menu Bar
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hFileMenu, L"&File");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hEditMenu, L"&Edit");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hSearchMenu, L"&Search");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hViewMenu, L"&View");
    AppendMenuW(hMenuBar, MF_POPUP, (UINT_PTR)hHelpMenu, L"&Help");

    return hMenuBar;
}

} // namespace PaperPad
