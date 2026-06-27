#include "TerminalHighlighter.h"

#include <QColor>

TerminalHighlighter::TerminalHighlighter(QTextDocument *parent)
    : QSyntaxHighlighter(parent)
{
    commandFormat_.setForeground(QColor(255, 222, 70));
    commandFormat_.setFontWeight(QFont::DemiBold);

    promptFormat_.setForeground(QColor(170, 170, 170));
}

void TerminalHighlighter::highlightBlock(const QString &text)
{
    if (!text.startsWith(">")) {
        return;
    }

    const int promptLength = text.size() > 1 && text.at(1) == ' ' ? 2 : 1;
    setFormat(0, promptLength, promptFormat_);

    int start = promptLength;
    while (start < text.size() && text.at(start).isSpace()) {
        ++start;
    }

    int end = start;
    while (end < text.size() && !text.at(end).isSpace()) {
        ++end;
    }

    if (end > start) {
        setFormat(start, end - start, commandFormat_);
    }
}
