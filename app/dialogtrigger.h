#ifndef DIALOGTRIGGER_H
#define DIALOGTRIGGER_H

#include <QDialog>
#include <QPushButton>

class Trigger;

namespace Ui {
class DialogTrigger;
}

class DialogTrigger : public QDialog
{
    Q_OBJECT

public:
    explicit DialogTrigger(Trigger *trigger, QWidget *parent = 0);
    ~DialogTrigger();

public slots:
    void changed();
    virtual void accept();

private:
    Ui::DialogTrigger *ui;

    QPushButton *m_ok;

    Trigger *m_trigger;
};

#endif // DIALOGTRIGGER_H
