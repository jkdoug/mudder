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


#ifndef LOGGEREXPORTABLE_H
#define LOGGEREXPORTABLE_H

#include "logging_global.h"
#include <QDataStream>

class LOGGINGSHARED_EXPORT LoggerExportable
{
public:
    LoggerExportable() {}
    virtual ~LoggerExportable() {}

    enum ExportMode { Binary };
    Q_DECLARE_FLAGS(ExportModeFlags, ExportMode)
    Q_FLAGS(ExportModeFlags)

    virtual ExportModeFlags supportedFormats() const = 0;
    virtual bool exportBinary(QDataStream &stream) const = 0;
    virtual bool importBinary(QDataStream &stream) = 0;
    virtual QString factoryTag() const = 0;
    virtual QString instanceName() const = 0;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(LoggerExportable::ExportModeFlags)
Q_DECLARE_INTERFACE(LoggerExportable, "org.mudder.LoggerExportable/1.0")

#endif // LOGGEREXPORTABLE_H
