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


#include "editgroup.h"
#include "ui_editgroup.h"
#include "logger.h"
#include "group.h"

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
    LOG_TRACE("EditGroup::load", item->fullName());

    Group *group = qobject_cast<Group *>(item);
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
    if (!item)
    {
        LOG_ERROR("Attempted to save a group to a null address.");
        return false;
    }

    Group *group = qobject_cast<Group *>(item);
    Q_ASSERT(group != 0);
    if (!group)
    {
        LOG_ERROR("Attempted to save a group to a non-group item.");
        return false;
    }

    LOG_TRACE("EditGroup::save", item->fullName());

    group->setName(ui->name->text());
    group->enable(ui->enabled->isChecked());

    changed();

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
