#pragma once

#include <QString>

class PathTools {
public:
    static QString homePath();
    static QString initialDirectory();
    static QString cleanDisplayPath(const QString &path);
    static QString resolvePath(const QString &base, const QString &input);
};
