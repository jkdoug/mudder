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

#include "dialogprofile.h"
#include "ui_dialogprofile.h"
#include "configconnection.h"
#include "configscripting.h"
#include "profile.h"
#include <QPushButton>


DialogProfile::DialogProfile(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogProfile)
{
    ui->setupUi(this);

    m_profile = new Profile(this);

    ui->config->addPage(new ConfigConnection(m_profile, ui->config));
    ui->config->addPage(new ConfigScripting(m_profile, ui->config));
}

DialogProfile::~DialogProfile()
{
    delete ui;
}

void DialogProfile::load(Profile *profile)
{
    Q_ASSERT(profile != 0);
    Q_ASSERT(m_profile != 0);

    m_profile->setOptions(profile->options());

    ui->config->load();
}

void DialogProfile::accept()
{
    if (ui->config->validate())
    {
        ui->config->save();

        QDialog::accept();
    }
}
