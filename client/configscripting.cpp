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

#include "configscripting.h"
#include "ui_configscripting.h"
#include "profile.h"
#include <QFileDialog>

ConfigScripting::ConfigScripting(Profile *profile, ConfigWidget *parent) :
    ConfigPage(parent),
    ui(new Ui::ConfigScripting),
    m_profile(profile)
{
    ui->setupUi(this);
}

ConfigScripting::~ConfigScripting()
{
    delete ui;
}

void ConfigScripting::load()
{
    ui->scriptFilename->setText(m_profile->scriptFileName());
    ui->scriptPrefix->setText(m_profile->scriptPrefix());
}

void ConfigScripting::save()
{
    m_profile->setScriptFileName(ui->scriptFilename->text());
    m_profile->setScriptPrefix(ui->scriptPrefix->text());
}

void ConfigScripting::on_scriptFilenameBrowse_clicked()
{
    QString filename(QFileDialog::getOpenFileName(this, tr("Choose Script File"), ui->scriptFilename->text(), tr("Lua Scripts (*.lua);;All files (*)")));
    if (!filename.isEmpty())
    {
        QFileInfo fi(filename);
        ui->scriptFilename->setText(fi.filePath());
    }
}
