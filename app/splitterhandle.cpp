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

#include <QBrush>
#include <QLinearGradient>
#include <QPainter>

#include "splitterhandle.h"

SplitterHandle::SplitterHandle(Qt::Orientation orient, QSplitter *parent) :
    QSplitterHandle(orient, parent)
{
}

void SplitterHandle::paintEvent(QPaintEvent *evt)
{
    QPainter painter(this);

    // TODO: customizable splitter handle colors :)

    QLinearGradient gradient(QPointF(100, 100), QPointF(200, 200));
    gradient.setColorAt(0, Qt::black);
    gradient.setColorAt(1, Qt::darkGray);

    if (orientation() == Qt::Horizontal)
    {
        gradient.setStart(rect().left(), rect(). height() / 2);
        gradient.setFinalStop(rect().right(), rect().height() / 2);
    }
    else
    {
        gradient.setStart(rect().width() / 2, rect().top());
        gradient.setFinalStop(rect().width() / 2, rect().bottom());
    }

    painter.fillRect(evt->rect(), QBrush(gradient));
}
