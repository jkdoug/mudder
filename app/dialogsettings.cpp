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

#include "dialogsettings.h"
#include "ui_dialogsettings.h"
#include "group.h"
#include "alias.h"
#include "timer.h"
#include "trigger.h"
#include "variable.h"
#include "dialogaccelerator.h"
#include "dialogalias.h"
#include "dialoggroup.h"
#include "dialogtimer.h"
#include "dialogtrigger.h"
#include "dialogvariable.h"
#include "xmlexception.h"

#include <QClipboard>
#include <QDebug>
#include <QFile>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QXmlSchema>
#include <QXmlSchemaValidator>

const int SETTINGS_ROLE_ITEM = Qt::UserRole + 1;

enum Columns { Name, Value, Sequence };

DialogSettings::DialogSettings(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogSettings)
{
    ui->setupUi(this);

    m_profile = 0;

    ui->copySetting->setShortcut(QKeySequence(QKeySequence::Copy));
    ui->deleteSetting->setShortcut(QKeySequence(QKeySequence::Delete));
    ui->pasteSetting->setShortcut(QKeySequence(QKeySequence::Paste));

    m_addAccelerator = new QAction(tr("A&ccelerator"), this);
    m_addAccelerator->setStatusTip(tr("Create a new key accelerator"));
    m_addAccelerator->setIcon(QIcon(":/icons/accelerator"));
    connect(m_addAccelerator, SIGNAL(triggered()), this, SLOT(addAccelerator()));

    m_addAlias = new QAction(tr("&Alias"), this);
    m_addAlias->setStatusTip(tr("Create a new alias"));
    m_addAlias->setIcon(QIcon(":/icons/alias"));
    connect(m_addAlias, SIGNAL(triggered()), this, SLOT(addAlias()));

    m_addGroup = new QAction(tr("&Group"), this);
    m_addGroup->setStatusTip(tr("Create a new group"));
    m_addGroup->setIcon(QIcon(":/icons/group"));
    connect(m_addGroup, SIGNAL(triggered()), this, SLOT(addGroup()));

    m_addTimer = new QAction(tr("&Timer"), this);
    m_addTimer->setStatusTip(tr("Create a new timer"));
    m_addTimer->setIcon(QIcon(":/icons/timer"));
    connect(m_addTimer, SIGNAL(triggered()), this, SLOT(addTimer()));

    m_addTrigger = new QAction(tr("T&rigger"), this);
    m_addTrigger->setStatusTip(tr("Create a new trigger"));
    m_addTrigger->setIcon(QIcon(":/icons/trigger"));
    connect(m_addTrigger, SIGNAL(triggered()), this, SLOT(addTrigger()));

    m_addVariable = new QAction(tr("&Variable"), this);
    m_addVariable->setStatusTip(tr("Create a new variable"));
    m_addVariable->setIcon(QIcon(":/icons/variable"));
    connect(m_addVariable, SIGNAL(triggered()), this, SLOT(addVariable()));

    QMenu *menuAddSetting = new QMenu(this);
    menuAddSetting->addAction(m_addAccelerator);
    menuAddSetting->addAction(m_addAlias);
    menuAddSetting->addAction(m_addGroup);
    menuAddSetting->addAction(m_addTimer);
    menuAddSetting->addAction(m_addTrigger);
    menuAddSetting->addAction(m_addVariable);
    ui->addSetting->setMenu(menuAddSetting);

    QAction *filterAccelerator = new QAction(tr("A&ccelerator"), this);
    filterAccelerator->setStatusTip(tr("Toggle the display of key accelerators in the tree view"));
    filterAccelerator->setCheckable(true);
    filterAccelerator->setChecked(true);
    connect(filterAccelerator, SIGNAL(toggled(bool)), this, SLOT(filterAccelerator(bool)));

    QAction *filterAlias = new QAction(tr("&Alias"), this);
    filterAlias->setStatusTip(tr("Toggle the display of aliases in the tree view"));
    filterAlias->setCheckable(true);
    filterAlias->setChecked(true);
    connect(filterAlias, SIGNAL(toggled(bool)), this, SLOT(filterAlias(bool)));

    QAction *filterTimer = new QAction(tr("&Timer"), this);
    filterTimer->setStatusTip(tr("Toggle the display of timers in the tree view"));
    filterTimer->setCheckable(true);
    filterTimer->setChecked(true);
    connect(filterTimer, SIGNAL(toggled(bool)), this, SLOT(filterTimer(bool)));

    QAction *filterTrigger = new QAction(tr("T&rigger"), this);
    filterTrigger->setStatusTip(tr("Toggle the display of triggers in the tree view"));
    filterTrigger->setCheckable(true);
    filterTrigger->setChecked(true);
    connect(filterTrigger, SIGNAL(toggled(bool)), this, SLOT(filterTrigger(bool)));

    QAction *filterVariable = new QAction(tr("&Variable"), this);
    filterVariable->setStatusTip(tr("Toggle the display of variables in the tree view"));
    filterVariable->setCheckable(true);
    filterVariable->setChecked(true);
    connect(filterVariable, SIGNAL(toggled(bool)), this, SLOT(filterVariable(bool)));

    QMenu *menuFilterSetting = new QMenu(this);
    menuFilterSetting->addAction(filterAccelerator);
    menuFilterSetting->addAction(filterAlias);
    menuFilterSetting->addAction(filterTimer);
    menuFilterSetting->addAction(filterTrigger);
    menuFilterSetting->addAction(filterVariable);
    ui->filterSetting->setMenu(menuFilterSetting);

    m_showAccelerators = true;
    m_showAliases = true;
    m_showTimers = true;
    m_showTriggers = true;
    m_showVariables = true;

    ui->settings->setColumnCount(3);
    ui->settings->setHeaderLabels(QStringList() << tr("Name") << tr("Value") << ("Sequence"));
    ui->settings->header()->resizeSection(0, 200);
    ui->settings->header()->resizeSection(1, 300);
    ui->settings->header()->resizeSection(2, 40);
    ui->settings->header()->setSectionResizeMode(0, QHeaderView::Interactive);
    ui->settings->header()->setSectionResizeMode(1, QHeaderView::Interactive);

    connect(QApplication::clipboard(), SIGNAL(dataChanged()), this, SLOT(clipboardChanged()));
}

