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


#ifndef CONFIGWIDGET_H
#define CONFIGWIDGET_H

#include "core_global.h"
#include <QIcon>
#include <QWidget>

namespace Ui {
class ConfigWidget;
}

class ConfigPage;

class CORESHARED_EXPORT ConfigWidget : public QWidget
{
    Q_OBJECT

    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex STORED true)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(int count READ count)

public:
    explicit ConfigWidget(QWidget *parent = 0);
    ~ConfigWidget();

    int currentIndex() const;
    int count() const;

    ConfigPage * page(int index) const;
    ConfigPage * currentPage() const;

    QSize iconSize() const;

    void addPage(ConfigPage *page, const QIcon &icon = QIcon(), const QString &name = QString());
    void insertPage(int index, ConfigPage *page, const QIcon &icon = QIcon(), const QString &name = QString());

    bool eventFilter(QObject *object, QEvent *event);

public slots:
    void setCurrentIndex(int index);
    void setPageIcon(int index, const QIcon &icon);
    void setPageName(int index, const QString &name);
    void setIconSize(const QSize &size);

    void load();
    void save();
    bool validate();
    void restoreDefaults();

signals:
    void currentIndexChanged(int index);

private:
    Ui::ConfigWidget *ui;

    Q_DISABLE_COPY(ConfigWidget)
};

#endif // CONFIGWIDGET_H
