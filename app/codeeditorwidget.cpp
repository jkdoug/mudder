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


#include "codeeditorwidget.h"
#include "options.h"
#include <QApplication>
#include <QFileDialog>
#include <QFileInfo>
#include <QFileSystemWatcher>
#include <QMessageBox>
#include <QMutex>

struct CodeEditorWidgetData
{
    QString fileName;

    QFileSystemWatcher watcher;
    QMutex watcherMutex;
};

CodeEditorWidget::CodeEditorWidget(QWidget *parent) :
    CodeEditor(parent)
{
    d = new CodeEditorWidgetData;

    connect(&d->watcher, SIGNAL(fileChanged(QString)), this, SLOT(handleFileChange(QString)));
}

CodeEditorWidget::~CodeEditorWidget()
{
    maybeSave();

    delete d;
}

bool CodeEditorWidget::loadFile(const QString &path)
{
    maybeSave();

    if (path.isEmpty())
    {
        return false;
    }

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    document()->setModified(false);
    setPlainText(file.readAll());
    setWindowModified(false);

    QString previous(d->fileName);
    d->fileName = path;
    emit fileNameChanged(d->fileName);

    if (d->watcher.files().contains(previous))
    {
        d->watcher.removePath(previous);
    }
    d->watcher.addPath(d->fileName);

    return true;
}

bool CodeEditorWidget::saveFile(QString path)
{
    if (path.isEmpty())
    {
        path = d->fileName;
    }

    if (path.isEmpty())
    {
        return false;
    }

    if (d->watcher.files().contains(d->fileName))
    {
        d->watcher.removePath(d->fileName);
    }

    QFile file(path);
    if (!file.open(QFile::WriteOnly))
    {
        return false;
    }

    file.write(toPlainText().toUtf8());
    file.close();

    document()->setModified(false);

    d->fileName = path;
    emit fileNameChanged(d->fileName);

    d->watcher.addPath(d->fileName);

    return true;
}

QString CodeEditorWidget::fileName() const
{
    return d->fileName;
}

bool CodeEditorWidget::isModified() const
{
    return document()->isModified();
}

void CodeEditorWidget::handleFileChange(const QString &path)
{
    Q_UNUSED(path)
}

bool CodeEditorWidget::maybeSave()
{
//    if (!d->editor->document()->isModified())
//    {
//        return true;
//    }

//    QMessageBox::StandardButton ret;
//    if (d->fileName.isEmpty())
//    {
//        ret = QMessageBox::warning(this, tr("Code Editor"), tr("The document has been modified.\nDo you want to save your changes?"), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
//    }
//    else
//    {
//        ret = QMessageBox::warning(this, tr("File Changed"), tr("The modified document is linked to the following file:\n\n%1\n\nDo you want to save your changes?").arg(d->fileName), QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
//    }

//    if (ret == QMessageBox::Save)
//    {
//        return actionSave();
//    }
//    else if (ret == QMessageBox::Cancel)
//    {
//        return false;
//    }

    return true;
}
