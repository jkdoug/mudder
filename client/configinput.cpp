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


#include "configinput.h"
#include "ui_configinput.h"
#include "profile.h"

ConfigInput::ConfigInput(Profile *profile, ConfigWidget *parent) :
    ConfigPage(parent),
    ui(new Ui::ConfigInput),
    m_profile(profile)
{
    ui->setupUi(this);

    QStringList fontSizes;
    for (int n = 6; n <= 20; n++)
    {
        fontSizes << QString::number(n);
    }
    ui->comboFontSize->addItems(fontSizes);
}

ConfigInput::~ConfigInput()
{
    delete ui;
}

void ConfigInput::load()
{
    QFont font(m_profile->inputFont());
    ui->comboFont->setCurrentFont(font);
    ui->comboFontSize->setCurrentIndex(font.pointSize() - 6);
    ui->checkAntiAliased->setChecked(font.styleStrategy() == QFont::PreferAntialias);

    ui->checkClear->setChecked(m_profile->clearCommandLine());
    ui->checkEscape->setChecked(m_profile->escapeClearsCommand());
    ui->commandSeparator->setText(m_profile->commandSeparator());
}

void ConfigInput::save()
{
    QFont font(ui->comboFont->currentFont());
    font.setPointSize(ui->comboFontSize->currentText().toInt());
    font.setStyleHint(QFont::TypeWriter);
    font.setStyleStrategy(ui->checkAntiAliased->isChecked()?QFont::PreferAntialias:QFont::NoAntialias);
    m_profile->setInputFont(font);

    m_profile->setClearCommandLine(ui->checkClear->isChecked());
    m_profile->setEscapeClearsCommand(ui->checkEscape->isChecked());
    m_profile->setCommandSeparator(ui->commandSeparator->text());
}
