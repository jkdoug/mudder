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


#include "profileitem.h"
#include "group.h"
#include "xmlerror.h"
#include <QRegularExpression>

ProfileItem::ProfileItem(QObject *parent) :
    QObject(parent)
{
    m_enabled = true;
    m_sequence = 1000;
}

ProfileItem::ProfileItem(const ProfileItem &rhs, QObject *parent) :
    QObject(parent)
{
    clone(rhs);
}

void ProfileItem::setName(const QString &name)
{
    if (name != m_name)
    {
        m_name = name;
        emit modified(this);
    }
}

void ProfileItem::setSequence(int sequence)
{
    if (sequence != m_sequence)
    {
        m_sequence = sequence;
        emit modified(this);
    }
}

void ProfileItem::enable(bool flag)
{
    if (flag != m_enabled)
    {
        m_enabled = flag;
        emit modified(this);
    }
}

ProfileItem & ProfileItem::operator =(const ProfileItem &rhs)
{
    clone(rhs);

    return *this;
}

bool ProfileItem::operator ==(const ProfileItem &rhs)
{
    if (m_sequence != rhs.m_sequence)
    {
        return false;
    }

    if (m_name != rhs.m_name)
    {
        return false;
    }

    if (m_enabled != rhs.m_enabled)
    {
        return false;
    }

    return true;
}

bool ProfileItem::operator !=(const ProfileItem &rhs)
{
    return !(*this == rhs);
}

bool ProfileItem::operator <(const ProfileItem &rhs)
{
    return sequence() < rhs.sequence();
}

void ProfileItem::clone(const ProfileItem &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    m_name = rhs.m_name;

    m_enabled = rhs.m_enabled;

    m_sequence = rhs.m_sequence;
}

QString ProfileItem::fullName() const
{
    if (group() != 0)
    {
        return QString("%1/%2").arg(group()->path()).arg(name());
    }

    return name();
}

Group * ProfileItem::group() const
{
    return qobject_cast<Group *>(parent());
}

bool ProfileItem::enabled() const
{
    return m_enabled && (group() == 0 || group()->enabled());
}

void ProfileItem::toXml(QXmlStreamWriter &xml)
{
    xml.writeAttribute("name", name());
    if (!enabledFlag())
    {
        xml.writeAttribute("enabled", "n");
    }
    if (sequence() > 0)
    {
        xml.writeAttribute("sequence", QString::number(sequence()));
    }
}

void ProfileItem::fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    QString name(xml.attributes().value("name").toString().trimmed());
    if (!validateName(name))
    {
        XmlError *err = new XmlError(xml.lineNumber(), xml.columnNumber());
        if (name.isEmpty())
        {
            err->setMessage(tr("missing 'name' attribute"));
        }
        else
        {
            err->setMessage(tr("invalid value for 'name' attribute: %1").arg(name));
        }
        errors << err;

        name = unnamed();
    }
    setName(name);

    enable(xml.attributes().value("enabled").compare("n", Qt::CaseInsensitive) != 0);

    QString seq(xml.attributes().value("sequence").toString());
    int sequence = 1000;
    if (!seq.isEmpty())
    {
        bool valid = true;
        sequence = seq.toInt(&valid);
        if (!valid)
        {
            errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("invalid value for 'sequence' attribute: %1").arg(seq));

            sequence = 1000;
        }
    }
    setSequence(qBound(1, sequence, 100000));
}

bool ProfileItem::validateName(const QString &name, bool allowEmpty)
{
    if (name.isEmpty())
    {
        return allowEmpty;
    }

    const QRegularExpression validName("^[\\w,'\"@#%& \\-\\$\\*\\(\\)\\[\\]\\.]+$");
    if (!validName.match(name).hasMatch())
    {
        return false;
    }

    return true;
}

QString ProfileItem::unnamed()
{
    static int counter = 0;
    return QString("!unnamed%1").arg(++counter);
}
