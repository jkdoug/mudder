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


#include "consoledocumentlayout.h"
#include <QTextBlock>

ConsoleDocumentLayout::ConsoleDocumentLayout(QTextDocument *doc) :
    QAbstractTextDocumentLayout(doc)
{
}

ConsoleDocumentLayout::~ConsoleDocumentLayout()
{
}

void ConsoleDocumentLayout::draw(QPainter *painter, const PaintContext &context)
{
    QRectF r(context.clip);

    int y = r.height();

    QTextBlock textBlock(bottomBlock(context.cursorPosition));
    while (y > 0 && textBlock.isValid())
    {
        QTextLayout * textLayout = textBlock.layout();
        if (!textLayout)
        {
            textLayout = new QTextLayout(textBlock);
        }

        textLayout->beginLayout();

        int blockHeight = y;

        QList<QTextLine> lines;
        QTextLine line(textLayout->createLine());
        while (line.isValid())
        {
            lines.append(line);
            line.setLineWidth(r.width());

            y -= line.height();

            line = textLayout->createLine();
        }

        qreal height = blockHeight;
        for (int n = lines.count() - 1; n >= 0; n--)
        {
            line = lines.at(n);
            height -= line.height();
            line.setPosition(QPointF(r.left(), height));
        }

        textLayout->endLayout();

        textLayout->draw(painter, QPointF(0, 0));

        textBlock = textBlock.previous();
    }
}

int ConsoleDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    Q_UNUSED(point)
    Q_UNUSED(accuracy)

    return 0;
}

int ConsoleDocumentLayout::pageCount() const
{
    return 1;
}

QSizeF ConsoleDocumentLayout::documentSize() const
{
    return QSizeF(0, document()->blockCount());
}

QRectF ConsoleDocumentLayout::frameBoundingRect(QTextFrame *frame) const
{
    Q_UNUSED(frame)

    return QRectF();
}


QRectF ConsoleDocumentLayout::blockBoundingRect(const QTextBlock &block) const
{
    if (block.layout())
    {
        return block.layout()->boundingRect();
    }

    return QRectF();
}

void ConsoleDocumentLayout::documentChanged(int from, int charsRemoved, int charsAdded)
{
    Q_UNUSED(from)
    Q_UNUSED(charsRemoved)
    Q_UNUSED(charsAdded)
}

QTextBlock ConsoleDocumentLayout::bottomBlock(int scroll) const
{
    if (!document())
    {
        return QTextBlock();
    }

    QTextBlock block;
    if (scroll > 0)
    {
        block = document()->findBlockByNumber(scroll - 1);
    }
    else
    {
        block = document()->lastBlock();
    }

    if (block.text().isEmpty())
    {
        return block.previous();
    }

    return block;
}
