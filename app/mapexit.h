#ifndef MAPEXIT_H
#define MAPEXIT_H

#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class MapExit : public QObject
{
    Q_OBJECT
public:
    explicit MapExit(QObject *parent = 0);

    enum Direction
    {
        South,
        Southwest,
        West,
        Northwest,
        North,
        Northeast,
        East,
        Southeast,
        In,
        Out,
        Up,
        Down,
        Other,
        Invalid
    };

    Direction direction() const { return m_direction; }
    void setDirection(Direction dir) { m_direction = dir; }
    int leadsTo() const { return m_to; }
    int comesFrom() const { return m_from; }
    void setLeadsTo(int room) { m_to = room; }
    void setComesFrom(int room) { m_from = room; }
    QString command() const;
    void setCommand(const QString &cmd) { m_command = cmd; }
    bool isHidden() const { return m_hidden; }
    void setHidden(bool flag = true) { m_hidden = flag; }

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml);

    static QString command(Direction dir);
    static Direction direction(QString command);

private:
    Direction m_direction;
    int m_to;
    int m_from;
    bool m_hidden;
    QString m_command;
};

#endif // MAPEXIT_H
