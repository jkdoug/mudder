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


#ifndef LOGGERFACTORY_H
#define LOGGERFACTORY_H

#include <QMap>
#include <QString>
#include <QStringList>

template <class BaseClass>
class LoggerFactoryInterface
{
public:
    LoggerFactoryInterface() {}
    virtual ~LoggerFactoryInterface() {}

    virtual BaseClass * createInstance() = 0;
};

template <class BaseClass, class ActualClass>
class LoggerFactoryItem : public LoggerFactoryInterface<BaseClass>
{
public:
    LoggerFactoryItem() {}
    virtual ~LoggerFactoryItem() {}

    inline virtual BaseClass * createInstance() { return new ActualClass; }

protected:
    QString m_tag;
};

template <class BaseClass>
class LoggerFactory
{
public:
    LoggerFactory() {}
    ~LoggerFactory() {}

    void registerFactoryInterface(const QString &tag, LoggerFactoryInterface<BaseClass> *factoryInterface)
    {
        Q_ASSERT(!tag.isEmpty());
        if (!tag.isEmpty() && !m_registered.values().contains(factoryInterface))
        {
            m_registered[tag] = factoryInterface;
        }
    }
    inline void unregisterFactoryInterface(const QString &tag) { m_registered.remove(tag); }

    QStringList tags() const { return m_registered.keys(); }
    inline bool isTagValid(const QString &tag) const { return m_registered.contains(tag); }

    BaseClass * createInstance(const QString &tag)
    {
        Q_ASSERT(isTagValid(tag));
        return isTagValid(tag)?m_registered[tag]->createInstance():0;
    }

 private:
    QMap<QString, LoggerFactoryInterface<BaseClass> *> m_registered;
};

#endif // LOGGERFACTORY_H
