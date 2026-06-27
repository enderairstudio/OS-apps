#pragma once

#include <QObject>
#include <QSocketNotifier>
#include <QString>

class ShellSession;

class PtySession : public QObject {
    Q_OBJECT

public:
    explicit PtySession(QObject *parent = nullptr);
    ~PtySession() override;

    bool start(const QString &workingDirectory);
    void writeInput(const QByteArray &input);
    void resizeTerminal(int columns, int rows);
    void close();

    QString currentDirectory() const;
    bool isRunning() const;

signals:
    void outputReady(const QString &text);
    void directoryChanged(const QString &path);
    void finished();

private:
    void readAvailable();
    QString stripMetadata(const QString &text);
#ifdef Q_OS_WIN
    void runWindowsCommand();
    void writeWindowsPrompt();
#endif

    int masterFd_ = -1;
    int childPid_ = -1;
    QSocketNotifier *readNotifier_ = nullptr;
    QString currentDirectory_;
    QString metadataBuffer_;
#ifdef Q_OS_WIN
    ShellSession *windowsShell_ = nullptr;
    QString windowsLineBuffer_;
    bool windowsRunning_ = false;
#endif
};
