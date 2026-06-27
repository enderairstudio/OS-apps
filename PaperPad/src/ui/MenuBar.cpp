#include "ui/MenuBar.hpp"
#include "utils/Logger.hpp"

namespace PaperPad {

MenuBar::MenuBar()
    : m_activeMenu(-1)
    , m_activeItem(-1)
    , m_pendingAction(MenuAction::None) {
    m_bounds = {0, 0, 80, 1};
    initMenus();
}

void MenuBar::initMenus() {
    m_menus.push_back({L"File", {
        {L"New", L"Ctrl+N", MenuAction::FileNew},
        {L"Open...", L"Ctrl+O", MenuAction::FileOpen},
        {L"Save", L"Ctrl+S", MenuAction::FileSave},
        {L"Save As...", L"", MenuAction::FileSaveAs},
        {L"Quit", L"Ctrl+Q", MenuAction::FileQuit}
    }});

    m_menus.push_back({L"Edit", {
        {L"Undo", L"Ctrl+Z", MenuAction::EditUndo},
        {L"Redo", L"Ctrl+Y", MenuAction::EditRedo},
        {L"Cut", L"Ctrl+X", MenuAction::EditCut},
        {L"Copy", L"Ctrl+C", MenuAction::EditCopy},
        {L"Paste", L"Ctrl+V", MenuAction::EditPaste},
        {L"Select All", L"Ctrl+A", MenuAction::EditSelectAll}
    }});

    m_menus.push_back({L"Search", {
        {L"Find...", L"Ctrl+F", MenuAction::SearchFind}
    }});

    m_menus.push_back({L"View", {
        {L"Toggle Sidebar", L"Ctrl+W", MenuAction::ViewToggleSidebar},
        {L"Toggle Line Numbers", L"", MenuAction::ViewToggleLineNumbers}
    }});

    m_menus.push_back({L"Help", {
        {L"About PaperPad", L"", MenuAction::HelpAbout}
    }});

    // Calculate dimensions
    for (auto& menu : m_menus) {
        int maxW = 0;
        for (const auto& item : menu.items) {
            int w = static_cast<int>(item.label.length() + item.shortcut.length() + 6);
            if (w > maxW) maxW = w;
        }
        menu.width = maxW;
    }
}

void MenuBar::draw(Console& console) {
    const auto& theme = console.getTheme();
    
    // Fill the menu bar background
    console.fillRect({m_bounds.x, m_bounds.y, m_bounds.w, 1}, L' ', theme.menuBar);

    int currentX = 1;
    for (size_t i = 0; i < m_menus.size(); ++i) {
        auto& menu = m_menus[i];
        menu.screenX = currentX;

        std::wstring header = L" " + menu.name + L" ";
        WORD attr = theme.menuBarText;
        if (static_cast<int>(i) == m_activeMenu) {
            attr = theme.menuBarActive;
        }

        console.writeText(currentX, 0, header, attr);
        currentX += static_cast<int>(header.length()) + 2;
    }

    // Draw active dropdown
    if (m_activeMenu != -1 && m_activeMenu < static_cast<int>(m_menus.size())) {
        const auto& menu = m_menus[m_activeMenu];
        int dropdownX = menu.screenX;
        int dropdownY = 1;
        int dropdownW = menu.width;
        int dropdownH = static_cast<int>(menu.items.size()) + 2;

        // Draw dropdown border and background
        console.fillRect({dropdownX, dropdownY, dropdownW, dropdownH}, L' ', theme.menuBar);
        console.drawBorder({dropdownX, dropdownY, dropdownW, dropdownH}, theme.border, false);

        for (size_t i = 0; i < menu.items.size(); ++i) {
            const auto& item = menu.items[i];
            int itemY = dropdownY + 1 + static_cast<int>(i);

            WORD attr = theme.menuBarText;
            if (static_cast<int>(i) == m_activeItem) {
                attr = theme.menuBarActive;
            }

            // Print item label and shortcut
            console.writeText(dropdownX + 2, itemY, item.label, attr);
            if (!item.shortcut.empty()) {
                int shortcutX = dropdownX + dropdownW - 2 - static_cast<int>(item.shortcut.length());
                console.writeText(shortcutX, itemY, item.shortcut, attr);
            }
        }
    }
}

void MenuBar::handleInput(const UIEvent& ev) {
    if (ev.type == UIEvent::Type::Key) {
        // Handle dropdown menu navigation
        if (m_activeMenu != -1) {
            const auto& menu = m_menus[m_activeMenu];
            if (ev.key.code == Key::Escape) {
                closeMenu();
            } 
            else if (ev.key.code == Key::Left) {
                m_activeMenu = (m_activeMenu - 1 + static_cast<int>(m_menus.size())) % static_cast<int>(m_menus.size());
                m_activeItem = 0;
            } 
            else if (ev.key.code == Key::Right) {
                m_activeMenu = (m_activeMenu + 1) % static_cast<int>(m_menus.size());
                m_activeItem = 0;
            } 
            else if (ev.key.code == Key::Down) {
                m_activeItem = (m_activeItem + 1) % static_cast<int>(menu.items.size());
            } 
            else if (ev.key.code == Key::Up) {
                m_activeItem = (m_activeItem - 1 + static_cast<int>(menu.items.size())) % static_cast<int>(menu.items.size());
            } 
            else if (ev.key.code == Key::Enter) {
                if (m_activeItem >= 0 && m_activeItem < static_cast<int>(menu.items.size())) {
                    m_pendingAction = menu.items[m_activeItem].action;
                    closeMenu();
                }
            }
        } 
        else if (ev.key.altPressed) {
            // Check for Alt-key menu triggers (e.g. Alt+F -> File)
            wchar_t ch = ev.key.ch;
            if (ch == L'f' || ch == L'F') { m_activeMenu = 0; m_activeItem = 0; }
            else if (ch == L'e' || ch == L'E') { m_activeMenu = 1; m_activeItem = 0; }
            else if (ch == L's' || ch == L'S') { m_activeMenu = 2; m_activeItem = 0; }
            else if (ch == L'v' || ch == L'V') { m_activeMenu = 3; m_activeItem = 0; }
            else if (ch == L'h' || ch == L'H') { m_activeMenu = 4; m_activeItem = 0; }
        }
    } 
    else if (ev.type == UIEvent::Type::Mouse) {
        Point m = ev.mouse.pos;
        bool isClick = (ev.mouse.buttonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;
        bool isHover = (ev.mouse.eventFlags & MOUSE_MOVED) != 0;

        // Hover or click menu bar headers
        if (m.y == 0) {
            for (size_t i = 0; i < m_menus.size(); ++i) {
                const auto& menu = m_menus[i];
                int start = menu.screenX;
                int end = start + static_cast<int>(menu.name.length()) + 2;

                if (m.x >= start && m.x < end) {
                    if (isClick) {
                        if (m_activeMenu == static_cast<int>(i)) {
                            closeMenu();
                        } else {
                            m_activeMenu = static_cast<int>(i);
                            m_activeItem = 0;
                        }
                    } else if (isHover && m_activeMenu != -1) {
                        // Switch menu dropdown on hover if already open
                        m_activeMenu = static_cast<int>(i);
                        m_activeItem = 0;
                    }
                }
            }
        }
        // Click inside dropdown
        else if (m_activeMenu != -1) {
            const auto& menu = m_menus[m_activeMenu];
            int dx = menu.screenX;
            int dy = 1;
            int dw = menu.width;
            int dh = static_cast<int>(menu.items.size()) + 2;

            if (m.x >= dx && m.x < dx + dw && m.y >= dy && m.y < dy + dh) {
                int index = m.y - dy - 1;
                if (index >= 0 && index < static_cast<int>(menu.items.size())) {
                    if (isHover) {
                        m_activeItem = index;
                    } else if (isClick) {
                        m_pendingAction = menu.items[index].action;
                        closeMenu();
                    }
                }
            } else if (isClick) {
                // Clicked outside active dropdown
                closeMenu();
            }
        }
    }
}

void MenuBar::closeMenu() {
    m_activeMenu = -1;
    m_activeItem = -1;
}

MenuAction MenuBar::getPendingAction() {
    MenuAction a = m_pendingAction;
    m_pendingAction = MenuAction::None;
    return a;
}

} // namespace PaperPad
