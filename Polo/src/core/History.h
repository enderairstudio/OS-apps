#pragma once

#include <QString>
#include <QStringList>

class History {
public:
    void add(const QString &command);
    QString previous(const QString &fallback);
    QString next(const QString &fallback);
    void resetCursor();

private:
    QStringList commands_;
    int cursor_ = -1;
};
