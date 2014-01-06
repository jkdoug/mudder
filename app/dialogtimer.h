#ifndef DIALOGTIMER_H
#define DIALOGTIMER_H

#include <QDialog>
#include <QPushButton>

namespace Ui {
class DialogTimer;
}

class Timer;

class DialogTimer : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogTimer(Timer *timer, QWidget *parent = 0);
    ~DialogTimer();

public slots:
    void changed();
    virtual void accept();

private:
    Ui::DialogTimer *ui;

    QPushButton *m_ok;

    Timer *m_timer;
};

#endif // DIALOGTIMER_H
