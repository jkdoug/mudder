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


#ifndef FIXED_H
#define FIXED_H

/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/


#include <QDebug>
#include <QPoint>
#include <QSize>

struct Fixed {
private:
    Q_DECL_CONSTEXPR Fixed(int val, int) : val(val) {} // 2nd int is just a dummy for disambiguation
public:
    Q_DECL_CONSTEXPR Fixed() : val(0) {}
    Q_DECL_CONSTEXPR Fixed(int i) : val(i<<6) {}
    Q_DECL_CONSTEXPR Fixed(long i) : val(i<<6) {}
    Fixed &operator=(int i) { val = (i<<6); return *this; }
    Fixed &operator=(long i) { val = (i<<6); return *this; }

    Q_DECL_CONSTEXPR static Fixed fromReal(qreal r) { return fromFixed((int)(r*qreal(64))); }
    Q_DECL_CONSTEXPR static Fixed fromFixed(int fixed) { return Fixed(fixed,0); } // uses private ctor

    Q_DECL_CONSTEXPR inline int value() const { return val; }
    inline void setValue(int value) { val = value; }

    Q_DECL_CONSTEXPR inline int toInt() const { return (((val)+32) & -64)>>6; }
    Q_DECL_CONSTEXPR inline qreal toReal() const { return ((qreal)val)/(qreal)64; }

    Q_DECL_CONSTEXPR inline int truncate() const { return val>>6; }
    Q_DECL_CONSTEXPR inline Fixed round() const { return fromFixed(((val)+32) & -64); }
    Q_DECL_CONSTEXPR inline Fixed floor() const { return fromFixed((val) & -64); }
    Q_DECL_CONSTEXPR inline Fixed ceil() const { return fromFixed((val+63) & -64); }

    Q_DECL_CONSTEXPR inline Fixed operator+(int i) const { return fromFixed((val + (i<<6))); }
    Q_DECL_CONSTEXPR inline Fixed operator+(uint i) const { return fromFixed((val + (i<<6))); }
    Q_DECL_CONSTEXPR inline Fixed operator+(const Fixed &other) const { return fromFixed((val + other.val)); }
    inline Fixed &operator+=(int i) { val += (i<<6); return *this; }
    inline Fixed &operator+=(uint i) { val += (i<<6); return *this; }
    inline Fixed &operator+=(const Fixed &other) { val += other.val; return *this; }
    Q_DECL_CONSTEXPR inline Fixed operator-(int i) const { return fromFixed((val - (i<<6))); }
    Q_DECL_CONSTEXPR inline Fixed operator-(uint i) const { return fromFixed((val - (i<<6))); }
    Q_DECL_CONSTEXPR inline Fixed operator-(const Fixed &other) const { return fromFixed((val - other.val)); }
    inline Fixed &operator-=(int i) { val -= (i<<6); return *this; }
    inline Fixed &operator-=(uint i) { val -= (i<<6); return *this; }
    inline Fixed &operator-=(const Fixed &other) { val -= other.val; return *this; }
    Q_DECL_CONSTEXPR inline Fixed operator-() const { return fromFixed(-val); }

    Q_DECL_CONSTEXPR inline bool operator==(const Fixed &other) const { return val == other.val; }
    Q_DECL_CONSTEXPR inline bool operator!=(const Fixed &other) const { return val != other.val; }
    Q_DECL_CONSTEXPR inline bool operator<(const Fixed &other) const { return val < other.val; }
    Q_DECL_CONSTEXPR inline bool operator>(const Fixed &other) const { return val > other.val; }
    Q_DECL_CONSTEXPR inline bool operator<=(const Fixed &other) const { return val <= other.val; }
    Q_DECL_CONSTEXPR inline bool operator>=(const Fixed &other) const { return val >= other.val; }
    Q_DECL_CONSTEXPR inline bool operator!() const { return !val; }

    inline Fixed &operator/=(int x) { val /= x; return *this; }
    inline Fixed &operator/=(const Fixed &o) {
        if (o.val == 0) {
            val = 0x7FFFFFFFL;
        } else {
            bool neg = false;
            qint64 a = val;
            qint64 b = o.val;
            if (a < 0) { a = -a; neg = true; }
            if (b < 0) { b = -b; neg = !neg; }

            int res = (int)(((a << 6) + (b >> 1)) / b);

            val = (neg ? -res : res);
        }
        return *this;
    }
    Q_DECL_CONSTEXPR inline Fixed operator/(int d) const { return fromFixed(val/d); }
    inline Fixed operator/(Fixed b) const { Fixed f = *this; return (f /= b); }
    inline Fixed operator>>(int d) const { Fixed f = *this; f.val >>= d; return f; }
    inline Fixed &operator*=(int i) { val *= i; return *this; }
    inline Fixed &operator*=(uint i) { val *= i; return *this; }
    inline Fixed &operator*=(const Fixed &o) {
        bool neg = false;
        qint64 a = val;
        qint64 b = o.val;
        if (a < 0) { a = -a; neg = true; }
        if (b < 0) { b = -b; neg = !neg; }

        int res = (int)((a * b + 0x20L) >> 6);
        val = neg ? -res : res;
        return *this;
    }
    Q_DECL_CONSTEXPR inline Fixed operator*(int i) const { return fromFixed(val * i); }
    Q_DECL_CONSTEXPR inline Fixed operator*(uint i) const { return fromFixed(val * i); }
    inline Fixed operator*(const Fixed &o) const { Fixed f = *this; return (f *= o); }

private:
    Q_DECL_CONSTEXPR Fixed(qreal i) : val((int)(i*qreal(64))) {}
    Fixed &operator=(qreal i) { val = (int)(i*qreal(64)); return *this; }
    Q_DECL_CONSTEXPR inline Fixed operator+(qreal i) const { return fromFixed((val + (int)(i*qreal(64)))); }
    inline Fixed &operator+=(qreal i) { val += (int)(i*64); return *this; }
    Q_DECL_CONSTEXPR inline Fixed operator-(qreal i) const { return fromFixed((val - (int)(i*qreal(64)))); }
    inline Fixed &operator-=(qreal i) { val -= (int)(i*64); return *this; }
    inline Fixed &operator/=(qreal r) { val = (int)(val/r); return *this; }
    Q_DECL_CONSTEXPR inline Fixed operator/(qreal d) const { return fromFixed((int)(val/d)); }
    inline Fixed &operator*=(qreal d) { val = (int) (val*d); return *this; }
    Q_DECL_CONSTEXPR inline Fixed operator*(qreal d) const { return fromFixed((int) (val*d)); }
    int val;
};
Q_DECLARE_TYPEINFO(Fixed, Q_PRIMITIVE_TYPE);

