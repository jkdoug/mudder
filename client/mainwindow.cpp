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


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "lua.hpp"
#include "logging.h"
#include "codeeditorwindow.h"
#include "coreapplication.h"
#include "console.h"
#include "contextmanager.h"
#include "dialogprofile.h"
#include "settingswindow.h"
#include <QtDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMdiSubWindow>
#include <QMessageBox>
#include <QThread>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    qCDebug(MUDDER_APP) << "Setting up main window UI.";

    ui->setupUi(this);

    setObjectName("app.mudder.MainWindow");
    CONTEXT_MANAGER->registerContext(objectName());
    CONTEXT_MANAGER->appendContext(objectName());

    m_editor = new CodeEditorWindow(this);
    ui->dockEditor->setWidget(m_editor);
    ui->menuWindow->addAction(ui->dockEditor->toggleViewAction());

    m_settings = new SettingsWindow(this);
    ui->dockSettings->setWidget(m_settings);
    ui->menuWindow->addAction(ui->dockSettings->toggleViewAction());

    QTabBar *mdiTabBar = ui->mdiArea->findChild<QTabBar *>();
    mdiTabBar->setExpanding(false);
    mdiTabBar->setIconSize(QSize(0, 0));

    connect(ui->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), SLOT(updateActions()));

    connect(CoreApplication::instance(), SIGNAL(busyStateChanged(bool)), SLOT(onBusyStateChanged(bool)));

    initializeRecentFiles();

    m_windowActions = new QActionGroup(this);

    restoreGeometry(SETTINGS->value("MainWindow/Geometry").toByteArray());
    restoreState(SETTINGS->value("MainWindow/State").toByteArray());

    updateActions();

    qCDebug(MUDDER_APP) << "Main window initialized.";
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    ui->mdiArea->closeAllSubWindows();
    if (!ui->mdiArea->subWindowList().isEmpty())
    {
        qCDebug(MUDDER_APP) << "Main window will not close yet; console windows still open.";

        e->ignore();
    }
    else if (!m_editor->close())
    {
        qCDebug(MUDDER_APP) << "Main window will not close yet; editor windows still open.";

        e->ignore();
        return;
    }
    else
    {
        qCDebug(MUDDER_APP) << "Main window saving state and closing.";

        SETTINGS->setValue("MainWindow/Geometry", saveGeometry());
        SETTINGS->setValue("MainWindow/State", saveState());

        e->accept();
    }
}

void MainWindow::onBusyStateChanged(bool busy)
{
    if (busy)
    {
        CoreApplication::setOverrideCursor(Qt::WaitCursor);
    }
    else
    {
        CoreApplication::restoreOverrideCursor();
    }
}

void MainWindow::onRecentFile(const QString &fileName)
{
    if (QFileInfo(fileName).suffix().compare("mp", Qt::CaseInsensitive) == 0)
    {
        openConsole(fileName);
    }
    else
    {
        m_editor->openEditor(fileName);
    }
}

