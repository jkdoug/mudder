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


#include "settingsfiltermodel.h"
#include "logging.h"

SettingsFilterModel::SettingsFilterModel(QObject *parent) :
    QSortFilterProxyModel(parent)
{
}

bool SettingsFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if (filterRegExp().isEmpty())
    {
        return true;
    }

    QModelIndex sourceIndex(sourceModel()->index(sourceRow, 0, sourceParent));
    if (!sourceIndex.isValid())
    {
        qCDebug(MUDDER_PROFILE) << "Invalid source model index:" << sourceIndex;
        return true;
    }

    if (sourceModel()->hasChildren(sourceIndex))
    {
        for (int n = 0; n < sourceModel()->rowCount(sourceIndex); n++)
        {
            if (filterAcceptsRow(n, sourceIndex))
            {
                return true;
            }
        }

        return false;
    }

    QString key(sourceModel()->data(sourceIndex, Qt::DisplayRole).toString());
    return key.contains(filterRegExp());
}
