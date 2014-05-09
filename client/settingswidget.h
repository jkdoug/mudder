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


#ifndef SETTINGSWIDGET_H
#define SETTINGSWIDGET_H

#include <QMap>
#include <QItemSelectionModel>
#include <QStackedWidget>
#include <QWidget>

namespace Ui {
class SettingsWidget;
}

class Group;
class ProfileItem;
class SettingsModel;

class SettingsWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsWidget(QWidget *parent = 0);
    ~SettingsWidget();

    void setRootGroup(Group *group);
    Group * rootGroup() const;
    void addItem(ProfileItem *item);

public slots:
    void updateCurrentItem(bool changed, bool valid);
    void saveCurrentItem();
    void discardCurrentItem();

private slots:
    void currentChanged(const QModelIndex &current, const QModelIndex &previous);
    void selectionChanged(const QItemSelection &selected, const QItemSelection &deselected);

signals:
    void settingModified(bool changed, bool valid);

private:
    Ui::SettingsWidget *ui;

    QStackedWidget *m_stackedEditors;
    QMap<QString, int> m_editors;

    SettingsModel *m_model;
    QItemSelectionModel *m_selection;
};

#endif // SETTINGSWIDGET_H
