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


#include "trigger.h"
#include "xmlerror.h"

Trigger::Trigger(QObject *parent) :
    Matchable(parent)
{
    m_omit = false;
    m_repeat = false;
}

Trigger::Trigger(const Trigger &rhs, QObject *parent) :
    Matchable(parent)
{
    clone(rhs);
}

Trigger & Trigger::operator =(const Trigger &rhs)
{
    clone(rhs);

    return *this;
}

bool Trigger::operator ==(const Trigger &rhs)
{
    if (m_omit != rhs.m_omit)
    {
        return false;
    }

    if (m_repeat != rhs.m_repeat)
    {
        return false;
    }

    return Matchable::operator ==(rhs);
}

bool Trigger::operator !=(const Trigger &rhs)
{
    return !(*this == rhs);
}

void Trigger::clone(const Trigger &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    Matchable::clone(rhs);

    m_omit = rhs.m_omit;
    m_repeat = rhs.m_repeat;
}

bool Trigger::operator <(const Trigger &rhs)
{
    return Matchable::operator <(rhs);
}

bool Trigger::lessSequence(const Trigger *lhs, const Trigger *rhs)
{
    return Matchable::lessSequence(lhs, rhs);
}

void Trigger::toXml(QXmlStreamWriter &xml)
{
    if (repeat())
    {
        xml.writeAttribute("repeat", "y");
    }
    if (omit())
    {
        xml.writeAttribute("omit", "y");
    }

    Matchable::toXml(xml);
}

void Trigger::fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    setOmit(xml.attributes().value("omit").compare("y", Qt::CaseInsensitive) == 0);
    setRepeat(xml.attributes().value("repeat").compare("y", Qt::CaseInsensitive) == 0);

    Matchable::fromXml(xml, errors);

    if (!omit() && contents().isEmpty())
    {
        errors << new XmlError(xml.lineNumber(), xml.columnNumber(), "missing or empty 'send' element in trigger");
    }
}
