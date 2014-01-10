#include "dialogaccelerator.h"
#include "ui_dialogaccelerator.h"
#include "accelerator.h"

DialogAccelerator::DialogAccelerator(Accelerator *accelerator, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogAccelerator)
{
    ui->setupUi(this);

    Q_ASSERT(accelerator != 0);
    m_accelerator = accelerator;

    ui->key->setKeySequence(m_accelerator->key());

    ui->name->setText(m_accelerator->name());

    ui->enabled->setChecked(m_accelerator->enabledFlag());

    ui->script->setPlainText(m_accelerator->contents());

    m_ok = ui->buttonBox->button(QDialogButtonBox::Ok);

    changed();

    connect(ui->name, SIGNAL(textChanged(QString)), this, SLOT(changed()));
    connect(ui->script, SIGNAL(textChanged()), this, SLOT(changed()));
    connect(ui->enabled, SIGNAL(stateChanged(int)), this, SLOT(changed()));
    connect(ui->key, SIGNAL(keySequenceChanged(QKeySequence)), this, SLOT(changed()));
}

DialogAccelerator::~DialogAccelerator()
{
    delete ui;
}

void DialogAccelerator::changed()
{
    bool changed = m_accelerator->name() != ui->name->text() ||
            m_accelerator->key() != ui->key->keySequence() ||
            m_accelerator->enabledFlag() != ui->enabled->isChecked() ||
            m_accelerator->contents() != ui->script->toPlainText();

    bool valid = !ui->name->text().isEmpty() &&
            !ui->key->keySequence().isEmpty();

    m_ok->setEnabled(changed && valid);
}

void DialogAccelerator::accept()
{
    m_accelerator->setKey(ui->key->keySequence());

    m_accelerator->setName(ui->name->text());

    m_accelerator->enable(ui->enabled->isChecked());

    m_accelerator->setContents(ui->script->toPlainText());

    QDialog::accept();
}
