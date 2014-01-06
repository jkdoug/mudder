#include "dialogalias.h"
#include "ui_dialogalias.h"

#include "alias.h"
#include "luaedit.h"

DialogAlias::DialogAlias(Alias *alias, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAlias)
{
    ui->setupUi(this);

    QWidget::setTabOrder(ui->echo, ui->script);
    QWidget::setTabOrder(ui->script, ui->buttonBox);

    Q_ASSERT(alias != 0);
    m_alias = alias;

    ui->pattern->setText(m_alias->pattern());

    ui->sequence->setValue(m_alias->sequence());
    ui->name->setText(m_alias->name());

    ui->enabled->setChecked(m_alias->enabledFlag());
    ui->caseSensitive->setChecked(m_alias->caseSensitive());
    ui->keepEvaluating->setChecked(m_alias->keepEvaluating());
    ui->echo->setChecked(m_alias->echo());

    ui->script->setPlainText(m_alias->contents());

    m_ok = ui->buttonBox->button(QDialogButtonBox::Ok);

    changed();

    connect(ui->name, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(ui->pattern, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(ui->sequence, SIGNAL(valueChanged(int)), this, SLOT(changed()));
    connect(ui->script, SIGNAL(textChanged()), this, SLOT(changed()));
    connect(ui->caseSensitive, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    connect(ui->enabled, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    connect(ui->keepEvaluating, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    connect(ui->echo, SIGNAL(stateChanged(int)), this, SLOT(changed()));
}

DialogAlias::~DialogAlias()
{
    delete ui;
}

void DialogAlias::changed()
{
    bool changed = m_alias->name() != ui->name->text() ||
            m_alias->sequence() != ui->sequence->value() ||
            m_alias->pattern() != ui->pattern->text() ||
            m_alias->enabledFlag() != ui->enabled->isChecked() ||
            m_alias->keepEvaluating() != ui->keepEvaluating->isChecked() ||
            m_alias->caseSensitive() != ui->caseSensitive->isChecked() ||
            m_alias->echo() != ui->echo->isChecked() ||
            m_alias->contents() != ui->script->toPlainText();

    QRegularExpression regex(ui->pattern->text());
    bool valid = !ui->name->text().isEmpty() &&
            !ui->pattern->text().isEmpty() &&
            regex.isValid();

    m_ok->setEnabled(changed && valid);
}

void DialogAlias::accept()
{
    m_alias->setPattern(ui->pattern->text());

    m_alias->setName(ui->name->text());
    m_alias->setSequence(ui->sequence->value());

    m_alias->enable(ui->enabled->isChecked());
    m_alias->setKeepEvaluating(ui->keepEvaluating->isChecked());
    m_alias->setCaseSensitive(ui->caseSensitive->isChecked());
    m_alias->setEcho(ui->echo->isChecked());

    m_alias->setContents(ui->script->toPlainText());

    QDialog::accept();
}
