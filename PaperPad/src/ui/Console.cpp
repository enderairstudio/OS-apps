#include "ui/Console.hpp"
#include "utils/Logger.hpp"
#include <iostream>

namespace PaperPad {

Console::Console()
    : m_hIn(INVALID_HANDLE_VALUE)
    , m_hOut(INVALID_HANDLE_VALUE)
    , m_oldInMode(0)
    , m_oldOutMode(0)
    , m_width(80)
    , m_height(25)
    , m_screenBuffer(nullptr) {
    m_theme = Color::DarkTheme();
}

Console::~Console() {
    cleanup();
}

bool Console::init() {
    m_hIn = GetStdHandle(STD_INPUT_HANDLE);
    m_hOut = GetStdHandle(STD_OUTPUT_HANDLE);

    if (m_hIn == INVALID_HANDLE_VALUE || m_hOut == INVALID_HANDLE_VALUE) {
        return false;
    }

    // Save current modes
    GetConsoleMode(m_hIn, &m_oldInMode);
    GetConsoleMode(m_hOut, &m_oldOutMode);

    // Set console mode to support mouse events, window changes, and virtual terminal processing if available
    DWORD inMode = ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT | ENABLE_EXTENDED_FLAGS;
    SetConsoleMode(m_hIn, inMode);

    DWORD outMode = ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
    SetConsoleMode(m_hOut, outMode);

    // Get initial size
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(m_hOut, &csbi)) {
        m_width = csbi.srWindow.Right - csbi.srWindow.Left + 1;
        m_height = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
    }

    // Set window title
    SetConsoleTitleW(L"PaperPad C++ Editor (Ctrl+Q to Quit, Ctrl+O to Open, Ctrl+S to Save)");

    // Enable Unicode output mode on console
    SetConsoleCP(CP_UTF8);
    SetConsoleOutputCP(CP_UTF8);

    updateBufferSize(m_width, m_height);
    showCursor(true);

    LOG_INFO("Console UI initialized with dimensions: " + std::to_string(m_width) + "x" + std::to_string(m_height));
    return true;
}

void Console::cleanup() {
    showCursor(true);
    if (m_hIn != INVALID_HANDLE_VALUE) {
        SetConsoleMode(m_hIn, m_oldInMode);
    }
    if (m_hOut != INVALID_HANDLE_VALUE) {
        SetConsoleMode(m_hOut, m_oldOutMode);
    }
    delete[] m_screenBuffer;
    m_screenBuffer = nullptr;
}

void Console::updateBufferSize(int w, int h) {
    if (w <= 0) w = 80;
    if (h <= 0) h = 25;

    if (m_width != w || m_height != h || m_screenBuffer == nullptr) {
        m_width = w;
        m_height = h;

        delete[] m_screenBuffer;
        m_screenBuffer = new CHAR_INFO[m_width * m_height];
    }
    clear(m_theme.background);
}

void Console::clear(WORD attr) {
    if (!m_screenBuffer) return;
    for (int i = 0; i < m_width * m_height; ++i) {
        m_screenBuffer[i].Char.UnicodeChar = L' ';
        m_screenBuffer[i].Attributes = attr;
    }
}

void Console::writeChar(int x, int y, wchar_t ch, WORD attr) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height && m_screenBuffer) {
        m_screenBuffer[y * m_width + x].Char.UnicodeChar = ch;
        m_screenBuffer[y * m_width + x].Attributes = attr;
    }
}

void Console::writeText(int x, int y, const std::wstring& text, WORD attr) {
    if (y < 0 || y >= m_height || !m_screenBuffer) return;
    for (size_t i = 0; i < text.length(); ++i) {
        int cx = x + static_cast<int>(i);
        if (cx >= m_width) break;
        if (cx < 0) continue;
        m_screenBuffer[y * m_width + cx].Char.UnicodeChar = text[i];
        m_screenBuffer[y * m_width + cx].Attributes = attr;
    }
}

