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


#ifndef DIALOGGLOBAL_H
#define DIALOGGLOBAL_H

#include <QDialog>
#include <QFont>

namespace Ui {
class DialogGlobal;
}

class DialogGlobal : public QDialog
{
    Q_OBJECT

public:
    explicit DialogGlobal(QWidget *parent = 0);
    ~DialogGlobal();

    QFont editorFont() const { return m_editorFont; }

private:
    void load();
    void save();
    bool validate();

    Ui::DialogGlobal *ui;

    QFont m_editorFont;

public slots:
    virtual void accept();

private slots:
    void on_pageList_currentRowChanged(int currentRow);
};

#endif // DIALOGGLOBAL_H
