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
#include <QAbstractMessageHandler>
#include <QClipboard>
#include <QDateTime>
#include <QDir>
#include <QXmlSchema>
#include <QXmlSchemaValidator>

const int ColumnCount = 1;
enum Column { Name };

Profile::Profile(QObject *parent) :
    QAbstractItemModel(parent)
{
    m_root = new Group(this);
    m_activeGroup = m_root;
    connect(m_root, SIGNAL(modified(ProfileItem*)), SIGNAL(settingsChanged()));

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

    m_options.insert("scrollbackLines", 1000);
}

template <class C>
C * Profile::findItem(const QString &name, Group *parent) const
{
    if (name.startsWith('/'))
    {
        parent = rootGroup();
    }
    else if (!parent)
    {
        parent = activeGroup();
    }

    QStringList names(parent->path() + QDir::cleanPath(name).split('/', QString::SkipEmptyParts));

    QModelIndex itemIndex(indexForPath(names));
    if (!itemIndex.isValid())
    {
        qCDebug(MUDDER_PROFILE) << "Item not found:" << names;
        return 0;
    }

    return qobject_cast<C*>(itemForIndex(itemIndex));
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
        return parent?parent:activeGroup();
    }

    if (path.first().isEmpty())
    {
        return createGroup(path.mid(1), rootGroup());
    }
    else if (!parent)
    {
        return createGroup(path, activeGroup());
    }

    QString name(path.first());
    QList<Group *> groups = parent->sortedGroups(false, false);
    foreach (Group *group, groups)
    {
        if (group->name().compare(name) == 0)
        {
            return createGroup(path.mid(1), group);
        }
    }

    if (!Group::validateName(name))
    {
        return 0;
    }

    int row = parent->itemCount();
    beginInsertRows(indexForPath(parent->path()), row, row);
    Group *group = parent->createGroup(name);
    endInsertRows();

    emit settingsChanged();

    return createGroup(path.mid(1), group);
}

void Profile::setActiveGroup(Group *group)
{
    m_activeGroup = group?group:m_root;
}

QVariant Profile::getVariable(const QString &name)
{
    Variable *variable = findItem<Variable>(name);
    if (variable)
    {
        return variable->contents();
    }

    return QVariant();
}

