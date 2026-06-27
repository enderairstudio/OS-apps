#include "core/Application.hpp"
#include "utils/Logger.hpp"
#include <commctrl.h>

namespace PaperPad {

Application::Application() {}

Application::~Application() {}

bool Application::init() {
    Logger::getInstance().init("editor.log");
    LOG_INFO("Initializing Application...");

    // Initialize Common Controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_WIN95_CLASSES;
    InitCommonControlsEx(&icex);

    if (!m_mainWindow.create(L"PaperPad", 800, 600)) {
        LOG_ERROR("Failed to create MainWindow.");
        return false;
    }

    return true;
}

int Application::run(int nCmdShow) {
    m_mainWindow.show(nCmdShow);

    HACCEL hAccel = m_mainWindow.getAccel();
    HWND hwndMain = m_mainWindow.getHWND();
    MSG msg;

    while (GetMessageW(&msg, NULL, 0, 0)) {
        if (!TranslateAcceleratorW(hwndMain, hAccel, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
    }

    LOG_INFO("Application exiting.");
    return (int)msg.wParam;
}

} // namespace PaperPad
