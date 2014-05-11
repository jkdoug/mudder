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


#ifndef CONFIGCONNECTION_H
#define CONFIGCONNECTION_H

#include "configpage.h"

namespace Ui {
class ConfigConnection;
}

class Profile;

class ConfigConnection : public ConfigPage
{
    Q_OBJECT

public:
    explicit ConfigConnection(Profile *profile, ConfigWidget *parent = 0);
    ~ConfigConnection();

public slots:
    virtual void load();
    virtual void save();
    virtual bool validate();

private:
    Ui::ConfigConnection *ui;

    Profile *m_profile;
};

#endif // CONFIGCONNECTION_H
