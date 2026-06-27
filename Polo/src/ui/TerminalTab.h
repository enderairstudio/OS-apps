#pragma once

#include "core/PathTools.h"
#include "core/PtySession.h"

#include <QObject>
#include <QString>

class TerminalTab : public QObject {
    Q_OBJECT

public:
    explicit TerminalTab(QObject *parent = nullptr);

    PtySession *pty();
    QString title() const;
    QString displayDirectory() const;
    QString scrollback() const;
    void appendScrollback(const QString &text);
    void clearScrollback();

signals:
    void changed();
    void outputReady(const QString &text);

private:
    PtySession pty_;
    QString scrollback_;
};
