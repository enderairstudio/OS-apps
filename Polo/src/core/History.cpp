#include "History.h"

void History::add(const QString &command)
{
    const QString trimmed = command.trimmed();
    if (trimmed.isEmpty()) {
        resetCursor();
        return;
    }
    if (commands_.isEmpty() || commands_.last() != trimmed) {
        commands_ << trimmed;
    }
    resetCursor();
}

QString History::previous(const QString &fallback)
{
    if (commands_.isEmpty()) {
        return fallback;
    }
    if (cursor_ < 0) {
        cursor_ = commands_.size() - 1;
    } else if (cursor_ > 0) {
        --cursor_;
    }
    return commands_.value(cursor_, fallback);
}

QString History::next(const QString &fallback)
{
    if (commands_.isEmpty() || cursor_ < 0) {
        return fallback;
    }
    if (cursor_ < commands_.size() - 1) {
        ++cursor_;
        return commands_.value(cursor_, fallback);
    }
    resetCursor();
    return QString();
}

void History::resetCursor()
{
    cursor_ = -1;
}
