#pragma once

#include <windows.h>
#include <string>
#include "utils/Types.hpp"

namespace PaperPad {

class Console {
public:
    Console();
    ~Console();

    bool init();
    void cleanup();

    void clear(WORD attr = 0);
    void writeChar(int x, int y, wchar_t ch, WORD attr);
    void writeText(int x, int y, const std::wstring& text, WORD attr);
    void fillRect(const Rect& rect, wchar_t ch, WORD attr);
    
    // Draw borders
    void drawBorder(const Rect& rect, WORD attr, bool active);

    void present();

    // Event handling
    bool readEvent(UIEvent& ev);

    // Getters and Setters
    void getDimensions(int& width, int& height) const {
        width = m_width;
        height = m_height;
    }
    int getWidth() const { return m_width; }
    int getHeight() const { return m_height; }

    void setCursorPos(int x, int y);
    void showCursor(bool show);

    const Color::Scheme& getTheme() const { return m_theme; }
    void setTheme(const Color::Scheme& theme) { m_theme = theme; }

private:
    HANDLE m_hIn;
    HANDLE m_hOut;
    
    DWORD m_oldInMode;
    DWORD m_oldOutMode;
    
    int m_width;
    int m_height;
    
    CHAR_INFO* m_screenBuffer;
    Color::Scheme m_theme;

    void updateBufferSize(int w, int h);
};

} // namespace PaperPad
