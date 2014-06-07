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
#include <QMenu>
#include "settingswindow.h"
#include "ui_settingswindow.h"
#include "logging.h"
#include "profile.h"
#include "profileitem.h"
#include "profileitemfactory.h"
#include "editsetting.h"
#include <QClipboard>

SettingsWindow::SettingsWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    QAction *newAccelerator = new QAction(tr("A&ccelerator"), this);
    newAccelerator->setStatusTip(tr("Create a new key accelerator"));
    newAccelerator->setIcon(QIcon(":/icons/accelerator"));
    connect(newAccelerator, SIGNAL(triggered()), SLOT(addAccelerator()));

    QAction *newAlias = new QAction(tr("&Alias"), this);
    newAlias->setStatusTip(tr("Create a new alias"));
    newAlias->setIcon(QIcon(":/icons/alias"));
    connect(newAlias, SIGNAL(triggered()), SLOT(addAlias()));

    QAction *newEvent = new QAction(tr("&Event"), this);
    newEvent->setStatusTip(tr("Create a new event handler"));
    newEvent->setIcon(QIcon(":/icons/event"));
    connect(newEvent, SIGNAL(triggered()), SLOT(addEvent()));

    QAction *newGroup = new QAction(tr("&Group"), this);
    newGroup->setStatusTip(tr("Create a new group"));
    newGroup->setIcon(QIcon(":/icons/group"));
    connect(newGroup, SIGNAL(triggered()), SLOT(addGroup()));

    QAction *newTimer = new QAction(tr("&Timer"), this);
    newTimer->setStatusTip(tr("Create a new timer"));
    newTimer->setIcon(QIcon(":/icons/timer"));
    connect(newTimer, SIGNAL(triggered()), SLOT(addTimer()));

    QAction *newTrigger = new QAction(tr("T&rigger"), this);
    newTrigger->setStatusTip(tr("Create a new trigger"));
    newTrigger->setIcon(QIcon(":/icons/trigger"));
    connect(newTrigger, SIGNAL(triggered()), SLOT(addTrigger()));

    QAction *newVariable = new QAction(tr("&Variable"), this);
    newVariable->setStatusTip(tr("Create a new variable"));
    newVariable->setIcon(QIcon(":/icons/variable"));
    connect(newVariable, SIGNAL(triggered()), SLOT(addVariable()));

    QMenu *menuNew = new QMenu(this);
    menuNew->addAction(newAccelerator);
    menuNew->addAction(newAlias);
    menuNew->addAction(newEvent);
    menuNew->addAction(newGroup);
    menuNew->addAction(newTimer);
    menuNew->addAction(newTrigger);
    menuNew->addAction(newVariable);

    m_buttonNew = new QToolButton(this);
    m_buttonNew->setDefaultAction(newTrigger);
    m_buttonNew->setEnabled(false);
    m_buttonNew->setPopupMode(QToolButton::MenuButtonPopup);
    m_buttonNew->setMenu(menuNew);

    ui->toolBar->insertWidget(ui->actionSave, m_buttonNew);
    ui->toolBar->insertSeparator(ui->actionSave);

    connect(ui->actionCut, SIGNAL(triggered()), SLOT(cut()));
    connect(ui->actionCopy, SIGNAL(triggered()), SLOT(copy()));
    connect(ui->actionPaste, SIGNAL(triggered()), SLOT(paste()));

    connect(ui->actionSave, SIGNAL(triggered()), SLOT(saveCurrentItem()));
    connect(ui->actionDiscard, SIGNAL(triggered()), SLOT(discardCurrentItem()));

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

        connect(editor, SIGNAL(itemModified(bool, bool)), SLOT(settingModified(bool, bool)));
    }

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), SLOT(clipboardChanged()));
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::setProfile(Profile *p)
{
    if (p != profile())
    {
        if (ui->treeView->selectionModel())
        {
            ui->treeView->selectionModel()->disconnect(this);
        }

        ui->treeView->setModel(p);

        connect(ui->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(currentChanged(QModelIndex,QModelIndex)));
        connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(selectionChanged(QItemSelection,QItemSelection)));
    }

    m_buttonNew->setEnabled(p != 0);
}

