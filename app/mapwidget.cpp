#include "mapwidget.h"
#include "mapengine.h"
#include "maproom.h"
#include <QColor>
#include <QDebug>
#include <QLine>
#include <QPainter>
#include <QRect>
#include <QSqlError>
#include <QVector>

MapWidget::MapWidget(QWidget *parent) :
    QWidget(parent)
{
    int roomSize = 10;
    int halfRoom = roomSize / 2;
    int exitLength = 15;
    int less1 = exitLength - 1;

    m_roomRect = QRect(-halfRoom, -halfRoom, roomSize, roomSize);

    m_exitLines.insert(MapExit::South, QLine(0, halfRoom, 0, halfRoom + less1));
    m_exitLines.insert(MapExit::Southwest, QLine(-halfRoom, halfRoom, -halfRoom - less1, halfRoom + less1));
    m_exitLines.insert(MapExit::West, QLine(-halfRoom, 0, -halfRoom - less1, 0));
    m_exitLines.insert(MapExit::Northwest, QLine(-halfRoom, -halfRoom, -halfRoom - less1, -halfRoom - less1));
    m_exitLines.insert(MapExit::North, QLine(0, -halfRoom, 0, -halfRoom - less1));
    m_exitLines.insert(MapExit::Northeast, QLine(halfRoom, -halfRoom, halfRoom + less1, -halfRoom - less1));
    m_exitLines.insert(MapExit::East, QLine(halfRoom, 0, halfRoom + less1, 0));
    m_exitLines.insert(MapExit::Southeast, QLine(halfRoom, halfRoom, halfRoom + less1, halfRoom + less1));

    QPolygon exitUp(QVector<QPoint>() << QPoint(halfRoom + 3, -3) << QPoint(halfRoom + 6, -6) << QPoint(halfRoom + 9, -3));
    QPolygon exitDown(QVector<QPoint>() << QPoint(halfRoom + 3, 3) << QPoint(halfRoom + 6, 6) << QPoint(halfRoom + 9, 3));

    m_exitShapes.insert(MapExit::Up, exitUp);
    m_exitShapes.insert(MapExit::Down, exitDown);
}

MapEngine * MapWidget::mapper(const QString &name) const
{
    if (name.isEmpty())
    {
        return m_engines.value("__currentMap", 0);
    }

    return m_engines.value(name, 0);
}

void MapWidget::setMapper(const QString &name, MapEngine *mapper)
{
    if (name.compare("__currentMap") == 0)
    {
        qWarning() << "Do not use setMapper to change the current map";
        return;
    }

    if (!mapper)
    {
        m_engines.remove(name);
    }
    else
    {
        m_engines.insert(name, mapper);
    }
}

bool MapWidget::switchMapper(const QString &name)
{
    MapEngine *map = mapper(name);
    if (!map)
    {
        return false;
    }

    if (mapper() != map)
    {
        m_engines.insert("__currentMap", map);
        update();
    }

    return true;
}

void MapWidget::paintEvent(QPaintEvent *e)
{
    Q_UNUSED(e);

    MapEngine *map = mapper();
    if (!map)
    {
        return;
    }

    QPainter painter(this);
    if (!painter.isActive())
    {
        return;
    }

//    painter.setRenderHint(QPainter::Antialiasing);

    painter.fillRect(rect(), map->background());

    painter.translate(width() / 2, height() / 2);

    MapRoom *startRoom = map->lookupRoom(map->currentRoom());
    if (!startRoom)
    {
        return;
    }

    m_roomsDrawn.clear();
    m_coordinatesDrawn.clear();

    drawRoom(&painter, startRoom);
}

void MapWidget::drawRoom(QPainter *p, MapRoom *room, int depth)
{
    m_coordinatesDrawn.insert(qMakePair(room->x(), room->y()), room->id());

    if (m_roomsDrawn.contains(room->id()) || depth > 100)
    {
        return;
    }

    m_roomsDrawn.insert(room->id(), true);

    foreach (MapExit *exit, room->exits())
    {
        MapRoom *neighbor = mapper()->lookupRoom(exit->leadsTo());
        if (neighbor && neighbor->areaId() == room->areaId())
        {
            if (neighbor->z() != room->z())
            {
                continue;
            }

            int dx = neighbor->x() - room->x();
            int dy = room->y() - neighbor->y();

//            if (abs(dx) > width() / 2 || abs(dy) > height() / 2)
//            {
//                continue;
//            }

            int w = m_roomRect.width() + m_exitLines.value(MapExit::East).dx();
            int h = m_roomRect.height() + m_exitLines.value(MapExit::South).dy();

            QLine exitInfo(m_exitLines.value(exit->direction()));
            qDebug() << "room" << room->id() << room->coordinates() << dx << dy;
            qDebug() << "exit" << exit->direction() << exitInfo;

            p->save();
            p->setPen(Qt::white);
            p->drawLine(exitInfo);
            p->restore();

            p->save();
            p->translate(w * dx, h * dy);
            drawRoom(p, neighbor, depth + 1);
            p->restore();
        }
    }

    p->save();
    p->setPen(Qt::black);
    p->setBrush(mapper()->lookupEnvironment(room->environmentId())->color());
    p->drawRect(m_roomRect);
    p->restore();
}
