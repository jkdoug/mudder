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


#include "hyperlink.h"

Hyperlink::Hyperlink(QObject *parent) :
    ProfileItem(parent)
{
}

Hyperlink::Hyperlink(const Hyperlink &rhs, QObject *parent) :
    ProfileItem(parent)
{
    clone(rhs);
}

Hyperlink::Hyperlink(const QString &hint, const QVariant &link, QObject *parent) :
    ProfileItem(parent)
{
    m_hint = hint;
    m_link = link;
}

Hyperlink & Hyperlink::operator =(const Hyperlink &rhs)
{
    clone(rhs);
    return *this;
}

void Hyperlink::clone(const Hyperlink &rhs)
{
    ProfileItem::clone(rhs);

    m_hint = rhs.m_hint;
    m_link = rhs.m_link;
}
