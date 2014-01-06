#include "mapexit.h"
#include "xmlexception.h"

#include <QStringList>

MapExit::MapExit(QObject *parent) :
    QObject(parent)
{
}

QString MapExit::command() const
{
    if (!m_command.isEmpty())
    {
        return m_command;
    }

    return command(direction());
}

void MapExit::toXml(QXmlStreamWriter &xml)
{
    xml.writeStartElement("exit");
    xml.writeAttribute("direction", command());
    xml.writeAttribute("target", QString::number(leadsTo()));
    if (isHidden())
    {
        xml.writeAttribute("hidden", "1");
    }
    xml.writeEndElement();
}

void MapExit::fromXml(QXmlStreamReader &xml)
{
    QStringList warnings;

    bool valid = false;
    int target = xml.attributes().value("target").toInt(&valid);
    if (valid)
    {
        setLeadsTo(target);
    }
    else
    {
        warnings.append(tr("XML: Line %1; invalid exit target (%2)").arg(xml.lineNumber()).arg(xml.attributes().value("target").toString()));
    }

    QString dir(xml.attributes().value("direction").toString());
    setDirection(MapExit::direction(dir));
    if (direction() == MapExit::Invalid)
    {
        warnings.append(tr("XML: Line %1; invalid exit direction (%2)").arg(xml.lineNumber()).arg(dir));
    }

    setHidden(xml.attributes().value("hidden").toString().compare("1") == 0);

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "exit")
        {
            break;
        }
    }

    if (!warnings.isEmpty())
    {
        throw new XmlException(warnings);
    }
}

QString MapExit::command(MapExit::Direction dir)
{
    switch (dir)
    {
    case South:
        return "s";

    case Southwest:
        return "sw";

    case West:
        return "w";

    case Northwest:
        return "nw";

    case North:
        return "n";

    case Northeast:
        return "ne";

    case East:
        return "e";

    case Southeast:
        return "se";

    case In:
        return "in";

    case Out:
        return "out";

    case Up:
        return "u";

    case Down:
        return "d";

    default:
        return QString();
    }
}

MapExit::Direction MapExit::direction(QString command)
{
    QString cmd(command.toLower());
    if (cmd == "s" || cmd == "south")
    {
        return South;
    }
    if (cmd == "sw" || cmd == "southwest")
    {
        return Southwest;
    }
    if (cmd == "w" || cmd == "west")
    {
        return West;
    }
    if (cmd == "nw" || cmd == "northwest")
    {
        return Northwest;
    }
    if (cmd == "n" || cmd== "north")
    {
        return North;
    }
    if (cmd == "ne" || cmd == "northeast")
    {
        return Northeast;
    }
    if (cmd == "e" || cmd == "east")
    {
        return East;
    }
    if (cmd == "se" || cmd == "southeast")
    {
        return Southeast;
    }
    if (cmd == "in")
    {
        return In;
    }
    if (cmd == "out")
    {
        return Out;
    }
    if (cmd == "u" || cmd == "up")
    {
        return Up;
    }
    if (cmd == "d" || cmd == "down")
    {
        return Down;
    }
    if (!cmd.isEmpty())
    {
        return Other;
    }

    return Invalid;
}
