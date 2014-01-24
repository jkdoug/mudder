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


#ifndef CODEEDITORWIDGET_H
#define CODEEDITORWIDGET_H

#include "codeeditor.h"
#include <QPrinter>
#include <QWidget>

struct CodeEditorWidgetData;

class CodeEditorWidget : public CodeEditor
{
    Q_OBJECT
public:
    explicit CodeEditorWidget(QWidget *parent = 0);
    ~CodeEditorWidget();

    bool loadFile(const QString &path);
    bool saveFile(QString path = QString());

    QString fileName() const;

    bool isModified() const;

private slots:
    void handleFileChange(const QString &path);

signals:
    void fileNameChanged(const QString &path);

private:
    bool maybeSave();

    CodeEditorWidgetData *d;
};

#endif // CODEEDITORWIDGET_H