DialogSettings::~DialogSettings()
{
    delete ui;
}

void DialogSettings::load(Profile *profile)
{
    Q_ASSERT(profile != 0);

    ui->settings->clear();

    if (m_profile)
    {
        delete m_profile;
    }

    m_profile = new Profile(this);
    m_profile->copySettings(*profile);

    populateTree();

    filterAccelerator(m_showAccelerators);
    filterAlias(m_showAliases);
    filterTimer(m_showTimers);
    filterTrigger(m_showTriggers);
    filterVariable(m_showVariables);

    clipboardChanged();
}

void DialogSettings::populateTree()
{
    QTreeWidgetItem *top = ui->settings->invisibleRootItem();

    populateBranch(top, profile()->rootGroup());

    on_settings_itemSelectionChanged();
    ui->settings->sortItems(0, Qt::AscendingOrder);
}

void DialogSettings::populateBranch(QTreeWidgetItem *parent, Group *group)
{
    foreach (Group *child, group->groups())
    {
        QTreeWidgetItem *item = populateItem(parent, child, QIcon(":/icons/group"), TGroup);
        if (m_expanded.contains(child->path()))
        {
            ui->settings->expandItem(item);
        }
        populateBranch(item, child);
    }

    foreach (Accelerator *accelerator, group->accelerators())
    {
        populateItem(parent, accelerator, QIcon(":/icons/accelerator"), TAccelerator);
    }

    foreach (Alias *alias, group->aliases())
    {
        populateItem(parent, alias, QIcon(":/icons/alias"), TAlias);
    }

    foreach (Timer *timer, group->timers())
    {
        populateItem(parent, timer, QIcon(":/icons/timer"), TTimer);
    }

    foreach (Trigger *trigger, group->triggers())
    {
        populateItem(parent, trigger, QIcon(":/icons/trigger"), TTrigger);
    }

    foreach (Variable *variable, group->variables())
    {
        populateItem(parent, variable, QIcon(":/icons/variable"), TVariable);
    }
}

