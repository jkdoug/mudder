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


#ifndef CONSOLEDISPLAY_H
#define CONSOLEDISPLAY_H

#include <QAbstractTextDocumentLayout>
#include <QWidget>
#include "consoledocument.h"
#include "consoledocumentlayout.h"

class ConsoleDisplay : public QWidget
{
    Q_OBJECT
public:
    explicit ConsoleDisplay(QWidget *parent = 0);

    void setDocument(ConsoleDocument *doc) { m_document = doc; }

    void setScrollLines(int lines) { m_scrollLines = lines; }
    int scrollLines() const { return m_scrollLines; }

    ConsoleDocumentLayout * documentLayout();

protected:
    virtual void paintEvent(QPaintEvent *e);
    virtual void resizeEvent(QResizeEvent *e);

private:
    ConsoleDocument *m_document;

    int m_scrollLines;
};

#endif // CONSOLEDISPLAY_H
