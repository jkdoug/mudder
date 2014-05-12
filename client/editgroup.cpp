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


#include "editgroup.h"
#include "ui_editgroup.h"
#include "group.h"
#include <QMessageBox>

EditGroup::EditGroup(QWidget *parent) :
    EditSetting(parent),
    ui(new Ui::EditGroup)
{
    ui->setupUi(this);

    connect(ui->name, SIGNAL(textChanged(QString)), SLOT(changed()));
    connect(ui->enabled, SIGNAL(stateChanged(int)), SLOT(changed()));
}

EditGroup::~EditGroup()
{
    delete ui;
}

bool EditGroup::load(ProfileItem *item)
{
    Q_ASSERT(item != 0);
    if (!item)
    {
        return false;
    }

    Group *group = qobject_cast<Group *>(item);
    Q_ASSERT(group != 0);
    if (!group)
    {
        return false;
    }

    m_name = group->name();
    ui->name->setText(m_name);

    m_enabled = group->enabledFlag();
    ui->enabled->setChecked(m_enabled);

    return true;
}

bool EditGroup::save(ProfileItem *item)
{
    Q_ASSERT(item != 0);
    if (!item)
    {
        return false;
    }

    Group *group = qobject_cast<Group *>(item);
    Q_ASSERT(group != 0);
    if (!group)
    {
        return false;
    }

    QString name(ui->name->text());
    if (name.isEmpty())
    {
        QMessageBox::critical(this, tr("Invalid Group"), tr("Name may not be left empty."));
        return false;
    }
    if (!ProfileItem::validateName(name))
    {
        QMessageBox::critical(this, tr("Invalid Group"), tr("You may only use alphanumeric characters, underscores, and certain special characters in the name."));
        return false;
    }

    m_name = name;
    group->setName(m_name);
    m_enabled = ui->enabled->isChecked();
    group->enable(m_enabled);

    emit itemModified(false, true);

    return true;
}

void EditGroup::changed()
{
    if (m_name.isNull())
    {
        emit itemModified(false, false);
        return;
    }

    bool changed = m_name != ui->name->text() ||
        m_enabled != ui->enabled->isChecked();

    bool valid = !ui->name->text().isEmpty();

    emit itemModified(changed, valid);
}
