#include "mapenvironment.h"
#include "xmlexception.h"

#include <QColor>
#include <QStringList>

MapEnvironment::MapEnvironment(QObject *parent) :
    QObject(parent)
{
}

void MapEnvironment::toXml(QXmlStreamWriter &xml)
{
    xml.writeStartElement("environment");
    xml.writeAttribute("id", QString::number(id()));
    xml.writeAttribute("name", name());
    xml.writeAttribute("htmlcolor", color().name());
    xml.writeEndElement();
}

void MapEnvironment::fromXml(QXmlStreamReader &xml)
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
        warnings.append(tr("XML: Line %1; invalid environment ID (%2)").arg(xml.lineNumber()).arg(xml.attributes().value("id").toString()));
    }

    QString name(xml.attributes().value("name").toString());
    if (!name.isEmpty())
    {
        setName(name);
    }
    else
    {
        warnings.append(tr("XML: Line %1; invalid environment name (%2)").arg(xml.lineNumber()).arg(name));
    }

    QString colorString(xml.attributes().value("htmlcolor").toString());
    QColor bgColor(colorString);
    if (bgColor.isValid())
    {
        setColor(bgColor);
    }
    else
    {
        warnings.append(tr("XML: Line %1; invalid environment color (%2)").arg(xml.lineNumber()).arg(colorString));
    }

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "environment")
        {
            break;
        }
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}