Profile * SettingsWindow::profile() const
{
    return qobject_cast<Profile*>(ui->treeView->model());
}

void SettingsWindow::settingModified(bool changed, bool valid)
{
    qCDebug(MUDDER_PROFILE) << "Settings window modified:" << changed << valid;

    ui->actionSave->setEnabled(changed && valid);
    ui->actionDiscard->setEnabled(changed);
}

void SettingsWindow::cut()
{
    ui->treeView->setCurrentIndex(profile()->cutItem(ui->treeView->currentIndex()));
}

void SettingsWindow::copy()
{
    profile()->copyItem(ui->treeView->currentIndex());
}

void SettingsWindow::paste()
{
    ui->treeView->setCurrentIndex(profile()->pasteItem(ui->treeView->currentIndex()));
}

void SettingsWindow::clipboardChanged()
{
    ui->actionPaste->setEnabled(Profile::validateXml(QApplication::clipboard()->text()));
}

void SettingsWindow::saveCurrentItem()
{
    QModelIndex current(ui->treeView->currentIndex());
    if (!current.isValid())
    {
        return;
    }

    ProfileItem *item = profile()->itemForIndex(current);
    Q_ASSERT(item);
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
        emit profile()->settingsChanged();
    }

    settingModified(false, saved);
}

void SettingsWindow::discardCurrentItem()
{
    QModelIndex current(ui->treeView->currentIndex());
    if (!current.isValid())
    {
        return;
    }

    ProfileItem *item = profile()->itemForIndex(current);
    Q_ASSERT(item);
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

    settingModified(false, loaded);
}

void SettingsWindow::addAccelerator()
{
    ui->treeView->setCurrentIndex(profile()->newItem(ProfileItemFactory::create("accelerator"), ui->treeView->currentIndex()));
}

void SettingsWindow::addAlias()
{
    ui->treeView->setCurrentIndex(profile()->newItem(ProfileItemFactory::create("alias"), ui->treeView->currentIndex()));
}

void SettingsWindow::addEvent()
{
    ui->treeView->setCurrentIndex(profile()->newItem(ProfileItemFactory::create("event"), ui->treeView->currentIndex()));
}

void SettingsWindow::addGroup()
{
    ui->treeView->setCurrentIndex(profile()->newItem(ProfileItemFactory::create("group"), ui->treeView->currentIndex()));
}

void SettingsWindow::addTimer()
{
    ui->treeView->setCurrentIndex(profile()->newItem(ProfileItemFactory::create("timer"), ui->treeView->currentIndex()));
}

void SettingsWindow::addTrigger()
{
    ui->treeView->setCurrentIndex(profile()->newItem(ProfileItemFactory::create("trigger"), ui->treeView->currentIndex()));
}

void SettingsWindow::addVariable()
{
    ui->treeView->setCurrentIndex(profile()->newItem(ProfileItemFactory::create("variable"), ui->treeView->currentIndex()));
}


void SettingsWindow::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    qCDebug(MUDDER_PROFILE) << "Current settings index changed:" << previous << current;

    int index = 0;
    if (current.isValid())
    {
        ProfileItem *item = profile()->itemForIndex(current);
        Q_ASSERT(item);
        Q_ASSERT(m_editors.contains(item->tagName()));

        index = m_editors[item->tagName()];

        EditSetting *editor = qobject_cast<EditSetting *>(m_stackedEditors->widget(index));
        editor->load(item);
    }

    m_stackedEditors->setCurrentIndex(index);
}

void SettingsWindow::selectionChanged(const QItemSelection &selected, const QItemSelection &deselected)
{
    Q_UNUSED(deselected)

    qCDebug(MUDDER_PROFILE) << "selectionChanged";

    bool selection = !selected.isEmpty();
    ui->actionCopy->setEnabled(selection);
    ui->actionCut->setEnabled(selection);

    if (!selection)
    {
        m_stackedEditors->setCurrentIndex(0);
    }
}
