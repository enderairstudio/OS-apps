#pragma once

#include <windows.h>
#include "core/MainWindow.hpp"

namespace PaperPad {

class Application {
public:
    Application();
    ~Application();

    bool init();
    int run(int nCmdShow);

private:
    MainWindow m_mainWindow;
};

} // namespace PaperPad
