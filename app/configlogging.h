#ifndef CONFIGLOGGING_H
#define CONFIGLOGGING_H

#include <QWidget>
#include "configwidget.h"

namespace Ui {
class ConfigLogging;
}

class Profile;

class ConfigLogging : public QWidget
{
    Q_OBJECT

public:
    explicit ConfigLogging(Profile *profile, ConfigWidget *parent = 0);
    ~ConfigLogging();

private slots:
    void load();
    void save();
    bool validate();

    void on_loggingDirectoryBrowse_clicked();

private:
    Ui::ConfigLogging *ui;

    Profile *m_profile;
};

#endif // CONFIGLOGGING_H
