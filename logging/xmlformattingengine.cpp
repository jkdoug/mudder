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


#include "xmlformattingengine.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QTime>

QString XmlFormattingEngine::initializeString() const
{
    return QString("<session context=\"%1\" date=\"%2\">")
            .arg(QCoreApplication::applicationName())
            .arg(QDateTime::currentDateTime().toString());
}

QString XmlFormattingEngine::formatMessage(Logger::MessageType type, const QList<QVariant> &messages) const
{
    QString typeString(LOG->logLevelToString(type));
    QString timeString(QTime::currentTime().toString("hh:mm:ss.zzz"));
    QString message(QString("%3  <log time=\"%2\" type=\"%1\">")
                    .arg(typeString)
                    .arg(timeString)
                    .arg(endOfLine()));
    for (int i = 0; i < messages.count(); i++)
    {
        message.append(QString("%3    <message id=\"%1\">%2</message>")
                       .arg(i)
                       .arg(FormattingEngine::escape(messages.at(i).toString()))
                       .arg(endOfLine()));
    }
    message.append(QString("%1  </log>").arg(endOfLine()));

    return message;
}

QString XmlFormattingEngine::finalizeString() const
{
    return QString("%1</session>%1").arg(endOfLine());
}
