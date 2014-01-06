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


#include "accelerator.h"
#include "xmlexception.h"
#include <QDebug>
#include <QStringList>

Accelerator::Accelerator(QObject *parent) :
    Executable(parent)
{
}

Accelerator::Accelerator(const Accelerator &rhs, QObject *parent) :
    Executable(parent)
{
    clone(rhs);
}

Accelerator & Accelerator::operator =(const Accelerator &rhs)
{
    clone(rhs);

    return *this;
}

bool Accelerator::operator ==(const Accelerator &rhs)
{
    if (m_key != rhs.m_key)
    {
        return false;
    }

    return Executable::operator ==(rhs);
}

bool Accelerator::operator !=(const Accelerator &rhs)
{
    return !(*this == rhs);
}

void Accelerator::clone(const Accelerator &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    Executable::clone(rhs);

    m_key = rhs.m_key;
}

bool Accelerator::operator <(const Accelerator &rhs)
{
    return value().compare(rhs.value()) < 0;
}

bool Accelerator::lessSequence(const Accelerator *lhs, const Accelerator *rhs)
{
    return lhs->value().compare(rhs->value()) < 0;
}

void Accelerator::toXml(QXmlStreamWriter &xml)
{
    xml.writeStartElement("accelerator");

    xml.writeAttribute("key", key().toString());

    Executable::toXml(xml);

    xml.writeEndElement();
}

void Accelerator::fromXml(QXmlStreamReader &xml)
{
    QStringList warnings;

    QKeySequence key(QKeySequence::fromString(xml.attributes().value("key").toString()));
    if (key.isEmpty())
    {
        warnings.append(tr("XML: Line %1; invalid 'key' attribute in accelerator").arg(xml.lineNumber()));
    }
    setKey(key);

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
