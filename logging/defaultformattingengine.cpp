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


#include "defaultformattingengine.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QMetaEnum>
#include <QMetaObject>
#include <QTime>

QString DefaultFormattingEngine::initializeString() const
{
    return QString("%1 Session Log:%3Date: %2%3")
            .arg(QCoreApplication::applicationName())
            .arg(QDateTime::currentDateTime().toString())
            .arg(endOfLine());
}

QString DefaultFormattingEngine::formatMessage(Logger::MessageType type, const QList<QVariant> &messages) const
{
    QString message(QTime::currentTime().toString("hh:mm:ss.zzz"));

    const QMetaObject &metaLogger = Logger::staticMetaObject;
    int indexMessageType = metaLogger.indexOfEnumerator("MessageType");
    QMetaEnum metaMessageType(metaLogger.enumerator(indexMessageType));
    message.append(QString(" [%1] ").arg(metaMessageType.valueToKey(type), -8, QChar(' ')));

    message.append(messages.front().toString());
    message.append(endOfLine());

    for (int i = 1; i < messages.count(); i++)
    {
        message.append(QString("                %1%2").arg(messages.at(i).toString()).arg(endOfLine()));
    }

    return message;
}

QString DefaultFormattingEngine::finalizeString() const
{
    return QString("%2End of session log.%2%1")
            .arg(QDateTime::currentDateTime().toString())
            .arg(endOfLine());
}
