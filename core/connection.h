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


#ifndef CONNECTION_H
#define CONNECTION_H

#include "core_global.h"
#include <QBuffer>
#include <QLoggingCategory>
#include <QtNetwork>
#include <QObject>

Q_DECLARE_LOGGING_CATEGORY(CORE_CONNECTION)
Q_DECLARE_LOGGING_CATEGORY(CORE_CONNECTION_TELNET)

class CORESHARED_EXPORT Connection : public QObject
{
    Q_OBJECT
public:
    explicit Connection(QObject *parent = 0);

    void connectRemote(const QString &addr, int port);
    void disconnectRemote();

    bool send(const QString &data);
    bool sendRaw(const QByteArray &data);
    bool sendGmcp(const QString &msg, const QString &data = QString());
    bool sendTelnetOption(uchar type, uchar option);

    void setEncoding(const QString &encoding);

    bool isConnected() const { return m_socket.state() == QAbstractSocket::ConnectedState; }
    bool isConnecting() const { return m_socket.state() == QAbstractSocket::ConnectingState; }
    bool isDisconnected() const { return m_socket.state() == QAbstractSocket::UnconnectedState; }
    bool isDisconnecting() const { return m_socket.state() == QAbstractSocket::ClosingState; }
    const QDateTime & connectTime() const { return m_connectTime; }
    quint64 connectDuration();
    int latency() const { return m_latency; }

    static QString telnetString(const uchar option);

private:
    void handleTelnetCommand(const QString &cmd);
    void handleData(const QByteArray &data);
    void handlePrompt(const QByteArray &data);

    void sendDo(const uchar option);
    void sendDont(const uchar option);
    void sendWill(const uchar option);
    void sendWont(const uchar option);

    void postData();

    void reset();


    QTcpSocket m_socket;
    int m_lookup;

    QHostAddress m_address;
    QString m_hostname;
    int m_port;

    QTextCodec *m_codec;
    QTextDecoder *m_decoder;
    QTextEncoder *m_encoder;

    QDateTime m_connectTime;

    QString m_command;
    QByteArray m_data;
    QBuffer m_buffer;

    int m_commands;

    bool m_IAC;
    bool m_IAC2;
    bool m_SB;
    bool m_receivedGA;
    bool m_driverGA;
    bool m_GAtoLF;
    bool m_forceOffGA;
    bool m_bugfixGA;
    bool m_waitResponse;

    bool m_sentDo[256];
    bool m_sentDont[256];
    bool m_sentWill[256];
    bool m_sentWont[256];

    bool m_gotDo[256];
    bool m_gotDont[256];
    bool m_gotWill[256];
    bool m_gotWont[256];

    double m_latency;
    double m_latencyMin;
    double m_latencyMax;

    QTime m_latencyTime;

signals:
    void hostFound(const QHostInfo &hostInfo);
    void connected();
    void disconnected();
    void dataReceived(const QByteArray &data);
    void echo(bool on);
    void toggleGMCP(bool on);
    void receivedGMCP(const QString &name, const QString &args);

public slots:
    void lookupComplete(const QHostInfo &hostInfo);
    void connectionEstablished();
    void connectionLost();
    void readyToRead();
};

#endif // CONNECTION_H
