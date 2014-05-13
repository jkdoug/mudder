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


#include <QLabel>
#include "settingswidget.h"
#include "ui_settingswidget.h"
#include "logging.h"
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

    m_stackedEditors = new QStackedWidget(this);
    ui->splitter->addWidget(m_stackedEditors);

    QWidget *defaultForm = new QWidget(m_stackedEditors);
    QVBoxLayout *layout = new QVBoxLayout(defaultForm);
    QLabel *label = new QLabel(tr("Select an item in the tree."), defaultForm);
    layout->addWidget(label);
    layout->setAlignment(label, Qt::AlignCenter);
    defaultForm->setLayout(layout);
    m_stackedEditors->addWidget(defaultForm);

    QStringList editorTypes;
    editorTypes << "accelerator" << "alias" << "event" << "group" << "timer" << "trigger" << "variable";
    foreach (QString editorType, editorTypes)
    {
        EditSetting *editor = ProfileItemFactory::editor(editorType);
        if (!editor)
        {
            qCWarning(MUDDER_PROFILE) << "Invalid profile item editor requested:" << editorType;
            continue;
        }

        int index = m_stackedEditors->addWidget(editor);
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

Group * SettingsWidget::rootGroup() const
{
    return m_model->rootGroup();
}

void SettingsWidget::setRootGroup(Group *group)
{
    m_selection->clear();
    m_model->setRootGroup(group);
}

void SettingsWidget::addItem(ProfileItem *item)
{
    QModelIndex current(m_selection->currentIndex());
    if (current.isValid())
    {
        ProfileItem *parentItem = static_cast<ProfileItem *>(current.internalPointer());
        Group *parentGroup = qobject_cast<Group *>(parentItem);
        if (!parentGroup)
        {
            current = current.parent();
        }
    }

    static quint32 counter = 0;
    item->setName(QString("New Item %1").arg(++counter));

    QModelIndex index(m_model->appendItem(item, current));
    if (index.isValid())
    {
        qCDebug(MUDDER_PROFILE) << "Appended new profile item" << item->name() << "@" << index;

        m_selection->clear();
        m_selection->setCurrentIndex(index, QItemSelectionModel::SelectCurrent);
    }
}

void SettingsWidget::updateCurrentItem(bool changed, bool valid)
{
    qCDebug(MUDDER_PROFILE) << "Update current item on settings widget:" << sender()->metaObject()->className() << changed << valid;

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
        qCWarning(MUDDER_PROFILE) << "Attempted to save an invalid profile item.";
        return;
    }

    Q_ASSERT(m_editors.contains(item->tagName()));
    if (!m_editors.contains(item->tagName()))
    {
        qCWarning(MUDDER_PROFILE) << "No editor found:" << item->tagName();
        return;
    }

    int index = m_editors[item->tagName()];
    EditSetting *editor = qobject_cast<EditSetting *>(m_stackedEditors->widget(index));
    Q_ASSERT(editor != 0);

    bool saved = editor->save(item);
    if (!saved)
    {
        qCWarning(MUDDER_PROFILE) << "Something prevented saving:" << item->tagName() << "->" << item->fullName();
    }
    else
    {
        emit settingSaved();
    }

    emit settingModified(false, saved);
}

void SettingsWidget::discardCurrentItem()
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
        qCWarning(MUDDER_PROFILE) << "Attempted to discard an invalid profile item";
        return;
    }

    Q_ASSERT(m_editors.contains(item->tagName()));
    if (!m_editors.contains(item->tagName()))
    {
        qCWarning(MUDDER_PROFILE) << "No editor found:" << item->tagName();
        return;
    }

    int index = m_editors[item->tagName()];
    EditSetting *editor = qobject_cast<EditSetting *>(m_stackedEditors->widget(index));
    Q_ASSERT(editor != 0);

    bool loaded = editor->load(item);
    if (!loaded)
    {
        qCWarning(MUDDER_PROFILE) << "Something prevented loading:" << item->tagName() << "->" << item->fullName();
    }
    else
    {
        emit settingDiscarded();
    }

    emit settingModified(false, loaded);
}

void SettingsWidget::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)

    qCDebug(MUDDER_PROFILE) << "Current settings index changed:" << current.data();

    int index = 0;
    if (current.isValid())
    {
        ProfileItem *item = static_cast<ProfileItem *>(current.internalPointer());
        Q_ASSERT(m_editors.contains(item->tagName()));

        index = m_editors[item->tagName()];

        EditSetting *editor = qobject_cast<EditSetting *>(m_stackedEditors->widget(index));
        editor->load(item);

        connect(editor, SIGNAL(itemModified(bool, bool)), SLOT(updateCurrentItem(bool, bool)));
    }

    m_stackedEditors->setCurrentIndex(index);
}

void SettingsWidget::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)

    qCDebug(MUDDER_PROFILE) << "Settings selection changed:" << selected.indexes() << deselected.indexes();

    if (selected.isEmpty())
    {
        m_stackedEditors->setCurrentIndex(0);
    }
}
