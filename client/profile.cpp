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


#include "profile.h"
#include "logger.h"
#include "coreapplication.h"
#include "group.h"
#include "profileitem.h"
#include "profileitemfactory.h"
#include "xmlerror.h"
#include <QDateTime>

Profile::Profile(QObject *parent) :
    QObject(parent)
{
    m_root = new Group(0);
    m_activeGroup = m_root;
}

void Profile::setActiveGroup(Group *group)
{
    LOG_TRACE("Profile::setActiveGroup", group ? group->fullName() : "<NULL>");

    m_activeGroup = group;
}

void Profile::setName(const QString &name)
{
    if (name != m_name)
    {
        m_name = name;
        emit optionChanged("name");
    }
}

void Profile::setAddress(const QString &address)
{
    if (address != m_address)
    {
        m_address = address;
        emit optionChanged("address");
    }
}

void Profile::setPort(int port)
{
    if (port != m_port)
    {
        m_port = port;
        emit optionChanged("port");
    }
}

void Profile::setAutoConnect(bool flag)
{
    if (flag != m_autoConnect)
    {
        m_autoConnect = flag;
        emit optionChanged("autoConnect");
    }
}

void Profile::setAutoReconnect(bool flag)
{
    if (flag != m_autoReconnect)
    {
        m_autoReconnect = flag;
        emit optionChanged("autoReconnect");
    }
}

void Profile::toXml(QXmlStreamWriter &xml)
{
    LOG_TRACE("Profile::toXml", xml.device()->objectName());

    xml.writeStartDocument();

    xml.writeStartElement("mudder");
    xml.writeAttribute("version", CoreApplication::applicationVersion());
    xml.writeAttribute("saved", QDateTime::currentDateTime().toString());

    xml.writeStartElement("profile");
    xml.writeAttribute("name", name());
    xml.writeEndElement();

    xml.writeStartElement("settings");
    m_root->toXml(xml);
    xml.writeEndElement();

    xml.writeEndDocument();
}

void Profile::fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    LOG_TRACE("Profile::fromXml", xml.device() ? xml.device()->objectName() : "no device");

    while (!xml.atEnd())
    {
        xml.readNext();

        QString name(xml.name().toString());
        if (xml.isStartElement())
        {
            ProfileItem *item = ProfileItemFactory::create(name, activeGroup());
            if (item)
            {
                item->fromXml(xml, errors);
                activeGroup()->addItem(item);
            }

            if (name == "group")
            {
                setActiveGroup(qobject_cast<Group *>(item));
            }
            else if (name == "profile")
            {
                readProfile(xml, errors);
            }
        }
        else if (xml.isEndElement())
        {
            if (name == "group")
            {
                setActiveGroup(activeGroup()->group());
            }
        }
    }
}

void Profile::readProfile(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    setName(xml.attributes().value("name").toString());

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "profile")
        {
            break;
        }

        if (xml.isStartElement())
        {
            if (xml.name() == "host")
            {
                setAutoConnect(xml.attributes().value("connect").compare("y", Qt::CaseInsensitive) == 0);
                setAutoReconnect(xml.attributes().value("reconnect").compare("y", Qt::CaseInsensitive) == 0);

                QString address(xml.attributes().value("address").toString());
                if (address.isEmpty())
                {
                    errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("missing or empty host address"));
                }
                setAddress(address);

                bool valid = true;
                int port = xml.attributes().value("port").toString().toInt(&valid);
                if (!valid)
                {
                    port = 23;
                    errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("missing or invalid host port, defaulting to 23"));
                }
                setPort(port);
            }
//            else if (xml.name() == "commandLine")
//            {
//                QString sep(xml.attributes().value("separator").toString());
//                if (!sep.isEmpty())
//                {
//                    if (sep.length() > 1)
//                    {
//                        warnings.append(tr("XML: Line %1; command separator should be a single character").arg(xml.lineNumber()));
//                    }
//                    setCommandSeparator(sep.at(0));
//                }

//                setClearCommandAfterSend(xml.attributes().value("clear").compare("n", Qt::CaseInsensitive) != 0);
//            }
//            else if (xml.name() == "logging")
//            {
//                try
//                {
//                    readLogging(xml);
//                }
//                catch (XmlException *xe)
//                {
//                    warnings.append(xe->warnings());

//                    delete xe;
//                }
//            }
//            else if (xml.name() == "scripting")
//            {
//                QString prefix(xml.attributes().value("prefix").toString());
//                if (!prefix.isEmpty())
//                {
//                   setScriptPrefix(prefix);
//                }
//                QString script(xml.attributes().value("file").toString());
//                if (!script.isEmpty())
//                {
//                    setScriptFilename(script);
//                }
//            }
//            else if (xml.name() == "display")
//            {
//                try
//                {
//                    readDisplay(xml);
//                }
//                catch (XmlException *xe)
//                {
//                    warnings.append(xe->warnings());

//                    delete xe;
//                }
//            }
//            else if (xml.name() == "mapper")
//            {
//                QString colorString(xml.attributes().value("bg").toString());
//                QColor bg(colorString);
//                if (bg.isValid())
//                {
//                    setMapBackground(bg);
//                }
//                else if (!colorString.isEmpty())
//                {
//                    warnings.append(tr("XML: Line %1; invalid mapper background color (%2)").arg(xml.lineNumber()).arg(colorString));
//                }

//                QString filename(xml.attributes().value("file").toString());
//                if (!filename.isEmpty())
//                {
//                    setMapFilename(filename);
//                }
//            }
        }
    }
}
