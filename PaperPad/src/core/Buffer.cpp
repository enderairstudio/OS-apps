#include "core/Buffer.hpp"
#include "io/FileHandler.hpp"
#include "utils/Logger.hpp"
#include <algorithm>
#include <sstream>

namespace PaperPad {

Buffer::Buffer() {
    clear();
}

void Buffer::clear() {
    m_lines.clear();
    m_lines.push_back(L"");
    m_filepath = L"";
    m_dirty = false;
    m_cursor = {0, 0};
    m_selectionStart = {0, 0};
    m_selectionEnd = {0, 0};
    m_hasSelection = false;
    m_history.clear();
}

bool Buffer::load(const std::wstring& filepath) {
    std::vector<std::wstring> newLines;
    if (FileHandler::loadFile(filepath, newLines)) {
        m_lines = std::move(newLines);
        m_filepath = filepath;
        m_dirty = false;
        m_cursor = {0, 0};
        m_selectionStart = {0, 0};
        m_selectionEnd = {0, 0};
        m_hasSelection = false;
        m_history.clear();
        return true;
    }
    return false;
}

bool Buffer::save() {
    if (m_filepath.empty()) {
        return false;
    }
    if (FileHandler::saveFile(m_filepath, m_lines)) {
        m_dirty = false;
        return true;
    }
    return false;
}

bool Buffer::saveAs(const std::wstring& filepath) {
    if (FileHandler::saveFile(filepath, m_lines)) {
        m_filepath = filepath;
        m_dirty = false;
        return true;
    }
    return false;
}

std::wstring Buffer::getFileName() const {
    if (m_filepath.empty()) {
        return L"Untitled";
    }
    size_t lastSlash = m_filepath.find_last_of(L"\\/");
    if (lastSlash == std::wstring::npos) {
        return m_filepath;
    }
    return m_filepath.substr(lastSlash + 1);
}

const std::wstring& Buffer::getLine(int index) const {
    static const std::wstring empty = L"";
    if (index < 0 || index >= static_cast<int>(m_lines.size())) {
        return empty;
    }
    return m_lines[index];
}

void Buffer::setCursor(Point p) {
    m_cursor = p;
    normalizeCursor();
}

void Buffer::normalizeCursor() {
    if (m_lines.empty()) {
        m_lines.push_back(L"");
    }
    if (m_cursor.y < 0) m_cursor.y = 0;
    if (m_cursor.y >= static_cast<int>(m_lines.size())) {
        m_cursor.y = static_cast<int>(m_lines.size()) - 1;
    }
    int lineLen = static_cast<int>(m_lines[m_cursor.y].length());
    if (m_cursor.x < 0) m_cursor.x = 0;
    if (m_cursor.x > lineLen) m_cursor.x = lineLen;
}

void Buffer::moveCursor(int dx, int dy, bool select) {
    Point oldCursor = m_cursor;
    
    if (dx != 0) {
        m_cursor.x += dx;
        // Wrap horizontally
        if (m_cursor.x < 0) {
            if (m_cursor.y > 0) {
                m_cursor.y--;
                m_cursor.x = static_cast<int>(m_lines[m_cursor.y].length());
            } else {
                m_cursor.x = 0;
            }
        } else if (m_cursor.x > static_cast<int>(m_lines[m_cursor.y].length())) {
            if (m_cursor.y < static_cast<int>(m_lines.size()) - 1) {
                m_cursor.y++;
                m_cursor.x = 0;
            } else {
                m_cursor.x = static_cast<int>(m_lines[m_cursor.y].length());
            }
        }
    }

    if (dy != 0) {
        m_cursor.y += dy;
        if (m_cursor.y < 0) {
            m_cursor.y = 0;
            m_cursor.x = 0;
        } else if (m_cursor.y >= static_cast<int>(m_lines.size())) {
            m_cursor.y = static_cast<int>(m_lines.size()) - 1;
            m_cursor.x = static_cast<int>(m_lines[m_cursor.y].length());
        } else {
            // Keep X bounded
            m_cursor.x = std::min(m_cursor.x, static_cast<int>(m_lines[m_cursor.y].length()));
        }
    }

    if (select) {
        if (!m_hasSelection) {
            m_selectionStart = oldCursor;
            m_hasSelection = true;
        }
        m_selectionEnd = m_cursor;
    } else {
        clearSelection();
    }
}

void Buffer::clearSelection() {
    m_hasSelection = false;
    m_selectionStart = {0, 0};
    m_selectionEnd = {0, 0};
}

void Buffer::setSelectionStart(Point p) {
    m_selectionStart = p;
    m_hasSelection = true;
}

void Buffer::setSelectionEnd(Point p) {
    m_selectionEnd = p;
    m_hasSelection = true;
}

void Buffer::getSelectionRange(Point& start, Point& end) const {
    if (!m_hasSelection) {
        start = m_cursor;
        end = m_cursor;
        return;
    }
    
    // Determine which point comes first
    if (m_selectionStart.y < m_selectionEnd.y) {
        start = m_selectionStart;
        end = m_selectionEnd;
    } else if (m_selectionStart.y > m_selectionEnd.y) {
        start = m_selectionEnd;
        end = m_selectionStart;
    } else {
        // Same line
        if (m_selectionStart.x <= m_selectionEnd.x) {
            start = m_selectionStart;
            end = m_selectionEnd;
        } else {
            start = m_selectionEnd;
            end = m_selectionStart;
        }
    }
}

std::wstring Buffer::getSelectedText() const {
    if (!m_hasSelection) return L"";
    Point start, end;
    getSelectionRange(start, end);

    std::wstring result;
    if (start.y == end.y) {
        return m_lines[start.y].substr(start.x, end.x - start.x);
    }

    result += m_lines[start.y].substr(start.x) + L"\n";
    for (int y = start.y + 1; y < end.y; ++y) {
        result += m_lines[y] + L"\n";
    }
    result += m_lines[end.y].substr(0, end.x);
    return result;
}

std::vector<std::wstring> Buffer::getSelectedLines() const {
    std::vector<std::wstring> result;
    if (!m_hasSelection) return result;
    Point start, end;
    getSelectionRange(start, end);

    if (start.y == end.y) {
        result.push_back(m_lines[start.y].substr(start.x, end.x - start.x));
        return result;
    }

    result.push_back(m_lines[start.y].substr(start.x));
    for (int y = start.y + 1; y < end.y; ++y) {
        result.push_back(m_lines[y]);
    }
    result.push_back(m_lines[end.y].substr(0, end.x));
    return result;
}

void Buffer::insertChar(wchar_t ch) {
    if (m_hasSelection) {
        deleteSelection();
    }
    m_history.executeCommand(*this, std::make_unique<InsertCharCommand>(m_cursor, ch));
    m_dirty = true;
}

void Buffer::deleteChar() {
    if (m_hasSelection) {
        deleteSelection();
        return;
    }

    if (m_cursor.x < static_cast<int>(m_lines[m_cursor.y].length())) {
        wchar_t ch = m_lines[m_cursor.y][m_cursor.x];
        m_history.executeCommand(*this, std::make_unique<DeleteCharCommand>(m_cursor, ch, false));
        m_dirty = true;
    } else if (m_cursor.y < static_cast<int>(m_lines.size()) - 1) {
        // Merge with line below
        m_history.executeCommand(*this, std::make_unique<MergeLineCommand>(m_cursor.y, m_lines[m_cursor.y].length()));
        m_dirty = true;
    }
}

void Buffer::backspace() {
    if (m_hasSelection) {
        deleteSelection();
        return;
    }

    if (m_cursor.x > 0) {
        wchar_t ch = m_lines[m_cursor.y][m_cursor.x - 1];
        m_history.executeCommand(*this, std::make_unique<DeleteCharCommand>(m_cursor, ch, true));
        m_dirty = true;
    } else if (m_cursor.y > 0) {
        // Merge with line above
        int prevLineLen = static_cast<int>(m_lines[m_cursor.y - 1].length());
        m_history.executeCommand(*this, std::make_unique<MergeLineCommand>(m_cursor.y - 1, prevLineLen));
        m_dirty = true;
    }
}

void Buffer::insertNewLine() {
    if (m_hasSelection) {
        deleteSelection();
    }
    m_history.executeCommand(*this, std::make_unique<SplitLineCommand>(m_cursor));
    m_dirty = true;
}

void Buffer::deleteSelection() {
    if (!m_hasSelection) return;
    Point start, end;
    getSelectionRange(start, end);
    auto deletedText = getSelectedLines();
    m_history.executeCommand(*this, std::make_unique<DeleteSelectionCommand>(start, end, deletedText));
    clearSelection();
    m_dirty = true;
}

void Buffer::insertText(const std::wstring& text) {
    if (text.empty()) return;
    if (m_hasSelection) {
        deleteSelection();
    }
    
    // Parse text into lines
    std::vector<std::wstring> lines;
    std::wstring line;
    std::wstringstream wss(text);
    while (std::getline(wss, line)) {
        if (!line.empty() && line.back() == L'\r') {
            line.pop_back();
        }
        lines.push_back(line);
    }
    if (lines.empty()) lines.push_back(L"");

    m_history.executeCommand(*this, std::make_unique<InsertTextCommand>(m_cursor, lines));
    m_dirty = true;
}

// Helper called during Undo to reinsert lines
void Buffer::insertTextLines(const std::vector<std::wstring>& lines) {
    if (lines.empty()) return;
    if (lines.size() == 1) {
        m_lines[m_cursor.y].insert(m_cursor.x, lines[0]);
    } else {
        std::wstring remainder = m_lines[m_cursor.y].substr(m_cursor.x);
        m_lines[m_cursor.y] = m_lines[m_cursor.y].substr(0, m_cursor.x) + lines[0];
        for (size_t i = 1; i < lines.size() - 1; ++i) {
            m_lines.insert(m_lines.begin() + m_cursor.y + i, lines[i]);
        }
        m_lines.insert(m_lines.begin() + m_cursor.y + lines.size() - 1, lines.back() + remainder);
    }
}

void Buffer::undo() {
    m_history.undo(*this);
    m_dirty = true;
}

void Buffer::redo() {
    m_history.redo(*this);
    m_dirty = true;
}

// ==========================================
// Raw manipulations
// ==========================================

void Buffer::insertCharRaw(int row, int col, wchar_t ch) {
    if (row >= 0 && row < static_cast<int>(m_lines.size())) {
        if (col >= 0 && col <= static_cast<int>(m_lines[row].length())) {
            m_lines[row].insert(m_lines[row].begin() + col, ch);
        }
    }
}

wchar_t Buffer::deleteCharRaw(int row, int col) {
    wchar_t removed = L'\0';
    if (row >= 0 && row < static_cast<int>(m_lines.size())) {
        if (col >= 0 && col < static_cast<int>(m_lines[row].length())) {
            removed = m_lines[row][col];
            m_lines[row].erase(m_lines[row].begin() + col);
        }
    }
    return removed;
}

void Buffer::insertLineRaw(int row, const std::wstring& content) {
    if (row >= 0 && row <= static_cast<int>(m_lines.size())) {
        m_lines.insert(m_lines.begin() + row, content);
    }
}

std::wstring Buffer::deleteLineRaw(int row) {
    std::wstring content;
    if (row >= 0 && row < static_cast<int>(m_lines.size())) {
        content = m_lines[row];
        m_lines.erase(m_lines.begin() + row);
    }
    if (m_lines.empty()) {
        m_lines.push_back(L"");
    }
    return content;
}

void Buffer::splitLineRaw(int row, int col) {
    if (row >= 0 && row < static_cast<int>(m_lines.size())) {
        if (col >= 0 && col <= static_cast<int>(m_lines[row].length())) {
            std::wstring right = m_lines[row].substr(col);
            m_lines[row] = m_lines[row].substr(0, col);
            m_lines.insert(m_lines.begin() + row + 1, right);
        }
    }
}

void Buffer::mergeLineRaw(int row) {
    if (row >= 0 && row < static_cast<int>(m_lines.size()) - 1) {
        m_lines[row] += m_lines[row + 1];
        m_lines.erase(m_lines.begin() + row + 1);
    }
}

} // namespace PaperPad
