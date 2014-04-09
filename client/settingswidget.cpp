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


#include <QLabel>
#include "settingswidget.h"
#include "ui_settingswidget.h"
#include "logger.h"
#include "settingsmodel.h"
#include "group.h"
#include "editvariable.h"

SettingsWidget::SettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);

    m_defaultForm = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(m_defaultForm);
    QLabel *label = new QLabel(tr("Select an item in the tree."), m_defaultForm);
    layout->addWidget(label);
    layout->setAlignment(label, Qt::AlignCenter);
    m_defaultForm->setLayout(layout);
    ui->layoutEdit->addWidget(m_defaultForm);

    m_editor = 0;

    m_model = new SettingsModel(this);
    ui->treeView->setModel(m_model);

    m_selection = ui->treeView->selectionModel();
    connect(m_selection, SIGNAL(currentChanged(QModelIndex, QModelIndex)), SLOT(currentChanged(QModelIndex, QModelIndex)));
    connect(m_selection, SIGNAL(selectionChanged(QItemSelection, QItemSelection)), SLOT(selectionChanged(QItemSelection, QItemSelection)));
}

SettingsWidget::~SettingsWidget()
{
    delete ui;
}

void SettingsWidget::setRootGroup(Group *group)
{
    m_model->setRootGroup(group);
}

void SettingsWidget::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)

    LOG_TRACE("SettingsWidget::currentChanged", current.data());

    if (current.isValid())
    {
        ProfileItem *item = static_cast<ProfileItem *>(current.internalPointer());
        if (item)
        {
            if (m_editor)
            {
                ui->layoutEdit->removeWidget(m_editor);

                delete m_editor;
            }

            m_editor = new EditVariable(this);
            ui->layoutEdit->removeWidget(m_defaultForm);
            ui->layoutEdit->addWidget(m_editor);
        }
    }
}

void SettingsWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)

    LOG_TRACE("SettingsWidget::selectionChanged", selected.count());

    if (selected.isEmpty() && m_editor)
    {
        ui->layoutEdit->removeWidget(m_editor);
        ui->layoutEdit->addWidget(m_defaultForm);

        delete m_editor;
        m_editor = 0;
    }
}
