#include "maproom.h"
#include "mapexit.h"
#include "xmlexception.h"

MapRoom::MapRoom(QObject *parent) :
    QObject(parent),
    m_coordinates(3)
{
    m_id = -1;
}

void MapRoom::toXml(QXmlStreamWriter &xml)
{
    xml.writeStartElement("room");

    xml.writeStartElement("coord");
    xml.writeAttribute("x", QString::number(m_coordinates.at(0)));
    xml.writeAttribute("y", QString::number(m_coordinates.at(1)));
    xml.writeAttribute("z", QString::number(m_coordinates.at(2)));
    xml.writeEndElement();

    foreach (MapExit *exit, m_exits)
    {
        exit->toXml(xml);
    }

    xml.writeEndElement();
}

void MapRoom::fromXml(QXmlStreamReader &xml)
{
    QStringList warnings;

    bool valid = false;
    int id = xml.attributes().value("id").toInt(&valid);
    if (valid)
    {
        m_id = id;
    }
    else
    {
        warnings.append(tr("XML: Line %1; invalid room ID (%2)").arg(xml.lineNumber()).arg(xml.attributes().value("id").toString()));
    }

    int area = xml.attributes().value("area").toInt(&valid);
    if (valid)
    {
        m_areaId = area;
    }
    else
    {
        warnings.append(tr("XML: Line %1; invalid room area ID (%2)").arg(xml.lineNumber()).arg(xml.attributes().value("area").toString()));
    }

    int environment = xml.attributes().value("environment").toInt(&valid);
    if (valid)
    {
        m_environmentId = environment;
    }
    else
    {
        warnings.append(tr("XML: Line %1; invalid room environment (%2)").arg(xml.lineNumber()).arg(xml.attributes().value("environment").toString()));
    }

    QString title(xml.attributes().value("title").toString());
    if (!title.isEmpty())
    {
        m_title = title;
    }
    else
    {
        warnings.append(tr("XML: Line %1; missing or invalid room title").arg(xml.lineNumber()));
    }

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "room")
        {
            break;
        }

        if (xml.isStartElement())
        {
            if (xml.name() == "coord")
            {
                try
                {
                    readCoordinates(xml);
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                }
            }
            else if (xml.name() == "exit")
            {
                MapExit *exit = new MapExit(this);
                try
                {
                    exit->fromXml(xml);
                    exit->setComesFrom(m_id);
                    m_exits.append(exit);
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                    delete exit;
                }
            }
        }
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}

void MapRoom::readCoordinates(QXmlStreamReader &xml)
{
    QStringList warnings;

    bool valid = false;
    int x = xml.attributes().value("x").toInt(&valid);
    if (valid)
    {
        m_coordinates[0] = x;
    }
    else
    {
        warnings.append(tr("XML: Line %1; invalid X coordinate (%2)").arg(xml.lineNumber()).arg(xml.attributes().value("x").toString()));
    }

    int y = xml.attributes().value("y").toInt(&valid);
    if (valid)
    {
        m_coordinates[1] = y;
    }
    else
    {
        warnings.append(tr("XML: Line %1; invalid Y coordinate (%2)").arg(xml.lineNumber()).arg(xml.attributes().value("y").toString()));
    }

    int z = xml.attributes().value("z").toInt(&valid);
    if (valid)
    {
        m_coordinates[2] = z;
    }
    else
    {
        warnings.append(tr("XML: Line %1; invalid Z coordinate (%2)").arg(xml.lineNumber()).arg(xml.attributes().value("z").toString()));
    }

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "coord")
        {
            break;
        }
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}
