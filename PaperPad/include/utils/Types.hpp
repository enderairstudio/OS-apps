#pragma once

#include <windows.h>
#include <string>

namespace PaperPad {

struct Point {
    int x = 0;
    int y = 0;

    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
};

struct Rect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;

    bool contains(const Point& p) const {
        return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
    }
};

// Console Color codes combining Foreground (FG) and Background (BG)
namespace Color {
    // Standard 16-color console palette attributes
    const WORD BLACK          = 0;
    const WORD DARK_BLUE      = FOREGROUND_BLUE;
    const WORD DARK_GREEN     = FOREGROUND_GREEN;
    const WORD DARK_CYAN      = FOREGROUND_GREEN | FOREGROUND_BLUE;
    const WORD DARK_RED       = FOREGROUND_RED;
    const WORD DARK_MAGENTA   = FOREGROUND_RED | FOREGROUND_BLUE;
    const WORD DARK_YELLOW    = FOREGROUND_RED | FOREGROUND_GREEN;
    const WORD LIGHT_GRAY     = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE;
    const WORD DARK_GRAY      = FOREGROUND_INTENSITY;
    const WORD BLUE           = FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    const WORD GREEN          = FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    const WORD CYAN           = FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    const WORD RED            = FOREGROUND_RED | FOREGROUND_INTENSITY;
    const WORD MAGENTA        = FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY;
    const WORD YELLOW         = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY;
    const WORD WHITE          = FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY;

    // Background modifiers
    const WORD BG_BLACK        = 0;
    const WORD BG_DARK_BLUE    = BACKGROUND_BLUE;
    const WORD BG_DARK_GREEN   = BACKGROUND_GREEN;
    const WORD BG_DARK_CYAN    = BACKGROUND_GREEN | BACKGROUND_BLUE;
    const WORD BG_DARK_RED     = BACKGROUND_RED;
    const WORD BG_DARK_MAGENTA = BACKGROUND_RED | BACKGROUND_BLUE;
    const WORD BG_DARK_YELLOW  = BACKGROUND_RED | BACKGROUND_GREEN;
    const WORD BG_LIGHT_GRAY   = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE;
    const WORD BG_DARK_GRAY    = BACKGROUND_INTENSITY;
    const WORD BG_BLUE         = BACKGROUND_BLUE | BACKGROUND_INTENSITY;
    const WORD BG_GREEN        = BACKGROUND_GREEN | BACKGROUND_INTENSITY;
    const WORD BG_CYAN         = BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
    const WORD BG_RED          = BACKGROUND_RED | BACKGROUND_INTENSITY;
    const WORD BG_MAGENTA      = BACKGROUND_RED | BACKGROUND_BLUE | BACKGROUND_INTENSITY;
    const WORD BG_YELLOW       = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_INTENSITY;
    const WORD BG_WHITE        = BACKGROUND_RED | BACKGROUND_GREEN | BACKGROUND_BLUE | BACKGROUND_INTENSITY;

    // Theme schemes
    struct Scheme {
        WORD text;
        WORD background;
        WORD selectedText;
        WORD selectedBackground;
        WORD cursor;
        WORD border;
        WORD activeBorder;
        WORD menuBar;
        WORD menuBarActive;
        WORD menuBarText;
        WORD statusBar;
        WORD statusBarText;
        WORD sidebarText;
        WORD sidebarBackground;
        WORD lineNumbers;
        WORD keyword;
        WORD stringLiteral;
        WORD comment;
        WORD number;
    };

    // Default dark theme
    inline Scheme DarkTheme() {
        Scheme s;
        s.text = LIGHT_GRAY | BG_BLACK;
        s.background = BG_BLACK;
        s.selectedText = WHITE | BG_DARK_BLUE;
        s.selectedBackground = BG_DARK_BLUE;
        s.cursor = BLACK | BG_WHITE;
        s.border = DARK_GRAY | BG_BLACK;
        s.activeBorder = WHITE | BG_BLACK;
        s.menuBar = BLACK | BG_LIGHT_GRAY;
        s.menuBarActive = WHITE | BG_DARK_BLUE;
        s.menuBarText = BLACK | BG_LIGHT_GRAY;
        s.statusBar = WHITE | BG_DARK_BLUE;
        s.statusBarText = WHITE | BG_DARK_BLUE;
        s.sidebarText = LIGHT_GRAY | BG_DARK_GRAY;
        s.sidebarBackground = BG_DARK_GRAY;
        s.lineNumbers = DARK_GRAY | BG_BLACK;
        
        // Syntax highlighting attributes
        s.keyword = BLUE | BG_BLACK;
        s.stringLiteral = GREEN | BG_BLACK;
        s.comment = DARK_GREEN | BG_BLACK;
        s.number = YELLOW | BG_BLACK;
        return s;
    }
}

enum class Key {
    Unknown = 0,
    Char,
    Enter,
    Backspace,
    Delete,
    Tab,
    Up,
    Down,
    Left,
    Right,
    Home,
    End,
    PageUp,
    PageDown,
    Escape,
    
    // Shortcuts (Ctrl + Key)
    Ctrl_S,
    Ctrl_O,
    Ctrl_N,
    Ctrl_Q,
    Ctrl_Z,
    Ctrl_Y,
    Ctrl_F,
    Ctrl_H,
    Ctrl_A,
    Ctrl_W,
    
    // Function keys
    F1, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12
};

struct UIEvent {
    enum class Type { Key, Mouse, Resize } type;
    
    // Keyboard details
    struct KeyDetail {
        Key code;
        wchar_t ch;
        bool controlPressed;
        bool shiftPressed;
        bool altPressed;
    } key;

    // Mouse details
    struct MouseDetail {
        Point pos;
        DWORD buttonState;
        DWORD eventFlags;
    } mouse;

    // Resize details
    struct ResizeDetail {
        int width;
        int height;
    } resize;
};

} // namespace PaperPad
