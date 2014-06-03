/*
  Mudder, a cross-platform text gaming client

  Copyright (C) 2014 Jason Douglas
  jkdoug@gmail.com

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

*/


#include "profile.h"
#include "logging.h"
#include "coreapplication.h"
#include "group.h"
#include "timer.h"
#include "variable.h"
#include "profileitem.h"
#include "profileitemfactory.h"
#include "xmlerror.h"
#include <QDateTime>
#include <QDir>

const int ColumnCount = 1;
enum Column { Name };

Profile::Profile(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_root = new Group(this);
    m_activeGroup = m_root;
    connect(m_root, SIGNAL(modified(ProfileItem*)), SLOT(updateSetting()));
    connect(m_root, SIGNAL(settingAdded(ProfileItem*)), SLOT(addSetting(ProfileItem*)));
    connect(m_root, SIGNAL(settingRemoved(ProfileItem*)), SLOT(removeSetting(ProfileItem*)));

    m_options.insert("name", QString());

    m_options.insert("address", QString("localhost"));
    m_options.insert("port", 23);
    m_options.insert("autoConnect", false);
    m_options.insert("autoReconnect", false);

    m_options.insert("scriptFileName", QString());
    m_options.insert("scriptPrefix", QString());

    m_options.insert("commandSeparator", QString(";"));
    m_options.insert("clearCommandLine", false);
    m_options.insert("escapeClearsCommand", true);

    QFont inputFont("Courier New", 8);
    inputFont.setStyleHint(QFont::TypeWriter);
    inputFont.setStyleStrategy(QFont::PreferAntialias);
    m_options.insert("inputFont", inputFont);

    QFont outputFont("Consolas", 10);
    outputFont.setStyleHint(QFont::TypeWriter);
    outputFont.setStyleStrategy(QFont::PreferAntialias);
    m_options.insert("outputFont", outputFont);
}

template <class C>
C * Profile::findItem(const QString &name) const
{
    QString itemName(name.section('/', -1));
    qCDebug(MUDDER_PROFILE) << "itemName:" << itemName;
    if (itemName.isEmpty())
    {
        return 0;
    }

    QString groupName(name.left(name.length() - itemName.length() - 1));
    qCDebug(MUDDER_PROFILE) << "groupName:" << groupName;
    Group *group = findGroup(groupName);
    if (!group)
    {
        return 0;
    }

    foreach (ProfileItem *item, group->items())
    {
        C *sub = qobject_cast<C *>(item);
        if (sub && item->name().compare(itemName, Qt::CaseInsensitive) == 0)
        {
            return sub;
        }
    }

    return 0;
}

Group * Profile::findGroup(const QString &path) const
{
    QString cleanPath(QDir::cleanPath(path));
    if (cleanPath.isEmpty())
    {
        return activeGroup();
    }

    if (cleanPath.at(0) == '/')
    {
        return findGroup(cleanPath.mid(1).split('/', QString::SkipEmptyParts), rootGroup());
    }

    return findGroup(cleanPath.split('/', QString::SkipEmptyParts), activeGroup());
}

Group * Profile::findGroup(const QStringList &path, Group *parent) const
{
    if (path.isEmpty())
    {
        return parent;
    }

    QString name(path.first());
    QList<Group *> groups = parent->sortedGroups(false, false);
    foreach (Group *group, groups)
    {
        if (group->name().compare(name, Qt::CaseInsensitive) == 0)
        {
            return findGroup(path.mid(1), group);
        }
    }

    return 0;
}

Group * Profile::createGroup(const QString &path)
{
    QString cleanPath(QDir::cleanPath(path));
    if (cleanPath.isEmpty())
    {
        return activeGroup();
    }

    if (cleanPath.at(0) == '/')
    {
        return createGroup(cleanPath.mid(1).split('/', QString::SkipEmptyParts), rootGroup());
    }

    return createGroup(cleanPath.split('/', QString::SkipEmptyParts), activeGroup());
}

Group * Profile::createGroup(const QStringList &path, Group *parent)
{
    if (path.isEmpty())
    {
        return parent;
    }

    QString name(path.first());
    QList<Group *> groups = parent->sortedGroups(false, false);
    foreach (Group *group, groups)
    {
        if (group->name().compare(name, Qt::CaseInsensitive) == 0)
        {
            return createGroup(path.mid(1), group);
        }
    }

    if (!Group::validateName(name))
    {
        return 0;
    }

    Group *created = new Group;
    created->setName(name);
    parent->addItem(created);

    return createGroup(path.mid(1), created);
}

void Profile::setActiveGroup(Group *group)
{
    m_activeGroup = group?group:m_root;
}

