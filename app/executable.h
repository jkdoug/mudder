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


#ifndef EXECUTABLE_H
#define EXECUTABLE_H

#include <QObject>
#include <QString>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include "profileitem.h"

class Engine;

class Executable : public ProfileItem
{
    Q_OBJECT

    Q_PROPERTY(QString contents READ contents WRITE setContents)

public:
    explicit Executable(QObject *parent = 0);
    Executable(const Executable &rhs, QObject *parent = 0);

    Executable & operator =(const Executable &rhs);

    bool operator ==(const Executable &rhs);
    bool operator !=(const Executable &rhs);

    const QString & contents() const { return m_contents; }
    void setContents(const QString &contents) { m_contents = contents; }

    bool execute(Engine *e);

    virtual void toXml(QXmlStreamWriter &xml);
    virtual void fromXml(QXmlStreamReader &xml);

protected:
    void clone(const Executable &rhs);

private:
    QString m_contents;
};

#endif // EXECUTABLE_H
