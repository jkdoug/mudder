#ifndef MAPWIDGET_H
#define MAPWIDGET_H

#include <QLine>
#include <QMap>
#include <QPaintEvent>
#include <QPair>
#include <QPolygon>
#include <QRect>
#include <QWidget>
#include "mapexit.h"
#include <math.h>

class MapEngine;
class MapRoom;

class MapWidget : public QWidget
{
    Q_OBJECT
public:
    explicit MapWidget(QWidget *parent = 0);

    MapEngine *mapper(const QString &name = QString()) const;
    void setMapper(const QString &name, MapEngine *mapper);
    bool switchMapper(const QString &name);

    int exitLength() const { return 15; }
    int exitLength45() const { return sqrt(15 * 15 + 15 * 15); }

protected:
    void paintEvent(QPaintEvent *e);

    void drawRoom(QPainter *p, MapRoom *room, int depth = 0);

private:
    QMap<QString, MapEngine *> m_engines;

    QRect m_roomRect;
    QMap<MapExit::Direction, QLine> m_exitLines;
    QMap<MapExit::Direction, QPolygon> m_exitShapes;

    QMap<int, bool> m_roomsDrawn;
    QMap<QPair<int, int>, int> m_coordinatesDrawn;
};

#endif // MAPWIDGET_H
