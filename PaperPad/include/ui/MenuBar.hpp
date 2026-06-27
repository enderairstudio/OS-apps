#pragma once

#include "ui/Window.hpp"
#include <vector>
#include <string>

namespace PaperPad {

enum class MenuAction {
    None = 0,
    FileNew,
    FileOpen,
    FileSave,
    FileSaveAs,
    FileQuit,
    EditUndo,
    EditRedo,
    EditCut,
    EditCopy,
    EditPaste,
    EditSelectAll,
    SearchFind,
    ViewToggleSidebar,
    ViewToggleLineNumbers,
    HelpAbout
};

struct MenuItem {
    std::wstring label;
    std::wstring shortcut;
    MenuAction action;
};

struct Menu {
    std::wstring name;
    std::vector<MenuItem> items;
    int screenX = 0; // Cached coordinate of the menu header
    int width = 0;    // Calculated width of the dropdown box
};

class MenuBar : public Window {
public:
    MenuBar();
    ~MenuBar() override = default;

    void draw(Console& console) override;
    void handleInput(const UIEvent& ev) override;

    MenuAction getPendingAction();

private:
    std::vector<Menu> m_menus;
    int m_activeMenu; // -1 if closed, 0..N if a menu dropdown is open
    int m_activeItem; // Selected item index in the active menu dropdown

    MenuAction m_pendingAction;

    void initMenus();
    void closeMenu();
};

} // namespace PaperPad
