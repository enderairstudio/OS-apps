#pragma once

#include <windows.h>
#include <string>

namespace PaperPad {

class EditorCtrl {
public:
    EditorCtrl();
    ~EditorCtrl();

    bool create(HWND hwndParent, int id);
    void destroy();

    HWND getHWND() const { return m_hWnd; }

    void setBounds(int x, int y, int w, int h);
    void setFocus();

    std::wstring getText() const;
    void setText(const std::wstring& text);

    bool isModified() const;
    void setModified(bool modified);

    void undo();
    void cut();
    void copy();
    void paste();
    void deleteSelected();
    void selectAll();

    void setFont(const std::wstring& faceName, int sizePoint);

private:
    HWND m_hWnd;
    HFONT m_hFont;
};

} // namespace PaperPad
