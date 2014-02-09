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


#include <QDebug>
#include "dialogglobal.h"
#include "ui_dialogglobal.h"
#include "configeditor.h"

DialogGlobal::DialogGlobal(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogGlobal)
{
    ui->setupUi(this);

    ui->config->addPage(new ConfigEditor(ui->config));

    ui->config->load();

    connect(ui->buttonBox, SIGNAL(clicked(QAbstractButton*)), SLOT(buttonClicked(QAbstractButton*)));
}

DialogGlobal::~DialogGlobal()
{
    delete ui;
}

void DialogGlobal::buttonClicked(QAbstractButton *button)
{
    QDialogButtonBox::ButtonRole role = ui->buttonBox->buttonRole(button);
    switch (role)
    {
    case QDialogButtonBox::AcceptRole:
        if (ui->config->validate())
        {
            ui->config->save();
            accept();
        }
        break;

    case QDialogButtonBox::ApplyRole:
        if (ui->config->validate())
        {
            ui->config->save();
        }
        break;

    case QDialogButtonBox::RejectRole:
        close();
        break;

    case QDialogButtonBox::ResetRole:
        ui->config->restoreDefaults();
        break;
    }
}
