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


#ifndef CONSOLEDOCUMENTLAYOUT_H
#define CONSOLEDOCUMENTLAYOUT_H

#include <QAbstractTextDocumentLayout>
#include <QTextDocument>

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

private:
    QTextBlock bottomBlock(int scroll = 0) const;
};

#endif // CONSOLEDOCUMENTLAYOUT_H
