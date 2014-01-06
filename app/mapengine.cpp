#include "mapengine.h"
#include "xmlexception.h"
#include <QDebug>

MapEngine::MapEngine(QObject *parent) :
    QObject(parent)
{
    m_background = QColor("#999999");
}

bool MapEngine::initialize(const QString &filename)
{
    m_error.clear();

    m_database = QSqlDatabase::addDatabase("QSQLITE", "mapper");
    m_database.setDatabaseName(filename);

    if (!m_database.open())
    {
        setError(m_database.lastError());
        return false;
    }

    return true;
}

QList<MapRoom *> MapEngine::roomsByArea(int areaId)
{
    QList<MapRoom *> rooms;
    foreach (MapRoom *room, m_rooms)
    {
        if (room->areaId() == areaId)
        {
            rooms.append(room);
        }
    }

    return rooms;
}

void MapEngine::importXml(QXmlStreamReader &xml)
{
    QStringList warnings;

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isStartElement())
        {
            if (xml.name() == "areas")
            {
                try
                {
                    readAreas(xml);
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                }
            }
            else if (xml.name() == "environments")
            {
                try
                {
                    readEnvironments(xml);
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                }
            }
            else if (xml.name() == "rooms")
            {
                try
                {
                    readRooms(xml);
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                }
            }
        }
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }

    qDebug() << "areas:" << m_areas.size() << "environments:" << m_environments.size() << "rooms:" << m_rooms.size();
}

void MapEngine::readAreas(QXmlStreamReader &xml)
{
    QStringList warnings;

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "areas")
        {
            break;
        }

        if (xml.isStartElement())
        {
            if (xml.name() == "area")
            {
                int lineNumber = xml.lineNumber();
                MapArea *area = new MapArea(this);
                try
                {
                    area->fromXml(xml);
                    if (m_areas.contains(area->id()))
                    {
                        warnings.append(tr("XML: Line %1; duplicate area ID (%2)").arg(lineNumber).arg(area->id()));
                    }
                    else
                    {
                        m_areas.insert(area->id(), area);
                    }
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                    delete area;
                }
            }
        }
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}

void MapEngine::readEnvironments(QXmlStreamReader &xml)
{
    QStringList warnings;

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "environments")
        {
            break;
        }

        if (xml.isStartElement())
        {
            if (xml.name() == "environment")
            {
                int lineNumber = xml.lineNumber();
                MapEnvironment *environment = new MapEnvironment(this);
                try
                {
                    environment->fromXml(xml);
                    if (m_environments.contains(environment->id()))
                    {
                        warnings.append(tr("XML: Line %1; duplicate environment ID (%2)").arg(lineNumber).arg(environment->id()));
                    }
                    else
                    {
                        m_environments.insert(environment->id(), environment);
                    }
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                    delete environment;
                }
            }
        }
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}

void MapEngine::readRooms(QXmlStreamReader &xml)
{
    QStringList warnings;

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "rooms")
        {
            break;
        }

        if (xml.isStartElement())
        {
            if (xml.name() == "room")
            {
                int lineNumber = xml.lineNumber();
                MapRoom *room = new MapRoom(this);
                try
                {
                    room->fromXml(xml);
                    if (m_rooms.contains(room->id()))
                    {
                        warnings.append(tr("XML: Line %1; duplicate room ID (%2)").arg(lineNumber).arg(room->id()));
                    }
                    else
                    {
                        m_rooms.insert(room->id(), room);
                    }
                }
                catch (XmlException *xe)
                {
                    warnings.append(xe->warnings());

                    delete xe;
                    delete room;
                }
            }
        }
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}

void MapEngine::setError(const QSqlError &error)
{
    m_error = tr("Database Error %1: %2").arg(error.type()).arg(error.text());
}