QTreeWidgetItem *DialogSettings::populateItem(QTreeWidgetItem *parent, ProfileItem *item, const QIcon &icon, ItemType type)
{
    QTreeWidgetItem *node = new QTreeWidgetItem(parent, QStringList() << item->name(), type);
    node->setIcon(Name, icon);
//    node->setFlags(node->flags() | Qt::ItemIsEditable);
    ui->settings->blockSignals(true);
    node->setData(0, SETTINGS_ROLE_ITEM, QVariant(QMetaType::QObjectStar, &item));

    updateItem(node, item);
    ui->settings->blockSignals(false);

    return node;
}

void DialogSettings::updateItem(QTreeWidgetItem *node, ProfileItem *item)
{
    if (qobject_cast<Variable *>(item) == 0)
    {
        node->setCheckState(Name, item->enabledFlag()?Qt::Checked:Qt::Unchecked);
    }
    node->setData(Name, Qt::DisplayRole, item->name());
    node->setData(Value, Qt::DisplayRole, item->value());
    if (item->sequence() > 0)
    {
        node->setData(Sequence, Qt::DisplayRole, item->sequence());
    }
}

void DialogSettings::on_settings_itemSelectionChanged()
{
    ui->copySetting->setEnabled(ui->settings->selectedItems().count() > 0);
    ui->cutSetting->setEnabled(ui->settings->selectedItems().count() > 0);
    ui->deleteSetting->setEnabled(ui->settings->selectedItems().count() > 0);
    ui->editSetting->setEnabled(ui->settings->selectedItems().count() == 1);
}

void DialogSettings::toggle(QTreeWidgetItem *item, ItemType type, bool flag)
{
    for (int n = 0; n < item->childCount(); n++)
    {
        QTreeWidgetItem *child = item->child(n);
        ItemType childType = (ItemType)child->type();
        if (childType == type)
        {
            child->setHidden(!flag);

            // Check for the hiding of the currently selected item
            // Move the selection to the parent, if any, or else no selection
            if (child->isHidden() && child->isSelected())
            {
                if (child->parent())
                {
                    ui->settings->setCurrentItem(item);
                }
                else
                {
                    ui->settings->setCurrentItem(0);
                }
            }
        }
        toggle(child, type, flag);
    }
}

QTreeWidgetItem *DialogSettings::parentGroup()
{
    QTreeWidgetItem *item = ui->settings->currentItem();
    QTreeWidgetItem *parent = 0;
    Group *group = 0;
    if (!item)
    {
        parent = ui->settings->invisibleRootItem();
        group = profile()->rootGroup();
    }
    else
    {
        ItemType type = (ItemType)item->type();
        QObject *obj = qvariant_cast<QObject *>(item->data(0, SETTINGS_ROLE_ITEM));
        if (type == TGroup)
        {
            parent = item;
            group = qobject_cast<Group *>(obj);
        }
        else
        {
            parent = item->parent();
            if (parent == 0)
            {
                parent = ui->settings->invisibleRootItem();
                group = profile()->rootGroup();
            }
            else
            {
                group = qobject_cast<Group *>(obj->parent());
            }
        }
    }

    Q_ASSERT(parent != 0);
    Q_ASSERT(group != 0);
    profile()->setActiveGroup(group);

    return parent;
}

void DialogSettings::filterAccelerator(bool checked)
{
    m_showAccelerators = checked;
    toggle(ui->settings->invisibleRootItem(), TAccelerator, checked);
}

void DialogSettings::filterAlias(bool checked)
{
    m_showAliases = checked;
    toggle(ui->settings->invisibleRootItem(), TAlias, checked);
}

void DialogSettings::filterTimer(bool checked)
{
    m_showTimers = checked;
    toggle(ui->settings->invisibleRootItem(), TTimer, checked);
}

void DialogSettings::filterTrigger(bool checked)
{
    m_showTriggers = checked;
    toggle(ui->settings->invisibleRootItem(), TTrigger, checked);
}

