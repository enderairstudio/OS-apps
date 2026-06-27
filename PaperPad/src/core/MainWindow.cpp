#include "core/MainWindow.hpp"
#include "core/MenuBar.hpp"
#include "core/FindDialog.hpp"
#include "io/FileHandler.hpp"
#include "utils/Logger.hpp"

namespace PaperPad {

MainWindow::MainWindow()
    : m_hWnd(NULL)
    , m_hMenu(NULL)
    , m_hAccel(NULL)
    , m_sidebarVisible(true) {}

MainWindow::~MainWindow() {
    if (m_hAccel) {
        DestroyAcceleratorTable(m_hAccel);
    }
}

bool MainWindow::create(const std::wstring& title, int width, int height) {
    HINSTANCE hInstance = GetModuleHandleW(NULL);

    WNDCLASSW wc = { 0 };
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = L"PaperPadMainWindow";
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);

    if (!RegisterClassW(&wc)) {
        LOG_ERROR("Failed to register MainWindow class.");
        return false;
    }

    m_hMenu = MenuBar::create();
    initAccelerators();

    // Center window on screen
    int screenW = GetSystemMetrics(SM_CXSCREEN);
    int screenH = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenW - width) / 2;
    int y = (screenH - height) / 2;

    m_hWnd = CreateWindowExW(
        0,
        L"PaperPadMainWindow",
        title.c_str(),
        WS_OVERLAPPEDWINDOW,
        x, y, width, height,
        NULL,
        m_hMenu,
        hInstance,
        this
    );

    if (!m_hWnd) {
        LOG_ERROR("Failed to create MainWindow.");
        return false;
    }

    // Initialize child controls
    m_sidebar.create(m_hWnd, 3001);
    m_editor.create(m_hWnd, 3002);

    // Initial folder set to active desktop folder path
    m_sidebar.setWorkingDirectory(L"pads");

    updateTitle();
    layoutControls();

    return true;
}

void MainWindow::show(int nCmdShow) {
    if (m_hWnd) {
        ShowWindow(m_hWnd, nCmdShow);
        UpdateWindow(m_hWnd);
        m_editor.setFocus();
    }
}

void MainWindow::initAccelerators() {
    std::vector<ACCEL> accels;

    accels.push_back({ FVIRTKEY | FCONTROL, 'N', IDM_FILE_NEW });
    accels.push_back({ FVIRTKEY | FCONTROL, 'O', IDM_FILE_OPEN });
    accels.push_back({ FVIRTKEY | FCONTROL, 'S', IDM_FILE_SAVE });
    accels.push_back({ FVIRTKEY | FCONTROL, 'Z', IDM_EDIT_UNDO });
    accels.push_back({ FVIRTKEY | FCONTROL, 'F', IDM_SEARCH_FIND });
    accels.push_back({ FVIRTKEY | FCONTROL, 'W', IDM_VIEW_SIDEBAR });
    accels.push_back({ FVIRTKEY | FCONTROL, 'A', IDM_EDIT_SELECT_ALL });

    m_hAccel = CreateAcceleratorTableW(&accels[0], static_cast<int>(accels.size()));
}

void MainWindow::updateTitle() {
    std::wstring filename = m_filepath.empty() ? L"Untitled" : m_filepath.substr(m_filepath.find_last_of(L"\\/") + 1);
    std::wstring dirtyMarker = m_editor.isModified() ? L"*" : L"";
    std::wstring title = filename + dirtyMarker + L" - PaperPad";
    SetWindowTextW(m_hWnd, title.c_str());
}

void MainWindow::layoutControls() {
    RECT rect;
    GetClientRect(m_hWnd, &rect);
    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;

    if (m_sidebarVisible) {
        int sidebarW = 200;
        m_sidebar.setBounds(0, 0, sidebarW, height);
        m_editor.setBounds(sidebarW, 0, width - sidebarW, height);
        ShowWindow(m_sidebar.getHWND(), SW_SHOW);
    } else {
        ShowWindow(m_sidebar.getHWND(), SW_HIDE);
        m_editor.setBounds(0, 0, width, height);
    }
}

