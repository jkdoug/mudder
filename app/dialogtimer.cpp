#include "dialogtimer.h"
#include "ui_dialogtimer.h"
#include "timer.h"

DialogTimer::DialogTimer(Timer *timer, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTimer)
{
    ui->setupUi(this);

    m_timer = timer;

    ui->interval->setTime(m_timer->interval());
    ui->name->setText(m_timer->name());

    ui->enabled->setChecked(m_timer->enabledFlag());
    ui->once->setChecked(m_timer->once());

    ui->script->setPlainText(m_timer->contents());

    ui->timesFired->setText(QLocale::system().toString(m_timer->firedCount()));
    ui->avgTime->setText(QString::number(m_timer->averageTime()));

    m_ok = ui->buttonBox->button(QDialogButtonBox::Ok);

    changed();

    connect(ui->interval, SIGNAL(timeChanged(QTime)), this, SLOT(changed()));
    connect(ui->name, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(ui->script, SIGNAL(textChanged()), this, SLOT(changed()));
    connect(ui->enabled, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    connect(ui->once, SIGNAL(stateChanged(int)), this, SLOT(changed()));
}

DialogTimer::~DialogTimer()
{
    delete ui;
}

void DialogTimer::changed()
{
    bool changed = m_timer->name() != ui->name->text() ||
            m_timer->interval() != ui->interval->time() ||
            m_timer->enabledFlag() != ui->enabled->isChecked() ||
            m_timer->once() != ui->once->isChecked() ||
            m_timer->contents() != ui->script->toPlainText();

    bool valid = !ui->name->text().isEmpty() &&
            ui->interval->time().isValid();

    m_ok->setEnabled(changed && valid);
}

void DialogTimer::accept()
{
    m_timer->setInterval(ui->interval->time());
    m_timer->setName(ui->name->text());

    m_timer->enable(ui->enabled->isChecked());
    m_timer->setOnce(ui->once->isChecked());

    m_timer->setContents(ui->script->toPlainText());

    QDialog::accept();
}
