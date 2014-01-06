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


#ifndef CONSOLEDOCUMENTLAYOUT_H
#define CONSOLEDOCUMENTLAYOUT_H

#include "fixed.h"
#include <QAbstractTextDocumentLayout>
#include <QMap>
#include <QMargins>
#include <QPainter>
#include <QPointF>
#include <QRectF>
#include <QSizeF>
#include <QTextBlock>
#include <QTextDocument>
#include <QTextFrame>
#include <QTextLayout>

class ConsoleDocumentLayout : public QAbstractTextDocumentLayout
{
    Q_OBJECT
public:
    explicit ConsoleDocumentLayout(QTextDocument *doc);
    ~ConsoleDocumentLayout();

    virtual void draw(QPainter *painter, const PaintContext &context);
    virtual int hitTest(const QPointF &point, Qt::HitTestAccuracy accuracy) const;

    virtual int pageCount() const;
    virtual QSizeF documentSize() const;

    virtual QRectF frameBoundingRect(QTextFrame *frame) const;
    virtual QRectF blockBoundingRect(const QTextBlock &block) const;

protected:
    virtual void documentChanged(int from, int charsRemoved, int charsAdded);

    enum HitPoint
    {
        PointBefore,
        PointAfter,
        PointInside,
        PointExact
    };
    HitPoint hitTest(QTextBlock bl, const QPointF &point, int *position, Qt::HitTestAccuracy accuracy) const;

private:
    QTextBlock bottomBlock(int scroll = 0) const;
    void clearLayouts();

    QMap<QTextBlock, QTextLayout *> m_layouts;

    QMargins m_margins;
};

#endif // CONSOLEDOCUMENTLAYOUT_H
