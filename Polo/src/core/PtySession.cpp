#include "PtySession.h"

#include "ShellSession.h"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>

#ifndef Q_OS_WIN
#include <fcntl.h>
#include <pty.h>
#include <signal.h>
#include <sys/ioctl.h>
#include <sys/wait.h>
#include <unistd.h>
#endif

PtySession::PtySession(QObject *parent)
    : QObject(parent)
{
}

PtySession::~PtySession()
{
    close();
}

bool PtySession::start(const QString &workingDirectory)
{
    currentDirectory_ = workingDirectory;

#ifdef Q_OS_WIN
    if (windowsRunning_) {
        return true;
    }

    windowsShell_ = new ShellSession(this);
    connect(windowsShell_, &ShellSession::directoryChanged, this, [this](const QString &path) {
        currentDirectory_ = path;
        emit directoryChanged(path);
    });
    connect(windowsShell_, &ShellSession::commandStarted, this, [this](const QString &) {
        emit directoryChanged(currentDirectory_);
    });

    currentDirectory_ = windowsShell_->currentDirectory();
    windowsRunning_ = true;
    writeWindowsPrompt();
    return true;
#else
    if (masterFd_ >= 0) {
        return true;
    }

    struct winsize size {};
    size.ws_col = 100;
    size.ws_row = 32;

    const pid_t pid = forkpty(&masterFd_, nullptr, nullptr, &size);
    if (pid < 0) {
        emit outputReady("\x1b[31mfailed to create PTY session\x1b[0m\n");
        masterFd_ = -1;
        return false;
    }

    if (pid == 0) {
        const QByteArray cwd = QFile::encodeName(workingDirectory);
        ::chdir(cwd.constData());
        ::setenv("TERM", "xterm-256color", 1);
        ::setenv("COLORTERM", "truecolor", 1);

        const QString rcPath = QCoreApplication::applicationDirPath() + "/assets/polo_bashrc";
        const QByteArray rc = QFile::encodeName(rcPath);

        if (QFileInfo::exists("/bin/bash")) {
            ::execl("/bin/bash", "bash", "--rcfile", rc.constData(), "-i", static_cast<char *>(nullptr));
        }

        const QByteArray shell = qgetenv("SHELL").isEmpty() ? QByteArray("/bin/sh") : qgetenv("SHELL");
        ::execl(shell.constData(), shell.constData(), "-i", static_cast<char *>(nullptr));
        ::_exit(127);
    }

    childPid_ = pid;
    const int flags = ::fcntl(masterFd_, F_GETFL, 0);
    ::fcntl(masterFd_, F_SETFL, flags | O_NONBLOCK);

    readNotifier_ = new QSocketNotifier(masterFd_, QSocketNotifier::Read, this);
    connect(readNotifier_, &QSocketNotifier::activated, this, &PtySession::readAvailable);
    return true;
#endif
}

void PtySession::writeInput(const QByteArray &input)
{
#ifndef Q_OS_WIN
    if (masterFd_ >= 0 && !input.isEmpty()) {
        ::write(masterFd_, input.constData(), static_cast<size_t>(input.size()));
    }
#else
    if (!windowsRunning_ || input.isEmpty()) {
        return;
    }

    if (input.startsWith('\x1b')) {
        return;
    }

    bool printableText = true;
    for (const char byte : input) {
        const unsigned char value = static_cast<unsigned char>(byte);
        if (byte != '\t' && (value < 0x20 || value == 0x7f)) {
            printableText = false;
            break;
        }
    }
    if (printableText) {
        const QString text = QString::fromUtf8(input);
        windowsLineBuffer_.append(text);
        emit outputReady(text);
        return;
    }

    for (const char byte : input) {
        if (byte == '\r' || byte == '\n') {
            emit outputReady("\n");
            runWindowsCommand();
            continue;
        }

        if (byte == char(0x03)) {
            windowsLineBuffer_.clear();
            emit outputReady("^C\n");
            writeWindowsPrompt();
            continue;
        }

        if (byte == char(0x15)) {
            const int chars = windowsLineBuffer_.size();
            windowsLineBuffer_.clear();
            emit outputReady(QString(chars, QChar('\b')));
            continue;
        }

        if (byte == char(0x7f) || byte == '\b') {
            if (!windowsLineBuffer_.isEmpty()) {
                windowsLineBuffer_.chop(1);
                emit outputReady("\b");
            }
            continue;
        }

        Q_UNUSED(byte);
    }
#endif
}

