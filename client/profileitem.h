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


#ifndef PROFILEITEM_H
#define PROFILEITEM_H

#include <QIcon>
#include <QList>
#include <QObject>
#include <QStringList>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class Group;
class XmlError;

class ProfileItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString fullName READ fullName)
    Q_PROPERTY(bool enabled READ enabledFlag WRITE enable)
    Q_PROPERTY(int sequence READ sequence WRITE setSequence)

public:
    virtual const QString & name() const { return m_name; }
    void setName(const QString &name);
    virtual QString fullName() const;
    virtual bool enabled() const;
    bool enabledFlag() const { return m_enabled; }
    virtual void enable(bool flag = true);
    virtual int sequence() const { return m_sequence; }
    void setSequence(int sequence);
    Group * group() const;
    QStringList path() const;

    virtual QIcon icon() const { return QIcon(); }
    virtual QString value() const { return QString(); }

    virtual QString tagName() const { return "item"; }
    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml, QList<XmlError *> &errors);

    static bool validateName(const QString &name, bool allowEmpty = false);

signals:
    void modified(ProfileItem *item);

protected:
    explicit ProfileItem(QObject *parent = 0);
    ProfileItem(const ProfileItem &rhs, QObject *parent = 0);

    ProfileItem & operator =(const ProfileItem &rhs);

    bool operator ==(const ProfileItem &rhs);
    bool operator !=(const ProfileItem &rhs);

    bool operator <(const ProfileItem &rhs);

    void clone(const ProfileItem &rhs);

    QString unnamed();

protected:
    QString m_name;

    bool m_enabled;

    int m_sequence;
};

#endif // PROFILEITEM_H
