#include "ui/TextArea.hpp"
#include "utils/Logger.hpp"
#include <windows.h>
#include <sstream>
#include <cmath>
#include <unordered_set>

namespace PaperPad {

static const std::unordered_set<std::wstring> CPP_KEYWORDS = {
    L"alignas", L"alignof", L"and", L"and_eq", L"asm", L"atomic_cancel", L"atomic_commit", 
    L"atomic_noexcept", L"auto", L"bitand", L"bitor", L"bool", L"break", L"case", L"catch", 
    L"char", L"char8_t", L"char16_t", L"char32_t", L"class", L"compl", L"concept", L"const", 
    L"consteval", L"constexpr", L"constinit", L"const_cast", L"continue", L"co_await", 
    L"co_return", L"co_yield", L"decltype", L"default", L"delete", L"do", L"double", 
    L"dynamic_cast", L"else", L"enum", L"explicit", L"export", L"extern", L"false", L"float", 
    L"for", L"friend", L"goto", L"if", L"inline", L"int", L"long", L"mutable", L"namespace", 
    L"new", L"noexcept", L"not", L"not_eq", L"nullptr", L"operator", L"or", L"or_eq", 
    L"private", L"protected", L"public", L"reflexpr", L"register", L"reinterpret_cast", 
    L"requires", L"return", L"short", L"signed", L"sizeof", L"static", L"static_assert", 
    L"static_cast", L"struct", L"switch", L"template", L"this", L"thread_local", L"throw", 
    L"true", L"try", L"typedef", L"typeid", L"typename", L"union", L"unsigned", L"using", 
    L"virtual", L"void", L"volatile", L"wchar_t", L"while", L"xor", L"xor_eq", L"override", L"final",
    L"#include", L"#define", L"#ifdef", L"#ifndef", L"#endif", L"#pragma"
};

TextArea::TextArea(Buffer& buffer)
    : m_buffer(buffer)
    , m_scroll({0, 0})
    , m_showLineNumbers(true)
    , m_tabSize(4) {
    m_bounds = {22, 1, 58, 23};
}

bool TextArea::isKeyword(const std::wstring& word) {
    return CPP_KEYWORDS.find(word) != CPP_KEYWORDS.end();
}

std::vector<TextArea::HighlightToken> TextArea::getLineHighlights(const std::wstring& line, WORD defaultAttr, const Color::Scheme& theme) {
    std::vector<HighlightToken> tokens;
    if (line.empty()) return tokens;

    size_t i = 0;
    size_t len = line.length();

    auto addToken = [&](size_t start, size_t count, WORD attr) {
        tokens.push_back({start, count, attr});
    };

    while (i < len) {
        // Skip whitespace
        if (iswspace(line[i])) {
            i++;
            continue;
        }

        // Line comment
        if (i + 1 < len && line[i] == L'/' && line[i+1] == L'/') {
            addToken(i, len - i, theme.comment);
            break;
        }

        // String literal
        if (line[i] == L'"') {
            size_t start = i;
            i++; // Skip opening quote
            while (i < len && line[i] != L'"') {
                if (line[i] == L'\\' && i + 1 < len) {
                    i += 2; // Skip escaped character
                } else {
                    i++;
                }
            }
            if (i < len && line[i] == L'"') {
                i++; // Include closing quote
            }
            addToken(start, i - start, theme.stringLiteral);
            continue;
        }

        // Character literal
        if (line[i] == L'\'') {
            size_t start = i;
            i++;
            while (i < len && line[i] != L'\'') {
                if (line[i] == L'\\' && i + 1 < len) {
                    i += 2;
                } else {
                    i++;
                }
            }
            if (i < len && line[i] == L'\'') {
                i++;
            }
            addToken(start, i - start, theme.stringLiteral);
            continue;
        }

        // Words / Identifiers / Keywords / Preprocessor
        if (iswalnum(line[i]) || line[i] == L'_' || line[i] == L'#') {
            size_t start = i;
            while (i < len && (iswalnum(line[i]) || line[i] == L'_')) {
                i++;
            }
            std::wstring word = line.substr(start, i - start);
            if (isKeyword(word)) {
                addToken(start, word.length(), theme.keyword);
            } else if (iswdigit(word[0])) {
                addToken(start, word.length(), theme.number);
            }
            continue;
        }

        // Operators or punctuation
        i++;
    }

    return tokens;
}

void TextArea::draw(Console& console) {
    const auto& theme = console.getTheme();

    // Fill area background
    console.fillRect(m_bounds, L' ', theme.background);

    // Dynamic line number area width
    int lineCount = static_cast<int>(m_buffer.getLineCount());
    int numWidth = m_showLineNumbers ? static_cast<int>(log10(std::max(1, lineCount))) + 2 : 0;
    
    // Draw vertical divider after line numbers
    if (m_showLineNumbers) {
        for (int y = m_bounds.y; y < m_bounds.y + m_bounds.h; ++y) {
            console.writeChar(m_bounds.x + numWidth, y, L'│', theme.border);
        }
    }

    int textXStart = m_bounds.x + numWidth + (m_showLineNumbers ? 1 : 0);
    int textWidth = m_bounds.w - numWidth - (m_showLineNumbers ? 1 : 0);
    int textHeight = m_bounds.h;

    // Constrain scrolls
    if (m_scroll.y >= lineCount) {
        m_scroll.y = lineCount - 1;
    }
    if (m_scroll.y < 0) m_scroll.y = 0;

    // Load active selection ranges
    bool hasSel = m_buffer.hasSelection();
    Point selStart, selEnd;
    m_buffer.getSelectionRange(selStart, selEnd);

    for (int y = 0; y < textHeight; ++y) {
        int lineIndex = m_scroll.y + y;
        int drawY = m_bounds.y + y;

        if (lineIndex >= lineCount) {
            break; // No more lines
        }

        const auto& line = m_buffer.getLine(lineIndex);

        // Draw Line Number
        if (m_showLineNumbers) {
            std::wstringstream wss;
            wss.width(numWidth);
            wss << (lineIndex + 1);
            console.writeText(m_bounds.x, drawY, wss.str(), theme.lineNumbers);
        }

        // Calculate character colors/syntax highlighting
        std::vector<WORD> charAttrs(line.length(), theme.text);
        auto highlights = getLineHighlights(line, theme.text, theme);
        for (const auto& token : highlights) {
            for (size_t ti = 0; ti < token.length; ++ti) {
                if (token.start + ti < charAttrs.size()) {
                    charAttrs[token.start + ti] = token.attr;
                }
            }
        }

        // Override background colors for text selection
        if (hasSel) {
            for (size_t ci = 0; ci < line.length(); ++ci) {
                Point p = {static_cast<int>(ci), lineIndex};
                bool isSelected = false;
                if (lineIndex > selStart.y && lineIndex < selEnd.y) {
                    isSelected = true;
                } else if (lineIndex == selStart.y && lineIndex == selEnd.y) {
                    isSelected = (p.x >= selStart.x && p.x < selEnd.x);
                } else if (lineIndex == selStart.y) {
                    isSelected = (p.x >= selStart.x);
                } else if (lineIndex == selEnd.y) {
                    isSelected = (p.x < selEnd.x);
                }

                if (isSelected) {
                    // Combine foreground with selected background color
                    charAttrs[ci] = theme.selectedText;
                }
            }
        }

        // Render characters within viewport limits
        int lineScrollX = m_scroll.x;
        for (int x = 0; x < textWidth; ++x) {
            int charIdx = lineScrollX + x;
            int drawX = textXStart + x;

            if (charIdx < static_cast<int>(line.length())) {
                console.writeChar(drawX, drawY, line[charIdx], charAttrs[charIdx]);
            } else {
                // If it's selected past the end of the line (e.g. newline selected)
                bool isPastSel = false;
                if (hasSel) {
                    Point p = {charIdx, lineIndex};
                    if (lineIndex > selStart.y && lineIndex < selEnd.y) {
                        isPastSel = true;
                    } else if (lineIndex == selStart.y && lineIndex < selEnd.y && charIdx >= selStart.x) {
                        isPastSel = true;
                    } else if (lineIndex == selEnd.y && lineIndex > selStart.y && charIdx < selEnd.x) {
                        // Note: don't highlight infinitely, just highlight one character width for newline indicator
                        isPastSel = (charIdx == static_cast<int>(line.length()));
                    }
                }
                
                if (isPastSel && charIdx == static_cast<int>(line.length())) {
                    console.writeChar(drawX, drawY, L' ', theme.selectedBackground);
                } else {
                    console.writeChar(drawX, drawY, L' ', theme.background);
                }
            }
        }
    }
}

void TextArea::scrollToCursor() {
    Point cursor = m_buffer.getCursor();
    int lineCount = static_cast<int>(m_buffer.getLineCount());
    
    // Line number area
    int numWidth = m_showLineNumbers ? static_cast<int>(log10(std::max(1, lineCount))) + 2 : 0;
    int textWidth = m_bounds.w - numWidth - (m_showLineNumbers ? 1 : 0);
    int textHeight = m_bounds.h;

    // Scroll vertical viewport
    if (cursor.y < m_scroll.y) {
        m_scroll.y = cursor.y;
    }
    if (cursor.y >= m_scroll.y + textHeight) {
        m_scroll.y = cursor.y - textHeight + 1;
    }

    // Scroll horizontal viewport
    if (cursor.x < m_scroll.x) {
        m_scroll.x = cursor.x;
    }
    if (cursor.x >= m_scroll.x + textWidth) {
        m_scroll.x = cursor.x - textWidth + 1;
    }
}

void TextArea::handleInput(const UIEvent& ev) {
    if (ev.type == UIEvent::Type::Key) {
        bool select = ev.key.shiftPressed;
        
        switch (ev.key.code) {
            case Key::Left:
                m_buffer.moveCursor(-1, 0, select);
                break;
            case Key::Right:
                m_buffer.moveCursor(1, 0, select);
                break;
            case Key::Up:
                m_buffer.moveCursor(0, -1, select);
                break;
            case Key::Down:
                m_buffer.moveCursor(0, 1, select);
                break;
            case Key::Home:
                m_buffer.setCursor({0, m_buffer.getCursor().y});
                break;
            case Key::End: {
                int lineIdx = m_buffer.getCursor().y;
                int len = static_cast<int>(m_buffer.getLine(lineIdx).length());
                m_buffer.setCursor({len, lineIdx});
                break;
            }
            case Key::PageUp:
                m_buffer.moveCursor(0, -m_bounds.h, select);
                break;
            case Key::PageDown:
                m_buffer.moveCursor(0, m_bounds.h, select);
                break;
            case Key::Backspace:
                m_buffer.backspace();
                break;
            case Key::Delete:
                m_buffer.deleteChar();
                break;
            case Key::Enter:
                m_buffer.insertNewLine();
                break;
            case Key::Tab:
                // Soft tabs (spaces)
                m_buffer.insertText(std::wstring(m_tabSize, L' '));
                break;
            case Key::Char:
                if (!ev.key.controlPressed && !ev.key.altPressed) {
                    m_buffer.insertChar(ev.key.ch);
                }
                break;
            case Key::Ctrl_A:
                selectAll();
                break;
            case Key::Ctrl_Z:
                m_buffer.undo();
                break;
            case Key::Ctrl_Y:
                m_buffer.redo();
                break;
            default:
                break;
        }

        scrollToCursor();
    } 
    else if (ev.type == UIEvent::Type::Mouse) {
        Point m = ev.mouse.pos;
        bool isClick = (ev.mouse.buttonState & FROM_LEFT_1ST_BUTTON_PRESSED) != 0;
        bool isScrollUp = (ev.mouse.eventFlags & MOUSE_WHEELED) && (ev.mouse.buttonState & 0xFF000000); // positive wheel delta
        bool isScrollDown = (ev.mouse.eventFlags & MOUSE_WHEELED) && !(ev.mouse.buttonState & 0xFF000000); // negative wheel delta

        // Mouse scrolls
        if (isScrollUp) {
            m_scroll.y = std::max(0, m_scroll.y - 3);
        } else if (isScrollDown) {
            int limit = static_cast<int>(m_buffer.getLineCount()) - m_bounds.h;
            m_scroll.y = std::min(std::max(0, limit), m_scroll.y + 3);
        }

        if (m_bounds.contains(m)) {
            int lineCount = static_cast<int>(m_buffer.getLineCount());
            int numWidth = m_showLineNumbers ? static_cast<int>(log10(std::max(1, lineCount))) + 2 : 0;
            int textXStart = m_bounds.x + numWidth + (m_showLineNumbers ? 1 : 0);

            if (m.x >= textXStart && isClick) {
                int clickedLine = m_scroll.y + (m.y - m_bounds.y);
                int clickedCol = m_scroll.x + (m.x - textXStart);

                if (clickedLine >= 0 && clickedLine < lineCount) {
                    int lineLen = static_cast<int>(m_buffer.getLine(clickedLine).length());
                    clickedCol = std::min(clickedCol, lineLen);
                    
                    if (ev.key.shiftPressed) {
                        m_buffer.setSelectionEnd({clickedCol, clickedLine});
                    } else {
                        m_buffer.clearSelection();
                        m_buffer.setCursor({clickedCol, clickedLine});
                    }
                }
            }
        }
    }
}

void TextArea::copyToClipboard() {
    if (!m_buffer.hasSelection()) return;
    std::wstring text = m_buffer.getSelectedText();

    if (!OpenClipboard(nullptr)) return;
    EmptyClipboard();

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (text.length() + 1) * sizeof(wchar_t));
    if (hMem) {
        wchar_t* pLocked = static_cast<wchar_t*>(GlobalLock(hMem));
        if (pLocked) {
            wcscpy_s(pLocked, text.length() + 1, text.c_str());
            GlobalUnlock(hMem);
            SetClipboardData(CF_UNICODETEXT, hMem);
        }
    }
    CloseClipboard();
}

void TextArea::cutToClipboard() {
    if (!m_buffer.hasSelection()) return;
    copyToClipboard();
    m_buffer.deleteSelection();
    scrollToCursor();
}

void TextArea::pasteFromClipboard() {
    if (!OpenClipboard(nullptr)) return;

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData) {
        wchar_t* pText = static_cast<wchar_t*>(GlobalLock(hData));
        if (pText) {
            std::wstring text(pText);
            m_buffer.insertText(text);
            GlobalUnlock(hData);
        }
    }
    CloseClipboard();
    scrollToCursor();
}

void TextArea::selectAll() {
    int lineCount = static_cast<int>(m_buffer.getLineCount());
    int lastLineLen = static_cast<int>(m_buffer.getLine(lineCount - 1).length());
    m_buffer.setSelectionStart({0, 0});
    m_buffer.setSelectionEnd({lastLineLen, lineCount - 1});
    m_buffer.setCursor({lastLineLen, lineCount - 1});
}

} // namespace PaperPad