bool Profile::setVariable(const QString &name, const QVariant &val)
{
    if (!val.isValid())
    {
        return deleteVariable(name);
    }

    Variable *variable = findItem<Variable>(name);
    if (variable)
    {
        variable->setContents(val);
        return true;
    }

    QStringList path(QDir::cleanPath(name).split('/'));
    if (path.isEmpty())
    {
        return false;
    }

    QString varName(path.takeLast());
    if (varName.isEmpty())
    {
        return false;
    }

    Group *group = createGroup(path);
    if (!group)
    {
        return false;
    }

    foreach (Variable *var, group->sortedVariables(false))
    {
        if (var->name().compare(varName) == 0)
        {
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

    int row = group->itemCount();
    beginInsertRows(indexForPath(group->path()), row, row);
    row = group->addItem(created);
    endInsertRows();

    emit settingsChanged();

    return true;
}

bool Profile::deleteVariable(const QString &name)
{
    Variable *variable = findItem<Variable>(name);
    if (variable)
    {
        Group *group = variable->group();
        if (!group)
        {
            return false;
        }

        return removeRow(group->itemIndex(variable), indexForPath(group->path()));
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
    xml.writeAttribute("scrollback", QString::number(scrollbackLines()));

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
    beginResetModel();
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
    endResetModel();
}

class ProfileValidator : public QAbstractMessageHandler
{
public:
    explicit ProfileValidator(QObject *parent = 0) : QAbstractMessageHandler(parent) {}
    ~ProfileValidator() { qDeleteAll(errors); }

    QList<XmlError *> errors;

protected:
    virtual void handleMessage(QtMsgType type, const QString &description, const QUrl &identifier, const QSourceLocation &sourceLocation)
    {
        Q_UNUSED(type)
        Q_UNUSED(identifier)

        errors << new XmlError(sourceLocation.line(), sourceLocation.column(), description);
    }
};

bool Profile::validateXml(const QString &text, QList<XmlError *> *errors)
{
    QFile res(":/schema/profile");
    res.open(QFile::ReadOnly | QFile::Text);
    QTextStream ts(&res);
    QString content(ts.readAll());

    QXmlSchema schema;
    schema.load(content.toUtf8());
    Q_ASSERT(schema.isValid());

    QXmlSchemaValidator validator(schema);
    ProfileValidator *pv = new ProfileValidator;
    validator.setMessageHandler(pv);
    if (!validator.validate(text.toUtf8()))
    {
        if (errors)
        {
            foreach (XmlError *err, pv->errors)
            {
                errors->append(new XmlError(*err));
            }
        }

        delete pv;
        return false;
    }

    delete pv;
    return true;
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
            Executable *executable = qobject_cast<Executable*>(item);
            switch (index.column())
            {
            case Name:
                if (executable && executable->failed())
                {
                    return QString("<p style=\"color:red;\">%1").arg(item->name());
                }
                else if (item->enabled())
                {
                    return item->name();
                }
                else
                {
                    return QString("<p style=\"color:dimgray;\">%1").arg(item->name());
                }

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
    return parentItem ? parentItem->itemCount() : 0;
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

    if (row >= parentItem->itemCount())
    {
        return QModelIndex();
    }

    ProfileItem *item = parentItem->item(row);
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
                int row = grandparentItem->itemIndex(parentItem);
                return createIndex(row, 0, parentItem);
            }
        }
    }

    return QModelIndex();
}

bool Profile::removeRows(int row, int count, const QModelIndex &parent)
{
    if (count < 1)
    {
        return false;
    }

    Group *group = qobject_cast<Group*>(itemForIndex(parent));
    Q_ASSERT(group);
    if (!group)
    {
        return false;
    }

    count = qMin(count, group->itemCount() - row);

    beginRemoveRows(parent, row, row + count - 1);
    for (int n = row; n < row + count; n++)
    {
        group->removeItem(n);
    }
    endRemoveRows();

    emit settingsChanged();

    return true;
}

ProfileItem * Profile::itemForIndex(const QModelIndex &index) const
{
    if (!index.isValid())
    {
        return m_root;
    }

    ProfileItem * item = static_cast<ProfileItem*>(index.internalPointer());
    return item;
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
    if (path.isEmpty() || path.join('/') == "/")
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

QModelIndex Profile::deleteItem(const QModelIndex &index)
{
    ProfileItem *item = itemForIndex(index);
    Q_ASSERT(item);

    Group *parentItem = item->group();
    Q_ASSERT(parentItem);

    int row = parentItem->itemIndex(item);
    Q_ASSERT(row == index.row());

    beginRemoveRows(index.parent(), row, row);
    parentItem->removeItem(row);
    endRemoveRows();

    delete item;

    emit settingsChanged();

    if (row > 0)
    {
        return createIndex(row - 1, 0, parentItem->item(row));
    }

    if (parentItem != m_root)
    {
        Group *grandparentItem = parentItem->group();
        Q_ASSERT(grandparentItem);
        return createIndex(grandparentItem->itemIndex(parentItem), 0, parentItem);
    }

    return QModelIndex();
}

QModelIndex Profile::cutItem(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return index;
    }

    copyItem(index);

    return deleteItem(index);
}

void Profile::copyItem(const QModelIndex &index) const
{
    ProfileItem *item = itemForIndex(index);
    Q_ASSERT(item);

    QString output;
    QXmlStreamWriter xml(&output);

    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(2);

    xml.writeStartDocument();
    xml.writeStartElement("settings");

    xml.writeStartElement(item->tagName());
    item->toXml(xml);
    xml.writeEndElement();

    xml.writeEndElement();
    xml.writeEndDocument();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(output);
}

QModelIndex Profile::pasteItem(const QModelIndex &index)
{
    QModelIndex pasteIndex(index);
    ProfileItem *siblingItem = itemForIndex(pasteIndex);
    Q_ASSERT(siblingItem);

    Group *parentItem = qobject_cast<Group*>(siblingItem);
    if (!parentItem)
    {
        parentItem = siblingItem->group();
        pasteIndex = pasteIndex.parent();
    }
    Q_ASSERT(parentItem);

    const QClipboard *clipboard = QApplication::clipboard();
    QXmlStreamReader xml(clipboard->text());
    QList<XmlError*> errors;

    Profile *pastedItems = new Profile;
    pastedItems->fromXml(xml, errors);

    if (!errors.isEmpty())
    {
        qCWarning(MUDDER_PROFILE) << "Paste errors:" << errors;
    }

    int count = pastedItems->m_root->itemCount();
    if (count < 1)
    {
        qCDebug(MUDDER_PROFILE) << "Pasted zero items";

        delete pastedItems;
        return index;
    }

    ProfileItem *firstItem = 0;
    int row = parentItem->itemCount();
    qCDebug(MUDDER_PROFILE) << "Pasting" << count << "items at row" << row << "under" << parentItem->fullName();

    beginInsertRows(pasteIndex, row, row + count - 1);
    for (int n = 0; n < count; n++)
    {
        ProfileItem *child = pastedItems->m_root->item(n);
        if (!firstItem)
        {
            firstItem = child;
        }

        parentItem->addItem(child);
    }
    endInsertRows();

    delete pastedItems;

    emit settingsChanged();

    return createIndex(row, 0, firstItem);
}

QModelIndex Profile::newItem(ProfileItem *item, const QModelIndex &index)
{
    ProfileItem *siblingItem = itemForIndex(index);
    Q_ASSERT(siblingItem);

    QModelIndex parentIndex(index);
    Group *parentItem = qobject_cast<Group*>(siblingItem);
    if (!parentItem)
    {
        parentItem = siblingItem->group();
        parentIndex = index.parent();
    }
    Q_ASSERT(parentItem);

    // TODO: smarter naming scheme to try and avoid duplicates
    static quint32 counter = 0;
    item->setName(QString("New Item %1").arg(++counter));

    int row = parentItem->itemCount();
    beginInsertRows(parentIndex, row, row);
    row = parentItem->addItem(item);
    endInsertRows();

    emit settingsChanged();

    return createIndex(row, 0, item);
}

void Profile::changeOption(const QString &key, const QVariant &val)
{
    if (m_options.value(key) != val)
    {
        m_options.insert(key, val);
        emit optionChanged(key, val);
    }
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
                setClearCommandLine(xml.attributes().value("clear").compare("n", Qt::CaseInsensitive) != 0);
                setEscapeClearsCommand(xml.attributes().value("escape").compare("n", Qt::CaseInsensitive) != 0);

                setCommandSeparator(xml.attributes().value("separator").toString());
            }
            else if (xml.name() == "scripting")
            {
                setScriptPrefix(xml.attributes().value("prefix").toString());
                setScriptFileName(xml.attributes().value("file").toString());
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
