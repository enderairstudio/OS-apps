#include "core/EditorCtrl.hpp"
#include <vector>

namespace PaperPad {

EditorCtrl::EditorCtrl() : m_hWnd(NULL), m_hFont(NULL) {}

EditorCtrl::~EditorCtrl() {
    destroy();
}

bool EditorCtrl::create(HWND hwndParent, int id) {
    m_hWnd = CreateWindowExW(
        0,
        L"EDIT",
        L"",
        WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | 
        ES_MULTILINE | ES_AUTOVSCROLL | ES_AUTOHSCROLL | ES_NOHIDESEL,
        0, 0, 0, 0,
        hwndParent,
        (HMENU)(UINT_PTR)id,
        GetModuleHandle(NULL),
        NULL
    );

    if (!m_hWnd) {
        return false;
    }

    // Set a premium default font (Consolas size 11)
    setFont(L"Consolas", 11);
    return true;
}

void EditorCtrl::destroy() {
    if (m_hFont) {
        DeleteObject(m_hFont);
        m_hFont = NULL;
    }
    m_hWnd = NULL;
}

void EditorCtrl::setBounds(int x, int y, int w, int h) {
    if (m_hWnd) {
        MoveWindow(m_hWnd, x, y, w, h, TRUE);
    }
}

void EditorCtrl::setFocus() {
    if (m_hWnd) {
        SetFocus(m_hWnd);
    }
}

std::wstring EditorCtrl::getText() const {
    if (!m_hWnd) return L"";

    int len = GetWindowTextLengthW(m_hWnd);
    if (len <= 0) return L"";

    std::wstring text(len + 1, 0);
    int read = GetWindowTextW(m_hWnd, &text[0], len + 1);
    text.resize(read);
    return text;
}

void EditorCtrl::setText(const std::wstring& text) {
    if (m_hWnd) {
        SetWindowTextW(m_hWnd, text.c_str());
    }
}

bool EditorCtrl::isModified() const {
    if (!m_hWnd) return false;
    return SendMessageW(m_hWnd, EM_GETMODIFY, 0, 0) != 0;
}

void EditorCtrl::setModified(bool modified) {
    if (m_hWnd) {
        SendMessageW(m_hWnd, EM_SETMODIFY, modified ? TRUE : FALSE, 0);
    }
}

void EditorCtrl::undo() {
    if (m_hWnd) {
        SendMessageW(m_hWnd, EM_UNDO, 0, 0);
    }
}

void EditorCtrl::cut() {
    if (m_hWnd) {
        SendMessageW(m_hWnd, WM_CUT, 0, 0);
    }
}

void EditorCtrl::copy() {
    if (m_hWnd) {
        SendMessageW(m_hWnd, WM_COPY, 0, 0);
    }
}

void EditorCtrl::paste() {
    if (m_hWnd) {
        SendMessageW(m_hWnd, WM_PASTE, 0, 0);
    }
}

void EditorCtrl::deleteSelected() {
    if (m_hWnd) {
        SendMessageW(m_hWnd, WM_CLEAR, 0, 0);
    }
}

void EditorCtrl::selectAll() {
    if (m_hWnd) {
        SendMessageW(m_hWnd, EM_SETSEL, 0, -1);
    }
}

void EditorCtrl::setFont(const std::wstring& faceName, int sizePoint) {
    if (!m_hWnd) return;

    HDC hdc = GetDC(m_hWnd);
    int height = -MulDiv(sizePoint, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    ReleaseDC(m_hWnd, hdc);

    HFONT newFont = CreateFontW(
        height, 0, 0, 0,
        FW_NORMAL, FALSE, FALSE, FALSE,
        DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
        CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE,
        faceName.c_str()
    );

    if (newFont) {
        SendMessageW(m_hWnd, WM_SETFONT, (WPARAM)newFont, TRUE);
        if (m_hFont) {
            DeleteObject(m_hFont);
        }
        m_hFont = newFont;
    }
}

} // namespace PaperPad
