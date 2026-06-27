#include "TerminalTab.h"

TerminalTab::TerminalTab(QObject *parent)
    : QObject(parent),
      pty_(this)
{
    connect(&pty_, &PtySession::outputReady, this, [this](const QString &text) {
        appendScrollback(text);
        emit outputReady(text);
    });
    connect(&pty_, &PtySession::directoryChanged, this, &TerminalTab::changed);
    connect(&pty_, &PtySession::finished, this, &TerminalTab::changed);
    pty_.start(PathTools::initialDirectory());
}

PtySession *TerminalTab::pty()
{
    return &pty_;
}

QString TerminalTab::title() const
{
    return displayDirectory();
}

QString TerminalTab::displayDirectory() const
{
    return PathTools::cleanDisplayPath(pty_.currentDirectory());
}

QString TerminalTab::scrollback() const
{
    return scrollback_;
}

void TerminalTab::appendScrollback(const QString &text)
{
    scrollback_.append(text);
    constexpr int maxChars = 200000;
    if (scrollback_.size() > maxChars) {
        scrollback_.remove(0, scrollback_.size() - maxChars);
    }
}

void TerminalTab::clearScrollback()
{
    scrollback_.clear();
}
