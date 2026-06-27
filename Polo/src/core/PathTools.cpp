#include "PathTools.h"

#include <QDir>
#include <QStandardPaths>

QString PathTools::homePath()
{
    return QDir::homePath();
}

QString PathTools::initialDirectory()
{
    const QString desktop = QStandardPaths::writableLocation(QStandardPaths::DesktopLocation);
    return desktop.isEmpty() ? QDir::homePath() : desktop;
}

QString PathTools::cleanDisplayPath(const QString &path)
{
    const QString home = QDir::toNativeSeparators(QDir::homePath());
    const QString native = QDir::toNativeSeparators(QDir(path).absolutePath());
    if (native == home) {
        return "~";
    }
    if (native.startsWith(home + QDir::separator())) {
        return "~" + native.mid(home.size());
    }
    return native;
}

QString PathTools::resolvePath(const QString &base, const QString &input)
{
    if (input.trimmed().isEmpty() || input == "~") {
        return QDir::homePath();
    }

    QString expanded = input;
    if (expanded.startsWith("~/") || expanded.startsWith("~\\")) {
        expanded = QDir::homePath() + expanded.mid(1);
    }

    QDir dir(expanded);
    if (dir.isRelative()) {
        dir = QDir(base + QDir::separator() + expanded);
    }
    return QDir::cleanPath(dir.absolutePath());
}
