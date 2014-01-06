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


#ifndef HYPERLINK_H
#define HYPERLINK_H

#include <QObject>
#include <QString>
#include <QVariant>
#include "profileitem.h"

class Hyperlink : public ProfileItem
{
    Q_OBJECT
public:
    explicit Hyperlink(QObject *parent = 0);
    Hyperlink(const Hyperlink &rhs, QObject *parent = 0);
    Hyperlink(const QString &hint, const QVariant &link, QObject *parent = 0);

    Hyperlink & operator =(const Hyperlink &rhs);

    virtual const QString & name() const { return m_hint; }
    void setHint(const QString &hint) { m_hint = hint; }
    const QString & hint() const { return m_hint; }
    void setLink(const QVariant &link) { m_link = link; }
    const QVariant & link() const { return m_link; }

protected:
    void clone(const Hyperlink &rhs);

private:
    QString m_hint;
    QVariant m_link;
};

#endif // HYPERLINK_H
