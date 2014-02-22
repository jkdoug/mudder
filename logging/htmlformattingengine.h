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


#ifndef HTMLFORMATTINGENGINE_H
#define HTMLFORMATTINGENGINE_H

#include "logging_global.h"
#include "formattingengine.h"

class LOGGINGSHARED_EXPORT HtmlFormattingEngine : virtual public FormattingEngine
{
public:
    static HtmlFormattingEngine & instance()
    {
        static HtmlFormattingEngine *theInstance = new HtmlFormattingEngine;
        return *theInstance;
    }
    ~HtmlFormattingEngine() {}

protected:
    HtmlFormattingEngine() : FormattingEngine()
    {
        setObjectName(name());
    }

public:
    QString initializeString() const;
    QString finalizeString() const;
    QString formatMessage(Logger::MessageType type, const QList<QVariant> &messages) const;
    QString fileExtension() const { return "html"; }
    QString name() const { return "HTML Format"; }
    QString endOfLine() const { return "<br>"; }
};

#endif // HTMLFORMATTINGENGINE_H
