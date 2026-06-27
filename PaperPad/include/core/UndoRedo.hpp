#pragma once

#include <memory>
#include <vector>
#include "utils/Types.hpp"

namespace PaperPad {

class Buffer;

class Command {
public:
    virtual ~Command() = default;
    virtual void execute(Buffer& buffer) = 0;
    virtual void undo(Buffer& buffer) = 0;
    virtual bool mergeWith(Command* other) { return false; }
};

class InsertCharCommand : public Command {
public:
    InsertCharCommand(Point pos, wchar_t ch);
    void execute(Buffer& buffer) override;
    void undo(Buffer& buffer) override;
    bool mergeWith(Command* other) override;

private:
    Point m_pos;
    std::wstring m_chars;
};

class DeleteCharCommand : public Command {
public:
    DeleteCharCommand(Point pos, wchar_t ch, bool backspace);
    void execute(Buffer& buffer) override;
    void undo(Buffer& buffer) override;

private:
    Point m_pos;
    wchar_t m_ch;
    bool m_backspace;
};

class SplitLineCommand : public Command {
public:
    SplitLineCommand(Point pos);
    void execute(Buffer& buffer) override;
    void undo(Buffer& buffer) override;

private:
    Point m_pos;
};

class MergeLineCommand : public Command {
public:
    MergeLineCommand(int lineIndex, size_t originalLength);
    void execute(Buffer& buffer) override;
    void undo(Buffer& buffer) override;

private:
    int m_lineIndex;
    size_t m_originalLength;
};

class DeleteSelectionCommand : public Command {
public:
    DeleteSelectionCommand(Point start, Point end, const std::vector<std::wstring>& deletedText);
    void execute(Buffer& buffer) override;
    void undo(Buffer& buffer) override;

private:
    Point m_start;
    Point m_end;
    std::vector<std::wstring> m_deletedLines;
};

class InsertTextCommand : public Command {
public:
    InsertTextCommand(Point pos, const std::vector<std::wstring>& lines);
    void execute(Buffer& buffer) override;
    void undo(Buffer& buffer) override;

private:
    Point m_pos;
    std::vector<std::wstring> m_lines;
    Point m_endPos;
};

class CommandHistory {
public:
    CommandHistory() = default;
    ~CommandHistory() = default;

    void executeCommand(Buffer& buffer, std::unique_ptr<Command> cmd);
    void undo(Buffer& buffer);
    void redo(Buffer& buffer);
    void clear();

    bool canUndo() const { return m_undoStack.size() > 0; }
    bool canRedo() const { return m_redoStack.size() > 0; }

private:
    std::vector<std::unique_ptr<Command>> m_undoStack;
    std::vector<std::unique_ptr<Command>> m_redoStack;
};

} // namespace PaperPad
