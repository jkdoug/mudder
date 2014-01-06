#ifndef MAPAREA_H
#define MAPAREA_H

#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class MapArea : public QObject
{
    Q_OBJECT
public:
    explicit MapArea(QObject *parent = 0);

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }
    QString title() const { return m_title; }
    void setTitle(const QString &title) { m_title = title; }

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml);

private:
    int m_id;
    QString m_title;
};

#endif // MAPAREA_H
