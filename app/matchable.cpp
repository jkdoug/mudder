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


#include "matchable.h"
#include "xmlexception.h"
#include <QDebug>
#include <QStringList>


Matchable::Matchable(QObject *parent) :
    Executable(parent)
{
    m_match = 0;
    m_matchCount = 0;
    m_evalCount = 0;
    m_keepEvaluating = false;

    setCaseSensitive(true);
    setPattern("");
}

Matchable::Matchable(const Matchable &rhs, QObject *parent) :
    Executable(parent)
{
    clone(rhs);
}

Matchable & Matchable::operator =(const Matchable &rhs)
{
    clone(rhs);

    return *this;
}

bool Matchable::operator ==(const Matchable &rhs)
{
    if (m_lineMatched != rhs.m_lineMatched)
    {
        return false;
    }

    if (m_regex != rhs.m_regex)
    {
        return false;
    }

    if (m_keepEvaluating != rhs.m_keepEvaluating)
    {
        return false;
    }

    return Executable::operator ==(rhs);
}

bool Matchable::operator !=(const Matchable &rhs)
{
    return !(*this == rhs);
}

void Matchable::clone(const Matchable &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    Executable::clone(rhs);

    m_match = 0;

    m_lineMatched = rhs.m_lineMatched;

    m_regex = rhs.m_regex;

    m_lastMatched = rhs.m_lastMatched;
    m_matchCount = rhs.m_matchCount;
    m_evalCount = rhs.m_evalCount;

    m_keepEvaluating = rhs.m_keepEvaluating;
}

int Matchable::matchStart() const
{
    if (!m_match)
    {
        return -1;
    }

    return m_match->capturedStart();
}

int Matchable::matchEnd() const
{
    if (!m_match)
    {
        return 10000000;
    }

    return m_match->capturedEnd();
}

void Matchable::setCaseSensitive(bool flag)
{
    // Alter only the case sensitivity flag, leaving other options intact
    QRegularExpression::PatternOptions opts = m_regex.patternOptions();
    if (flag)
    {
        opts &= ~QRegularExpression::CaseInsensitiveOption;
    }
    else
    {
        opts |= QRegularExpression::CaseInsensitiveOption;
    }
    m_regex.setPatternOptions(opts);
}

bool Matchable::match(const QString &str, int offset)
{
    m_evalCount++;

    m_match = new QRegularExpressionMatch(m_regex.match(str, offset));
    if (m_match->hasMatch())
    {
        m_lineMatched = str;
        m_lastMatched = QDateTime::currentDateTime();
        m_matchCount++;
        return true;
    }

    reset();
    return false;
}

void Matchable::reset()
{
    m_lineMatched.clear();
    m_match = 0;
}

void Matchable::toXml(QXmlStreamWriter &xml)
{
    xml.writeAttribute("pattern", pattern());
    if (keepEvaluating())
    {
        xml.writeAttribute("keep_evaluating", "y");
    }
    if (!caseSensitive())
    {
        xml.writeAttribute("ignore_case", "y");
    }

    Executable::toXml(xml);
}

void Matchable::fromXml(QXmlStreamReader &xml)
{
    QStringList warnings;

    QString pattern(xml.attributes().value("pattern").toString());
    if (pattern.isEmpty())
    {
        warnings.append(tr("XML: Line %1; missing 'pattern' attribute").arg(xml.lineNumber()));
    }
    else
    {
        setPattern(pattern);
        if (!regex().isValid())
        {
            warnings.append(tr("XML: Line %1; invalid regular expression pattern (%2)").arg(xml.lineNumber()).arg(pattern));
        }
    }

    setKeepEvaluating(xml.attributes().value("keep_evaluating").compare("y", Qt::CaseInsensitive) == 0);
    setCaseSensitive(xml.attributes().value("ignore_case").compare("y", Qt::CaseInsensitive) != 0);

    try
    {
        Executable::fromXml(xml);
    }
    catch (XmlException *xe)
    {
        warnings.append(xe->warnings());

        delete xe;
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}
