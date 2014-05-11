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

#include "configconnection.h"
#include "ui_configconnection.h"
#include "profile.h"
#include <QIntValidator>
#include <QMessageBox>

ConfigConnection::ConfigConnection(Profile *profile, ConfigWidget *parent) :
    ConfigPage(parent),
    ui(new Ui::ConfigConnection),
    m_profile(profile)
{
    ui->setupUi(this);
    ui->port->setValidator(new QIntValidator(1, 65535, this));
}

ConfigConnection::~ConfigConnection()
{
    delete ui;
}

void ConfigConnection::load()
{
    ui->name->setText(m_profile->name());
    ui->address->setText(m_profile->address());
    ui->port->setText(QString::number(m_profile->port()));

    ui->checkAutoConnect->setChecked(m_profile->autoConnect());
    ui->checkReconnect->setChecked(m_profile->autoReconnect());
}

void ConfigConnection::save()
{
    m_profile->setName(ui->name->text());
    m_profile->setAddress(ui->address->text());
    m_profile->setPort(ui->port->text().toInt());

    m_profile->setAutoConnect(ui->checkAutoConnect->isChecked());
    m_profile->setAutoReconnect(ui->checkReconnect->isChecked());
}

bool ConfigConnection::validate()
{
    if (ui->name->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, tr("Invalid Entry"), tr("The profile name must not be empty."));
        return false;
    }

    if (ui->address->text().trimmed().isEmpty())
    {
        QMessageBox::warning(this, tr("Invalid Entry"), tr("The host address must not be empty."));
        return false;
    }

    bool valid = false;
    int port = ui->port->text().toInt(&valid);
    if (!valid || port < 1 || port > 65535)
    {
        QMessageBox::warning(this, tr("Invalid Entry"), tr("The host port must be a valid number between 1 and 65535."));
        return false;
    }

    return true;
}