void DialogSettings::filterVariable(bool checked)
{
    m_showVariables = checked;
    toggle(ui->settings->invisibleRootItem(), TVariable, checked);
}


void DialogSettings::addAccelerator()
{
    QTreeWidgetItem *parent = parentGroup();

    Accelerator* accelerator = new Accelerator(this);
    DialogAccelerator dlg(accelerator);
    if (dlg.exec() == QDialog::Accepted)
    {
        profile()->addAccelerator(accelerator);

        parent->setExpanded(true);

        QTreeWidgetItem *itemAccelerator = populateItem(parent, accelerator, QIcon(":/icons/accelerator"), TAccelerator);

        ui->settings->setCurrentItem(itemAccelerator);
    }
    else
    {
        delete accelerator;
    }
}

void DialogSettings::addAlias()
{
    QTreeWidgetItem *parent = parentGroup();

    Alias* alias = new Alias(this);
    DialogAlias dlg(alias);
    if (dlg.exec() == QDialog::Accepted)
    {
        profile()->addAlias(alias);

        parent->setExpanded(true);

        QTreeWidgetItem *itemAlias = populateItem(parent, alias, QIcon(":/icons/alias"), TAlias);

        ui->settings->setCurrentItem(itemAlias);
    }
    else
    {
        delete alias;
    }
}

void DialogSettings::addGroup()
{
    QTreeWidgetItem *parent = parentGroup();

    Group* group = new Group(this);
    DialogGroup dlg(group, this);
    if (dlg.exec() == QDialog::Accepted)
    {
        profile()->addGroup(group);

        parent->setExpanded(true);

        QTreeWidgetItem *itemGroup = populateItem(parent, group, QIcon(":/icons/group"), TGroup);

        ui->settings->setCurrentItem(itemGroup);
    }
    else
    {
        delete group;
    }
}

void DialogSettings::addTimer()
{
    QTreeWidgetItem *parent = parentGroup();

    Timer* timer = new Timer(this);
    DialogTimer dlg(timer);
    if (dlg.exec() == QDialog::Accepted)
    {
        profile()->addTimer(timer);

        parent->setExpanded(true);

        QTreeWidgetItem *itemTimer = populateItem(parent, timer, QIcon(":/icons/timer"), TTimer);

        ui->settings->setCurrentItem(itemTimer);
    }
    else
    {
        delete timer;
    }
}

void DialogSettings::addTrigger()
{
    QTreeWidgetItem *parent = parentGroup();

    Trigger* trigger = new Trigger(this);
    DialogTrigger dlg(trigger);
    if (dlg.exec() == QDialog::Accepted)
    {
        profile()->addTrigger(trigger);

        parent->setExpanded(true);

        QTreeWidgetItem *itemTrigger = populateItem(parent, trigger, QIcon(":/icons/trigger"), TTrigger);

        ui->settings->setCurrentItem(itemTrigger);
    }
    else
    {
        delete trigger;
    }
}

void DialogSettings::addVariable()
{
    QTreeWidgetItem *parent = parentGroup();

    Variable* variable = new Variable(this);
    DialogVariable dlg(variable);
    if (dlg.exec() == QDialog::Accepted)
    {
        profile()->addVariable(variable);

        parent->setExpanded(true);

        QTreeWidgetItem *itemVariable = populateItem(parent, variable, QIcon(":/icons/variable"), TVariable);

        ui->settings->setCurrentItem(itemVariable);
    }
    else
    {
        delete variable;
    }
}

void DialogSettings::clipboardChanged()
{
    ui->pasteSetting->setEnabled(Profile::validateXml(QApplication::clipboard()->text()));
}

void DialogSettings::on_editSetting_clicked()
{
    QTreeWidgetItem *item = ui->settings->currentItem();
    if (!item)
    {
        ui->editSetting->setEnabled(false);
        return;
    }

    on_settings_itemDoubleClicked(item, 0);
}

