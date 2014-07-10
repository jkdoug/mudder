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


#include <QClipboard>
#include <QLabel>
#include <QMenu>
#include "settingswindow.h"
#include "ui_settingswindow.h"
#include "logging.h"
#include "profile.h"
#include "profileitem.h"
#include "profileitemfactory.h"
#include "variable.h"
#include "editsetting.h"
#include "richtextdelegate.h"

SettingsWindow::SettingsWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SettingsWindow),
    m_proxy(0)
{
    ui->setupUi(this);

    ui->treeView->setItemDelegateForColumn(0, new RichTextDelegate(this));

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

    m_newItem = new QMenu(tr("&New"), this);
    m_newItem->addAction(newAccelerator);
    m_newItem->addAction(newAlias);
    m_newItem->addAction(newEvent);
    m_newItem->addAction(newGroup);
    m_newItem->addAction(newTimer);
    m_newItem->addAction(newTrigger);
    m_newItem->addAction(newVariable);

    m_buttonNew = new QToolButton(this);
    m_buttonNew->setDefaultAction(newTrigger);
    m_buttonNew->setEnabled(false);
    m_buttonNew->setPopupMode(QToolButton::MenuButtonPopup);
    m_buttonNew->setMenu(m_newItem);

    ui->toolBar->insertWidget(ui->actionDelete, m_buttonNew);
    ui->toolBar->insertSeparator(ui->actionDelete);

    connect(ui->actionEnabled, SIGNAL(triggered(bool)), SLOT(enableItem(bool)));
    connect(ui->actionDelete, SIGNAL(triggered()), SLOT(deleteItem()));
    connect(ui->actionCut, SIGNAL(triggered()), SLOT(cutItem()));
    connect(ui->actionCopy, SIGNAL(triggered()), SLOT(copyItem()));
    connect(ui->actionPaste, SIGNAL(triggered()), SLOT(pasteItem()));

    connect(ui->actionApply, SIGNAL(triggered()), SLOT(saveCurrentItem()));
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
    connect(ui->treeView, SIGNAL(customContextMenuRequested(QPoint)), SLOT(showContextMenu(QPoint)));
    connect(ui->filter, SIGNAL(textChanged(QString)), SLOT(filterTextChanged(QString)));
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

        if (m_proxy)
        {
            delete m_proxy;
        }
        m_proxy = new SettingsFilterModel(this);
        m_proxy->setSourceModel(p);

        ui->treeView->setModel(m_proxy);

        connect(ui->treeView->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), SLOT(currentChanged(QModelIndex,QModelIndex)));
        connect(ui->treeView->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), SLOT(selectionChanged(QItemSelection,QItemSelection)));
    }

    m_buttonNew->setEnabled(p != 0);

    if (!p)
    {
        ui->actionDelete->setEnabled(false);
        ui->actionCopy->setEnabled(false);
        ui->actionCut->setEnabled(false);
        ui->actionPaste->setEnabled(false);

        ui->actionApply->setEnabled(false);
        ui->actionDiscard->setEnabled(false);

        m_stackedEditors->setCurrentIndex(0);
    }
}

Profile * SettingsWindow::profile() const
{
    if (!m_proxy)
    {
        return 0;
    }

    return qobject_cast<Profile*>(m_proxy->sourceModel());
}

void SettingsWindow::settingModified(bool changed, bool valid)
{
    ui->actionApply->setEnabled(changed && valid);
    ui->actionDiscard->setEnabled(changed);
}

void SettingsWindow::enableItem(bool on)
{
    QModelIndex current(sourceIndex(ui->treeView->currentIndex()));
    if (!current.isValid())
    {
        return;
    }

    ProfileItem *item = profile()->itemForIndex(current);
    Q_ASSERT(item);
    if (!item)
    {
        qCWarning(MUDDER_PROFILE) << "Attempted to toggle an invalid profile item.";
        return;
    }

    item->enable(on);

    int index = m_editors[item->tagName()];
    EditSetting *editor = qobject_cast<EditSetting *>(m_stackedEditors->widget(index));
    editor->enableItem(on);
}

void SettingsWindow::deleteItem()
{
    QModelIndex current(sourceIndex(ui->treeView->currentIndex()));
    if (!current.isValid())
    {
        return;
    }

    profile()->removeRow(current.row(), current.parent());
}

