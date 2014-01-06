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


#include "connection.h"
#include <QApplication>

static const QLatin1Char ESC('\x1B');
static const QLatin1Char ANSI_START('[');
static const QLatin1Char ANSI_SEPARATOR(';');
static const QLatin1Char ANSI_END('m');
static const QLatin1Char GA('\xFF');
static const QLatin1Char CR('\r');
static const QLatin1Char LF('\n');
static const QString CRLF = QString(CR) + QString(LF);
static const QString CRLFGA(CRLF + GA);

static const uchar Telnet_ESC = 0x1Bu;

static const uchar Telnet_SubnegotiationEnd = 240u;
static const uchar Telnet_NoOperation = 241u;
static const uchar Telnet_DataMark = 242u;
static const uchar Telnet_Break = 243u;
static const uchar Telnet_InterruptProcess = 244u;
static const uchar Telnet_AbortOutput = 245u;
static const uchar Telnet_AreYouThere = 246u;
static const uchar Telnet_EraseCharacter = 247u;
static const uchar Telnet_EraseLine = 248u;
static const uchar Telnet_GoAhead = 249u;
static const uchar Telnet_SubnegotiationBegin = 250u;
static const uchar Telnet_Will = 251u;
static const uchar Telnet_Wont = 252u;
static const uchar Telnet_Do = 253u;
static const uchar Telnet_Dont = 254u;
static const uchar Telnet_InterpretAsCommand = 255u;

static const uchar TelnetOption_Echo = 1u;
static const uchar TelnetOption_SuppressGoAhead = 3u;
static const uchar TelnetOption_Status = 5u;
static const uchar TelnetOption_TimingMark = 6u;
static const uchar TelnetOption_TerminalType = 24u;
static const uchar TelnetOption_NegotiateAboutWindowSize = 31u;
static const uchar TelnetOption_CompressV1 = 85u;
static const uchar TelnetOption_CompressV2 = 86u;
static const uchar TelnetOption_MudSoundProtocol = 90u;
static const uchar TelnetOption_MudExtentionProtocol = 91u;
static const uchar TelnetOption_ATCP = 200u;
static const uchar TelnetOption_GMCP = 201u;


Connection::Connection(QObject *parent) :
    QObject(parent)
{
    connect(&m_socket, SIGNAL(connected()), this, SLOT(connectionEstablished()));
    connect(&m_socket, SIGNAL(disconnected()), this, SLOT(connectionLost()));
    connect(&m_socket, SIGNAL(readyRead()), this, SLOT(readyToRead()));

    setEncoding("UTF-8");

    m_port = 0;
    m_commands = 0;
    m_lookup = 0;

    m_IAC = false;
    m_IAC2 = false;
    m_SB = false;
    m_GMCP = false;
    m_receivedGA = false;
    m_driverGA = true;
    m_GAtoLF = false;
    m_forceOffGA = false;
    m_bugfixGA = true;
    m_waitResponse = false;

    m_latency = 0;
}

void Connection::setEncoding(const QString &encoding)
{
    m_codec = QTextCodec::codecForName(encoding.toLatin1().data());

    m_decoder = m_codec->makeDecoder();
    m_encoder = m_codec->makeEncoder();
}

void Connection::connectRemote(const QString &addr, int port)
{
    if (m_socket.state() != QAbstractSocket::UnconnectedState)
    {
        m_socket.abort();
        connectRemote(addr, port);
        return;
    }

    m_hostname = addr;
    m_port = port;
    m_lookup = QHostInfo::lookupHost(addr, this, SLOT(lookupComplete(QHostInfo)));
}

void Connection::disconnectRemote()
{
    if (m_lookup)
    {
        QHostInfo::abortHostLookup(m_lookup);
        m_lookup = 0;
    }

    m_socket.disconnectFromHost();
}

void Connection::lookupComplete(const QHostInfo &hostInfo)
{
    if (!hostInfo.addresses().isEmpty())
    {
        m_address = hostInfo.addresses().first();
        m_socket.connectToHost(m_address, m_port);
    }
    else
    {
        m_hostname = hostInfo.hostName();
        m_socket.connectToHost(hostInfo.hostName(), m_port);
    }

    emit hostFound(hostInfo);
}

void Connection::connectionEstablished()
{
    m_connectTime = QDateTime::currentDateTime();

    emit connected();
}

void Connection::connectionLost()
{
    postData();

    m_GMCP = false;
    m_receivedGA = false;

    emit disconnected();
}

