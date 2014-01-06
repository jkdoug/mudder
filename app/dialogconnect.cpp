/***************************************************************************
 *   Copyright (C) 2013 by Jason Douglas                                   *
 *   larkin.dischai@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "dialogconnect.h"
#include "ui_dialogconnect.h"

#include <QMessageBox>
#include <QSettings>

DialogConnect::DialogConnect(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogConnect)
{
    ui->setupUi(this);

    load();
}

DialogConnect::~DialogConnect()
{
    delete ui;
}

void DialogConnect::accept()
{
    if (validate())
    {
        save();

        QDialog::accept();
    }
}

void DialogConnect::load()
{
    QSettings settings;
    ui->name->setText(settings.value("Defaults/ProfileName", "").toString());
    ui->address->setText(settings.value("Defaults/HostAddress", "lusternia.com").toString());
    ui->port->setText(settings.value("Defaults/HostPort", 23).toString());
    ui->connect->setChecked(settings.value("Defaults/AutoConnect", true).toBool());
}

void DialogConnect::save()
{
    m_name = ui->name->text();
    m_address = ui->address->text();
    m_port = ui->port->text().toInt();
    m_connect = ui->connect->isChecked();

    QSettings settings;
    settings.setValue("Defaults/ProfileName", m_name);
    settings.setValue("Defaults/HostAddress", m_address);
    settings.setValue("Defaults/HostPort", m_port);
    settings.setValue("Defaults/AutoConnect", m_connect);
}

bool DialogConnect::validate()
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
