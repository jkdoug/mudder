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

#ifndef CONSOLE_H
#define CONSOLE_H

#include <QByteArray>
#include <QCloseEvent>
#include <QColor>
#include <QHostInfo>
#include <QKeyEvent>
#include <QList>
#include <QMediaContent>
#include <QMediaPlayer>
#include <QMouseEvent>
#include <QScrollBar>
#include <QString>
#include <QTextCharFormat>
#include <QTextStream>
#include <QWidget>

#include "connection.h"
#include "dialogprofile.h"
#include "dialogsettings.h"
#include "hyperlink.h"
#include "textdocument.h"
#include "profile.h"

class CommandLine;
class Engine;
class MapEngine;
class Splitter;
class TextDocument;
class TextEditor;

namespace Ui {
class Console;
}

class Console : public QWidget
{
    Q_OBJECT

public:
    explicit Console(QWidget *parent = 0);
    ~Console();

    Connection * connection() { return m_connection; }
    TextDocument * document() { return m_document; }
    Profile * profile() { return m_profile; }
    Engine * engine() { return m_engine; }

    DialogProfile * preferences();
    DialogSettings * settings();

    bool isConnected() const { return m_connection->isConnected(); }
    void connectToServer();
    void disconnectFromServer();

    void startEngine(bool restart = false);

    bool loadProfile(const QString &filename);
    bool saveProfile(const QString &filename);

    bool echoOn() const { return m_echoOn; }
    void setEchoOn(bool on = true) { m_echoOn = on; }

    int screenHeight() const;
    int screenWidth() const;

    QFont inputFont() const;
    void setInputFont(const QFont &font);
    QFont outputFont() const;
    void setOutputFont(const QFont &font);

    void setFormatSelection(const QTextCharFormat &fmt) { m_formatSelection = fmt; }
    QTextCharFormat formatSelection() const { return m_formatSelection; }

    void focusCommandLine();

    void scrollUp(int lines);
    void scrollDown(int lines);
    void scrollTo(int line);
    void scrollToTop();
    void scrollToBottom();

    QTextStream & log() { return m_log; }
    bool isLogging() const { return m_log.device() != 0 && !m_logPaused; }
    QString startLog(const QString &filename = QString(), bool append = true);
    const QFileInfo & logInfo() const { return m_logInfo; }
    void logNote(const QString &text);
    void finishLog();
    void pauseLog(bool flag = true) { m_logPaused = flag; }

    void systemPrefix(const QString &id);
    void systemInfo(const QString &txt);
    void systemWarn(const QString &txt);
    void systemErr(const QString &txt);

    void send(const QString &cmd, bool show = true);
    bool sendGmcp(const QString &msg, const QString &data = QString());
    void colorTell(const QColor &fg, const QColor &bg, const QString &text);
    void colorNote(const QColor &fg, const QColor &bg, const QString &text);
    int hyperlink(const QString &str, const QColor &fg, const QColor &bg, Hyperlink *info);

    QString playMedia(QMediaContent content);

    void timeLatency(double time);
    void timeProcessing(double time);
    double meanLatency() const;
    double meanProcessing() const;

protected:
    virtual void closeEvent(QCloseEvent *e);
    virtual void mousePressEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void wheelEvent(QWheelEvent *e);

signals:
    void logOpened(const QString &filename);
    void logClosed();

public slots:
    void connectionEstablished();
    void connectionLost();
    void lookupComplete(const QHostInfo &hostInfo);
    void dataReceived(const QByteArray &data);
    void echoToggled(bool on);
    void gmcpToggled(bool on);

    void handleGMCP(const QString &name, const QString &data);
    void handleMouseEvent(QMouseEvent *e, const QPoint &pos);

    void lineAdded(const QTextBlock &block);
    void commandEntered(const QString &cmd);
    void scriptEntered(const QString &code);

    void scrollbarMoved(int val);

    bool processKey(const QKeySequence &key, Group *base = 0);
    bool processHyperlink(Hyperlink *link);
    bool processAliases(const QString &cmd);
    bool processTriggers(const QString &text);

    void backgroundChanged();
    void inputFontChanged();
    void outputFontChanged();
    void updateScroll();

private:
    void logLatest();

    Ui::Console *ui;

    Connection *m_connection;
    TextDocument *m_document;
    Profile *m_profile;
    Engine *m_engine;

    DialogProfile *m_preferences;
    DialogSettings *m_settings;

    bool m_echoOn;
    bool m_disconnecting;

    QTextCharFormat m_formatSelection;

    bool m_mousePressed;
    QString m_linkHovered;
    int m_selectionStart;
    int m_selectionEnd;
    int m_clickPos;

    QTextStream m_log;
    QFileInfo m_logInfo;
    int m_logPosition;
    bool m_logPaused;

    QMediaPlayer *m_media;

    QList<double> m_latency;
    QList<double> m_processing;
    int m_maxPackets;
};

#endif // CONSOLE_H
