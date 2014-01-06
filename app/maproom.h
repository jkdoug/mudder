#ifndef MAPROOM_H
#define MAPROOM_H

#include <QList>
#include <QObject>
#include <QString>
#include <QVariantMap>
#include <QVector>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class MapExit;

class MapRoom : public QObject
{
    Q_OBJECT
public:
    explicit MapRoom(QObject *parent = 0);

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }
    int areaId() const { return m_areaId; }
    void setAreaId(int id) { m_areaId = id; }
    int environmentId() const { return m_environmentId; }
    void setEnvironmentId(int id) { m_environmentId = id; }

    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }
    QString description() const { return m_description; }
    void setDescription(const QString &description) { m_description = description; }

    const QVector<int> & coordinates() const { return m_coordinates; }
    int x() const { return m_coordinates.at(0); }
    int y() const { return m_coordinates.at(1); }
    int z() const { return m_coordinates.at(2); }

    QVariant userDatum(const QString &name) const { m_userData.value(name); }
    void setUserDatum(const QString &name, const QVariant &value) { m_userData.insert(name, value); }

    QList<MapExit *> exits() const { return m_exits; }

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml);

private:
    void readCoordinates(QXmlStreamReader &xml);

    int m_id;
    int m_areaId;
    int m_environmentId;

    QString m_title;
    QString m_description;

    QVector<int> m_coordinates;

    QVariantMap m_userData;

    QList<MapExit *> m_exits;
};

#endif // MAPROOM_H
