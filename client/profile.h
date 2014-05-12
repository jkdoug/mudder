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

#include <QFont>
#include <QHash>
#include <QList>
#include <QObject>
#include <QVariantMap>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class Group;
class ProfileItem;
class XmlError;

class Profile : public QObject
{
    Q_OBJECT
public:
    explicit Profile(QObject *parent = 0);

    Group * rootGroup() const { return m_root; }
    Group * activeGroup() const { return m_activeGroup?m_activeGroup:m_root; }
    void setActiveGroup(Group *group);

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

public slots:
    void changeOption(const QString &key, const QVariant &val);

signals:
    void optionChanged(const QString &key, const QVariant &val);
    void settingsChanged();

protected:
    void readProfile(QXmlStreamReader &xml, QList<XmlError *> &errors);
    void readDisplay(QXmlStreamReader &xml, QList<XmlError *> &errors);

private:
    Group *m_root;
    Group *m_activeGroup;

    QVariantMap m_options;
};

#endif // PROFILE_H