void Console::fillRect(const Rect& rect, wchar_t ch, WORD attr) {
    for (int y = rect.y; y < rect.y + rect.h; ++y) {
        for (int x = rect.x; x < rect.x + rect.w; ++x) {
            writeChar(x, y, ch, attr);
        }
    }
}

void Console::drawBorder(const Rect& rect, WORD attr, bool active) {
    if (rect.w <= 1 || rect.h <= 1) return;

    wchar_t tl = active ? L'╔' : L'┌';
    wchar_t tr = active ? L'╗' : L'┐';
    wchar_t bl = active ? L'╚' : L'└';
    wchar_t br = active ? L'╝' : L'┘';
    wchar_t horz = active ? L'═' : L'─';
    wchar_t vert = active ? L'║' : L'│';

    // Top & bottom lines
    for (int x = rect.x + 1; x < rect.x + rect.w - 1; ++x) {
        writeChar(x, rect.y, horz, attr);
        writeChar(x, rect.y + rect.h - 1, horz, attr);
    }

    // Left & right lines
    for (int y = rect.y + 1; y < rect.y + rect.h - 1; ++y) {
        writeChar(rect.x, y, vert, attr);
        writeChar(rect.x + rect.w - 1, y, vert, attr);
    }

    // Corners
    writeChar(rect.x, rect.y, tl, attr);
    writeChar(rect.x + rect.w - 1, rect.y, tr, attr);
    writeChar(rect.x, rect.y + rect.h - 1, bl, attr);
    writeChar(rect.x + rect.w - 1, rect.y + rect.h - 1, br, attr);
}

void Console::present() {
    if (!m_screenBuffer) return;

    COORD charBufSize = { (SHORT)m_width, (SHORT)m_height };
    COORD charBufCoord = { 0, 0 };
    SMALL_RECT writeRegion = { 0, 0, (SHORT)(m_width - 1), (SHORT)(m_height - 1) };

    WriteConsoleOutputW(m_hOut, m_screenBuffer, charBufSize, charBufCoord, &writeRegion);
}

