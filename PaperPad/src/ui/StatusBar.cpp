#include "ui/StatusBar.hpp"
#include <sstream>

namespace PaperPad {

StatusBar::StatusBar() {
    m_bounds = {0, 24, 80, 1};
    m_cursorPos = {0, 0};
}

void StatusBar::update(const Buffer& buffer) {
    m_filename = buffer.getFileName();
    m_dirty = buffer.isDirty();
    m_cursorPos = buffer.getCursor();
    m_lineCount = buffer.getLineCount();
}

void StatusBar::draw(Console& console) {
    const auto& theme = console.getTheme();

    // Fill background
    console.fillRect(m_bounds, L' ', theme.statusBar);

    // Left info
    std::wstringstream leftWss;
    leftWss << L"  " << m_filename << (m_dirty ? L"*" : L"") << L"  |  Lines: " << m_lineCount;
    std::wstring leftStr = leftWss.str();

    // Right info
    std::wstringstream rightWss;
    rightWss << L"Ln " << (m_cursorPos.y + 1) << L", Col " << (m_cursorPos.x + 1) << L"  [UTF-8]  [CRLF]  ";
    std::wstring rightStr = rightWss.str();

    // Center help hints (adjust depending on spacing)
    std::wstring centerStr = L"Ctrl+O Open ∙ Ctrl+S Save ∙ Ctrl+F Find ∙ Ctrl+Z Undo ∙ Ctrl+W Sidebar";

    // Write left string
    console.writeText(m_bounds.x, m_bounds.y, leftStr, theme.statusBarText);

    // Write right string (aligned to right edge)
    int rightX = m_bounds.x + m_bounds.w - static_cast<int>(rightStr.length());
    if (rightX > m_bounds.x + static_cast<int>(leftStr.length())) {
        console.writeText(rightX, m_bounds.y, rightStr, theme.statusBarText);
    }

    // Write center string if it fits
    int centerX = m_bounds.x + (m_bounds.w - static_cast<int>(centerStr.length())) / 2;
    int leftLimit = m_bounds.x + static_cast<int>(leftStr.length()) + 2;
    int rightLimit = rightX - 2;

    if (centerX >= leftLimit && centerX + static_cast<int>(centerStr.length()) <= rightLimit) {
        console.writeText(centerX, m_bounds.y, centerStr, theme.statusBarText);
    }
}

void StatusBar::handleInput(const UIEvent& ev) {
    // StatusBar doesn't handle inputs
}

} // namespace PaperPad
