/*
  Mudder, a cross-platform text gaming client

  Copyright (C) 2014 Jason Douglas
  larkin.dischai@gmail.com

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

#include <QHash>
#include <QList>
#include <QObject>
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

    const QString & name() const { return m_name; }
    void setName(const QString &name);

    const QString & address() const { return m_address; }
    void setAddress(const QString &address);
    int port() const { return m_port; }
    void setPort(int port);
    bool autoConnect() const { return m_autoConnect; }
    void setAutoConnect(bool flag = true);
    bool autoReconnect() const { return m_autoReconnect; }
    void setAutoReconnect(bool flag = true);

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors);

signals:
    void optionChanged(const QString &key);
    void settingsChanged();

protected:
    void readProfile(QXmlStreamReader &xml, QList<XmlError *> &errors);

private:
    Group *m_root;
    Group *m_activeGroup;

    QString m_name;

    QString m_address;
    int m_port;
    bool m_autoConnect;
    bool m_autoReconnect;
};

#endif // PROFILE_H
