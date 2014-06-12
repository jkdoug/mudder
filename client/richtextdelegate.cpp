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

#include "richtextdelegate.h"
#include <QAbstractTextDocumentLayout>
#include <QApplication>
//#include <QIcon>
#include <QModelIndex>
#include <QPainter>
//#include <QPixmapCache>
#include "logging.h"


RichTextDelegate::RichTextDelegate(QObject *parent)
    : QStyledItemDelegate(parent)
{
}

RichTextDelegate::~RichTextDelegate()
{
}

void RichTextDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setHtml(opt.text);

    opt.text = QString();
    style->drawControl(QStyle::CE_ItemViewItem, &opt, painter, opt.widget);

    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette = opt.palette;

    QRect textRect(style->subElementRect(QStyle::SE_ItemViewItemText, &opt));

    painter->save();
    painter->translate(textRect.topLeft());
    doc.documentLayout()->draw(painter, ctx);
    painter->restore();
}

QSize RichTextDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    initStyleOption(&opt, index);

    QStyle *style = opt.widget ? opt.widget->style() : QApplication::style();

    QTextDocument doc;
    doc.setDefaultFont(option.font);
    doc.setHtml(opt.text);
    doc.setTextWidth(opt.rect.width());

    QRect iconRect(style->subElementRect(QStyle::SE_ItemViewItemDecoration, &opt));

    return QSize(doc.idealWidth() + iconRect.width(), doc.size().height());
}
