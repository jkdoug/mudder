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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAction>
#include <QCloseEvent>
#include <QIcon>
#include <QMainWindow>
#include <QMdiArea>
#include <QMdiSubWindow>
#include <QSignalMapper>
#include <QString>
#include <QWidget>
#include "mapwidget.h"

namespace Ui {
class MainWindow;
}

class Console;
class LuaScript;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

public slots:
    void connectionEstablished();
    void connectionLost();

    void logOpened(const QString &filename);
    void logClosed();

    void consoleWindowActivated(QMdiSubWindow *win);
    void scriptWindowActivated(QMdiSubWindow *win);

protected:
    virtual void closeEvent(QCloseEvent *event);

private:
    QMdiSubWindow * activeWindow();
    Console * activeConsole();

    QMdiSubWindow * activeScriptWindow();
    LuaScript *activeScriptEditor();

    void createMapDock();
    void createScriptDock();

    void loadProfile(const QString &filename);
    void saveProfile(const QString &filename = QString());

    void loadScript(const QString &filename);
    void saveScript(const QString &filename = QString());

    void setCurrentFile(const QString &filename);

    void addWindow(QWidget *widget, const QString &name, const QIcon &icon = QIcon());

    void updateActions();
    void updateRecentFiles();
    void updateTitle();

private slots:
    void onProfileChanged();
    void onScriptChanged();
    void onRecentFile(const QString &filename);

    void onImportMap();

    void onNewScript();
    void onOpenScript();
    void onSaveScript();
    void onSaveScriptAs();
    void onCompileScript();

    void on_action_Connect_triggered(bool checked);
    void on_action_Log_triggered(bool checked);
    void on_action_NewProfile_triggered();
    void on_action_NewScript_triggered();
    void on_action_Open_triggered();
    void on_action_Save_triggered();
    void on_action_SaveAs_triggered();
    void on_action_Close_triggered();
    void on_action_Exit_triggered();
    void on_action_Preferences_triggered();
    void on_action_Settings_triggered();
    void on_action_Reload_triggered();

    void on_action_About_triggered();
    void on_action_AboutQt_triggered();

    void on_action_ScriptEditor_triggered(bool checked);
    void on_action_Mapper_triggered(bool checked);

    void on_scriptDock_visibilityChanged(bool visible);

    void on_mapDock_visibilityChanged(bool visible);

private:
    Ui::MainWindow *ui;

    QMdiArea *m_mdiScript;
    QAction *m_actionScriptSave;
    QAction *m_actionScriptSaveAs;
    QAction *m_actionScriptCompile;

    MapWidget *m_mapper;

    enum { MaxRecentFiles = 5 };
    QAction *m_recentFileActs[MaxRecentFiles];
    QSignalMapper *m_recentSignalMapper;
};

#endif // MAINWINDOW_H
