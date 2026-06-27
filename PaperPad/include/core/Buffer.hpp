#pragma once

#include <string>
#include <vector>
#include "utils/Types.hpp"
#include "core/UndoRedo.hpp"

namespace PaperPad {

class Buffer {
public:
    Buffer();
    ~Buffer() = default;

    // File info
    bool load(const std::wstring& filepath);
    bool save();
    bool saveAs(const std::wstring& filepath);
    void clear();

    const std::wstring& getFilePath() const { return m_filepath; }
    std::wstring getFileName() const;
    bool isDirty() const { return m_dirty; }
    void setDirty(bool dirty) { m_dirty = dirty; }

    // Line access
    size_t getLineCount() const { return m_lines.size(); }
    const std::wstring& getLine(int index) const;
    const std::vector<std::wstring>& getLines() const { return m_lines; }

    // Cursor management
    Point getCursor() const { return m_cursor; }
    void setCursor(Point p);
    void moveCursor(int dx, int dy, bool select = false);

    // Selection
    bool hasSelection() const { return m_hasSelection; }
    void clearSelection();
    void setSelectionStart(Point p);
    void setSelectionEnd(Point p);
    void getSelectionRange(Point& start, Point& end) const;
    std::wstring getSelectedText() const;
    std::vector<std::wstring> getSelectedLines() const;

    // Editing APIs (These record history)
    void insertChar(wchar_t ch);
    void deleteChar();
    void backspace();
    void insertNewLine();
    void deleteSelection();
    void insertText(const std::wstring& text);
    void insertTextLines(const std::vector<std::wstring>& lines);

    // Undo / Redo
    void undo();
    void redo();
    bool canUndo() const { return m_history.canUndo(); }
    bool canRedo() const { return m_history.canRedo(); }

    // Raw manipulation (called by Commands, does NOT record history directly)
    void insertCharRaw(int row, int col, wchar_t ch);
    wchar_t deleteCharRaw(int row, int col);
    void insertLineRaw(int row, const std::wstring& content);
    std::wstring deleteLineRaw(int row);
    void splitLineRaw(int row, int col);
    void mergeLineRaw(int row);

private:
    std::vector<std::wstring> m_lines;
    std::wstring m_filepath;
    bool m_dirty;

    Point m_cursor;
    Point m_selectionStart;
    Point m_selectionEnd;
    bool m_hasSelection;

    CommandHistory m_history;

    void normalizeCursor();
};

} // namespace PaperPad
