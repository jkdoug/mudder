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


#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>
#include <QLoggingCategory>
#include <QStandardPaths>
#include "coreapplication.h"
#include "coresettings.h"

int main(int argc, char *argv[])
{
    qSetMessagePattern("[%{if-debug}D%{endif}%{if-warning}W%{endif}%{if-critical}C%{endif}%{if-fatal}F%{endif}] %{category} - %{message}");

    CoreApplication a(argc, argv);
    a.setApplicationName("Mudder");
    a.setApplicationVersion("0.4");
    a.setOrganizationName("Iasmos");

    QLoggingCategory::setFilterRules(QStringLiteral("*.debug=true\nqt.*=false"));

    SETTINGS->setValue("LastRun", QDateTime::currentDateTime());

    MainWindow w;
    CoreApplication::setMainWindow(&w);
    w.show();

    return a.exec();
}