quint64 Connection::connectDuration()
{
    if (m_connectTime.isValid())
    {
        return m_connectTime.msecsTo(QDateTime::currentDateTime());
    }

    return 0;
}

void Connection::readyToRead()
{
    if (m_waitResponse)
    {
        m_waitResponse = false;

        int time = m_latencyTime.elapsed();
        m_latency = time;
    }

    qint64 amt = m_socket.bytesAvailable();
    m_buffer.setData(m_socket.read(amt));

    if (amt <= 0)
    {
        return;
    }

//    qDebug() << "Bytes Read:" << amt;

    m_receivedGA = false;

    QByteArray data;
    for (int b = 0; b < amt; b++)
    {
        uchar ch = (uchar)m_buffer.data().at(b);

        if (m_IAC || m_IAC2 || m_SB || ch == Telnet_InterpretAsCommand)
        {
            if (!(m_IAC || m_IAC2 || m_SB) && ch == Telnet_InterpretAsCommand)
            {
                m_IAC = true;
                m_command += ch;
            }
            else if (m_IAC && ch == Telnet_InterpretAsCommand && !m_SB)
            {
                m_IAC = false;
                data += ch;
                m_command.clear();
            }
            else if (m_IAC && !m_SB && (ch == Telnet_Will || ch == Telnet_Wont || ch == Telnet_Do || ch == Telnet_Dont))
            {
                m_IAC = false;
                m_IAC2 = true;
                m_command += ch;
            }
            else if (m_IAC2)
            {
                m_IAC2 = false;
                m_command += ch;
                handleTelnetCommand(m_command);
                m_command.clear();
            }
            else if (m_IAC && !m_SB && ch == Telnet_SubnegotiationBegin)
            {
                m_IAC = false;
                m_SB = true;
                m_command += ch;
            }
            else if (m_IAC && !m_SB && ch == Telnet_SubnegotiationEnd)
            {
                m_command.clear();
                m_IAC = false;
            }
            else if (m_SB)
            {
                m_command += ch;

                if (m_IAC && ch == Telnet_SubnegotiationEnd)
                {
                    handleTelnetCommand(m_command);
                    m_command.clear();
                    m_IAC = false;
                    m_SB = false;
                }

                if (m_IAC)
                {
                    m_IAC = false;
                }
                else if (ch == Telnet_InterpretAsCommand)
                {
                    m_IAC = true;
                }
            }
            else
            {
                m_IAC = false;
                m_command += ch;
                handleTelnetCommand(m_command);
                m_command.clear();
            }
        }
        else
        {
            if (ch != CR)
            {
                data += ch;
            }
        }

        if (m_receivedGA)
        {
            if (!m_forceOffGA)
            {
                m_receivedGA = false;
                m_driverGA = true;

                if (m_commands > 0)
                {
                    m_commands--;
                    if (m_latencyTime.elapsed() > 2000)
                    {
                        m_commands = 0;
                    }
                }

                data.append(GA.toLatin1());
                handlePrompt(data);
                data.clear();
            }
            else
            {
                if (m_GAtoLF)
                {
                    data.append(LF.toLatin1());
                }
            }
        }
    }

    if (data.length() > 0)
    {
        handleData(data);
    }
}

