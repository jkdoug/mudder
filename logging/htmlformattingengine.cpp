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
    return QString("<html><head><title>%1 - %2</title></head>\n"
                   "<body><font face=\"Arial\">\n"
                   "<table width=\"100%\" style=\"table-layout:auto; margin: auto; border-width:thin thin thin thin; border-color:#000000;\">\n"
                   "<tr><th width=\"10%\"><b>Time</b></th><th><b>Message</b></th></tr>\n")
                   .arg(QCoreApplication::applicationName())
                   .arg(QDate::currentDate().toString());
}

QString HtmlFormattingEngine::formatMessage(Logger::MessageType type, const QList<QVariant> &messages) const
{
    if (messages.isEmpty())
    {
        return "";
    }

    QString timeString(QTime::currentTime().toString("hh:mm:ss.zzz"));
    QString formattedString(FormattingEngine::escape(messages.front().toString()));
    QString message;
    switch (type)
    {
    case Logger::Trace:
        message = QString("<td>%1</td><td><font color='gray'>%2</font></td>").arg(timeString).arg(formattedString);
        break;

    case Logger::Debug:
        message = QString("<td>%1</td><td><font color='gray'>%2</font></td>").arg(timeString).arg(formattedString);
        break;

    case Logger::Warning:
        message = QString("<td>%1</td><td><font color='orange'>%2</font></td>").arg(timeString).arg(formattedString);
        break;

    case Logger::Info:
        message = QString("<td>%1</td><td><font color='black'>%2</font></td>").arg(timeString).arg(formattedString);
        break;

    case Logger::Error:
        message = QString("<td>%1</td><td><font color='red'>%2</font></td>").arg(timeString).arg(formattedString);
        break;

    case Logger::Fatal:
        message = QString("<td>%1</td><td><font color='red'>%2</font></td>").arg(timeString).arg(formattedString);
        break;

    default:
        return QString();
    }

    message.prepend("<tr>");
    message.append("</tr>\n");
    return message;
}

QString HtmlFormattingEngine::finalizeString() const
{
    return QString("</table>%2End of session log: %1</body></html>\n")
            .arg(QDateTime::currentDateTime().toString())
            .arg(endOfLine());
}
