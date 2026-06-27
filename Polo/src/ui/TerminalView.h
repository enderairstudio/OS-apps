#pragma once

#include "ui/TerminalHighlighter.h"
#include "ui/TerminalTab.h"

#include <QPlainTextEdit>
#include <QTextCharFormat>

class TerminalView : public QPlainTextEdit {
    Q_OBJECT

public:
    explicit TerminalView(QWidget *parent = nullptr);

    void attachTab(TerminalTab *tab);
    void resetPrompt();
    void appendOutput(const QString &text);
    void clearTerminal();

signals:
    void show3DRequested();
    void closeTabRequested();
    void tabStateChanged();

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void sendKeyToPty(QKeyEvent *event);
    bool handleLocalCommand();
    void appendLocalLine(const QString &text);
    void updatePtySize();
    QTextCharFormat ansiFormat(const QList<int> &codes, const QTextCharFormat &current) const;
    void insertAnsiText(QTextCursor &cursor, const QString &text);

    TerminalTab *tab_ = nullptr;
    TerminalHighlighter *highlighter_ = nullptr;
    QMetaObject::Connection outputConnection_;
    QMetaObject::Connection changedConnection_;
    QString lineBuffer_;
};
