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


#include "connection.h"
#include <QApplication>
#include <QDebug>

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
static const uchar TelnetOption_EndOfRecord = 25u;
static const uchar TelnetOption_NegotiateAboutWindowSize = 31u;
static const uchar TelnetOption_TerminalSpeed = 32u;
static const uchar TelnetOption_RemoteFlowControl = 33u;
static const uchar TelnetOption_LineMode = 34u;
static const uchar TelnetOption_EnvironmentVariables = 36;
static const uchar TelnetOption_CharacterSet = 42;
static const uchar TelnetOption_MudServerDataProtocol = 69u;
static const uchar TelnetOption_CompressV1 = 85u;
static const uchar TelnetOption_CompressV2 = 86u;
static const uchar TelnetOption_MudSoundProtocol = 90u;
static const uchar TelnetOption_MudExtensionProtocol = 91u;
static const uchar TelnetOption_ZenithMudProtocol = 93u;
static const uchar TelnetOption_Aardwolf = 102u;
static const uchar TelnetOption_ATCP = 200u;
static const uchar TelnetOption_GMCP = 201u;

Q_LOGGING_CATEGORY(CORE_CONNECTION, "core.connection")
Q_LOGGING_CATEGORY(CORE_CONNECTION_TELNET, "core.connection.telnet")


Connection::Connection(QObject *parent) :
    QObject(parent)
{
    connect(&m_socket, SIGNAL(connected()), SLOT(connectionEstablished()));
    connect(&m_socket, SIGNAL(disconnected()), SLOT(connectionLost()));
    connect(&m_socket, SIGNAL(readyRead()), SLOT(readyToRead()));

    setEncoding("UTF-8");

    m_port = 0;
    m_lookup = 0;

    reset();
}

void Connection::setEncoding(const QString &encoding)
{
    m_codec = QTextCodec::codecForName(qPrintable(encoding));

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

    qCDebug(CORE_CONNECTION) << "Connecting" << addr << port;
}

void Connection::disconnectRemote()
{
    if (m_lookup)
    {
        QHostInfo::abortHostLookup(m_lookup);
        m_lookup = 0;
    }

    qCDebug(CORE_CONNECTION) << "Disconnecting";

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
        m_socket.connectToHost(m_hostname, m_port);
    }

    qCDebug(CORE_CONNECTION) << "Host lookup complete" << m_hostname << m_address << m_port;

    emit hostFound(hostInfo);
}

void Connection::connectionEstablished()
{
    m_connectTime = QDateTime::currentDateTime();

    qCDebug(CORE_CONNECTION) << "Connected" << m_connectTime;

    emit connected();
}

