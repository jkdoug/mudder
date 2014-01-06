#ifndef PROFILEITEM_H
#define PROFILEITEM_H

#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QObject>

class Group;

class ProfileItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(bool enabled READ enabledFlag WRITE enable)
    Q_PROPERTY(int sequence READ sequence WRITE setSequence)

public:
    explicit ProfileItem(QObject *parent = 0);
    ProfileItem(const ProfileItem &rhs, QObject *parent = 0);

    ProfileItem & operator =(const ProfileItem &rhs);

    bool operator ==(const ProfileItem &rhs);
    bool operator !=(const ProfileItem &rhs);

    bool operator <(const ProfileItem &rhs);
    static bool lessSequence(const ProfileItem *lhs, const ProfileItem *rhs);

    virtual const QString & name() const { return m_name; }
    void setName(const QString &name) { m_name = name; }
    Group *group() const;
    bool enabled() const;
    bool enabledFlag() const { return m_enabled; }
    virtual void enable(bool flag = true) { m_enabled = flag; }
    virtual int sequence() const { return m_sequence; }
    void setSequence(int sequence) { m_sequence = sequence; }

    virtual QString value() const { return QString(); }

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml);

protected:
    void clone(const ProfileItem &rhs);

private:
    QString m_name;

    bool m_enabled;

    int m_sequence;
};

#endif // PROFILEITEM_H
