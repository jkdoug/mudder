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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "logger.h"
#include "coreapplication.h"
#include <QtDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QThread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(CoreApplication::instance(), SIGNAL(busyStateChanged(bool)), SLOT(onBusyStateChanged(bool)));

    initializeRecentFiles();

    LOG_INFO(tr("Main window initialized."));
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::onBusyStateChanged(bool busy)
{
    if (busy)
    {
        CoreApplication::setOverrideCursor(QCursor(Qt::WaitCursor));
    }
    else
    {
        CoreApplication::restoreOverrideCursor();
    }
}

void MainWindow::onRecentFile(const QString &fileName)
{
    // TODO

    LOG_INFO("onRecentFile", fileName);

    SETTINGS->removeRecentFile(fileName);
    SETTINGS->addRecentFile(fileName);
}

void MainWindow::onRecentFilesChanged(const QStringList &fileNames)
{
    int numRecentFiles = fileNames.size();

    int count = 1;
    for (int i = 0; i < numRecentFiles; i++)
    {
        if (QFile::exists(fileNames[i]))
        {
            m_recentFileActions[i]->setText(tr("&%1 %2").arg(count++).arg(QFileInfo(fileNames[i]).fileName()));
            m_recentFileActions[i]->setVisible(true);
            m_recentSignalMapper->setMapping(m_recentFileActions[i], fileNames[i]);
        }
        else
        {
            m_recentFileActions[i]->setVisible(false);
        }
    }

    for (int j = numRecentFiles; j < CoreSettings::MaxRecentFiles; j++)
    {
        m_recentFileActions[j]->setVisible(false);
    }
}

void MainWindow::on_actionOpen_triggered()
{
    QString fileName(QFileDialog::getOpenFileName(this, tr("Open File"), QString(), tr("Mudder profiles (*.mp);;Lua script files (*.lua);;All files (*)")));
    if (fileName.isEmpty())
    {
        return;
    }

    // TODO

    LOG_INFO("on_actionOpen_triggered", fileName);

    SETTINGS->removeRecentFile(fileName);
    SETTINGS->addRecentFile(fileName);
}

void MainWindow::on_actionSave_triggered()
{
    // TODO

    LOG_INFO("on_actionSave_triggered");

//    CoreApplication::setApplicationBusy(true);
//    QThread::sleep(7);
//    CoreApplication::setApplicationBusy(false);
}

void MainWindow::on_actionExit_triggered()
{
    LOG_INFO("Main window closing.");
    close();
}

void MainWindow::initializeRecentFiles()
{
    m_recentSignalMapper = new QSignalMapper(this);
    QMenu *menu = ui->menuFile;
    menu->setSeparatorsCollapsible(true);
    for (int i = 0; i < CoreSettings::MaxRecentFiles; i++)
    {
        m_recentFileActions[i] = new QAction(this);
        m_recentFileActions[i]->setVisible(false);
        menu->insertAction(ui->actionExit, m_recentFileActions[i]);

        connect(m_recentFileActions[i], SIGNAL(triggered()), m_recentSignalMapper, SLOT(map()));
    }
    menu->insertSeparator(ui->actionExit);

    connect(m_recentSignalMapper, SIGNAL(mapped(QString)), SLOT(onRecentFile(QString)));
    connect(SETTINGS, SIGNAL(recentFilesChanged(QStringList)), SLOT(onRecentFilesChanged(QStringList)));

    onRecentFilesChanged(SETTINGS->recentFileList());
}
