#include "ShellSession.h"

#include "PathTools.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QTextStream>

ShellSession::ShellSession(QObject *parent)
    : QObject(parent),
      currentDirectory_(PathTools::initialDirectory())
{
}

QString ShellSession::currentDirectory() const
{
    return currentDirectory_;
}

QString ShellSession::displayDirectory() const
{
    return PathTools::cleanDisplayPath(currentDirectory_);
}

QString ShellSession::runningCommand() const
{
    return runningCommand_;
}

bool ShellSession::isRunning() const
{
    return !runningCommand_.isEmpty();
}

CommandResult ShellSession::run(const QString &line)
{
    const ParsedCommand command = CommandParser::parse(line);
    if (command.empty) {
        return {};
    }

    runningCommand_ = command.original;
    emit commandStarted(runningCommand_);

    CommandResult result = runBuiltin(command);
    if (result.action == CommandAction::None && result.output.isNull()) {
        result = runExternal(command);
    }

    runningCommand_.clear();
    emit commandFinished();
    return result;
}

CommandResult ShellSession::runBuiltin(const ParsedCommand &command)
{
    const QString program = command.program.toLower();

    if (program == "say") {
        return {command.args.join(' ') + "\n"};
    }

    if (program == "echo") {
        return {command.args.join(' ') + "\n"};
    }

    if (program == "cd") {
        const QString target = command.args.isEmpty() ? QStringLiteral("~") : command.args.join(' ');
        const QString resolved = PathTools::resolvePath(currentDirectory_, target);
        QDir next(resolved);
        if (!next.exists()) {
            return {QString("\x1b[31mcd: no such directory: %1\x1b[0m\n").arg(target)};
        }
        currentDirectory_ = next.absolutePath();
        emit directoryChanged(currentDirectory_);
        return {QString("")};
    }

    if (program == "pwd") {
        return {QDir::toNativeSeparators(currentDirectory_) + "\n"};
    }

    if (program == "ls") {
        const QString target = command.args.isEmpty() ? QStringLiteral(".") : command.args.join(' ');
        QDir dir(PathTools::resolvePath(currentDirectory_, target));
        if (!dir.exists()) {
            return {QString("\x1b[31mls: no such directory: %1\x1b[0m\n").arg(target)};
        }

        const QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot,
                                                        QDir::DirsFirst | QDir::Name);
        QStringList names;
        for (const QFileInfo &entry : entries) {
            QString name = entry.fileName();
            if (entry.isDir()) {
                name.append(QDir::separator());
            }
            names << name;
        }
        return {names.join("  ") + (names.isEmpty() ? QString() : QString("\n"))};
    }

    if (program == "mkdir") {
        if (command.args.isEmpty()) {
            return {"\x1b[31mmkdir: missing directory name\x1b[0m\n"};
        }
        QString output;
        for (const QString &arg : command.args) {
            QDir dir;
            const QString path = PathTools::resolvePath(currentDirectory_, arg);
            if (!dir.mkpath(path)) {
                output += QString("\x1b[31mmkdir: could not create %1\x1b[0m\n").arg(arg);
            }
        }
        return {output};
    }

    if (program == "touch") {
        if (command.args.isEmpty()) {
            return {"\x1b[31mtouch: missing file name\x1b[0m\n"};
        }
        QString output;
        for (const QString &arg : command.args) {
            QFile file(PathTools::resolvePath(currentDirectory_, arg));
            if (!file.open(QIODevice::Append)) {
                output += QString("\x1b[31mtouch: could not write %1\x1b[0m\n").arg(arg);
                continue;
            }
            file.close();
        }
        return {output};
    }

    if (program == "cat" || program == "head" || program == "tail" || program == "wc") {
        if (command.args.isEmpty()) {
            return {QString("\x1b[31m%1: missing file name\x1b[0m\n").arg(program)};
        }

        const QString name = command.args.first();
        QFile file(PathTools::resolvePath(currentDirectory_, name));
        if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
            return {QString("\x1b[31m%1: could not read %2\x1b[0m\n").arg(program, name)};
        }

        QTextStream stream(&file);
        const QString text = stream.readAll();
        if (program == "cat") {
            return {text + (text.endsWith('\n') || text.isEmpty() ? QString() : QString("\n"))};
        }

        const QStringList lines = text.split('\n');
        if (program == "head") {
            return {lines.mid(0, 10).join('\n') + "\n"};
        }
        if (program == "tail") {
            return {lines.mid(qMax(0, lines.size() - 10)).join('\n') + "\n"};
        }

        const int lineCount = text.isEmpty() ? 0 : text.count('\n') + (text.endsWith('\n') ? 0 : 1);
        const int wordCount = text.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts).size();
        return {QString("%1 %2 %3 %4\n").arg(lineCount).arg(wordCount).arg(text.size()).arg(name)};
    }

    if (program == "whoami") {
        QString name = qEnvironmentVariable("USERNAME");
        if (name.isEmpty()) {
            name = qEnvironmentVariable("USER", "polo");
        }
        return {name + "\n"};
    }

    if (program == "date") {
        return {QDateTime::currentDateTime().toString(Qt::TextDate) + "\n"};
    }

    if (program == "clear" || program == "cls") {
        return {QString(), CommandAction::Clear};
    }

    if (program == "clip3d" || program == "3d") {
        return {QString(), CommandAction::Show3D};
    }

    if (program == "help") {
        return {
            "Built-ins: say, echo, cd, pwd, ls, mkdir, touch, cat, head, tail, wc, whoami, date, clear, cls, clip3d, 3d, exit\n"
            "Examples: say hello 😄 | cd .. | clip3d\n"};
    }

    if (program == "exit") {
        return {QString(), CommandAction::CloseTab};
    }

    return {QString()};
}

CommandResult ShellSession::runExternal(const ParsedCommand &command)
{
    QProcess process;
    process.setWorkingDirectory(currentDirectory_);
    process.setProcessChannelMode(QProcess::MergedChannels);

#ifdef Q_OS_WIN
    const QString comspec = qEnvironmentVariable("COMSPEC", "cmd.exe");
    process.start(comspec, {"/C", command.original});
#else
    process.start(command.program, command.args);
#endif

    if (!process.waitForStarted(3000)) {
        return {QString("\x1b[31m%1: invalid command\x1b[0m\n").arg(command.program)};
    }

    QByteArray collected;
    while (!process.waitForFinished(50)) {
        collected += process.readAll();
        QCoreApplication::processEvents();
    }
    collected += process.readAll();

    QString text = QString::fromLocal8Bit(collected);
    if (!text.endsWith('\n') && !text.isEmpty()) {
        text.append('\n');
    }
    return {text};
}
