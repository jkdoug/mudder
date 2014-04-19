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


#ifndef EDITGROUP_H
#define EDITGROUP_H

#include "editsetting.h"

namespace Ui {
class EditGroup;
}

class EditGroup : public EditSetting
{
    Q_OBJECT

public:
    explicit EditGroup(QWidget *parent = 0);
    ~EditGroup();

    virtual bool load(ProfileItem *item);
    virtual bool save(ProfileItem *item);

private slots:
    void changed();

private:
    Ui::EditGroup *ui;
};

#endif // EDITGROUP_H
