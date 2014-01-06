#ifndef DIALOGACCELERATOR_H
#define DIALOGACCELERATOR_H

#include <QDialog>
#include <QPushButton>

namespace Ui {
class DialogAccelerator;
}

class Accelerator;

class DialogAccelerator : public QDialog
{
    Q_OBJECT
    
public:
    explicit DialogAccelerator(Accelerator *accelerator, QWidget *parent = 0);
    ~DialogAccelerator();

public slots:
    void changed();
    virtual void accept();

private:
    Ui::DialogAccelerator *ui;

    QPushButton *m_ok;

    Accelerator *m_accelerator;
};

#endif // DIALOGACCELERATOR_H
