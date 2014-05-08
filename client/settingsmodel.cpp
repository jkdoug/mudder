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


#include <QIcon>
#include "settingsmodel.h"
#include "group.h"
#include "alias.h"

SettingsModel::SettingsModel(QObject *parent) :
    QAbstractItemModel(parent),
    m_rootGroup(0)
{
}

void SettingsModel::setRootGroup(Group *group)
{
    if (group != m_rootGroup)
    {
        beginResetModel();
        m_rootGroup = group;
        endResetModel();
    }
}

QModelIndex SettingsModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!m_rootGroup || row < 0 || column < 0)
    {
        return QModelIndex();
    }

    ProfileItem *parentItem = itemFromIndex(parent);
    Group *group = static_cast<Group *>(parentItem);
    if (!group)
    {
        return QModelIndex();
    }

    QList<ProfileItem *> children(group->items());
    if (row >= children.count())
    {
        return QModelIndex();
    }

    return createIndex(row, column, children.at(row));
}

QModelIndex SettingsModel::parent(const QModelIndex &child) const
{
    ProfileItem *item = itemFromIndex(child);
    if (!item)
    {
        return QModelIndex();
    }

    Group *group = item->group();
    if (!group)
    {
        return QModelIndex();
    }

    Group *groupParent = group->group();
    if (!groupParent)
    {
        return QModelIndex();
    }

    int row = groupParent->items().indexOf(group);
    return createIndex(row, 0, group);
}

int SettingsModel::rowCount(const QModelIndex &parent) const
{
    if (!m_rootGroup || parent.column() > 0)
    {
        return 0;
    }

    ProfileItem *parentItem = itemFromIndex(parent);
    Group *group = qobject_cast<Group *>(parentItem);
    if (!group)
    {
        return 0;
    }

    return group->items().count();
}

int SettingsModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return 1;
}

QVariant SettingsModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        ProfileItem *item = itemFromIndex(index);
        if (item)
        {
            switch (index.column())
            {
            case 0:
                return item->name();
            case 1:
                return !item->value().isEmpty()?item->value():QVariant();
            case 2:
                return item->sequence() > 0?item->sequence():QVariant();
            default:
                return "???";
            }
        }
    }
    else if (role == Qt::DecorationRole && index.column() == 0)
    {
        ProfileItem *item = itemFromIndex(index);
        if (item)
        {
            return item->icon();
        }
    }

    return QVariant();
}

QVariant SettingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    Q_UNUSED(orientation)

    if (role == Qt::DisplayRole)
    {
        switch (section)
        {
        case 0:
            return tr("Name");
        case 1:
            return tr("Value");
        case 2:
            return tr("Sequence");
        }
    }

    return QVariant();
}

bool SettingsModel::appendItem(ProfileItem *item, const QModelIndex &parent)
{
    if (!m_rootGroup || !parent.isValid())
    {
        return false;
    }

    Group *group = qobject_cast<Group *>(itemFromIndex(parent));
    if (!group)
    {
        return false;
    }

    beginInsertRows(parent, 0, 0);
    group->addItem(item);
    endInsertRows();

    return true;
}

ProfileItem * SettingsModel::itemFromIndex(const QModelIndex &index) const
{
    if (index.isValid())
    {
        return static_cast<ProfileItem *>(index.internalPointer());
    }

    return m_rootGroup;
}
