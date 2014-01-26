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


#ifndef CODEEDITORWINDOW_H
#define CODEEDITORWINDOW_H

#include "codeeditorwidget.h"
#include <QMainWindow>
#include <QMdiSubWindow>

struct CodeEditorWindowData;

class CodeEditorWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit CodeEditorWindow(QWidget *parent = 0);

    QString defaultPath() const;
    void setDefaultPath(const QString &path);

    bool loadFile(const QString &fileName);

public slots:
    void actionNew();
    void actionOpen();
    bool actionSave();
    bool actionSaveAs();
    void actionPreferences();
    void actionCompile();
    void actionPrint();
    void actionPrintPreview();
    void actionPrintPdf();
    void showEditorSettings();
    void showSearchBox();
    void printPreview(QPrinter *printer);

private slots:
    void updateSaveAction();
    void subWindowActivated(QMdiSubWindow *win);

private:
    CodeEditorWindowData *d;
};

#endif // CODEEDITORWINDOW_H