void PtySession::resizeTerminal(int columns, int rows)
{
#ifndef Q_OS_WIN
    if (masterFd_ < 0) {
        return;
    }
    struct winsize size {};
    size.ws_col = static_cast<unsigned short>(qMax(1, columns));
    size.ws_row = static_cast<unsigned short>(qMax(1, rows));
    ::ioctl(masterFd_, TIOCSWINSZ, &size);
#else
    Q_UNUSED(columns);
    Q_UNUSED(rows);
#endif
}

void PtySession::close()
{
#ifndef Q_OS_WIN
    if (readNotifier_) {
        readNotifier_->setEnabled(false);
        readNotifier_->deleteLater();
        readNotifier_ = nullptr;
    }
    if (childPid_ > 0) {
        ::kill(childPid_, SIGHUP);
        ::waitpid(childPid_, nullptr, WNOHANG);
        childPid_ = -1;
    }
    if (masterFd_ >= 0) {
        ::close(masterFd_);
        masterFd_ = -1;
    }
#else
    windowsRunning_ = false;
    windowsLineBuffer_.clear();
    if (windowsShell_) {
        windowsShell_->deleteLater();
        windowsShell_ = nullptr;
    }
#endif
}

QString PtySession::currentDirectory() const
{
    return currentDirectory_;
}

bool PtySession::isRunning() const
{
#ifdef Q_OS_WIN
    return windowsRunning_;
#else
    return masterFd_ >= 0;
#endif
}

void PtySession::readAvailable()
{
#ifndef Q_OS_WIN
    QByteArray data;
    char buffer[4096];
    while (true) {
        const ssize_t count = ::read(masterFd_, buffer, sizeof(buffer));
        if (count > 0) {
            data.append(buffer, static_cast<int>(count));
            continue;
        }
        break;
    }

    if (data.isEmpty()) {
        int status = 0;
        if (childPid_ > 0 && ::waitpid(childPid_, &status, WNOHANG) == childPid_) {
            childPid_ = -1;
            emit finished();
        }
        return;
    }

    const QString text = QString::fromLocal8Bit(data);
    const QString visible = stripMetadata(text);
    if (!visible.isEmpty()) {
        emit outputReady(visible);
    }
#endif
}

QString PtySession::stripMetadata(const QString &text)
{
    QString combined = metadataBuffer_ + text;
    metadataBuffer_.clear();

    QString visible;
    for (int i = 0; i < combined.size(); ++i) {
        if (combined.at(i) == QChar(0x1b) && i + 1 < combined.size() && combined.at(i + 1) == ']') {
            int end = i + 2;
            while (end < combined.size() && combined.at(end) != QChar(0x07)) {
                if (combined.at(end) == QChar(0x1b) && end + 1 < combined.size() && combined.at(end + 1) == '\\') {
                    break;
                }
                ++end;
            }

            if (end >= combined.size()) {
                metadataBuffer_ = combined.mid(i);
                break;
            }

            const QString payload = combined.mid(i + 2, end - i - 2);
            if (payload.startsWith("7;file://")) {
                const QString pathPart = payload.mid(QString("7;file://").size());
                const int firstSlash = pathPart.indexOf('/');
                if (firstSlash >= 0) {
                    currentDirectory_ = QDir::cleanPath(pathPart.mid(firstSlash));
                    emit directoryChanged(currentDirectory_);
                }
            }

            i = end;
            if (i + 1 < combined.size() && combined.at(i) == QChar(0x1b) && combined.at(i + 1) == '\\') {
                ++i;
            }
            continue;
        }

        visible.append(combined.at(i));
    }

    return visible;
}

#ifdef Q_OS_WIN
void PtySession::runWindowsCommand()
{
    if (!windowsShell_) {
        writeWindowsPrompt();
        return;
    }

    const QString command = windowsLineBuffer_;
    windowsLineBuffer_.clear();
    const CommandResult result = windowsShell_->run(command);
    currentDirectory_ = windowsShell_->currentDirectory();

    if (result.action == CommandAction::Clear) {
        emit outputReady("\x1b[2J");
    } else if (result.action == CommandAction::CloseTab) {
        emit finished();
        return;
    } else if (!result.output.isNull()) {
        emit outputReady(result.output);
    }

    writeWindowsPrompt();
}

void PtySession::writeWindowsPrompt()
{
    emit outputReady("\x1b[90m> \x1b[0m");
}
#endif
