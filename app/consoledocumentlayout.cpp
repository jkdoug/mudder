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


#include "consoledocumentlayout.h"
#include <QTextLayout>

static bool isFrameFromInlineObject(QTextFrame *f)
{
    return f->firstPosition() > f->lastPosition();
}


ConsoleDocumentLayout::ConsoleDocumentLayout(QTextDocument *doc) :
    QAbstractTextDocumentLayout(doc)
{
    m_margins = QMargins(2, 2, 2, 2);
}

ConsoleDocumentLayout::~ConsoleDocumentLayout()
{
    clearLayouts();
}

void ConsoleDocumentLayout::draw(QPainter *painter, const PaintContext &context)
{
    // TODO: optimize this

//    clearLayouts();

    QRectF r(context.clip);

    int y = r.height() - m_margins.bottom();

    QTextBlock textBlock(bottomBlock(context.cursorPosition));
    while (y > 0 && textBlock.isValid())
    {
        QTextLayout * textLayout = new QTextLayout(textBlock);
        m_layouts.insert(textBlock, textLayout);

        textLayout->beginLayout();

        QList<QTextLine> lines;
        QTextLine line(textLayout->createLine());
        while (y > 0 && line.isValid())
        {
            lines.append(line);
            line.setLineWidth(r.width() - m_margins.left() - m_margins.right());

            y -= line.height();

            line = textLayout->createLine();
        }

        qreal height = 0;
        foreach (line, lines)
        {
            line.setPosition(QPointF(m_margins.left(), y + height));
            height += line.height();
        }

        textLayout->endLayout();

        QVector<QTextLayout::FormatRange> selections;
        int blockPos = textBlock.position();
        int blockLen = textBlock.length();
        for (int i = 0; i < context.selections.size(); i++)
        {
            const QAbstractTextDocumentLayout::Selection &range = context.selections.at(i);
            const int selStart = range.cursor.selectionStart() - blockPos;
            const int selEnd = range.cursor.selectionEnd() - blockPos;

            if (selStart < blockLen && selEnd > 0 && selEnd > selStart)
            {
                QTextLayout::FormatRange o;
                o.start = selStart;
                o.length = selEnd - selStart;
                o.format = range.format;
                selections.append(o);
            }
        }

        textLayout->draw(painter, QPoint(0, 0), selections, context.clip);

        textBlock = textBlock.previous();
    }
}

int ConsoleDocumentLayout::hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const
{
    HitPoint hit = PointBefore;
    int position = 0;

    QTextBlock textBlock(bottomBlock());
    while (textBlock.isValid())
    {
        int pos = -1;
        HitPoint hp = hitTest(textBlock, point, &pos, accuracy);

        if (hp >= PointInside)
        {
            hit = hp;
            position = pos;
            break;
        }

        if (hp == PointBefore && pos < position)
        {
            position = pos;
            hit = hp;
        }
        else if (hp == PointAfter && pos > position)
        {
            position = pos;
            hit = hp;
        }

        textBlock = textBlock.previous();
    }

    if (accuracy == Qt::ExactHit && hit < PointExact)
    {
        return -1;
    }

    return position;
}

ConsoleDocumentLayout::HitPoint
ConsoleDocumentLayout::hitTest(QTextBlock bl, const QPointF &point, int *position, Qt::HitTestAccuracy accuracy) const
{
    if (!m_layouts.contains(bl))
    {
        qDebug() << "hitTest: block not found";
        return PointBefore;
    }

    QTextLayout * tl = m_layouts.value(bl);

    QRectF textRect(tl->boundingRect());
    textRect.translate(tl->position());

    *position = bl.position();
    if (point.y() < textRect.top())
    {
        return PointBefore;
    }
    else if (point.y() > textRect.bottom())
    {
        *position += bl.length();
        return PointAfter;
    }

    QPointF pos(point - tl->position());

    HitPoint hit = PointInside;
    int off = 0;
    for (int i = 0; i < tl->lineCount(); i++)
    {
        QTextLine line(tl->lineAt(i));
        const QRectF lr(line.naturalTextRect());
        if (lr.top() > pos.y())
        {
            off = qMin(off, line.textStart());
        }
        else if (lr.bottom() <= pos.y())
        {
            off = qMax(off, line.textStart() + line.textLength());
        }
        else
        {
            if (lr.left() <= pos.x() && lr.right() >= pos.x())
            {
                hit = PointExact;
            }

            if (accuracy == Qt::ExactHit)
            {
                off = line.xToCursor(pos.x(), QTextLine::CursorOnCharacter);
            }
            else
            {
                off = line.xToCursor(pos.x(), QTextLine::CursorBetweenCharacters);
            }
            break;
        }
    }
    *position += off;

    return hit;
}


int ConsoleDocumentLayout::pageCount() const
{
    // TODO?
    return 1;
}

QSizeF ConsoleDocumentLayout::documentSize() const
{
    return QSizeF(0, document()->blockCount());
}

QRectF ConsoleDocumentLayout::frameBoundingRect(QTextFrame *frame) const
{
    // TODO
    return QRectF();
}


QRectF ConsoleDocumentLayout::blockBoundingRect(const QTextBlock &block) const
{
    if (m_layouts.contains(block))
    {
        QTextLayout * textLayout = m_layouts.value(block);
        return textLayout->boundingRect();
    }

    return QRectF();
}

void ConsoleDocumentLayout::documentChanged(int from, int charsRemoved, int charsAdded)
{
    // TODO
//    qDebug() << "documentChanged" << from << charsRemoved << charsAdded;
}


QTextBlock ConsoleDocumentLayout::bottomBlock(int scroll) const
{
    if (!document())
    {
        return QTextBlock();
    }

    if (scroll > 0)
    {
        return document()->findBlockByNumber(scroll - 1);
    }

    if (document()->lastBlock().text().isEmpty())
    {
        return document()->lastBlock().previous();
    }

    return document()->lastBlock();
}

void ConsoleDocumentLayout::clearLayouts()
{
    foreach (QTextLayout * tl, m_layouts.values())
    {
        delete tl;
    }
    m_layouts.clear();
}
