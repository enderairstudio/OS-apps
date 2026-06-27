#pragma once

#include <QString>
#include <QStringList>

struct ParsedCommand {
    QString program;
    QStringList args;
    QString original;
    bool empty = true;
};

class CommandParser {
public:
    static ParsedCommand parse(const QString &line);
};
