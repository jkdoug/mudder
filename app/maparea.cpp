#include "maparea.h"
#include "xmlexception.h"

#include <QStringList>

MapArea::MapArea(QObject *parent) :
    QObject(parent)
{
}

void MapArea::toXml(QXmlStreamWriter &xml)
{
    xml.writeStartElement("area");
    xml.writeAttribute("id", QString::number(id()));
    xml.writeAttribute("name", title());
    xml.writeEndElement();
}

void MapArea::fromXml(QXmlStreamReader &xml)
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
        warnings.append(tr("XML: Line %1; invalid area ID (%2)").arg(xml.lineNumber()).arg(xml.attributes().value("id").toString()));
    }

    QString name(xml.attributes().value("name").toString());
    if (name.isEmpty())
    {
        warnings.append(tr("XML: Line %1; invalid area name (%2)").arg(xml.lineNumber()).arg(name));
    }
    else
    {
        setTitle(name);
    }

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "area")
        {
            break;
        }
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}
