#include "TerminalView.h"

#include <QColor>
#include <QGuiApplication>
#include <QKeyEvent>
#include <QResizeEvent>
#include <QScrollBar>
#include <QClipboard>
#include <QTextBlock>
#include <QTextCharFormat>

TerminalView::TerminalView(QWidget *parent)
    : QPlainTextEdit(parent)
{
    setObjectName("terminalView");
    setFrameShape(QFrame::NoFrame);
    setUndoRedoEnabled(false);
    setWordWrapMode(QTextOption::NoWrap);
    setTabChangesFocus(false);
    setAcceptDrops(false);
    highlighter_ = new TerminalHighlighter(document());
}

void TerminalView::attachTab(TerminalTab *tab)
{
    if (outputConnection_) {
        disconnect(outputConnection_);
    }
    if (changedConnection_) {
        disconnect(changedConnection_);
    }

    tab_ = tab;
    lineBuffer_.clear();
    clear();
    if (tab_) {
        outputConnection_ = connect(tab_, &TerminalTab::outputReady, this, &TerminalView::appendOutput);
        changedConnection_ = connect(tab_, &TerminalTab::changed, this, &TerminalView::tabStateChanged);
        appendOutput(tab_->scrollback());
        updatePtySize();
    }
}

void TerminalView::resetPrompt()
{
    moveCursor(QTextCursor::End);
}

void TerminalView::appendOutput(const QString &text)
{
    if (text.isEmpty()) {
        return;
    }
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    insertAnsiText(cursor, text);
    setTextCursor(cursor);
    verticalScrollBar()->setValue(verticalScrollBar()->maximum());
}

void TerminalView::clearTerminal()
{
    clear();
    if (tab_) {
        tab_->clearScrollback();
    }
}

void TerminalView::keyPressEvent(QKeyEvent *event)
{
    if (!tab_) {
        return;
    }

    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
        if (handleLocalCommand()) {
            return;
        }
        lineBuffer_.clear();
        tab_->pty()->writeInput("\r");
        return;
    }

    sendKeyToPty(event);
}

void TerminalView::mousePressEvent(QMouseEvent *event)
{
    QPlainTextEdit::mousePressEvent(event);
    moveCursor(QTextCursor::End);
}

void TerminalView::resizeEvent(QResizeEvent *event)
{
    QPlainTextEdit::resizeEvent(event);
    updatePtySize();
}

void TerminalView::sendKeyToPty(QKeyEvent *event)
{
    if (!tab_) {
        return;
    }

    if (event->modifiers().testFlag(Qt::ControlModifier)) {
        if (event->key() == Qt::Key_C) {
            if (textCursor().hasSelection()) {
                copy();
                return;
            }
            tab_->pty()->writeInput(QByteArray(1, char(0x03)));
            lineBuffer_.clear();
            return;
        }
        if (event->key() == Qt::Key_V) {
            const QString text = QGuiApplication::clipboard()->text();
            if (!text.isEmpty()) {
                lineBuffer_.append(text);
                tab_->pty()->writeInput(text.toUtf8());
            }
            return;
        }
        if (event->key() == Qt::Key_D) {
            tab_->pty()->writeInput(QByteArray(1, char(0x04)));
            return;
        }
        if (event->key() == Qt::Key_L) {
            clearTerminal();
            return;
        }
    }

    QByteArray input;
    switch (event->key()) {
    case Qt::Key_Backspace:
        input = QByteArray(1, char(0x7f));
        if (!lineBuffer_.isEmpty()) {
            lineBuffer_.chop(1);
        }
        break;
    case Qt::Key_Left:
        input = "\x1b[D";
        break;
    case Qt::Key_Right:
        input = "\x1b[C";
        break;
    case Qt::Key_Up:
        input = "\x1b[A";
        break;
    case Qt::Key_Down:
        input = "\x1b[B";
        break;
    case Qt::Key_Home:
        input = "\x1b[H";
        break;
    case Qt::Key_End:
        input = "\x1b[F";
        break;
    case Qt::Key_Tab:
        input = "\t";
        lineBuffer_.append('\t');
        break;
    case Qt::Key_Escape:
        input = QByteArray(1, char(0x1b));
        break;
    default:
        if (!event->text().isEmpty()) {
            input = event->text().toUtf8();
            lineBuffer_.append(event->text());
        }
        break;
    }

    if (!input.isEmpty()) {
        tab_->pty()->writeInput(input);
    }
}

bool TerminalView::handleLocalCommand()
{
    const QString command = lineBuffer_.trimmed();
    if (command == "clip3d" || command == "3d") {
        tab_->pty()->writeInput(QByteArray(1, char(0x15)));
        clearTerminal();
        lineBuffer_.clear();
        emit show3DRequested();
        emit tabStateChanged();
        return true;
    }

    if (command == "clear" || command == "cls") {
        clearTerminal();
        tab_->pty()->writeInput("\r");
        lineBuffer_.clear();
        emit tabStateChanged();
        return true;
    }

    if (command == "exit") {
        tab_->pty()->writeInput("\r");
        lineBuffer_.clear();
        emit closeTabRequested();
        return true;
    }

    return false;
}

