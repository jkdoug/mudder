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


#include "htmlformattingengine.h"
#include <QCoreApplication>
#include <QDateTime>
#include <QTime>

QString HtmlFormattingEngine::initializeString() const
{
    return QString("<html>\n<head>\n <title>%1 - %2</title>\n <style>\n"
                   "table { border-collapse: collapse; }\n"
                   "th { border: 2px solid black; }\n"
                   "td { border: 1px solid black; }\n"
                   " </style>\n</head>\n"
                   "<body>\n<font face=\"Arial\">\n"
                   " <table width=\"100%\" style=\"table-layout:auto; margin:auto;\">\n"
                   "  <tr><th width=\"10%\"><b>Time</b></th><th><b>Message</b></th></tr>\n")
                   .arg(QCoreApplication::applicationName())
                   .arg(QDate::currentDate().toString());
}

QString HtmlFormattingEngine::formatMessage(Logger::MessageType type, const QList<QVariant> &messages) const
{
    if (messages.isEmpty())
    {
        return QString();
    }

    QString color;
    switch (type)
    {
    case Logger::Trace:
    case Logger::Debug:
        color = "gray";
        break;

    case Logger::Warning:
        color = "orange";
        break;

    case Logger::Info:
        color = "black";
        break;

    case Logger::Error:
        color = "red";
        break;

    default:
        return QString();
    }

    QStringList formattedMessages;
    foreach (QVariant message, messages)
    {
        formattedMessages << FormattingEngine::escape(message.toString());
    }

    return QString("  <tr><td>%1</td><td>%2</td></tr>\n")
            .arg(QTime::currentTime().toString("hh:mm:ss.zzz"))
            .arg(formattedMessages.join("<li>"));
}

QString HtmlFormattingEngine::finalizeString() const
{
    return QString("</table>\n<br>End of session log: %1\n</font>\n</body>\n</html>\n")
            .arg(QDateTime::currentDateTime().toString());
}
