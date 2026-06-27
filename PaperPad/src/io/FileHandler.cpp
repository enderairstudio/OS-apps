#include "io/FileHandler.hpp"
#include "utils/Logger.hpp"
#include <fstream>
#include <sstream>

namespace PaperPad {

// Helper to convert UTF-8 string to std::wstring
static std::wstring Utf8ToWstring(const std::string& str) {
    if (str.empty()) return L"";
    int sizeNeeded = MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), NULL, 0);
    std::wstring wstrTo(sizeNeeded, 0);
    MultiByteToWideChar(CP_UTF8, 0, &str[0], (int)str.size(), &wstrTo[0], sizeNeeded);
    return wstrTo;
}

// Helper to convert std::wstring to UTF-8 string
static std::string WstringToUtf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string strTo(sizeNeeded, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &strTo[0], sizeNeeded, NULL, NULL);
    return strTo;
}

bool FileHandler::showOpenDialog(HWND hwndOwner, std::wstring& outPath) {
    OPENFILENAMEW ofn;
    wchar_t szFile[MAX_PATH] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndOwner;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
    ofn.lpstrFilter = L"Text Documents (*.txt)\0*.txt\0C++ Files (*.cpp;*.h;*.hpp)\0*.cpp;*.h;*.hpp\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;

    if (GetOpenFileNameW(&ofn)) {
        outPath = szFile;
        return true;
    }
    return false;
}

bool FileHandler::showSaveDialog(HWND hwndOwner, std::wstring& outPath) {
    OPENFILENAMEW ofn;
    wchar_t szFile[MAX_PATH] = { 0 };

    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hwndOwner;
    ofn.lpstrFile = szFile;
    ofn.nMaxFile = sizeof(szFile) / sizeof(szFile[0]);
    ofn.lpstrFilter = L"Text Documents (*.txt)\0*.txt\0C++ Files (*.cpp;*.h;*.hpp)\0*.cpp;*.h;*.hpp\0All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT | OFN_NOCHANGEDIR;

    if (GetSaveFileNameW(&ofn)) {
        outPath = szFile;
        return true;
    }
    return false;
}

bool FileHandler::loadFile(const std::wstring& filepath, std::wstring& outContent) {
    outContent.clear();
    
    std::ifstream file(filepath.c_str(), std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file: " + WstringToUtf8(filepath));
        return false;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();
    file.close();

    outContent = Utf8ToWstring(content);
    
    // Normalize line endings to Windows CRLF for the Edit control to prevent rendering anomalies
    std::wstring normalized;
    normalized.reserve(outContent.length());
    for (size_t i = 0; i < outContent.length(); ++i) {
        if (outContent[i] == L'\n') {
            if (i == 0 || outContent[i - 1] != L'\r') {
                normalized += L'\r';
            }
        }
        normalized += outContent[i];
    }
    outContent = std::move(normalized);

    LOG_INFO("Successfully loaded: " + WstringToUtf8(filepath));
    return true;
}

bool FileHandler::saveFile(const std::wstring& filepath, const std::wstring& content) {
    std::ofstream file(filepath.c_str(), std::ios::binary | std::ios::trunc);
    if (!file.is_open()) {
        LOG_ERROR("Failed to save file: " + WstringToUtf8(filepath));
        return false;
    }

    std::string utf8Content = WstringToUtf8(content);
    file.write(utf8Content.c_str(), utf8Content.size());
    file.close();

    LOG_INFO("Successfully saved: " + WstringToUtf8(filepath));
    return true;
}

} // namespace PaperPad
