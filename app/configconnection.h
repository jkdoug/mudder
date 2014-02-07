#ifndef CONFIGCONNECTION_H
#define CONFIGCONNECTION_H

#include <QWidget>
#include "configwidget.h"

namespace Ui {
class ConfigConnection;
}

class Profile;

class ConfigConnection : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigConnection(Profile *profile, ConfigWidget *parent = 0);
    ~ConfigConnection();

private slots:
    void load();
    void save();
    bool validate();

private:
    Ui::ConfigConnection *ui;

    Profile *m_profile;
};

#endif // CONFIGCONNECTION_H
