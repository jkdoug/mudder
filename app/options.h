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


#ifndef OPTIONS_H
#define OPTIONS_H

#include <QFont>
#include <QObject>
#include <QSettings>

class Options : public QObject
{
    Q_OBJECT

public:
    static Options * P();

    QVariant value(const QString &key, const QVariant &def = QVariant());
    void setValue(const QString &key, const QVariant &value);

    QFont editorFont();
    void setEditorFont(const QFont &font);

    QString homePath();

    enum { MaxRecentFiles = 5 };
    QStringList recentFileList();
    void addRecentFile(const QString &fileName);
    void removeRecentFile(const QString &fileName);

signals:
    void valueChanged(const QString &key, const QVariant &value);
    void editorFontChanged(const QFont &font);
    void recentFilesChanged(const QStringList &fileNames);

private:
    explicit Options(QObject *parent = 0);
    Q_DISABLE_COPY(Options)

    QFont loadFont(const QString &key, const QFont &def = QFont());
    void saveFont(const QString &key, const QFont &font);

    QSettings m_settings;
};

#define OPTIONS (Options::P())

#endif // OPTIONS_H
