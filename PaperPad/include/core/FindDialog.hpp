#pragma once

#include <windows.h>
#include <string>

namespace PaperPad {

class FindDialog {
public:
    // Shows a modal Win32 dialog window to enter search terms.
    static bool show(HWND hwndParent, std::wstring& outQuery);
};

} // namespace PaperPad