bool Console::readEvent(UIEvent& ev) {
    INPUT_RECORD record;
    DWORD readCount = 0;

    while (true) {
        if (!ReadConsoleInputW(m_hIn, &record, 1, &readCount) || readCount == 0) {
            return false;
        }

        if (record.EventType == KEY_EVENT) {
            auto& keyEvent = record.Event.KeyEvent;
            if (!keyEvent.bKeyDown) {
                // We typically handle keydown events only, EXCEPT if it's Alt, which can be sticky.
                // But for keyup, just skip.
                continue;
            }

            ev.type = UIEvent::Type::Key;
            ev.key.controlPressed = (keyEvent.dwControlKeyState & (LEFT_CTRL_PRESSED | RIGHT_CTRL_PRESSED)) != 0;
            ev.key.shiftPressed = (keyEvent.dwControlKeyState & SHIFT_PRESSED) != 0;
            ev.key.altPressed = (keyEvent.dwControlKeyState & (LEFT_ALT_PRESSED | RIGHT_ALT_PRESSED)) != 0;
            ev.key.ch = keyEvent.uChar.UnicodeChar;
            ev.key.code = Key::Unknown;

            // Map special keys
            switch (keyEvent.wVirtualKeyCode) {
                case VK_UP:      ev.key.code = Key::Up; break;
                case VK_DOWN:    ev.key.code = Key::Down; break;
                case VK_LEFT:    ev.key.code = Key::Left; break;
                case VK_RIGHT:   ev.key.code = Key::Right; break;
                case VK_BACK:    ev.key.code = Key::Backspace; break;
                case VK_RETURN:  ev.key.code = Key::Enter; break;
                case VK_DELETE:  ev.key.code = Key::Delete; break;
                case VK_TAB:     ev.key.code = Key::Tab; break;
                case VK_ESCAPE:  ev.key.code = Key::Escape; break;
                case VK_HOME:    ev.key.code = Key::Home; break;
                case VK_END:     ev.key.code = Key::End; break;
                case VK_PRIOR:   ev.key.code = Key::PageUp; break;
                case VK_NEXT:    ev.key.code = Key::PageDown; break;
                case VK_F1:      ev.key.code = Key::F1; break;
                case VK_F2:      ev.key.code = Key::F2; break;
                case VK_F3:      ev.key.code = Key::F3; break;
                case VK_F4:      ev.key.code = Key::F4; break;
                case VK_F5:      ev.key.code = Key::F5; break;
                case VK_F6:      ev.key.code = Key::F6; break;
                case VK_F7:      ev.key.code = Key::F7; break;
                case VK_F8:      ev.key.code = Key::F8; break;
                case VK_F9:      ev.key.code = Key::F9; break;
                case VK_F10:     ev.key.code = Key::F10; break;
                case VK_F11:     ev.key.code = Key::F11; break;
                case VK_F12:     ev.key.code = Key::F12; break;
                default:         ev.key.code = Key::Char; break;
            }

            // Map shortcuts
            if (ev.key.controlPressed) {
                wchar_t wc = keyEvent.wVirtualKeyCode;
                if (wc == 'S') ev.key.code = Key::Ctrl_S;
                else if (wc == 'O') ev.key.code = Key::Ctrl_O;
                else if (wc == 'N') ev.key.code = Key::Ctrl_N;
                else if (wc == 'Q') ev.key.code = Key::Ctrl_Q;
                else if (wc == 'Z') ev.key.code = Key::Ctrl_Z;
                else if (wc == 'Y') ev.key.code = Key::Ctrl_Y;
                else if (wc == 'F') ev.key.code = Key::Ctrl_F;
                else if (wc == 'H') ev.key.code = Key::Ctrl_H;
                else if (wc == 'A') ev.key.code = Key::Ctrl_A;
                else if (wc == 'W') ev.key.code = Key::Ctrl_W;
            }

            return true;
        } 
        else if (record.EventType == MOUSE_EVENT) {
            ev.type = UIEvent::Type::Mouse;
            ev.mouse.pos = { record.Event.MouseEvent.dwMousePosition.X, record.Event.MouseEvent.dwMousePosition.Y };
            ev.mouse.buttonState = record.Event.MouseEvent.dwButtonState;
            ev.mouse.eventFlags = record.Event.MouseEvent.dwEventFlags;
            return true;
        } 
        else if (record.EventType == WINDOW_BUFFER_SIZE_EVENT) {
            int w = record.Event.WindowBufferSizeEvent.dwSize.X;
            int h = record.Event.WindowBufferSizeEvent.dwSize.Y;
            
            // Query actual screen buffer info just in case
            CONSOLE_SCREEN_BUFFER_INFO csbi;
            if (GetConsoleScreenBufferInfo(m_hOut, &csbi)) {
                w = csbi.srWindow.Right - csbi.srWindow.Left + 1;
                h = csbi.srWindow.Bottom - csbi.srWindow.Top + 1;
            }

            ev.type = UIEvent::Type::Resize;
            ev.resize.width = w;
            ev.resize.height = h;

            updateBufferSize(w, h);
            return true;
        }
    }
}

void Console::setCursorPos(int x, int y) {
    if (x >= 0 && x < m_width && y >= 0 && y < m_height) {
        COORD pos = { (SHORT)x, (SHORT)y };
        SetConsoleCursorPosition(m_hOut, pos);
    }
}

void Console::showCursor(bool show) {
    CONSOLE_CURSOR_INFO cci;
    if (GetConsoleCursorInfo(m_hOut, &cci)) {
        cci.bVisible = show ? TRUE : FALSE;
        // Make the cursor standard size
        cci.dwSize = 20; 
        SetConsoleCursorInfo(m_hOut, &cci);
    }
}

} // namespace PaperPad
