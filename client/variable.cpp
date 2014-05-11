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


#include "variable.h"
#include "xmlerror.h"
#include <QRegularExpression>

Variable::Variable(QObject *parent) :
    ProfileItem(parent),
    m_contents(QVariant::String)
{
    enable(true);  // unused
}

Variable::Variable(const Variable &rhs, QObject *parent) :
    ProfileItem(parent)
{
    clone(rhs);
}

Variable & Variable::operator =(const Variable &rhs)
{
    clone(rhs);

    return *this;
}

bool Variable::operator ==(const Variable &rhs)
{
    if (m_contents != rhs.m_contents)
    {
        return false;
    }

    return ProfileItem::operator ==(rhs);
}

bool Variable::operator !=(const Variable &rhs)
{
    return !(*this == rhs);
}

void Variable::clone(const Variable &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    ProfileItem::clone(rhs);

    enable(true);  // unused
    m_contents = rhs.m_contents;
}

bool Variable::operator <(const Variable &rhs)
{
    return name().compare(rhs.name()) < 0;
}

QString Variable::typeName()
{
    switch (m_contents.type())
    {
    case QVariant::Double:
        return QString("numeric");

    case QVariant::Bool:
        return QString("boolean");

    case QVariant::String:
        return QString("string");

    default:
        return QString("invalid");
    }
}

QVariant::Type Variable::translateType(const QString &type)
{
    if (type.isEmpty() || type.compare("string", Qt::CaseInsensitive) == 0)
    {
        return QVariant::String;
    }
    else if (type.compare("boolean", Qt::CaseInsensitive) == 0)
    {
        return QVariant::Bool;
    }
    else if (type.compare("numeric", Qt::CaseInsensitive) == 0)
    {
        return QVariant::Double;
    }

    return QVariant::Invalid;
}

QVariant Variable::translateValue(const QVariant &value, QVariant::Type type)
{
    QVariant::Type fromType = value.type();
    if (type == fromType)
    {
        return QVariant(value);
    }

    switch (fromType)
    {
    case QVariant::Double:
        switch (type)
        {
        case QVariant::Bool:
            if (value == QVariant(1.0))
            {
                return QVariant(true);
            }
            else if (value == QVariant(0.0))
            {
                return QVariant(false);
            }
            return QVariant();

        default:
            return value.toString();
        }
    break;

    case QVariant::Bool:
        switch (type)
        {
        case QVariant::Double:
            if (value == QVariant(true))
            {
                return QVariant(1.0);
            }
            return QVariant(0.0);

        default:
            if (value.toBool())
            {
                return QVariant("true");
            }
            return QVariant("false");
        }
    break;

    default:
        switch (type)
        {
        case QVariant::Double:
            if (value.toString().compare("true", Qt::CaseInsensitive) == 0)
            {
                return QVariant(1.0);
            }
            else if (value.toString().compare("false", Qt::CaseInsensitive) == 0)
            {
                return QVariant(0.0);
            }
            return QVariant(value.toDouble());

        default:
            if (value.toString().compare("true", Qt::CaseInsensitive) == 0 || value.toString().compare("1") == 0)
            {
                return QVariant(true);
            }
            return QVariant(false);
        }
    break;
    }

    return QVariant();
}

void Variable::toXml(QXmlStreamWriter &xml)
{
    ProfileItem::toXml(xml);

    xml.writeAttribute("type", typeName());

    xml.writeCharacters(value());
}

void Variable::fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    ProfileItem::fromXml(xml, errors);

    if (!name().startsWith('!'))
    {
        QString varName(xml.attributes().value("name").toString().trimmed());
        if (!validateName(varName))
        {
            errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("invalid value for 'name' attribute: %1").arg(varName));

            varName = unnamed();
        }
        setName(varName);
    }

    QString type(xml.attributes().value("type").toString());
    QVariant::Type enumType = Variable::translateType(type);
    if (enumType == QVariant::Invalid)
    {
        errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("invalid value for 'type' attribute: %1").arg(type));
    }
    else
    {
        QVariant val(xml.readElementText().trimmed());
        if (!val.convert(enumType))
        {
            errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("type mismatch in variable: %1").arg(type));
        }

        setContents(val);
    }
}

bool Variable::validateName(const QString &name)
{
    const QRegularExpression validName("^\\w+$");
    if (!validName.match(name).hasMatch())
    {
        return false;
    }

    return true;
}
