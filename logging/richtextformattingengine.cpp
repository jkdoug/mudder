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


#include "richtextformattingengine.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QTime>

QString RichTextFormattingEngine::initializeString() const
{
    return QString("%1 Session Log:%3Date: %2%3")
            .arg(QCoreApplication::applicationName())
            .arg(QDateTime::currentDateTime().toString())
            .arg(endOfLine());
}

QString RichTextFormattingEngine::formatMessage(Logger::MessageType type, const QList<QVariant> &messages) const
{
    QString message(endOfLine());
    message.append(QTime::currentTime().toString("hh:mm:ss.zzz"));
    message.append(QString(" [%1] ").arg(LOG->logLevelToString(type), -8, QChar(QChar::Nbsp)));

    QString formattedMessage(FormattingEngine::escape(messages.front().toString()));
    message.append(formattedMessage);
    for (int i = 1; i < messages.count(); i++)
    {
        message.append(QString("%2                %1")
                       .arg(FormattingEngine::escape(messages.at(i).toString()))
                       .arg(endOfLine()));
    }
    message.append("</font>");

    QString colorHint;
    if (messages.count() > 0)
    {
        colorHint = matchColorFormattingHint(messages.front().toString(), type);
    }

    if (!colorHint.isEmpty())
    {
        message.prepend(QString("<font color='%1'>").arg(colorHint));
    }
    else
    {
        switch (type) {
        case Logger::Info:
               message.prepend("<font color='black'>");
           break;

        case Logger::Warning:
               message.prepend("<font color='orange'>");
           break;

        case Logger::Error:
               message.prepend("<font color='red'>");
           break;

        case Logger::Fatal:
               message.prepend("<font color='purple'>");
           break;

        case Logger::Debug:
               message.prepend("<font color='gray'>");
           break;

        case Logger::Trace:
               message.prepend("<font color='lightgray'>");
           break;

        default:
           break;
        }
    }

    return message;
}

QString RichTextFormattingEngine::finalizeString() const {
    return QString("%2End of session log%2%1")
            .arg(QDateTime::currentDateTime().toString())
            .arg(endOfLine());
}
