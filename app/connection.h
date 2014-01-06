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


#ifndef CONNECTION_H
#define CONNECTION_H

#include <QBuffer>
#include <QtNetwork>
#include <QObject>

class Connection : public QObject
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

    bool isConnected() const { return m_socket.state() != QAbstractSocket::UnconnectedState; }
    const QDateTime & connectTime() const { return m_connectTime; }
    quint64 connectDuration();
    int latency() const { return m_latency; }

private:
    void handleTelnetCommand(const QString &cmd);
    void handleData(const QByteArray &data);
    void handlePrompt(const QByteArray &data);

    void postData();


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
    bool m_GMCP;
    bool m_receivedGA;
    bool m_driverGA;
    bool m_GAtoLF;
    bool m_forceOffGA;
    bool m_bugfixGA;
    bool m_waitResponse;

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
