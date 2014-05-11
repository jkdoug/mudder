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


#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QActionGroup>
#include <QCloseEvent>
#include <QMainWindow>
#include <QSignalMapper>
#include "coresettings.h"

namespace Ui {
class MainWindow;
}

class CodeEditorWindow;
class Console;
class SettingsWindow;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    void onBusyStateChanged(bool busy);
    void onRecentFile(const QString &fileName);
    void onRecentFilesChanged(const QStringList &fileNames);
    void on_actionNew_triggered();
    void on_actionOpen_triggered();
    void on_actionSave_triggered();
    void on_actionSaveAs_triggered();
    void on_actionConnect_triggered(bool checked);
    void on_actionOptions_triggered();
    void on_actionClose_triggered();
    void on_actionCloseAll_triggered();
    void on_actionNext_triggered();
    void on_actionPrevious_triggered();
    void on_actionExit_triggered();
    void on_actionAbout_triggered();
    void on_actionAboutQt_triggered();

    void updateActions();

private:
    void initializeRecentFiles();

    void newConsole();
    void openConsole(const QString &fileName = QString());
    void saveConsole();
    void addConsole(Console *console);
    Console * activeConsole();

private:
    Ui::MainWindow *ui;

    QAction *m_recentFileActions[CoreSettings::MaxRecentFiles];
    QSignalMapper *m_recentSignalMapper;
    QActionGroup *m_windowActions;

    CodeEditorWindow *m_editor;
    SettingsWindow *m_settings;
};

#endif // MAINWINDOW_H
