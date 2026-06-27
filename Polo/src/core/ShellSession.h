#pragma once

#include "CommandParser.h"

#include <QElapsedTimer>
#include <QObject>
#include <QProcess>
#include <QString>

enum class CommandAction {
    None,
    Clear,
    Show3D,
    CloseTab
};

struct CommandResult {
    QString output;
    CommandAction action = CommandAction::None;
};

class ShellSession : public QObject {
    Q_OBJECT

public:
    explicit ShellSession(QObject *parent = nullptr);

    QString currentDirectory() const;
    QString displayDirectory() const;
    QString runningCommand() const;
    bool isRunning() const;

    CommandResult run(const QString &line);

signals:
    void outputReady(const QString &text);
    void commandStarted(const QString &command);
    void commandFinished();
    void directoryChanged(const QString &path);

private:
    CommandResult runBuiltin(const ParsedCommand &command);
    CommandResult runExternal(const ParsedCommand &command);

    QString currentDirectory_;
    QString runningCommand_;
};