void Connection::handleTelnetCommand(const QString &cmd)
{
    if (cmd.length() < 1)
    {
        return;
    }

    uchar ch = cmd.at(1).toLatin1();
    uchar option = 0;

    switch (ch)
    {
        case Telnet_GoAhead:
        {
            m_receivedGA = true;
            break;
        }

        case Telnet_Will:
        {
            option = cmd.at(2).toLatin1();
            switch (option)
            {
                case TelnetOption_Echo:
                {
                    emit echo(false);
                    break;
                }

                case TelnetOption_GMCP:
                {
                    if (m_GMCP)
                    {
                        break;
                    }

                    m_GMCP = true;

                    sendTelnetOption(Telnet_Do, TelnetOption_GMCP);
                    emit toggleGMCP(true);

                    QByteArray hello;
                    hello.append(Telnet_InterpretAsCommand);
                    hello.append(Telnet_SubnegotiationBegin);
                    hello.append(TelnetOption_GMCP);
                    hello.append(QString("Core.Hello { \"client\": \"%1\", \"version\": \"%2\" }").arg(QApplication::applicationName()).arg(QApplication::applicationVersion()));
                    hello.append(Telnet_InterpretAsCommand);
                    hello.append(Telnet_SubnegotiationEnd);

                    sendRaw(hello);

                    QByteArray supports;
                    supports.append(Telnet_InterpretAsCommand);
                    supports.append(Telnet_SubnegotiationBegin);
                    supports.append(TelnetOption_GMCP);
                    supports.append("Core.Supports.Set [ \"Char 1\", \"Char.Skills 1\", \"Char.Items 1\", \"Room 1\" ]");
                    supports.append(Telnet_InterpretAsCommand);
                    supports.append(Telnet_SubnegotiationEnd);

                    sendRaw(supports);
                    break;
                }

                default:
                {
                    sendTelnetOption(Telnet_Dont, option);
                }
            }
            break;
        }

        case Telnet_Wont:
        {
            option = cmd.at(2).toLatin1();
            switch (option)
            {
                case TelnetOption_Echo:
                {
                    emit echo(true);
                    break;
                }
            }

            break;
        }

        case Telnet_Do:
        {
            option = cmd.at(2).toLatin1();
            if (option == TelnetOption_GMCP)
            {
                m_GMCP = true;

                sendTelnetOption(Telnet_Will, TelnetOption_GMCP);
                emit toggleGMCP(true);
            }
            else
            {
                sendTelnetOption(Telnet_Wont, option);
                emit toggleGMCP(false);
            }
            break;
        }

        case Telnet_Dont:
        {
            // TODO: Telnet Dont handling
            break;
        }

        case Telnet_SubnegotiationBegin:
        {
            option = cmd.at(2).toLatin1();
            if (option == TelnetOption_GMCP)
            {
                if (cmd.length() < 6)
                {
                    return;
                }

                QString msg(cmd.mid(3, cmd.length() - 5));

                QString name;
                QString args;
                if (msg.indexOf(LF) > -1)
                {
                    name = msg.section(LF, 0, 0);
                    args = msg.section(LF, 1);
                }
                else
                {
                    name = msg.section(' ', 0, 0);
                    args = msg.section(' ', 1);
                }

                emit receivedGMCP(name, args);
            }
            break;
        }
    }
}

void Connection::handleData(const QByteArray &data)
{
    if (data.length() < 1)
    {
        return;
    }

    m_data += data;

    postData();
}

void Connection::handlePrompt(const QByteArray &data)
{
    m_data += data;

    // Search for leading linefeed, skip ANSI control sequences
    if (m_driverGA && m_bugfixGA)
    {
        int nPos = 0;
        int nMax = m_data.length();
        while (nPos < nMax)
        {
            if (m_data.at(nPos) == QLatin1Char(Telnet_ESC))
            {
                while (nPos < nMax)
                {
                    if (m_data.at(nPos) == ANSI_END)
                    {
                        break;
                    }
                    nPos++;
                }
            }
            else if (m_data.at(nPos) == LF)
            {
                m_data.remove(nPos, 1);
                break;
            }
            else
            {
                break;
            }
            nPos++;
        }
    }

    postData();
}

void Connection::postData()
{
    emit dataReceived(m_data);
    m_data.clear();
}

bool Connection::send(const QString &data)
{
    QString dataSend(data);
    dataSend.replace(QString(LF), "");

    QByteArray dataOut(m_codec->fromUnicode(dataSend).data());
    dataOut.append(CRLF);

    return sendRaw(dataOut);
}

bool Connection::sendRaw(const QByteArray &data)
{
    if (!m_socket.isWritable())
    {
        return false;
    }

    int nRemaining = data.length();
    do
    {
        qint64 nWritten = m_socket.write(data);

        if (nWritten == -1)
        {
            return false;
        }

        nRemaining -= nWritten;
    }
    while (nRemaining > 0);

    if (m_driverGA)
    {
        m_commands++;
        if (m_commands == 1)
        {
            m_waitResponse = true;
            m_latencyTime.restart();
        }
    }

    return true;
}

bool Connection::sendGmcp(const QString &msg, const QString &data)
{
    QByteArray out;
    out.append(Telnet_InterpretAsCommand);
    out.append(Telnet_SubnegotiationBegin);
    out.append(TelnetOption_GMCP);
    out.append(msg);
    if (!data.isEmpty())
    {
        out.append(" ");
        out.append(data);
    }
    out.append(Telnet_InterpretAsCommand);
    out.append(Telnet_SubnegotiationEnd);

    return sendRaw(out);
}

bool Connection::sendTelnetOption(uchar type, uchar option)
{
    QByteArray data;
    data.append(Telnet_InterpretAsCommand);
    data.append(type);
    data.append(option);

    return sendRaw(data);
}
