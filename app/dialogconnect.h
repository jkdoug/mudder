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


#ifndef DIALOGCONNECT_H
#define DIALOGCONNECT_H

#include <QDialog>
#include <QString>

namespace Ui {
class DialogConnect;
}

class DialogConnect : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogConnect(QWidget *parent = 0);
    ~DialogConnect();

    void load();
    void save();
    bool validate();

    const QString & name() const { return m_name; }
    const QString & address() const { return m_address; }
    int port() const { return m_port; }
    bool autoConnect() const { return m_connect; }

private:
    Ui::DialogConnect *ui;

    QString m_name;
    QString m_address;
    int m_port;
    bool m_connect;

public slots:
    virtual void accept();
};

#endif // DIALOGCONNECT_H
