#pragma once

#include <windows.h>
#include <string>
#include "core/EditorCtrl.hpp"
#include "core/SidebarCtrl.hpp"

namespace PaperPad {

class MainWindow {
public:
    MainWindow();
    ~MainWindow();

    bool create(const std::wstring& title, int width, int height);
    void show(int nCmdShow);
    HWND getHWND() const { return m_hWnd; }
    HACCEL getAccel() const { return m_hAccel; }


    // Run the main window procedure callback (static adapter mapping)
    static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

private:
    HWND m_hWnd;
    HMENU m_hMenu;
    HACCEL m_hAccel;

    EditorCtrl m_editor;
    SidebarCtrl m_sidebar;

    bool m_sidebarVisible;
    std::wstring m_filepath;

    void updateTitle();
    void layoutControls();
    
    // Command Handlers
    void onFileNew();
    void onFileOpen();
    void onFileSave();
    void onFileSaveAs();
    bool querySaveConfirmation();
    
    void onEditUndo();
    void onEditCut();
    void onEditCopy();
    void onEditPaste();
    void onEditDelete();
    void onEditSelectAll();

    void onSearchFind();
    void onViewToggleSidebar();
    void onHelpAbout();

    void openFile(const std::wstring& path);

    // Setup accelerators
    void initAccelerators();
};

} // namespace PaperPad
