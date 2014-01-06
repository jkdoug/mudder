/***************************************************************************
 *   Copyright (C) 2013 by Jason Douglas                                   *
 *   larkin.dischai@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef MATCHABLE_H
#define MATCHABLE_H

#include "executable.h"
#include <QDateTime>
#include <QObject>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QString>
#include <QStringList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class Matchable : public Executable
{
    Q_OBJECT

    Q_PROPERTY(QString lineMatched READ lineMatched RESET reset)
    Q_PROPERTY(QRegularExpression regex READ regex)
    Q_PROPERTY(QString pattern READ pattern WRITE setPattern)
    Q_PROPERTY(int matchStart READ matchStart)
    Q_PROPERTY(int matchEnd READ matchEnd)
    Q_PROPERTY(int matchCount READ matchCount)
    Q_PROPERTY(QDateTime matchTime READ lastMatched)
    Q_PROPERTY(bool keepEvaluating READ keepEvaluating WRITE setKeepEvaluating)
    Q_PROPERTY(bool caseSensitive READ caseSensitive WRITE setCaseSensitive)

public:
    explicit Matchable(QObject *parent = 0);
    Matchable(const Matchable &rhs, QObject *parent = 0);

    Matchable & operator =(const Matchable &rhs);

    bool operator ==(const Matchable &rhs);
    bool operator !=(const Matchable &rhs);

    const QString & lineMatched() const { return m_lineMatched; }
    const QRegularExpression & regex() const { return m_regex; }
    QString pattern() const { return m_regex.pattern(); }
    void setPattern(const QString &pattern) { m_regex.setPattern(pattern); }
    QRegularExpressionMatch * lastMatch() const { return m_match; }
    int matchStart() const;
    int matchEnd() const;
    const QDateTime & lastMatched() const { return m_lastMatched; }
    int matchCount() const { return m_matchCount; }
    bool keepEvaluating() const { return m_keepEvaluating; }
    void setKeepEvaluating(bool flag) { m_keepEvaluating = flag; }
    bool caseSensitive() const { return !m_regex.patternOptions().testFlag(QRegularExpression::CaseInsensitiveOption); }
    void setCaseSensitive(bool flag);

    virtual QString value() const { return pattern(); }

    bool match(const QString &str, int offset = 0);
    void reset();

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml);

protected:
    void clone(const Matchable &rhs);

private:
    QString m_lineMatched;

    QRegularExpression m_regex;
    QRegularExpressionMatch *m_match;

    QDateTime m_lastMatched;
    int m_matchCount;

    bool m_keepEvaluating;
};

#endif // MATCHABLE_H
