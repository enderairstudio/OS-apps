#include "core/Application.hpp"
#include <windows.h>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow) {
    PaperPad::Application app;
    if (!app.init()) {
        return 1;
    }
    return app.run(nCmdShow);
}
