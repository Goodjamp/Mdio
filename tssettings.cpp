#include "tssettings.h"
#include "ui_tssettings.h"

TsSettings::TsSettings(int index, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TsSettings)
{
    ui->setupUi(this);
    ui->lTS->setText("TC" + QString::number(index));
    ui->cbTS->setCurrentText("Не задано");
}

TsSettings::~TsSettings()
{
    delete ui;
}

bool TsSettings::isInvert()
{
    return ui->cbTS->currentIndex() == 1;
}

void TsSettings::setInvert(bool invert)
{
    ui->cbTS->setCurrentIndex(invert == false ? 0 : 1);
}

void  TsSettings::setEnableCb(bool enable)
{
    ui->cbTS->setEnabled(enable);
    if (enable == false) {
        ui->cbTS->setCurrentIndex(-1);
    }
}

bool TsSettings::isConfigurationSeted()
{
    return ui->cbTS->currentIndex() != -1;
}
