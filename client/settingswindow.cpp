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


#include "settingswindow.h"
#include "ui_settingswindow.h"
#include "group.h"
#include "logger.h"

SettingsWindow::SettingsWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    connect(ui->editor, SIGNAL(settingModified(bool, bool)), SLOT(settingModified(bool, bool)));
    connect(ui->actionSave, SIGNAL(triggered()), ui->editor, SLOT(saveCurrentItem()));
    connect(ui->actionDiscard, SIGNAL(triggered()), ui->editor, SLOT(discardCurrentItem()));
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::setRootGroup(Group *group)
{
    ui->editor->setRootGroup(group);
}

void SettingsWindow::settingModified(bool changed, bool valid)
{
    ui->actionSave->setEnabled(changed && valid);
    ui->actionDiscard->setEnabled(changed);
}
