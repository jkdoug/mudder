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


#include "alias.h"
#include "xmlerror.h"

Alias::Alias(QObject *parent) :
    Matchable(parent)
{
    m_echo = false;
}

Alias::Alias(const Alias &rhs, QObject *parent) :
    Matchable(parent)
{
    clone(rhs);
}

Alias & Alias::operator =(const Alias &rhs)
{
    clone(rhs);

    return *this;
}

bool Alias::operator ==(const Alias &rhs)
{
    if (m_echo != rhs.m_echo)
    {
        return false;
    }

    return Matchable::operator ==(rhs);
}

bool Alias::operator !=(const Alias &rhs)
{
    return !(*this == rhs);
}

bool Alias::operator <(const Alias &rhs)
{
    return Matchable::operator <(rhs);
}

bool Alias::lessSequence(const Alias *lhs, const Alias *rhs)
{
    return Matchable::lessSequence(lhs, rhs);
}

void Alias::clone(const Alias &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    Matchable::clone(rhs);

    m_echo = rhs.m_echo;
}

void Alias::toXml(QXmlStreamWriter &xml)
{
    if (echo())
    {
        xml.writeAttribute("echo", "y");
    }

    Matchable::toXml(xml);
}

void Alias::fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    setEcho(xml.attributes().value("echo").compare("y", Qt::CaseInsensitive) == 0);

    Matchable::fromXml(xml, errors);

    if (contents().isEmpty())
    {
        errors << new XmlError(xml.lineNumber(), xml.columnNumber(), "missing or empty 'send' element in alias");
    }
}