void SettingsWindow::cutItem()
{
    QModelIndex current(sourceIndex(ui->treeView->currentIndex()));
    QModelIndex future(proxyIndex(profile()->cutItem(current)));
    setCurrentIndex(future);
}

void SettingsWindow::copyItem()
{
    QModelIndex current(sourceIndex(ui->treeView->currentIndex()));
    profile()->copyItem(current);
}

void SettingsWindow::pasteItem()
{
    QModelIndex current(sourceIndex(ui->treeView->currentIndex()));
    QModelIndex future(proxyIndex(profile()->pasteItem(current)));
    setCurrentIndex(future);
}

void SettingsWindow::clipboardChanged()
{
    ui->actionPaste->setEnabled(Profile::validateXml(QApplication::clipboard()->text()));
}

void SettingsWindow::saveCurrentItem()
{
    QModelIndex current(sourceIndex(ui->treeView->currentIndex()));
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
    QModelIndex current(sourceIndex(ui->treeView->currentIndex()));
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

void SettingsWindow::addItem(const QString &type, QAction *action)
{
    QModelIndex current(sourceIndex(ui->treeView->currentIndex()));
    ProfileItem *item = ProfileItemFactory::create(type);
    QModelIndex future(proxyIndex(profile()->newItem(item, current)));
    setCurrentIndex(future);

    if (action)
    {
        m_buttonNew->setDefaultAction(action);
    }
}

void SettingsWindow::addAccelerator()
{
    addItem("accelerator", qobject_cast<QAction*>(sender()));
}

void SettingsWindow::addAlias()
{
    addItem("alias", qobject_cast<QAction*>(sender()));
}

void SettingsWindow::addEvent()
{
    addItem("event", qobject_cast<QAction*>(sender()));
}

void SettingsWindow::addGroup()
{
    addItem("group", qobject_cast<QAction*>(sender()));
}

void SettingsWindow::addTimer()
{
    addItem("timer", qobject_cast<QAction*>(sender()));
}

void SettingsWindow::addTrigger()
{
    addItem("trigger", qobject_cast<QAction*>(sender()));
}

void SettingsWindow::addVariable()
{
    addItem("variable", qobject_cast<QAction*>(sender()));
}


void SettingsWindow::currentChanged(const QModelIndex &current, const QModelIndex &previous)
{
    Q_UNUSED(previous)

    int index = 0;
    if (current.isValid())
    {
        ProfileItem *item = profile()->itemForIndex(sourceIndex(current));
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

    bool selection = !selected.isEmpty();
    ui->actionCopy->setEnabled(selection);
    ui->actionCut->setEnabled(selection);
    ui->actionDelete->setEnabled(selection);

    if (!selection)
    {
        m_stackedEditors->setCurrentIndex(0);
    }
}

void SettingsWindow::showContextMenu(const QPoint &point)
{
    if (!profile())
    {
        return;
    }

    QMenu * popup = new QMenu(this);

    popup->addAction(m_newItem->menuAction());
    popup->addSeparator();

    QModelIndex selected(ui->treeView->indexAt(point));
    if (selected.isValid())
    {
        ProfileItem *item = profile()->itemForIndex(sourceIndex(selected));
        if (item && !qobject_cast<Variable*>(item))
        {
            ui->actionEnabled->setChecked(item->enabledFlag());

            popup->addAction(ui->actionEnabled);
            popup->addSeparator();
        }

        popup->addAction(ui->actionDelete);
        popup->addAction(ui->actionCut);
        popup->addAction(ui->actionCopy);
    }

    popup->addAction(ui->actionPaste);

    popup->exec(ui->treeView->mapToGlobal(point));

    delete popup;
}

void SettingsWindow::filterTextChanged(const QString &text)
{
    m_proxy->setFilterWildcard(text);
}

QModelIndex SettingsWindow::proxyIndex(const QModelIndex &modelIndex)
{
    if (modelIndex.model() != m_proxy)
    {
        return m_proxy->mapFromSource(modelIndex);
    }

    return modelIndex;
}

QModelIndex SettingsWindow::sourceIndex(const QModelIndex &modelIndex)
{
    if (modelIndex.model() == m_proxy)
    {
        return m_proxy->mapToSource(modelIndex);
    }

    return modelIndex;
}

void SettingsWindow::setCurrentIndex(const QModelIndex &index)
{
    ui->treeView->setCurrentIndex(index);
    ui->treeView->scrollTo(index);
}
