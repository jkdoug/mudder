/*
  Mudder, a cross-platform text gaming client

  Copyright (C) 2014 Jason Douglas
  jkdoug@gmail.com

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


#include "accelerator.h"
#include "xmlerror.h"

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
    xml.writeAttribute("key", key().toString());

    Executable::toXml(xml);
}

void Accelerator::fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    QString keyName(xml.attributes().value("key").toString());
    QKeySequence key(QKeySequence::fromString(keyName));
    if (key.isEmpty())
    {
        errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("invalid value for 'key' attribute: %1").arg(keyName));
    }
    setKey(key);

    Executable::fromXml(xml, errors);
}