#define FIXED_MAX (INT_MAX/256)

Q_DECL_CONSTEXPR inline int qRound(const Fixed &f) { return f.toInt(); }
Q_DECL_CONSTEXPR inline int qFloor(const Fixed &f) { return f.floor().truncate(); }

Q_DECL_CONSTEXPR inline Fixed operator*(int i, const Fixed &d) { return d*i; }
Q_DECL_CONSTEXPR inline Fixed operator+(int i, const Fixed &d) { return d+i; }
Q_DECL_CONSTEXPR inline Fixed operator-(int i, const Fixed &d) { return -(d-i); }
Q_DECL_CONSTEXPR inline Fixed operator*(uint i, const Fixed &d) { return d*i; }
Q_DECL_CONSTEXPR inline Fixed operator+(uint i, const Fixed &d) { return d+i; }
Q_DECL_CONSTEXPR inline Fixed operator-(uint i, const Fixed &d) { return -(d-i); }
// Q_DECL_CONSTEXPR inline Fixed operator*(qreal d, const Fixed &d2) { return d2*d; }

Q_DECL_CONSTEXPR inline bool operator==(const Fixed &f, int i) { return f.value() == (i<<6); }
Q_DECL_CONSTEXPR inline bool operator==(int i, const Fixed &f) { return f.value() == (i<<6); }
Q_DECL_CONSTEXPR inline bool operator!=(const Fixed &f, int i) { return f.value() != (i<<6); }
Q_DECL_CONSTEXPR inline bool operator!=(int i, const Fixed &f) { return f.value() != (i<<6); }
Q_DECL_CONSTEXPR inline bool operator<=(const Fixed &f, int i) { return f.value() <= (i<<6); }
Q_DECL_CONSTEXPR inline bool operator<=(int i, const Fixed &f) { return (i<<6) <= f.value(); }
Q_DECL_CONSTEXPR inline bool operator>=(const Fixed &f, int i) { return f.value() >= (i<<6); }
Q_DECL_CONSTEXPR inline bool operator>=(int i, const Fixed &f) { return (i<<6) >= f.value(); }
Q_DECL_CONSTEXPR inline bool operator<(const Fixed &f, int i) { return f.value() < (i<<6); }
Q_DECL_CONSTEXPR inline bool operator<(int i, const Fixed &f) { return (i<<6) < f.value(); }
Q_DECL_CONSTEXPR inline bool operator>(const Fixed &f, int i) { return f.value() > (i<<6); }
Q_DECL_CONSTEXPR inline bool operator>(int i, const Fixed &f) { return (i<<6) > f.value(); }

#ifndef QT_NO_DEBUG_STREAM
inline QDebug &operator<<(QDebug &dbg, const Fixed &f)
{ return dbg << f.toReal(); }
#endif

struct FixedPoint {
    Fixed x;
    Fixed y;
    Q_DECL_CONSTEXPR inline FixedPoint() {}
    Q_DECL_CONSTEXPR inline FixedPoint(const Fixed &_x, const Fixed &_y) : x(_x), y(_y) {}
    Q_DECL_CONSTEXPR QPointF toPointF() const { return QPointF(x.toReal(), y.toReal()); }
    Q_DECL_CONSTEXPR static FixedPoint fromPointF(const QPointF &p) {
        return FixedPoint(Fixed::fromReal(p.x()), Fixed::fromReal(p.y()));
    }
};
Q_DECLARE_TYPEINFO(FixedPoint, Q_PRIMITIVE_TYPE);

Q_DECL_CONSTEXPR inline FixedPoint operator-(const FixedPoint &p1, const FixedPoint &p2)
{ return FixedPoint(p1.x - p2.x, p1.y - p2.y); }
Q_DECL_CONSTEXPR inline FixedPoint operator+(const FixedPoint &p1, const FixedPoint &p2)
{ return FixedPoint(p1.x + p2.x, p1.y + p2.y); }

struct FixedSize {
    Fixed width;
    Fixed height;
    Q_DECL_CONSTEXPR FixedSize() {}
    Q_DECL_CONSTEXPR FixedSize(Fixed _width, Fixed _height) : width(_width), height(_height) {}
    Q_DECL_CONSTEXPR QSizeF toSizeF() const { return QSizeF(width.toReal(), height.toReal()); }
    Q_DECL_CONSTEXPR static FixedSize fromSizeF(const QSizeF &s) {
        return FixedSize(Fixed::fromReal(s.width()), Fixed::fromReal(s.height()));
    }
};
Q_DECLARE_TYPEINFO(FixedSize, Q_PRIMITIVE_TYPE);

#endif // FIXED_H
