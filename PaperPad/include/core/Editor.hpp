#pragma once

#include "ui/Console.hpp"
#include "core/Buffer.hpp"
#include "ui/MenuBar.hpp"
#include "ui/StatusBar.hpp"
#include "ui/FileExplorer.hpp"
#include "ui/TextArea.hpp"
#include "ui/Dialog.hpp"

namespace PaperPad {

class Editor {
public:
    Editor();
    ~Editor();

    bool init();
    void run();

private:
    Console m_console;
    Buffer m_buffer;

    MenuBar m_menuBar;
    StatusBar m_statusBar;
    FileExplorer m_explorer;
    TextArea m_textArea;
    Dialog m_dialog;

    bool m_running = true;

    // Active state track for callbacks/nested queries
    enum class PendingAction {
        None,
        ConfirmCloseForNew,
        ConfirmCloseForOpen,
        ConfirmCloseForQuit,
        SaveAsNew,
        SaveAsOpen,
        SaveAsSave,
        SaveAsQuit,
        OpenFilePath,
        SearchQuery
    } m_pendingState = PendingAction::None;

    std::wstring m_nextFilePath; // Temp storage during confirmation dialogs

    void layoutComponents();
    void processEvents();
    
    void handleMenuAction(MenuAction action);
    void handleDialogCompletion();
    
    void checkSaveStateBeforeAction(PendingAction nextAction);
    void performFileSave();
    void performFileSaveAs(PendingAction nextState);
    void performFileOpen(const std::wstring& path);

    void findText(const std::wstring& query);
};

} // namespace PaperPad