void MainWindow::openFile(const std::wstring& path) {
    std::wstring content;
    if (FileHandler::loadFile(path, content)) {
        m_editor.setText(content);
        m_filepath = path;
        m_editor.setModified(false);
        updateTitle();
        LOG_INFO("Opened file in editor.");
    } else {
        MessageBoxW(m_hWnd, L"Error loading file!", L"Error", MB_OK | MB_ICONERROR);
    }
}

bool MainWindow::querySaveConfirmation() {
    if (m_editor.isModified()) {
        int result = MessageBoxW(m_hWnd, L"Save changes to this document?", L"Unsaved Changes", MB_YESNOCANCEL | MB_ICONQUESTION);
        if (result == IDYES) {
            onFileSave();
            return !m_editor.isModified(); // True if saved successfully
        } else if (result == IDNO) {
            return true; // Discard changes
        } else {
            return false; // Cancel
        }
    }
    return true; // No modifications
}

void MainWindow::onFileNew() {
    if (querySaveConfirmation()) {
        m_editor.setText(L"");
        m_filepath.clear();
        m_editor.setModified(false);
        updateTitle();
    }
}

void MainWindow::onFileOpen() {
    if (querySaveConfirmation()) {
        std::wstring path;
        if (FileHandler::showOpenDialog(m_hWnd, path)) {
            openFile(path);
        }
    }
}

void MainWindow::onFileSave() {
    if (m_filepath.empty()) {
        onFileSaveAs();
    } else {
        std::wstring content = m_editor.getText();
        if (FileHandler::saveFile(m_filepath, content)) {
            m_editor.setModified(false);
            updateTitle();
            m_sidebar.refresh();
        } else {
            MessageBoxW(m_hWnd, L"Failed to save file!", L"Error", MB_OK | MB_ICONERROR);
        }
    }
}

void MainWindow::onFileSaveAs() {
    std::wstring path;
    if (FileHandler::showSaveDialog(m_hWnd, path)) {
        std::wstring content = m_editor.getText();
        if (FileHandler::saveFile(path, content)) {
            m_filepath = path;
            m_editor.setModified(false);
            updateTitle();
            m_sidebar.refresh();
        } else {
            MessageBoxW(m_hWnd, L"Failed to save file!", L"Error", MB_OK | MB_ICONERROR);
        }
    }
}

void MainWindow::onEditUndo() { m_editor.undo(); }
void MainWindow::onEditCut() { m_editor.cut(); }
void MainWindow::onEditCopy() { m_editor.copy(); }
void MainWindow::onEditPaste() { m_editor.paste(); }
void MainWindow::onEditDelete() { m_editor.deleteSelected(); }
void MainWindow::onEditSelectAll() { m_editor.selectAll(); }

void MainWindow::onSearchFind() {
    std::wstring query;
    if (FindDialog::show(m_hWnd, query)) {
        std::wstring text = m_editor.getText();
        size_t idx = text.find(query);
        if (idx != std::wstring::npos) {
            SendMessageW(m_editor.getHWND(), EM_SETSEL, idx, idx + query.length());
            SendMessageW(m_editor.getHWND(), EM_SCROLLCARET, 0, 0);
        } else {
            MessageBoxW(m_hWnd, (L"Could not find word: \"" + query + L"\"").c_str(), L"Find Text", MB_OK | MB_ICONINFORMATION);
        }
    }
}

void MainWindow::onViewToggleSidebar() {
    m_sidebarVisible = !m_sidebarVisible;
    
    // Toggle checked checkmark on view menu bar
    CheckMenuItem(m_hMenu, IDM_VIEW_SIDEBAR, m_sidebarVisible ? MF_CHECKED : MF_UNCHECKED);
    
    layoutControls();
}

void MainWindow::onHelpAbout() {
    MessageBoxW(m_hWnd, L"PaperPad Document Organizer.\nSimple, clean, and fully native.", L"About PaperPad", MB_OK | MB_ICONINFORMATION);
}

