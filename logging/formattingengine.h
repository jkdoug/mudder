/*
  Mudder, a cross-platform text gaming client

  Copyright (C) 2014 Jason Douglas
  larkin.dischai@gmail.com

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/


#ifndef FORMATTINGENGINE_H
#define FORMATTINGENGINE_H

#include "logging_global.h"
#include "logger.h"
#include <QObject>
#include <QRegularExpression>
#include <QVariant>

struct CustomFormattingHint
{
    CustomFormattingHint(const QRegularExpression &regex, const QString &hint, Logger::MessageTypeFlags typeFlags = Logger::AllLogLevels) :
        regex(regex),
        hint(hint),
        typeFlags(typeFlags)
    {
    }
    CustomFormattingHint(const CustomFormattingHint &rhs)
    {
        hint = rhs.hint;
        typeFlags = rhs.typeFlags;
        regex = rhs.regex;
    }
    CustomFormattingHint& operator=(const CustomFormattingHint &rhs)
    {
        if (this == &rhs)
        {
            return *this;
        }

        hint = rhs.hint;
        typeFlags = rhs.typeFlags;
        regex = rhs.regex;

        return *this;
    }
    bool operator==(const CustomFormattingHint &rhs)
    {
        if (hint != rhs.hint)
        {
            return false;
        }
        if (typeFlags != rhs.typeFlags)
        {
            return false;
        }
        if (regex != rhs.regex)
        {
            return false;
        }

        return true;
    }
    bool operator!=(const CustomFormattingHint &rhs)
    {
        return !(*this == rhs);
    }

    QString                     hint;
    QRegularExpression          regex;
    Logger::MessageTypeFlags    typeFlags;
};

class LOGGINGSHARED_EXPORT FormattingEngine : public QObject
{
    Q_OBJECT
public:
    FormattingEngine() {}
    virtual ~FormattingEngine() {}

    virtual QString initializeString() const = 0;
    virtual QString finalizeString() const = 0;
    virtual QString formatMessage(Logger::MessageType type, const QList<QVariant> &messages) const = 0;
    virtual QString name() const = 0;
    virtual QString fileExtension() const = 0;
    virtual QString endOfLine() const = 0;

    bool addColorFormattingHint(CustomFormattingHint hint)
    {
        if (!hint.regex.isValid())
        {
            return false;
        }

        m_hints.append(hint);
        return true;
    }
    QList<CustomFormattingHint> colorFormattingHints() const { return m_hints; }
    void clearColorFormattingHints() { m_hints.clear(); }
    void removeColorFormattingHint(CustomFormattingHint hint) { m_hints.removeOne(hint); }
    QString matchColorFormattingHint(const QString &message, Logger::MessageType type) const
    {
        foreach (CustomFormattingHint hint, m_hints)
        {
            if (hint.typeFlags.testFlag(type))
            {
                QRegularExpressionMatch m = hint.regex.match(message);
                if (m.hasMatch())
                {
                    return hint.hint;
                }
            }
        }

        return QString();
    }

    static QString escape(const QString &plain)
    {
        QString rich;
        rich.reserve(int(plain.length() * 1.1));
        for (int i = 0; i < plain.length(); i++)
        {
            if (plain.at(i) == QLatin1Char('<'))
            {
                rich += QLatin1String("&lt;");
            }
            else if (plain.at(i) == QLatin1Char('>'))
            {
                rich += QLatin1String("&gt;");
            }
            else if (plain.at(i) == QLatin1Char('&'))
            {
                rich += QLatin1String("&amp;");
            }
            else if (plain.at(i) == QLatin1Char('"'))
            {
                rich += QLatin1String("&quot;");
            }
            else
            {
                rich += plain.at(i);
            }
        }
        return rich;
    }

private:
    QList<CustomFormattingHint> m_hints;
};

#endif // FORMATTINGENGINE_H