void Connection::connectionLost()
{
    postData();

    qCDebug(CORE_CONNECTION) << "Disconnected" << QDateTime::currentDateTime();

    reset();

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

QString Connection::telnetString(const uchar option)
{
    switch (option)
    {
    case Telnet_ESC:
        return "ESC";

    case Telnet_SubnegotiationEnd:
        return "SE";
    case Telnet_NoOperation:
        return "NOP";
    case Telnet_DataMark:
        return "DM";
    case Telnet_Break:
        return "BRK";
    case Telnet_InterruptProcess:
        return "IP";
    case Telnet_AbortOutput:
        return "AO";
    case Telnet_AreYouThere:
        return "AYT";
    case Telnet_EraseCharacter:
        return "EC";
    case Telnet_EraseLine:
        return "EL";
    case Telnet_GoAhead:
        return "GA";
    case Telnet_SubnegotiationBegin:
        return "SB";
    case Telnet_Will:
        return "WILL";
    case Telnet_Wont:
        return "WONT";
    case Telnet_Do:
        return "DO";
    case Telnet_Dont:
        return "DONT";
    case Telnet_InterpretAsCommand:
        return "IAC";

    case TelnetOption_Echo:
        return "ECHO";
    case TelnetOption_SuppressGoAhead:
        return "SGA";
    case TelnetOption_Status:
        return "STAT";
    case TelnetOption_TimingMark:
        return "TM";
    case TelnetOption_TerminalType:
        return "TERM";
    case TelnetOption_EndOfRecord:
        return "EOR";
    case TelnetOption_NegotiateAboutWindowSize:
        return "NAWS";
    case TelnetOption_TerminalSpeed:
        return "SPD";
    case TelnetOption_RemoteFlowControl:
        return "RFC";
    case TelnetOption_LineMode:
        return "LINE";
    case TelnetOption_EnvironmentVariables:
        return "EV";
    case TelnetOption_CharacterSet:
        return "CS";
    case TelnetOption_MudServerDataProtocol:
        return "MSDP";
    case TelnetOption_CompressV1:
        return "MCCP1";
    case TelnetOption_CompressV2:
        return "MCCP2";
    case TelnetOption_MudSoundProtocol:
        return "MSP";
    case TelnetOption_MudExtensionProtocol:
        return "MXP";
    case TelnetOption_ZenithMudProtocol:
        return "ZMP";
    case TelnetOption_Aardwolf:
        return "AARD";
    case TelnetOption_ATCP:
        return "ATCP";
    case TelnetOption_GMCP:
        return "GMCP";

    default:
        break;
    }

    return "?";
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

    if (!data.isEmpty())
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
            qCDebug(CORE_CONNECTION_TELNET) << telnetString(ch);

            m_receivedGA = true;
            break;
        }

        case Telnet_Will:
        {
            option = cmd.at(2).toLatin1();

            qCDebug(CORE_CONNECTION_TELNET) << telnetString(ch) << telnetString(option) << QString::number(option);

            switch (option)
            {
                case TelnetOption_Echo:
                {
                    sendDo(option);
                    emit echo(false);
                    break;
                }

                case TelnetOption_GMCP:
                {
                    if (m_sentDo[TelnetOption_GMCP])
                    {
                        break;
                    }

                    sendDo(option);
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
                    supports.append("Core.Supports.Set [ \"Char 1\", \"Char.Skills 1\", \"Char.Items 1\", \"Room 1\", \"IRE.Rift 1\" ]");
                    supports.append(Telnet_InterpretAsCommand);
                    supports.append(Telnet_SubnegotiationEnd);

                    sendRaw(supports);
                    break;
                }

                default:
                {
                    sendDont(option);
                }
            }
            break;
        }

        case Telnet_Wont:
        {
            option = cmd.at(2).toLatin1();

            qCDebug(CORE_CONNECTION_TELNET) << telnetString(ch) << telnetString(option) << QString::number(option);

            sendDont(option);

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

            qCDebug(CORE_CONNECTION_TELNET) << telnetString(ch) << telnetString(option) << QString::number(option);

            if (option == TelnetOption_GMCP)
            {
                sendWill(option);
                emit toggleGMCP(true);
            }
            else
            {
                sendWont(option);
            }
            break;
        }

        case Telnet_Dont:
        {
            option = cmd.at(2).toLatin1();

            qCDebug(CORE_CONNECTION_TELNET) << telnetString(ch) << telnetString(option) << QString::number(option);

            sendWont(option);
            if (option == TelnetOption_GMCP)
            {
                emit toggleGMCP(false);
            }
            break;
        }

        case Telnet_SubnegotiationBegin:
        {
            option = cmd.at(2).toLatin1();

            qCDebug(CORE_CONNECTION_TELNET) << telnetString(ch) << telnetString(option) << QString::number(option);

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

                qCDebug(CORE_CONNECTION) << "GMCP" << name << args;

                emit receivedGMCP(name, args);
            }
            break;
        }
    }
}

void Connection::handleData(const QByteArray &data)
{
    if (data.isEmpty())
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

void Connection::sendDo(const uchar option)
{
    if (m_sentDo[option])
    {
        return;
    }

    sendTelnetOption(Telnet_Do, option);

    m_sentDo[option] = true;
    m_sentDont[option] = false;
}

void Connection::sendDont(const uchar option)
{
    if (m_sentDont[option])
    {
        return;
    }

    sendTelnetOption(Telnet_Dont, option);

    m_sentDo[option] = false;
    m_sentDont[option] = true;
}

void Connection::sendWill(const uchar option)
{
    if (m_sentWill[option])
    {
        return;
    }

    sendTelnetOption(Telnet_Will, option);

    m_sentWill[option] = true;
    m_sentWont[option] = false;
}

void Connection::sendWont(const uchar option)
{
    if (m_sentWont[option])
    {
        return;
    }

    sendTelnetOption(Telnet_Wont, option);

    m_sentWill[option] = false;
    m_sentWont[option] = true;
}

void Connection::postData()
{
    emit dataReceived(m_data);
    m_data.clear();
}

void Connection::reset()
{
    m_command.clear();
    m_data.clear();
    m_buffer.reset();

    m_IAC = false;
    m_IAC2 = false;
    m_SB = false;
    m_receivedGA = false;
    m_driverGA = true;
    m_GAtoLF = false;
    m_forceOffGA = false;
    m_bugfixGA = true;
    m_waitResponse = false;

    memset(&m_sentDo, false, sizeof(m_sentDo));
    memset(&m_sentDont, false, sizeof(m_sentDont));
    memset(&m_sentWill, false, sizeof(m_sentWill));
    memset(&m_sentWont, false, sizeof(m_sentWont));

    memset(&m_gotDo, false, sizeof(m_gotDo));
    memset(&m_gotDont, false, sizeof(m_gotDont));
    memset(&m_gotWill, false, sizeof(m_gotWill));
    memset(&m_gotWont, false, sizeof(m_gotWont));

    m_commands = 0;
    m_latency = 0.0;
    m_latencyMin = 0.0;
    m_latencyMax = 0.0;
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