QVariant Profile::getVariable(const QString &name)
{
    QString varName(name.section('/', -1));
    qCDebug(MUDDER_PROFILE) << "GetVariable varName:" << varName;
    if (varName.isEmpty())
    {
        return QVariant();
    }

    QString groupName(name.left(qMax(name.length() - varName.length(), 0)));
    qCDebug(MUDDER_PROFILE) << "GetVariable groupName:" << groupName;
    Group *group = findGroup(groupName);
    if (!group)
    {
        return QVariant();
    }

    foreach (Variable *var, group->sortedVariables(groupName.isEmpty()))
    {
        if (var->name().compare(varName, Qt::CaseInsensitive) == 0)
        {
            qCDebug(MUDDER_PROFILE) << "GetVariable found:" << var->fullName();
            return var->contents();
        }
    }

    return QVariant();
}

bool Profile::setVariable(const QString &name, const QVariant &val)
{
    if (!val.isValid())
    {
        return deleteVariable(name);
    }

    QString varName(name.section('/', -1));
    qCDebug(MUDDER_PROFILE) << "SetVariable varName:" << varName;
    if (varName.isEmpty())
    {
        return false;
    }

    QString groupName(name.left(qMax(name.length() - varName.length(), 0)));
    qCDebug(MUDDER_PROFILE) << "SetVariable groupName:" << groupName;
    Group *group = createGroup(groupName);
    if (!group)
    {
        return false;
    }

    foreach (Variable *var, group->sortedVariables(groupName.isEmpty()))
    {
        if (var->name().compare(varName, Qt::CaseInsensitive) == 0)
        {
            qCDebug(MUDDER_PROFILE) << "SetVariable found:" << var->fullName();
            var->setContents(val);
            return true;
        }
    }

    if (!Variable::validateName(varName))
    {
        return false;
    }

    Variable *created = new Variable;
    created->setName(varName);
    created->setContents(val);
    group->addItem(created);

    qCDebug(MUDDER_PROFILE) << "SetVariable created:" << created->fullName();

    return true;
}

bool Profile::deleteVariable(const QString &name)
{
    QString varName(name.section('/', -1));
    qCDebug(MUDDER_PROFILE) << "DeleteVariable varName:" << varName;
    if (varName.isEmpty())
    {
        return false;
    }

    QString groupName(name.left(qMax(name.length() - varName.length(), 0)));
    qCDebug(MUDDER_PROFILE) << "DeleteVariable groupName:" << groupName;
    Group *group = findGroup(groupName);
    if (!group)
    {
        return false;
    }

    foreach (Variable *var, group->sortedVariables(groupName.isEmpty()))
    {
        if (var->name().compare(varName, Qt::CaseInsensitive) == 0)
        {
            return group->removeItem(var);
        }
    }

    return false;
}

void Profile::setOptions(const QVariantMap &options)
{
    foreach (QString key, options.keys())
    {
        changeOption(key, options.value(key));
    }
}

void Profile::toXml(QXmlStreamWriter &xml)
{
    xml.writeStartDocument();

    xml.writeStartElement("mudder");
    xml.writeAttribute("version", CoreApplication::applicationVersion());
    xml.writeAttribute("saved", QDateTime::currentDateTime().toString());

    xml.writeStartElement("profile");
    xml.writeAttribute("name", name());

    xml.writeStartElement("host");
    xml.writeAttribute("connect", autoConnect()?"y":"n");
    xml.writeAttribute("reconnect", autoReconnect()?"y":"n");
    xml.writeAttribute("address", address());
    xml.writeAttribute("port", QString::number(port()));
    xml.writeEndElement();

    xml.writeStartElement("commandLine");
    xml.writeAttribute("clear", clearCommandLine()?"y":"n");
    xml.writeAttribute("escape", escapeClearsCommand()?"y":"n");
    xml.writeAttribute("separator", commandSeparator());
    xml.writeEndElement();

    xml.writeStartElement("scripting");
    xml.writeAttribute("file", scriptFileName());
    xml.writeAttribute("prefix", scriptPrefix());
    xml.writeEndElement();

    xml.writeStartElement("display");

    xml.writeStartElement("inputFont");
    xml.writeAttribute("family", inputFont().family());
    xml.writeAttribute("size", QString::number(inputFont().pointSize()));
    xml.writeAttribute("antialias", (inputFont().styleStrategy() == QFont::PreferAntialias)?"y":"n");
    xml.writeEndElement();

    xml.writeStartElement("outputFont");
    xml.writeAttribute("family", outputFont().family());
    xml.writeAttribute("size", QString::number(outputFont().pointSize()));
    xml.writeAttribute("antialias", (outputFont().styleStrategy() == QFont::PreferAntialias)?"y":"n");
    xml.writeEndElement();

    xml.writeEndElement();  // display

    xml.writeEndElement();  // profile

    xml.writeStartElement("settings");
    m_root->toXml(xml);
    xml.writeEndElement();

    xml.writeEndDocument();
}

