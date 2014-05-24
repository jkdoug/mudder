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


#ifndef SETTINGSMODEL_H
#define SETTINGSMODEL_H

#include <QAbstractItemModel>

class Group;
class ProfileItem;

class SettingsModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit SettingsModel(QObject *parent = 0);

    Group * rootGroup() const { return m_rootGroup; }
    void setRootGroup(Group *group);

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex &child) const;

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;

    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;

    QModelIndex appendItem(ProfileItem *item, const QModelIndex &parent);

private slots:
    void updateSetting(ProfileItem *item);
    void addSetting(ProfileItem *item);
    void removeSetting(ProfileItem *item);

private:
    ProfileItem * itemFromIndex(const QModelIndex &index) const;
    QModelIndex indexFromItem(ProfileItem *item) const;

    Group *m_rootGroup;
};

#endif // SETTINGSMODEL_H
