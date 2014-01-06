#ifndef MAPENVIRONMENT_H
#define MAPENVIRONMENT_H

#include <QColor>
#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class MapEnvironment : public QObject
{
    Q_OBJECT
public:
    explicit MapEnvironment(QObject *parent = 0);

    int id() const { return m_id; }
    void setId(int id) { m_id = id; }
    QString name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    QColor color() const { return m_color; }
    void setColor(const QColor &color) { m_color = color; }

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml);

private:
    int m_id;
    QString m_name;
    QColor m_color;
};

#endif // MAPENVIRONMENT_H