LRESULT CALLBACK MainWindow::WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    MainWindow* pThis = NULL;

    if (message == WM_NCCREATE) {
        CREATESTRUCTW* pCreate = reinterpret_cast<CREATESTRUCTW*>(lParam);
        pThis = reinterpret_cast<MainWindow*>(pCreate->lpCreateParams);
        SetWindowLongPtrW(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pThis));
    } else {
        pThis = reinterpret_cast<MainWindow*>(GetWindowLongPtrW(hWnd, GWLP_USERDATA));
    }

    if (pThis) {
        pThis->m_hWnd = hWnd;

        switch (message) {
            case WM_COMMAND: {
                int wmId = LOWORD(wParam);
                int wmEvent = HIWORD(wParam);

                // Handle Menu Commands
                switch (wmId) {
                    case IDM_FILE_NEW: pThis->onFileNew(); return 0;
                    case IDM_FILE_OPEN: pThis->onFileOpen(); return 0;
                    case IDM_FILE_SAVE: pThis->onFileSave(); return 0;
                    case IDM_FILE_SAVE_AS: pThis->onFileSaveAs(); return 0;
                    case IDM_FILE_EXIT: SendMessageW(hWnd, WM_CLOSE, 0, 0); return 0;
                    case IDM_EDIT_UNDO: pThis->onEditUndo(); return 0;
                    case IDM_EDIT_CUT: pThis->onEditCut(); return 0;
                    case IDM_EDIT_COPY: pThis->onEditCopy(); return 0;
                    case IDM_EDIT_PASTE: pThis->onEditPaste(); return 0;
                    case IDM_EDIT_DELETE: pThis->onEditDelete(); return 0;
                    case IDM_EDIT_SELECT_ALL: pThis->onEditSelectAll(); return 0;
                    case IDM_SEARCH_FIND: pThis->onSearchFind(); return 0;
                    case IDM_VIEW_SIDEBAR: pThis->onViewToggleSidebar(); return 0;
                    case IDM_HELP_ABOUT: pThis->onHelpAbout(); return 0;
                    
                    // ListBox Sidebar Notifications
                    case 3001: {
                        if (wmEvent == LBN_DBLCLK) {
                            HWND hListBox = pThis->m_sidebar.getHWND();
                            POINT pt;
                            GetCursorPos(&pt);
                            ScreenToClient(hListBox, &pt);

                            // Send LB_ITEMFROMPOINT to check if double-clicked inside an item
                            LRESULT itemRes = SendMessageW(hListBox, LB_ITEMFROMPOINT, 0, MAKELPARAM(pt.x, pt.y));
                            int index = LOWORD(itemRes);
                            int isOutside = HIWORD(itemRes);

                            if (isOutside != 0 || index < 0) {
                                // Double clicked on empty space - create new pad!
                                if (pThis->querySaveConfirmation()) {
                                    std::wstring newPadPath = pThis->m_sidebar.createNewPad();
                                    if (!newPadPath.empty()) {
                                        pThis->openFile(newPadPath);
                                    }
                                }
                            } else {
                                // Double clicked on a valid pad item - open it!
                                std::wstring selected = pThis->m_sidebar.getSelectedPath();
                                if (!selected.empty()) {
                                    if (pThis->querySaveConfirmation()) {
                                        pThis->openFile(selected);
                                    }
                                }
                            }
                        }
                        return 0;
                    }

                    // Edit Control Notifications (dirty check triggers)
                    case 3002: {
                        if (wmEvent == EN_CHANGE) {
                            pThis->updateTitle();
                        }
                        return 0;
                    }
                }
                break;
            }

            case WM_SIZE: {
                pThis->layoutControls();
                return 0;
            }

            case WM_CLOSE: {
                if (pThis->querySaveConfirmation()) {
                    DestroyWindow(hWnd);
                }
                return 0;
            }

            case WM_DESTROY: {
                PostQuitMessage(0);
                return 0;
            }
        }
    }

    return DefWindowProcW(hWnd, message, wParam, lParam);
}

} // namespace PaperPad
