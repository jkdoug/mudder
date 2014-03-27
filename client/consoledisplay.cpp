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


#include "consoledisplay.h"
#include <QPainter>

ConsoleDisplay::ConsoleDisplay(QWidget *parent) :
    QWidget(parent)
{
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
    if (!m_document)
    {
        return 0;
    }

    return m_document->documentLayout();
}

void ConsoleDisplay::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e)

    QPainter painter(this);
    painter.fillRect(rect(), palette().window());

    if (!m_document)
    {
        return;
    }

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.clip = rect();
    ctx.palette = palette();
    ctx.cursorPosition = m_scrollLines;

    m_document->documentLayout()->draw(&painter, ctx);
}
