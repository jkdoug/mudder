/***************************************************************************
 *   Copyright (C) 2013 by Jason Douglas                                   *
 *   larkin.dischai@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "console.h"
#include "dialogconnect.h"
#include "dialogprofile.h"
#include "dialogsettings.h"
#include "engine.h"
#include "luascript.h"
#include "mapengine.h"
#include "xmlexception.h"
#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QMessageBox>
#include <QTabBar>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    createMapDock();
    createScriptDock();

    m_recentSignalMapper = new QSignalMapper(this);
    QMenu *mru = new QMenu(this);
    ui->action_RecentFiles->setMenu(mru);
    for (int i = 0; i < MaxRecentFiles; i++)
    {
        m_recentFileActs[i] = new QAction(this);
        m_recentFileActs[i]->setVisible(false);
        mru->addAction(m_recentFileActs[i]);

        connect(m_recentFileActs[i], SIGNAL(triggered()), m_recentSignalMapper, SLOT(map()));
    }
    connect(m_recentSignalMapper, SIGNAL(mapped(QString)), this, SLOT(onRecentFile(QString)));

    QTabBar *mdiTabBar = ui->mdiArea->findChild<QTabBar *>();
    Q_ASSERT(mdiTabBar != 0);
    mdiTabBar->setExpanding(false);

    connect(ui->mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(consoleWindowActivated(QMdiSubWindow*)));

    updateTitle();
    updateActions();
    updateRecentFiles();

    QSettings settings;
    restoreGeometry(settings.value("geometry").toByteArray());
    restoreState(settings.value("state").toByteArray());
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::connectionEstablished()
{
    updateActions();
}

void MainWindow::connectionLost()
{
    updateActions();
}

void MainWindow::logOpened(const QString &filename)
{
    Q_UNUSED(filename);

    updateActions();
}

void MainWindow::logClosed()
{
    updateActions();
}

void MainWindow::consoleWindowActivated(QMdiSubWindow *win)
{
    Q_UNUSED(win);

    Console *console = activeConsole();
    if (console)
    {
        m_mapper->switchMapper(console->profile()->mapFilename());
    }

    updateActions();
    updateTitle();
}

void MainWindow::scriptWindowActivated(QMdiSubWindow *win)
{
    Q_UNUSED(win);

    updateActions();
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    QSettings settings;
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());

    // Check for unsaved profiles and prompt to save first
    QList<QMdiSubWindow *> wins = ui->mdiArea->subWindowList();
    bool saveAll = false;
    for (int n = 0; n < wins.length(); n++)
    {
        QMdiSubWindow *win = wins.at(n);
        Q_ASSERT(win != 0);

        Console *console = qobject_cast<Console *>(win->widget());
        Q_ASSERT(console != 0);
        Q_ASSERT(console->profile() != 0);

        if (console->profile()->isDirty())
        {
            if (!saveAll)
            {
                QMessageBox::StandardButton answer = QMessageBox::question(this, tr("Modified Profile"), tr("Profile '%1' has been modified.\nDo you wish to save changes before exiting?").arg(console->profile()->name()), QMessageBox::Yes | QMessageBox::No | QMessageBox::YesToAll | QMessageBox::NoToAll | QMessageBox::Cancel, QMessageBox::Yes);
                switch (answer)
                {
                case QMessageBox::Cancel:   // Stop closing the app
                    event->ignore();
                    return;

                case QMessageBox::YesToAll: // Stop asking, keep saving
                    saveAll = true;
                    break;

                case QMessageBox::No:       // Move along to the next one
                    continue;

                case QMessageBox::NoToAll:  // Close without saving
                    event->accept();
                    return;

                default:                    // Save this profile
                    break;
                }
            }

            QString filename(console->profile()->filename());
            if (filename.isEmpty())
            {
                filename = QFileDialog::getSaveFileName(this, tr("Save Profile"), QString(), tr("Mudder Profiles (*.mp);;All files (*)"));
            }

            if (!filename.isEmpty())
            {
                if (!console->saveProfile(filename))
                {
                    QMessageBox::critical(this, tr("File Error"), tr("Unable to save profile '%1' to disk.\nCheck to make sure the file is not locked.").arg(console->profile()->name()));
                }
            }
        }
    }

    event->accept();
}

QMdiSubWindow * MainWindow::activeWindow()
{
    return ui->mdiArea->currentSubWindow();
}

Console * MainWindow::activeConsole()
{
    QMdiSubWindow *win = activeWindow();
    if (win)
    {
        return qobject_cast<Console *>(win->widget());
    }

    return 0;
}

QMdiSubWindow * MainWindow::activeScriptWindow()
{
    return m_mdiScript->currentSubWindow();
}

LuaScript * MainWindow::activeScriptEditor()
{
    QMdiSubWindow *win = activeScriptWindow();
    if (win)
    {
        return qobject_cast<LuaScript *>(win->widget());
    }

    return 0;
}

void MainWindow::createMapDock()
{
    QMainWindow *mapWin = new QMainWindow(this);
    ui->mapDock->setWidget(mapWin);

    m_mapper = new MapWidget(mapWin);
    mapWin->setCentralWidget(m_mapper);

    QToolBar *mapBar = new QToolBar(mapWin);
    mapBar->setIconSize(QSize(16, 16));
    mapWin->addToolBar(mapBar);

    QAction *actionImport = new QAction(tr("&Import"), mapWin);
    actionImport->setIcon(QIcon(":/icons/small_new"));
    actionImport->setToolTip(tr("Import a map database from an XML file"));
    mapBar->addAction(actionImport);
    connect(actionImport, SIGNAL(triggered()), this, SLOT(onImportMap()));
}

void MainWindow::createScriptDock()
{
    QMainWindow *scriptWin = new QMainWindow(this);
    ui->scriptDock->setWidget(scriptWin);

    m_mdiScript = new QMdiArea(scriptWin);
    m_mdiScript->setDocumentMode(true);
    m_mdiScript->setViewMode(QMdiArea::TabbedView);
    m_mdiScript->setTabsClosable(true);
    m_mdiScript->setTabsMovable(true);
    scriptWin->setCentralWidget(m_mdiScript);
    connect(m_mdiScript, SIGNAL(subWindowActivated(QMdiSubWindow*)), this, SLOT(scriptWindowActivated(QMdiSubWindow*)));

    QToolBar *scriptBar = new QToolBar(scriptWin);
    scriptBar->setIconSize(QSize(16, 16));
    scriptWin->addToolBar(scriptBar);

    QAction *actionNew = new QAction(tr("&New"), scriptWin);
    actionNew->setIcon(QIcon(":/icons/small_new"));
    actionNew->setToolTip(tr("Create a new script"));
    scriptBar->addAction(actionNew);
    connect(actionNew, SIGNAL(triggered()), this, SLOT(onNewScript()));

    QAction *actionOpen = new QAction(tr("&Open"), scriptWin);
    actionOpen->setIcon(QIcon(":/icons/small_open"));
    actionOpen->setToolTip(tr("Load script file from disk"));
    scriptBar->addAction(actionOpen);
    connect(actionOpen, SIGNAL(triggered()), this, SLOT(onOpenScript()));

    m_actionScriptSave = new QAction(tr("&Save"), scriptWin);
    m_actionScriptSave->setIcon(QIcon(":/icons/small_save"));
    m_actionScriptSave->setToolTip(tr("Save script to disk"));
    scriptBar->addAction(m_actionScriptSave);
    connect(m_actionScriptSave, SIGNAL(triggered()), this, SLOT(onSaveScript()));

    m_actionScriptSaveAs = new QAction(tr("Save &As"), scriptWin);
    m_actionScriptSaveAs->setIcon(QIcon(":/icons/small_save_as"));
    m_actionScriptSaveAs->setToolTip(tr("Save script to disk with a new name"));
    scriptBar->addAction(m_actionScriptSaveAs);
    connect(m_actionScriptSaveAs, SIGNAL(triggered()), this, SLOT(onSaveScriptAs()));

    scriptBar->addSeparator();

    m_actionScriptCompile = new QAction(tr("&Compile"), scriptWin);
    m_actionScriptCompile->setIcon(QIcon(":/icons/compile"));
    m_actionScriptCompile->setToolTip(tr("Compile script to check for errors"));
    scriptBar->addAction(m_actionScriptCompile);
    connect(m_actionScriptCompile, SIGNAL(triggered()), this, SLOT(onCompileScript()));

    QTabBar *mdiTabBar = m_mdiScript->findChild<QTabBar *>();
    Q_ASSERT(mdiTabBar != 0);
    mdiTabBar->setExpanding(false);
}

void MainWindow::loadProfile(const QString &filename)
{
    Console *console = new Console(this);
    if (console->loadProfile(filename))
    {
        setCurrentFile(filename);
        statusBar()->showMessage(tr("Profile loaded"), 2000);
    }
    else
    {
        QSettings settings;
        QStringList files(settings.value("RecentFileList").toStringList());

        files.removeAll(filename);

        settings.setValue("RecentFileList", files);

        updateRecentFiles();

        console->systemErr("Failed to load profile");
    }

    connect(console->connection(), SIGNAL(connected()), this, SLOT(connectionEstablished()));
    connect(console->connection(), SIGNAL(disconnected()), this, SLOT(connectionLost()));
    connect(console->profile(), SIGNAL(changed()), this, SLOT(onProfileChanged()));
    connect(console, SIGNAL(logOpened(QString)), this, SLOT(logOpened(QString)));
    connect(console, SIGNAL(logClosed()), this, SLOT(logClosed()));

    addWindow(console, console->profile()->name());

//    QList<QTabBar *> tabBarList = ui->mdiArea->findChildren<QTabBar*>();
//    QTabBar *tabBar = tabBarList.at(0);
//    if (tabBar)
//    {
//        qDebug() << "tabBar" << tabBar->maximumSize() << tabBar->minimumSize() << tabBar->size();
//        if (ui->mdiArea->subWindowList().count() > 1)
//        {
//            tabBar->show();
//        }
//        else
//        {
//            tabBar->hide();
//        }
//    }

    if (console->profile()->autoLog())
    {
        QString result(console->startLog());
        if (!result.isEmpty())
        {
            console->systemErr(result);
        }
    }

    console->startEngine();

    if (console->profile()->autoConnect())
    {
        console->connectToServer();
    }
}

void MainWindow::saveProfile(const QString &filename)
{
    Console *console = activeConsole();
    Q_ASSERT(console != 0);

    QString fileSave(filename);
    if (fileSave.isEmpty())
    {
        fileSave = QFileDialog::getSaveFileName(this, tr("Save Profile"), console->profile()->filename(), tr("Mudder Profiles (*.mp);;All files (*)"));
    }

    if (!fileSave.isEmpty())
    {
        if (!console->saveProfile(fileSave))
        {
            QMessageBox::critical(this, tr("File Error"), tr("Unable to save profile to disk."));
            return;
        }

        setCurrentFile(fileSave);
        statusBar()->showMessage(tr("Profile saved"), 2000);
    }
}

void MainWindow::loadScript(const QString &filename)
{
    LuaScript *script = new LuaScript(this);
    if (!script->load(filename))
    {
        QSettings settings;
        QStringList files(settings.value("RecentFileList").toStringList());

        files.removeAll(filename);

        settings.setValue("RecentFileList", files);

        updateRecentFiles();

        QMessageBox::critical(this, tr("File Error"), tr("Failed to load script file."));

        delete script;
        return;
    }

    if (!ui->scriptDock->isVisible())
    {
        ui->scriptDock->show();
    }

    setCurrentFile(filename);
    statusBar()->showMessage(tr("Script loaded"), 2000);

    connect(script->document(), SIGNAL(contentsChanged()), this, SLOT(onScriptChanged()));

    addWindow(script, QFileInfo(filename).fileName());
}

void MainWindow::saveScript(const QString &filename)
{
    LuaScript *script = activeScriptEditor();
    Q_ASSERT(script != 0);

    QString fileSave(filename);
    if (fileSave.isEmpty())
    {
        fileSave = QFileDialog::getSaveFileName(this, tr("Save Script"), script->filename(), tr("Lua script files (*.lua);;All files (*)"));
    }

    if (!fileSave.isEmpty())
    {
        if (!script->save(fileSave))
        {
            QMessageBox::critical(this, tr("Script Error"), tr("Unable to save file to disk."));
            return;
        }

        setCurrentFile(fileSave);
        statusBar()->showMessage(tr("Script saved"), 2000);
    }
}

void MainWindow::setCurrentFile(const QString &filename)
{
    QSettings settings;
    QStringList files(settings.value("RecentFileList").toStringList());

    files.removeAll(filename);
    files.prepend(filename);

    while (files.size() > MaxRecentFiles)
    {
        files.removeLast();
    }

    settings.setValue("RecentFileList", files);

    updateRecentFiles();
}

void MainWindow::addWindow(QWidget *widget, const QString &name, const QIcon &icon)
{
    QMdiSubWindow *win = new QMdiSubWindow(this);
    win->setWidget(widget);
    win->setWindowTitle(name);
    win->setWindowIcon(icon);
    win->setAttribute(Qt::WA_DeleteOnClose);

    if (widget->inherits("Console"))
    {
        ui->mdiArea->addSubWindow(win);
    }
    else if (widget->inherits("LuaScript"))
    {
        m_mdiScript->addSubWindow(win);
    }

    win->showMaximized();
}

void MainWindow::updateActions()
{
    bool hasConsole = !ui->mdiArea->subWindowList().isEmpty();
    bool isDirty = activeConsole() && activeConsole()->profile()->isDirty();

    ui->action_Save->setEnabled(hasConsole && isDirty);
    ui->action_SaveAs->setEnabled(hasConsole);
    ui->action_Close->setEnabled(hasConsole);
    ui->action_Preferences->setEnabled(hasConsole);
    ui->action_Settings->setEnabled(hasConsole);
    ui->action_Connect->setEnabled(hasConsole);
    ui->action_Log->setEnabled(hasConsole);
    ui->action_Reload->setEnabled(hasConsole);

    if (hasConsole)
    {
        Console *console = activeConsole();
        Q_ASSERT(console != 0);

        ui->action_Connect->setChecked(console->isConnected());
        if (console->isConnected())
        {
            ui->action_Connect->setText(tr("&Disconnect"));
            ui->action_Connect->setToolTip(tr("Disconnect from game server"));
        }
        else
        {
            ui->action_Connect->setText(tr("&Connect"));
            ui->action_Connect->setToolTip(tr("Connect to game server"));
        }

        ui->action_Log->setChecked(console->isLogging());
    }


    bool hasScript = !m_mdiScript->subWindowList().isEmpty();
    bool isScriptDirty = activeScriptEditor() && activeScriptEditor()->document()->isModified();
    bool isScriptEmpty = !activeScriptEditor() || activeScriptEditor()->document()->isEmpty();

    m_actionScriptSave->setEnabled(hasScript && isScriptDirty);
    m_actionScriptSaveAs->setEnabled(hasScript);
    m_actionScriptCompile->setEnabled(!isScriptEmpty);

    ui->action_ScriptEditor->setChecked(ui->scriptDock->isVisible());

    ui->action_Mapper->setChecked(ui->mapDock->isVisible());
}

void MainWindow::updateRecentFiles()
{
    QSettings settings;
    QStringList files(settings.value("RecentFileList").toStringList());

    int numRecentFiles = qMin(files.size(), (int)MaxRecentFiles);

    for (int i = 0; i < numRecentFiles; ++i)
    {
        m_recentFileActs[i]->setText(tr("&%1 %2").arg(i + 1).arg(QFileInfo(files[i]).fileName()));
        m_recentFileActs[i]->setVisible(true);
        m_recentSignalMapper->setMapping(m_recentFileActs[i], files[i]);
    }

    for (int j = numRecentFiles; j < MaxRecentFiles; ++j)
    {
        m_recentFileActs[j]->setVisible(false);
    }

    ui->action_RecentFiles->setVisible(numRecentFiles > 0);
}

void MainWindow::updateTitle()
{
    static QString appTitle(tr("%1 %2").arg(QApplication::applicationName()).arg(QApplication::applicationVersion()));
    QString winTitle;
    QString modified;
    QString separator;

    QMdiSubWindow *win = activeWindow();
    if (win)
    {
        winTitle = QString("%1").arg(win->windowTitle());
        separator = " - ";

        Console *console = activeConsole();
        if (console)
        {
            if (console->profile()->isDirty())
            {
                modified = "*";
            }

            console->focusCommandLine();
        }
    }

    setWindowTitle(QString("%1%2%3%4").arg(winTitle).arg(modified).arg(separator).arg(appTitle));
}


void MainWindow::onProfileChanged()
{
    updateActions();
    updateTitle();
}

void MainWindow::onScriptChanged()
{
    updateActions();
}

void MainWindow::on_action_Connect_triggered(bool checked)
{
    Console *console = activeConsole();
    Q_ASSERT(console != 0);

    if (checked)
    {
        console->connectToServer();
    }
    else
    {
        console->disconnectFromServer();
    }
}

void MainWindow::on_action_Log_triggered(bool checked)
{
    Console *console = activeConsole();
    Q_ASSERT(console != 0);
    Q_ASSERT(console->isLogging() != checked);

    if (checked)
    {
        QString result(console->startLog());
        if (!result.isEmpty())
        {
            console->systemErr(result);
        }
    }
    else
    {
        console->finishLog();
        console->systemInfo(tr("Log closed"));
    }
}

void MainWindow::on_action_NewProfile_triggered()
{
    DialogConnect *dlg = new DialogConnect(this);
    if (dlg->exec() == QDialog::Accepted)
    {
        Console *console = new Console(this);
        console->profile()->setName(dlg->name());
        console->profile()->setAddress(dlg->address());
        console->profile()->setPort(dlg->port());
        console->profile()->setAutoConnect(dlg->autoConnect());

        connect(console->connection(), SIGNAL(connected()), this, SLOT(connectionEstablished()));
        connect(console->connection(), SIGNAL(disconnected()), this, SLOT(connectionLost()));
        connect(console->profile(), SIGNAL(changed()), this, SLOT(onProfileChanged()));

        addWindow(console, dlg->name());

        console->startEngine();

        if (console->profile()->autoConnect())
        {
            console->connectToServer();
        }

        updateActions();
        updateTitle();
    }
    delete dlg;
}

void MainWindow::on_action_NewScript_triggered()
{
    if (!ui->scriptDock->isVisible())
    {
        ui->scriptDock->show();
    }

    onNewScript();
}

void MainWindow::on_action_Open_triggered()
{
    QString filename(QFileDialog::getOpenFileName(this, tr("Open File"), QString(), tr("Mudder profiles (*.mp);;Lua script files (*.lua);;All files (*)")));
    if (filename.isEmpty())
    {
        return;
    }

    QFileInfo fileInfo(filename);
    if (fileInfo.suffix().compare("mp", Qt::CaseInsensitive) == 0)
    {
        loadProfile(filename);
    }
    else
    {
        loadScript(filename);
    }
}

void MainWindow::on_action_Save_triggered()
{
    saveProfile(activeConsole()->profile()->filename());
}

void MainWindow::on_action_SaveAs_triggered()
{
    saveProfile();
}

void MainWindow::on_action_Close_triggered()
{
    QMdiSubWindow *win = activeWindow();
    Q_ASSERT(win != 0);

    if (win)
    {
        win->close();
    }
}

void MainWindow::on_action_Exit_triggered()
{
    close();
}

void MainWindow::on_action_Preferences_triggered()
{
    Console *console = activeConsole();
    Q_ASSERT(console != 0);

    Profile *profile = console->profile();
    Q_ASSERT(profile != 0);

    DialogProfile *dlg = console->preferences();
    if (dlg->exec() == QDialog::Accepted)
    {
        if (*dlg->profile() == *profile)
        {
            return;
        }

        profile->copyPreferences(*dlg->profile());

        profile->setDirty(true);

        console->setInputFont(profile->inputFont());
        console->setOutputFont(profile->outputFont());

//        if (profile->autoWrap())
//        {
//            profile->setWrapColumn(console->screenWidth());
//        }

        if (!profile->name().isEmpty())
        {
            console->setWindowTitle(profile->name());
        }
        else if (!profile->filename().isEmpty())
        {
            console->setWindowTitle(profile->filename());
        }
        else
        {
            console->setWindowTitle("(untitled)");
        }

        updateTitle();
        updateActions();
    }
}

void MainWindow::on_action_Settings_triggered()
{
    Console *console = activeConsole();
    Q_ASSERT(console != 0);
    Q_ASSERT(console->profile() != 0);

    DialogSettings *dlg = console->settings();
    if (dlg->exec() == QDialog::Accepted)
    {
        if (dlg->isDirty())
        {
            console->profile()->copySettings(*dlg->profile());

            updateActions();
        }
    }
}

void MainWindow::on_action_Reload_triggered()
{
    Console *console = activeConsole();
    Q_ASSERT(console != 0);

    console->startEngine(true);
}

void MainWindow::onRecentFile(const QString &filename)
{
    QFileInfo fileInfo(filename);
    if (fileInfo.suffix().compare("mp", Qt::CaseInsensitive) == 0)
    {
        loadProfile(filename);
    }
    else
    {
        loadScript(filename);
    }
}

void MainWindow::onImportMap()
{
    Console *console = activeConsole();
    if (!console)
    {
        return;
    }

//    QString filename(QFileDialog::getOpenFileName(this, tr("Open File"), QString(), tr("XML map data (*.xml);;All files (*)")));
    QString filename("D:/Dropbox/Mudder/map_db.xml");
    if (filename.isEmpty())
    {
        return;
    }

    QApplication::setOverrideCursor(Qt::WaitCursor);
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        console->systemErr("Failed to open file");

        QApplication::restoreOverrideCursor();
        return;
    }

    try
    {
        QXmlStreamReader xml(&file);
        MapEngine *mapper = new MapEngine(this);
        mapper->importXml(xml);

        m_mapper->setMapper(filename, mapper);
        m_mapper->switchMapper(filename);

        console->profile()->setMapFilename(filename);
    }
    catch (XmlException *xe)
    {
        foreach (QString warning, xe->warnings())
        {
            console->systemWarn(warning);
        }

        delete xe;
    }

    m_mapper->mapper()->setCurrentRoom(1054);  // TODO

    m_mapper->update();

    QApplication::restoreOverrideCursor();
}

void MainWindow::onNewScript()
{
    static int num = 0;
    LuaScript *script = new LuaScript(this);
    addWindow(script, tr("(untitled-%1)").arg(++num));

    connect(script->document(), SIGNAL(contentsChanged()), this, SLOT(onScriptChanged()));

    updateActions();
}

void MainWindow::onOpenScript()
{
    QString filename(QFileDialog::getOpenFileName(this, tr("Open File"), QString(), tr("Lua script files (*.lua);;All files (*)")));
    if (filename.isEmpty())
    {
        return;
    }

    loadScript(filename);
}

void MainWindow::onSaveScript()
{
    LuaScript *script = activeScriptEditor();
    Q_ASSERT(script != 0);
    Q_ASSERT(!script->filename().isEmpty());

    saveScript(script->filename());

    updateActions();
}

void MainWindow::onSaveScriptAs()
{
    LuaScript *script = activeScriptEditor();
    Q_ASSERT(script != 0);

    saveScript();

    updateActions();
}

void MainWindow::onCompileScript()
{
    LuaScript *script = activeScriptEditor();
    Q_ASSERT(script != 0);

    QString errMessage;
    if (!Engine::compile(script->document()->toPlainText(), tr("Script Editor"), &errMessage))
    {
        QMessageBox::warning(this, tr("Compile Error"), errMessage);
    }
    else
    {
        QMessageBox::information(this, tr("Compiled"), tr("I'm making a note here: huge success."));
    }
}

void MainWindow::on_action_About_triggered()
{
    QString msg(tr("<b>%1, Version %2</b>").arg(QApplication::applicationDisplayName()).arg(QApplication::applicationVersion()));
    msg += tr("<p>Copyright (C) 2014 Jason K. Douglas<br/><a href='mailto:larkin.dischai@gmail.com'>larkin.dischai@gmail.com</a></p>");
    msg += tr("<p>%1<br/>%2</p>").arg(LUA_COPYRIGHT).arg(LUA_AUTHORS);

    QMessageBox::about(this, tr("About"), msg);
}

void MainWindow::on_action_AboutQt_triggered()
{
    QMessageBox::aboutQt(this);
}

void MainWindow::on_action_ScriptEditor_triggered(bool checked)
{
    ui->scriptDock->setVisible(checked);
}

void MainWindow::on_action_Mapper_triggered(bool checked)
{
    ui->mapDock->setVisible(checked);
}

void MainWindow::on_scriptDock_visibilityChanged(bool visible)
{
    Q_UNUSED(visible);

    updateActions();
}

void MainWindow::on_mapDock_visibilityChanged(bool visible)
{
    Q_UNUSED(visible);

    updateActions();
}
