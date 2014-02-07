#ifndef CONFIGDISPLAY_H
#define CONFIGDISPLAY_H

#include <QColor>
#include <QWidget>
#include "configwidget.h"

namespace Ui {
class ConfigDisplay;
}

class Profile;

class ConfigDisplay : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigDisplay(Profile *profile, ConfigWidget *parent = 0);
    ~ConfigDisplay();

private slots:
    void load();
    void save();

    void on_fgCommand_clicked();
    void on_bgCommand_clicked();
    void on_fgNote_clicked();
    void on_bgNote_clicked();
    void on_checkAutoWrap_stateChanged(int arg1);

private:
    Ui::ConfigDisplay *ui;

    Profile *m_profile;

    QColor m_bgCommand;
    QColor m_fgCommand;
    QColor m_bgNote;
    QColor m_fgNote;
};

#endif // CONFIGDISPLAY_H
