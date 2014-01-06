#include "profileitem.h"
#include "group.h"
#include "xmlexception.h"
#include <QDebug>
#include <QStringList>

ProfileItem::ProfileItem(QObject *parent) :
    QObject(parent)
{
    m_enabled = true;
    m_sequence = 1000;
}

ProfileItem::ProfileItem(const ProfileItem &rhs, QObject *parent) :
    QObject(parent)
{
    clone(rhs);
}

ProfileItem & ProfileItem::operator =(const ProfileItem &rhs)
{
    clone(rhs);

    return *this;
}

bool ProfileItem::operator ==(const ProfileItem &rhs)
{
    if (m_sequence != rhs.m_sequence)
    {
        return false;
    }

    if (m_name != rhs.m_name)
    {
        return false;
    }

    if (m_enabled != rhs.m_enabled)
    {
        return false;
    }

    return true;
}

bool ProfileItem::operator !=(const ProfileItem &rhs)
{
    return !(*this == rhs);
}

bool ProfileItem::operator <(const ProfileItem &rhs)
{
    return sequence() < rhs.sequence();
}

bool ProfileItem::lessSequence(const ProfileItem *lhs, const ProfileItem *rhs)
{
    return lhs->sequence() < rhs->sequence();
}

void ProfileItem::clone(const ProfileItem &rhs)
{
    if (this == &rhs)
    {
        return;
    }

    m_name = rhs.m_name;

    m_enabled = rhs.m_enabled;

    m_sequence = rhs.m_sequence;
}

QString ProfileItem::fullName() const
{
    if (group() != 0)
    {
        return QString("%1/%2").arg(group()->path()).arg(name());
    }

    return name();
}

Group * ProfileItem::group() const
{
    return qobject_cast<Group *>(parent());
}

bool ProfileItem::enabled() const
{
    return m_enabled && (group() == 0 || group()->enabled());
}

void ProfileItem::toXml(QXmlStreamWriter &xml)
{
    xml.writeAttribute("name", name());
    if (!enabledFlag())
    {
        xml.writeAttribute("enabled", "n");
    }
    if (sequence() > 0)
    {
        xml.writeAttribute("sequence", QString::number(sequence()));
    }
}

void ProfileItem::fromXml(QXmlStreamReader &xml)
{
    QStringList warnings;

    QString name(xml.attributes().value("name").toString());
    if (name.isEmpty())
    {
        warnings.append(tr("XML: Line %1; invalid or missing 'name' attribute").arg(xml.lineNumber()));
    }
    setName(name);

    enable(xml.attributes().value("enabled").compare("n", Qt::CaseInsensitive) != 0);

    bool valid = true;
    int sequence = xml.attributes().value("sequence").toString().toInt(&valid);
    if (!valid)
    {
        sequence = 1000;
    }
    setSequence(qBound(1, sequence, 100000));

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}
