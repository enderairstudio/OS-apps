#include "CommandParser.h"

ParsedCommand CommandParser::parse(const QString &line)
{
    ParsedCommand parsed;
    parsed.original = line.trimmed();
    parsed.empty = parsed.original.isEmpty();
    if (parsed.empty) {
        return parsed;
    }

    QString current;
    bool inQuote = false;
    QChar quoteChar;
    QStringList tokens;

    for (int i = 0; i < parsed.original.size(); ++i) {
        const QChar ch = parsed.original.at(i);
        if ((ch == '"' || ch == '\'') && (!inQuote || quoteChar == ch)) {
            if (inQuote && quoteChar == ch) {
                inQuote = false;
            } else {
                inQuote = true;
                quoteChar = ch;
            }
            continue;
        }

        if (ch.isSpace() && !inQuote) {
            if (!current.isEmpty()) {
                tokens << current;
                current.clear();
            }
            continue;
        }

        if (ch == '\\' && i + 1 < parsed.original.size()) {
            current.append(parsed.original.at(++i));
            continue;
        }

        current.append(ch);
    }

    if (!current.isEmpty()) {
        tokens << current;
    }

    if (!tokens.isEmpty()) {
        parsed.program = tokens.takeFirst();
        parsed.args = tokens;
        parsed.empty = false;
    }

    return parsed;
}
