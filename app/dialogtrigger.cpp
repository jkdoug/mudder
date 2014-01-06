#include "dialogtrigger.h"
#include "ui_dialogtrigger.h"
#include "trigger.h"

DialogTrigger::DialogTrigger(Trigger *trigger, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogTrigger)
{
    ui->setupUi(this);

    Q_ASSERT(trigger != 0);
    m_trigger = trigger;

    ui->pattern->setText(m_trigger->pattern());

    ui->sequence->setValue(m_trigger->sequence());
    ui->name->setText(m_trigger->name());

    ui->enabled->setChecked(m_trigger->enabledFlag());
    ui->caseSensitive->setChecked(m_trigger->caseSensitive());
    ui->keepEvaluating->setChecked(m_trigger->keepEvaluating());
    ui->repeat->setChecked(m_trigger->repeat());
    ui->omit->setChecked(m_trigger->omit());

    ui->script->setPlainText(m_trigger->contents());

    m_ok = ui->buttonBox->button(QDialogButtonBox::Ok);

    changed();

    connect(ui->name, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(ui->pattern, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(ui->sequence, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(ui->script, SIGNAL(textChanged()), this, SLOT(changed()));
    connect(ui->caseSensitive, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    connect(ui->enabled, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    connect(ui->keepEvaluating, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    connect(ui->repeat, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    connect(ui->omit, SIGNAL(stateChanged(int)), this, SLOT(changed()));
}

DialogTrigger::~DialogTrigger()
{
    delete ui;
}

void DialogTrigger::changed()
{
    bool changed = m_trigger->name() != ui->name->text() ||
            m_trigger->sequence() != ui->sequence->value() ||
            m_trigger->pattern() != ui->pattern->text() ||
            m_trigger->enabledFlag() != ui->enabled->isChecked() ||
            m_trigger->keepEvaluating() != ui->keepEvaluating->isChecked() ||
            m_trigger->caseSensitive() != ui->caseSensitive->isChecked() ||
            m_trigger->repeat() != ui->repeat->isChecked() ||
            m_trigger->omit() != ui->omit->isChecked() ||
            m_trigger->contents() != ui->script->toPlainText();

    QRegularExpression regex(ui->pattern->text());
    bool valid = !ui->name->text().isEmpty() &&
            !ui->pattern->text().isEmpty() &&
            regex.isValid();

    m_ok->setEnabled(changed && valid);
}

void DialogTrigger::accept()
{
    m_trigger->setPattern(ui->pattern->text());

    m_trigger->setName(ui->name->text());
    m_trigger->setSequence(ui->sequence->value());

    m_trigger->enable(ui->enabled->isChecked());
    m_trigger->setKeepEvaluating(ui->keepEvaluating->isChecked());
    m_trigger->setCaseSensitive(ui->caseSensitive->isChecked());
    m_trigger->setRepeat(ui->repeat->isChecked());
    m_trigger->setOmit(ui->omit->isChecked());

    m_trigger->setContents(ui->script->toPlainText());

    QDialog::accept();
}