void Profile::fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    while (!xml.atEnd())
    {
        xml.readNext();

        QString name(xml.name().toString());
        if (xml.isStartElement())
        {
            ProfileItem *item = ProfileItemFactory::create(name, activeGroup());
            if (item)
            {
                item->fromXml(xml, errors);
                activeGroup()->addItem(item);
            }

            if (name == "group")
            {
                setActiveGroup(qobject_cast<Group *>(item));
            }
            else if (name == "timer")
            {
                Timer *timer = qobject_cast<Timer *>(item);
                if (timer)
                {
                    connect(timer, SIGNAL(fired(Timer*)), SLOT(handleTimer(Timer*)));
                }
            }
            else if (name == "profile")
            {
                readProfile(xml, errors);
            }
        }
        else if (xml.isEndElement())
        {
            if (name == "group")
            {
                setActiveGroup(activeGroup()->group());
            }
        }
    }
}

Qt::ItemFlags Profile::flags(const QModelIndex &index) const
{
    Qt::ItemFlags theFlags = QAbstractItemModel::flags(index);

    if (index.isValid())
    {
        theFlags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled;
    }

    return theFlags;
}

QVariant Profile::data(const QModelIndex &index, int role) const
{
    if (!m_root || !index.isValid() || index.column() < 0 || index.column() >= ColumnCount)
    {
        return QVariant();
    }

    ProfileItem * item = itemForIndex(index);
    if (item)
    {
        if (role == Qt::DisplayRole || role == Qt::EditRole)
        {
            switch (index.column())
            {
            case Name:
                return item->name();

            default:
                Q_ASSERT(false);
            }
        }

        if (role == Qt::DecorationRole && index.column() == Name)
        {
            return item->icon();
        }
    }

    return QVariant();
}

QVariant Profile::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
    {
        switch (section)
        {
        case Name:
            return tr("Name");

        default:
            Q_ASSERT(false);
        }
    }

    return QVariant();
}

int Profile::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid() && parent.column() != 0)
    {
        return 0;
    }

    Group * parentItem = qobject_cast<Group*>(itemForIndex(parent));
    return parentItem ? parentItem->items().count() : 0;
}

int Profile::columnCount(const QModelIndex &parent) const
{
    return parent.isValid() && parent.column() != 0 ? 0 : ColumnCount;
}


QModelIndex Profile::index(int row, int column,
                             const QModelIndex &parent) const
{
    if (!m_root || row < 0 || column < 0 || column >= ColumnCount ||
        (parent.isValid() && parent.column() != 0))
    {
        return QModelIndex();
    }

    Group *parentItem = qobject_cast<Group*>(itemForIndex(parent));
    Q_ASSERT(parentItem);

    ProfileItem *item = parentItem->items().at(row);
    if (!item)
    {
        return QModelIndex();
    }

    return createIndex(row, column, item);
}

QModelIndex Profile::parent(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }

    ProfileItem *item = itemForIndex(index);
    if (item)
    {
        Group *parentItem = item->group();
        if (parentItem)
        {
            if (parentItem == m_root)
            {
                return QModelIndex();
            }

            Group *grandparentItem = parentItem->group();
            if (grandparentItem)
            {
                int row = grandparentItem->items().indexOf(parentItem);
                return createIndex(row, 0, parentItem);
            }
        }
    }

    return QModelIndex();
}

ProfileItem * Profile::itemForIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        ProfileItem * item = static_cast<ProfileItem*>(index.internalPointer());
        if (item)
        {
            return item;
        }
    }

    return m_root;
}

QStringList Profile::pathForIndex(const QModelIndex &index) const
{
    QStringList path;
    QModelIndex thisIndex(index);
    while (thisIndex.isValid())
    {
        path.prepend(data(thisIndex).toString());
        thisIndex = thisIndex.parent();
    }
    return path;
}

QModelIndex Profile::indexForPath(const QStringList &path) const
{
    return indexForPath(QModelIndex(), path);
}

QModelIndex Profile::indexForPath(const QModelIndex &parent, const QStringList &path) const
{
    if (path.isEmpty())
    {
        return QModelIndex();
    }

    for (int row = 0; row < rowCount(parent); row++)
    {
        QModelIndex thisIndex(index(row, 0, parent));
        if (data(thisIndex).toString() == path.first())
        {
            if (path.count() == 1)
            {
                return thisIndex;
            }

            thisIndex = indexForPath(thisIndex, path.mid(1));
            if (thisIndex.isValid())
            {
                return thisIndex;
            }
        }
    }

    return QModelIndex();
}

void Profile::changeOption(const QString &key, const QVariant &val)
{
    if (m_options.value(key) != val)
    {
        m_options.insert(key, val);
        emit optionChanged(key, val);
    }
}

