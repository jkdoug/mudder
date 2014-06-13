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


#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QAction>
#include <QItemSelection>
#include <QMainWindow>
#include <QMap>
#include <QStackedWidget>
#include <QToolButton>
#include "settingsfiltermodel.h"

namespace Ui {
class SettingsWindow;
}

class Group;
class Profile;
class SettingsModel;

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit SettingsWindow(QWidget *parent = 0);
    ~SettingsWindow();

    Group * rootGroup() const;
    void setProfile(Profile *p);
    Profile * profile() const;

private slots:
    void settingModified(bool changed, bool valid);

    void cut();
    void copy();
    void paste();
    void clipboardChanged();

    void saveCurrentItem();
    void discardCurrentItem();

    void addAccelerator();
    void addAlias();
    void addEvent();
    void addGroup();
    void addTimer();
    void addTrigger();
    void addVariable();

    void deleteItem();

    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

    void showContextMenu(const QPoint &point);

    void filterTextChanged(const QString &text);

private:
    QModelIndex proxyIndex(const QModelIndex &modelIndex);
    QModelIndex sourceIndex(const QModelIndex &modelIndex);

    void addItem(const QString &type);

    Ui::SettingsWindow *ui;

    QToolButton *m_buttonNew;

    QAction *m_deleteItem;

    QStackedWidget *m_stackedEditors;
    QMap<QString, int> m_editors;

    SettingsFilterModel *m_proxy;
};

#endif // SETTINGSWINDOW_H
