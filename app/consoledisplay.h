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


#ifndef CONSOLEDISPLAY_H
#define CONSOLEDISPLAY_H

#include <QCursor>
#include <QMargins>
#include <QMouseEvent>
#include <QPaintEvent>
#include <QString>
#include <QTextBlock>
#include <QWheelEvent>
#include <QWidget>

class Console;
//class ConsoleDocumentLayout;
class QAbstractTextDocumentLayout;
class QMouseEvent;
class QPaintEvent;
class TextDocument;

class ConsoleDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit ConsoleDisplay(QWidget *parent = 0);

    void setConsole(Console *console) { m_console = console; }
    void setDocument(TextDocument *doc);

    void setScrollLines(int lines) { m_scrollLines = lines; }
    int scrollLines() const { return m_scrollLines; }

    QAbstractTextDocumentLayout * documentLayout();

public slots:
    void copy();
    void copyHtml();

protected:
//    virtual void mousePressEvent(QMouseEvent *e);
//    virtual void mouseReleaseEvent(QMouseEvent *e);
//    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void paintEvent(QPaintEvent *e);

    Console * console() const { return m_console; }

private:
    Console *m_console;
    TextDocument *m_document;
//    ConsoleDocumentLayout *m_layoutDoc;

    int m_scrollLines;
};

#endif // CONSOLEDISPLAY_H