void DialogSettings::on_deleteSetting_clicked()
{
    QList<QTreeWidgetItem *> items(ui->settings->selectedItems());
    if (items.isEmpty())
    {
        ui->deleteSetting->setEnabled(false);
        return;
    }

    QString title(tr("Confirm Delete"));
    QString msg;
    if (items.size() == 1)
    {
        msg = tr("Are you sure you want to remove this setting?");
    }
    else
    {
        msg = tr("Are you sure you want to remove %1 settings?").arg(items.size());
    }

    if (QMessageBox::question(this, title, msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes) == QMessageBox::Yes)
    {
        ui->settings->clearSelection();
        ui->settings->setCurrentItem(0);

        foreach (QTreeWidgetItem *item, items)
        {
            ProfileItem *obj = qvariant_cast<ProfileItem *>(item->data(0, SETTINGS_ROLE_ITEM));
            if (obj == 0)
            {
                qDebug() << "deleteSetting: null object";
                delete item;
                continue;
            }

            ItemType itemType = (ItemType)item->type();
            switch (itemType)
            {
            case TAccelerator:
                profile()->deleteAccelerator(qobject_cast<Accelerator *>(obj));
                break;

            case TAlias:
                profile()->deleteAlias(qobject_cast<Alias *>(obj));
                break;

            case TGroup:
            {
                Group *group = qobject_cast<Group *>(obj);
                m_expanded.removeOne(group->path());
                profile()->deleteGroup(group);
            }
                break;

            case TTimer:
                profile()->deleteTimer(qobject_cast<Timer *>(obj));
                break;

            case TTrigger:
                profile()->deleteTrigger(qobject_cast<Trigger *>(obj));
                break;

            case TVariable:
                profile()->deleteVariable(qobject_cast<Variable *>(obj));
                break;
            }

            delete item;
        }
    }
}

void DialogSettings::on_settings_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    Q_UNUSED(column);

    if (item == 0)
    {
        return;
    }

    ProfileItem *obj = qvariant_cast<ProfileItem *>(item->data(0, SETTINGS_ROLE_ITEM));
    if (obj == 0)
    {
        return;
    }

    ItemType itemType = (ItemType)item->type();
    switch (itemType)
    {
        case TAccelerator:
        {
            Accelerator *accelerator = qobject_cast<Accelerator *>(obj);
            DialogAccelerator *dlg = new DialogAccelerator(accelerator, this);
            if (dlg->exec() == QDialog::Accepted)
            {
                updateItem(item, accelerator);
            }
        }
        break;

        case TAlias:
        {
            Alias *alias = qobject_cast<Alias *>(obj);
            DialogAlias *dlg = new DialogAlias(alias, this);
            if (dlg->exec() == QDialog::Accepted)
            {
                updateItem(item, alias);
            }
        }
        break;

        case TGroup:
        {
            Group *group = qobject_cast<Group *>(obj);
            DialogGroup *dlg = new DialogGroup(group, this);
            if (dlg->exec() == QDialog::Accepted)
            {
                updateItem(item, group);
            }
        }
        break;

        case TTimer:
        {
            Timer *timer = qobject_cast<Timer *>(obj);
            DialogTimer *dlg = new DialogTimer(timer, this);
            if (dlg->exec() == QDialog::Accepted)
            {
                updateItem(item, timer);
            }
        }
        break;

        case TTrigger:
        {
            Trigger *trigger = qobject_cast<Trigger *>(obj);
            DialogTrigger *dlg = new DialogTrigger(trigger, this);
            if (dlg->exec() == QDialog::Accepted)
            {
                updateItem(item, trigger);
            }
        }
        break;

        case TVariable:
        {
            Variable *variable = qobject_cast<Variable *>(obj);
            DialogVariable *dlg = new DialogVariable(variable, this);
            if (dlg->exec() == QDialog::Accepted)
            {
                updateItem(item, variable);
            }
        }
        break;
    }
}

