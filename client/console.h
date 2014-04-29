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

#include "connection.h"
#include "profile.h"
#include <QCloseEvent>
#include <QHostInfo>
#include <QTextBlock>
#include <QWidget>

namespace Ui {
class Console;
}

class ConsoleDocument;
class Engine;

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

    Profile * profile() { return m_profile; }

    void connectToServer();
    void disconnectFromServer();
    bool isConnected() const { return m_connection->isConnected(); }
    bool isConnecting() const { return m_connection->isConnecting(); }
    bool isDisconnected() const { return m_connection->isDisconnected(); }
    bool isDisconnecting() const { return m_connection->isDisconnecting(); }

    QAction * windowAction() const { return m_action; }
    const QString & fileName() const { return m_fileName; }

    void scrollUp(int lines);
    void scrollDown(int lines);
    void scrollTo(int line);
    void scrollToTop();
    void scrollToBottom();

    void printInfo(const QString &msg);
    void printWarning(const QString &msg);
    void printError(const QString &msg);

signals:
    void connectionStatusChanged(bool connected);

protected:
    void closeEvent(QCloseEvent *e);

private slots:
    void contentsModified();
    void commandEntered(const QString &cmd);
    void scriptEntered(const QString &code);
    void connectionEstablished();
    void connectionLost();
    void lookupComplete(const QHostInfo &hostInfo);
    void dataReceived(const QByteArray &data);
    void echoToggled(bool on);
    void gmcpToggled(bool on);
    void scrollbarMoved(int pos);
    void updateScroll();
    void processTriggers(QTextBlock block, bool prompt);

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
    Engine *m_engine;
    Profile *m_profile;
    Connection *m_connection;

    bool m_echoOn;
};

#endif // CONSOLE_H
