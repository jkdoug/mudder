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
#include "profileitem.h"
#include "profileitemfactory.h"
#include "group.h"
#include "variable.h"
#include "editsetting.h"

SettingsWidget::SettingsWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SettingsWidget)
{
    ui->setupUi(this);

    m_layoutEdit = new QStackedLayout;
    ui->layoutWidget->addLayout(m_layoutEdit);

    QWidget *defaultForm = new QWidget(this);
    QVBoxLayout *layout = new QVBoxLayout(defaultForm);
    QLabel *label = new QLabel(tr("Select an item in the tree."), defaultForm);
    layout->addWidget(label);
    layout->setAlignment(label, Qt::AlignCenter);
    defaultForm->setLayout(layout);
    m_layoutEdit->addWidget(defaultForm);

    QStringList editorTypes;
    editorTypes << "accelerator" << "alias" << "event" << "group" << "timer" << "trigger" << "variable";
    foreach (QString editorType, editorTypes)
    {
        EditSetting *editor = ProfileItemFactory::editor(editorType);
        if (!editor)
        {
            LOG_ERROR(tr("Invalid profile item editor requested: %1").arg(editorType));
            continue;
        }

        int index = m_layoutEdit->addWidget(editor);
        m_editors.insert(editorType, index);
    }

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

void SettingsWidget::updateCurrentItem(bool changed, bool valid)
{
    LOG_TRACE("SettingsWidget::updateCurrentItem", sender()->metaObject()->className(), changed, valid);

    emit settingModified(changed, valid);
}

void SettingsWidget::saveCurrentItem()
{
    QModelIndex current(m_selection->currentIndex());
    if (!current.isValid())
    {
        return;
    }

    ProfileItem *item = static_cast<ProfileItem *>(current.internalPointer());
    Q_ASSERT(item != 0);
    if (!item)
    {
        LOG_ERROR(tr("Attempted to save an invalid profile item."));
        return;
    }

    Q_ASSERT(m_editors.contains(item->tagName()));
    if (!m_editors.contains(item->tagName()))
    {
        LOG_WARNING(tr("No editor in place for %1 items.").arg(item->tagName()));
        return;
    }

    int index = m_editors[item->tagName()];
    EditSetting *editor = qobject_cast<EditSetting *>(m_layoutEdit->widget(index));
    Q_ASSERT(editor != 0);

    if (!editor->save(item))
    {
        LOG_WARNING(tr("Something got in the way of saving %1 called %2.").arg(item->tagName()).arg(item->fullName()));
    }
}

void SettingsWidget::discardCurrentItem()
{

}

void SettingsWidget::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)

    LOG_TRACE("SettingsWidget::currentChanged", current.data());

    for (int page = 0; page < m_layoutEdit->count(); page++)
    {
        EditSetting *editor = qobject_cast<EditSetting *>(m_layoutEdit->widget(page));
        disconnect(editor, 0, this, 0);
    }

    int index = 0;
    if (current.isValid())
    {
        ProfileItem *item = static_cast<ProfileItem *>(current.internalPointer());
        Q_ASSERT(m_editors.contains(item->tagName()));

        index = m_editors[item->tagName()];

        EditSetting *editor = qobject_cast<EditSetting *>(m_layoutEdit->widget(index));
        editor->load(item);

        connect(editor, SIGNAL(itemModified(bool, bool)), SLOT(updateCurrentItem(bool, bool)));
    }

    m_layoutEdit->setCurrentIndex(index);
}

void SettingsWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)

    LOG_TRACE("SettingsWidget::selectionChanged", selected.count());

    if (selected.isEmpty())
    {
        m_layoutEdit->setCurrentIndex(0);
    }
}
