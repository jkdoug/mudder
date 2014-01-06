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


#include "alias.h"
#include "xmlexception.h"
#include <QDebug>
#include <QStringList>

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
    xml.writeStartElement("alias");

    if (echo())
    {
        xml.writeAttribute("echo", "y");
    }

    Matchable::toXml(xml);

    xml.writeEndElement();
}

void Alias::fromXml(QXmlStreamReader &xml)
{
    QStringList warnings;

    setEcho(xml.attributes().value("echo").compare("y", Qt::CaseInsensitive) == 0);

    try
    {
        Matchable::fromXml(xml);
    }
    catch (XmlException *xe)
    {
        warnings.append(xe->warnings());

        delete xe;
    }

    if (contents().isEmpty())
    {
        warnings.append(tr("XML: Line %1; missing or empty 'send' element in alias").arg(xml.lineNumber()));
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}