void Profile::updateSetting()
{
    emit settingsChanged();
}

void Profile::addSetting(ProfileItem *item)
{
    emit settingAdded(item);
}

void Profile::removeSetting(ProfileItem *item)
{
    emit settingRemoved(item);
}

void Profile::handleTimer(Timer *timer)
{
    if (!timer)
    {
        qCDebug(MUDDER_PROFILE) << "Invalid timer object just fired.";
        return;
    }

    emit timerFired(timer);
}

void Profile::readProfile(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    setName(xml.attributes().value("name").toString());

    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "profile")
        {
            break;
        }

        if (xml.isStartElement())
        {
            if (xml.name() == "host")
            {
                setAutoConnect(xml.attributes().value("connect").compare("y", Qt::CaseInsensitive) == 0);
                setAutoReconnect(xml.attributes().value("reconnect").compare("y", Qt::CaseInsensitive) == 0);

                QString address(xml.attributes().value("address").toString());
                if (address.isEmpty())
                {
                    errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("missing or empty host address"));
                }
                setAddress(address);

                bool valid = true;
                int port = xml.attributes().value("port").toString().toInt(&valid);
                if (!valid)
                {
                    port = 23;
                    errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("missing or invalid host port, defaulting to 23"));
                }
                setPort(port);
            }
            else if (xml.name() == "commandLine")
            {
                QString sep(xml.attributes().value("separator").toString());
                if (!sep.isEmpty())
                {
                    if (sep.length() > 1)
                    {
                        errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("command separator should be a single character"));
                    }
                    setCommandSeparator(sep.at(0));
                }

                setClearCommandLine(xml.attributes().value("clear").compare("n", Qt::CaseInsensitive) != 0);
                setEscapeClearsCommand(xml.attributes().value("escape").compare("n", Qt::CaseInsensitive) != 0);
            }
            else if (xml.name() == "scripting")
            {
                QString prefix(xml.attributes().value("prefix").toString());
                if (!prefix.isEmpty())
                {
                    setScriptPrefix(prefix);
                }

                QString script(xml.attributes().value("file").toString());
                if (!script.isEmpty())
                {
                    setScriptFileName(script);
                }
            }
            else if (xml.name() == "display")
            {
                readDisplay(xml, errors);
            }
        }
    }
}

void Profile::readDisplay(QXmlStreamReader &xml, QList<XmlError *> &errors)
{
    while (!xml.atEnd())
    {
        xml.readNext();

        if (xml.isEndElement() && xml.name() == "display")
        {
            break;
        }

        if (xml.isStartElement())
        {
            if (xml.name() == "inputFont")
            {
                QFont defaultFont(m_options.value("inputFont").value<QFont>());

                QString family(xml.attributes().value("family").toString());
                if (family.isEmpty())
                {
                    errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("missing or invalid input font family name"));
                    family = defaultFont.family();
                }

                bool valid = true;
                int size = xml.attributes().value("size").toString().toInt(&valid);
                if (!valid)
                {
                    errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("missing or invalid input font point size"));
                    size = defaultFont.pointSize();
                }
                size = qBound(6, size, 20);

                bool antiAlias = defaultFont.styleStrategy() & QFont::PreferAntialias;
                if (!xml.attributes().value("antialias").isEmpty())
                {
                    antiAlias = xml.attributes().value("antialias").compare("n", Qt::CaseInsensitive) != 0;
                }

                QFont font(family, size);
                font.setStyleHint(QFont::TypeWriter);
                font.setStyleStrategy(antiAlias?QFont::PreferAntialias:QFont::NoAntialias);
                setInputFont(font);
            }
            else if (xml.name() == "outputFont")
            {
                QFont defaultFont(m_options.value("outputFont").value<QFont>());

                QString family(xml.attributes().value("family").toString());
                if (family.isEmpty())
                {
                    errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("missing or invalid output font family name"));
                    family = defaultFont.family();
                }

                bool valid = true;
                int size = xml.attributes().value("size").toString().toInt(&valid);
                if (!valid)
                {
                    errors << new XmlError(xml.lineNumber(), xml.columnNumber(), tr("missing or invalid output font point size"));
                    size = defaultFont.pointSize();
                }
                size = qBound(6, size, 20);

                bool antiAlias = defaultFont.styleStrategy() & QFont::PreferAntialias;
                if (!xml.attributes().value("antialias").isEmpty())
                {
                    antiAlias = xml.attributes().value("antialias").compare("n", Qt::CaseInsensitive) != 0;
                }

                QFont font(family, size);
                font.setStyleHint(QFont::TypeWriter);
                font.setStyleStrategy(antiAlias?QFont::PreferAntialias:QFont::NoAntialias);
                setOutputFont(font);
            }
        }
    }
}
