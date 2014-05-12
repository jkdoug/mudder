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


#include <QMenu>
#include "settingswindow.h"
#include "ui_settingswindow.h"
#include "logging.h"
#include "group.h"
#include "profileitemfactory.h"

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

    connect(ui->editor, SIGNAL(settingModified(bool, bool)), SLOT(settingModified(bool, bool)));
    connect(ui->actionSave, SIGNAL(triggered()), ui->editor, SLOT(saveCurrentItem()));
    connect(ui->actionDiscard, SIGNAL(triggered()), ui->editor, SLOT(discardCurrentItem()));
}

SettingsWindow::~SettingsWindow()
{
    delete ui;
}

void SettingsWindow::setRootGroup(Group *group)
{
    ui->editor->setRootGroup(group);
    m_buttonNew->setEnabled(group != 0);
}

Group * SettingsWindow::rootGroup() const
{
    return ui->editor->rootGroup();
}

void SettingsWindow::settingModified(bool changed, bool valid)
{
    qCDebug(MUDDER_PROFILE) << "Settings window modified:" << changed << valid;

    ui->actionSave->setEnabled(changed && valid);
    ui->actionDiscard->setEnabled(changed);
}

void SettingsWindow::addAccelerator()
{
    ui->editor->addItem(ProfileItemFactory::create("accelerator"));
}

void SettingsWindow::addAlias()
{
    ui->editor->addItem(ProfileItemFactory::create("alias"));
}

void SettingsWindow::addEvent()
{
    ui->editor->addItem(ProfileItemFactory::create("event"));
}

void SettingsWindow::addGroup()
{
    ui->editor->addItem(ProfileItemFactory::create("group"));
}

void SettingsWindow::addTimer()
{
    ui->editor->addItem(ProfileItemFactory::create("timer"));
}

void SettingsWindow::addTrigger()
{
    ui->editor->addItem(ProfileItemFactory::create("trigger"));
}

void SettingsWindow::addVariable()
{
    ui->editor->addItem(ProfileItemFactory::create("variable"));
}
