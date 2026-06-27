#pragma once

#include <windows.h>
#include <string>

namespace PaperPad {

class FileHandler {
public:
    // Standard system dialogs using Comdlg32
    static bool showOpenDialog(HWND hwndOwner, std::wstring& outPath);
    static bool showSaveDialog(HWND hwndOwner, std::wstring& outPath);

    // Read file contents as a raw wide string (retains line break structures for edit controls)
    static bool loadFile(const std::wstring& filepath, std::wstring& outContent);

    // Save content to a file, handling UTF-8 serialization
    static bool saveFile(const std::wstring& filepath, const std::wstring& content);
};

} // namespace PaperPad