void MainWindow::onRecentFilesChanged(const QStringList &fileNames)
{
    int numRecentFiles = fileNames.size();

    int count = 1;
    for (int i = 0; i < numRecentFiles; i++)
    {
        if (QFile::exists(fileNames[i]))
        {
            QFileInfo fi(fileNames[i]);
            m_recentFileActions[i]->setText(tr("&%1 %2").arg(count++).arg(fi.fileName()));
            m_recentFileActions[i]->setStatusTip(fi.canonicalFilePath());
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

void MainWindow::on_actionNew_triggered()
{
    newConsole();
}

void MainWindow::on_actionOpen_triggered()
{
    openConsole();
}

void MainWindow::on_actionSave_triggered()
{
    saveConsole();
}

void MainWindow::on_actionSaveAs_triggered()
{
    Console *console = activeConsole();
    if (console)
    {
        if (console->saveAs())
        {
            SETTINGS->addRecentFile(console->fileName());
        }
    }
}

void MainWindow::on_actionConnect_triggered(bool checked)
{
    Console *console = activeConsole();
    if (console)
    {
        if (checked)
        {
            console->connectToServer();
        }
        else
        {
            console->disconnectFromServer();
        }
    }
}

void MainWindow::on_actionOptions_triggered()
{
    Console *console = activeConsole();
    if (!console)
    {
        return;
    }

    DialogProfile *dlg = new DialogProfile(this);
    dlg->load(console->profile());
    if (dlg->exec() == QDialog::Accepted &&
        console->profile()->options() != dlg->profile()->options())
    {
        console->profile()->setOptions(dlg->profile()->options());
        console->setWindowTitle(console->profile()->name() + "[*]");
    }
}

void MainWindow::on_actionClose_triggered()
{
    ui->mdiArea->closeActiveSubWindow();
}

void MainWindow::on_actionCloseAll_triggered()
{
    ui->mdiArea->closeAllSubWindows();
}

void MainWindow::on_actionNext_triggered()
{
    ui->mdiArea->activateNextSubWindow();
}

void MainWindow::on_actionPrevious_triggered()
{
    ui->mdiArea->activatePreviousSubWindow();
}

void MainWindow::on_actionExit_triggered()
{
    close();
}

void MainWindow::on_actionAbout_triggered()
{
    QString msg(tr("<p><b>%1, Version %2</b><br/>Compiled %3 %4</p>")
                .arg(QApplication::applicationDisplayName())
                .arg(QApplication::applicationVersion())
                .arg(__DATE__).arg(__TIME__));
    msg += QString("<p>%1<br/>%2</p>").arg("Copyright (C) 2014 Jason K. Douglas").arg("<a href='https://github.com/jkdoug/mudder'>https://github.com/jkdoug/mudder</a>");
    msg += QString("<p>%1<br/>%2<br/>%3</p>").arg(LUA_COPYRIGHT).arg(LUA_AUTHORS).arg("<a href='http://lua.org'>http://lua.org</a>");
    msg += QString("<p>%1<br/>%2</p>").arg("LuaBridge 2.0 Copyright (C) 2012 Vinnie Falco").arg("<a href='https://github.com/vinniefalco/LuaBridge'>https://github.com/vinniefalco/LuaBridge</a>");
    msg += QString("<p>%1<br/>%2</p>").arg("Portions inspired by Jaco Naude's Qtilities project").arg("<a href='https://github.com/JPNaude'>https://github.com/JPNaude/Qtilities</a>");

    QMessageBox::about(this, tr("About"), msg);
}

void MainWindow::on_actionAboutQt_triggered()
{
    CoreApplication::aboutQt();
}

void MainWindow::updateActions()
{
    bool hasConsole = activeConsole() != 0;
    bool modified = hasConsole && activeConsole()->isWindowModified();

    qCDebug(MUDDER_APP) << tr("Updating actions:") << hasConsole << modified;

    ui->actionSave->setEnabled(hasConsole && modified);
    ui->actionSaveAs->setEnabled(hasConsole);
    ui->actionClose->setEnabled(hasConsole);
    ui->actionCloseAll->setEnabled(hasConsole);

    ui->menuGame->menuAction()->setVisible(hasConsole);
    ui->actionConnect->setVisible(hasConsole);
    ui->actionOptions->setVisible(hasConsole);

    if (hasConsole)
    {
        Console *console = activeConsole();
        console->windowAction()->setChecked(true);

        bool con = console->isConnected() || console->isConnecting();
        ui->actionConnect->setChecked(con);
        if (con)
        {
            ui->actionConnect->setText(tr("&Disconnect"));
            ui->actionConnect->setToolTip(tr("Disconnect from game server"));
        }
        else
        {
            ui->actionConnect->setText(tr("&Connect"));
            ui->actionConnect->setToolTip(tr("Connect to game server"));
        }

        if (console->isConnected())
        {
            ui->actionConnect->setIcon(QIcon(":/icons/connected"));
        }
        else
        {
            ui->actionConnect->setIcon(QIcon(":/icons/disconnected"));
        }

        m_settings->setProfile(console->profile());
    }
    else
    {
        m_settings->setProfile(0);
    }
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

void MainWindow::newConsole()
{
    Console *console = new Console;
    console->newFile();
    addConsole(console);
}

void MainWindow::openConsole(const QString &fileName)
{
    Console *console = 0;
    if (fileName.isEmpty())
    {
        console = Console::open(this);
    }
    else
    {
        console = Console::openFile(fileName, this);
    }

    if (console)
    {
        SETTINGS->addRecentFile(console->fileName());

        addConsole(console);
    }
}

void MainWindow::saveConsole()
{
    if (activeConsole())
    {
        if (activeConsole()->save())
        {
            updateActions();
        }
    }
}

void MainWindow::addConsole(Console *console)
{
    m_settings->setProfile(console->profile());

    connect(console, SIGNAL(connectionStatusChanged(bool)), SLOT(updateActions()));
    connect(console, SIGNAL(modified()), SLOT(updateActions()));

    if (ui->mdiArea->subWindowList().isEmpty())
    {
        static bool separated = false;
        if (!separated)
        {
            ui->menuWindow->addSeparator();
            separated = true;
        }
    }

    QMdiSubWindow *subWindow = ui->mdiArea->addSubWindow(console);
    ui->menuWindow->addAction(console->windowAction());
    m_windowActions->addAction(console->windowAction());
    subWindow->showMaximized();
}

Console * MainWindow::activeConsole()
{
    QMdiSubWindow * subWindow = ui->mdiArea->activeSubWindow();
    if (subWindow)
    {
        return qobject_cast<Console *>(subWindow->widget());
    }

    return 0;
}
