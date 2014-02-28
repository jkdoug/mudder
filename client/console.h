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


#ifndef CONSOLE_H
#define CONSOLE_H

#include <QCloseEvent>
#include <QWidget>

namespace Ui {
class Console;
}

class ConsoleDocument;
class Profile;

class Console : public QWidget
{
    Q_OBJECT

public:
    explicit Console(QWidget *parent = 0);
    ~Console();

    void newFile();
    bool save();
    bool saveAs();

    static Console * open(QWidget *parent = 0);
    static Console * openFile(const QString &fileName, QWidget *parent = 0);

    QAction * windowAction() const { return m_action; }
    const QString & fileName() const { return m_fileName; }

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    void contentsModified();

private:
    bool okToContinue();
    bool saveFile(const QString &fileName);
    void setCurrentFile(const QString &fileName);
    bool readFile(const QString &fileName);
    bool writeFile(const QString &fileName);

    Ui::Console *ui;

    QAction *m_action;
    QString m_fileName;
    bool m_isUntitled;

    ConsoleDocument *m_document;
    Profile *m_profile;
};

#endif // CONSOLE_H