void TerminalView::appendLocalLine(const QString &text)
{
    QTextCursor cursor = textCursor();
    cursor.movePosition(QTextCursor::End);
    cursor.insertText(text);
    setTextCursor(cursor);
}

void TerminalView::updatePtySize()
{
    if (!tab_) {
        return;
    }
    const QFontMetrics metrics(font());
    const int columns = qMax(20, viewport()->width() / qMax(1, metrics.horizontalAdvance('M')));
    const int rows = qMax(5, viewport()->height() / qMax(1, metrics.height()));
    tab_->pty()->resizeTerminal(columns, rows);
}


QTextCharFormat TerminalView::ansiFormat(const QList<int> &codes, const QTextCharFormat &current) const
{
    QTextCharFormat format = current;
    if (codes.isEmpty()) {
        format.setForeground(QColor(244, 244, 244));
        format.setFontWeight(QFont::Normal);
        return format;
    }

    const QList<QColor> normal = {
        QColor(0, 0, 0), QColor(230, 72, 72), QColor(80, 220, 120), QColor(255, 222, 70),
        QColor(90, 155, 255), QColor(220, 90, 255), QColor(80, 230, 230), QColor(244, 244, 244)
    };
    const QList<QColor> bright = {
        QColor(90, 90, 90), QColor(255, 100, 100), QColor(120, 255, 160), QColor(255, 235, 120),
        QColor(130, 185, 255), QColor(238, 135, 255), QColor(120, 255, 255), QColor(255, 255, 255)
    };

    for (int i = 0; i < codes.size(); ++i) {
        const int code = codes.at(i);
        if (code == 0) {
            format.setForeground(QColor(244, 244, 244));
            format.setFontWeight(QFont::Normal);
        } else if (code == 1) {
            format.setFontWeight(QFont::Bold);
        } else if (code == 22) {
            format.setFontWeight(QFont::Normal);
        } else if (code == 39) {
            format.setForeground(QColor(244, 244, 244));
        } else if (code >= 30 && code <= 37) {
            format.setForeground(normal.at(code - 30));
        } else if (code >= 90 && code <= 97) {
            format.setForeground(bright.at(code - 90));
        } else if ((code == 38 || code == 48) && i + 2 < codes.size() && codes.at(i + 1) == 5) {
            const int color = codes.at(i + 2);
            const QColor mapped = bright.at(qBound(0, color % 8, 7));
            if (code == 38) {
                format.setForeground(mapped);
            }
            i += 2;
        } else if ((code == 38 || code == 48) && i + 4 < codes.size() && codes.at(i + 1) == 2) {
            const QColor rgb(qBound(0, codes.at(i + 2), 255),
                             qBound(0, codes.at(i + 3), 255),
                             qBound(0, codes.at(i + 4), 255));
            if (code == 38) {
                format.setForeground(rgb);
            }
            i += 4;
        }
    }

    return format;
}

void TerminalView::insertAnsiText(QTextCursor &cursor, const QString &text)
{
    QTextCharFormat format;
    format.setForeground(QColor(244, 244, 244));

    QString chunk;
    for (int i = 0; i < text.size(); ++i) {
        if (text.at(i) == QChar(0x1b) && i + 1 < text.size() && text.at(i + 1) == '[') {
            if (!chunk.isEmpty()) {
                cursor.insertText(chunk, format);
                chunk.clear();
            }

            int end = i + 2;
            while (end < text.size() && !QStringLiteral("mJKH").contains(text.at(end))) {
                ++end;
            }

            if (end < text.size()) {
                const QChar command = text.at(end);
                const QStringList parts = text.mid(i + 2, end - i - 2)
                                              .remove('?')
                                              .split(';', Qt::SkipEmptyParts);
                QList<int> codes;
                for (const QString &part : parts) {
                    bool ok = false;
                    const int code = part.toInt(&ok);
                    if (ok) {
                        codes << code;
                    }
                }
                if (command == 'm') {
                    format = ansiFormat(codes, format);
                } else if (command == 'J' && (codes.isEmpty() || codes.contains(2) || codes.contains(3))) {
                    clearTerminal();
                }
                i = end;
                continue;
            }
        }

        if (text.at(i) == '\r') {
            continue;
        }

        if (text.at(i) == '\b') {
            cursor.deletePreviousChar();
            continue;
        }

        chunk.append(text.at(i));
    }

    if (!chunk.isEmpty()) {
        cursor.insertText(chunk, format);
    }
}
