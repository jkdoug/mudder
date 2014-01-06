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
//#include "consoledocumentlayout.h"
#include "textdocument.h"
#include <QAbstractTextDocumentLayout>
#include <QAction>
#include <QApplication>
#include <QClipboard>
#include <QDebug>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QPaintEvent>
#include <QRectF>
#include <QTextBlock>
#include <QTextLayout>
#include <QTextLine>
#include <QToolTip>

ConsoleDisplay::ConsoleDisplay(QWidget *parent) :
    QWidget(parent)
{
    m_console = 0;
    m_document = 0;
//    m_layoutDoc = 0;

    m_scrollLines = 0;

    QPalette pal(palette());
    pal.setBrush(QPalette::Base, Qt::black);
    pal.setBrush(QPalette::Window, Qt::black);
    pal.setBrush(QPalette::Text, Qt::lightGray);
    setPalette(pal);
}

void ConsoleDisplay::setDocument(TextDocument *doc)
{
    m_document = doc;
//    m_layoutDoc = new ConsoleDocumentLayout(m_document);
}

QAbstractTextDocumentLayout * ConsoleDisplay::documentLayout()
{
    if (!m_document)
    {
        return 0;
    }

    return m_document->documentLayout();
}

void ConsoleDisplay::copy()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(m_document->toPlainText());

//    m_mousePressed = false;
    m_document->selectNone();
}

void ConsoleDisplay::copyHtml()
{
    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(m_document->toHtml());

//    m_mousePressed = false;
    m_document->selectNone();
}

//void ConsoleDisplay::mousePressEvent(QMouseEvent *e)
//{
//    if (!m_document)
//    {
//        return;
//    }

//    if (e->button() == Qt::LeftButton)
//    {
//        m_clickPos = documentLayout()->hitTest(e->pos(), Qt::ExactHit);

//        int pos = documentLayout()->hitTest(e->pos(), Qt::FuzzyHit);
//        if (pos < 0)
//        {
//            return;
//        }

//        m_mousePressed = true;
//        m_selectionStart = pos;
//        m_selectionEnd = pos;

//        qDebug() << "mousePress" << m_selectionStart << m_selectionEnd;
//    }
//}

//void ConsoleDisplay::mouseReleaseEvent(QMouseEvent *e)
//{
////    QPoint pt(textCursor(e->pos()));
////    handleMouseEvent(e, pt);

//    if (m_document && e->button() == Qt::RightButton)
//    {
//        QMenu *popup = new QMenu(this);

//        if (m_document->hasSelection())
//        {
//            QAction *actionCopy = new QAction(tr("&Copy"), this);
//            actionCopy->setStatusTip(tr("Copy selected text to clipboard"));
//            connect(actionCopy, SIGNAL(triggered()), this, SLOT(copy()));

//            QAction *actionCopyHtml = new QAction(tr("Copy as &HTML"), this);
//            actionCopyHtml->setStatusTip(tr("Copy selected text to clipboard as HTML"));
//            connect(actionCopyHtml, SIGNAL(triggered()), this, SLOT(copyHtml()));

//            QAction *actionSelectNone = new QAction(tr("Select &None"), this);
//            actionSelectNone->setStatusTip(tr("Removes the current text selection"));
//            connect(actionSelectNone, SIGNAL(triggered()), m_document, SLOT(selectNone()));

//            popup->addAction(actionCopy);
//            popup->addAction(actionCopyHtml);
//            popup->addSeparator();
//            popup->addAction(actionSelectNone);
//        }

//        QAction *actionSelectAll = new QAction(tr("Select &All"), this);
//        actionSelectAll->setStatusTip(tr("Select all buffered output text"));
//        connect(actionSelectAll, SIGNAL(triggered()), m_document, SLOT(selectAll()));

//        QAction *actionClearBuffer = new QAction(tr("C&lear output buffer"), this);
//        actionClearBuffer->setStatusTip(tr("Deletes all text stored in the output text buffer"));
//        connect(actionClearBuffer, SIGNAL(triggered()), m_document, SLOT(clear()));

//        popup->addAction(actionSelectAll);
//        popup->addSeparator();
//        popup->addAction(actionClearBuffer);

//        popup->popup(mapToGlobal(e->pos()), popup->actions().at(0));
//    }
//    else if (e->button() == Qt::MiddleButton)
//    {
//        console()->scrollTo(m_document->blockCount());
//    }
//    else if (e->button() == Qt::LeftButton)
//    {
//        qDebug() << "mouseRelease" << m_selectionStart << m_selectionEnd;

//        // TODO: don't clear selection when clicking within the selected text?
//        if (documentLayout()->hitTest(e->pos(), Qt::ExactHit) == m_clickPos)
//        {
//            m_document->selectNone();

//            m_selectionStart = 0;
//            m_selectionEnd = 0;
//        }

//        m_mousePressed = false;
//        m_clickPos = -1;
//    }

//    e->accept();
//}

//void ConsoleDisplay::mouseMoveEvent(QMouseEvent *e)
//{
//    if (!m_document)
//    {
//        return;
//    }

//    if (m_mousePressed)
//    {
//        int pos = documentLayout()->hitTest(e->pos(), Qt::FuzzyHit);
//        if (pos < 0)
//        {
//            e->ignore();
//            return;
//        }

//        m_selectionEnd = pos;

//        m_document->select(m_selectionStart, m_selectionEnd);

//        if (e->y() < 10)
//        {
//            console()->scrollUp(1);
//        }
//        else if (e->y() > height() - 10)
//        {
//            console()->scrollDown(1);
//        }
//    }
//    else
//    {
//        QString anchor(documentLayout()->anchorAt(e->pos()));
//        if (anchor != m_linkHovered)
//        {
//            m_linkHovered = anchor;

//            if (m_linkHovered.isEmpty())
//            {
//                QToolTip::hideText();
//                setCursor(Qt::IBeamCursor);
//            }
//            else
//            {
//                QToolTip::showText(e->globalPos(), "Test hint\nhttp://google.com");
//                setCursor(Qt::PointingHandCursor);
//            }
//        }
//    }

//    e->accept();
//}

void ConsoleDisplay::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    QPainter painter(this);
    painter.fillRect(rect(), palette().window());

    if (!m_document)
    {
        return;
    }

    QVector<QAbstractTextDocumentLayout::Selection> selections;
    if (m_document->hasSelection())
    {
        QAbstractTextDocumentLayout::Selection selection;
        selection.cursor = *m_document->cursor();
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
