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


#ifndef EDITVARIABLE_H
#define EDITVARIABLE_H

#include <QVariant>
#include "editsetting.h"

namespace Ui {
class EditVariable;
}

class EditVariable : public EditSetting
{
    Q_OBJECT

public:
    explicit EditVariable(QWidget *parent = 0);
    ~EditVariable();

    virtual bool load(ProfileItem *item);
    virtual bool save(ProfileItem *item);

private slots:
    void changed();

private:
    Ui::EditVariable *ui;

    QVariant m_contents;
};

#endif // EDITVARIABLE_H
