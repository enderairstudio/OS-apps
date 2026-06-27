#include "core/UndoRedo.hpp"
#include "core/Buffer.hpp"
#include "utils/Logger.hpp"

namespace PaperPad {

// ==========================================
// InsertCharCommand
// ==========================================

InsertCharCommand::InsertCharCommand(Point pos, wchar_t ch)
    : m_pos(pos), m_chars(1, ch) {}

void InsertCharCommand::execute(Buffer& buffer) {
    Point curr = m_pos;
    for (wchar_t ch : m_chars) {
        buffer.insertCharRaw(curr.y, curr.x, ch);
        curr.x++;
    }
    buffer.setCursor(curr);
}

void InsertCharCommand::undo(Buffer& buffer) {
    for (size_t i = 0; i < m_chars.size(); ++i) {
        buffer.deleteCharRaw(m_pos.y, m_pos.x);
    }
    buffer.setCursor(m_pos);
}

bool InsertCharCommand::mergeWith(Command* other) {
    InsertCharCommand* o = dynamic_cast<InsertCharCommand*>(other);
    if (!o) return false;

    // Check if the other character is typed immediately after this one
    if (o->m_pos.y == m_pos.y && o->m_pos.x == m_pos.x + static_cast<int>(m_chars.size())) {
        m_chars += o->m_chars;
        return true;
    }
    return false;
}

// ==========================================
// DeleteCharCommand
// ==========================================

DeleteCharCommand::DeleteCharCommand(Point pos, wchar_t ch, bool backspace)
    : m_pos(pos), m_ch(ch), m_backspace(backspace) {}

void DeleteCharCommand::execute(Buffer& buffer) {
    if (m_backspace) {
        buffer.deleteCharRaw(m_pos.y, m_pos.x - 1);
        buffer.setCursor({m_pos.x - 1, m_pos.y});
    } else {
        buffer.deleteCharRaw(m_pos.y, m_pos.x);
        buffer.setCursor(m_pos);
    }
}

void DeleteCharCommand::undo(Buffer& buffer) {
    if (m_backspace) {
        buffer.insertCharRaw(m_pos.y, m_pos.x - 1, m_ch);
        buffer.setCursor(m_pos);
    } else {
        buffer.insertCharRaw(m_pos.y, m_pos.x, m_ch);
        buffer.setCursor(m_pos);
    }
}

// ==========================================
// SplitLineCommand
// ==========================================

SplitLineCommand::SplitLineCommand(Point pos) : m_pos(pos) {}

void SplitLineCommand::execute(Buffer& buffer) {
    buffer.splitLineRaw(m_pos.y, m_pos.x);
    buffer.setCursor({0, m_pos.y + 1});
}

void SplitLineCommand::undo(Buffer& buffer) {
    buffer.mergeLineRaw(m_pos.y);
    buffer.setCursor(m_pos);
}

// ==========================================
// MergeLineCommand
// ==========================================

MergeLineCommand::MergeLineCommand(int lineIndex, size_t originalLength)
    : m_lineIndex(lineIndex), m_originalLength(originalLength) {}

void MergeLineCommand::execute(Buffer& buffer) {
    buffer.mergeLineRaw(m_lineIndex);
    buffer.setCursor({static_cast<int>(m_originalLength), m_lineIndex});
}

void MergeLineCommand::undo(Buffer& buffer) {
    buffer.splitLineRaw(m_lineIndex, static_cast<int>(m_originalLength));
    buffer.setCursor({0, m_lineIndex + 1});
}

// ==========================================
// DeleteSelectionCommand
// ==========================================

DeleteSelectionCommand::DeleteSelectionCommand(Point start, Point end, const std::vector<std::wstring>& deletedText)
    : m_start(start), m_end(end), m_deletedLines(deletedText) {}

void DeleteSelectionCommand::execute(Buffer& buffer) {
    // Delete text from start to end (exclusive of end index/coords as bounds)
    Point current = m_end;
    while (current != m_start) {
        if (current.x > 0) {
            buffer.deleteCharRaw(current.y, current.x - 1);
            current.x--;
        } else if (current.y > 0) {
            // Merge with line above
            int prevLineLen = static_cast<int>(buffer.getLine(current.y - 1).length());
            buffer.mergeLineRaw(current.y - 1);
            current.y--;
            current.x = prevLineLen;
        }
    }
    buffer.setCursor(m_start);
}

void DeleteSelectionCommand::undo(Buffer& buffer) {
    buffer.insertTextLines(m_deletedLines);
    buffer.setCursor(m_end);
}

// ==========================================
// InsertTextCommand
// ==========================================

InsertTextCommand::InsertTextCommand(Point pos, const std::vector<std::wstring>& lines)
    : m_pos(pos), m_lines(lines) {}

void InsertTextCommand::execute(Buffer& buffer) {
    buffer.setCursor(m_pos);
    buffer.insertTextLines(m_lines);
    // End position calculation
    int endY = m_pos.y + static_cast<int>(m_lines.size()) - 1;
    int endX = (m_lines.size() == 1) 
        ? m_pos.x + static_cast<int>(m_lines[0].length())
        : static_cast<int>(m_lines.back().length());
    m_endPos = {endX, endY};
    buffer.setCursor(m_endPos);
}

void InsertTextCommand::undo(Buffer& buffer) {
    Point current = m_endPos;
    while (current != m_pos) {
        if (current.x > 0) {
            buffer.deleteCharRaw(current.y, current.x - 1);
            current.x--;
        } else if (current.y > 0) {
            int prevLineLen = static_cast<int>(buffer.getLine(current.y - 1).length());
            buffer.mergeLineRaw(current.y - 1);
            current.y--;
            current.x = prevLineLen;
        }
    }
    buffer.setCursor(m_pos);
}

// ==========================================
// CommandHistory
// ==========================================

void CommandHistory::executeCommand(Buffer& buffer, std::unique_ptr<Command> cmd) {
    m_redoStack.clear();

    if (!m_undoStack.empty()) {
        if (m_undoStack.back()->mergeWith(cmd.get())) {
            // Merged successfully, execute the merged portion
            cmd->execute(buffer);
            return;
        }
    }

    cmd->execute(buffer);
    m_undoStack.push_back(std::move(cmd));
}

void CommandHistory::undo(Buffer& buffer) {
    if (m_undoStack.empty()) return;

    auto cmd = std::move(m_undoStack.back());
    m_undoStack.pop_back();

    cmd->undo(buffer);
    m_redoStack.push_back(std::move(cmd));
}

void CommandHistory::redo(Buffer& buffer) {
    if (m_redoStack.empty()) return;

    auto cmd = std::move(m_redoStack.back());
    m_redoStack.pop_back();

    cmd->execute(buffer);
    m_undoStack.push_back(std::move(cmd));
}

void CommandHistory::clear() {
    m_undoStack.clear();
    m_redoStack.clear();
}

} // namespace PaperPad
