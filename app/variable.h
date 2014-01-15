/***************************************************************************
 *   Copyright (C) 2013 by Jason Douglas                                   *
 *   larkin.dischai@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#ifndef VARIABLE_H
#define VARIABLE_H

#include "profileitem.h"
#include <QList>
#include <QMap>
#include <QObject>
#include <QString>
#include <QVariant>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>

class Group;

class Variable : public ProfileItem
{
    Q_OBJECT

    Q_PROPERTY(QVariant contents READ contents WRITE setContents)
    Q_PROPERTY(QString typeName READ typeName)

public:
    explicit Variable(QObject *parent = 0);
    Variable(const Variable &rhs, QObject *parent = 0);

    Variable & operator =(const Variable &rhs);

    bool operator ==(const Variable &rhs);
    bool operator !=(const Variable &rhs);

    bool operator <(const Variable &rhs);
    static bool lessName(const Variable *lhs, const Variable *rhs);

    const QVariant & contents() const { return m_contents; }
    void setContents(const QVariant &contents) { m_contents = contents; }
    QString typeName();

    virtual QString value() const { return contents().toString(); }
    virtual int sequence() const { return 0; }

    static QVariant::Type translateType(const QString &type);
    static QVariant translateValue(const QVariant &value, QVariant::Type type);

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml);

private:
    void clone(const Variable &rhs);

    QVariant m_contents;
};

typedef QList<Variable *> VariableList;

#endif // VARIABLE_H
