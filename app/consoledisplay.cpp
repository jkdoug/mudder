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


#include "consoledisplay.h"
#include "console.h"
#include "textdocument.h"
#include <QAbstractTextDocumentLayout>
#include <QApplication>
#include <QDebug>
#include <QPainter>
#include <QPaintEvent>
#include <QPalette>
#include <QVector>

ConsoleDisplay::ConsoleDisplay(QWidget *parent) :
    QWidget(parent)
{
    m_console = 0;
    m_document = 0;

    m_scrollLines = 0;

    QPalette pal(palette());
    pal.setBrush(QPalette::Base, Qt::black);
    pal.setBrush(QPalette::Window, Qt::black);
    pal.setBrush(QPalette::Text, Qt::lightGray);
    setPalette(pal);
}

QAbstractTextDocumentLayout * ConsoleDisplay::documentLayout()
{
    if (!document())
    {
        return 0;
    }

    return document()->documentLayout();
}

void ConsoleDisplay::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter painter(this);
    painter.fillRect(rect(), palette().window());

    if (!document())
    {
        return;
    }

    QVector<QAbstractTextDocumentLayout::Selection> selections;
    if (document()->hasSelection())
    {
        QAbstractTextDocumentLayout::Selection selection;
        selection.cursor = *document()->cursor();
        selection.format = console()->formatSelection();
        selections.append(selection);
    }

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.clip = rect();
    ctx.cursorPosition = m_scrollLines;
    ctx.selections = selections;
    ctx.palette = console()->palette();

    documentLayout()->draw(&painter, ctx);
}
