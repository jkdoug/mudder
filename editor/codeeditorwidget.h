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


#ifndef CODEEDITORWIDGET_H
#define CODEEDITORWIDGET_H

#include "editor_global.h"
#include "codeeditor.h"
#include <QMutex>

class QFileSystemWatcher;

class EDITORSHARED_EXPORT CodeEditorWidget : public CodeEditor
{
    Q_OBJECT
public:
    explicit CodeEditorWidget(QWidget *parent = 0);
    ~CodeEditorWidget();

    void newFile();
    bool save();
    bool saveAs();

    static CodeEditorWidget * open(QWidget *parent = 0);
    static CodeEditorWidget * openFile(const QString &fileName, QWidget *parent = 0);

    const QString & fileName() const { return m_fileName; }

private slots:
    void handleFileChange(const QString &path);
    void contentsModified(bool changed);

protected:
    virtual void closeEvent(QCloseEvent *e);

private:
    bool okToContinue();
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    bool readFile(const QString &fileName);
    bool writeFile(const QString &fileName);

    static QString m_fileFilter;

    QString m_fileName;
    bool m_isUntitled;

    QFileSystemWatcher *m_watcher;
    QMutex m_watcherMutex;
};

#endif // CODEEDITORWIDGET_H
