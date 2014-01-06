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

#ifndef DIALOGPROFILE_H
#define DIALOGPROFILE_H

#include <QColor>
#include <QDialog>

namespace Ui {
class DialogProfile;
}

class Profile;

class DialogProfile : public QDialog
{
    Q_OBJECT

public:
    explicit DialogProfile(QWidget *parent = 0);
    ~DialogProfile();

    Profile * profile() const { return m_profile; }
    void load(Profile *profile);

private:
    void load();
    void save();
    bool validate();

private:
    Ui::DialogProfile *ui;

    Profile *m_profile;
    QColor m_fgCommand;
    QColor m_bgCommand;
    QColor m_fgNote;
    QColor m_bgNote;

public slots:
    virtual void accept();

private slots:
    void on_pageList_currentRowChanged(int currentRow);
    void on_scriptFilenameBrowse_clicked();
    void on_fgCommand_clicked();
    void on_bgCommand_clicked();
    void on_fgNote_clicked();
    void on_bgNote_clicked();
    void on_checkAutoWrap_stateChanged(int arg1);
    void on_loggingDirectoryBrowse_clicked();
};

#endif // DIALOGPROFILE_H
