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


#ifndef EDITSETTING_H
#define EDITSETTING_H

#include <QWidget>
#include "profileitem.h"

class EditSetting : public QWidget
{
    Q_OBJECT
public:
    explicit EditSetting(QWidget *parent = 0);

    virtual bool load(ProfileItem *item) = 0;
    virtual bool save(ProfileItem *item) = 0;

signals:
    void itemModified(bool changed, bool valid);

protected:
    QString m_name;

    bool m_enabled;

    int m_sequence;
};

#endif // EDITSETTING_H