void DialogSettings::on_settings_itemChanged(QTreeWidgetItem *item, int column)
{
    if (item == 0)
    {
        return;
    }

    ProfileItem *obj = qvariant_cast<ProfileItem *>(item->data(0, SETTINGS_ROLE_ITEM));
    if (obj == 0)
    {
        return;
    }

//    ItemType itemType = (ItemType)item->type();
    switch (column)
    {
        case 0:
            // TODO: validate here?
//            obj->setName(item->text(column));
            obj->enable(item->checkState(column) == Qt::Checked);
            break;

//        case 2:
//            bool ok = false;
//            int sequence = item->text(column).toInt(&ok);
//            if (ok)
//            {
//                obj->setSequence(sequence);
//            }
//            break;
    }

//    switch (itemType)
//    {
//        case TGroup:
//        {
//            Group *group = qobject_cast<Group *>(obj);
//        }
//        break;

//        case TAlias:
//        {
//            Alias *alias = qobject_cast<Alias *>(obj);
//        }
//        break;
//    }
}

void DialogSettings::on_settings_itemExpanded(QTreeWidgetItem *item)
{
    Group *group = qvariant_cast<Group *>(item->data(0, SETTINGS_ROLE_ITEM));
    if (!group)
    {
        qDebug() << "expanded a non-group";
        return;
    }

    if (!m_expanded.contains(group->path()))
    {
        m_expanded.append(group->path());
    }
}

void DialogSettings::on_settings_itemCollapsed(QTreeWidgetItem *item)
{
    Group *group = qvariant_cast<Group *>(item->data(0, SETTINGS_ROLE_ITEM));
    if (!group)
    {
        qDebug() << "collapsed a non-group";
        return;
    }

    if (m_expanded.contains(group->path()))
    {
        m_expanded.removeOne(group->path());
    }
}

void DialogSettings::on_buttonBox_accepted()
{
    QDialog::accept();
}

void DialogSettings::on_buttonBox_rejected()
{
    QDialog::reject();
}

void DialogSettings::on_copySetting_clicked()
{
    QString output;
    QXmlStreamWriter xml(&output);

    xml.setAutoFormatting(true);
    xml.setAutoFormattingIndent(2);

    xml.writeStartDocument();
    xml.writeStartElement("settings");

    foreach (QTreeWidgetItem *item, ui->settings->selectedItems())
    {
        ProfileItem *obj = 0;
        QVariant itemData(item->data(0, SETTINGS_ROLE_ITEM));
        ItemType itemType = (ItemType)item->type();
        switch (itemType)
        {
            case TAccelerator:
            {
                obj = qvariant_cast<Accelerator *>(itemData);
            }
            break;

            case TAlias:
            {
                obj = qvariant_cast<Alias *>(itemData);
            }
            break;

            case TGroup:
            {
                obj = qvariant_cast<Group *>(itemData);
            }
            break;

            case TTimer:
            {
                obj = qvariant_cast<Timer *>(itemData);
            }
            break;

            case TTrigger:
            {
                obj = qvariant_cast<Trigger *>(itemData);
            }
            break;

            case TVariable:
            {
                obj = qvariant_cast<Variable *>(itemData);
            }
            break;
        }

        if (obj == 0)
        {
            ui->copySetting->setEnabled(false);
            return;
        }

        try
        {
            obj->toXml(xml);
        }
        catch (XmlException *xe)
        {
            qDebug() << xe->warnings();
            delete xe;
        }
        catch (...)
        {
            qDebug() << "unknown exception";
        }
    }

    xml.writeEndElement();
    xml.writeEndDocument();

    QClipboard *clipboard = QApplication::clipboard();
    clipboard->setText(output);
}

void DialogSettings::on_pasteSetting_clicked()
{
    const QClipboard *clipboard = QApplication::clipboard();

    try
    {
        parentGroup();

        m_expanded.append(profile()->activeGroup()->path());

        QXmlStreamReader xml(clipboard->text());
        profile()->appendXml(xml);

        ui->settings->clear();
        populateTree();
    }
    catch (XmlException *xe)
    {
        QMessageBox mb(QMessageBox::Warning, tr("Invalid Settings"), tr("Unable to process XML from the clipboard."));
        mb.setDetailedText(xe->warnings().join('\n'));
        mb.exec();

        delete xe;
        ui->pasteSetting->setEnabled(false);
    }
}

void DialogSettings::on_cutSetting_clicked()
{
    on_copySetting_clicked();
    on_deleteSetting_clicked();
}
