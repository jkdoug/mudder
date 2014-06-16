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


#ifndef PROFILE_H
#define PROFILE_H

#include <QAbstractItemModel>
#include <QFont>
#include <QHash>
#include <QList>
#include <QObject>
#include <QVariantMap>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class Group;
class ProfileItem;
class Timer;
class XmlError;

class Profile : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit Profile(QObject *parent = 0);

    Group * rootGroup() const { return m_root; }
    Group * activeGroup() const { return m_activeGroup?m_activeGroup:m_root; }
    void setActiveGroup(Group *group);

    QVariant getVariable(const QString &name);
    bool setVariable(const QString &name, const QVariant &val);
    bool deleteVariable(const QString &name);

    const QVariantMap & options() const { return m_options; }
    QVariant getOption(const QString &key) const { return m_options.value(key); }
    void setOptions(const QVariantMap &options);

    QString name() const { return m_options.value("name").toString(); }
    void setName(const QString &name) { changeOption("name", name); }

    QString address() const { return m_options.value("address").toString(); }
    void setAddress(const QString &address) { changeOption("address", address); }
    int port() const { return m_options.value("port").toInt(); }
    void setPort(int port) { changeOption("port", port); }
    bool autoConnect() const { return m_options.value("autoConnect").toBool(); }
    void setAutoConnect(bool flag = true) { changeOption("autoConnect", flag); }
    bool autoReconnect() const { return m_options.value("autoReconnect").toBool(); }
    void setAutoReconnect(bool flag = true) { changeOption("autoReconnect", flag); }

    QString scriptFileName() const { return m_options.value("scriptFileName").toString(); }
    void setScriptFileName(const QString &fileName) { changeOption("scriptFileName", fileName); }
    QString scriptPrefix() const { return m_options.value("scriptPrefix").toString(); }
    void setScriptPrefix(const QString &prefix) { changeOption("scriptPrefix", prefix); }

    QString commandSeparator() const { return m_options.value("commandSeparator").toString(); }
    void setCommandSeparator(const QString &sep) { changeOption("commandSeparator", sep); }
    bool clearCommandLine() const { return m_options.value("clearCommandLine").toBool(); }
    void setClearCommandLine(bool flag = true) { changeOption("clearCommandLine", flag); }
    bool escapeClearsCommand() const { return m_options.value("escapeClearsCommand").toBool(); }
    void setEscapeClearsCommand(bool flag = true) { changeOption("escapeClearsCommand", flag); }

    QFont inputFont() const { return m_options.value("inputFont").value<QFont>(); }
    void setInputFont(const QFont &font) { changeOption("inputFont", font); }

    QFont outputFont() const { return m_options.value("outputFont").value<QFont>(); }
    void setOutputFont(const QFont &font) { changeOption("outputFont", font); }

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors);
    static bool validateXml(const QString &text, QList<XmlError *> *errors = 0);


    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &index) const;

    virtual bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    ProfileItem * itemForIndex(const QModelIndex &index) const;
    QStringList pathForIndex(const QModelIndex &index) const;
    QModelIndex indexForPath(const QStringList &path) const;
    QModelIndex cutItem(const QModelIndex &index);
    void copyItem(const QModelIndex &index) const;
    QModelIndex pasteItem(const QModelIndex &index);
    QModelIndex newItem(ProfileItem *item, const QModelIndex &index = QModelIndex());

public slots:
    void changeOption(const QString &key, const QVariant &val);
    void handleTimer(Timer *timer);

signals:
    void optionChanged(const QString &key, const QVariant &val);
    void settingsChanged();
    void timerFired(Timer *timer);

protected:
    void readProfile(QXmlStreamReader &xml, QList<XmlError *> &errors);
    void readDisplay(QXmlStreamReader &xml, QList<XmlError *> &errors);

private:
    QModelIndex indexForPath(const QModelIndex &parent, const QStringList &path) const;

    template<class C>
    C * findItem(const QString &name, Group *parent = 0) const;

    Group * createGroup(const QString &path);
    Group * createGroup(const QStringList &path, Group *parent = 0);

    Group *m_root;
    Group *m_activeGroup;

    QVariantMap m_options;
};

#endif // PROFILE_H
