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


#ifndef CORESETTINGS_H
#define CORESETTINGS_H

#include "core_global.h"
#include <QObject>
#include <QSettings>

class CORESHARED_EXPORT CoreSettings : public QObject
{
    Q_OBJECT

public:
    static CoreSettings * instance();

    QVariant value(const QString &key, const QVariant &def = QVariant());
    void setValue(const QString &key, const QVariant &val);
    void setDefault(const QString &key, const QVariant &val);

    enum { MaxRecentFiles = 5 };
    QStringList recentFileList();
    void addRecentFile(const QString &fileName);
    void removeRecentFile(const QString &fileName);

signals:
    void valueChanged(const QString &key, const QVariant &value);
    void recentFilesChanged(const QStringList &fileNames);

private:
    explicit CoreSettings(QObject *parent = 0);
    Q_DISABLE_COPY(CoreSettings)

    QSettings m_settings;
    QVariantMap m_defaults;
};

#define SETTINGS CoreSettings::instance()

#endif // CORESETTINGS_H
