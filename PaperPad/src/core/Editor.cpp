#include "core/Editor.hpp"
#include "io/Config.hpp"
#include "utils/Logger.hpp"
#include <windows.h>
#include <algorithm>
#include <cmath>


namespace PaperPad {

Editor::Editor()
    : m_textArea(m_buffer) {
}

Editor::~Editor() {
    ConfigManager::getInstance().save();
    m_console.cleanup();
}

bool Editor::init() {
    Logger::getInstance().init("editor.log");
    LOG_INFO("Initializing editor application...");

    ConfigManager::getInstance().load();

    if (!m_console.init()) {
        LOG_ERROR("Failed to initialize console wrapper.");
        return false;
    }

    // Set working directory to config value
    m_explorer.setWorkingDirectory(ConfigManager::getInstance().getConfig().workingDirectory);

    // Apply configuration settings
    m_textArea.setShowLineNumbers(ConfigManager::getInstance().getConfig().showLineNumbers);
    m_textArea.setTabSize(ConfigManager::getInstance().getConfig().tabSize);

    layoutComponents();
    return true;
}

void Editor::layoutComponents() {
    int w = m_console.getWidth();
    int h = m_console.getHeight();

    // MenuBar always at top
    m_menuBar.setBounds({0, 0, w, 1});

    // StatusBar always at bottom
    m_statusBar.setBounds({0, h - 1, w, 1});

    const auto& config = ConfigManager::getInstance().getConfig();

    if (config.showSidebar) {
        int explorerW = config.sidebarWidth;
        m_explorer.setBounds({0, 1, explorerW, h - 2});
        m_textArea.setBounds({explorerW, 1, w - explorerW, h - 2});
    } else {
        m_explorer.setBounds({0, 0, 0, 0});
        m_textArea.setBounds({0, 1, w, h - 2});
    }
}

void Editor::run() {
    while (m_running) {
        // Render step
        m_console.clear(m_console.getTheme().background);

        m_menuBar.draw(m_console);
        
        if (ConfigManager::getInstance().getConfig().showSidebar) {
            m_explorer.draw(m_console);
        }
        
        m_textArea.draw(m_console);
        
        m_statusBar.update(m_buffer);
        m_statusBar.draw(m_console);

        // Draw dialog over top if active
        if (m_dialog.isActive()) {
            m_dialog.draw(m_console);
        } else {
            // Position console cursor to match TextArea cursor
            int lineCount = static_cast<int>(m_buffer.getLineCount());
            int numWidth = m_textArea.getShowLineNumbers() ? static_cast<int>(log10(std::max(1, lineCount))) + 2 : 0;
            int textXStart = m_textArea.getBounds().x + numWidth + (m_textArea.getShowLineNumbers() ? 1 : 0);

            Point bufCursor = m_buffer.getCursor();
            Point viewScroll = m_textArea.getScrollOffset();

            int cursorX = textXStart + (bufCursor.x - viewScroll.x);
            int cursorY = m_textArea.getBounds().y + (bufCursor.y - viewScroll.y);

            // Make sure console cursor is within TextArea bounds
            const Rect& tabounds = m_textArea.getBounds();
            if (cursorX >= textXStart && cursorX < tabounds.x + tabounds.w &&
                cursorY >= tabounds.y && cursorY < tabounds.y + tabounds.h) {
                m_console.setCursorPos(cursorX, cursorY);
                m_console.showCursor(true);
            } else {
                m_console.showCursor(false);
            }
        }

        m_console.present();

        // Process step
        processEvents();
    }
}

void Editor::processEvents() {
    UIEvent ev;
    if (!m_console.readEvent(ev)) {
        return;
    }

    // Handshake dialog inputs exclusively if active
    if (m_dialog.isActive()) {
        m_dialog.handleInput(ev);
        if (!m_dialog.isActive()) {
            handleDialogCompletion();
        }
        return;
    }

    // Let the menu bar handle clicks and hotkeys
    m_menuBar.handleInput(ev);
    MenuAction action = m_menuBar.getPendingAction();
    if (action != MenuAction::None) {
        handleMenuAction(action);
        return;
    }

    // Handle generic keyboard shortcuts
    if (ev.type == UIEvent::Type::Key) {
        if (ev.key.controlPressed) {
            switch (ev.key.code) {
                case Key::Ctrl_Q: handleMenuAction(MenuAction::FileQuit); return;
                case Key::Ctrl_O: handleMenuAction(MenuAction::FileOpen); return;
                case Key::Ctrl_S: handleMenuAction(MenuAction::FileSave); return;
                case Key::Ctrl_N: handleMenuAction(MenuAction::FileNew); return;
                case Key::Ctrl_F: handleMenuAction(MenuAction::SearchFind); return;
                case Key::Ctrl_W: handleMenuAction(MenuAction::ViewToggleSidebar); return;
                default: break;
            }
            // Clipboard shortcuts
            if (ev.key.ch == L'c' || ev.key.ch == L'C') {
                m_textArea.copyToClipboard();
                return;
            }
            if (ev.key.ch == L'x' || ev.key.ch == L'X') {
                m_textArea.cutToClipboard();
                return;
            }
            if (ev.key.ch == L'v' || ev.key.ch == L'V') {
                m_textArea.pasteFromClipboard();
                return;
            }
        }
    }

    // Resize window listener
    if (ev.type == UIEvent::Type::Resize) {
        layoutComponents();
        return;
    }

    // Mouse focus router / widget routing
    if (ev.type == UIEvent::Type::Mouse) {
        Point m = ev.mouse.pos;
        if (m_explorer.getBounds().contains(m)) {
            m_explorer.handleInput(ev);
            // If explorer double-clicked or opened a file
            if (m_explorer.hasPendingFileToOpen()) {
                m_nextFilePath = m_explorer.getPendingFileToOpen();
                checkSaveStateBeforeAction(PendingAction::ConfirmCloseForOpen);
            }
        } else {
            m_textArea.handleInput(ev);
        }
    } else {
        // Send keyboard inputs directly to TextArea
        m_textArea.handleInput(ev);
    }
}

void Editor::handleMenuAction(MenuAction action) {
    switch (action) {
        case MenuAction::FileNew:
            checkSaveStateBeforeAction(PendingAction::ConfirmCloseForNew);
            break;
        case MenuAction::FileOpen:
            checkSaveStateBeforeAction(PendingAction::ConfirmCloseForOpen);
            break;
        case MenuAction::FileSave:
            performFileSave();
            break;
        case MenuAction::FileSaveAs:
            performFileSaveAs(PendingAction::None);
            break;
        case MenuAction::FileQuit:
            checkSaveStateBeforeAction(PendingAction::ConfirmCloseForQuit);
            break;
        case MenuAction::EditUndo:
            m_buffer.undo();
            m_textArea.scrollToCursor();
            break;
        case MenuAction::EditRedo:
            m_buffer.redo();
            m_textArea.scrollToCursor();
            break;
        case MenuAction::EditCut:
            m_textArea.cutToClipboard();
            break;
        case MenuAction::EditCopy:
            m_textArea.copyToClipboard();
            break;
        case MenuAction::EditPaste:
            m_textArea.pasteFromClipboard();
            break;
        case MenuAction::EditSelectAll:
            m_textArea.selectAll();
            break;
        case MenuAction::SearchFind:
            m_dialog.show(DialogType::Input, L"Find Text", L"Enter search term:");
            m_pendingState = PendingAction::SearchQuery;
            break;
        case MenuAction::ViewToggleSidebar: {
            auto& config = ConfigManager::getInstance().getConfig();
            config.showSidebar = !config.showSidebar;
            layoutComponents();
            break;
        }
        case MenuAction::ViewToggleLineNumbers: {
            auto& config = ConfigManager::getInstance().getConfig();
            config.showLineNumbers = !config.showLineNumbers;
            m_textArea.setShowLineNumbers(config.showLineNumbers);
            break;
        }
        case MenuAction::HelpAbout:
            m_dialog.show(DialogType::Alert, L"About PaperPad", L"PaperPad Editor v1.0. Built in C++.");
            m_pendingState = PendingAction::None;
            break;
        default:
            break;
    }
}

void Editor::checkSaveStateBeforeAction(PendingAction nextAction) {
    if (m_buffer.isDirty()) {
        m_pendingState = nextAction;
        m_dialog.show(DialogType::Confirm, L"Unsaved Changes", L"Save changes before continuing?");
    } else {
        // Safe to bypass confirmation
        if (nextAction == PendingAction::ConfirmCloseForNew) {
            m_buffer.clear();
        } 
        else if (nextAction == PendingAction::ConfirmCloseForOpen) {
            if (!m_nextFilePath.empty()) {
                performFileOpen(m_nextFilePath);
                m_nextFilePath.clear();
            } else {
                m_dialog.show(DialogType::Input, L"Open File", L"Enter file path:");
                m_pendingState = PendingAction::OpenFilePath;
            }
        } 
        else if (nextAction == PendingAction::ConfirmCloseForQuit) {
            m_running = false;
        }
    }
}

void Editor::performFileSave() {
    if (m_buffer.getFilePath().empty()) {
        performFileSaveAs(PendingAction::None);
    } else {
        if (m_buffer.save()) {
            m_explorer.refresh(); // Refresh in case file created/saved
        } else {
            m_dialog.show(DialogType::Alert, L"Save Error", L"Could not save file!");
            m_pendingState = PendingAction::None;
        }
    }
}

void Editor::performFileSaveAs(PendingAction nextState) {
    m_dialog.show(DialogType::Input, L"Save As", L"Enter save file path:", m_buffer.getFilePath());
    if (nextState == PendingAction::None) {
        m_pendingState = PendingAction::SaveAsSave;
    } else {
        m_pendingState = nextState;
    }
}

void Editor::performFileOpen(const std::wstring& path) {
    if (m_buffer.load(path)) {
        m_textArea.setScrollOffset({0, 0});
        m_textArea.scrollToCursor();
    } else {
        m_dialog.show(DialogType::Alert, L"Open Error", L"Could not load the specified file!");
        m_pendingState = PendingAction::None;
    }
}

void Editor::handleDialogCompletion() {
    DialogResult res = m_dialog.getResult();
    std::wstring inputVal = m_dialog.getInputValue();

    PendingAction state = m_pendingState;
    m_pendingState = PendingAction::None;

    switch (state) {
        case PendingAction::ConfirmCloseForNew:
            if (res == DialogResult::Yes) {
                performFileSaveAs(PendingAction::SaveAsNew);
            } else if (res == DialogResult::No) {
                m_buffer.clear();
            }
            break;

        case PendingAction::ConfirmCloseForOpen:
            if (res == DialogResult::Yes) {
                performFileSaveAs(PendingAction::SaveAsOpen);
            } else if (res == DialogResult::No) {
                if (!m_nextFilePath.empty()) {
                    performFileOpen(m_nextFilePath);
                    m_nextFilePath.clear();
                } else {
                    m_dialog.show(DialogType::Input, L"Open File", L"Enter file path:");
                    m_pendingState = PendingAction::OpenFilePath;
                }
            }
            break;

        case PendingAction::ConfirmCloseForQuit:
            if (res == DialogResult::Yes) {
                performFileSaveAs(PendingAction::SaveAsQuit);
            } else if (res == DialogResult::No) {
                m_running = false;
            }
            break;

        case PendingAction::SaveAsNew:
            if (res == DialogResult::Ok && !inputVal.empty()) {
                if (m_buffer.saveAs(inputVal)) {
                    m_buffer.clear();
                    m_explorer.refresh();
                } else {
                    m_dialog.show(DialogType::Alert, L"Save Error", L"Failed to save document!");
                }
            }
            break;

        case PendingAction::SaveAsOpen:
            if (res == DialogResult::Ok && !inputVal.empty()) {
                if (m_buffer.saveAs(inputVal)) {
                    m_explorer.refresh();
                    if (!m_nextFilePath.empty()) {
                        performFileOpen(m_nextFilePath);
                        m_nextFilePath.clear();
                    } else {
                        m_dialog.show(DialogType::Input, L"Open File", L"Enter file path:");
                        m_pendingState = PendingAction::OpenFilePath;
                    }
                } else {
                    m_dialog.show(DialogType::Alert, L"Save Error", L"Failed to save document!");
                }
            }
            break;

        case PendingAction::SaveAsSave:
            if (res == DialogResult::Ok && !inputVal.empty()) {
                if (m_buffer.saveAs(inputVal)) {
                    m_explorer.refresh();
                } else {
                    m_dialog.show(DialogType::Alert, L"Save Error", L"Failed to save document!");
                }
            }
            break;

        case PendingAction::SaveAsQuit:
            if (res == DialogResult::Ok && !inputVal.empty()) {
                if (m_buffer.saveAs(inputVal)) {
                    m_running = false;
                } else {
                    m_dialog.show(DialogType::Alert, L"Save Error", L"Failed to save document!");
                }
            }
            break;

        case PendingAction::OpenFilePath:
            if (res == DialogResult::Ok && !inputVal.empty()) {
                performFileOpen(inputVal);
            }
            break;

        case PendingAction::SearchQuery:
            if (res == DialogResult::Ok && !inputVal.empty()) {
                findText(inputVal);
            }
            break;

        default:
            break;
    }
}

void Editor::findText(const std::wstring& query) {
    if (query.empty()) return;

    int lineCount = static_cast<int>(m_buffer.getLineCount());
    Point cur = m_buffer.getCursor();

    // Start scanning from current cursor location + 1, to find next occurrences
    int startY = cur.y;
    int startX = cur.x;

    bool found = false;
    int foundY = -1;
    size_t foundX = std::wstring::npos;

    // Scan lines starting from active row
    for (int y = startY; y < lineCount; ++y) {
        const std::wstring& line = m_buffer.getLine(y);
        size_t idx = (y == startY) ? line.find(query, startX) : line.find(query);
        if (idx != std::wstring::npos) {
            found = true;
            foundY = y;
            foundX = idx;
            break;
        }
    }

    // Wrap around to scan from beginning
    if (!found) {
        for (int y = 0; y <= startY; ++y) {
            const std::wstring& line = m_buffer.getLine(y);
            size_t maxLimit = (y == startY) ? startX : std::wstring::npos;
            size_t idx = line.find(query);
            if (idx != std::wstring::npos && (y < startY || idx < maxLimit)) {
                found = true;
                foundY = y;
                foundX = idx;
                break;
            }
        }
    }

    if (found) {
        // Highlight selection
        m_buffer.clearSelection();
        m_buffer.setSelectionStart({static_cast<int>(foundX), foundY});
        m_buffer.setSelectionEnd({static_cast<int>(foundX + query.length()), foundY});
        m_buffer.setCursor({static_cast<int>(foundX + query.length()), foundY});
        
        m_textArea.scrollToCursor();
    } else {
        m_dialog.show(DialogType::Alert, L"Find Text", L"Text not found: " + query);
        m_pendingState = PendingAction::None;
    }
}

} // namespace PaperPad
