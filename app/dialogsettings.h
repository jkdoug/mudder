/***************************************************************************
 *   Copyright (C) 2013 by Jason Douglas                                   *
 *   larkin.dischai@gmail.com                                              *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef DIALOGSETTINGS_H
#define DIALOGSETTINGS_H

#include "profile.h"
#include <QAbstractButton>
#include <QAction>
#include <QDialog>
#include <QIcon>
#include <QStringList>
#include <QTreeWidgetItem>
#include <QWidget>

namespace Ui {
class DialogSettings;
}

class Group;
class Alias;
class Timer;
class Trigger;
class Variable;

class DialogSettings : public QDialog
{
    Q_OBJECT

public:
    enum ItemType { TGroup, TAccelerator, TAlias, TTimer, TTrigger, TVariable };

    explicit DialogSettings(QWidget *parent = 0);
    ~DialogSettings();

    Profile * profile() const { return m_profile; }
    void load(Profile *profile);
    bool isDirty() const { return m_profile->isDirty(); }

private slots:
    void on_editSetting_clicked();
    void on_deleteSetting_clicked();
    void on_copySetting_clicked();
    void on_pasteSetting_clicked();

    void on_settings_itemSelectionChanged();
    void on_settings_itemDoubleClicked(QTreeWidgetItem *item, int column);
    void on_settings_itemChanged(QTreeWidgetItem *item, int column);
    void on_settings_itemExpanded(QTreeWidgetItem *item);
    void on_settings_itemCollapsed(QTreeWidgetItem *item);

    void on_buttonBox_accepted();
    void on_buttonBox_rejected();

    void filterAccelerator(bool checked);
    void filterAlias(bool checked);
    void filterTimer(bool checked);
    void filterTrigger(bool checked);
    void filterVariable(bool checked);

    void addAccelerator();
    void addAlias();
    void addGroup();
    void addTimer();
    void addTrigger();
    void addVariable();

    void clipboardChanged();

    void on_cutSetting_clicked();

private:
    void populateTree();
    void populateBranch(QTreeWidgetItem *parent, Group *group);
    QTreeWidgetItem *populateItem(QTreeWidgetItem *parent, ProfileItem *item, const QIcon &icon, ItemType type);
    void updateItem(QTreeWidgetItem *node, ProfileItem *item);
    void expandGroups(QTreeWidgetItem *item);

    void toggle(QTreeWidgetItem *item, ItemType type, bool flag);

    QTreeWidgetItem *parentGroup();


    Ui::DialogSettings *ui;

    QAction *m_addAccelerator;
    QAction *m_addAlias;
    QAction *m_addGroup;
    QAction *m_addTimer;
    QAction *m_addTrigger;
    QAction *m_addVariable;

    bool m_showAccelerators;
    bool m_showAliases;
    bool m_showTimers;
    bool m_showTriggers;
    bool m_showVariables;

    Profile *m_profile;

    QStringList m_expanded;
};

#endif // DIALOGSETTINGS_H
