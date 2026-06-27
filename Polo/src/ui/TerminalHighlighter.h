#pragma once

#include <QSyntaxHighlighter>
#include <QTextCharFormat>

class TerminalHighlighter : public QSyntaxHighlighter {
    Q_OBJECT

public:
    explicit TerminalHighlighter(QTextDocument *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    QTextCharFormat commandFormat_;
    QTextCharFormat promptFormat_;
};
