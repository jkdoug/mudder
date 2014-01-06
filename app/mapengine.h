#ifndef MAPENGINE_H
#define MAPENGINE_H

#include <QColor>
#include <QMap>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

#include "maparea.h"
#include "mapenvironment.h"
#include "maproom.h"

class MapEngine : public QObject
{
    Q_OBJECT
public:
    explicit MapEngine(QObject *parent = 0);

    bool initialize(const QString &filename);

    QString name() const { return m_database.databaseName(); }

    bool isOpen() const { return m_database.isOpen(); }
    bool isValid() const { return m_database.isValid(); }

    const QString & lastError() const { return m_error; }

    const QColor & background() const { return m_background; }
    void setBackground(const QColor &color) { m_background = color; }

    int currentRoom() const { return m_currentRoom; }
    void setCurrentRoom(int id) { m_currentRoom = id; }

    MapArea * lookupArea(int id) { return m_areas.value(id, 0); }
    MapEnvironment * lookupEnvironment(int id) { return m_environments.value(id, 0); }
    MapRoom * lookupRoom(int id) { return m_rooms.value(id, 0); }

    QList<MapRoom *> roomsByArea(int areaId);

    void importXml(QXmlStreamReader &xml);

protected:
    void readAreas(QXmlStreamReader &xml);
    void readEnvironments(QXmlStreamReader &xml);
    void readRooms(QXmlStreamReader &xml);

    void setError(const QString &error) { m_error = error; }
    void setError(const QSqlError &error);

private:
    QSqlDatabase m_database;
    QString m_error;

    QColor m_background;

    int m_currentRoom;

    QMap<int, MapArea *> m_areas;
    QMap<int, MapEnvironment *> m_environments;
    QMap<int, MapRoom *> m_rooms;
};

#endif // MAPENGINE_H
