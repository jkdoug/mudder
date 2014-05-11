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


#ifndef EDITTRIGGER_H
#define EDITTRIGGER_H

#include "editsetting.h"

namespace Ui {
class EditTrigger;
}

class EditTrigger : public EditSetting
{
    Q_OBJECT

public:
    explicit EditTrigger(QWidget *parent = 0);
    ~EditTrigger();

    virtual bool load(ProfileItem *item);
    virtual bool save(ProfileItem *item);

private slots:
    void changed();

private:
    Ui::EditTrigger *ui;

    QString m_pattern;
    QString m_script;

    bool m_caseSensitive;
    bool m_keepEvaluating;
    bool m_repeat;
    bool m_omit;
};

#endif // EDITTRIGGER_H
