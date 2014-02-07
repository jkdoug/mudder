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


#include "configwidget.h"
#include "ui_configwidget.h"
#include <QAbstractItemDelegate>
#include <QDebug>
#include <QPainter>

class ConfigWidgetDelegate : public QAbstractItemDelegate
{
public:
    ConfigWidgetDelegate(QObject *parent=0) :
        QAbstractItemDelegate(parent)
    {}

    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex& index) const
    {
        QStyleOptionViewItem opt(option);
        if (opt.state.testFlag(QStyle::State_Selected))
        {
            painter->fillRect(opt.rect, opt.palette.highlight());
        }

        QIcon icon(index.data(Qt::DecorationRole).value<QIcon>());
        QSize ds(opt.decorationSize);
        QRect decorationRect(opt.rect.x() + 4, opt.rect.top() + 4, opt.rect.width() - 8, ds.height());

        icon.paint(painter, decorationRect, Qt::AlignHCenter | Qt::AlignTop, opt.state.testFlag(QStyle::State_Enabled) ? ((opt.state.testFlag(QStyle::State_Selected)) && opt.showDecorationSelected ? QIcon::Selected : QIcon::Normal) : QIcon::Disabled);

        QString displayText(index.data(Qt::DisplayRole).toString());
        QRect displayRect(opt.rect.adjusted(2, ds.height() + 2, -2, -2));

        painter->setPen(opt.palette.color(option.state.testFlag(QStyle::State_Selected) ? QPalette::HighlightedText : QPalette::Text));
        painter->drawText(displayRect, Qt::AlignHCenter | Qt::AlignBottom | Qt::TextWordWrap, opt.fontMetrics.elidedText(displayText, opt.textElideMode, displayRect.width()));
    }

    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
    {
        QSize ds(option.decorationSize);
        int totalWidth = option.fontMetrics.width(index.data(Qt::DisplayRole).toString());
        QSize ts(totalWidth, option.fontMetrics.height());
        return QSize(qBound(ds.width(), ts.width(), option.rect.width()), ds.height() + 6 + ts.height());
    }
};

ConfigWidget::ConfigWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ConfigWidget)
{
    m_loading = false;
    m_saving = false;
    m_validating = false;
    m_applying = false;
    m_discarding = false;

    ui->setupUi(this);

    ui->stack->setCurrentIndex(-1);
    ui->list->setItemDelegate(new ConfigWidgetDelegate(ui->list));

    connect(ui->list, SIGNAL(currentRowChanged(int)), SLOT(setCurrentIndex(int)));
}

ConfigWidget::~ConfigWidget()
{
    delete ui;
}

int ConfigWidget::currentIndex() const
{
    return ui->stack->currentIndex();
}

int ConfigWidget::count() const
{
    return ui->stack->count();
}

QWidget * ConfigWidget::page(int index) const
{
    return ui->stack->widget(index);
}

QWidget * ConfigWidget::currentPage() const
{
    return ui->stack->currentWidget();
}

QSize ConfigWidget::iconSize() const
{
    return ui->list->iconSize();
}

void ConfigWidget::addPage(QWidget *group, const QIcon &icon, const QString &name)
{
    insertPage(count(), group, icon, name);
}

void ConfigWidget::insertPage(int index, QWidget *group, const QIcon &icon, const QString &name)
{
    ui->stack->insertWidget(index, group);
    QListWidgetItem *item = new QListWidgetItem;

    if (name.isNull())
    {
        item->setText(group->windowTitle());
    }
    else
    {
        item->setText(name);
        group->setWindowTitle(name);
    }

    if (icon.isNull())
    {
        item->setIcon(group->windowIcon());
    }
    else
    {
        item->setIcon(icon);
        group->setWindowIcon(icon);
    }

    ui->list->insertItem(index, item);
    if (count() == 1)
    {
        ui->title->setText(ui->list->item(index)->text());
        setCurrentIndex(0);
    }
    group->installEventFilter(this);
}

bool ConfigWidget::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::WindowTitleChange)
    {
        QWidget *w = qobject_cast<QWidget*>(object);
        for (int idx = 0; idx < count(); idx++)
        {
            if (page(idx) == w)
            {
                setPageName(idx, w->windowTitle());
            }
        }

        return false;
    }

    return QWidget::eventFilter(object, event);
}

void ConfigWidget::setCurrentIndex(int index)
{
    if (index < 0 || index >= ui->list->count())
    {
        return;
    }

    ui->stack->setCurrentIndex(index);
    ui->list->setCurrentRow(index);
    ui->title->setText(ui->list->item(index)->text());

    emit currentIndexChanged(index);
}

void ConfigWidget::setPageIcon(int index, const QIcon &icon)
{
    if (index < 0 || index >= ui->list->count())
    {
        return;
    }

    page(index)->setWindowIcon(icon);
    ui->list->item(index)->setIcon(icon);
}

void ConfigWidget::setPageName(int index, const QString &name)
{
    if (index < 0 || index >= ui->list->count())
    {
        return;
    }

    page(index)->setWindowTitle(name);
    ui->list->item(index)->setText(name);

    if (index == currentIndex())
    {
        ui->title->setText(name);
    }
}

void ConfigWidget::setIconSize(const QSize &size)
{
    ui->list->setIconSize(size);
}

void ConfigWidget::load()
{
    if (m_loading)
    {
        return;
    }

    m_loading = true;
    emit loading();
    m_loading = false;
}

void ConfigWidget::save()
{
    if (m_saving)
    {
        return;
    }

    m_saving = true;
    emit saving();
    m_saving = false;
}

bool ConfigWidget::validate()
{
    if (m_validating)
    {
        return false;
    }

    m_validating = true;
    bool result = true;
    for (int w = 0; w < ui->stack->count(); w++)
    {
        bool ret = true;
        QMetaObject::invokeMethod(ui->stack->widget(w), "validate", Qt::DirectConnection,
                                  Q_RETURN_ARG(bool, ret));
        if (!ret)
        {
            result = false;
            break;
        }
    }
    m_validating = false;

    return result;
}

void ConfigWidget::apply()
{
    if (m_applying)
    {
        return;
    }

    m_applying = true;
    emit applying(currentIndex());
    QMetaObject::invokeMethod(currentPage(), "apply");
    m_applying = false;
}

void ConfigWidget::discard()
{
    if (m_discarding)
    {
        return;
    }

    m_discarding = true;
    emit discarding(currentIndex());
    QMetaObject::invokeMethod(currentPage(), "discard");
    m_discarding = false;
}
